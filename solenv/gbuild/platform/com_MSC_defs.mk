# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2011 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

# set tmpdir to some mixed case path, suitable for native tools
gb_TMPDIR:=$(if $(TMPDIR),$(shell cygpath -m $(TMPDIR)),$(shell cygpath -m /tmp))

# please make generic Windows modifications to windows.mk
include $(GBUILDDIR)/platform/windows.mk

gb_CC := cl
gb_CXX := cl
gb_LINK := link
gb_AWK := awk
gb_CLASSPATHSEP := ;
gb_RC := rc
gb_YACC := bison

# use CC/CXX if they are nondefaults
ifneq ($(origin CC),default)
gb_CC := $(CC)
gb_GCCP := $(CC)
endif
ifneq ($(origin CXX),default)
gb_CXX := $(CXX)
endif

gb_COMPILERDEFS := \
	-DMSC \
	-D_CRT_NON_CONFORMING_SWPRINTFS \
	-D_CRT_NONSTDC_NO_DEPRECATE \
	-D_CRT_SECURE_NO_DEPRECATE \
	-D_MT \
	-D_DLL \
	-DBOOST_MEM_FN_ENABLE_CDECL \
	-DCPPU_ENV=$(gb_CPPU_ENV) \
	-DM1500 \

gb_RCDEFS := \
	 -DWINVER=0x0400 \
	 -DWIN32 \

gb_RCFLAGS := \
	 -V

gb_AFLAGS := /c /Cp

gb_CFLAGS := \
	-Gd \
	-GR \
	-Gs \
	-GS \
	-MD \
	-nologo \
	-Wall \
	-wd4005 \
	-wd4061 \
	-wd4127 \
	-wd4180 \
	-wd4189 \
	-wd4191 \
	-wd4217 \
	-wd4242 \
	-wd4244 \
	-wd4245 \
	-wd4250 \
	-wd4251 \
	-wd4255 \
	-wd4275 \
	-wd4290 \
	-wd4294 \
	-wd4350 \
	-wd4355 \
	-wd4365 \
	-wd4503 \
	-wd4505 \
	-wd4511 \
	-wd4512 \
	-wd4514 \
	-wd4555 \
	-wd4611 \
	-wd4619 \
	-wd4625 \
	-wd4626 \
	-wd4640 \
	-wd4668 \
	-wd4675 \
	-wd4686 \
	-wd4692 \
	-wd4706 \
	-wd4710 \
	-wd4711 \
	-wd4373 \
	-wd4738 \
	-wd4786 \
	-wd4800 \
	-wd4820 \
	-wd4826 \
	-wd4917 \
	-Zc:forScope,wchar_t- \
	-Zm500 \

gb_CXXFLAGS := \
	-Gd \
	-GR \
	-Gs \
	-GS \
	-Gy \
	-MD \
	-nologo \
	-Wall \
	-wd4005 \
	-wd4061 \
	-wd4127 \
	-wd4180 \
	-wd4189 \
	-wd4191 \
	-wd4217 \
	-wd4242 \
	-wd4244 \
	-wd4245 \
	-wd4250 \
	-wd4251 \
	-wd4265 \
	-wd4275 \
	-wd4290 \
	-wd4294 \
	-wd4350 \
	-wd4351 \
	-wd4355 \
	-wd4365 \
	-wd4435 \
	-wd4503 \
	-wd4505 \
	-wd4511 \
	-wd4512 \
	-wd4514 \
	-wd4555 \
	-wd4611 \
	-wd4619 \
	-wd4625 \
	-wd4626 \
	-wd4628 \
	-wd4640 \
	-wd4668 \
	-wd4675 \
	-wd4686 \
	-wd4692 \
	-wd4706 \
	-wd4710 \
	-wd4711 \
	-wd4373 \
	-wd4738 \
	-wd4786 \
	-wd4800 \
	-wd4820 \
	-wd4826 \
	-wd4917 \
	-wd4996 \
	-Zc:forScope,wchar_t- \
	-Zm500 \

gb_STDLIBS := \
	advapi32.lib \

ifneq ($(EXTERNAL_WARNINGS_NOT_ERRORS),TRUE)
gb_CFLAGS_WERROR := -WX -DLIBO_WERROR
gb_CXXFLAGS_WERROR := -WX -DLIBO_WERROR
endif

ifeq ($(MERGELIBS),TRUE)
gb_CFLAGS += -DLIBO_MERGELIBS
gb_CXXFLAGS += -DLIBO_MERGELIBS
endif

gb_LinkTarget_EXCEPTIONFLAGS := \
	-DEXCEPTIONS_ON \
	-EHa \

gb_LinkTarget_NOEXCEPTIONFLAGS := \
	-DEXCEPTIONS_OFF \

gb_LinkTarget_LDFLAGS := \
	$(if $(findstring s,$(filter-out --%,$(MAKEFLAGS))),-nologo,) \
	-MANIFEST \
	$(patsubst %,-LIBPATH:%,$(filter-out .,$(subst ;, ,$(subst \,/,$(ILIB))))) \

gb_DEBUG_CFLAGS := -Zi

# this does not use CFLAGS so it is not overridable
ifneq ($(ENABLE_CRASHDUMP),)
gb_CFLAGS+=-Zi
gb_CXXFLAGS+=-Zi
endif

ifeq ($(gb_SYMBOL),$(true))
gb_CFLAGS+=$(gb_DEBUG_CFLAGS)
gb_CXXFLAGS+=$(gb_DEBUG_CFLAGS)
endif

gb_COMPILEROPTFLAGS := -Ob1 -Oxs -Oy-
gb_COMPILERNOOPTFLAGS := -Od

ifeq ($(gb_FULLDEPS),$(true))
gb_COMPILERDEPFLAGS := -showIncludes
define gb_create_deps
| $(GBUILDDIR)/platform/filter-showIncludes.awk -vdepfile=$(1) -vobjectfile=$(2) -vsourcefile=$(3); exit $${PIPESTATUS[0]}
endef
else
gb_COMPILERDEPFLAGS :=
define gb_create_deps
endef
endif

gb_COMPILER_LTOFLAGS := $(if $(filter TRUE,$(ENABLE_LTO)),-GL)

# Helper class

gb_Helper_OUTDIRLIBDIR := $(OUTDIR)/bin
gb_Helper_OUTDIR_FOR_BUILDLIBDIR := $(OUTDIR_FOR_BUILD)/bin

gb_Helper_set_ld_path := PATH="$${PATH}:$(shell cygpath -u $(OUTDIR)/bin)"

# vim: set noet sw=4:
