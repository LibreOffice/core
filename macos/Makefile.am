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

## The languages we ship are the ones in the shared Langs.conf, like the Qt app
## and the browser ship (see qt/Makefile.am). Apple spells some of them
## differently: zh-CN is zh-Hans, zh-TW is zh-Hant, and en-US is Base.lproj.
LANGS_CONF = $(top_srcdir)/engine/distro-configs/Langs.conf
CONF_LANGS = $(shell test ! -f "$(LANGS_CONF)" || sed -n 's/^--with-lang=//p' "$(LANGS_CONF)" \
    | tr ' ' '\n' | grep -v '^en-US$$' | sed -e 's/^zh-CN$$/zh-Hans/' -e 's/^zh-TW$$/zh-Hant/')

## All PO files, the shipped ones and the ones Weblate added on top
ALL_PO_FILES = $(wildcard $(PO_DIR)/*.po)
PO_LANGS = $(basename $(notdir $(ALL_PO_FILES)))

## A language in Langs.conf ships once Weblate has created its PO file. Without
## Langs.conf (a dist tarball) every PO file ships.
LANGS = $(if $(CONF_LANGS),$(filter $(CONF_LANGS),$(PO_LANGS)),$(PO_LANGS))
PO_FILES = $(addprefix $(PO_DIR)/,$(addsuffix .po,$(LANGS)))

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
	@# The Xcode build copies every <lang>.lproj into the bundle, so drop the
	@# ones an earlier build made for a language that no longer ships.
	@set -e; \
	for dir in $(CODA_DIR)/*.lproj; do \
	    lang=$$(basename "$$dir" .lproj); \
	    case " Base $(LANGS) " in \
	        *" $$lang "*) ;; \
	        *) echo "  removing stale $$dir"; rm -rf "$$dir";; \
	    esac; \
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
	for po in $(ALL_PO_FILES); do \
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
	for lang in $(PO_LANGS); do \
	    rm -rf "$(CODA_DIR)/$$lang.lproj"; \
	done
	rm -rf coda.derived coda.xcarchive
	rm -rf "$(srcdir)/coda/coda.xcodeproj/project.xcworkspace" "$(srcdir)/coda/coda.xcodeproj/xcuserdata"
	@set -e; \
	for iconset in $(DOCUMENT_ICON_SETS); do \
	    rm -rf "$(CODA_DIR)/Assets.xcassets/$$iconset.imageset"; \
	done
