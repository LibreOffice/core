NDK_HOME:=$(shell type -p ndk-build)
NDK_HOME:=$(shell dirname $(NDK_HOME))

SODEST=libs/armeabi-v7a

all:
	ndk-build V=1
#
# Copy shared libraries we need to libs/armeabi-v7a so that ant will
# include them in the .apk.
# First ones from here, sal
	cp ../../$(INPATH)/bin/cppunittester $(SODEST)/libcppunittester.so
#
# Then the cppunit library
	cp $(OUTDIR)/lib/libcppunit-1.12.so $(SODEST)
#
# Then cppunit "plug-ins", first ones from sal
#
	cp ../../$(INPATH)/lib/*.so $(SODEST)
#
# Then ones from other modules. Note that depending on when you try
# this, these might not have been built yet.
#
	-for F in $(SRC_ROOT)/cppu/$(INPATH)/lib/qa_*.so; do \
	    test -f $${F} && cp $${F} $(SODEST); \
	done
#
	-for F in i18npool_test_breakiterator; do \
	    test -f $(WORKDIR)/LinkTarget/CppunitTest/libtest_$${F}.so && cp $(WORKDIR)/LinkTarget/CppunitTest/libtest_$${F}.so $(SODEST); \
	done
#
# Then libs that the tests from other modules need.
#
	-for F in libuno_cppu libuno_salhelpergcc3 libuno_cppuhelpergcc3; do \
	    test -f $(OUTDIR)/lib/$${F}.so && cp $(OUTDIR)/lib/$${F}.so $(SODEST); \
	done
#
# Then the shared GNU C++ library
	cp $(NDK_HOME)/sources/cxx-stl/gnu-libstdc++/libs/armeabi-v7a/libgnustl_shared.so $(SODEST)
#
# Copy them to obj/local/armeabi-v7a, too, where gdb will look for
# them. Not sure if this is useful or not; I have great problems with
# ndk-gdb. Actually, commenting out this part for now...
#
#	cp ../../$(INPATH)/bin/cppunittester obj/local/armeabi-v7a/libcppunittester.so
#	cp $(OUTPATH)/lib/libcppunit-1.12.so obj/local/armeabi-v7a
#	cp ../../$(INPATH)/lib/*.so obj/local/armeabi-v7a
#	cp $(NDK_HOME)/sources/cxx-stl/gnu-libstdc++/libs/armeabi-v7a/libgnustl_shared.so obj/local/armeabi-v7a
	unset JAVA_HOME && ant debug
	@echo 'Install it on the device with ant debug install'
	@echo 'Then run it with something like what "make run" does (see Makefile)'

run: all
	unset JAVA_HOME && ant debug install
#
# Note: this is of course just an example. The full path the the test
# .so needs to be supplied, unfortunately, I guess cppunittester
# checks its existance using the pathname instead of just
# osl_loadModule'ing it.
	adb shell am start -n org.libreoffice.android/.Bootstrap -e lo-main-library libcppunittester -e lo-main-cmdline "cppunittester /data/data/org.libreoffice.android/lib/libqa_sal_types.so"
