include ../../../config_host.mk

# Stuff lifted from solenv/gbuild/platform/IOS_ARM_GCC.mk

gb_OBJC_OBJCXX_COMMON_FLAGS := -fobjc-abi-version=2 -fobjc-legacy-dispatch -D__IPHONE_OS_VERSION_MIN_REQUIRED=40300
gb_OBJCFLAGS := -x objective-c $(gb_OBJC_OBJCXX_COMMON_FLAGS)
gb_OBJCXXFLAGS := -x objective-c++ $(gb_OBJC_OBJCXX_COMMON_FLAGS)

gb_Library__FRAMEWORKS := \
    Foundation \
    CoreFoundation \
    CoreGraphics \
    UIKit \

APP=lo-qa-sc-filters-test
APPDIR=$(APP).app

SRCS = filters-test.m

CFLAGS = $(SOLARINC)

LIBS = -Wl,$(OUTDIR)/bin/cppunit/cppunittester.a -Wl,$(WORKDIR)/LinkTarget/CppunitTest/libtest_sc_filters_test.a -Wl,$(OUTDIR)/lib/unoexceptionprotector.a $(SOLARLIB) -lavmedialo -lbasegfxlo -lcomphelpgcc3 -leditenglo -lfwelo -lfwilo -lfwklo -li18nisolang1gcc3 -li18npaperlo -licuuc -ljpeg -llnglo -lsalcpprt -lsfxlo -lsvllo -lsvtlo -lutllo -lvcllo -lsaxlo -lsclo -lsotlo -ltest -ltklo -ltllo -lucbhelper4gcc3 -lxolo -lcppunit -lunotest -luno_cppuhelpergcc3 -luno_cppu -luno_salhelpergcc3 -luno_sal -lsal_textenc $(addprefix -framework , $(gb_Library__FRAMEWORKS)) -lobjc

all: $(APPDIR)/$(APP)

$(APPDIR)/$(APP): $(SRCS)
	mkdir -p $(APPDIR)
	$(CXX) -o $@ $(CFLAGS) $(gb_OBJCXXFLAGS) $(SRCS) $(LIBS)
