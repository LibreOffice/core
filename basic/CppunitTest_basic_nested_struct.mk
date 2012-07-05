$(eval $(call gb_CppunitTest_CppunitTest,basic_nested_struct))

$(eval $(call gb_CppunitTest_add_exception_objects,basic_nested_struct, \
    basic/qa/cppunit/test_nested_struct \
))

# add a list of all needed libraries here
$(eval $(call gb_CppunitTest_use_libraries,basic_nested_struct, \
    comphelper \
    cppu \
    cppuhelper \
    sal \
    salhelper \
    sb \
    sot \
    svl \
    svt \
    tl \
    utl \
    vcl \
    xcr \
    test \
    $(gb_STDLIBS) \
))

ifeq ($(GUI),WNT)
$(eval $(call gb_CppunitTest_use_libraries,basic_nested_struct, \
	oleaut32 \
))
endif

$(eval $(call gb_CppunitTest_set_include,basic_nested_struct,\
-I$(SRCDIR)/basic/source/inc \
-I$(SRCDIR)/basic/inc \
$$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,basic_nested_struct,\
    offapi \
    udkapi \
))

$(eval $(call gb_CppunitTest_use_ure,basic_nested_struct))

$(eval $(call gb_CppunitTest_use_components,basic_nested_struct,\
    configmgr/source/configmgr \
    ucb/source/core/ucb1 \
))
$(eval $(call gb_CppunitTest_use_configuration,basic_nested_struct))
