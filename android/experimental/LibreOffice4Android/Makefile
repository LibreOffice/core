include ../../../config_host.mk
include ../../Bootstrap/Makefile.shared

# The package of this app
APP_PACKAGE=org.libreoffice

# The default target just builds.

all: build-ant

copy-stuff:
# First always clean
	rm -rf libs $(OBJLOCAL)
	mkdir -p $(SODEST) $(OBJLOCAL)
#
# Copy shared libraries (including UNO components) we need to
# libs/armeabi-v7a so that ant will include them in the .apk.
#
# Copy them to obj/local/armeabi-v7a, too, where gdb will look for
# them.
#
	for F in $(strip \
		   analysislo \
		   basebmplo \
		   basegfxlo \
		   bootstrap.uno \
		   chartcontrollerlo \
		   chartcorelo \
		   comphelpgcc3 \
		   cppcanvaslo \
		   ctllo \
		   datelo \
		   dbaxmllo \
		   dbtoolslo \
		   embobj \
		   evtattlo \
		   expwrap.uno \
		   fastsax.uno \
		   fileacc \
		   forlo \
		   foruilo \
		   frmlo \
		   fsstorage.uno \
		   gcc3_uno \
		   hwplo \
		   hyphenlo \
		   i18nisolang1gcc3 \
		   i18npool.uno \
		   i18nutilgcc3 \
		   icudatalo \
		   icui18nlo \
		   iculelo \
		   icuuclo \
		   introspection.uno \
		   java_uno \
		   juh \
		   juhx \
		   jvmaccessgcc3 \
		   jvmfwk \
		   libotouchlo \
		   lnthlo \
		   lo-bootstrap \
		   localebe1.uno \
		   localedata_en \
		   localedata_es \
		   localedata_euro \
		   localedata_others \
		   lwpftlo \
		   mergedlo \
		   msfilterlo \
		   mswordlo \
		   ooxlo \
		   reflection.uno \
		   reg \
		   saxlo \
		   sclo \
		   sclo \
		   scdlo \
		   scfiltlo \
		   sdlo \
		   sddlo \
		   smdlo \
		   sotlo \
		   spelllo \
		   stocservices.uno \
		   store \
		   svgfilterlo \
		   svllo \
		   swdlo \
		   swlo \
		   swdlo \
		   t602filterlo \
		   textinstream.uno \
		   tllo \
		   ucbhelper4gcc3 \
		   ucppkg1 \
		   uno_cppu \
		   uno_cppuhelpergcc3 \
		   uno_sal \
		   uno_salhelpergcc3 \
		   uno_cppuhelpergcc3 \
		   unordflo \
		   unoxmllo \
		   utllo \
		   vbahelperlo \
		   vbaswobj.uno \
		   wpftdrawlo \
		   wpftwriterlo \
		   vcllo \
		   xml2 \
		   xmlfdlo \
		   xmlreader \
		   xmlsecurity \
		   xoflo \
		   xslt \
		   xstor \
		  ); do \
	    $(call COPYSO,$(OUTDIR)/lib/lib$${F}.so); \
	done
#
# Then the shared GNU C++ library
	$(call COPYSO,$(ANDROID_NDK_HOME)/sources/cxx-stl/gnu-libstdc++/$(ANDROID_NDK_TOOLCHAIN_VERSION_SUBDIR)libs/$(ABI_PATH)/libgnustl_shared.so)
#
# Then other "assets". Let the directory structure under assets mimic
# that under solver for now.
#
# Please note that I have no idea what all of this is really necessary and for
# much of this stuff being copied, no idea whether it makes any sense at all.
# Much of this is copy-pasted from android/qa/sc/Makefile (where a couple of
# unit tests for sc are built, and those do seem to mostly work) and
# android/qa/desktop/Makefile (mmeeks's desktop demo, also works to some
# extent).
#
	mkdir -p assets/bin/ure assets/lib assets/program assets/xml/ure assets/ComponentTarget/i18npool/util
	cp $(OUTDIR)/bin/udkapi.rdb assets/bin
	cp $(OUTDIR)/bin/types.rdb assets/bin
	cp $(OUTDIR)/bin/ure/types.rdb assets/bin/ure
# For some reason the vnd.sun.star.expand:$LO_LIB_DIR doesn't seem to work, it expands to empty!?
# So just hardcode the known APP_DATA_PATH for now...
	for F in xml/services xml/ure/services; do \
		sed -e 's!uri="vnd.sun.star.expand:$$LO_LIB_DIR/!uri="file://$(APP_DATA_PATH)/lib/!g' <$(OUTDIR)/$$F.rdb >assets/$$F.rdb; \
	done
	cp $(SRC_ROOT)/odk/examples/java/DocumentHandling/test/test1.odt \
	   $(SRC_ROOT)/sc/qa/unit/data/xls/border.xls \
	   $(SRC_ROOT)/sw/qa/core/data/odt/test.odt \
	   $(SRC_ROOT)/sw/qa/core/data/doc/testVba.doc \
		assets
	cp $(WORKDIR)/ComponentTarget/i18npool/util/i18npool.component assets/ComponentTarget/i18npool/util
#
	mkdir -p assets/ure/share/misc assets/share/registry/res assets/share/config/soffice.cfg
	cp -R $(OUTDIR)/xml/*.xcd assets/share/registry
	mv assets/share/registry/fcfg_langpack_en-US.xcd assets/share/registry/res
	cp -R $(OUTDIR)/xml/uiconfig/* assets/share/config/soffice.cfg
	cp -R $(OUTDIR)/xml/registry/* assets/share/registry
#
# Set up rc, the "inifile". See BootstrapMap::getBaseIni(). As this app
# doesn't use soffice_main() (at least I think it shouldn't), the
# rtl::Bootstrap::setIniFilename() call there that hardcodes
# /assets/program/lofficerc isn't executed. Instead the hardcoding of
# /assets/rc in BootstrapMap::getBaseIni() gets used.
	echo '[Bootstrap]' > assets/rc
	echo 'Logo=1' >> assets/rc
	echo 'NativeProgress=1' >> assets/rc
	echo 'URE_BOOTSTRAP=file:///assets/program/fundamentalrc' >> assets/rc
#	echo 'RTL_LOGFILE=file:///dev/log/main' >> assets/rc
	echo "HOME=$(APP_DATA_PATH)/cache" >> assets/rc
	echo "OSL_SOCKET_PATH=$(APP_DATA_PATH)/cache" >> assets/rc
#
# Set up fundamentalrc
	echo '[Bootstrap]' > assets/program/fundamentalrc
	echo "LO_LIB_DIR=file:$(APP_DATA_PATH)/lib/" >> assets/program/fundamentalrc
	echo "URE_LIB_DIR=file://$(APP_DATA_PATH)/lib/" >> assets/program/fundamentalrc # checkme - is this used to find configs ?
	echo 'BRAND_BASE_DIR=file:///assets' >> assets/program/fundamentalrc
	echo 'CONFIGURATION_LAYERS=xcsxcu:$${BRAND_BASE_DIR}/share/registry module:$${BRAND_BASE_DIR}/share/registry/modules res:$${BRAND_BASE_DIR}/share/registry' >> assets/program/fundamentalrc
	echo 'URE_BIN_DIR=file:///assets/ure/bin/dir/not-here/can-we/exec-anyway' >> assets/program/fundamentalrc
	echo 'URE_MORE_TYPES=file:///assets/bin/ure/types.rdb file:///assets/bin/types.rdb' >> assets/program/fundamentalrc
	echo 'URE_MORE_SERVICES=file:///assets/xml/services.rdb <$$BRAND_BASE_DIR/program/services>*' >> assets/program/fundamentalrc
#
# Set up unorc
	echo '[Bootstrap]' > assets/program/unorc
	echo "URE_INTERNAL_LIB_DIR=file://$(APP_DATA_PATH)/lib/" >> assets/program/unorc
	echo 'UNO_TYPES=file:///assets/bin/ure/types.rdb file:///assets/bin/types.rdb $${URE_MORE_TYPES}' >> assets/program/unorc
	echo 'UNO_SERVICES=file:///assets/xml/ure/services.rdb $${URE_MORE_SERVICES}' >> assets/program/unorc
#
# Set up bootstraprc
	echo '[Bootstrap]' > assets/program/bootstraprc
	echo 'InstallMode=<installmode>' >> assets/program/bootstraprc
	echo 'ProductKey=LibreOffice 3.6' >> assets/program/bootstraprc
	echo "UserInstallation=file://$(APP_DATA_PATH)" >> assets/program/bootstraprc
#
# Set up versionrc
	echo '[Version]' > assets/program/versionrc
	echo 'AllLanguages=en-US' >> assets/program/versionrc
	echo 'BuildVersion=' >> assets/program/versionrc
	echo 'buildid=dead-beef' >> assets/program/versionrc
	echo 'ProductBuildid=3' >> assets/program/versionrc
	echo 'ProductMajor=360' >> assets/program/versionrc
	echo 'ProductMinor=1' >> assets/program/versionrc
	echo 'ProductSource=OOO350' >> assets/program/versionrc
	echo 'ReferenceOOoMajorMinor=3.6' >> assets/program/versionrc
#
# .res files
	mkdir -p assets/program/resource
	cp $(OUTDIR)/bin/*en-US.res assets/program/resource
#
# Assets that are unpacked at run-time into the app's data directory. These
# are files read by non-LO code, fontconfig and freetype for now, that doesn't
# understand "/assets" paths.
	mkdir -p assets/unpack/etc/fonts
	cp fonts.conf assets/unpack/etc/fonts
	mkdir -p assets/unpack/user/fonts
# $UserInstallation/user/fonts is added to the fontconfig path in
# vcl/generic/fontmanager/helper.cxx: psp::getFontPath(). UserInstallation is
# set to the app's data dir above.
	cp $(OUTDIR)/pck/Liberation*.ttf assets/unpack/user/fonts
	cp $(OUTDIR)/pck/Gen*.ttf assets/unpack/user/fonts
	cp $(OUTDIR)/pck/opens___.ttf assets/unpack/user/fonts
#
# Then gdbserver and gdb.setup so that we can debug with ndk-gdb.
#
	cp $(ANDROID_NDK_GDBSERVER) $(SODEST)
	echo set solib-search-path ./obj/local/$(ABI_PATH) >$(SODEST)/gdb.setup

build-ant: copy-stuff properties
	echo sdk.dir=$(ANDROID_SDK_HOME) >../../abs-lib/local.properties

#
# Copy jar files we need
#
	for F in $(strip \
		   java_uno \
		   juh \
		   jurt \
		   ridl \
		   unoil \
		   unoloader \
		  ); do \
	    $(call COPYSOJAR,$(OUTDIR)/bin/$${F}.jar); \
	done
#
	unset JAVA_HOME && $(ANT) debug

run:
# /data/local/tmp/sample-document.odt
	adb shell am start -n $(APP_PACKAGE)/.ui.LibreOfficeUIActivity -e input /assets/test1.odt -e lo-main-delay 20
