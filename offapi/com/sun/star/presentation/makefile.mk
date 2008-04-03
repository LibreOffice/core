#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.14 $
#
#   last change: $Author: kz $ $Date: 2008-04-03 15:39:06 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
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
