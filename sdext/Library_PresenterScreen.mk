# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,PresenterScreen))

$(eval $(call gb_Library_set_componentfile,PresenterScreen,sdext/source/presenter/presenter,services))

$(eval $(call gb_Library_set_precompiled_header,PresenterScreen,sdext/inc/pch/precompiled_PresenterScreen))

$(eval $(call gb_Library_use_externals,PresenterScreen,\
	boost_headers \
))

$(eval $(call gb_Library_use_sdk_api,PresenterScreen))

$(eval $(call gb_Library_use_libraries,PresenterScreen,\
    cppu \
    cppuhelper \
    sal \
    tl \
    vcl \
))

$(eval $(call gb_Library_add_exception_objects,PresenterScreen,\
    sdext/source/presenter/PresenterAccessibility \
    sdext/source/presenter/PresenterBitmapContainer \
    sdext/source/presenter/PresenterButton \
    sdext/source/presenter/PresenterCanvasHelper \
    sdext/source/presenter/PresenterConfigurationAccess \
    sdext/source/presenter/PresenterController \
    sdext/source/presenter/PresenterCurrentSlideObserver \
    sdext/source/presenter/PresenterFrameworkObserver \
    sdext/source/presenter/PresenterGeometryHelper \
    sdext/source/presenter/PresenterHelpView \
    sdext/source/presenter/PresenterHelper \
    sdext/source/presenter/PresenterNotesView \
    sdext/source/presenter/PresenterPaintManager \
    sdext/source/presenter/PresenterPane \
    sdext/source/presenter/PresenterPaneBase \
    sdext/source/presenter/PresenterPaneBorderPainter \
    sdext/source/presenter/PresenterPaneContainer \
    sdext/source/presenter/PresenterPaneFactory \
    sdext/source/presenter/PresenterProtocolHandler \
    sdext/source/presenter/PresenterScreen \
    sdext/source/presenter/PresenterScrollBar \
    sdext/source/presenter/PresenterSlidePreview \
    sdext/source/presenter/PresenterSlideShowView \
    sdext/source/presenter/PresenterSlideSorter \
    sdext/source/presenter/PresenterSprite \
    sdext/source/presenter/PresenterSpritePane \
    sdext/source/presenter/PresenterTextView \
    sdext/source/presenter/PresenterTheme \
    sdext/source/presenter/PresenterTimer \
    sdext/source/presenter/PresenterToolBar \
    sdext/source/presenter/PresenterUIPainter \
    sdext/source/presenter/PresenterViewFactory \
    sdext/source/presenter/PresenterWindowManager \
))

# vim:set noet sw=4 ts=4:
