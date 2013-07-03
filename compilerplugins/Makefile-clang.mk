#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# Make sure variables in this Makefile do not conflict with other variables (e.g. from gbuild).

# You may occassionally want to override some of these

# Compile flags ('make CLANGCXXFLAGS=-g' if you need to debug the plugin)
CLANGCXXFLAGS=-O2 -Wall -Wextra -g

# The uninteresting rest.

# Clang headers require these.
CLANGDEFS=-D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS -fno-rtti
# All include locations needed.
CLANGINCLUDES=-isystem $(CLANGDIR)/include -isystem $(CLANGDIR)/tools/clang/include -isystem $(CLANGBUILD)/include -isystem $(CLANGBUILD)/tools/clang/include

# Clang/LLVM libraries are intentionally not linked in, they are usually built as static libraries, which means the resulting
# plugin would be big (even though the clang binary already includes it all) and it'd be necessary to explicitly specify
# also all the dependency libraries.

CLANGINDIR=$(SRCDIR)/compilerplugins/clang
# Cannot use $(WORKDIR), the plugin should survive even 'make clean', otherwise the rebuilt
# plugin will cause cache misses with ccache.
CLANGOUTDIR=$(BUILDDIR)/compilerplugins/obj

QUIET=$(if $(VERBOSE)$(verbose),,@)

ifneq ($(EXTERNAL_WARNINGS_NOT_ERRORS),TRUE)
CLANGWERROR := -Werror
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
$(CLANGOUTDIR)/plugin.so: CLANGFORCE
endif
# Make the .so also explicitly depend on the sources list, to force update in case CLANGSRCCHANGED was e.g. during 'make clean'.
$(CLANGOUTDIR)/plugin.so: $(CLANGOUTDIR)/sources.txt
$(CLANGOUTDIR)/sources.txt:
	touch $@

compilerplugins-build: $(CLANGOUTDIR) $(CLANGOUTDIR)/plugin.so

compilerplugins-clean:
	rm -rf $(CLANGOUTDIR)

$(CLANGOUTDIR):
	mkdir -p $(CLANGOUTDIR)

CLANGOBJS=

define clangbuildsrc
$(3): $(2) $(SRCDIR)/compilerplugins/Makefile-clang.mk $(CLANGOUTDIR)/clang-timestamp
	@echo [build CXX] $(subst $(SRCDIR)/,,$(2))
	$(QUIET)$(CXX) $(CLANGCXXFLAGS) $(CLANGWERROR) $(CLANGDEFS) $(CLANGINCLUDES) -I$(BUILDDIR)/config_host $(2) -fPIC $(CXXFLAGS_CXX11) -c -o $(3) -MMD -MT $(3) -MP -MF $(CLANGOUTDIR)/$(1).d

-include $(CLANGOUTDIR)/$(1).d

$(CLANGOUTDIR)/plugin.so: $(3)
$(CLANGOUTDIR)/plugin.so: CLANGOBJS += $(3)
endef

$(foreach src, $(CLANGSRC), $(eval $(call clangbuildsrc,$(src),$(CLANGINDIR)/$(src),$(CLANGOUTDIR)/$(src:.cxx=.o))))

$(CLANGOUTDIR)/plugin.so: $(CLANGOBJS)
	@echo [build LNK] $(subst $(BUILDDIR)/,,$@)
	$(QUIET)$(CXX) -shared $(CLANGOBJS) -o $@

# Clang most probably doesn't maintain binary compatibility, so rebuild when clang changes.
$(CLANGOUTDIR)/clang-timestamp: $(CLANGBUILD)/bin/clang
	$(QUIET)touch $@

# vim: set noet sw=4 ts=4:
