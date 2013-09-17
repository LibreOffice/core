# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
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

$(eval $(call gb_Library_set_precompiled_header,sal,$(SRCDIR)/sal/inc/pch/precompiled_sal))

$(eval $(call gb_Library_set_include,sal,\
	$$(INCLUDE) \
	-I$(SRCDIR)/sal/inc \
))

$(eval $(call gb_Library_add_defs,sal,\
	$(if $(filter $(ALLOC),SYS_ALLOC TCMALLOC JEMALLOC), \
		-DFORCE_SYSALLOC \
	) \
	$(if $(filter $(OS),IOS), \
		-DNO_CHILD_PROCESSES \
	) \
	$(LFS_CFLAGS) \
	-DSAL_DLLIMPLEMENTATION \
	-DRTL_OS="\"$(RTL_OS)"\" \
	-DRTL_ARCH="\"$(RTL_ARCH)"\" \
	-DSRCDIR="\"$(SRCDIR)\"" \
))

$(eval $(call gb_Library_use_libraries,sal,\
	$(if $(filter $(OS),ANDROID), \
		lo-bootstrap \
	) \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_use_externals,sal,\
    valgrind \
    boost_headers \
))

$(eval $(call gb_Library_use_system_win32_libs,sal,\
	advapi32 \
	comdlg32 \
	mpr \
	ole32 \
	shell32 \
	user32 \
	ws2_32 \
))

$(eval $(call gb_Library_add_libs,sal,\
	$(if $(filter-out $(OS),WNT), \
		$(if $(filter $(OS),ANDROID),, \
			-lpthread \
		) \
	) \
	$(if $(filter $(OS),LINUX), \
		-ldl \
		-lrt \
	) \
	$(if $(filter $(OS),SOLARIS), \
		-lnsl \
		-lsocket \
	) \
))

ifeq ($(OS),MACOSX)
$(eval $(call gb_Library_use_system_darwin_frameworks,sal,\
	Carbon \
	CoreFoundation \
	Foundation \
))
endif

$(eval $(call gb_Library_add_exception_objects,sal,\
	sal/osl/all/compat \
	sal/osl/all/debugbase \
	sal/osl/all/loadmodulerelative \
	sal/osl/all/log  \
	sal/osl/all/trace  \
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
	sal/rtl/logfile \
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
$(eval $(call gb_Library_add_cobjects,sal,\
	sal/osl/all/filepath \
))

ifeq ($(OS),IOS)
$(eval $(call gb_Library_add_cxxflags,sal,\
    $(gb_OBJCXXFLAGS) \
))
endif

ifeq ($(OS),ANDROID)
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
	sal/osl/unx/conditn \
	sal/osl/unx/diagnose \
	sal/osl/unx/file \
	sal/osl/unx/file_error_transl \
	sal/osl/unx/file_misc \
	sal/osl/unx/file_path_helper \
	sal/osl/unx/file_stat \
	sal/osl/unx/file_url \
	sal/osl/unx/file_volume \
	sal/osl/unx/module \
	sal/osl/unx/process \
	sal/osl/unx/process_impl \
        $(if $(filter DESKTOP,$(BUILD_TYPE)), sal/osl/unx/salinit) \
))
$(eval $(call gb_Library_add_cobjects,sal,\
	sal/osl/unx/mutex \
	sal/osl/unx/nlsupport \
	sal/osl/unx/pipe \
	sal/osl/unx/profile \
	sal/osl/unx/readwrite_helper \
	sal/osl/unx/security \
	sal/osl/unx/socket \
	sal/osl/unx/system \
	sal/osl/unx/tempfile \
	sal/osl/unx/thread \
	sal/osl/unx/time \
	sal/osl/unx/util \
))
$(eval $(call gb_Library_add_cobject,sal,sal/osl/unx/signal, \
	$(if $(filter $(ENABLE_CRASHDUMP),YES)$(filter $(ENABLE_DBGUTIL),TRUE), \
		-DSAL_ENABLE_CRASH_REPORT) \
))

# Note that the uunxapi.mm file just includes the uunxapi.cxx one
ifeq ($(OS),MACOSX)
$(eval $(call gb_Library_add_objcxxobjects,sal,\
	sal/osl/unx/uunxapi \
))
else
$(eval $(call gb_Library_add_exception_objects,sal,\
	sal/osl/unx/uunxapi \
))
endif

ifneq ($(filter $(OS),MACOSX IOS),)
$(eval $(call gb_Library_add_exception_objects,sal,\
	sal/osl/unx/osxlocale \
))
endif
ifneq ($(filter $(OS),SOLARIS FREEBSD NETBSD MACOSX AIX OPENBSD DRAGONFLY)$(filter $(OS)$(CPUNAME),LINUXSPARC),)
$(eval $(call gb_Library_add_cobjects,sal,\
	sal/osl/unx/backtrace \
))
endif
ifneq ($(filter $(OS)$(CPU),SOLARISU SOLARISS NETBSDS LINUXS),)
$(eval $(call gb_Library_add_asmobjects,sal,\
	sal/osl/unx/asm/interlck_sparc \
))
else ifeq ($(OS)$(CPUNAME),SOLARISINTEL)
$(eval $(call gb_Library_add_asmobjects,sal,\
	sal/osl/unx/asm/interlck_x86 \
))
else
$(eval $(call gb_Library_add_cobjects,sal,\
	sal/osl/unx/interlck \
))
endif

else # $(OS) == WNT

# FIXME ?
# .IF "$(CCNUMVER)" >= "001400000000"
# $(eval $(call gb_Library_add_defs,sal,\
	-D_CRT_NON_CONFORMING_SWPRINTFS \
))
# .ENDIF

$(eval $(call gb_Library_add_exception_objects,sal,\
	sal/osl/w32/file \
	sal/osl/w32/file_dirvol \
	sal/osl/w32/file_url \
	sal/osl/w32/module \
	sal/osl/w32/path_helper \
	sal/osl/w32/process \
	sal/osl/w32/procimpl \
	sal/osl/w32/profile \
	sal/osl/w32/salinit \
	sal/osl/w32/signal \
	sal/osl/w32/socket \
	sal/osl/w32/tempfile \
))
$(eval $(call gb_Library_add_cobjects,sal,\
	sal/osl/w32/conditn \
	sal/osl/w32/diagnose \
	sal/osl/w32/dllentry \
	sal/osl/w32/file_error \
	sal/osl/w32/interlck \
	sal/osl/w32/mutex \
	sal/osl/w32/nlsupport \
	sal/osl/w32/pipe \
	sal/osl/w32/security \
	sal/osl/w32/thread \
	sal/osl/w32/time \
	sal/osl/w32/util \
))

endif # ifneq ($(OS),WNT)

# vim: set noet sw=4 ts=4:
