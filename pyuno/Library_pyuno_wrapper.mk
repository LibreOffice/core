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

ifneq ($(OS)$(COM),WNTMSC)
ifeq ($(filter DRAGONFLY FREEBSD NETBSD OPENBSD MACOSX,$(OS)),)

$(eval $(call gb_Library_add_libs,pyuno_wrapper,\
	-ldl \
))

endif
endif

$(eval $(call gb_Library_add_cobjects,pyuno_wrapper,\
    pyuno/source/module/pyuno_dlopenwrapper \
))

# vim:set noet sw=4 ts=4:
