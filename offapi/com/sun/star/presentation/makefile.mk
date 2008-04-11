#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.15 $
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

PRJ=..$/..$/..$/..

PRJNAME=offapi

TARGET=csspresentation
PACKAGE=com$/sun$/star$/presentation

# --- Settings -----------------------------------------------------
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# ------------------------------------------------------------------------

IDLFILES=\
    HeaderShape.idl\
    FooterShape.idl\
    DateTimeShape.idl\
    SlideNumberShape.idl\
    AnimationEffect.idl\
    AnimationSpeed.idl\
    ChartShape.idl\
    ClickAction.idl\
    CustomPresentation.idl\
    CustomPresentationAccess.idl\
    DocumentSettings.idl\
    DrawPage.idl\
    FadeEffect.idl\
    GraphicObjectShape.idl\
    HandoutShape.idl\
    HandoutView.idl\
    NotesShape.idl\
    NotesView.idl\
    OLE2Shape.idl\
    OutlinerShape.idl\
    OutlineView.idl\
    PageShape.idl\
    Presentation.idl\
    PresentationDocument.idl\
    PresentationRange.idl\
    PresentationView.idl\
    PreviewView.idl\
    Shape.idl\
    SlidesView.idl\
    SubtitleShape.idl\
    TitleTextShape.idl\
    XCustomPresentationSupplier.idl\
    XHandoutMasterSupplier.idl\
    XPresentation.idl\
    XPresentationPage.idl\
    XPresentationSupplier.idl\
    EffectNodeType.idl\
    EffectPresetClass.idl\
    ParagraphTarget.idl\
    ShapeAnimationSubType.idl\
    TextAnimationType.idl\
    EffectCommands.idl\
    XShapeEventListener.idl\
    XSlideShow.idl\
    XSlideShowController.idl\
    XSlideShowListener.idl\
    XSlideShowView.idl\
    XPresentation2.idl\
    XTransition.idl\
    XTransitionFactory.idl

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
