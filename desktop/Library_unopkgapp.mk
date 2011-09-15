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
# Copyright (C) 2011 David Tardon, Red Hat Inc. <dtardon@redhat.com> (initial developer)
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

$(eval $(call gb_Library_Library,unopkgapp))

$(eval $(call gb_Library_add_precompiled_header,unopkgapp,desktop/inc/pch/precompiled_desktop.hxx))

$(eval $(call gb_Library_set_include,unopkgapp,\
    $$(INCLUDE) \
    -I$(SRCDIR)/desktop/inc \
    -I$(SRCDIR)/desktop/inc/pch \
    -I$(SRCDIR)/desktop/source/deployment/inc \
    -I$(SRCDIR)/desktop/source/inc \
))

$(eval $(call gb_Library_add_api,unopkgapp,\
    offapi \
    udkapi \
))

$(eval $(call gb_Library_add_defs,unopkgapp,\
    -DDESKTOP_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_add_linked_libs,unopkgapp,\
    comphelper \
    cppu \
    cppuhelper \
    deploymentmisc \
    sal \
    tl \
    ucbhelper \
    utl \
    vcl \
))

$(eval $(call gb_Library_add_exception_objects,unopkgapp,\
    desktop/source/app/lockfile \
    desktop/source/pkgchk/unopkg/unopkg_app \
    desktop/source/pkgchk/unopkg/unopkg_cmdenv \
    desktop/source/pkgchk/unopkg/unopkg_misc \
))

# vim: set ts=4 sw=4 et:
