#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: vg $ $Date: 2006-03-16 11:08:35 $
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

PRJ = ..$/..$/..$/..
TARGET  = FCFGUtils
PRJNAME = $(TARGET)
PACKAGE = com$/sun$/star$/filter$/config$/tools$/utils

# --- Settings -----------------------------------------------------

.INCLUDE: settings.mk

#----- compile .java files -----------------------------------------

JARFILES        =   \
                    ridl.jar        \
                    unoil.jar       \
                    jurt.jar        \
                    juh.jar         \
                    jut.jar         \
                    java_uno.jar    \
                    xml-apis.jar    \
                    xercesImpl.jar

JAVACLASSFILES  =   \
                    $(CLASSDIR)$/$(PACKAGE)$/AnalyzeStartupLog.class              \
                    $(CLASSDIR)$/$(PACKAGE)$/ConfigHelper.class                   \
                    $(CLASSDIR)$/$(PACKAGE)$/Logger.class                         \
                    $(CLASSDIR)$/$(PACKAGE)$/FileHelper.class                     \
                    $(CLASSDIR)$/$(PACKAGE)$/MalformedCommandLineException.class  \
                    $(CLASSDIR)$/$(PACKAGE)$/Cache.class                          \
                    $(CLASSDIR)$/$(PACKAGE)$/XMLHelper.class

MAXLINELENGTH   =   100000

# --- targets -----------------------------------------------------

.INCLUDE :  target.mk
