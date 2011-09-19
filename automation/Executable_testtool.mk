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

$(eval $(call gb_Executable_Executable,testtool))

$(eval $(call gb_Executable_set_targettype_gui,testtool,YES))

$(eval $(call gb_Executable_add_package_headers,testtool,\
    automation_testtool \
))

$(eval $(call gb_Executable_add_precompiled_header,testtool,automation/inc/pch/precompiled_automation.hxx))

$(eval $(call gb_Executable_set_include,testtool,\
    $$(INCLUDE) \
    -I$(call gb_CustomTarget_get_workdir,automation/source/testtool) \
    -I$(SRCDIR)/automation/inc/pch \
    -I$(SRCDIR)/automation/inc \
    -I$(SRCDIR)/automation/source/inc \
))

$(eval $(call gb_Executable_add_api,testtool,\
    offapi \
    udkapi \
))

$(eval $(call gb_Executable_add_linked_libs,testtool,\
    comphelper \
    communi \
    cppu \
    cppuhelper \
    sal \
    sb \
    simplecm \
    sts \
    svl \
    svt \
    tl \
    ucbhelper \
    utl \
    vcl \
    $(if $(findstring WNT,$(GUI)),\
        advapi32 \
        shell32 \
    ) \
))

$(eval $(call gb_Executable_add_linked_static_libs,testtool,\
    app \
    vclmain \
    sample \
))

ifeq ($(OS),SOLARIS)

$(eval $(call gb_Executable_add_libs,testtool,\
    -lXm
))

endif

ifeq ($(GUI) $(OS),UNX LINUX)

$(eval $(call gb_Executable_add_libs,testtool,\
    -lXext \
    -lX11 \
    -lSM \
    -lICE \
))

endif

$(eval $(call gb_Executable_add_noexception_objects,testtool,\
    automation/source/app/testbasi \
    automation/source/testtool/cretstrm \
    automation/source/testtool/httprequest \
))

$(eval $(call gb_Executable_add_exception_objects,testtool,\
    automation/source/testtool/cmdstrm \
    automation/source/testtool/comm_bas \
    automation/source/testtool/objtest \
    automation/source/testtool/tcommuni \
))

ifeq ($(GUI),WNT)

$(eval $(call gb_Executable_add_noexception_objects,testtool,\
    automation/source/testtool/registry_win \
    automation/source/testtool/sysdir_win \
))

endif

# vim: set ts=4 sw=4 et:
