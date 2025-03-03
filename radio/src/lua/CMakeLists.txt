
macro(add_lua_export_target target)
  add_custom_command(
    OUTPUT lua_exports_${target}.inc
    COMMAND ${CMAKE_C_COMPILER} -E ${ARGN} -DEXPORT ${RADIO_SRC_DIRECTORY}/dataconstants.h > lua_exports_${target}.txt
    COMMAND ${PYTHON_EXECUTABLE} ${RADIO_DIRECTORY}/util/luaexport.py ${VERSION} lua_exports_${target}.txt lua_exports_${target}.inc lua_fields_${target}.txt
    )
  add_custom_target(lua_export_${target} DEPENDS lua_exports_${target}.inc)
endmacro(add_lua_export_target)

set(LUA_INCLUDES
  -DCPUARM -DLUA -DLUA_INPUTS -DLUA_EXPORT_GENERATION -DSIMU
  -I${RADIO_SRC_DIRECTORY}
  -I${RADIO_SRC_DIRECTORY}/targets/${TARGET_DIR}
  -I${RADIO_SRC_DIRECTORY}/thirdparty
  -I${RADIO_SRC_DIRECTORY}/targets/common/arm/stm32
  )

if(PYTHONINTERP_FOUND)
  add_lua_export_target(x9lite    ${LUA_INCLUDES} -DPCBTARANIS -DPCBX9LITE -DPCBX9)
  add_lua_export_target(x9lites   ${LUA_INCLUDES} -DPCBTARANIS -DPCBX9LITES -DPCBX9LITE -DPCBX9)
  add_lua_export_target(x7        ${LUA_INCLUDES} -DPCBTARANIS -DPCBX7 -DRADIO_X7)
  add_lua_export_target(x7access  ${LUA_INCLUDES} -DPCBTARANIS -DPCBX7ACCESS -DPCBX7 -DRADIO_X7ACCESS)
  add_lua_export_target(xlite     ${LUA_INCLUDES} -DPCBTARANIS -DPCBXLITE)
  add_lua_export_target(xlites    ${LUA_INCLUDES} -DPCBTARANIS -DPCBXLITES -DPCBXLITE -DGYRO)
  add_lua_export_target(x9d       ${LUA_INCLUDES} -DPCBTARANIS -DPCBX9D -DPCBX9)
  add_lua_export_target(x9d+2019  ${LUA_INCLUDES} -DPCBTARANIS -DPCBX9D -DPCBX9 -DRADIO_X9DP2019)
  add_lua_export_target(x9e       ${LUA_INCLUDES} -DPCBTARANIS -DPCBX9E -DPCBX9)
  add_lua_export_target(tango     ${LUA_INCLUDES} -DPCBTARANIS -DPCBX7 -DPCBTANGO -DRADIO_TANGO)
  add_lua_export_target(mambo     ${LUA_INCLUDES} -DPCBTARANIS -DPCBX7 -DPCBMAMBO -DRADIO_MAMBO)
  add_lua_export_target(x10       ${LUA_INCLUDES} -DPCBHORUS   -DPCBX10)
  add_lua_export_target(x12s      ${LUA_INCLUDES} -DPCBHORUS   -DPCBX12S)
  add_lua_export_target(t12       ${LUA_INCLUDES} -DPCBTARANIS -DPCBX7 -DRADIO_T12)
  add_lua_export_target(tlite     ${LUA_INCLUDES} -DPCBTARANIS -DPCBX7 -DRADIO_TLITE)
  add_lua_export_target(tx12      ${LUA_INCLUDES} -DPCBTARANIS -DPCBX7 -DRADIO_TX12)
  add_lua_export_target(t8        ${LUA_INCLUDES} -DPCBTARANIS -DPCBX7 -DRADIO_T8)
  add_lua_export_target(t16       ${LUA_INCLUDES} -DPCBHORUS   -DPCBX10 -DRADIO_T16)
endif()
