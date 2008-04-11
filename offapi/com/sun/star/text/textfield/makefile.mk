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
# $Revision: 1.16 $
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
