# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# Major Contributor(s):
# Copyright (C) 2012 David Ostrovsky <d.ostrovsky@gmx.de> (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_Library_Library,pyuno_wrapper))

$(eval $(call gb_Library_use_packages,pyuno_wrapper,\
    sal_generated \
    sal_odk_headers \
))

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
