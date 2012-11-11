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
# Copyright (C) 2010 Red Hat, Inc., David Tardon <dtardon@redhat.com>
#  (initial developer)
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

$(eval $(call gb_Library_Library,sdui))

$(eval $(call gb_Library_set_include,sdui,\
    $$(INCLUDE) \
    -I$(SRCDIR)/sd/inc \
    -I$(SRCDIR)/sd/source/ui/inc \
))

$(eval $(call gb_Library_add_defs,sdui,\
    -DSD_DLLIMPLEMENTATION \
))

ifeq ($(ENABLE_SDREMOTE),YES)
$(eval $(call gb_Library_add_defs,sdui,\
    -DENABLE_SDREMOTE \
))

ifeq ($(ENABLE_BLUETOOTH),YES)
$(eval $(call gb_Library_add_defs,sdui,\
    -DENABLE_BLUETOOTH \
))
endif

endif

$(eval $(call gb_Library_use_sdk_api,sdui))

$(eval $(call gb_Library_use_libraries,sdui,\
    basegfx \
    comphelper \
    cppcanvas \
    cppu \
    cppuhelper \
    editeng \
    i18nisolang1 \
    sal \
    sd \
    sfx \
    sot \
    svl \
    svt \
    svxcore \
    svx \
    tl \
    utl \
    vcl \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_exception_objects,sdui,\
    sd/source/filter/html/htmlattr \
    sd/source/filter/html/pubdlg \
    sd/source/ui/dlg/brkdlg \
    sd/source/ui/dlg/copydlg \
    sd/source/ui/dlg/custsdlg \
    sd/source/ui/dlg/dlgass \
    sd/source/ui/dlg/dlgassim \
    sd/source/ui/dlg/dlgchar \
    sd/source/ui/dlg/dlgfield \
    sd/source/ui/dlg/dlgolbul \
    sd/source/ui/dlg/dlgpage \
    sd/source/ui/dlg/dlgsnap \
    sd/source/ui/dlg/headerfooterdlg \
    sd/source/ui/dlg/inspagob \
    sd/source/ui/dlg/masterlayoutdlg \
    sd/source/ui/dlg/morphdlg \
    sd/source/ui/dlg/paragr \
    sd/source/ui/dlg/present \
    sd/source/ui/dlg/prltempl \
    sd/source/ui/dlg/prntopts \
    sd/source/ui/dlg/RemoteDialog \
    sd/source/ui/dlg/RemoteDialogClientBox \
    sd/source/ui/dlg/sddlgfact \
    sd/source/ui/dlg/sdpreslt \
    sd/source/ui/dlg/sduiexp \
    sd/source/ui/dlg/tabtempl \
    sd/source/ui/dlg/tpaction \
    sd/source/ui/dlg/tpoption \
    sd/source/ui/dlg/vectdlg \
))

# $(WORKDIR)/inc/sd/sddll0.hxx :
	# echo "#define" DLL_NAME \"$(if $(filter UNX,$(GUI)),lib)sd$(DLLPOSTFIX)$(DLLPOST)\" > $@

# vim: set noet sw=4 ts=4:
