## macos/Makefile.am
## macOS-only localization rules (storyboard + gettext)

MAINTAINERCLEANFILES = Makefile.in

## ------------------------------------------------------------
## Paths
## ------------------------------------------------------------

CODA_DIR = $(srcdir)/coda/coda
PO_DIR   = $(CODA_DIR)/po
BASE_LPROJ = $(CODA_DIR)/Base.lproj

BASE_STORYBOARD = $(BASE_LPROJ)/Main.storyboard
BASE_STRINGS    = $(BASE_LPROJ)/Main.strings
POT_FILE        = $(PO_DIR)/Main.pot

## Python tools (relative to top source dir)
IBTOOL_UTF8     = $(top_srcdir)/scripts/ibtool_utf8.sh
STRINGS_TO_POT  = $(top_srcdir)/scripts/strings_to_pot.py
PO_TO_STRINGS   = $(top_srcdir)/scripts/po_to_strings.py

## All PO files
PO_FILES = $(wildcard $(PO_DIR)/*.po)

## Language codes derived from PO file names
LANGS = $(basename $(notdir $(PO_FILES)))

## ------------------------------------------------------------
## Build-time rule: generate localized .strings from PO
## ------------------------------------------------------------

## This runs as part of "make all"
all-local: macos-localized-strings

macos-localized-strings: $(BASE_STRINGS) $(PO_FILES)
	@echo "Generating localized Main.strings from PO files"
	@set -e; \
	for lang in $(LANGS); do \
	    outdir="$(CODA_DIR)/$$lang.lproj"; \
	    outfile="$$outdir/Main.strings"; \
	    echo "  $$outfile"; \
	    mkdir -p "$$outdir"; \
	    $(PYTHON) $(PO_TO_STRINGS) \
	        $(BASE_STRINGS) \
	        $(PO_DIR)/$$lang.po \
	        "$$outfile"; \
	done

## ------------------------------------------------------------
## POT generation
## ------------------------------------------------------------

## Regenerate Base.lproj/Main.strings and Main.pot
pot: $(POT_FILE)

$(POT_FILE): $(BASE_STORYBOARD)
	@echo "Updating Base.strings from storyboard"
	@$(IBTOOL_UTF8) $(BASE_STORYBOARD) $(BASE_STRINGS)
	@echo "Generating POT file"
	@$(PYTHON) $(STRINGS_TO_POT) $(BASE_STRINGS) $(POT_FILE)

## ------------------------------------------------------------
## Update PO files (msgmerge)
## ------------------------------------------------------------

l10n: pot
	@echo "Updating PO files from $(POT_FILE)"
	@set -e; \
	for po in $(PO_FILES); do \
	    echo "  $$po"; \
	    msgmerge --quiet --update $$po $(POT_FILE); \
	done

## ------------------------------------------------------------
## Cleanup helpers
## ------------------------------------------------------------

clean-local:
	@echo "Cleaning generated .strings files"
	@set -e; \
	for lang in $(LANGS); do \
	    rm -f "$(CODA_DIR)/$$lang.lproj/Main.strings"; \
	done
