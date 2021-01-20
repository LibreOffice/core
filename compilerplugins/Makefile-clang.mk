#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# Make sure variables in this Makefile do not conflict with other variables (e.g. from gbuild).

# Set to 1 if you need to debug the plugin).
CLANGDEBUG=

# Compile flags, you may occasionally want to override these:
ifeq ($(OS),WNT)
# See LLVM's cmake/modules/AddLLVM.cmake and LLVM build's
# tools/llvm-config/BuildVariables.inc:
# * Ignore "warning C4141: 'inline': used more than once" as emitted upon
#   "LLVM_ATTRIBUTE_ALWAYS_INLINE inline" in various LLVM include files.
# * Ignore "warning C4577: 'noexcept' used with no exception handling mode
#   specified; termination on exception is not guaranteed. Specify /EHsc".
CLANGCXXFLAGS=/nologo /D_HAS_EXCEPTIONS=0 /wd4141 /wd4577 /EHs-c- /GR-
ifeq ($(CLANGDEBUG),)
CLANGCXXFLAGS+=/O2 /Oi
else
CLANGCXXFLAGS+=/DEBUG /Od
endif
else # WNT
CLANGCXXFLAGS=-Wall -Wextra -Wundef -fvisibility-inlines-hidden
ifeq ($(CLANGDEBUG),)
CLANGCXXFLAGS+=-O2
else
CLANGCXXFLAGS+=-g -O0 -UNDEBUG
endif
endif

# Whether to make plugins use one shared ASTRecursiveVisitor (plugins run faster).
# By default enabled, disable if you work on an affected plugin (re-generating takes time).
LO_CLANG_SHARED_PLUGINS=1
#TODO: Windows doesn't use LO_CLANG_SHARED_PLUGINS for now, see corresponding TODO comment in
# configure.ac:
ifeq ($(OS),WNT)
LO_CLANG_SHARED_PLUGINS=
endif

# Whether to use precompiled headers for the sources. This is actually controlled
# by gb_ENABLE_PCH like everywhere else, but unsetting this disables PCH.
LO_CLANG_USE_PCH=1

# The uninteresting rest.

include $(SRCDIR)/solenv/gbuild/gbuild.mk
include $(SRCDIR)/solenv/gbuild/Output.mk

CLANG_COMMA :=,

ifeq ($(OS),WNT)
CLANG_DL_EXT = .dll
CLANG_EXE_EXT = .exe
else
CLANG_DL_EXT = .so
CLANG_EXE_EXT =
endif

# Clang headers require these.
CLANGDEFS:=$(COMPILER_PLUGINS_CXXFLAGS)
# All include locations needed (using -isystem silences various warnings when
# including those files):
ifneq ($(OS),WNT)
CLANGDEFS:=$(filter-out -isystem/usr/include,$(foreach opt,$(CLANGDEFS),$(patsubst -I%,-isystem%,$(opt))))
endif

# Clang/LLVM libraries are intentionally not linked in, they are usually built as static libraries, which means the resulting
# plugin would be big (even though the clang binary already includes it all) and it'd be necessary to explicitly specify
# also all the dependency libraries.

CLANGINDIR=$(SRCDIR)/compilerplugins/clang
# Cannot use $(WORKDIR), the plugin should survive even 'make clean', otherwise the rebuilt
# plugin will cause cache misses with ccache.
CLANGOUTDIR=$(BUILDDIR)/compilerplugins/clang
CLANGOBJDIR=$(CLANGOUTDIR)/obj

ifdef LO_CLANG_SHARED_PLUGINS
CLANGCXXFLAGS+=-DLO_CLANG_SHARED_PLUGINS
endif

ifneq ($(CLANGDEBUG),)
ifeq ($(HAVE_GCC_SPLIT_DWARF),TRUE)
CLANGCXXFLAGS+=-gsplit-dwarf
endif
endif

QUIET=$(if $(verbose),,@)

ifneq ($(ENABLE_WERROR),)
ifeq ($(OS),WNT)
CLANGWERROR :=
#TODO: /WX
else
CLANGWERROR := -Werror
# When COMPILER_PLUGINS_CXXFLAGS (obtained via `llvm-config --cxxflags`) contains options like
# -Wno-maybe-uninitialized that are targeting GCC (when LLVM was actually built with GCC), and
# COMPILER_PLUGINS_CXX (defaulting to CXX) denotes a Clang that does not understand those options,
# it fails with -Werror,-Wunknown-warning-option, so we need -Wno-unknown-warning-option (but which
# GCC does not understand) at least with -Werror:
ifeq ($(COMPILER_PLUGINS_COM_IS_CLANG),TRUE)
CLANGWERROR += -Wno-unknown-warning-option
endif
endif
endif

ifneq ($(LO_CLANG_USE_PCH),)
# Reset and enable only if actually supported and enabled.
LO_CLANG_USE_PCH=
ifneq ($(gb_ENABLE_PCH),)
ifneq ($(OS),WNT)
# Currently only Clang PCH is supported (which should usually be the case, as Clang is usually self-built).
ifneq ($(findstring clang,$(COMPILER_PLUGINS_CXX)),)
LO_CLANG_USE_PCH=1
LO_CLANG_PCH_FLAGS:=-Xclang -fno-pch-timestamp
endif
endif
endif
endif


compilerplugins: compilerplugins-build

ifdef LO_CLANG_SHARED_PLUGINS
# The shared source, intentionally put first in the list because it takes the longest to build.
CLANGSRCOUTDIR=$(CLANGOUTDIR)/sharedvisitor/sharedvisitor.cxx
CLANGSRC+=$(CLANGSRCOUTDIR)
endif
# The list of source files, generated automatically (all files in clang/, but not subdirs).
CLANGSRCINDIR=$(sort $(foreach src,$(wildcard $(CLANGINDIR)/*.cxx), $(notdir $(src))))
CLANGSRC+=$(CLANGSRCINDIR)

# Remember the sources and if they have changed, force plugin relinking.
CLANGSRCCHANGED= \
    $(shell mkdir -p $(CLANGOUTDIR) ; \
            echo $(CLANGSRC) | sort > $(CLANGOUTDIR)/sources-new.txt; \
            if diff $(CLANGOUTDIR)/sources.txt $(CLANGOUTDIR)/sources-new.txt >/dev/null 2>/dev/null; then \
                echo 0; \
            else \
                mv $(CLANGOUTDIR)/sources-new.txt $(CLANGOUTDIR)/sources.txt; \
                echo 1; \
            fi; \
    )
ifeq ($(CLANGSRCCHANGED),1)
.PHONY: CLANGFORCE
CLANGFORCE:
$(CLANGOUTDIR)/plugin$(CLANG_DL_EXT): CLANGFORCE
endif
# Make the .so also explicitly depend on the sources list, to force update in case CLANGSRCCHANGED was e.g. during 'make clean'.
$(CLANGOUTDIR)/plugin$(CLANG_DL_EXT): $(CLANGOUTDIR)/sources.txt
$(CLANGOUTDIR)/sources.txt:
	touch $@

compilerplugins-build: $(CLANGOUTDIR) $(CLANGOBJDIR) $(CLANGOUTDIR)/plugin$(CLANG_DL_EXT)

compilerplugins-clean:
	rm -rf \
        $(CLANGOBJDIR) \
        $(CLANGOUTDIR)/clang-timestamp \
        $(CLANGOUTDIR)/plugin$(CLANG_DL_EXT) \
        $(CLANGOUTDIR)/clang.pch{,.d} \
        $(CLANGOUTDIR)/sharedvisitor/*.plugininfo \
        $(CLANGOUTDIR)/sharedvisitor/clang.pch{,.d} \
        $(CLANGOUTDIR)/sharedvisitor/sharedvisitor.{cxx,d,o} \
        $(CLANGOUTDIR)/sharedvisitor/{analyzer,generator}{$(CLANG_EXE_EXT),.d,.o} \
        $(CLANGOUTDIR)/sources-new.txt \
        $(CLANGOUTDIR)/sources-shared-new.txt \
        $(CLANGOUTDIR)/sources-shared.txt \
        $(CLANGOUTDIR)/sources.txt

$(CLANGOUTDIR):
	mkdir -p $(CLANGOUTDIR)

$(CLANGOBJDIR):
	mkdir -p $(CLANGOBJDIR)

CLANGOBJS=

ifeq ($(OS),WNT)

# clangbuildsrc cxxfile objfile dfile
define clangbuildsrc
$(2): $(1) $(SRCDIR)/compilerplugins/Makefile-clang.mk $(CLANGOUTDIR)/clang-timestamp
	$$(call gb_Output_announce,$(subst $(SRCDIR)/,,$(subst $(BUILDDIR)/,,$(1))),$(true),CXX,3)
	$(QUIET)$(COMPILER_PLUGINS_CXX) $(CLANGDEFS) $(CLANGCXXFLAGS) $(CLANGWERROR) \
        $(CLANGINCLUDES) /I$(BUILDDIR)/config_host /I$(CLANGINDIR) $(1) /MD \
        /c /Fo: $(2)

-include $(3) #TODO

$(CLANGOUTDIR)/plugin$(CLANG_DL_EXT): $(2)
$(CLANGOUTDIR)/plugin$(CLANG_DL_EXT): CLANGOBJS += $(2)
endef

else

# clangbuildsrc cxxfile ofile dfile
define clangbuildsrc
$(2): $(1) $(SRCDIR)/compilerplugins/Makefile-clang.mk $(CLANGOUTDIR)/clang-timestamp \
        $(if $(LO_CLANG_USE_PCH),$(CLANGOUTDIR)/clang.pch)
	$$(call gb_Output_announce,$(subst $(SRCDIR)/,,$(subst $(BUILDDIR)/,,$(1))),$(true),CXX,3)
	$(QUIET)$(COMPILER_PLUGINS_CXX) $(CLANGDEFS) $(CLANGCXXFLAGS) $(CLANGWERROR) \
	$(CLANGINCLUDES) -I$(BUILDDIR)/config_host -I$(CLANGINDIR) $(1) \
	$(if $(LO_CLANG_USE_PCH),-include-pch $(CLANGOUTDIR)/clang.pch -DPCH_LEVEL=$(gb_ENABLE_PCH)) \
	-fPIC -c -o $(2) -MMD -MT $(2) -MP -MF $(3)

-include $(3)

$(CLANGOUTDIR)/plugin$(CLANG_DL_EXT): $(2)
$(CLANGOUTDIR)/plugin$(CLANG_DL_EXT): CLANGOBJS += $(2)
endef

endif

$(foreach src, $(CLANGSRCOUTDIR), $(eval $(call clangbuildsrc,$(src),$(src:.cxx=.o),$(src:.cxx=.d))))
$(foreach src, $(CLANGSRCINDIR), $(eval $(call clangbuildsrc,$(CLANGINDIR)/$(src),$(CLANGOBJDIR)/$(src:.cxx=.o),$(CLANGOBJDIR)/$(src:.cxx=.d))))

$(CLANGOUTDIR)/plugin$(CLANG_DL_EXT): $(CLANGOBJS)
	$(call gb_Output_announce,$(subst $(BUILDDIR)/,,$@),$(true),LNK,4)
ifeq ($(OS),WNT)
	$(QUIET)$(COMPILER_PLUGINS_CXX) /LD $(CLANGOBJS) /Fe: $@ $(CLANGLIBDIR)/clang.lib \
        mincore.lib version.lib /link $(COMPILER_PLUGINS_CXX_LINKFLAGS)
else
	$(QUIET)$(COMPILER_PLUGINS_CXX) -shared $(CLANGOBJS) -o $@ \
		$(if $(filter MACOSX,$(OS)),-Wl$(CLANG_COMMA)-flat_namespace \
			-Wl$(CLANG_COMMA)-undefined -Wl$(CLANG_COMMA)suppress)
endif

# Clang most probably doesn't maintain binary compatibility, so rebuild when clang changes
# (either the binary can change if it's a local build, or config_clang.h will change if configure detects
# a new version of a newly installed system clang).
$(CLANGOUTDIR)/clang-timestamp: $(CLANGDIR)/bin/clang$(CLANG_EXE_EXT) $(BUILDDIR)/config_host/config_clang.h
	$(QUIET)touch $@


ifdef LO_CLANG_SHARED_PLUGINS
SHARED_SOURCES := $(sort $(shell grep -l "LO_CLANG_SHARED_PLUGINS" $(CLANGINDIR)/*.cxx))
SHARED_SOURCE_INFOS := $(foreach source,$(SHARED_SOURCES),$(patsubst $(CLANGINDIR)/%.cxx,$(CLANGOUTDIR)/sharedvisitor/%.plugininfo,$(source)))

$(CLANGOUTDIR)/sharedvisitor/%.plugininfo: $(CLANGINDIR)/%.cxx \
            $(CLANGOUTDIR)/sharedvisitor/analyzer$(CLANG_EXE_EXT) \
            $(CLANGOUTDIR)/sharedvisitor/clang.pch
	$(call gb_Output_announce,$(subst $(BUILDDIR)/,,$@),$(true),GEN,1)
	$(QUIET)$(ICECREAM_RUN) $(CLANGOUTDIR)/sharedvisitor/analyzer$(CLANG_EXE_EXT) \
        $(COMPILER_PLUGINS_TOOLING_ARGS:%=-arg=%) $< > $@

$(CLANGOUTDIR)/sharedvisitor/sharedvisitor.cxx: $(SHARED_SOURCE_INFOS) $(CLANGOUTDIR)/sharedvisitor/generator$(CLANG_EXE_EXT)
	$(call gb_Output_announce,$(subst $(BUILDDIR)/,,$@),$(true),GEN,1)
	$(QUIET)$(ICECREAM_RUN) $(CLANGOUTDIR)/sharedvisitor/generator$(CLANG_EXE_EXT) \
        $(SHARED_SOURCE_INFOS) > $@

# Flags used internally in analyzer.
# Older versions of Clang have a problem to find their own internal headers, so add it.
# Also filter out the c++ library, it's not necessary to be specific about it in this case
# and it can also cause trouble with finding the proper headers.
CLANGTOOLDEFS = $(filter-out -stdlib=%,$(CLANGDEFS) -I$(CLANGSYSINCLUDE))
CLANGTOOLDEFS += -w
ifneq ($(filter-out MACOSX WNT,$(OS)),)
ifneq ($(CLANGDIR),/usr)
# Help the generator find Clang shared libs, if Clang is built so and installed in a non-standard prefix.
CLANGTOOLLIBS += -Wl,--rpath,$(CLANGLIBDIR)
endif
endif

$(CLANGOUTDIR)/sharedvisitor/analyzer$(CLANG_EXE_EXT): $(CLANGINDIR)/sharedvisitor/analyzer.cxx \
        | $(CLANGOUTDIR)/sharedvisitor
	$(call gb_Output_announce,$(subst $(BUILDDIR)/,,$@),$(true),GEN,1)
	$(QUIET)$(COMPILER_PLUGINS_CXX) $(CLANGDEFS) $(CLANGCXXFLAGS) $(CLANGWERROR) $(CLANGINCLUDES) \
        -I$(BUILDDIR)/config_host -DCLANGFLAGS='"$(CLANGTOOLDEFS)"' \
        -DLO_CLANG_USE_ANALYZER_PCH=$(if $(COMPILER_PLUGINS_ANALYZER_PCH),1,0) \
        -c $< -o $(CLANGOUTDIR)/sharedvisitor/analyzer.o -MMD -MT $@ -MP \
        -MF $(CLANGOUTDIR)/sharedvisitor/analyzer.d
	$(QUIET)$(COMPILER_PLUGINS_CXX) $(CLANGDEFS) $(CLANGCXXFLAGS) $(CLANGOUTDIR)/sharedvisitor/analyzer.o \
        -o $@ $(CLANGTOOLLIBS)

$(CLANGOUTDIR)/sharedvisitor/generator$(CLANG_EXE_EXT): $(CLANGINDIR)/sharedvisitor/generator.cxx \
        | $(CLANGOUTDIR)/sharedvisitor
	$(call gb_Output_announce,$(subst $(BUILDDIR)/,,$@),$(true),GEN,1)
	$(QUIET)$(COMPILER_PLUGINS_CXX) $(CLANGDEFS) $(CLANGCXXFLAGS) $(CLANGWERROR) \
        -c $< -o $(CLANGOUTDIR)/sharedvisitor/generator.o -MMD -MT $@ -MP \
        -MF $(CLANGOUTDIR)/sharedvisitor/generator.d
	$(QUIET)$(COMPILER_PLUGINS_CXX) $(CLANGDEFS) $(CLANGCXXFLAGS) $(CLANGOUTDIR)/sharedvisitor/generator.o \
        -o $@

$(CLANGOUTDIR)/sharedvisitor/analyzer$(CLANG_EXE_EXT): $(SRCDIR)/compilerplugins/Makefile-clang.mk $(CLANGOUTDIR)/clang-timestamp

$(CLANGOUTDIR)/sharedvisitor/generator$(CLANG_EXE_EXT): $(SRCDIR)/compilerplugins/Makefile-clang.mk

$(CLANGOUTDIR)/sharedvisitor:
	mkdir -p $(CLANGOUTDIR)/sharedvisitor

-include $(CLANGOUTDIR)/sharedvisitor/analyzer.d
-include $(CLANGOUTDIR)/sharedvisitor/generator.d
# TODO WNT version

# Remember the sources that are shared and if they have changed, force sharedvisitor.cxx generating.
# Duplicated from CLANGSRCCHANGED above.
CLANGSRCSHAREDCHANGED= \
    $(shell mkdir -p $(CLANGOUTDIR) ; \
            echo $(SHARED_SOURCES) | sort > $(CLANGOUTDIR)/sources-shared-new.txt; \
            if diff $(CLANGOUTDIR)/sources-shared.txt $(CLANGOUTDIR)/sources-shared-new.txt >/dev/null 2>/dev/null; then \
                echo 0; \
            else \
                mv $(CLANGOUTDIR)/sources-shared-new.txt $(CLANGOUTDIR)/sources-shared.txt; \
                echo 1; \
            fi; \
    )
ifeq ($(CLANGSRCSHAREDCHANGED),1)
.PHONY: CLANGFORCE
CLANGFORCE:
$(CLANGOUTDIR)/sharedvisitor/sharedvisitor.cxx: CLANGFORCE
endif
# Make sharedvisitor.cxx also explicitly depend on the sources list, to force update in case CLANGSRCSHAREDCHANGED was e.g. during 'make clean'.
$(CLANGOUTDIR)/sharedvisitor/sharedvisitor.cxx: $(CLANGOUTDIR)/sources-shared.txt
$(CLANGOUTDIR)/sources-shared.txt:
	touch $@
endif

ifneq ($(LO_CLANG_USE_PCH),)
# the PCH for plugin sources themselves

ifeq ($(OS),WNT)
# TODO
else
$(CLANGOUTDIR)/clang.pch: $(CLANGINDIR)/precompiled_clang.hxx \
        $(SRCDIR)/compilerplugins/Makefile-clang.mk $(CLANGOUTDIR)/clang-timestamp
	$(call gb_Output_announce,$(subst $(BUILDDIR)/,,$@),$(true),PCH,1)
	$(QUIET)$(COMPILER_PLUGINS_CXX) -x c++-header $(CLANGDEFS) $(CLANGCXXFLAGS) $(CLANGWERROR) \
        $(CLANGINCLUDES) -I$(BUILDDIR)/config_host -I$(CLANGINDIR) -DPCH_LEVEL=$(gb_ENABLE_PCH) \
        $(LO_CLANG_PCH_FLAGS) \
        -fPIC -c $< -o $@ -MMD -MT $@ -MP -MF $(CLANGOUTDIR)/clang.pch.d
endif
-include $(CLANGOUTDIR)/clang.pch.d

endif

ifeq ($(COMPILER_PLUGINS_ANALYZER_PCH),TRUE)
# the PCH for usage in sharedvisitor/analyzer

# these are from the invocation in analyzer.cxx
LO_CLANG_ANALYZER_PCH_CXXFLAGS := -I$(BUILDDIR)/config_host $(CLANGTOOLDEFS)

$(CLANGOUTDIR)/sharedvisitor/clang.pch: $(CLANGINDIR)/sharedvisitor/precompiled_clang.hxx \
        $(SRCDIR)/compilerplugins/Makefile-clang.mk $(CLANGOUTDIR)/clang-timestamp \
        | $(CLANGOUTDIR)/sharedvisitor
	$(call gb_Output_announce,$(subst $(BUILDDIR)/,,$@),$(true),PCH,1)
	$(QUIET)$(CLANGDIR)/bin/clang -x c++-header $(LO_CLANG_ANALYZER_PCH_CXXFLAGS) \
        $(LO_CLANG_PCH_FLAGS) $(COMPILER_PLUGINS_TOOLING_ARGS) -c $< -o $@ -MMD -MT $@ -MP \
        -MF $(CLANGOUTDIR)/sharedvisitor/clang.pch.d

-include $(CLANGOUTDIR)/sharedvisitor/clang.pch.d

else
$(CLANGOUTDIR)/sharedvisitor/clang.pch:
	touch $@
endif

# vim: set noet sw=4 ts=4:
