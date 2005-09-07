#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 20:19:03 $
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

PRJNAME=sj2
TARGET=controller

PACKAGE=stardiv$/controller
JARFILES=sandbox.jar tkt.jar

.INCLUDE :  $(PRJ)$/util$/makefile.pmk

.IF "$(JDK)" == "gcj"
all:
        @echo This dir cannot be build with gcj because of com.sun.star.lib.sandbox.ResourceProxy
.ELSE

# --- Files --------------------------------------------------------

JAVAFILES=	\
    SjSettings.java					

JAVACLASSFILES=	\
                $(CLASSDIR)$/$(PACKAGE)$/SjSettings.class				\
                $(CLASSDIR)$/$(PACKAGE)$/StreamObserver.class			\
                $(CLASSDIR)$/$(PACKAGE)$/NativeStreamObserver.class	\
                $(CLASSDIR)$/$(PACKAGE)$/AppStarter.class				\
                $(CLASSDIR)$/$(PACKAGE)$/AppStarterStatus.class		\
                $(CLASSDIR)$/$(PACKAGE)$/AppStarterStatusNative.class	\
                $(CLASSDIR)$/$(PACKAGE)$/Console.class					\
                $(CLASSDIR)$/$(PACKAGE)$/JavaSystemMonitor.class		\
                $(CLASSDIR)$/$(PACKAGE)$/PropertyEditor.class

# --- Targets ------------------------------------------------------
.ENDIF

.INCLUDE :  target.mk

.INCLUDE :  $(PRJ)$/util$/target.pmk

