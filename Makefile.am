#
# Copyright the Collabora Online contributors.
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
ETAGS=ctags -e

build-samples:
	$(QUIET_MKDIRP) mkdir -p $(abs_top_builddir)/test/samples
	$(QUIET_SAMPLES) $(abs_top_srcdir)/scripts/build-samples.py $(abs_top_srcdir)/test/samples $(abs_top_srcdir)/test/samples

if ENABLE_MOBILEAPP

if ENABLE_MACOS
SUBDIRS = macos browser
else
if ENABLE_QTAPP
SUBDIRS = qt browser

if ENABLE_QTAPP_TESTS
check-qtapp:
	$(MAKE) $(AM_MAKEFLAGS) -C qt check-qtapp
endif ENABLE_QTAPP_TESTS

else
if ENABLE_WASM
SUBDIRS = wasm browser
else !ENABLE_WASM
SUBDIRS = browser
endif !ENABLE_WASM
endif
endif

else

SUBDIRS = browser .

if ENABLE_TESTS
SUBDIRS += test
endif

if ENABLE_CYPRESS
SUBDIRS += cypress_test
endif

export ENABLE_DEBUG

QUIET_MKDIRP = $(QUIET_MKDIRP_$(V))
QUIET_MKDIRP_ = $(QUIET_MKDIRP_$(AM_DEFAULT_VERBOSITY))
QUIET_MKDIRP_0 = @echo "  MKDIRP  " $@;

if ENABLE_LIBFUZZER
bin_PROGRAMS =
else
bin_PROGRAMS = \
	coolconvert \
	coolforkit-caps \
	coolforkit-ns \
	coolmount \
	coolstress \
	coolconfig \
	coolwsd
if ENABLE_DEBUG
bin_PROGRAMS += coolwsd-inproc
endif
endif

dist_bin_SCRIPTS = coolwsd-systemplate-setup

man_MANS = man/coolwsd.1 \
           man/coolforkit-caps.1 \
           man/coolforkit-ns.1 \
           man/coolconvert.1 \
           man/coolconfig.1 \
           man/coolstress.1 \
           man/coolwsd-systemplate-setup.1 \
           man/coolmount.1


dist_doc_DATA = wsd/README.vars \
                wsd/protocol.txt \
                wsd/metrics.txt \
                THIRDPARTYLICENSES

doc_DATA = collabora-online-sbom.spdx.json

coolwsddatadir = @COOLWSD_DATADIR@

coolwsddata_DATA = discovery.xml \
                   favicon.ico

coolwsdconfigdir = @COOLWSD_CONFIGDIR@

coolwsdconfig_DATA = coolwsd.xml \
		     coolkitconfig.xcu

apache2configdir = ${sysconfdir}/apache2/conf-available

apache2config_DATA = etc/apache2/coolwsd.conf

nginxconfigdir = ${sysconfdir}/nginx/snippets

nginxconfig_DATA = etc/nginx/coolwsd.conf

ACLOCAL_AMFLAGS = -I m4

# quick and easy for now.
include_paths = -I${top_srcdir}/common -I${top_srcdir}/net -I${top_srcdir}/wsd -I${top_srcdir}/kit ${ZLIB_CFLAGS} ${ZSTD_CFLAGS} ${PNG_CFLAGS}
if ENABLE_SSL
include_paths += ${OPENSSL_CFLAGS}
endif

AM_CPPFLAGS = -pthread \
	      ${include_paths}

if !ENABLE_DEBUG
AM_CPPFLAGS += -DNDEBUG
endif

AM_LDFLAGS = -lpam $(ZLIB_LIBS) $(ZSTD_LIBS) ${PNG_LIBS}
if !ENABLE_MACOS
AM_LDFLAGS += -Wl,-E
endif

# Clang's linker doesn't like -pthread.
if !HAVE_CLANG
AM_LDFLAGS += -pthread
endif

coolwsd_LDADD = libwsdglobals.a
coolconfig_LDADD = libglobals.a

if ENABLE_SSL
AM_LDFLAGS += ${OPENSSL_LIBS}
else
coolwsd_LDADD += ${OPENSSL_LIBS}
coolconfig_LDADD += ${OPENSSL_LIBS}
endif

if ENABLE_MACOS
util_platform_cpp = common/Util-macos.cpp
else
util_platform_cpp = common/Util-linux.cpp
endif

AM_ETAGSFLAGS = --c++-kinds=+p --fields=+iaS --extra=+q -R --totals=yes --exclude=browser/node_modules --exclude=browser/dist *
AM_CTAGSFLAGS = $(AM_ETAGSFLAGS)

shared_sources = \
                 $(util_platform_cpp) \
                 common/Authorization.cpp \
                 common/CommandControl.cpp \
                 common/ConfigUtil.cpp \
                 common/CoolMount.cpp \
                 common/FileUtil-unix.cpp \
                 common/FileUtil.cpp \
                 common/JailUtil.cpp \
                 common/Log-common.cpp \
                 common/Log-poco.cpp \
                 common/MobileApp.cpp \
                 common/ProcUtil.cpp \
                 common/Protocol.cpp \
                 common/RegexUtil.cpp \
                 common/Seccomp.cpp \
                 common/Session.cpp \
                 common/SigUtil-server.cpp \
                 common/Simd.cpp \
                 common/SpookyV2.cpp \
                 common/StringVector.cpp \
                 common/Syscall.cpp \
                 common/TraceEvent.cpp \
                 common/Unit-server.cpp \
                 common/Unit.cpp \
                 common/Uri.cpp \
                 common/Util-server.cpp \
                 common/Util-unix.cpp \
                 common/Util.cpp \
                 kit/KitQueue.cpp \
                 kit/LogUI.cpp \
                 net/DelaySocket.cpp \
                 net/HttpHelper.cpp \
                 net/HttpRequest.cpp \
                 net/NetUtil.cpp \
                 net/Socket.cpp \
                 wsd/Exceptions.cpp
if ENABLE_SSL
shared_sources += net/Ssl.cpp
endif

coolwsd_sources = \
                  common/Crypto.cpp \
                  wsd/Admin.cpp \
                  wsd/AdminModel.cpp \
                  wsd/Auth.cpp \
                  wsd/COOLWSD.cpp \
                  wsd/CacheUtil.cpp \
                  wsd/ClientRequestDispatcher.cpp \
                  wsd/ClientSession.cpp \
                  wsd/ContentType.cpp \
                  wsd/DocumentBroker.cpp \
                  wsd/FileServer.cpp \
                  wsd/FileServerUtil.cpp \
                  wsd/HostUtil.cpp \
                  wsd/ProofKey.cpp \
                  wsd/ProxyProtocol.cpp \
                  wsd/ProxyRequestHandler.cpp \
                  wsd/QuarantineUtil.cpp \
                  wsd/RemoteConfig.cpp \
                  wsd/RequestDetails.cpp \
                  wsd/RequestVettingStation.cpp \
                  wsd/ServerAuditUtil.cpp \
                  wsd/SlideCache.cpp \
                  wsd/SpecialBrokers.cpp \
                  wsd/Storage.cpp \
                  wsd/TileCache.cpp \
                  wsd/dumpWsdState.cpp \
                  wsd/wopi/CheckFileInfo.cpp \
                  wsd/wopi/StorageConnectionManager.cpp \
                  wsd/wopi/WopiProxy.cpp \
                  wsd/wopi/WopiStorage.cpp

coolwsd_SOURCES = $(coolwsd_sources) \
                  $(shared_sources) \
                  wsd/coolwsd-fork.cpp

EXTRA_coolwsd_DEPENDENCIES = browser/node_modules

if ENABLE_LIBFUZZER
noinst_PROGRAMS =
else
noinst_PROGRAMS = \
                  clientnb \
                  connect \
                  coolbench \
                  coolmap \
                  coolsocketdump \
                  lokitclient
endif

if ENABLE_LIBFUZZER
if ENABLE_AFL
# AFL++ builds 4 variants per fuzzer target
else
noinst_PROGRAMS += \
		   admin_fuzzer \
		   clientrequest_fuzzer \
		   clientsession_fuzzer \
		   httpresponse_fuzzer \
		   httpecho_fuzzer
endif
endif

connect_SOURCES = \
                  $(util_platform_cpp) \
                  common/DummyTraceEventEmitter.cpp \
                  common/Log-common.cpp \
                  common/Log-poco.cpp \
                  common/ProcUtil.cpp \
                  common/Protocol.cpp \
                  common/StringVector.cpp \
                  common/Util-server.cpp \
                  common/Util-unix.cpp \
                  common/Util.cpp \
                  tools/Connect.cpp

connect_LDADD = libglobals.a

lokitclient_SOURCES = \
                      $(util_platform_cpp) \
                      common/DummyTraceEventEmitter.cpp \
                      common/Log-common.cpp \
                      common/Log-poco.cpp \
                      common/Protocol.cpp \
                      common/ProcUtil.cpp \
                      common/StringVector.cpp \
                      common/TraceEvent.cpp \
                      common/Util-server.cpp \
                      common/Util-unix.cpp \
                      common/Util.cpp \
                      tools/KitClient.cpp

lokitclient_LDADD = libglobals.a

noinst_LIBRARIES = libsimd.a libglobals.a libkitglobals.a libwsdglobals.a libkitwsdglobals.a

libsimd_a_SOURCES = kit/DeltaSimd.c
libsimd_a_CFLAGS = @SIMD_CFLAGS@

libglobals_a_SOURCES = common/Globals.cpp

libkitglobals_a_SOURCES = kit/KitGlobals.cpp

libwsdglobals_a_SOURCES = wsd/WSDGlobals.cpp

libkitwsdglobals_a_SOURCES = wsd/KitWSDGlobals.cpp

coolforkit_sources = \
                     kit/ChildSession.cpp \
                     kit/ForKit.cpp \
                     kit/Kit.cpp \
                     kit/KitWebSocket.cpp

coolforkit_ldadd = libsimd.a libkitglobals.a

coolforkit_caps_SOURCES = $(coolforkit_sources) \
                          $(shared_sources) \
                          kit/forkit-main.cpp

coolforkit_caps_LDADD = ${coolforkit_ldadd}

# Define variant to ensure different binary content and thus different build-id
coolforkit_caps_CPPFLAGS = $(AM_CPPFLAGS) -DCOOL_FORKIT_VARIANT=\"caps\"

coolforkit_ns_SOURCES = $(coolforkit_sources) \
                        $(shared_sources) \
                        kit/forkit-main.cpp

coolforkit_ns_LDADD = ${coolforkit_ldadd}

# Define variant to ensure different binary content and thus different build-id
coolforkit_ns_CPPFLAGS = $(AM_CPPFLAGS) -DCOOL_FORKIT_VARIANT=\"ns\"

if ENABLE_DEBUG
coolwsd_inproc_SOURCES = $(coolwsd_sources) \
                         $(shared_sources) \
                         $(coolforkit_sources) \
                         wsd/coolwsd-inproc.cpp

coolwsd_inproc_LDADD = libsimd.a libkitwsdglobals.a

if !ENABLE_SSL
coolwsd_inproc_LDADD += ${OPENSSL_LIBS}
endif

endif

if ENABLE_LIBFUZZER
coolforkit_caps_SOURCES += \
		       common/DummyTraceEventEmitter.cpp
coolforkit_ns_SOURCES += \
		       common/DummyTraceEventEmitter.cpp

common_fuzzer_sources = \
		       fuzzer/Common.cpp \
		       common/DummyTraceEventEmitter.cpp \
		       $(coolwsd_sources) \
		       wsd/coolwsd-inproc.cpp \
		       wsd/KitWSDGlobals.cpp \
		       $(coolforkit_sources) \
		       $(shared_sources)

if ENABLE_AFL
# AFL++ multi-variant fuzzer build.
# 4 static libraries (one per variant) from common sources,
# then 5 fuzzers x 4 variants. To add a new fuzzer, just add to AFL_FUZZERS.

AFL_CPPFLAGS = $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) -I${top_srcdir}/test
AFL_CXXFLAGS = $(AFL_CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS)
AFL_CFLAGS = $(AFL_CPPFLAGS) $(AM_CFLAGS) $(CFLAGS)
AFL_LDFLAGS_COMMON = $(AM_LDFLAGS) $(LIBS)

# Variant configurations: ENV_<variant>, CXX_<variant>, CC_<variant>
ENV_native =
CXX_native = $(CXX)
CC_native = $(CC)

ENV_san = AFL_LLVM_ONLY_FSRV=1 AFL_USE_ASAN=1 AFL_USE_UBSAN=1 AFL_USE_CFISAN=1
CXX_san = $(AFL_CLANG_FAST_CXX)
CC_san = $(AFL_CLANG_FAST_CC)

ENV_cmpcov = AFL_LLVM_LAF_ALL=1
CXX_cmpcov = $(CXX)
CC_cmpcov = $(CC)

ENV_cmplog = AFL_LLVM_CMPLOG=1
CXX_cmplog = $(CXX)
CC_cmplog = $(CC)

AFL_VARIANTS = native san cmpcov cmplog

# Common sources split by language
common_fuzzer_cpp = $(common_fuzzer_sources)
common_fuzzer_c = kit/DeltaSimd.c

# Fuzzer registry: name=source (add new fuzzers here)
AFL_FUZZERS = \
	admin=fuzzer/Admin.cpp \
	clientrequest=fuzzer/ClientRequestDispatcherFuzz.cpp \
	clientsession=fuzzer/ClientSession.cpp \
	httpresponse=fuzzer/HttpResponse.cpp \
	httpecho=fuzzer/HttpEcho.cpp

# Extract just the names
AFL_FUZZER_NAMES = $(foreach f,$(AFL_FUZZERS),$(word 1,$(subst =, ,$f)))

# Generate all target names: <name>_fuzzer_<variant>.afl
AFL_FUZZER_TARGETS = $(foreach v,$(AFL_VARIANTS),$(foreach n,$(AFL_FUZZER_NAMES),$(n)_fuzzer_$(v).afl))

# Per-variant common object files
afl_common_objs = $(common_fuzzer_cpp:%.cpp=.afl-$(1)/%.o) $(common_fuzzer_c:%.c=.afl-$(1)/%.o)

# Per-variant static library names
AFL_LIBS = $(foreach v,$(AFL_VARIANTS),aflfuzz_$(v).a)

# Pattern rules for compiling each variant's objects
define AFL_VARIANT_RULES
.afl-$(1)/%.o: %.cpp
	@mkdir -p $$(dir $$@)
	$(ENV_$(1)) $$(CXX_$(1)) $$(AFL_CXXFLAGS) -c -o $$@ $$<

.afl-$(1)/%.o: %.c
	@mkdir -p $$(dir $$@)
	$(ENV_$(1)) $$(CC_$(1)) $$(AFL_CFLAGS) @SIMD_CFLAGS@ -c -o $$@ $$<

aflfuzz_$(1).a: $$(call afl_common_objs,$(1))
	$$(AR) crs $$@ $$^
endef

$(foreach v,$(AFL_VARIANTS),$(eval $(call AFL_VARIANT_RULES,$(v))))

# Link rule: <name>_fuzzer_<variant>.afl from variant-compiled fuzzer source + static lib
define AFL_FUZZER_LINK
$(1)_fuzzer_$(2).afl: .afl-$(2)/$(3:.cpp=.o) aflfuzz_$(2).a
	$(ENV_$(2)) $$(CXX_$(2)) -o $$@ $$< aflfuzz_$(2).a $$(AFL_DRIVER_LIB) $$(AFL_LDFLAGS_COMMON)
endef

$(foreach f,$(AFL_FUZZERS),\
  $(foreach v,$(AFL_VARIANTS),\
    $(eval $(call AFL_FUZZER_LINK,$(word 1,$(subst =, ,$f)),$(v),$(word 2,$(subst =, ,$f))))))

clean-afl:
	rm -rf $(foreach v,$(AFL_VARIANTS),.afl-$(v)) $(AFL_LIBS) $(AFL_FUZZER_TARGETS)

else
# libFuzzer build (original)

LIBFUZZER_LDFLAGS = -fsanitize=fuzzer

admin_fuzzer_CPPFLAGS = \
			$(AM_CPPFLAGS)
admin_fuzzer_SOURCES = \
		       $(common_fuzzer_sources) \
		       fuzzer/Admin.cpp
admin_fuzzer_LDFLAGS = $(LIBFUZZER_LDFLAGS) $(AM_LDFLAGS)
admin_fuzzer_LDADD = libsimd.a

clientrequest_fuzzer_CPPFLAGS = \
				$(AM_CPPFLAGS)
clientrequest_fuzzer_SOURCES = \
			       $(common_fuzzer_sources) \
			       fuzzer/ClientRequestDispatcherFuzz.cpp
clientrequest_fuzzer_LDFLAGS = $(LIBFUZZER_LDFLAGS) $(AM_LDFLAGS)
clientrequest_fuzzer_LDADD = libsimd.a

clientsession_fuzzer_CPPFLAGS = \
				$(AM_CPPFLAGS)
clientsession_fuzzer_SOURCES = \
			       $(common_fuzzer_sources) \
			       fuzzer/ClientSession.cpp
clientsession_fuzzer_LDFLAGS = $(LIBFUZZER_LDFLAGS) $(AM_LDFLAGS)
clientsession_fuzzer_LDADD = libsimd.a

httpresponse_fuzzer_CPPFLAGS = \
				$(AM_CPPFLAGS)
httpresponse_fuzzer_SOURCES = \
			       $(common_fuzzer_sources) \
			       fuzzer/HttpResponse.cpp
httpresponse_fuzzer_LDFLAGS = $(LIBFUZZER_LDFLAGS) $(AM_LDFLAGS)
httpresponse_fuzzer_LDADD = libsimd.a

httpecho_fuzzer_CPPFLAGS = \
				$(AM_CPPFLAGS) \
        -I${top_srcdir}/test
httpecho_fuzzer_SOURCES = \
			       $(common_fuzzer_sources) \
			       fuzzer/HttpEcho.cpp
httpecho_fuzzer_LDFLAGS = $(LIBFUZZER_LDFLAGS) $(AM_LDFLAGS)
httpecho_fuzzer_LDADD = libsimd.a

endif # ENABLE_AFL

endif # ENABLE_LIBFUZZER

clientnb_SOURCES = \
                   $(util_platform_cpp) \
                   common/DummyTraceEventEmitter.cpp \
                   common/Log-common.cpp \
                   common/Log-poco.cpp \
                   common/ProcUtil.cpp \
                   common/StringVector.cpp \
                   common/Util-server.cpp \
                   common/Util-unix.cpp \
                   common/Util.cpp \
                   net/clientnb.cpp

clientnb_LDADD = libsimd.a libglobals.a

coolmount_SOURCES = tools/mount.cpp \
                    common/CoolMount.cpp

coolmap_SOURCES = tools/map.cpp

coolbench_SOURCES = \
                    $(util_platform_cpp) \
                    common/DummyTraceEventEmitter.cpp \
                    common/Log-common.cpp \
                    common/Log-poco.cpp \
                    common/ProcUtil.cpp \
                    common/Simd.cpp \
                    common/StringVector.cpp \
                    common/Util-server.cpp \
                    common/Util-unix.cpp \
                    common/Util.cpp \
                    tools/Benchmark.cpp

coolbench_LDADD = libsimd.a libglobals.a

coolconvert_SOURCES = tools/Tool.cpp

coolstress_TDOC_CPPFLAGS = -DTDOC=\"$(abs_top_srcdir)/test/data\"
coolstress_CPPFLAGS = ${coolstress_TDOC_CPPFLAGS} ${include_paths} \
			$(AM_CPPFLAGS)
coolstress_SOURCES = tools/Stress.cpp \
                     common/DummyTraceEventEmitter.cpp \
                     $(shared_sources)

coolstress_LDADD = libglobals.a

coolconfig_SOURCES = \
                    common/ConfigUtil.cpp \
                    common/Crypto.cpp \
                    common/DummyTraceEventEmitter.cpp \
                    common/Log-common.cpp \
                    common/Log-poco.cpp \
                    common/ProcUtil.cpp \
                    common/StringVector.cpp \
                    common/Util-server.cpp \
                    common/Util-unix.cpp \
                    common/Util.cpp \
                    tools/Config.cpp \
                    tools/ConfigMigrationAssistant.cpp

coolsocketdump_SOURCES = tools/WebSocketDump.cpp \
			 common/DummyTraceEventEmitter.cpp \
			 $(shared_sources)

coolsocketdump_LDADD = libglobals.a

wsd_headers = \
              wsd/Admin.hpp \
              wsd/AdminModel.hpp \
              wsd/Auth.hpp \
              wsd/COOLWSD.hpp \
              wsd/COOLWSDServer.hpp \
              wsd/CacheUtil.hpp \
              wsd/ClientRequestDispatcher.hpp \
              wsd/ClientSession.hpp \
              wsd/ContentSecurityPolicy.hpp \
              wsd/ContentType.hpp \
              wsd/DocumentToolDescriptions.hpp \
              wsd/DocumentBroker.hpp \
              wsd/Exceptions.hpp \
              wsd/FileServer.hpp \
              wsd/HostUtil.hpp \
              wsd/PlatformDesktop.hpp \
              wsd/PlatformMobile.hpp \
              wsd/PlatformUnix.hpp \
              wsd/PresetsInstall.hpp \
              wsd/Process.hpp \
              wsd/ProofKey.hpp \
              wsd/ProxyProtocol.hpp \
              wsd/ProxyRequestHandler.hpp \
              wsd/QuarantineUtil.hpp \
              wsd/RemoteConfig.hpp \
              wsd/RequestDetails.hpp \
              wsd/RequestVettingStation.hpp \
              wsd/SenderQueue.hpp \
              wsd/ServerAuditUtil.hpp \
              wsd/ServerURL.hpp \
              wsd/SlideCache.hpp \
              wsd/SpecialBrokers.hpp \
              wsd/SslConfig.hpp \
              wsd/Storage.hpp \
              wsd/TileCache.hpp \
              wsd/TileDesc.hpp \
              wsd/TraceFile.hpp \
              wsd/UserMessages.hpp \
              wsd/WSDGlobals.hpp \
              wsd/wopi/CheckFileInfo.hpp \
              wsd/wopi/StorageConnectionManager.hpp \
              wsd/wopi/WopiProxy.hpp \
              wsd/wopi/WopiStorage.hpp

shared_headers = \
                 common/Anonymizer.hpp \
                 common/Authorization.hpp \
                 common/CharacterConverter.hpp \
                 common/Clipboard.hpp \
                 common/ClipboardData.hpp \
                 common/CommandControl.hpp \
                 common/Common.hpp \
                 common/ConfigUtil.hpp \
                 common/Crypto.hpp \
                 common/FileUtil.hpp \
                 common/Globals.hpp \
                 common/HexUtil.hpp \
                 common/JailUtil.hpp \
                 common/JsonUtil.hpp \
                 common/LangUtil.hpp \
                 common/Log.hpp \
                 common/Message.hpp \
                 common/MobileApp.hpp \
                 common/NumUtil.hpp \
                 common/Png.hpp \
                 common/ProcUtil.hpp \
                 common/Protocol.hpp \
                 common/Rectangle.hpp \
                 common/RegexUtil.hpp \
                 common/RenderTiles.hpp \
                 common/Seccomp.hpp \
                 common/Session.hpp \
                 common/SettingsStorage.hpp \
                 common/SigHandlerTrap.hpp \
                 common/SigUtil.hpp \
                 common/Simd.hpp \
                 common/SpookyV2.h \
                 common/StateEnum.hpp \
                 common/StaticLogHelper.hpp \
                 common/StringVector.hpp \
                 common/Syscall.hpp \
                 common/ThreadPool.hpp \
                 common/TraceEvent.hpp \
                 common/Unit.hpp \
                 common/Uri.hpp \
                 common/Util.hpp \
                 common/Watchdog.hpp \
                 common/base64.hpp \
                 common/security.h \
                 kit/BgSaveWatchDog.hpp \
                 kit/KitGlobals.hpp \
                 kit/KitQueue.hpp \
                 kit/LogUI.hpp \
                 net/AsyncDNS.hpp \
                 net/Buffer.hpp \
                 net/DelaySocket.hpp \
                 net/FakeSocket.hpp \
                 net/HttpHelper.hpp \
                 net/HttpRequest.hpp \
                 net/HttpServer.hpp \
                 net/NetUtil.hpp \
                 net/ServerSocket.hpp \
                 net/Socket.hpp \
                 net/Uri.hpp \
                 net/WebSocketHandler.hpp \
                 tools/Replay.hpp
if ENABLE_SSL
shared_headers += net/Ssl.hpp \
                  net/SslSocket.hpp
endif

kit_headers = \
              kit/ChildSession.hpp \
              kit/Delta.hpp \
              kit/DeltaSimd.h \
              kit/DummyCOKit.hpp \
              kit/Kit.hpp \
              kit/KitHelper.hpp \
              kit/KitWebSocket.hpp \
              kit/SetupKitEnvironment.hpp \
              kit/SlideCompressor.hpp \
              kit/StateRecorder.hpp \
              kit/Watermark.hpp

noinst_HEADERS = \
                 $(kit_headers) \
                 $(shared_headers) \
                 $(wsd_headers) \
                 test/HttpTestServer.hpp \
                 test/KitPidHelpers.hpp \
                 test/UnitHTTP.hpp \
                 test/UnitWSDClient.hpp \
                 test/WOPIUploadConflictCommon.hpp \
                 test/WebSocketSession.hpp \
                 test/WopiTestServer.hpp \
                 test/helpers.hpp \
                 test/lokassert.hpp \
                 test/test.hpp \
                 test/testlog.hpp \
                 tools/COOLWebSocket.hpp

dist-hook:
	git log -1 --format=%h --abbrev=10 > $(distdir)/dist_git_hash 2> /dev/null || rm $(distdir)/dist_git_hash
	$(QUIET_MKDIRP) mkdir -p $(distdir)/bundled/include/COKit/
	cp @LOKIT_PATH@/COKit/COKit.h \
	   @LOKIT_PATH@/COKit/COKit.hxx \
	   @LOKIT_PATH@/COKit/COKitEnums.h \
	   @LOKIT_PATH@/COKit/COKitInit.h \
	   @LOKIT_PATH@/COKit/COKitTypes.h \
	   $(distdir)/bundled/include/COKit/

EXTRA_DIST = \
             $(man_MANS) \
             README.md \
             THIRDPARTYLICENSES \
             cool-sbom-template.spdx.json \
             coolkitconfig.xcu.in \
             coolwsd.service \
             coolwsd.spec \
             coolwsd.xml.in \
             debian/apparmor-profile \
             debian/changelog \
             debian/compat \
             debian/control \
             debian/coolwsd-deprecated.install \
             debian/coolwsd-deprecated.postinst \
             debian/coolwsd.install \
             debian/coolwsd.postinst \
             debian/coolwsd.postrm \
             debian/coolwsd.preinst \
             debian/coolwsd.service \
             debian/copyright \
             debian/rules \
             debian/source/format \
             discovery.xml \
             etc/apache2/coolwsd.conf \
             etc/ca-chain.cert.pem \
             etc/cert.pem \
             etc/key.pem \
             etc/nginx/coolwsd.conf \
             favicon.ico \
             scripts/create-sbom.py \
             scripts/refresh-git-hooks \
             scripts/unocommands.py

if ENABLE_LIBFUZZER
CLEANUP_COMMAND=true
else
# Use coolwsd to cleanup jails, if any. If it fails, we may have a broken/old coolwsd binary, remove it to rebuild.
# A CI box may switch branches without cleaning up the binaries, if coolwsd from a broken branch is used here
# it will fail all subsequent builds, until it's rebuilt from the new branch. So removing coolwsd after failing is needed.
CLEANUP_COMMAND=if test -s ./coolwsd -a -x ./coolwsd; then echo "Cleaning up..." && ./coolwsd --disable-cool-user-checking --cleanup --o:logging.level=trace || rm -f ./coolwsd; fi
endif

CLEANUP_COVERAGE=rm -rf ${abs_top_srcdir}/gcov; find . -iname "*.gcda" -delete

if HAVE_LO_PATH

SYSTEM_STAMP = @SYSTEMPLATE_PATH@/system_stamp
RUN_GDB = $(if $(GDB_FRONTEND),$(GDB_FRONTEND),gdb --tui --args)
TEST_APPARMOR=$(shell if test 1 -eq `cat /proc/sys/kernel/apparmor_restrict_unprivileged_userns 2> /dev/null || echo 0`; then echo "enabled"; fi)
ENABLE_NAMESPACE = $(if $(TEST_APPARMOR),podman unshare,)

if ENABLE_LIBFUZZER
CLEANUP_DEPS=
else
CLEANUP_DEPS=coolwsd coolmount coolforkit-caps coolforkit-ns
endif

# Build coolwsd and coolmount first, so we can cleanup before updating
# the systemplate directory, which we can't rm if it's mounted.
$(SYSTEM_STAMP): ${top_srcdir}/coolwsd-systemplate-setup $(CLEANUP_DEPS)
	$(CLEANUP_COMMAND)
	if test "z@SYSTEMPLATE_PATH@" != "z"; then \
		chmod u+w -R --silent "@SYSTEMPLATE_PATH@"; \
		(rm -rf "@SYSTEMPLATE_PATH@" || \
			echo "WARNING: failed to remove the systemplate") && \
		${top_srcdir}/coolwsd-systemplate-setup "@SYSTEMPLATE_PATH@" "@LO_PATH@" && \
		touch $@; \
	fi

@JAILS_PATH@:
	@$(CLEANUP_COMMAND)
	$(QUIET_MKDIRP) mkdir -p $@

@CACHE_PATH@:
	@$(CLEANUP_COMMAND)
	$(QUIET_MKDIRP) mkdir -p $@

presets-dir:
	@rm -rf $(abs_top_srcdir)/test/data/presets
	@cp -r $(abs_top_srcdir)/test/data/presets-default $(abs_top_srcdir)/test/data/presets

cleanup:
	$(CLEANUP_COMMAND)

# Always cleanup before removing the binaries.
# Note: this can break with -j option!
clean-am: cleanup clean-binPROGRAMS clean-generic clean-libtool clean-local clean-noinstPROGRAMS mostlyclean-am clean-afl

clean-local:
	$(CLEANUP_COMMAND)
	if test "z@SYSTEMPLATE_PATH@" != "z"; then rm -rf "@SYSTEMPLATE_PATH@" || echo "WARNING: failed to remove the systemplate"; fi
	if test "z@JAILS_PATH@" != "z"; then rm -rf "@JAILS_PATH@" || echo "WARNING: failed to remove all jails cleanly"; fi
	if test "z@CACHE_PATH@" != "z"; then rm -rf "@CACHE_PATH@" || echo "WARNING: failed to remove all cache cleanly"; fi
	rm -rf "${top_srcdir}/loleaflet"
	rm -rf loolconfig loolconvert loolforkit loolmap loolmount # kill old binaries
	rm -rf loolwsd loolwsd_fuzzer coolwsd_fuzzer loolstress loolsocketdump
	rm -rf collabora-online-sbom.spdx.json
	$(CLEANUP_COVERAGE)

if ENABLE_DEBUG
# can write to /tmp/coolwsd.log
  OUTPUT_TO_FILE=true
  FIND_FREE_PORT_PARAM=--find-free-port
else
# can't write to /var/log/coolwsd.log
  OUTPUT_TO_FILE=false
endif

QUIET_SAMPLES = $(QUIET_SAMPLES_$(V))
QUIET_SAMPLES_ = $(QUIET_SAMPLES_$(AM_DEFAULT_VERBOSITY))
QUIET_SAMPLES_0 = @echo "  SAMPLES " $@;

QUIET_SBOM = $(QUIET_SBOM_$(V))
QUIET_SBOM_ = $(QUIET_SBOM_$(AM_DEFAULT_VERBOSITY))
QUIET_SBOM_0 = @echo "  SBOM    " $@;

setup-wsd: all @JAILS_PATH@ @CACHE_PATH@ presets-dir build-samples
	@echo "Launching coolwsd"
	@fc-cache "@LO_PATH@"/share/fonts/truetype
	@echo

COMMON_PARAMS = \
	$(FIND_FREE_PORT_PARAM) \
	$(if $(TEST_APPARMOR),--disable-cool-user-checking) \
	--o:sys_template_path="@SYSTEMPLATE_PATH@" \
	--o:child_root_path="@JAILS_PATH@" \
	--o:cache_files.path="@CACHE_PATH@" \
	--o:storage.filesystem[@allow]=true \
	--o:ssl.cert_file_path="$(abs_top_srcdir)/etc/cert.pem" \
	--o:ssl.key_file_path="$(abs_top_srcdir)/etc/key.pem" \
	--o:ssl.ca_file_path="$(abs_top_srcdir)/etc/ca-chain.cert.pem" \
	--o:admin_console.username=admin --o:admin_console.password=admin

run: setup-wsd
	@if test "${TEST_APPARMOR}" = "enabled"; then \
		echo apparmor_restrict_unprivileged_userns is enabled, which blocks user namespaces; \
	        (which podman > /dev/null 2>&1 && echo "Using podman unshare to workaround") || \
		(echo "WARNING: install podman to workaround this" && false); \
	fi
	$(ENABLE_NAMESPACE) ./coolwsd $(COMMON_PARAMS) \
			--o:logging.file[@enable]=${OUTPUT_TO_FILE} --o:logging.level=trace \
			--o:logging_ui_cmd.file[@enable]=${OUTPUT_TO_FILE} \
			--o:trace_event[@enable]=true

if ENABLE_DEBUG
run-one: setup-wsd
	$(ENABLE_NAMESPACE) ./coolwsd $(COMMON_PARAMS) \
			--o:logging.file[@enable]=true --o:logging.level=trace \
			--o:logging_ui_cmd.file[@enable]=true \
			--singlekit

run-inproc: setup-wsd
	@echo "Launching coolwsd-inproc"
	./coolwsd-inproc $(COMMON_PARAMS) \
			--o:logging.file[@enable]=true --o:logging.level=trace \
			--o:logging_ui_cmd.file[@enable]=true

run-massif-inproc: setup-wsd
	@echo "Launching coolwsd under valgrind for single process"
	$(ENABLE_NAMESPACE) valgrind --tool=massif --num-callers=64 --trace-children=no -v \
		./coolwsd-inproc $(COMMON_PARAMS) \
			--o:logging.file[@enable]=false --o:logging.level=trace \
			--o:logging_ui_cmd.file[@enable]=false

run-heaptrack-inproc: setup-wsd
	@echo "Launching coolwsd under heaptrack for single process"
	$(ENABLE_NAMESPACE) heaptrack \
		./coolwsd-inproc $(COMMON_PARAMS) \
			--o:logging.file[@enable]=false --o:logging.level=trace \
			--o:logging_ui_cmd.file[@enable]=false
endif

sync-writer:
	browser-sync start --config browsersync-config.js --startPath "browser/96c23f663/debug.html?file_path=/$(abs_top_srcdir)/test/samples/writer-edit.fodt"

sync-calc:
	browser-sync start --config browsersync-config.js --startPath "browser/96c23f663/debug.html?file_path=/$(abs_top_srcdir)/test/samples/calc-edit.ods"

sync-impress:
	browser-sync start --config browsersync-config.js --startPath "browser/96c23f663/debug.html?file_path=/$(abs_top_srcdir)/test/samples/impress-edit.fodp"

run-trace: setup-wsd
	$(ENABLE_NAMESPACE) ./coolwsd $(COMMON_PARAMS) \
			--o:logging.file[@enable]=false --o:logging.level=trace \
			--o:logging_ui_cmd.file[@enable]=false \
			--o:trace[@enable]=true --o:trace.path=${builddir}/trace.txt.gz \
			--o:trace.outgoing.record=false

run-valgrind: setup-wsd
	@echo "Launching coolwsd under valgrind (but not forkit/coolkit, yet)"
	$(ENABLE_NAMESPACE) valgrind --tool=memcheck --trace-children=no -v --read-var-info=yes \
		./coolwsd $(COMMON_PARAMS) \
			--o:logging.file[@enable]=false --o:logging.level=trace \
			--o:logging_ui_cmd.file[@enable]=false

run-gdb: setup-wsd
	@echo "Launching coolwsd under gdb"
	$(ENABLE_NAMESPACE) $(RUN_GDB) \
		./coolwsd $(COMMON_PARAMS) \
			--o:logging.file[@enable]=false --o:logging.level=error \
			--o:logging_ui_cmd.file[@enable]=false

run-callgrind: setup-wsd
	@echo "Launching coolwsd under valgrind's callgrind"
	$(ENABLE_NAMESPACE) valgrind --tool=callgrind --simulate-cache=yes --dump-instr=yes --num-callers=50 --error-limit=no --trace-children=yes \
		./coolwsd $(COMMON_PARAMS) \
			--o:logging.file[@enable]=false --o:logging.level=error \
			--o:logging_ui_cmd.file[@enable]=false

run-strace: setup-wsd
	@echo "Launching coolwsd under strace"
	$(ENABLE_NAMESPACE) strace -o strace.log -f -tt -s 256 \
		./coolwsd $(COMMON_PARAMS) \
			--o:logging.file[@enable]=false --o:logging.level=error \
			--o:logging_ui_cmd.file[@enable]=false

run-tcmalloc: setup-wsd
	@echo "Launching coolwsd under tcmalloc"
	if ! ldconfig -p|grep -q libtcmalloc.so.4; then \
		echo Install your distro provided tcmalloc packages. Fedora: gperftools-libs, Debian: libgoogle-perftools4; \
		exit 1; \
	fi
	@echo see https://gperftools.github.io/gperftools/heapprofile.html for Modifying Runtime Behavior via env variables
	$(ENABLE_NAMESPACE) LD_PRELOAD=libtcmalloc.so.4 ./coolwsd $(COMMON_PARAMS) \
			--o:logging.file[@enable]=${OUTPUT_TO_FILE} --o:logging.level=trace \
			--o:logging_ui_cmd.file[@enable]=${OUTPUT_TO_FILE} \
			--o:trace_event[@enable]=true

else

SYSTEM_STAMP =

endif

if ENABLE_AFL
all-local: $(AFL_FUZZER_TARGETS)
clean-local: clean-afl
endif

.PHONY: cleanup setup-wsd clean-afl

if ENABLE_AFL
ALL_LOCAL_DEPS=$(AFL_FUZZER_TARGETS)
else
if ENABLE_LIBFUZZER
ALL_LOCAL_DEPS=
else
ALL_LOCAL_DEPS=coolwsd
endif
endif

all-local: $(ALL_LOCAL_DEPS) @JAILS_PATH@ @CACHE_PATH@ $(SYSTEM_STAMP)
	@$(abs_top_srcdir)/scripts/refresh-git-hooks

# just run the build without any tests
build-nocheck: all-am

endif

define file_target
$(1): $(2)

endef

define file_targets
$(foreach file,$(1),$(call file_target,$(file),$(patsubst %.cmd,%.cpp,$(file))))
endef

CLANGXX_COMPILE_FLAGS=clang++ $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) \
	$(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS)

JSON_COMPILE_FLAGS=$(subst ",\",$(subst \,\\,$(CLANGXX_COMPILE_FLAGS) $(coolstress_TDOC_CPPFLAGS)))

all_compiled_sources = $(sort \
	$(shared_sources) \
	$(coolwsd_sources) \
	$(coolforkit_sources) \
	$(coolwsd_SOURCES) \
	$(coolforkit_caps_SOURCES) \
	$(connect_SOURCES) \
	$(lokitclient_SOURCES) \
	$(coolstress_SOURCES) \
	$(coolconfig_SOURCES) \
	$(coolsocketdump_SOURCES) \
	$(coolbench_SOURCES) \
	$(coolconvert_SOURCES) \
	$(coolmount_SOURCES) \
	$(coolmap_SOURCES) \
	$(clientnb_SOURCES) \
	$(libsimd_a_SOURCES) \
	$(libglobals_a_SOURCES) \
	$(libkitglobals_a_SOURCES) \
	$(libwsdglobals_a_SOURCES) \
	$(libkitwsdglobals_a_SOURCES) \
)

JSON_COMPILE_SRC = $(patsubst %.cpp,%.cmd,$(filter %.cpp,$(all_compiled_sources)))

$(eval $(call file_targets,$(JSON_COMPILE_SRC)))

.cpp.cmd:
	@echo -n "{\"directory\":\"$(abs_builddir)\",\"command\":\"" > $@
	@echo -n $(JSON_COMPILE_FLAGS) >> $@
	@echo -n " -o $(patsubst %.cpp,%.o,$<) -c $<\",\"file\":\"$<\"}" >> $@

$(abs_srcdir)/compile_commands.json: $(JSON_COMPILE_SRC)
	@$(MAKE) -C test compile_commands
	@echo -n "[" > $@
	@for file in $(JSON_COMPILE_SRC) ; \
	do \
		cat $$file; \
		echo ","; \
		rm -f $$file; \
	done >> $@
	@for file in $$(find test -name '*.cmd' 2>/dev/null) ; \
	do \
		cat $$file; \
		echo ","; \
		rm -f $$file; \
	done >> $@
	@echo -n "]" >> $@

compile_commands: $(abs_srcdir)/compile_commands.json

$(abs_srcdir)/.clangd: config.d/clangd.am
	@sed -e 's|@COOL_DIR@|${abs_srcdir}|g' -e 's|@LOKIT_DIR@|${LOKIT_PATH}|g' $< > $@

dot_clangd: $(abs_srcdir)/.clangd

$(abs_srcdir)/Doxyfile.cpp.cfg: config.d/Doxyfile.cpp.in
	@sed -e 's|@COOL_DIR@|${abs_srcdir}|g' -e 's|@LOKIT_DIR@|${LOKIT_PATH}|g' -e 's|@COOL_VERSION@|${COOLWSD_VERSION}|g' $< > $@

doxygen: $(abs_srcdir)/Doxyfile.cpp.cfg
	rm -rf doc/cpp
	$(QUIET_MKDIRP) mkdir -p doc/cpp
	doxygen $(abs_srcdir)/Doxyfile.cpp.cfg

$(abs_srcdir)/.vscode/settings.json: config.d/vscode/settings.json.am
	$(QUIET_MKDIRP) mkdir -p $(abs_srcdir)/.vscode
	sed -e 's|@COOL_DIR@|${abs_srcdir}|g' -e 's|@LOKIT_DIR@|${LOKIT_PATH}|g' $< > $@

$(abs_srcdir)/.vscode/online.code-workspace: config.d/vscode/online.code-workspace.am
	$(QUIET_MKDIRP) mkdir -p $(abs_srcdir)/.vscode
	sed -e 's|@COOL_DIR@|${abs_srcdir}|g' -e 's|@LOKIT_DIR@|${LOKIT_PATH}|g' $< > $@

gen-clangd: $(abs_srcdir)/compile_commands.json $(abs_srcdir)/.clangd

gen-vscode: gen-clangd $(abs_srcdir)/.vscode/settings.json $(abs_srcdir)/.vscode/online.code-workspace

clean-clangd:
	cd $(abs_srcdir) && rm -f .clangd

clean-vscode: clean-clangd
	cd $(abs_srcdir) && rm -rf .vscode

browser/node_modules: browser/package.json browser/node_shrinkpack
	@cd browser && UV_USE_IO_URING=0 npm ci --offline

eslint: browser/node_modules
	@$(MAKE) -C browser eslint

prettier: browser/node_modules
	@$(MAKE) -C browser prettier

prettier-write: browser/node_modules
	@$(MAKE) -C browser prettier-write

install-exec-hook:
	cd $(DESTDIR)$(bindir) && \
	test -e coolconfig && $(LN_S) coolconfig loolconfig || true; \
	test -e coolwsd && $(LN_S) coolwsd loolwsd || true; \
	test -e coolwsd-systemplate-setup && $(LN_S) coolwsd-systemplate-setup loolwsd-systemplate-setup || true

stress_file=$(abs_top_srcdir)/test/data/hello-world.odt
trace_dir=$(abs_top_srcdir)/test/traces
stress:
	@cp $(abs_top_srcdir)/test/data/hello.odt $(stress_file)
	@echo "Watch at: https://localhost:9980/browser/12345/cool.html?file_path=$(stress_file)"
	./coolstress wss://localhost:9980 \
		$(stress_file) $(trace_dir)/writer-quick.txt \
		$(stress_file) $(trace_dir)/writer-add-bullet.txt \
		$(stress_file) $(trace_dir)/writer-mash-text-table.txt \
		$(stress_file) $(trace_dir)/writer-document-edit.txt \
		$(stress_file) $(trace_dir)/writer-mash-text-table.txt \
		$(stress_file) $(trace_dir)/writer-hello-shape.txt \
		$(stress_file) $(trace_dir)/writer-quick.txt

if ENABLE_CODE_COVERAGE
GIT_BRANCH := $(shell git symbolic-ref --short HEAD)
GIT_HASH := $(shell git log -1 --format=%h --abbrev=10)
GEN_COVERAGE_COMMAND=$(QUIET_MKDIRP) mkdir -p ${abs_top_srcdir}/gcov && \
lcov --no-external --capture --rc 'lcov_excl_line=' --rc 'lcov_excl_br_line=LOG_|TST_|LOK_|WSD_|TRANSITION|assert' \
--compat libtool=on --directory ${abs_top_srcdir}/. --output-file ${abs_top_srcdir}/gcov/cool.coverage.test.info && \
genhtml --prefix ${abs_top_srcdir}/. --ignore-errors source ${abs_top_srcdir}/gcov/cool.coverage.test.info \
--legend --title "${GIT_BRANCH} @ ${GIT_HASH}" --output-directory=${abs_top_srcdir}/gcov/html && \
echo "Code-Coverage report generated in ${abs_top_srcdir}/gcov/html"
else
GEN_COVERAGE_COMMAND=true
endif

# typically the system nss won't work in a jail and fail to init which is fatal for
# exporting to pdf, which is something that gets tested
check-for-system-nss:
	@if test -e "@LO_PATH@/../config_host.mk"; then \
		if grep -q SYSTEM_NSS=TRUE "@LO_PATH@/../config_host.mk"; then \
			echo make check will fail unless core is configured using --without-system-nss; \
			exit 1; \
		fi \
	fi

check-recursive: eslint

check: all-local check-for-system-nss prettier eslint presets-dir check-recursive
	$(GEN_COVERAGE_COMMAND)

coverage-report:
	$(GEN_COVERAGE_COMMAND)

coverage-clean:
	$(CLEANUP_COVERAGE)

# a) -D/usr/include, don't care about system headers
# b) -xProtocolHandlerInterface, this class poses a problem for pahole 1.29
# (https://github.com/acmel/dwarves/issues/38 might be relevant)
# c) Redirect stderr to /dev/null because there are a lot of dwarf parse
# warnings: 'template_parameter_pack__load_params: DW_TAG_template_value_parameter'
# possibly related to https://gcc.gnu.org/wiki/TemplateParmsDwarf
check-struct-packing:
	@echo Note: results only reliable with clang++ and CXXFLAGS=-fno-eliminate-unused-debug-types
	@if pahole -D/usr/include -x ProtocolHandlerInterface -P coolwsd 2> /dev/null | grep .; then \
		echo you can examine each candidate with pahole -S -R --class_name=CANDIDATE coolwsd; \
		exit 1; \
	fi
	@echo coolwsd struct packing is acceptably optimal

czech: all-local
	@echo "Running unit-tests only"
	$(MAKE) check -C test

collabora-online-sbom.spdx.json: coolforkit-ns
	$(QUIET_SBOM) python3 $(abs_top_srcdir)/scripts/create-sbom.py $(abs_top_builddir)
