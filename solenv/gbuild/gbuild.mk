#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2009 by Sun Microsystems, Inc.
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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.	If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

# vars needed from the env/calling makefile

# CVER
# DEBUG
# GBUILDDIR
# INPATH
# JAVA_HOME
# JDKINCS
# LIBXML_CFLAGS
# OS
# PRODUCT
# SOLARINC
# SOLARLIB
# STLPORT_VER
# UPD

# GXX_INCLUDE_PATH (Linux)
# PTHREAD_CFLAGS (Linux)
# SYSTEM_ICU (Linux)
# SYSTEM_JPEG (Linux)
# SYSTEM_LIBXML (Linux)
# USE_SYSTEM_STL (Linux)

SHELL := /bin/sh

ifeq ($(strip $(SOLARSRC)),)
$(error No environment set)
endif

# extend for JDK include (seems only needed in setsolar env?)
SOLARINC += $(JDKINCS)

SRCDIR := $(SOLARSRC)
OUTDIR := $(SOLARVERSION)/$(INPATH)
WORKDIR := $(SOLARVERSION)/$(INPATH)/workdir

# HACK
ifeq ($(OS),WNT)
SRCDIR := $(shell cygpath -u $(SRCDIR))
WORKDIR := $(shell cygpath -u $(WORKDIR))
OUTDIR := $(shell cygpath -u $(OUTDIR))
endif
true := T
false :=

ifneq ($(strip $(PRODUCT)),)
gb_PRODUCT := $(true)
else
gb_PRODUCT := $(false)
endif

ifneq ($(strip $(DEBUG)),)
gb_DEBUGLEVEL := 2
else
ifeq ($(gb_PRODUCT),$(true))
gb_DEBUGLEVEL := 0
else
gb_DEBUGLEVEL := 1
endif
endif

gb_FULLDEPS := $(true)
#gb_FULLDEPS := $(false)
ifeq ($(MAKECMDGOALS),clean)
gb_FULLDEPS := $(false)
endif

include $(GBUILDDIR)/helper.mk
include $(GBUILDDIR)/libnames.mk
gb_Library_TARGETS := $(foreach namescheme,$(gb_Library_NAMESCHEMES),$(gb_Library_$(namescheme)LIBS))
gb_StaticLibrary_TARGETS := $(foreach namescheme,$(gb_StaticLibrary_NAMESCHEMES),$(gb_StaticLibrary_$(namescheme)LIBS))

ifeq ($(OS),LINUX)
include $(GBUILDDIR)/platform/linux.mk
else
ifeq ($(OS),WNT)
include $(GBUILDDIR)/platform/windows.mk
else
ifeq ($(OS),SOLARIS)
include $(SRCDIR)/solenv/inc/platform/solaris.mk
else
$(error unsupported OS: $(OS))
endif
endif
endif

gb_Library_KNOWNLIBS := $(foreach namescheme,$(gb_Library_NAMESCHEMES),$(gb_Library_$(namescheme)LIBS))
gb_StaticLibrary_KNOWNLIBS := $(foreach namescheme,$(gb_StaticLibrary_NAMESCHEMES),$(gb_StaticLibrary_$(namescheme)LIBS))

ifeq ($(USE_SYSTEM_STL),YES)
gb_Library_TARGETS := $(filter-out stl,$(gb_Library_TARGETS))
endif

ifeq ($(SYSTEM_LIBXML),YES)
gb_Library_TARGETS := $(filter-out xml2,$(gb_Library_TARGETS))
endif

ifeq ($(SYSTEM_ICU),YES)
gb_Library_TARGETS := $(filter-out icuuc,$(gb_Library_TARGETS))
endif

gb_GLOBALDEFS := \
    -D__DMAKE \
    -D_REENTRANT \
    -D_USE_NAMESPACE=1 \
    -DCUI \
    -DENABLE_LAYOUT_EXPERIMENTAL=0 \
    -DENABLE_LAYOUT=0 \
    -DNEW_SOLAR \
    -DOSL_DEBUG_LEVEL=$(gb_DEBUGLEVEL) \
    -DSOLAR_JAVA \
    -DSTLPORT_VERSION=$(STLPORT_VER) \
    -DSUPD=$(UPD) \
    -DVCL \
    $(gb_OSDEFS) \
    $(gb_COMPILERDEFS) \
    $(gb_CPUDEFS) \

ifeq ($(gb_PRODUCT),$(true))
gb_GLOBALDEFS += \
    -DPRODUCT \
    -DPRODUCT_FULL \

else
gb_GLOBALDEFS += \
    -DDBG_UTIL \
    -D_STLP_DEBUG \

endif

ifeq ($(gb_DEBUGLEVEL),2)
gb_GLOBALDEFS += \
    -DDEBUG \

else
gb_GLOBALDEFS += \
    -DOPTIMIZE \
    -DNDEBUG \

endif

gb_GLOBALDEFS := $(sort $(gb_GLOBALDEFS))

include $(GBUILDDIR)/target_names.mk

# static members declared here because they are used globally

gb_Library_OUTDIRLOCATION := $(OUTDIR)/lib
gb_Library_DLLDIR := $(WORKDIR)/LinkTarget/Library
gb_StaticLibrary_OUTDIRLOCATION := $(OUTDIR)/lib

# We are using a set of scopes that we might as well call classes.

include $(foreach class,\
    linktarget\
    library\
    static_library\
    executable\
    sdi\
    alllangres\
    package\
,$(GBUILDDIR)/$(class).mk)

# Module class

gb_Module_ALLMODULES :=

.PHONY : $(call gb_Module_get_clean_target,%)
$(call gb_Module_get_clean_target,%) :
    $(call gb_Helper_announce,Cleaning up module $* ...)
    -$(call gb_Helper_abbreviate_dirs,\
        rm -f $(call gb_Module_get_target,$*))

$(call gb_Module_get_target,%) :
    $(call gb_Helper_announce,Completed module $* ...)
    -$(call gb_Helper_abbreviate_dirs,\
        mkdir -p $(dir $@) && \
        touch $@)

define gb_Module_Module
$(call gb_Module_get_target,$(1)) : $(2)
$(call gb_Module_get_clean_target,$(1)) : $$(foreach target,$(2),$(call gb_Helper_get_outdir_clean_target,$$(target)))
gb_Module_ALLMODULES += $(1)

endef

define gb_Module_make_global_targets
include $(SRCDIR)/$(1)/prj/target_module_$(1).mk

.PHONY : all clean
all : $(call gb_Module_get_target,$(1))
clean : $(call gb_Module_get_clean_target,$(1))
.DEFAULT_GOAL := all

endef

define gb_Module_read_includes
include $$(foreach targetdef,$(2),$(SRCDIR)/$(1)/prj/target_$$(targetdef).mk)

endef

# vim: set noet sw=4 ts=4:
