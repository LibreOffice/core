include ../../../config_host.mk

APP=LODocumentLoader

# If run from Xcode, check the CURRENT_ARCH env var for which platform we are
# building, device or simulator

ifneq (,$(filter i386,$(CC)))
PLATFORM=iphonesimulator
else
PLATFORM=iphoneos
endif

APPDIR=build/Debug-$(PLATFORM)/$(APP).app

# Stuff lifted from solenv/gbuild

gb_CPUDEFS := -DARM32

gb_CPUDEFS += -D$(CPUNAME)

gb_OSDEFS := \
	-D$(OS) \
	-D_PTHREADS \
	-DUNIX \
	-DUNX \
	-D_REENTRANT \
	-DNO_PTHREAD_PRIORITY \
	$(EXTRA_CDEFS) \

gb_CPPU_ENV := gcc3

gb_COMPILERDEFS := \
	-D$(COM) \
	-DCPPU_ENV=$(gb_CPPU_ENV) \

gb_DEBUGLEVEL := 1

gb_GLOBALDEFS := \
	-D_REENTRANT \
	-DOSL_DEBUG_LEVEL=$(gb_DEBUGLEVEL) \
	-DSUPD=$(UPD) \
	$(gb_OSDEFS) \
	$(gb_COMPILERDEFS) \
	$(gb_CPUDEFS) \

gb_OBJC_OBJCXX_COMMON_FLAGS := -fobjc-abi-version=2 -fobjc-legacy-dispatch -D__IPHONE_OS_VERSION_MIN_REQUIRED=40300 -g
gb_OBJCFLAGS := -x objective-c $(gb_OBJC_OBJCXX_COMMON_FLAGS)
gb_OBJCXXFLAGS := -x objective-c++ $(gb_OBJC_OBJCXX_COMMON_FLAGS)

gb_Library__FRAMEWORKS := \
    Foundation \
    CoreFoundation \
    CoreGraphics \
    CoreText \
    UIKit \

SRCS = docloader.mm

CFLAGS = $(SOLARINC)

MOREINCLUDES = -I$(OUTDIR)/inc/udkapi -I$(OUTDIR)/inc/offapi

LIBS = $(foreach LIB, bin/cppunit/cppunittester lib/bootstrap.uno lib/configmgr.uno lib/expwrap.uno lib/fastsax.uno lib/i18npool.uno lib/introspection.uno lib/reflection.uno lib/stocservices.uno lib/unobootstrapprotector lib/unoexceptionprotector, -Wl,$(OUTDIR)/$(LIB).a) $(SOLARLIB) -lacclo -lanalysislo -lascii_expat_xmlparse -lavmedialo -lbasegfxlo -lcanvastoolslo -llcms2 -lcollator_data -lcomphelpgcc3 -lcppcanvaslo -lcppunit -lcrypto -lcuilo -ldatelo -ldbtoolslo -ldict_ja -ldict_zh -ldrawinglayerlo -leditenglo  -legilo -lemelo -lepblo -lepglo -lepplo -lepslo -leptlo -leralo -letilo -lexpat_xmlparse -lexpat_xmltok -lexplo -lfilterconfiglo -lforlo -lforuilo -lfwelo -lfwilo -lfwklo -lgcc3_uno -li18nisolang1gcc3 -li18nutilgcc3 -licdlo -licudata -licui18n -licuuc -lidxlo -limelo -lindex_data -lipblo -lipdlo -lipslo -liptlo -lipxlo -liralo -litglo -litilo -ljpeg -ljvmfwk -llnglo -llocaledata_en -llocaledata_es -llocaledata_euro -llocaledata_others -lmsfilterlo -lorcus -looxlo -lpackage2 -lreg -lsal_textenc -lsalcpprt -lsaxlo -lsax_shared -lscfiltlo -lsclo -lscuilo -lsfxlo -lsotlo -lstore -lsvllo -lsvtlo -lsvxlo -lsvxcorelo -ltest -ltextconv_dict -ltklo -ltllo -lucb1 -lucbhelper4gcc3 -lucpfile1 -lunoxmllo -luno_cppuhelpergcc3 -luno_cppu -luno_sal -luno_salhelpergcc3 -lunotest -lutllo -lvcllo -lxmlscriptlo -lxml2 -lxmlreader -lxolo -lxstor -lz $(addprefix -framework , $(gb_Library__FRAMEWORKS)) -liconv -lobjc

all: $(APPDIR)/$(APP) stuff

$(APPDIR)/$(APP): $(SRCS)
	mkdir -p $(APPDIR)
	$(CXX) -Wl,-map,$(APP).map.mangled -o $@ $(gb_GLOBALDEFS) $(CFLAGS) $(gb_OBJCXXFLAGS) $(MOREINCLUDES) $(SRCS) $(LIBS)
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
# An unorc is *mandatory*, sigh. We leave it empty except for the
# [Bootstrap} section header, though, and set all the stuff using
# -env: options in docloader.m. Whether that makes sense or not,
# no idea. The ideal would clearly be if *none* of the various rc
# files was mandatory, and the code would automatically use sane
# defaults. (Sane for the particular platform, that is.)
	( \
	 echo '[Bootstrap]' \
	) >$(APPDIR)/unorc
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
	mkdir -p $(APPDIR)/ure
	cp $(OUTDIR)/bin/ure/types.rdb $(APPDIR)/ure
#
# a bunch of .component files
	for F in framework/util/fwk i18npool/util/i18npool sfx2/util/sfx ucb/source/core/ucb1 ucb/source/ucp/file/ucpfile1 unoxml/source/service/unoxml configmgr/source/configmgr basic/util/sb chart2/source/controller/chartcontroller chart2/source/chartcore comphelper/util/comphelp eventattacher/source/evtatt fileaccess/source/fileacc filter/source/config/cache/filterconfig1 oox/util/oox package/source/xstor/xstor package/util/package2 sax/source/expatwrap/expwrap sax/source/fastparser/fastsax sc/util/sc sc/util/scfilt scaddins/source/analysis/analysis scaddins/source/datefunc/date sot/util/sot svl/util/svl toolkit/util/tk ucb/source/ucp/tdoc/ucptdoc1 unotools/util/utl unoxml/source/rdf/unordf; do \
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
#
# Test documents
	cp $(SRC_ROOT)/odk/examples/java/DocumentHandling/test/test1.odt $(APPDIR)

clean:
	rm -rf $(APPDIR)
