#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.9 $
#
#   last change: $Author: obo $ $Date: 2007-01-25 13:16:42 $
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

PRJ=..$/..

PRJNAME = OOoRunner
PACKAGE = util
TARGET = runner_util

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

JARFILES = ridl.jar jurt.jar unoil.jar

JAVAFILES =	AccessibilityTools.java	\
            BasicMacroTools.java    \
            BookmarkDsc.java		\
            ControlDsc.java			\
            dbg.java				\
            DBTools.java			\
            DefaultDsc.java			\
            DesktopTools.java		\
            DrawTools.java          \
            DynamicClassLoader.java \
            FootnoteDsc.java		\
            FormTools.java			\
            FrameDsc.java			\
            InstCreator.java		\
            InstDescr.java			\
            ParagraphDsc.java       \
            ReferenceMarkDsc.java	\
            RegistryTools.java		\
            ShapeDsc.java			\
            SOfficeFactory.java		\
            StyleFamilyDsc.java		\
            PropertyName.java		\
            SysUtils.java           \
            TableDsc.java			\
            TextSectionDsc.java		\
            XLayerImpl.java 		\
            XLayerHandlerImpl.java 	\
            XSchemaHandlerImpl.java \
            UITools.java            \
            utils.java              \
            ValueChanger.java		\
            ValueComparer.java		\
            WaitUnreachable.java    \
            WriterTools.java		\
            XInstCreator.java       \
            XMLTools.java

JAVACLASSFILES=	$(foreach,i,$(JAVAFILES) $(CLASSDIR)$/$(PACKAGE)$/$(i:b).class)

# --- Targets ------------------------------------------------------


.INCLUDE :  target.mk

TST:
        @echo $(JAVACLASSFILES)
