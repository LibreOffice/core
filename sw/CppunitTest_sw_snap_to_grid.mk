$(eval $(call gb_CppunitTest_CppunitTest,sw_snap_to_grid))

$(eval $(call gb_CppunitTest_use_common_precompiled_header,sw_snap_to_grid))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_snap_to_grid, \
    sw/qa/extras/snaptogrid/snaptogrid \
))

$(eval $(call gb_CppunitTest_use_libraries,sw_snap_to_grid, \
    comphelper \
    cppu \
    cppuhelper \
    docmodel \
    editeng \
    i18nlangtag \
    msword \
    sal \
    sfx \
    subsequenttest \
    svl \
    svt \
    svxcore \
    sw \
    swqahelper \
    test \
    unotest \
    vcl \
    tl \
    utl \
    svx \
))

$(eval $(call gb_CppunitTest_use_externals,sw_snap_to_grid,\
	boost_headers \
	libxml2 \
))

$(eval $(call gb_CppunitTest_use_custom_headers,sw_snap_to_grid,\
    officecfg/registry \
))

$(eval $(call gb_CppunitTest_set_include,sw_snap_to_grid,\
    -I$(SRCDIR)/sw/inc \
    -I$(SRCDIR)/sw/source/core/inc \
    -I$(SRCDIR)/sw/source/uibase/inc \
    -I$(SRCDIR)/sw/qa/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sw_snap_to_grid,\
	udkapi \
	offapi \
))

$(eval $(call gb_CppunitTest_use_ure,sw_snap_to_grid))
$(eval $(call gb_CppunitTest_use_vcl,sw_snap_to_grid))

$(eval $(call gb_CppunitTest_use_rdb,sw_snap_to_grid,services))

$(eval $(call gb_CppunitTest_use_configuration,sw_snap_to_grid))

$(eval $(call gb_CppunitTest_use_uiconfigs,sw_snap_to_grid,\
    modules/swriter \
))

$(eval $(call gb_CppunitTest_use_more_fonts,sw_snap_to_grid))
