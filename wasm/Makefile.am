#
# Copyright the Collabora Online contributors.
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
QUIET_M4 = $(QUIET_M4_$(V))
QUIET_M4_ = $(QUIET_M4_$(AM_DEFAULT_VERBOSITY))
QUIET_M4_0 = @echo "  M4      " $@;

EXEEXT = .js

AM_CXXFLAGS = \
	-pthread -s USE_PTHREADS=1 \
	-fwasm-exceptions

AM_CPPFLAGS = \
	-I$(top_srcdir) \
	-I$(top_srcdir)/common \
	-I$(top_srcdir)/kit \
	-I$(top_srcdir)/net \
	-I$(top_srcdir)/wsd \
	-I@LOBUILDDIR@/workdir/UnpackedTarball/libpng \
	-I@LOBUILDDIR@/workdir/UnpackedTarball/zlib \
	-DMOBILEAPP=1

bin_PROGRAMS = online

online_SOURCES = \
	wasmapp.cpp \
	../common/Authorization.cpp \
	../common/ConfigUtil.cpp \
	../common/Crypto-stub.cpp \
	../common/FileUtil.cpp \
	../common/FileUtil-unix.cpp \
	../common/CommandControl.cpp \
	../common/Log-common.cpp \
	../common/Log-poco.cpp \
	../common/TraceEvent.cpp \
	../common/ProcUtil.cpp \
	../common/Protocol.cpp \
	../common/RegexUtil.cpp \
	../common/StringVector.cpp \
	../common/Session.cpp \
	../common/SigUtil-mobile.cpp \
	../common/Simd.cpp \
	../common/SpookyV2.cpp \
	../common/Unit.cpp \
	../common/Unit-mobile.cpp \
	../common/Uri.cpp \
	../common/Util.cpp \
	../common/Util-mobile.cpp \
	../common/Util-unix.cpp \
	../kit/ChildSession.cpp \
	../kit/Kit.cpp \
	../kit/KitQueue.cpp \
	../kit/LogUI.cpp \
	../kit/KitWebSocket.cpp \
	../kit/DeltaSimd.c \
	../net/FakeSocket.cpp \
	../net/Socket.cpp \
	../wsd/ClientSession.cpp \
	../wsd/DocumentBroker.cpp \
	../wsd/COOLWSD.cpp \
	../wsd/dumpWsdState.cpp \
	../wsd/ClientRequestDispatcher.cpp \
	../wsd/RequestDetails.cpp \
	../wsd/RequestVettingStation.cpp \
	../wsd/SlideCache.cpp \
	../wsd/Storage.cpp \
	../wsd/TileCache.cpp \
	../wsd/WSDGlobals.cpp \
	../wsd/coolwsd-fork.cpp

online_DEPENDENCIES = \
	exports \
	@LOBUILDDIR@/instdir/program/soffice.js.linkdeps \
	soffice.data \
	soffice.data.js.metadata \
	@LOBUILDDIR@/workdir/CustomTarget/static/emscripten_fs_image/soffice.data.js.link \
	@LOSOURCEDIR@/static/emscripten/environment.js

if ENABLE_WASM_EMBIND_UNO
online_DEPENDENCIES += \
	@LOBUILDDIR@/workdir/CustomTarget/static/unoembind/bindings_uno.js \
	@LOSOURCEDIR@/static/emscripten/uno.js
endif

if ENABLE_WASM_EMBINDTEST
online_DEPENDENCIES += @LOSOURCEDIR@/unotest/source/embindtest/embindtest.js
endif

# note cannot add content of .linkdeps to DEPENDENCIES because it contains -lFoo
online_LDADD = \
	${POCOLIB}/libPocoEncodings@POCODEBUG@.a \
	${POCOLIB}/libPocoNet@POCODEBUG@.a \
	${POCOLIB}/libPocoUtil@POCODEBUG@.a \
	${POCOLIB}/libPocoXML@POCODEBUG@.a \
	${POCOLIB}/libPocoJSON@POCODEBUG@.a \
	${POCOLIB}/libPocoFoundation@POCODEBUG@.a \
	${ZSTDLIB}/libzstd.a \
	-L@LOBUILDDIR@/instdir/program \
	$(shell cat @LOBUILDDIR@/instdir/program/soffice.js.linkdeps)

if ENABLE_WASM_EMBIND_UNO
online_LDADD += @LOBUILDDIR@/workdir/LinkTarget/StaticLibrary/libunoembind.a
endif

# TODO these are just copypasta from core
online_LDFLAGS = \
	-pthread -s MODULARIZE -s EXPORT_NAME=createOnlineModule -s USE_PTHREADS=1 -s TOTAL_MEMORY=1GB -s PTHREAD_POOL_SIZE_STRICT=0 --bind -s FORCE_FILESYSTEM=1 -s WASM_BIGINT=1 -s ERROR_ON_UNDEFINED_SYMBOLS=1 -s FETCH=1 -s ASSERTIONS=1 -s EXIT_RUNTIME=0 -s EXPORTED_RUNTIME_METHODS=["UTF16ToString","stringToUTF16","UTF8ToString","stringToNewUTF8","ccall","cwrap","FS","registerType","ClassHandle","HEAPU16","HEAPU32"] -pthread -s USE_PTHREADS=1 -fwasm-exceptions -s EXPORTED_FUNCTIONS=@exports \
	--pre-js @LOSOURCEDIR@/static/emscripten/environment.js \
	--pre-js @LOBUILDDIR@/workdir/CustomTarget/static/emscripten_fs_image/soffice.data.js.link

if ENABLE_WASM_EMBIND_UNO
online_LDFLAGS += \
	--post-js @LOBUILDDIR@/workdir/CustomTarget/static/unoembind/bindings_uno.js \
	--post-js @LOSOURCEDIR@/static/emscripten/uno.js
endif

if ENABLE_WASM_EMBINDTEST
online_LDFLAGS += --post-js @LOSOURCEDIR@/unotest/source/embindtest/embindtest.js
endif

# Silence "running limited binaryen optimizations because DWARF info requested (or indirectly
# required)":
online_LDFLAGS += $(if $(filter -g,$(CXXFLAGS)),-Wno-limited-postlink-optimizations)

all_local =

if ENABLE_DEBUG

AM_CXXFLAGS += -gsplit-dwarf
online_LDFLAGS += \
	-gseparate-dwarf -gsplit-dwarf -gpubnames

%.wasm.dwp: %.wasm
	emdwp -e $< -o $@

online.wasm.debug.wasm: | online$(EXEEXT)

all_local += online.wasm.debug.wasm.dwp

endif

if ENABLE_WASM_ADDITIONAL_FILES
online_LDFLAGS += \
	--preload-file $(WASM_ADDITIONAL_FILES)@
endif

exports: @LOBUILDDIR@/workdir/CustomTarget/desktop/soffice_bin-emscripten-exports/exports
	cp $< $@
	printf '_handle_cool_message\n' >>$@

soffice.dat%: @LOBUILDDIR@/instdir/program/soffice.dat%
	cp $< $@

emscripten-module.js: emscripten-module.js.m4
	$(QUIET_M4) m4 -PE -DENABLE_WASM_EMBINDTEST=$(ENABLE_WASM_EMBINDTEST) \
		-DENABLE_WASM_ZETAJS=$(ENABLE_WASM_ZETAJS) \
		-DENABLE_WASM_ZETAJS_EXAMPLE=$(ENABLE_WASM_ZETAJS_EXAMPLE) $< >$@

all_local += emscripten-module.js

all-local: $(all_local)

clean-local:
	rm -f \
		emscripten-module.js \
		online.wasm.debug.wasm.dwp
