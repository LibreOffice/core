#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.15 $
#
#   last change: $Author: rt $ $Date: 2006-12-01 17:17:15 $
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
