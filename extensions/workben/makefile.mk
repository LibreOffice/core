#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: obo $ $Date: 2007-03-09 08:42:32 $
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

PRJ=..

PRJNAME=extensions
TARGET=workben
LIBTARGET=NO

TESTAPP=testpgp
#TESTAPP=testcomponent
#TESTAPP=pythontest
#TESTAPP=pythonautotest
#TESTAPP=testresource
#TESTAPP=testframecontrol

.IF "$(TESTAPP)" == "testresource" || "$(TESTAPP)" == "testframecontrol"
TARGETTYPE=GUI
.ELSE
TARGETTYPE=CUI
.ENDIF

JVM_TARGET=jvmtest

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

UNOUCROUT=$(OUT)$/inc$/$(PRJNAME)$/$(TARGET)
INCPRE+=$(UNOUCROUT)

# --- Files --------------------------------------------------------

#
# testpgp.
#
.IF "$(TESTAPP)" == "testpgp"

UNOTYPES= \
    com.sun.star.lang.XMultiServiceFactory \
    com.sun.star.io.XInputStream \
    com.sun.star.io.XOutputStream \
    com.sun.star.pgp.RecipientsEvent \
    com.sun.star.pgp.SignatureEvent \
    com.sun.star.pgp.XPGPDecoder \
    com.sun.star.pgp.XPGPDecoderListener \
    com.sun.star.pgp.XPGPEncoder \
    com.sun.star.pgp.XPGPPreferences \
    com.sun.star.uno.TypeClass \
    com.sun.star.uno.XInterface \
    com.sun.star.uno.XWeak

OBJFILES=	$(OBJ)$/testpgp.obj

APP1TARGET=	testpgp
APP1OBJS=	$(OBJFILES)
APP1STDLIBS= \
            $(CPPUHELPERLIB)\
            $(CPPULIB)		\
            $(VOSLIB)		\
            $(SALLIB)

.ENDIF # testpgp

#
# std testcomponent
#
.IF "$(TESTAPP)" == "testcomponent"

OBJFILES=	$(OBJ)$/testcomponent.obj

APP2TARGET = testcomponent
APP2OBJS   = $(OBJ)$/testcomponent.obj
APP2STDLIBS = \
              $(VOSLIB) \
              $(TOOLSLIB)	\
              $(SALLIB)

.ENDIF # testcomponent

#
# test python.
#
.IF "$(TESTAPP)" == "phytontest"

OBJFILES=	$(OBJ)$/pythontest.obj

#APP4TARGET=	pythontest
#APP4OBJS=	$(OBJ)$/pythontest.obj 
#APP4STDLIBS=$(TOOLSLIB)		\
#			$(VOSLIB)

.ENDIF # phytontest

#
# python auto test
#
.IF "$(TESTAPP)" == "phytonautotest"

OBJFILES=	$(OBJ)$/pythonautotest.obj

#APP5TARGET= pythonautotest
#APP5OBJS = $(OBJ)$/pythonautotest.obj
#APP5STDLIBS=$(TOOLSLIB)		\
#			$(VOSLIB)

.ENDIF # phytonautotest

#
# testresource.
#
.IF "$(TESTAPP)" == "testresource"

OBJFILES=	$(OBJ)$/testresource.obj

SRS2NAME =		testresource
SRC2FILES=		testresource.src
RESLIB2SRSFILES= $(SRS)$/testresource.srs
RESLIB2NAME=	testresource

APP2TARGET=	testresource
APP2OBJS=	$(OBJ)$/testresource.obj
APP2STDLIBS=$(TOOLSLIB)		\
            $(VCLLIB)		\
            $(SALLIB)

.ENDIF # testresource

#
# testframecontrol.
#
.IF "$(TESTAPP)" == "testframecontrol"

OBJFILES=	$(OBJ)$/testframecontrol.obj

APP3TARGET=	testframecontrol
APP3OBJS=	$(OBJ)$/testframecontrol.obj
APP3STDLIBS=$(TOOLSLIB)		\
            $(VOSLIB)		\
            $(SVTOOLLIB)	\
            $(VCLLIB)

.ENDIF # testframecontrol

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

