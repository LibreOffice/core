# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# Major Contributor(s):
# Copyright (C) 2011 Matúš Kukan <matus.kukan@gmail.com> (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_Library_Library,sal))

$(eval $(call gb_Library_set_soversion_script,sal,3,$(SRCDIR)/sal/util/sal.map))

$(eval $(call gb_Library_use_packages,sal,\
	sal_generated \
	sal_inc \
))

$(eval $(call gb_Library_set_include,sal,\
	$$(INCLUDE) \
	-I$(SRCDIR)/sal/inc \
))

$(eval $(call gb_Library_add_defs,sal,\
	$(if $(VALGRIND_CFLAGS), \
		$(VALGRIND_CFLAGS) \
		-DHAVE_MEMCHECK_H=1 \
	) \
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
))

$(eval $(call gb_Library_use_libraries,sal,\
	$(if $(filter $(GUI),UNX), \
		$(if $(filter $(OS),ANDROID),, \
			pthread \
		) \
	) \
	$(if $(filter $(OS),LINUX), \
		dl \
	) \
	$(if $(filter $(OS),SOLARIS), \
		nsl \
		socket \
	) \
	$(if $(filter $(OS),WNT), \
		advapi32 \
		comdlg32 \
		mpr \
		ole32 \
		shell32 \
		user32 \
		ws2_32 \
	) \
	$(if $(filter $(OS),ANDROID), \
		lo-bootstrap \
	) \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_standard_system_libs,sal))

ifeq ($(OS),MACOSX)
$(eval $(call gb_Library_use_externals,sal,\
	carbon \
	corefoundation \
))
endif

$(eval $(call gb_Library_add_exception_objects,sal,\
	sal/osl/all/debugbase \
	sal/osl/all/loadmodulerelative \
	sal/osl/all/log  \
	sal/osl/all/trace  \
	sal/osl/all/utility \
	sal/rtl/source/alloc_arena \
	sal/rtl/source/alloc_cache \
	sal/rtl/source/alloc_fini \
	sal/rtl/source/alloc_global \
	sal/rtl/source/bootstrap \
	sal/rtl/source/byteseq \
	sal/rtl/source/cipher \
	sal/rtl/source/cmdargs \
	sal/rtl/source/crc \
	sal/rtl/source/digest \
	sal/rtl/source/hash \
	sal/rtl/source/locale \
	sal/rtl/source/logfile \
	sal/rtl/source/math \
	sal/rtl/source/memory \
	sal/rtl/source/random \
	sal/rtl/source/rtl_process \
	sal/rtl/source/strbuf \
	sal/rtl/source/strimp \
	sal/rtl/source/string \
	sal/rtl/source/unload \
	sal/rtl/source/uri \
	sal/rtl/source/ustrbuf \
	sal/rtl/source/ustring \
	sal/rtl/source/uuid \
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

ifeq ($(GUI),UNX)
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
	sal/osl/unx/salinit \
	sal/osl/unx/uunxapi \
))
$(eval $(call gb_Library_add_cobjects,sal,\
	sal/osl/unx/mutex \
	sal/osl/unx/nlsupport \
	sal/osl/unx/pipe \
	sal/osl/unx/profile \
	sal/osl/unx/readwrite_helper \
	sal/osl/unx/security \
	sal/osl/unx/semaphor \
	sal/osl/unx/socket \
	sal/osl/unx/system \
	sal/osl/unx/tempfile \
	sal/osl/unx/thread \
	sal/osl/unx/time \
	sal/osl/unx/util \
))
$(eval $(call gb_Library_add_cobject,sal,sal/osl/unx/signal, \
	$(if $(filter $(ENABLE_CRASHDUMP),YES)$(if $(PRODUCT),,TRUE), \
		-DSAL_ENABLE_CRASH_REPORT) \
))

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

else # $(GUI) != UNX

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
	sal/osl/w32/semaphor \
	sal/osl/w32/thread \
	sal/osl/w32/time \
	sal/osl/w32/util \
))

endif # ifeq ($(GUI),UNX)

# vim: set noet sw=4 ts=4:
