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
    XStyleCondition.idl\
    XAutoStylesSupplier.idl\
    XAutoStyles.idl\
    XAutoStyleFamily.idl\
    XAutoStyle.idl
    

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
