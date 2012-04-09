include ../../../config_host.mk

APP=lo-qa-sc-filters-test

# If run from Xcode, check the CURRENT_ARCH env var for which platform we are
# building, device or simulator

ifeq ($(CURRENT_ARCH),i386)
PLATFORM=iphonesimulator
else
PLATFORM=iphoneos
endif

APPDIR=build/Debug-$(PLATFORM)/$(APP).app

# Stuff lifted from solenv/gbuild/platform/IOS_ARM_GCC.mk

gb_OSDEFS := \
	-D$(OS) \
	-D_PTHREADS \
	-DUNIX \
	-DUNX \
	-D_REENTRANT \
	-DNO_PTHREAD_PRIORITY \
	$(EXTRA_CDEFS) \

gb_OBJC_OBJCXX_COMMON_FLAGS := -fobjc-abi-version=2 -fobjc-legacy-dispatch -D__IPHONE_OS_VERSION_MIN_REQUIRED=40300 -g
gb_OBJCFLAGS := -x objective-c $(gb_OBJC_OBJCXX_COMMON_FLAGS)
gb_OBJCXXFLAGS := -x objective-c++ $(gb_OBJC_OBJCXX_COMMON_FLAGS)

gb_Library__FRAMEWORKS := \
    Foundation \
    CoreFoundation \
    CoreGraphics \
    CoreText \
    UIKit \

SRCS = filters-test.m

CFLAGS = $(SOLARINC)

LIBS = -Wl,$(OUTDIR)/bin/cppunit/cppunittester.a -Wl,$(WORKDIR)/LinkTarget/CppunitTest/libtest_sc_filters_test.a -Wl,$(OUTDIR)/lib/bootstrap.uno.a -Wl,$(OUTDIR)/lib/configmgr.uno.a -Wl,$(OUTDIR)/lib/i18npool.uno.a -Wl,$(OUTDIR)/lib/unoexceptionprotector.a $(SOLARLIB) -lavmedialo -lbasegfxlo -lcanvastoolslo -lcomphelpgcc3 -lcppcanvaslo -lcppunit -ldrawinglayerlo -leditenglo -lforlo -lforuilo -lfwelo -lfwilo -lfwklo -lgcc3_uno -li18nisolang1gcc3 -li18npaperlo -li18nutilgcc3 -licudata -licui18n -licuuc -ljpeg -ljvmfwk -llnglo -lreg -lsal_textenc -lsalcpprt -lsaxlo -lsclo -lsfxlo -lstore -lsvllo -lsvtlo -lsvxlo -lsvxcorelo -lsotlo -ltest -ltklo -ltllo -lucb1 -lucbhelper4gcc3 -lucpfile1 -luno_cppuhelpergcc3 -luno_cppu -luno_sal -luno_salhelpergcc3 -lunotest -lutllo -lvcllo -lxml2 -lxmlreader -lxolo -lz $(addprefix -framework , $(gb_Library__FRAMEWORKS)) -liconv -lobjc

all: $(APPDIR)/$(APP) stuff

$(APPDIR)/$(APP): $(SRCS)
	mkdir -p $(APPDIR)
	$(CXX) -Wl,-map,$(APP).map.mangled -o $@ $(gb_OSDEFS) $(CFLAGS) $(gb_OBJCXXFLAGS) $(SRCS) $(LIBS)
	c++filt <$(APP).map.mangled >$(APP).map && rm $(APP).map.mangled

stuff:
# inifile:
	echo 'URE_BOOTSTRAP=$${ORIGIN}/fundamentalrc' >$(APPDIR)/rc
#
# URE_BOOTSTRAP file, fundamentalrc:
	( \
	 echo '[Bootstrap]'; \
	 echo 'BRAND_BASE_DIR=$${ORIGIN}'; \
	 echo 'CONFIGURATION_LAYERS=xcsxcu:$${BRAND_BASE_DIR}/registry module:$${BRAND_BASE_DIR}/registry/modules res:$${BRAND_BASE_DIR}/registry' \
	) >$(APPDIR)/fundamentalrc

#
# hmm, once again, a hodgepodge of stuff, I don't really know which
# files of which are actually needed...
	mkdir -p $(APPDIR)/registry/res
	cp $(OUTDIR)/xml/*.xcd $(APPDIR)/registry
	mv $(APPDIR)/registry/fcfg_langpack_en-US.xcd $(APPDIR)/registry/res
	cp -R $(OUTDIR)/xml/registry/* $(APPDIR)/registry
#
# .rdb files
	cp $(OUTDIR)/bin/udkapi.rdb $(OUTDIR)/bin/types.rdb $(OUTDIR)/xml/ure/services.rdb $(APPDIR)
#
# a bunch of .component files
	for F in framework/util/fwk i18npool/util/i18npool sfx2/util/sfx ucb/source/core/ucb1 ucb/source/ucp/file/ucpfile1 unoxml/source/service/unoxml configmgr/source/configmgr basic/util/sb chart2/source/controller/chartcontroller chart2/source/tools/charttools chart2/source/model/chartmodel comphelper/util/comphelp eventattacher/source/evtatt fileaccess/source/fileacc filter/source/config/cache/filterconfig1 oox/util/oox package/source/xstor/xstor package/util/package2 sax/source/expatwrap/expwrap sax/source/fastparser/fastsax sc/util/sc sc/util/scfilt scaddins/source/analysis/analysis scaddins/source/datefunc/date sot/util/sot svl/util/svl toolkit/util/tk ucb/source/ucp/tdoc/ucptdoc1 unotools/util/utl unoxml/source/rdf/unordf; do \
		mkdir -p $(APPDIR)/ComponentTarget/`dirname $$F`; \
	        cp $(WORKDIR)/ComponentTarget/$$F.component $(APPDIR)/ComponentTarget/$$F.component; \
	done

#
# .res files
#
# $BRAND_BASE_DIR/program/resource hardcoded in
# ResMgrContainer::init() in tools/source/rc/resmgr.cxx, so let's use
# that.
	mkdir -p $(APPDIR)/program/resource
	for F in $(OUTDIR)/bin/*.res; do \
		cp $$F $(APPDIR)/program/resource; \
	done

clean:
	rm -rf $(APPDIR)
