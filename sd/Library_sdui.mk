# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,sdui))

$(eval $(call gb_Library_set_include,sdui,\
    $$(INCLUDE) \
    -I$(SRCDIR)/sd/inc \
    -I$(SRCDIR)/sd/source/ui/inc \
))

ifeq ($(ENABLE_SDREMOTE),YES)
$(eval $(call gb_Library_add_defs,sdui,\
    -DENABLE_SDREMOTE \
))

ifeq ($(ENABLE_SDREMOTE_BLUETOOTH),YES)
$(eval $(call gb_Library_add_defs,sdui,\
    -DENABLE_SDREMOTE_BLUETOOTH \
))
endif

endif

$(eval $(call gb_Library_use_custom_headers,sdui,\
	officecfg/registry \
))

$(eval $(call gb_Library_use_external,sdui,boost_headers))

$(eval $(call gb_Library_use_sdk_api,sdui))

$(eval $(call gb_Library_use_libraries,sdui,\
    basegfx \
    comphelper \
    cppcanvas \
    cppu \
    cppuhelper \
    editeng \
    i18nlangtag \
    fwk \
    fwl \
    fwe \
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
    sd/source/ui/dlg/PhotoAlbumDialog \
))

# $(WORKDIR)/inc/sd/sddll0.hxx :
	# echo "#define" DLL_NAME \"$(if $(filter-out WNT,$(OS)),lib)sd$(DLLPOSTFIX)$(DLLPOST)\" > $@

# vim: set noet sw=4 ts=4:
