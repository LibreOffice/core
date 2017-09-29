#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# Make sure variables in this Makefile do not conflict with other variables (e.g. from gbuild).

CLANG_COMMA :=,

ifeq ($(OS),WNT)
CLANG_DL_EXT = .dll
CLANG_EXE_EXT = .exe
else
CLANG_DL_EXT = .so
CLANG_EXE_EXT =
endif

ifeq ($(COMPILER_PLUGINS_CXX),)
CLANGCXX=$(filter-out -m32 -m64 -fsanitize%,$(CXX))
else
CLANGCXX=$(COMPILER_PLUGINS_CXX)
endif

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

# The uninteresting rest.

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

# Clang/LLVM libraries are intentionally not linked in, they are usually built as static libraries, which means the resulting
# plugin would be big (even though the clang binary already includes it all) and it'd be necessary to explicitly specify
# also all the dependency libraries.

CLANGINDIR=$(SRCDIR)/compilerplugins/clang
# Cannot use $(WORKDIR), the plugin should survive even 'make clean', otherwise the rebuilt
# plugin will cause cache misses with ccache.
CLANGOUTDIR=$(BUILDDIR)/compilerplugins/obj

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

# The list of source files, generated automatically (all files in clang/, but not subdirs).
CLANGSRC=$(foreach src,$(wildcard $(CLANGINDIR)/*.cxx), $(notdir $(src)))
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
	@echo [build CXX] $(subst $(SRCDIR)/,,$(2))
	$(QUIET)$(CLANGCXX) $(CLANGCXXFLAGS) $(CLANGWERROR) $(CLANGDEFS) \
        $(CLANGINCLUDES) /I$(BUILDDIR)/config_host $(2) $(CXXFLAGS_CXX11) /MD \
        /c /Fo: $(3)

-include $(CLANGOUTDIR)/$(1).d #TODO

$(CLANGOUTDIR)/plugin$(CLANG_DL_EXT): $(3)
$(CLANGOUTDIR)/plugin$(CLANG_DL_EXT): CLANGOBJS += $(3)
endef

else

define clangbuildsrc
$(3): $(2) $(SRCDIR)/compilerplugins/Makefile-clang.mk $(CLANGOUTDIR)/clang-timestamp
	@echo [build CXX] $(subst $(SRCDIR)/,,$(2))
	$(QUIET)$(CLANGCXX) $(CLANGCXXFLAGS) $(CLANGWERROR) $(CLANGDEFS) $(CLANGINCLUDES) -I$(BUILDDIR)/config_host $(2) -fPIC $(CXXFLAGS_CXX11) -c -o $(3) -MMD -MT $(3) -MP -MF $(CLANGOUTDIR)/$(1).d

-include $(CLANGOUTDIR)/$(1).d

$(CLANGOUTDIR)/plugin$(CLANG_DL_EXT): $(3)
$(CLANGOUTDIR)/plugin$(CLANG_DL_EXT): CLANGOBJS += $(3)
endef

endif

$(foreach src, $(CLANGSRC), $(eval $(call clangbuildsrc,$(src),$(CLANGINDIR)/$(src),$(CLANGOUTDIR)/$(src:.cxx=.o))))

$(CLANGOUTDIR)/plugin$(CLANG_DL_EXT): $(CLANGOBJS)
	@echo [build LNK] $(subst $(BUILDDIR)/,,$@)
ifeq ($(OS),WNT)
	$(QUIET)$(CLANGCXX) /LD $(CLANGOBJS) /Fe: $@ $(CLANGLIBDIR)/clang.lib \
        mincore.lib version.lib /link $(COMPILER_PLUGINS_CXX_LINKFLAGS)
else
	$(QUIET)$(CLANGCXX) -shared $(CLANGOBJS) -o $@ \
		$(if $(filter MACOSX,$(OS)),-Wl$(CLANG_COMMA)-flat_namespace \
			-Wl$(CLANG_COMMA)-undefined -Wl$(CLANG_COMMA)suppress)
endif

# Clang most probably doesn't maintain binary compatibility, so rebuild when clang changes.
$(CLANGOUTDIR)/clang-timestamp: $(CLANGDIR)/bin/clang$(CLANG_EXE_EXT)
	$(QUIET)touch $@

# vim: set noet sw=4 ts=4:
