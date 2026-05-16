MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = coda-qt

include_paths = -I${top_srcdir}/common -I${top_srcdir}/net -I${top_srcdir}/kit -I${top_srcdir}/wsd -I${top_srcdir} ${ZSTD_CFLAGS} ${QT6_CFLAGS}

warning_flags = -Wall -Werror -Wno-parentheses -Wno-sign-compare -Wno-unused-variable
define_flags = -DMOBILEAPP=1

AM_CPPFLAGS = -pthread \
              -DCOOLWSD_DATADIR='"'@COOLWSD_DATADIR@'"' \
              -DQTAPP \
              ${define_flags} \
              ${warning_flags} \
              ${include_paths}

if !ENABLE_DEBUG
AM_CPPFLAGS += -DNDEBUG
endif

AM_LDFLAGS = -pthread ${QT6_LIBS} -ldl $(ZSTD_LIBS) ${PNG_LIBS} -lPocoZip

# Android's CMakefile list re-worked:

cmake_list= \
            ../common/Authorization.cpp \
            ../common/ConfigUtil.cpp \
            ../common/SettingsStorage.cpp \
            ../common/CommandControl.cpp \
            ../common/Crypto-stub.cpp \
            ../common/FileUtil.cpp \
            ../common/FileUtil-unix.cpp \
            ../common/Log-common.cpp \
            ../common/Log-poco.cpp \
            ../common/MobileApp.cpp \
            ../common/TraceEvent.cpp \
            ../common/ProcUtil.cpp \
            ../common/Protocol.cpp \
            ../common/RegexUtil.cpp \
            ../common/Simd.cpp \
            ../common/StringVector.cpp \
            ../common/Session.cpp \
            ../common/SigUtil-mobile.cpp \
            ../common/SpookyV2.cpp \
            ../common/Unit.cpp \
            ../common/Unit-mobile.cpp \
            ../common/Util.cpp \
            ../common/Util-mobile.cpp \
            ../common/Util-unix.cpp \
            ../common/Uri.cpp \
            ../common/RecentFiles.cpp \
            ../kit/ChildSession.cpp \
            ../kit/DeltaSimd.c \
            ../kit/Kit.cpp \
            ../kit/KitQueue.cpp \
            ../kit/LogUI.cpp \
            ../kit/KitWebSocket.cpp \
            ../net/FakeSocket.cpp \
            ../net/Socket.cpp \
            ../wsd/ClientSession.cpp \
            ../wsd/DocumentBroker.cpp \
            ../wsd/COOLWSD.cpp \
            ../wsd/ClientRequestDispatcher.cpp \
            ../wsd/RequestDetails.cpp \
            ../wsd/RequestVettingStation.cpp \
            ../wsd/SlideCache.cpp \
            ../wsd/Storage.cpp \
            ../wsd/TileCache.cpp \
            ../wsd/WSDGlobals.cpp \
            ../wsd/coolwsd-fork.cpp \
            ../wsd/dumpWsdState.cpp


# Sources that will be created by Qt's MOC
BUILT_SOURCES = bridge.moc.cpp DBusService.moc.cpp
bridge.moc.cpp: bridge.hpp
	$(MOC) $(MOCFLAGS) -o $@ $<
DBusService.moc.cpp: DBusService.hpp
	$(MOC) $(MOCFLAGS) -o $@ $<

CLEANFILES = $(BUILT_SOURCES)

coda_qt_CXXFLAGS = $(AM_CXXFLAGS)
if X86
if !CXX_CLANG
coda_qt_CXXFLAGS += -mno-direct-extern-access
endif
endif

coda_qt_SOURCES = coda-qt.cpp Application.cpp Bridge.cpp QtClipboard.cpp DocumentOperations.cpp WebView.cpp DBusService.cpp Document.cpp $(cmake_list)
nodist_coda_qt_SOURCES = $(BUILT_SOURCES)

metainfodir = $(datadir)/metainfo
metainfo_DATA = com.collaboraoffice.Office.metainfo.xml

desktopdir = $(datadir)/applications
desktop_DATA = com.collaboraoffice.Office.desktop

iconsdir = $(datadir)/icons
nobase_dist_icons_DATA = \
	hicolor/16x16/apps/com.collaboraoffice.Office.startcenter.png \
	hicolor/24x24/apps/com.collaboraoffice.Office.startcenter.png \
	hicolor/32x32/apps/com.collaboraoffice.Office.startcenter.png \
	hicolor/48x48/apps/com.collaboraoffice.Office.startcenter.png \
	hicolor/64x64/apps/com.collaboraoffice.Office.startcenter.png \
	hicolor/128x128/apps/com.collaboraoffice.Office.startcenter.png \
	hicolor/256x256/apps/com.collaboraoffice.Office.startcenter.png \
	hicolor/512x512/apps/com.collaboraoffice.Office.startcenter.png \
	hicolor/scalable/apps/com.collaboraoffice.Office.startcenter.svg

EXTRA_DIST = tools/add-release.py

### --- Qt translations integration begin ---

# List of translation languages
LANGS = ar ca cs cy da de el es eu fi fr ga gl he hr hu hy id is it ja kk ko nl pl pt pt_BR ro ru sk sl sq sv tr uk zh_CN zh_TW

# Translation source (.ts) and compiled (.qm) files
TS_FILES = $(addprefix translations/coda_,$(addsuffix .ts,$(LANGS)))
QM_FILES = $(TS_FILES:.ts=.qm)

# Rule: generate .qm from .ts
%.qm: %.ts
	mkdir -p $(dir $@)
	@$(LRELEASE) $< -qm $@

# Update .ts files from source code
update-translations:
	@$(LUPDATE) $(top_srcdir) -ts $(TS_FILES)

# Build all translations
translations: $(QM_FILES)

# Include translations in normal build and install
all-local: translations
install-data-local: translations

# Where to install translations
translationsdir = $(datadir)/coda-qt/translations
translations_DATA = $(QM_FILES)

# Distribute sources and clean up generated files
EXTRA_DIST += $(TS_FILES) run-from-build.in
CLEANFILES += $(QM_FILES)

### --- Qt translations integration end ---

### --- WebdriverIO test integration begin ---

TEST_DIR = test

if ENABLE_QTAPP_TESTS

$(TEST_DIR)/node_modules: $(TEST_DIR)/package.json
	cd $(TEST_DIR) && npm install
	@touch $(TEST_DIR)/node_modules

# Run Qt app tests
check-qtapp: coda-qt $(TEST_DIR)/node_modules
	@echo "Running WebdriverIO tests..."
	cd $(TEST_DIR) && AT_SPI_DRIVER_PATH='@AT_SPI_DRIVER_PATH@' \
	    CODA_QT_BINARY=$(abs_builddir)/coda-qt \
	    ./run-tests.sh

# Clean test artifacts
clean-local:
	rm -rf $(TEST_DIR)/node_modules
	rm -f $(TEST_DIR)/package-lock.json

endif ENABLE_QTAPP_TESTS

# Distribute test files regardless of ENABLE_QTAPP_TESTS
EXTRA_DIST += \
	$(TEST_DIR)/README.md \
	$(TEST_DIR)/package.json \
	$(TEST_DIR)/tsconfig.json \
	$(TEST_DIR)/wdio.conf.ts \
	$(TEST_DIR)/wdio.d.ts \
	$(TEST_DIR)/run-tests.sh \
	$(TEST_DIR)/lib/coda-qt.service.ts \
	$(TEST_DIR)/lib/webview.ts \
	$(TEST_DIR)/fixtures/simple.odt \
	$(TEST_DIR)/specs/harness.spec.ts \
	$(TEST_DIR)/specs/document-lifecycle.spec.ts \
	$(TEST_DIR)/specs/open-file.spec.ts

### --- WebdriverIO test integration end ---

distclean-local:
	rm -rf .deps
