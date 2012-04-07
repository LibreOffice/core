$(eval $(call gb_CppunitTest_CppunitTest,basic_scanner))

$(eval $(call gb_CppunitTest_add_exception_objects,basic_scanner, \
    basic/qa/cppunit/test_scanner \
))

$(eval $(call gb_CppunitTest_use_library_objects,basic_scanner,sb))

# add a list of all needed libraries here
$(eval $(call gb_CppunitTest_use_libraries,basic_scanner, \
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
    $(gb_STDLIBS) \
))

ifeq ($(GUI),WNT)
$(eval $(call gb_CppunitTest_use_libraries,basic_scanner, \
	oleaut32 \
))
endif

$(eval $(call gb_CppunitTest_set_include,basic_scanner,\
-I$(SRCDIR)/basic/source/inc \
-I$(SRCDIR)/basic/inc \
$$(INCLUDE) \
))
