$(eval $(call gb_CppunitTest_CppunitTest,sw_page_line_spacing))

$(eval $(call gb_CppunitTest_use_common_precompiled_header,sw_page_line_spacing))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_page_line_spacing, \
    sw/qa/extras/pagelinespacing/pagelinespacing \
))

$(eval $(call gb_CppunitTest_use_libraries,sw_page_line_spacing, \
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

$(eval $(call gb_CppunitTest_use_externals,sw_page_line_spacing,\
	boost_headers \
	libxml2 \
))

$(eval $(call gb_CppunitTest_use_custom_headers,sw_page_line_spacing,\
    officecfg/registry \
))

$(eval $(call gb_CppunitTest_set_include,sw_page_line_spacing,\
    -I$(SRCDIR)/sw/inc \
    -I$(SRCDIR)/sw/source/core/inc \
    -I$(SRCDIR)/sw/source/core/text \
    -I$(SRCDIR)/sw/source/uibase/inc \
    -I$(SRCDIR)/sw/qa/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sw_page_line_spacing,\
	udkapi \
	offapi \
))

$(eval $(call gb_CppunitTest_use_ure,sw_page_line_spacing))
$(eval $(call gb_CppunitTest_use_vcl,sw_page_line_spacing))

$(eval $(call gb_CppunitTest_use_rdb,sw_page_line_spacing,services))

$(eval $(call gb_CppunitTest_use_configuration,sw_page_line_spacing))

$(eval $(call gb_CppunitTest_use_uiconfigs,sw_page_line_spacing,\
    modules/swriter \
))

$(eval $(call gb_CppunitTest_use_more_fonts,sw_page_line_spacing))
