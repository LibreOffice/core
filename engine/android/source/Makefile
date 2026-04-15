ifeq ($(BUILDDIR),)
gb_Side:=host
include ../../config_host.mk
endif

# The default target builds the APK and runs lint.

all: link-so

DISABLE_UI=TRUE
BUILD_VARIANT=$(if $(DISABLE_UI),StrippedUI)$(if $(ENABLE_ANDROID_EDITING),Editing)$(if $(ENABLE_RELEASE_BUILD),Release,Debug)
BOOTSTRAPDIR=../Bootstrap
include $(BOOTSTRAPDIR)/Makefile.shared

native-code.cxx: $(SRCDIR)/solenv/bin/native-code.py
	@echo "[build PY ] $@"
	$< -j -g core -g writer -g calc -g draw -g edit > $@

clean:
	rm -rf assets assets_strippedUI build jniLibs jniLibs_debug res_generated $(OBJLOCAL)
	rm -f native-code.cxx
	rm -f liboSettings.gradle
