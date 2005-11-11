#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.13 $
#
#   last change: $Author: rt $ $Date: 2005-11-11 13:24:01 $
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

PRJNAME=api

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
    XStyleCondition.idl

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
