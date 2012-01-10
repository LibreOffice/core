#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



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
    Presentation2.idl\
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
