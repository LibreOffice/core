NDK_HOME:=$(shell type -p ndk-build)
NDK_HOME:=$(shell dirname $(NDK_HOME))

SODEST=libs/armeabi-v7a
OBJLOCAL=obj/local/armeabi-v7a

define COPY
cp $(1) $(SODEST)$(if $(2),/$(2)) && \
arm-linux-androideabi-strip --strip-debug $(SODEST)$(if $(2),/$(2),/$(notdir $(1))) && \
cp $(1) $(OBJLOCAL)$(if $(2),/$(2))
endef

define CONDCOPY
test -f $(1) && $(call COPY,$(1),$(2))
endef

# The default target just builds
all: ndk-build
	unset JAVA_HOME && ant debug
	@echo 'Install it on the device with ant debug install'
	@echo 'Then run it with something like what "make run" does (see Makefile)'

ndk-build:
	ndk-build V=1
#
# Copy shared libraries we need to libs/armeabi-v7a so that ant will
# include them in the .apk.
#
# Copy them to obj/local/armeabi-v7a, too, where gdb will look for
# them.
#
# First the cppunittester "program" from here, which as all "program"
# files we build for Android actually is a shared object.
	$(call COPY,../../$(INPATH)/bin/cppunittester,libcppunittester.so)
#
# The cppunit library
	$(call COPY,$(OUTDIR)/lib/libcppunit-1.12.so)
#
# cppunit "plug-ins", first ones from sal
#
	$(call COPY,../../$(INPATH)/lib/*.so)
#
# and ones from other modules. Note that depending on when you try
# this, these might not have been built yet.
#
	for F in $(SRC_ROOT)/cppu/$(INPATH)/lib/qa_*.so; do \
	    $(call CONDCOPY,$${F},`basename $${F}`); \
	done
#
	for F in i18npool_test_breakiterator; do \
	    $(call CONDCOPY,$(WORKDIR)/LinkTarget/CppunitTest/libtest_$${F}.so); \
	done

#
# Other "programs"
	$(call COPY,$(OUTDIR)/bin/uno,libunoexe.so)
#
# Libs and UNO components that the tests from other modules need.
#
	for F in $(strip \
		  comphelpgcc3 \
		  gcc3_uno \
		  i18nisolang1gcc3 \
		  i18nutilgcc3 \
		  icudata \
		  icui18n \
		  icuuc \
		  reg \
		  store \
		  ucbhelper4gcc3 \
		  uno_cppu \
		  uno_salhelpergcc3 \
	          uno_cppuhelpergcc3 \
		  unoexceptionprotector \
		  xml2 \
		  xmlreader \
		  bootstrap.uno \
		  i18npool.uno); do \
	    $(call CONDCOPY,$(OUTDIR)/lib/lib$${F}.so); \
	done
#
# Then the shared GNU C++ library
	$(call COPY,$(NDK_HOME)/sources/cxx-stl/gnu-libstdc++/libs/armeabi-v7a/libgnustl_shared.so)
#
# Then other "assets". Let the directory structure under assets mimic
# that under solver for now.
	mkdir -p assets/bin assets/xml/ure assets/ComponentTarget/i18npool/util
	cp $(OUTDIR)/bin/udkapi.rdb assets/bin
	cp $(OUTDIR)/bin/types.rdb assets/bin
	cp $(OUTDIR)/xml/ure/services.rdb assets/xml/ure
	cp $(WORKDIR)/ComponentTarget/i18npool/util/i18npool.component assets/ComponentTarget/i18npool/util

install: ndk-build
	unset JAVA_HOME && ant debug install

run: install
# Note: these are just examples. The full path the the app's directory
# needs to be supplied in a couple of places, unfortunately.

#	adb shell am start -n org.libreoffice.android/.Bootstrap -e lo-main-library libcppunittester -e lo-main-cmdline "/data/data/org.libreoffice.android/lib/libqa_sal_types.so"
#	adb shell am start -n org.libreoffice.android/.Bootstrap -e lo-main-library libcppunittester -e lo-main-cmdline "/data/data/org.libreoffice.android/lib/libtest_i18npool_test_breakiterator.so --protector libunoexceptionprotector.so unoexceptionprotector '-env:UNO_TYPES=file:///assets/bin/udkapi.rdb file:///assets/bin/types.rdb'  '-env:UNO_SERVICES=file:///assets/xml/ure/services.rdb  file:///assets/ComponentTarget/i18npool/util/i18npool.component' -env:LO_LIB_DIR=file:///data/data/org.libreoffice.android/lib" 
