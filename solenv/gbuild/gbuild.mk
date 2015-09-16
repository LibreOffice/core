#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



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
# UPD

# GXX_INCLUDE_PATH (Linux)
# PTHREAD_CFLAGS (Linux)
# SYSTEM_ICU (Linux)
# SYSTEM_JPEG (Linux)
# SYSTEM_LIBXML (Linux)
# USE_SYSTEM_STL (Linux)

ifeq ($(OS),OS2)
SHELL := /@unixroot/usr/bin/sh
else
SHELL := /bin/sh
endif
true := T
false :=
define NEWLINE


endef

define WHITESPACE
 
endef

COMMA :=,

# optional extensions that should never be essential
ifneq ($(wildcard $(GBUILDDIR)/extensions/pre_*.mk),)
include $(wildcard $(GBUILDDIR)/extensions/pre_*.mk)
endif

include $(GBUILDDIR)/Output.mk

# BuildDirs uses the Output functions already
include $(GBUILDDIR)/BuildDirs.mk


ifneq ($(strip $(PRODUCT)$(product)),)
gb_PRODUCT := $(true)
else
ifneq ($(strip $(product)),)
gb_PRODUCT := $(true)
else
gb_PRODUCT := $(false)
endif
endif

ifneq ($(strip $(DEBUG)$(debug)),)
gb_DEBUGLEVEL := 2
else
ifeq ($(gb_PRODUCT),$(true))
gb_DEBUGLEVEL := 0
else
gb_DEBUGLEVEL := 1
endif
endif

ifneq ($(strip $(ENABLE_PCH)),)
gb_ENABLE_PCH := $(true)
else
gb_ENABLE_PCH := $(false)
endif

# for clean, setuplocal and removelocal goals we switch off dependencies
ifneq ($(filter cleanpackmodule clean setuplocal removelocal showdeliverables,$(MAKECMDGOALS)),)
gb_FULLDEPS := $(false)
else
gb_FULLDEPS := $(true)
endif

include $(GBUILDDIR)/Helper.mk
include $(GBUILDDIR)/TargetLocations.mk

$(eval $(call gb_Helper_init_registries))
$(eval $(call gb_Helper_add_repositories,$(gb_REPOS)))
$(eval $(call gb_Helper_collect_libtargets))

ifeq ($(OS),LINUX)
include $(GBUILDDIR)/platform/linux.mk
else
ifeq ($(OS),WNT)
ifneq ($(USE_MINGW),)
include $(GBUILDDIR)/platform/winmingw.mk
else
include $(GBUILDDIR)/platform/windows.mk
endif
else
ifeq ($(OS),SOLARIS)
include $(GBUILDDIR)/platform/solaris.mk
else
ifeq ($(OS),MACOSX)
include $(GBUILDDIR)/platform/macosx.mk
else
ifeq ($(OS),FREEBSD)
include $(GBUILDDIR)/platform/freebsd.mk
else
ifeq ($(OS),OS2)
include $(GBUILDDIR)/platform/os2.mk
else
$(eval $(call gb_Output_error,Unsupported OS: $(OS)))
endif
endif
endif
endif
endif
endif

include $(GBUILDDIR)/Tempfile.mk

# this is optional
-include $(foreach repo,$(gb_REPOS),$(repo)/RepositoryFixes.mk)

$(eval $(call gb_Helper_collect_knownlibs))

gb_GLOBALDEFS := \
	-D_REENTRANT \
	-DCUI \
	-DENABLE_LAYOUT_EXPERIMENTAL=0 \
	-DENABLE_LAYOUT=0 \
	-DOSL_DEBUG_LEVEL=$(gb_DEBUGLEVEL) \
	-DSOLAR_JAVA \
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

ifneq ($(strip $(ENABLE_GTK)),)
gb_GLOBALDEFS += -DENABLE_GTK
endif

ifneq ($(strip $(ENABLE_KDE)),)
gb_GLOBALDEFS += -DENABLE_KDE
endif

ifneq ($(strip $(ENABLE_KDE4)),)
gb_GLOBALDEFS += -DENABLE_KDE4
endif

ifeq ($(strip $(ENABLE_GRAPHITE)),TRUE)
gb_GLOBALDEFS += -DENABLE_GRAPHITE
endif

gb_GLOBALDEFS := $(sort $(gb_GLOBALDEFS))

include $(GBUILDDIR)/Deliver.mk

$(eval $(call gb_Deliver_init))

# We are using a set of scopes that we might as well call classes.

# It is important to include them in the right order as that is
# -- at least in part -- defining precedence. This is not an issue in the
# WORKDIR as there are no nameing collisions there, but OUTDIR is a mess
# and precedence is important there. This is also platform dependent.
# For example:
# $(OUTDIR)/bin/% for executables collides
#	with $(OUTDIR)/bin/%.res for resources on unix
# $(OUTDIR)/lib/%.lib collides
#	on windows (static and dynamic libs)
# $(OUTDIR)/xml/% for packageparts collides
#	with $(OUTDIR)/xml/component/%.component for components
# This is less of an issue with GNU Make versions > 3.82 which matches for
# shortest stem instead of first match. However, upon intoduction this version
# is not available everywhere by default.

include $(foreach class, \
	ComponentTarget \
	AllLangResTarget \
	WinResTarget \
	LinkTarget \
	Library \
	StaticLibrary \
	Executable \
	SdiTarget \
	Package \
	CustomTarget \
	PrecompiledHeaders \
	CppunitTest \
	GoogleTest \
	JavaClassSet \
	JunitTest \
	Module \
,$(GBUILDDIR)/$(class).mk)

# optional extensions that should never be essential
ifneq ($(wildcard $(GBUILDDIR)/extensions/post_*.mk),)
include $(wildcard $(GBUILDDIR)/extensions/post_*.mk)
endif

ifeq ($(SYSTEM_LIBXSLT),YES)
gb_XSLTPROCTARGET :=
gb_XSLTPROC := xsltproc
else
gb_XSLTPROCTARGET := $(call gb_Executable_get_target,xsltproc)
gb_XSLTPROC := $(gb_XSLTPROCPRECOMMAND) $(gb_XSLTPROCTARGET)
endif

export gb_AWK
export gb_XSLTPROC

# vim: set noet sw=4 ts=4:
