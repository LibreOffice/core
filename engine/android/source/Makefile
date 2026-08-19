ifeq ($(BUILDDIR),)
gb_Side:=host
include ../../config_host.mk
endif

# The default target builds the APK and runs lint.

all: link-so

BOOTSTRAPDIR=$(SRCDIR)/android/Bootstrap
include $(BOOTSTRAPDIR)/Makefile.shared

native-code.cxx: $(SRCDIR)/solenv/bin/native-code.py
	@echo "[build PY ] $@"
	$< -j -g core -g writer -g calc -g draw -g edit > $@

clean:
	rm -rf $(OBJLOCAL)
	rm -f native-code.cxx
