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

# TODO: this seems to be unused?

$(eval $(call gb_Executable_Executable,miniapp))

$(eval $(call gb_Executable_set_targettype_gui,miniapp,YES))

$(eval $(call gb_Executable_add_precompiled_header,miniapp,automation/inc/pch/precompiled_automation.hxx))

$(eval $(call gb_Executable_set_include,miniapp,\
    $$(INCLUDE) \
    -I$(SRCDIR)/automation/inc/pch \
))

$(eval $(call gb_Executable_add_api,miniapp,\
    udkapi \
    offapi \
))

$(eval $(call gb_Executable_add_linked_libs,miniapp,\
    comphelper \
    cppu \
    cppuhelper \
    sal \
    sts \
    svt \
    tl \
    ucbhelper \
    vcl \
))

$(eval $(call gb_Executable_add_linked_static_libs,miniapp,\
    vclmain \
))

$(eval $(call gb_Executable_add_exception_objects,miniapp,\
    automation/source/miniapp/editwin \
    automation/source/miniapp/servres \
    automation/source/miniapp/testapp \
))

# vim: set ts=4 sw=4 et:
