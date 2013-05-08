# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,pyuno_wrapper))

$(eval $(call gb_Library_set_include,pyuno_wrapper,\
    -I$(SRCDIR)/pyuno/source/module \
    -I$(SRCDIR)/pyuno/inc \
    $$(INCLUDE) \
))

# not using external "python" because we do not want to link against python
$(eval $(call gb_Library_use_externals,pyuno_wrapper,\
    python_headers \
))

# python "import pyuno" dlopens pyuno.so as RTLD_LOCAL, so g++ exception
# handling used to not work, so pyuno.so (pyuno_wrapper) is just a thin wrapper
# that dlopens libpyuno.so as RTLD_GLOBAL; but when pyuno.so wrapper links
# against libstdc++ (which has not previously been loaded into python process),
# that resolves its _ZNSs4_Rep20_S_empty_rep_storageE to itself, but later LO
# libs (loaded though RTLD_GLOBAL libpyuno.so) may resolve that symbol to e.g.
# cppu, because they happen to see that before libstdc++; so the requirement has
# always been that RTLD_LOCAL-loaded pyuno.so wrapper implicitly load into the
# process as little as possible:
$(eval $(call gb_Library_add_ldflags,pyuno_wrapper,-nostdlib))
$(eval $(call gb_Library_add_libs,pyuno_wrapper,-lc))

ifeq ($(filter DRAGONFLY FREEBSD NETBSD OPENBSD MACOSX,$(OS)),)

$(eval $(call gb_Library_add_libs,pyuno_wrapper,\
	-ldl \
))

endif

$(eval $(call gb_Library_add_cobjects,pyuno_wrapper,\
    pyuno/source/module/pyuno_dlopenwrapper \
))

# vim:set noet sw=4 ts=4:
