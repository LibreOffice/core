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



PRJ=..$/..$/..$/..$/..

PRJNAME=offapi

TARGET=csstextfield
PACKAGE=com$/sun$/star$/text$/textfield

# --- Settings -----------------------------------------------------
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# ------------------------------------------------------------------------

IDLFILES=\
    URL.idl\
    Annotation.idl\
    Author.idl\
    Bibliography.idl\
    Chapter.idl\
    CharacterCount.idl\
    CombinedCharacters.idl\
    ConditionalText.idl\
    Database.idl\
    DatabaseName.idl\
    DatabaseNextSet.idl\
    DatabaseNumberOfSet.idl\
    DatabaseSetNumber.idl\
    DateTime.idl\
    DDE.idl\
    DropDown.idl\
    EmbeddedObjectCount.idl\
    ExtendedUser.idl\
    FileName.idl\
    GetExpression.idl\
    GetReference.idl\
    GraphicObjectCount.idl\
    HiddenParagraph.idl\
    HiddenText.idl\
    Input.idl\
    InputUser.idl\
    JumpEdit.idl\
    Macro.idl\
    MetadataField.idl\
    PageCount.idl\
    PageNumber.idl\
    ParagraphCount.idl\
    ReferencePageGet.idl\
    ReferencePageSet.idl\
    Script.idl\
    SetExpression.idl\
    TableCount.idl\
    TableFormula.idl\
    TemplateName.idl\
    User.idl\
    WordCount.idl\

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
