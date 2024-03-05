# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,sdui))

$(eval $(call gb_Library_set_componentfile,sdui,sd/source/console/presenter,services))

$(eval $(call gb_Library_set_include,sdui,\
    $$(INCLUDE) \
    -I$(SRCDIR)/sd/inc \
    -I$(SRCDIR)/sd/source/ui/inc \
))

ifeq ($(ENABLE_SDREMOTE),TRUE)
$(eval $(call gb_Library_add_defs,sdui,\
    -DENABLE_SDREMOTE \
))

ifeq ($(ENABLE_SDREMOTE_BLUETOOTH),TRUE)
$(eval $(call gb_Library_add_defs,sdui,\
    -DENABLE_SDREMOTE_BLUETOOTH \
))
endif

endif

$(eval $(call gb_Library_use_custom_headers,sdui,\
	officecfg/registry \
))

$(eval $(call gb_Library_set_precompiled_header,sdui,sd/inc/pch/precompiled_sdui))

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
    sal \
    salhelper \
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
))

$(eval $(call gb_Library_add_exception_objects,sdui,\
    sd/source/ui/dlg/brkdlg \
    sd/source/ui/dlg/copydlg \
    sd/source/ui/dlg/custsdlg \
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
    sd/source/ui/dlg/BulletAndPositionDlg \
    sd/source/console/PresenterAccessibility \
    sd/source/console/PresenterBitmapContainer \
    sd/source/console/PresenterButton \
    sd/source/console/PresenterCanvasHelper \
    sd/source/console/PresenterConfigurationAccess \
    sd/source/console/PresenterController \
    sd/source/console/PresenterCurrentSlideObserver \
    sd/source/console/PresenterFrameworkObserver \
    sd/source/console/PresenterGeometryHelper \
    sd/source/console/PresenterHelpView \
    sd/source/console/PresenterHelper \
    sd/source/console/PresenterNotesView \
    sd/source/console/PresenterPaintManager \
    sd/source/console/PresenterPane \
    sd/source/console/PresenterPaneBase \
    sd/source/console/PresenterPaneBorderPainter \
    sd/source/console/PresenterPaneContainer \
    sd/source/console/PresenterPaneFactory \
    sd/source/console/PresenterProtocolHandler \
    sd/source/console/PresenterScreen \
    sd/source/console/PresenterScrollBar \
    sd/source/console/PresenterSlidePreview \
    sd/source/console/PresenterSlideShowView \
    sd/source/console/PresenterSlideSorter \
    sd/source/console/PresenterSprite \
    sd/source/console/PresenterSpritePane \
    sd/source/console/PresenterTextView \
    sd/source/console/PresenterTheme \
    sd/source/console/PresenterTimer \
    sd/source/console/PresenterToolBar \
    sd/source/console/PresenterUIPainter \
    sd/source/console/PresenterViewFactory \
    sd/source/console/PresenterWindowManager \
))

# $(WORKDIR)/inc/sd/sddll0.hxx :
	# echo "#define" DLL_NAME \"$(if $(filter-out WNT,$(OS)),lib)sd$(DLLPOSTFIX)$(DLLPOST)\" > $@

# vim: set noet sw=4 ts=4:
