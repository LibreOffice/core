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


PRJ=..$/..$/..

PRJNAME=oovapi

TARGET=word
PACKAGE=ooo$/vba$/word

# --- Settings -----------------------------------------------------
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# ------------------------------------------------------------------------


IDLFILES= XGlobals.idl\
    XApplication.idl \
    XDocument.idl \
    XWindow.idl \
    XSystem.idl \
    XRange.idl \
    XBookmark.idl \
    XBookmarks.idl \
    XVariable.idl \
    XVariables.idl \
    XView.idl \
    XPane.idl \
    XPanes.idl \
    XOptions.idl \
    XSelection.idl \
     XTemplate.idl \
    XParagraphFormat.idl \
    XAutoTextEntries.idl \
    XAutoTextEntry.idl \
    XParagraphs.idl \
    XParagraph.idl \
    XFind.idl \
    XReplacement.idl \
    XStyle.idl \
    XStyles.idl \
    XFont.idl \
    XTable.idl \
    XTables.idl \
    XField.idl \
    XFields.idl \
    XBorder.idl \
    XBorders.idl \
    XDocuments.idl \
    XHeaderFooter.idl \
    XAddins.idl \
    XAddin.idl \
    XDialogs.idl \
    XDialog.idl \
    XWrapFormat.idl \
    XPageSetup.idl \
    XSection.idl \
    XSections.idl \

# ------------------------------------------------------------------

.INCLUDE :  target.mk

