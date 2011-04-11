#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
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

TARGET=cssstyle
PACKAGE=com$/sun$/star$/style

# --- Settings -----------------------------------------------------
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# ------------------------------------------------------------------------

IDLFILES=\
    BreakType.idl\
    CaseMap.idl\
    CellStyle.idl\
    CharacterProperties.idl\
    CharacterPropertiesAsian.idl\
    CharacterPropertiesComplex.idl\
    CharacterStyle.idl\
    DropCapFormat.idl\
    FootnoteLineStyle.idl\
    GraphicLocation.idl\
    HorizontalAlignment.idl\
    LineNumberPosition.idl\
    LineSpacing.idl\
    LineSpacingMode.idl\
    NumberingAlignment.idl\
    NumberingLevel.idl\
    NumberingRule.idl\
    NumberingType.idl\
    PageProperties.idl\
    PageStyle.idl\
    PageStyleLayout.idl\
    ParagraphAdjust.idl\
    ParagraphProperties.idl\
    ParagraphPropertiesAsian.idl\
    ParagraphPropertiesComplex.idl\
    ParagraphStyle.idl\
    ParagraphStyleCategory.idl\
    Style.idl\
    StyleFamilies.idl\
    StyleFamily.idl\
    TabAlign.idl\
    TabStop.idl\
    VerticalAlignment.idl\
    XDefaultsSupplier.idl\
    XStyle.idl\
    XStyleFamiliesSupplier.idl\
    XStyleLoader.idl\
    XStyleCondition.idl\
    XAutoStylesSupplier.idl\
    XAutoStyles.idl\
    XAutoStyleFamily.idl\
    XAutoStyle.idl
    

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
