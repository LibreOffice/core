$(eval $(call gb_CppunitTest_CppunitTest,svl_lngmisc))

$(eval $(call gb_CppunitTest_add_exception_objects,svl_lngmisc, \
svl/qa/unit/test_lngmisc \
))

# add a list of all needed libraries here
$(eval $(call gb_CppunitTest_add_linked_libs,svl_lngmisc, \
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
$(eval $(call gb_CppunitTest_add_linked_libs,svl_lngmisc, \
	oleaut32 \
))
endif

$(eval $(call gb_CppunitTest_set_include,svl_lngmisc,\
-I$(realpath $(SRCDIR)/svl/source/inc) \
-I$(realpath $(SRCDIR)/svl/inc) \
$$(INCLUDE) \
-I$(OUTDIR)/inc \
))