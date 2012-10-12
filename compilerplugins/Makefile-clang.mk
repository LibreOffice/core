#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# Make sure variables in this Makefile do not conflict with other variables (e.g. from gbuild).

# The list of source files.
CLANGSRC=compileplugin.cxx \
    bodynotinblock.cxx \
    unusedvariablecheck.cxx \


# You may occassionally want to override some of these

# Compile flags ('make CLANGCXXFLAGS=-g' if you need to debug the plugin)
CLANGCXXFLAGS=-O2 -Wall -g
# The prefix where Clang resides, override to where Clang resides if using a source build.
CLANGDIR=/usr
# The build directory (different from CLANGDIR if using a Clang out-of-source build)
CLANGBUILD=/usr

# The uninteresting rest.

# Clang headers require these.
CLANGDEFS=-D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS -fno-rtti
# All include locations needed.
CLANGINCLUDES=-I$(CLANGDIR)/include -I$(CLANGDIR)/tools/clang/include -I$(CLANGBUILD)/include -I$(CLANGBUILD)/tools/clang/include

# Clang/LLVM libraries are intentionally not linked in, they are usually built as static libraries, which means the resulting
# plugin would be big (even though the clang binary already includes it all) and it'd be necessary to explicitly specify
# also all the dependency libraries.

CLANGINDIR=$(SRCDIR)/compilerplugins/clang
# Cannot use $(WORKDIR), the plugin should survive even 'make clean', otherwise the rebuilt
# plugin will cause cache misses with ccache.
CLANGOUTDIR=$(SRCDIR)/compilerplugins/obj

compilerplugins: $(CLANGOUTDIR) $(CLANGOUTDIR)/compileplugin.so

compilerplugins-clean:
	rm -rf $(CLANGOUTDIR)

$(CLANGOUTDIR):
	mkdir -p $(CLANGOUTDIR)

CLANGOBJS=

define clangbuildsrc
$(3): $(2) $(SRCDIR)/compilerplugins/Makefile-clang.mk $(CLANGOUTDIR)/clang-timestamp
	$(CXX) $(CLANGCXXFLAGS) $(CLANGDEFS) $(CLANGINCLUDES) $(2) -fPIC -c -o $(3) -MMD -MT $(3) -MP -MF $(CLANGOUTDIR)/$(1).d

-include $(CLANGOUTDIR)/$(1).d

$(CLANGOUTDIR)/compileplugin.so: $(3)
$(CLANGOUTDIR)/compileplugin.so: CLANGOBJS += $(3)
endef

$(foreach src, $(CLANGSRC), $(eval $(call clangbuildsrc,$(src),$(CLANGINDIR)/$(src),$(CLANGOUTDIR)/$(src:.cxx=.o))))

$(CLANGOUTDIR)/compileplugin.so: $(CLANGOBJS)
	$(CXX) -shared $(CLANGOBJS) -o $@

# Clang most probably doesn't maintain binary compatibility, so rebuild when clang changes.
$(CLANGOUTDIR)/clang-timestamp: $(CLANGBUILD)/bin/clang
	touch $@ -r $^

# vim: set noet sw=4 ts=4:
