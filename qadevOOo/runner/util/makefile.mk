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
# $Revision: 1.10 $
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
