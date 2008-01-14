#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.10 $
#
#   last change: $Author: ihi $ $Date: 2008-01-14 13:21:41 $
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
PACKAGE = helper
TARGET = runner_helper

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

JARFILES = ridl.jar jurt.jar juh.jar jut.jar \
           unoil.jar

JAVAFILES =	APIDescGetter.java      \
            ConfigurationRead.java  \
            StreamSimulator.java	\
            AppProvider.java        \
            URLHelper.java			\
            CfgParser.java          \
            SimpleMailSender.java          \
            WindowListener.java		\
            ClParser.java           \
            OfficeWatcher.java      \
            OfficeProvider.java		\
            ComplexDescGetter.java  \
            InetTools.java          \
            ProcessHandler.java	\
            ContextMenuInterceptor.java	\
            UnoProvider.java\
            PropertyHelper.java\
            FileTools.java

JAVACLASSFILES=	$(foreach,i,$(JAVAFILES) $(CLASSDIR)$/$(PACKAGE)$/$(i:b).class)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
