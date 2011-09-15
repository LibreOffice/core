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

$(eval $(call gb_Library_Library,sts))

$(eval $(call gb_Library_add_precompiled_header,sts,automation/inc/pch/precompiled_automation.hxx))

$(eval $(call gb_Library_set_include,sts,\
    $$(INCLUDE) \
    -I$(WORKDIR)/CustomTarget/automation/source/testtool \
    -I$(SRCDIR)/automation/inc/pch \
    -I$(SRCDIR)/automation/source/inc \
    -I$(SRCDIR)/automation/inc \
))

$(eval $(call gb_Library_add_api,sts,\
    offapi \
    udkapi \
))

$(eval $(call gb_Library_add_defs,sts,\
    -DSTS_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_add_linked_libs,sts,\
    communi \
    comphelper \
    cppu \
    cppuhelper \
    sal \
    sb \
    simplecm \
    sot \
    svl \
    svt \
    tl \
    utl \
    vcl \
    $(if $(findstring WNT,$(GUI)),\
        advapi32 \
        gdi32 \
    ) \
))

$(eval $(call gb_Library_add_exception_objects,sts,\
    automation/source/server/XMLParser \
    automation/source/server/profiler \
    automation/source/server/recorder \
    automation/source/server/server \
    automation/source/server/sta_list \
))

$(eval $(call gb_Library_add_noexception_objects,sts,\
    automation/source/server/cmdbasestream \
    automation/source/server/editwin \
    automation/source/server/retstrm \
    automation/source/server/scmdstrm \
    automation/source/server/svcommstream \
))

$(eval $(call gb_Library_add_cxxobjects,sts,\
    automation/source/server/statemnt \
    ,$(gb_COMPILERNOOPTFLAGS) $(gb_LinkTarget_EXCEPTIONFLAGS) \
))

ifeq ($(OS),SOLARIS)

$(eval $(call gb_Library_add_noexception_objects,sts,\
    automation/source/server/prof_usl \
))

else

$(eval $(call gb_Library_add_noexception_objects,sts,\
    automation/source/server/prof_nul \
))

endif

# vim: set ts=4 sw=4 et:
