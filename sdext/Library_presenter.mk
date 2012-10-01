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
# Copyright (C) 2012 Red Hat, Inc., David Tardon <dtardon@redhat.com>
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

$(eval $(call gb_Library_Library,PresenterScreen))

$(eval $(call gb_Library_set_componentfile,PresenterScreen,sdext/source/presenter/presenter))

$(eval $(call gb_Library_use_sdk_api,PresenterScreen))

$(eval $(call gb_Library_add_defs,PresenterScreen,\
    -DPRESENTER_IMPL_IDENTIFIER=\"com.sun.PresenterScreen-$(PLATFORMID)\" \
))

$(eval $(call gb_Library_use_libraries,PresenterScreen,\
    cppu \
    cppuhelper \
    sal \
))

$(eval $(call gb_Library_add_exception_objects,PresenterScreen,\
    sdext/source/presenter/PresenterAccessibility \
    sdext/source/presenter/PresenterBitmapContainer \
    sdext/source/presenter/PresenterButton \
    sdext/source/presenter/PresenterCanvasHelper \
    sdext/source/presenter/PresenterComponent \
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
    sdext/source/presenter/PresenterPaneBorderManager \
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

# vim:set shiftwidth=4 softtabstop=4 expandtab:
