#
# Copyright the Collabora Online contributors.
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# Source-list duplication mirrors the convention in test/Makefile.am: paths
# are sub-Makefile-relative (../common/Foo.cpp etc), so they don't share with
# the top-level shared_sources/coolwsd_sources/coolforkit_sources directly.
# When adding sources to those top-level lists, update the copies here too.

if ENABLE_MACOS
util_platform_cpp = ../common/Util-macos.cpp
else
util_platform_cpp = ../common/Util-linux.cpp
endif

shared_sources = \
                 $(util_platform_cpp) \
                 ../common/Authorization.cpp \
                 ../common/CommandControl.cpp \
                 ../common/ConfigUtil.cpp \
                 ../common/CoolMount.cpp \
                 ../common/FileUtil-unix.cpp \
                 ../common/FileUtil.cpp \
                 ../common/JailUtil.cpp \
                 ../common/Log-common.cpp \
                 ../common/Log-poco.cpp \
                 ../common/MobileApp.cpp \
                 ../common/ProcUtil.cpp \
                 ../common/Protocol.cpp \
                 ../common/RegexUtil.cpp \
                 ../common/Seccomp.cpp \
                 ../common/Session.cpp \
                 ../common/SigUtil-server.cpp \
                 ../common/Simd.cpp \
                 ../common/SpookyV2.cpp \
                 ../common/StringVector.cpp \
                 ../common/Syscall.cpp \
                 ../common/TraceEvent.cpp \
                 ../common/Unit-server.cpp \
                 ../common/Unit.cpp \
                 ../common/Uri.cpp \
                 ../common/Util-server.cpp \
                 ../common/Util-unix.cpp \
                 ../common/Util.cpp \
                 ../kit/KitQueue.cpp \
                 ../kit/LogUI.cpp \
                 ../net/HttpHelper.cpp \
                 ../net/HttpRequest.cpp \
                 ../net/NetUtil.cpp \
                 ../net/Socket.cpp \
                 ../wsd/Exceptions.cpp
if ENABLE_SSL
shared_sources += ../net/Ssl.cpp
endif

coolwsd_sources = \
                  ../common/Crypto.cpp \
                  ../wsd/Admin.cpp \
                  ../wsd/AdminModel.cpp \
                  ../wsd/AIChatSession.cpp \
                  ../wsd/Auth.cpp \
                  ../wsd/COOLWSD.cpp \
                  ../wsd/CacheUtil.cpp \
                  ../wsd/ClientRequestDispatcher.cpp \
                  ../wsd/ClientSession.cpp \
                  ../wsd/ContentType.cpp \
                  ../wsd/DocumentBroker.cpp \
                  ../wsd/FileServer.cpp \
                  ../wsd/FileServerUtil.cpp \
                  ../wsd/HostUtil.cpp \
                  ../wsd/ProofKey.cpp \
                  ../wsd/ProxyProtocol.cpp \
                  ../wsd/ProxyRequestHandler.cpp \
                  ../wsd/QuarantineUtil.cpp \
                  ../wsd/RemoteConfig.cpp \
                  ../wsd/RequestDetails.cpp \
                  ../wsd/RequestVettingStation.cpp \
                  ../wsd/ServerAuditUtil.cpp \
                  ../wsd/SlideCache.cpp \
                  ../wsd/SpecialBrokers.cpp \
                  ../wsd/Storage.cpp \
                  ../wsd/TileCache.cpp \
                  ../wsd/dumpWsdState.cpp \
                  ../wsd/wopi/CheckFileInfo.cpp \
                  ../wsd/wopi/StorageConnectionManager.cpp \
                  ../wsd/wopi/WopiProxy.cpp \
                  ../wsd/wopi/WopiStorage.cpp

coolforkit_sources = \
                     ../kit/ChildSession.cpp \
                     ../kit/ForKit.cpp \
                     ../kit/Kit.cpp \
                     ../kit/KitWebSocket.cpp

common_fuzzer_sources = \
                       Common.cpp \
                       ../common/DummyTraceEventEmitter.cpp \
                       $(coolwsd_sources) \
                       ../wsd/coolwsd-inproc.cpp \
                       ../wsd/KitWSDGlobals.cpp \
                       $(coolforkit_sources) \
                       $(shared_sources)

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
if !HAVE_CLANG
AM_LDFLAGS += -pthread
endif
if ENABLE_SSL
AM_LDFLAGS += ${OPENSSL_LIBS}
endif

if ENABLE_FUZZERS

noinst_PROGRAMS = \
                  admin_fuzzer \
                  clientrequest_fuzzer \
                  clientsession_fuzzer \
                  httpresponse_fuzzer \
                  httpecho_fuzzer

# CFI for the libFuzzer build (see comment near ENV_san for details).
LIBFUZZER_CFI_FLAGS = -fsanitize=cfi -fno-sanitize=cfi-derived-cast,cfi-unrelated-cast \
                      -flto -fvisibility=hidden -fno-sanitize-trap=cfi
LIBFUZZER_CXXFLAGS_COMMON = $(AM_CXXFLAGS) $(LIBFUZZER_CFI_FLAGS)
LIBFUZZER_LDFLAGS_COMMON = -fsanitize=fuzzer @FUZZER_LDFLAGS@ $(LIBFUZZER_CFI_FLAGS)

admin_fuzzer_CPPFLAGS = \
			$(AM_CPPFLAGS)
admin_fuzzer_CXXFLAGS = $(LIBFUZZER_CXXFLAGS_COMMON)
admin_fuzzer_SOURCES = \
		       $(common_fuzzer_sources) \
		       Admin.cpp
admin_fuzzer_LDFLAGS = $(LIBFUZZER_LDFLAGS_COMMON) $(AM_LDFLAGS)
admin_fuzzer_LDADD = ../libsimd.a

clientrequest_fuzzer_CPPFLAGS = \
				$(AM_CPPFLAGS)
clientrequest_fuzzer_CXXFLAGS = $(LIBFUZZER_CXXFLAGS_COMMON)
clientrequest_fuzzer_SOURCES = \
			       $(common_fuzzer_sources) \
			       ClientRequestDispatcherFuzz.cpp
clientrequest_fuzzer_LDFLAGS = $(LIBFUZZER_LDFLAGS_COMMON) $(AM_LDFLAGS)
clientrequest_fuzzer_LDADD = ../libsimd.a

clientsession_fuzzer_CPPFLAGS = \
				$(AM_CPPFLAGS)
clientsession_fuzzer_CXXFLAGS = $(LIBFUZZER_CXXFLAGS_COMMON)
clientsession_fuzzer_SOURCES = \
			       $(common_fuzzer_sources) \
			       ClientSession.cpp
clientsession_fuzzer_LDFLAGS = $(LIBFUZZER_LDFLAGS_COMMON) $(AM_LDFLAGS)
clientsession_fuzzer_LDADD = ../libsimd.a

httpresponse_fuzzer_CPPFLAGS = \
				$(AM_CPPFLAGS)
httpresponse_fuzzer_CXXFLAGS = $(LIBFUZZER_CXXFLAGS_COMMON)
httpresponse_fuzzer_SOURCES = \
			       $(common_fuzzer_sources) \
			       HttpResponse.cpp
httpresponse_fuzzer_LDFLAGS = $(LIBFUZZER_LDFLAGS_COMMON) $(AM_LDFLAGS)
httpresponse_fuzzer_LDADD = ../libsimd.a

httpecho_fuzzer_CPPFLAGS = \
				$(AM_CPPFLAGS) \
        -I${top_srcdir}/test
httpecho_fuzzer_CXXFLAGS = $(LIBFUZZER_CXXFLAGS_COMMON)
httpecho_fuzzer_SOURCES = \
			       $(common_fuzzer_sources) \
			       HttpEcho.cpp
httpecho_fuzzer_LDFLAGS = $(LIBFUZZER_LDFLAGS_COMMON) $(AM_LDFLAGS)
httpecho_fuzzer_LDADD = ../libsimd.a

if ENABLE_AFL
# AFL++ multi-variant fuzzer build.
# 4 static libraries (one per variant) from common sources,
# then 5 fuzzers x 4 variants. To add a new fuzzer, just add to AFL_FUZZERS.

AFL_CPPFLAGS = $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) -I${top_srcdir}/test
# AFL variants use afl-clang-* instrumentation; strip flags incompatible with
# the AFL toolchain that may have leaked into CXXFLAGS:
#  -fsanitize=fuzzer-no-link: AFL links libAFLDriver.a, not libclang_rt.fuzzer.a,
#     so SanitizerCoverage refs like __sancov_lowest_stack would be unresolved.
#  CFI flags (-fsanitize=cfi, -fno-sanitize-trap=cfi, -fsanitize-recover=cfi,
#     -fvisibility=hidden): CFI is intentionally off for AFL (afl-clang-fast's
#     instrumentation pass false-positives under CFI metadata).
AFL_FILTER_OUT = -fsanitize=fuzzer-no-link \
                 -fsanitize=cfi \
                 -fno-sanitize-trap=cfi \
                 -fsanitize-recover=cfi \
                 -fvisibility=hidden
AFL_CXXFLAGS = $(AFL_CPPFLAGS) $(AM_CXXFLAGS) $(filter-out $(AFL_FILTER_OUT),$(CXXFLAGS))
AFL_CFLAGS = $(AFL_CPPFLAGS) $(AM_CFLAGS) $(filter-out $(AFL_FILTER_OUT),$(CFLAGS))
# @FUZZER_LDFLAGS@ carries -flto -fuse-ld=lld -Wl,--no-gc-sections, needed
# because afl-clang-fast emits bitcode .o under SAND mode. It comes from
# configure (validated there) and is applied here, not in global LDFLAGS,
# so test/ binaries built alongside the fuzzers don't pay the LTO link cost.
AFL_LDFLAGS_COMMON = $(AM_LDFLAGS) $(LIBS) @FUZZER_LDFLAGS@

# Variant configurations: ENV_<variant>, CXX_<variant>, CC_<variant>
ENV_native =
CXX_native = $(AFL_CXX)
CC_native = $(AFL_CC)

# CFI is enabled on the san variant for parity with the libFuzzer build, so
# CFI findings can be compared between the two engines on the same input.
# AFL_USE_CFISAN=1 makes afl-clang-fast add -fsanitize=cfi -flto
# -fvisibility=hidden; we layer CXXFLAGS_san on top to:
#  - subtract cfi-derived-cast and cfi-unrelated-cast (false positives on
#    libstdc++'s _Sp_counted_ptr_inplace::_M_ptr() static_cast;
#  - swap the silent ud1 trap for a runtime diagnostic that prints the
#    offending type before aborting (-fno-sanitize-trap=cfi).
ENV_san = AFL_LLVM_ONLY_FSRV=1 AFL_USE_ASAN=1 AFL_USE_UBSAN=1 AFL_USE_CFISAN=1
CXX_san = $(AFL_CLANG_FAST_CXX)
CC_san = $(AFL_CLANG_FAST_CC)
CXXFLAGS_san = -fno-sanitize=cfi-derived-cast,cfi-unrelated-cast \
               -fno-sanitize-trap=cfi

ENV_cmpcov = AFL_LLVM_LAF_ALL=1
CXX_cmpcov = $(AFL_CXX)
CC_cmpcov = $(AFL_CC)

ENV_cmplog = AFL_LLVM_CMPLOG=1
CXX_cmplog = $(AFL_CXX)
CC_cmplog = $(AFL_CC)

AFL_VARIANTS = native san cmpcov cmplog

# Common sources split by language
common_fuzzer_cpp = $(common_fuzzer_sources)
common_fuzzer_c = ../kit/DeltaSimd.c

# Fuzzer registry: name=local-source-file (add new fuzzers here)
AFL_FUZZERS = \
	admin=Admin.cpp \
	clientrequest=ClientRequestDispatcherFuzz.cpp \
	clientsession=ClientSession.cpp \
	httpresponse=HttpResponse.cpp \
	httpecho=HttpEcho.cpp

# Extract just the names
AFL_FUZZER_NAMES = $(foreach f,$(AFL_FUZZERS),$(word 1,$(subst =, ,$f)))

# Generate all target names: <name>_fuzzer_<variant>.afl
AFL_FUZZER_TARGETS = $(foreach v,$(AFL_VARIANTS),$(foreach n,$(AFL_FUZZER_NAMES),$(n)_fuzzer_$(v).afl))

all-local: $(AFL_FUZZER_TARGETS)

clean-fuzzers:
	rm -rf $(foreach v,$(AFL_VARIANTS),.afl-$(v)) $(AFL_LIBS) $(AFL_FUZZER_TARGETS)

# Per-variant common object files.
# Cross-dir sources (../X/Y.cpp) -> .afl-VARIANT/X/Y.o
# Local sources (Y.cpp)          -> .afl-VARIANT/fuzzer/Y.o
afl_common_objs = \
	$(patsubst ../%,.afl-$(1)/%,$(filter ../%,$(common_fuzzer_cpp:.cpp=.o))) \
	$(addprefix .afl-$(1)/fuzzer/,$(filter-out ../%,$(common_fuzzer_cpp:.cpp=.o))) \
	$(patsubst ../%,.afl-$(1)/%,$(filter ../%,$(common_fuzzer_c:.c=.o))) \
	$(addprefix .afl-$(1)/fuzzer/,$(filter-out ../%,$(common_fuzzer_c:.c=.o)))

# Per-variant static library names
AFL_LIBS = $(foreach v,$(AFL_VARIANTS),aflfuzz_$(v).a)

# Pattern rules for compiling each variant's objects.
# Cross-dir prereq: ../X/Y.cpp -> .afl-VARIANT/X/Y.o
# Local prereq:      Y.cpp     -> .afl-VARIANT/fuzzer/Y.o
define AFL_VARIANT_RULES
.afl-$(1)/%.o: ../%.cpp
	@mkdir -p $$(dir $$@)
	$(ENV_$(1)) $$(CXX_$(1)) $$(AFL_CXXFLAGS) $$(CXXFLAGS_$(1)) -c -o $$@ $$<

.afl-$(1)/fuzzer/%.o: %.cpp
	@mkdir -p $$(dir $$@)
	$(ENV_$(1)) $$(CXX_$(1)) $$(AFL_CXXFLAGS) $$(CXXFLAGS_$(1)) -c -o $$@ $$<

.afl-$(1)/%.o: ../%.c
	@mkdir -p $$(dir $$@)
	$(ENV_$(1)) $$(CC_$(1)) $$(AFL_CFLAGS) @SIMD_CFLAGS@ -c -o $$@ $$<

aflfuzz_$(1).a: $$(call afl_common_objs,$(1))
	$$(AR) crs $$@ $$^
endef

$(foreach v,$(AFL_VARIANTS),$(eval $(call AFL_VARIANT_RULES,$(v))))

# Link rule: <name>_fuzzer_<variant>.afl from variant-compiled fuzzer source + static lib
# Fuzzer harness sources (Admin.cpp etc.) are local to fuzzer/, so .afl-VARIANT/fuzzer/X.o
define AFL_FUZZER_LINK
$(1)_fuzzer_$(2).afl: .afl-$(2)/fuzzer/$(3:.cpp=.o) aflfuzz_$(2).a
	$(ENV_$(2)) $$(CXX_$(2)) -o $$@ $$< aflfuzz_$(2).a $$(AFL_DRIVER_LIB) $$(AFL_LDFLAGS_COMMON)
endef

$(foreach f,$(AFL_FUZZERS),\
  $(foreach v,$(AFL_VARIANTS),\
    $(eval $(call AFL_FUZZER_LINK,$(word 1,$(subst =, ,$f)),$(v),$(word 2,$(subst =, ,$f))))))

else # !ENABLE_AFL (libFuzzer-only build)

clean-fuzzers: ;

endif # !ENABLE_AFL

else # !ENABLE_FUZZERS

clean-fuzzers: ;

endif # !ENABLE_FUZZERS

clean-local: clean-fuzzers
