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

LIBS = -Wl,$(OUTDIR)/bin/cppunit/cppunittester.a -Wl,$(WORKDIR)/LinkTarget/CppunitTest/libtest_sc_filters_test.a -Wl,$(OUTDIR)/lib/bootstrap.uno.a -Wl,$(OUTDIR)/lib/unoexceptionprotector.a $(SOLARLIB) -lavmedialo -lbasegfxlo -lcanvastoolslo -lcomphelpgcc3 -lcppcanvaslo -lcppunit -ldrawinglayerlo -leditenglo -lforlo -lforuilo -lfwelo -lfwilo -lfwklo -lgcc3_uno -li18nisolang1gcc3 -li18npaperlo -li18nutilgcc3 -licuuc -ljpeg -ljvmfwk -llnglo -lreg -lsal_textenc -lsalcpprt -lsaxlo -lsclo -lsfxlo -lstore -lsvllo -lsvtlo -lsvxlo -lsvxcorelo -lsotlo -ltest -ltklo -ltllo -lucbhelper4gcc3 -luno_cppuhelpergcc3 -luno_cppu -luno_sal -luno_salhelpergcc3 -lunotest -lutllo -lvcllo -lxml2 -lxmlreader -lxolo -lz $(addprefix -framework , $(gb_Library__FRAMEWORKS)) -liconv -lobjc

all: $(APPDIR)/$(APP)

$(APPDIR)/$(APP): $(SRCS)
	mkdir -p $(APPDIR)
	$(CXX) -o $@ $(CFLAGS) $(gb_OBJCXXFLAGS) $(SRCS) $(LIBS)

clean:
	rm -rf $(APPDIR)
