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

include $(dir $(realpath $(lastword $(MAKEFILE_LIST))))platform.mk

$(eval $(call gb_Extension_Extension,presenter-screen,sdext/source/presenter))

$(eval $(call gb_Extension_set_platform,presenter-screen,$(sdext_PLATFORM)))

$(eval $(call gb_Extension_add_libraries,presenter-screen,\
    PresenterScreen \
))

$(eval $(call gb_Extension_add_file,presenter-screen,components.rdb,$(call gb_Rdb_get_target,presenter)))

$(eval $(call gb_Extension_add_files,presenter-screen,bitmaps,\
    $(SRCDIR)/icon-themes/galaxy/desktop/res/extension_32.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/Background.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/BorderActiveBottom.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/BorderActiveBottomCallout.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/BorderActiveBottomLeft.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/BorderActiveBottomRight.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/BorderActiveLeft.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/BorderActiveRight.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/BorderActiveTop.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/BorderActiveTopLeft.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/BorderActiveTopRight.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/BorderBottom.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/BorderBottomLeft.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/BorderBottomRight.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/BorderCurrentSlideBottom.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/BorderCurrentSlideBottomLeft.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/BorderCurrentSlideBottomRight.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/BorderCurrentSlideLeft.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/BorderCurrentSlideRight.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/BorderCurrentSlideTop.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/BorderCurrentSlideTopLeft.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/BorderCurrentSlideTopRight.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/BorderLeft.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/BorderRight.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/BorderToolbarBottom.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/BorderToolbarLeft.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/BorderToolbarRight.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/BorderToolbarTop.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/BorderToolbarTopLeft.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/BorderToolbarTopRight.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/BorderTop.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/BorderTopLeft.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/BorderTopRight.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/ButtonEffectNextDisabled.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/ButtonEffectNextMouseOver.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/ButtonEffectNextNormal.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/ButtonEffectNextSelected.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/ButtonFrameCenterMouseOver.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/ButtonFrameCenterNormal.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/ButtonFrameLeftMouseOver.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/ButtonFrameLeftNormal.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/ButtonFrameRightMouseOver.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/ButtonFrameRightNormal.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/ButtonHelpDisabled.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/ButtonHelpMouseOver.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/ButtonHelpNormal.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/ButtonHelpSelected.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/ButtonMinusDisabled.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/ButtonMinusMouseOver.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/ButtonMinusNormal.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/ButtonMinusSelected.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/ButtonNotesDisabled.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/ButtonNotesMouseOver.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/ButtonNotesNormal.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/ButtonNotesSelected.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/ButtonPlusDisabled.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/ButtonPlusMouseOver.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/ButtonPlusNormal.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/ButtonPlusSelected.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/ButtonSlideNextDisabled.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/ButtonSlideNextMouseOver.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/ButtonSlideNextNormal.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/ButtonSlidePreviousDisabled.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/ButtonSlidePreviousMouseOver.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/ButtonSlidePreviousNormal.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/ButtonSlidePreviousSelected.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/ButtonSlideSorterDisabled.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/ButtonSlideSorterMouseOver.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/ButtonSlideSorterNormal.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/ButtonSlideSorterSelected.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/ButtonSwitchMonitorMouseOver.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/ButtonSwitchMonitorNormal.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/LabelMouseOverCenter.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/LabelMouseOverLeft.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/LabelMouseOverRight.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/ScrollbarArrowDownDisabled.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/ScrollbarArrowDownMouseOver.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/ScrollbarArrowDownNormal.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/ScrollbarArrowDownSelected.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/ScrollbarArrowUpDisabled.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/ScrollbarArrowUpMouseOver.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/ScrollbarArrowUpNormal.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/ScrollbarArrowUpSelected.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/ScrollbarPagerMiddleMouseOver.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/ScrollbarPagerMiddleNormal.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/ScrollbarThumbBottomMouseOver.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/ScrollbarThumbBottomNormal.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/ScrollbarThumbMiddleMouseOver.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/ScrollbarThumbMiddleNormal.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/ScrollbarThumbTopMouseOver.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/ScrollbarThumbTopNormal.png \
    $(SRCDIR)/sdext/source/presenter/bitmaps/ViewBackground.png \
))

$(eval $(call gb_Extension_add_file,presenter-screen,registry/schema/org/openoffice/Office/extension/PresenterScreen.xcs,$(call gb_XcsTarget_get_target,sdext/source/presenter/registry/schema/org/openoffice/Office/extension/PresenterScreen.xcs)))

$(eval $(call gb_Extension_add_file,presenter-screen,registry/data/org/openoffice/Office/extension/PresenterScreen.xcu,$(call gb_XcuMergeTarget_get_target,sdext/source/presenter/registry/data/org/openoffice/Office/extension/PresenterScreen.xcu)))

$(eval $(call gb_Extension_add_files,presenter-screen,registry/data/org/openoffice/Office,\
    $(call gb_XcuDataTarget_get_target,sdext/source/presenter/registry/data/org/openoffice/Office/Jobs.xcu) \
    $(call gb_XcuDataTarget_get_target,sdext/source/presenter/registry/data/org/openoffice/Office/ProtocolHandler.xcu) \
))

$(eval $(call gb_Extension_add_helpfile,presenter-screen,$(WORKDIR)/CustomTarget/sdext/source/presenter/help/en-US,com.sun.PresenterScreen-$(sdext_PLATFORM)/presenter.xhp,com.sun.PresenterScreen/presenter.xhp))

# vim:set shiftwidth=4 softtabstop=4 expandtab:
