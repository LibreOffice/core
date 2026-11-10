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

.PHONY: macos-localized-strings
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

.PHONY: app
app:
	xcodebuild -project $(srcdir)/coda/coda.xcodeproj -scheme coda -configuration Debug \
            -derivedDataPath coda.derived archive -archivePath coda.xcarchive ARCHS="$$(uname -m)" \
            CODE_SIGN_STYLE=Manual CODE_SIGN_IDENTITY=- DEVELOPMENT_TEAM= \
            MARKETING_VERSION=$(COOLWSD_VERSION_MAJOR)$(COOLWSD_VERSION_MINOR).$(COOLWSD_VERSION_MICRO).$(COOLWSD_VERSION_NANO) \
            COMPILER_INDEX_STORE_ENABLE=NO

## ------------------------------------------------------------
## Cleanup helpers
## ------------------------------------------------------------

## Icon sets that macos/coda/generate-document-icons.sh, an Xcode build-phase script, creates from
## scratch under Assets.xcassets when Xcode builds the app:
DOCUMENT_ICON_SETS = DocumentIcon SpreadsheetIcon PresentationIcon DrawingIcon

clean-local:
	@echo "Cleaning generated .strings files"
	@set -e; \
	for lang in $(LANGS); do \
	    rm -rf "$(CODA_DIR)/$$lang.lproj"; \
	done
	rm -rf coda.derived coda.xcarchive
	rm -rf "$(srcdir)/coda/coda.xcodeproj/project.xcworkspace" "$(srcdir)/coda/coda.xcodeproj/xcuserdata"
	@set -e; \
	for iconset in $(DOCUMENT_ICON_SETS); do \
	    rm -rf "$(CODA_DIR)/Assets.xcassets/$$iconset.imageset"; \
	done
