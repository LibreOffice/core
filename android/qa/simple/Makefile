THIS DIRECTORY IS UNMAINTAINED AND PRESUMABLY BROKEN ALREADY


include ../../../config_host.mk

# The package of this app
APP_PACKAGE=org.libreoffice.android.qa

# The name of the Bootstrap activity class
BOOTSTRAP=org.libreoffice.android.Bootstrap

# The full path the the app's directory needs to be supplied in a
# couple of places, unfortunately.

APP_DATA_PATH=/data/data/$(APP_PACKAGE)

SODEST=libs/$(ANDROID_APP_ABI)
OBJLOCAL=obj/local/$(ANDROID_APP_ABI)

define COPY
cp $(1) $(SODEST)$(if $(2),/$(2)) && $(STRIP) --strip-debug $(SODEST)$(if $(2),/$(2),/$(notdir $(1))) && \
cp $(1) $(OBJLOCAL)$(if $(2),/$(2))
endef

# The default target just builds.

all: build-ant

copy-stuff:
#
# Copy shared libraries we need to libs/$(ANDROID_APP_ABI) so that ant will
# include them in the .apk.
#
# Copy them to obj/local/$(ANDROID_APP_ABI), too, where gdb will look for
# them.
#
	mkdir -p $(SODEST) $(OBJLOCAL)
#
# First the most important one, liblo-bootstrap.so
	$(call COPY,$(OUTDIR)/lib/liblo-bootstrap.so)
#
# Then the cppunittester "program". As all "program"
# files we build for Android actually is a shared object.
	$(call COPY,$(OUTDIR)/bin/cppunit/cppunittester,libcppunittester.so)
#
# The cppunit library
	$(call COPY,$(OUTDIR)/lib/libcppunit-1.13.so)
#
# cppunit "plug-ins", first ones from sal
#
	$(call COPY,$(OUTDIR)/lib/libuno_sal*.so)
#
# and ones from other modules. Note that depending on when you try
# this, these might not have been built yet.
#
	for F in i18npool_test_breakiterator cppu cppuhelper sal; do \
	    $(call COPY,$(WORKDIR)/LinkTarget/CppunitTest/libtest_$${F}*.so); \
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
		  icudatalo \
		  icui18nlo \
		  icuuclo \
		  localedata_en \
		  localedata_others \
		  mergedlo \
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
		  ); do \
	    $(call COPY,$(OUTDIR)/lib/lib$${F}.so); \
	done
#
# Then the shared GNU C++ library
	$(call COPY,$(ANDROID_NDK_HOME)/sources/cxx-stl/gnu-libstdc++/libs/$(ANDROID_APP_ABI)/libgnustl_shared.so)
#
# bridgetest components, not in solver
	for F in bridgetest constructors cppobj; do \
		$(call COPY,$(SRC_ROOT)/testtools/$(INPATH)/lib/lib$${F}.uno.so); \
	done
#
# Then other "assets". Let the directory structure under assets mimic
# that under solver for now.
	mkdir -p assets/bin assets/lib assets/xml/ure assets/ComponentTarget/i18npool/util
	cp $(OUTDIR)/bin/udkapi.rdb assets/bin
	cp $(OUTDIR)/bin/types.rdb assets/bin
	cp $(OUTDIR)/xml/ure/services.rdb assets/xml/ure
	cp $(WORKDIR)/ComponentTarget/i18npool/util/i18npool.component assets/ComponentTarget/i18npool/util
	for F in uno_services uno_types; do \
		sed -e 's!uri="./!uri="$(APP_DATA_PATH)/lib/!g' <$(SRC_ROOT)/testtools/$(INPATH)/lib/$${F}.rdb >assets/lib/$${F}.rdb; \
	done

build-ant: copy-stuff
	unset JAVA_HOME && ant debug

install: copy-stuff
	unset JAVA_HOME && ant debug install
	@echo
	@echo 'Run it with something like what "make run" does (see Makefile)'
	@echo

run:
# Note: these are just examples. 
#
	adb shell am start -n $(APP_PACKAGE)/$(BOOTSTRAP) -e lo-main-library libcppunittester -e lo-main-cmdline "$(APP_DATA_PATH)/lib/libqa_sal_types.so"
#
	sleep 10
	adb shell am start -n $(APP_PACKAGE)/$(BOOTSTRAP) -e lo-main-library libcppunittester -e lo-main-cmdline "$(APP_DATA_PATH)/lib/libtest_i18npool_test_breakiterator.so --protector libunoexceptionprotector.so unoexceptionprotector '-env:UNO_TYPES=file:///assets/bin/udkapi.rdb file:///assets/bin/types.rdb'  '-env:UNO_SERVICES=file:///assets/xml/ure/services.rdb  file:///assets/ComponentTarget/i18npool/util/i18npool.component' -env:LO_LIB_DIR=file://$(APP_DATA_PATH)/lib"
#
#	adb shell am start -n $(APP_PACKAGE)/$(BOOTSTRAP) -e lo-main-library libunoexe -e lo-main-cmdline "-ro /assets/lib/uno_services.rdb -ro /assets/lib/uno_types.rdb -s com.sun.star.test.bridge.BridgeTest -- com.sun.star.test.bridge.CppTestObject" -e lo-main-delay 2

clean:
	ant clean
	rm -rf assets $(SODEST) $(OBJLOCAL)

