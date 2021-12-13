# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,sal))

$(eval $(call gb_Library_set_soversion_script,sal,$(SRCDIR)/sal/util/sal.map))

$(eval $(call gb_Library_set_precompiled_header,sal,sal/inc/pch/precompiled_sal))

$(eval $(call gb_Library_set_is_ure_library_or_dependency,sal))

$(eval $(call gb_Library_set_include,sal,\
	$$(INCLUDE) \
	-I$(SRCDIR)/sal/inc \
))

$(eval $(call gb_Library_add_defs,sal,\
	$(if $(filter FUZZERS,$(BUILD_TYPE)), \
		-DFORCE_DEFAULT_SIGNAL \
	) \
	$(if $(filter iOS,$(OS)), \
		-DNO_CHILD_PROCESSES \
	) \
	-DSAL_DLLIMPLEMENTATION \
	-DRTL_OS="\"$(RTL_OS)\"" \
	-DRTL_ARCH="\"$(RTL_ARCH)\"" \
	-DSRCDIR="\"$(SRCDIR)\"" \
))

$(eval $(call gb_Library_use_libraries,sal,\
	$(if $(filter ANDROID,$(OS)), \
		lo-bootstrap \
	) \
))

$(eval $(call gb_Library_use_externals,sal,\
    dtoa \
    valgrind \
    zlib \
))

$(eval $(call gb_Library_use_system_win32_libs,sal,\
	advapi32 \
	comdlg32 \
	dbghelp \
	mpr \
	ole32 \
	shell32 \
	user32 \
	userenv \
	wer \
	ws2_32 \
))

$(eval $(call gb_Library_add_libs,sal,\
	$(if $(filter LINUX,$(OS)), \
		-ldl \
		-lrt \
	) \
	$(if $(filter SOLARIS,$(OS)), \
		-lnsl \
		-lsocket \
	) \
	$(if $(filter HAIKU,$(OS)), \
		-lnetwork \
	) \
))

ifeq ($(OS),MACOSX)
$(eval $(call gb_Library_use_system_darwin_frameworks,sal,\
	Carbon \
	CoreFoundation \
	Foundation \
	$(if $(ENABLE_MACOSX_SANDBOX),Security) \
))
endif

$(eval $(call gb_Library_add_exception_objects,sal,\
	sal/osl/all/compat \
	sal/osl/all/debugbase \
	sal/osl/all/filepath \
	sal/osl/all/loadmodulerelative \
	sal/osl/all/log  \
	sal/osl/all/mutexshared \
	sal/osl/all/signalshared  \
	sal/osl/all/utility \
	sal/rtl/alloc_arena \
	sal/rtl/alloc_cache \
	sal/rtl/alloc_fini \
	sal/rtl/alloc_global \
	sal/rtl/bootstrap \
	sal/rtl/byteseq \
	sal/rtl/cipher \
	sal/rtl/cmdargs \
	sal/rtl/crc \
	sal/rtl/digest \
	sal/rtl/hash \
	sal/rtl/locale \
	sal/rtl/math \
	sal/rtl/random \
	sal/rtl/rtl_process \
	sal/rtl/strbuf \
	sal/rtl/strimp \
	sal/rtl/string \
	sal/rtl/unload \
	sal/rtl/uri \
	sal/rtl/ustrbuf \
	sal/rtl/ustring \
	sal/rtl/uuid \
	sal/textenc/converter \
	sal/textenc/convertsimple \
    sal/textenc/handleundefinedunicodetotextchar \
	sal/textenc/tcvtutf8 \
	sal/textenc/tencinfo \
	sal/textenc/textcvt \
	sal/textenc/textenc \
	sal/textenc/unichars \
))

ifeq ($(OS),iOS)
$(eval $(call gb_Library_add_cxxflags,sal,\
    $(gb_OBJCXXFLAGS) \
))
endif

ifeq (,$(call gb_CondSalTextEncodingLibrary,$(true)))
$(eval $(call gb_Library_add_exception_objects,sal,\
    sal/textenc/context \
    sal/textenc/convertbig5hkscs \
    sal/textenc/converteuctw \
    sal/textenc/convertgb18030 \
    sal/textenc/convertisciidevangari \
    sal/textenc/convertiso2022cn \
    sal/textenc/convertiso2022jp \
    sal/textenc/convertiso2022kr \
    sal/textenc/convertsinglebytetobmpunicode \
    sal/textenc/tables \
    sal/textenc/tcvtbyte \
    sal/textenc/tcvtmb \
    sal/textenc/tcvtutf7 \
))
endif

ifneq ($(OS),WNT)
$(eval $(call gb_Library_add_exception_objects,sal,\
	sal/osl/unx/backtraceapi \
	sal/osl/unx/conditn \
	sal/osl/unx/file \
	sal/osl/unx/file_error_transl \
	sal/osl/unx/file_misc \
	sal/osl/unx/file_path_helper \
	sal/osl/unx/file_stat \
	sal/osl/unx/file_url \
	sal/osl/unx/file_volume \
	sal/osl/unx/memory \
	sal/osl/unx/module \
	sal/osl/unx/mutex \
	sal/osl/unx/nlsupport \
	sal/osl/unx/pipe \
	sal/osl/unx/process \
	sal/osl/unx/process_impl \
	sal/osl/unx/profile \
	sal/osl/unx/random \
	sal/osl/unx/readwrite_helper \
	sal/osl/unx/salinit \
	sal/osl/unx/security \
	sal/osl/unx/signal \
	sal/osl/unx/socket \
	sal/osl/unx/soffice \
	sal/osl/unx/tempfile \
	sal/osl/unx/thread \
	sal/osl/unx/time \
))

# Note that the uunxapi.mm file just includes the uunxapi.cxx one. Ditto for system.mm
ifeq ($(OS),MACOSX)
$(eval $(call gb_Library_add_objcxxobjects,sal,\
	sal/osl/unx/uunxapi \
	sal/osl/unx/system \
))
else
$(eval $(call gb_Library_add_exception_objects,sal,\
	sal/osl/unx/uunxapi \
	sal/osl/unx/system \
))
endif

ifneq ($(filter MACOSX iOS,$(OS)),)
$(eval $(call gb_Library_add_exception_objects,sal,\
	sal/osl/unx/osxlocale \
))
endif
ifneq ($(OS),WNT)
$(eval $(call gb_Library_add_cobjects,sal,\
	sal/osl/unx/backtrace \
))
endif
ifneq ($(filter SPARC64 SPARC,$(CPUNAME)),)
$(eval $(call gb_Library_add_asmobjects,sal,\
	sal/osl/unx/asm/interlck_sparc \
))
else
$(eval $(call gb_Library_add_exception_objects,sal,\
	sal/osl/unx/interlck \
))
endif

else # $(OS) == WNT

$(eval $(call gb_Library_add_exception_objects,sal,\
	sal/osl/w32/backtrace \
	sal/osl/w32/conditn \
	sal/osl/w32/dllentry \
	sal/osl/w32/file \
	sal/osl/w32/file_dirvol \
	sal/osl/w32/file_error \
	sal/osl/w32/file_url \
	sal/osl/w32/interlck \
	sal/osl/w32/memory \
	sal/osl/w32/module \
	sal/osl/w32/mutex \
	sal/osl/w32/nlsupport \
	sal/osl/w32/path_helper \
	sal/osl/w32/pipe \
	sal/osl/w32/process \
	sal/osl/w32/procimpl \
	sal/osl/w32/profile \
	sal/osl/w32/random \
	sal/osl/w32/salinit \
	sal/osl/w32/security \
	sal/osl/w32/signal \
	sal/osl/w32/socket \
	sal/osl/w32/tempfile \
	sal/osl/w32/thread \
	sal/osl/w32/time \
))

endif # ifneq ($(OS),WNT)

ifeq ($(ENABLE_CIPHER_OPENSSL_BACKEND),TRUE)
$(eval $(call gb_Library_add_defs,sal,-DLIBO_CIPHER_OPENSSL_BACKEND))
$(eval $(call gb_Library_use_externals,sal, \
    openssl \
    openssl_headers \
))
endif

# vim: set noet sw=4 ts=4:
