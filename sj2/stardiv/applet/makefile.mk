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
# $Revision: 1.4 $
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

PRJNAME=sj2
TARGET=applet

PACKAGE=stardiv$/applet

.INCLUDE :  $(PRJ)$/util$/makefile.pmk

.IF "$(JDK)" == "gcj"
all:
        @echo This dir cannot be build with gcj because of com.sun.star.lib.sandbox.ResourceProxy
.ELSE

# --- Files --------------------------------------------------------

JARFILES= \
    sandbox.jar

JAVAFILES=\
    AppletExecutionContext.java	\
    Document.java				\
    DocumentProxy.java			\
    LiveConnectable.java		

#.IF "$(GUI)"=="WNT"
#JAVAFILES += 	WNativeAppletViewerFrame.java
#.ENDIF

JAVACLASSFILES=	\
    $(CLASSDIR)$/$(PACKAGE)$/DocumentProxy.class			\
    $(CLASSDIR)$/$(PACKAGE)$/Document.class				\
    $(CLASSDIR)$/$(PACKAGE)$/LiveConnectable.class			\
    $(CLASSDIR)$/$(PACKAGE)$/AppletExecutionContext.class

.ENDIF

#.IF "$(GUI)"=="WNT"
#JAVACLASSFILES += $(CLASSDIR)$/stardiv$/applet$/WNativeAppletViewerFrame.class
#.ENDIF


# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

.INCLUDE :  $(PRJ)$/util$/target.pmk
