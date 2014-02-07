$(eval $(call gb_CppunitTest_CppunitTest,vcl_app_test))

$(eval $(call gb_CppunitTest_add_exception_objects,vcl_app_test, \
    vcl/qa/cppunit/app/test_IconThemeScanner \
    vcl/qa/cppunit/app/test_IconThemeSelector \
))

# add a list of all needed libraries here
$(eval $(call gb_CppunitTest_use_libraries,vcl_app_test, \
sal \
vcl \
$(gb_STDLIBS) \
))

$(eval $(call gb_CppunitTest_set_include,vcl_app_test, \
$$(INCLUDE) \
-I$(OUTDIR)/inc \
 ))
