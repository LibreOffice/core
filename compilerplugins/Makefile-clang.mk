#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# Make sure variables in this Makefile do not conflict with other variables (e.g. from gbuild).

# Compile flags ('make CLANGCXXFLAGS=-g' if you need to debug the plugin); you
# may occasionally want to override these:
ifeq ($(OS),WNT)
# See LLVM's cmake/modules/AddLLVM.cmake and LLVM build's
# tools/llvm-config/BuildVariables.inc:
# * Ignore "warning C4141: 'inline': used more than once" as emitted upon
#   "LLVM_ATTRIBUTE_ALWAYS_INLINE inline" in various LLVM include files.
# * Ignore "warning C4577: 'noexcept' used with no exception handling mode
#   specified; termination on exception is not guaranteed. Specify /EHsc".
CLANGCXXFLAGS=/nologo /D_HAS_EXCEPTIONS=0 /wd4141 /wd4577 /O2 /Oi /EHs-c- /GR-
else
CLANGCXXFLAGS=-O2 -Wall -Wextra -Wundef -g
endif

# Whether to make plugins use one shared ASTRecursiveVisitor (plugins run faster).
# By default enabled, disable if you work on an affected plugin (re-generating takes time).
LO_CLANG_SHARED_PLUGINS=1

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
CLANGDEFS=-D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS
ifneq ($(OS),WNT)
CLANGDEFS += -fno-rtti
endif
# All include locations needed (using -isystem silences various warnings when
# including those files):
ifeq ($(OS),WNT)
CLANGINCLUDES=-I$(CLANGDIR)/include
else
CLANGINCLUDES=$(if $(filter /usr,$(CLANGDIR)),,-isystem $(CLANGDIR)/include)
endif

LLVMCONFIG=$(CLANGDIR)/bin/llvm-config

# Clang/LLVM libraries are intentionally not linked in, they are usually built as static libraries, which means the resulting
# plugin would be big (even though the clang binary already includes it all) and it'd be necessary to explicitly specify
# also all the dependency libraries.

CLANGINDIR=$(SRCDIR)/compilerplugins/clang
# Cannot use $(WORKDIR), the plugin should survive even 'make clean', otherwise the rebuilt
# plugin will cause cache misses with ccache.
CLANGOUTDIR=$(BUILDDIR)/compilerplugins/obj

ifdef LO_CLANG_SHARED_PLUGINS
CLANGCXXFLAGS+=-DLO_CLANG_SHARED_PLUGINS
endif

QUIET=$(if $(verbose),,@)

ifneq ($(ENABLE_WERROR),)
ifeq ($(OS),WNT)
CLANGWERROR :=
#TODO: /WX
else
CLANGWERROR := -Werror
endif
endif

compilerplugins: compilerplugins-build

ifdef LO_CLANG_SHARED_PLUGINS
# The shared source, intentionally put first in the list because it takes the longest to build.
CLANGSRC=sharedvisitor/sharedvisitor.cxx
endif
# The list of source files, generated automatically (all files in clang/, but not subdirs).
CLANGSRC+=$(foreach src,$(wildcard $(CLANGINDIR)/*.cxx), $(notdir $(src)))

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

compilerplugins-build: $(CLANGOUTDIR) $(CLANGOUTDIR)/plugin$(CLANG_DL_EXT)

compilerplugins-clean:
	rm -rf $(CLANGOUTDIR)

$(CLANGOUTDIR):
	mkdir -p $(CLANGOUTDIR)

CLANGOBJS=

ifeq ($(OS),WNT)

define clangbuildsrc
$(3): $(2) $(SRCDIR)/compilerplugins/Makefile-clang.mk $(CLANGOUTDIR)/clang-timestamp
	$$(call gb_Output_announce,$(subst $(SRCDIR)/,,$(2)),$(true),CXX,3)
	$(QUIET)$(COMPILER_PLUGINS_CXX) $(CLANGCXXFLAGS) $(CLANGWERROR) $(CLANGDEFS) \
        $(CLANGINCLUDES) /I$(BUILDDIR)/config_host $(2) /MD \
        /c /Fo: $(3)

-include $(CLANGOUTDIR)/$(1).d #TODO

$(CLANGOUTDIR)/plugin$(CLANG_DL_EXT): $(3)
$(CLANGOUTDIR)/plugin$(CLANG_DL_EXT): CLANGOBJS += $(3)
endef

else

define clangbuildsrc
$(3): $(2) $(SRCDIR)/compilerplugins/Makefile-clang.mk $(CLANGOUTDIR)/clang-timestamp
	$$(call gb_Output_announce,$(subst $(SRCDIR)/,,$(2)),$(true),CXX,3)
	$(QUIET)$(COMPILER_PLUGINS_CXX) $(CLANGCXXFLAGS) $(CLANGWERROR) $(CLANGDEFS) $(CLANGINCLUDES) -I$(BUILDDIR)/config_host $(2) -fPIC -c -o $(3) -MMD -MT $(3) -MP -MF $(CLANGOUTDIR)/$(1).d

-include $(CLANGOUTDIR)/$(1).d

$(CLANGOUTDIR)/plugin$(CLANG_DL_EXT): $(3)
$(CLANGOUTDIR)/plugin$(CLANG_DL_EXT): CLANGOBJS += $(3)
endef

endif

$(foreach src, $(CLANGSRC), $(eval $(call clangbuildsrc,$(src),$(CLANGINDIR)/$(src),$(CLANGOUTDIR)/$(src:.cxx=.o))))

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

# Clang most probably doesn't maintain binary compatibility, so rebuild when clang changes.
$(CLANGOUTDIR)/clang-timestamp: $(CLANGDIR)/bin/clang$(CLANG_EXE_EXT)
	$(QUIET)touch $@


ifdef LO_CLANG_SHARED_PLUGINS
# Update the shared visitor source if needed. It is intentionally included in the sources and generated only when
# needed, because the generating takes a while.
# If you want to remake the source with LO_CLANG_SHARED_PLUGINS disabled, run 'make LO_CLANG_SHARED_PLUGINS=1'.
$(CLANGINDIR)/sharedvisitor/sharedvisitor.cxx: $(shell grep -l "LO_CLANG_SHARED_PLUGINS" $(CLANGINDIR)/*.cxx)
$(CLANGINDIR)/sharedvisitor/sharedvisitor.cxx: $(CLANGOUTDIR)/sharedvisitor/generator$(CLANG_EXE_EXT)
	$(call gb_Output_announce,$(subst $(SRCDIR)/,,$@),$(true),GEN,1)
	$(CLANGOUTDIR)/sharedvisitor/generator$(CLANG_EXE_EXT) $(shell grep -l "LO_CLANG_SHARED_PLUGINS" $(CLANGINDIR)/*.cxx) \
        > $(CLANGINDIR)/sharedvisitor/sharedvisitor.cxx

CLANGTOOLLIBS = -lclangTooling -lclangDriver -lclangFrontend -lclangParse -lclangSema -lclangEdit -lclangAnalysis \
        -lclangAST -lclangLex -lclangSerialization -lclangBasic $(shell $(LLVMCONFIG) --ldflags --libs --system-libs)
# Path to the clang system headers (no idea if there's a better way to get it).
CLANGTOOLDEFS = -DCLANGSYSINCLUDE=$(shell $(LLVMCONFIG) --libdir)/clang/$(shell $(LLVMCONFIG) --version | sed 's/svn//')/include

$(CLANGOUTDIR)/sharedvisitor/generator$(CLANG_EXE_EXT): $(CLANGINDIR)/sharedvisitor/generator.cxx \
        | $(CLANGOUTDIR)/sharedvisitor
	$(call gb_Output_announce,$(subst $(BUILDDIR)/,,$@),$(true),GEN,1)
	$(QUIET)$(COMPILER_PLUGINS_CXX) $(CLANGCXXFLAGS) $(CLANGWERROR) $(CLANGDEFS) $(CLANGTOOLDEFS) $(CLANGINCLUDES) \
        -DCLANGDIR=$(CLANGDIR) -DBUILDDIR=$(BUILDDIR) -I$(BUILDDIR)/config_host \
        $< -o $@ -MMD -MT $@ -MP -MF $(CLANGOUTDIR)/sharedvisitor/generator.d $(CLANGTOOLLIBS)

$(CLANGOUTDIR)/sharedvisitor:
	mkdir -p $(CLANGOUTDIR)/sharedvisitor

-include $(CLANGOUTDIR)/sharedvisitor/generator.d
# TODO WNT version
endif

# vim: set noet sw=4 ts=4:
