#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.1.1.1 $
#
#   last change: $Author: hr $ $Date: 2000-09-18 16:16:56 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
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

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk

UNOUCRDEP=$(SOLARBINDIR)$/applicat.rdb
UNOUCRRDB=$(SOLARBINDIR)$/applicat.rdb
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

CXXFILES=	testpgp.cxx
OBJFILES=	$(OBJ)$/testpgp.obj

APP1TARGET=	testpgp
APP1OBJS=	$(OBJFILES)
APP1STDLIBS=$(USRLIB)		\
            $(CPPUHELPERLIB)\
            $(CPPULIB)		\
            $(VOSLIB)		\
            $(SALLIB)

APP1DEPN=	$(L)$/usr.lib
APP1DEF=	$(MISC)$/$(APP1TARGET).def

.ENDIF # testpgp

#
# std testcomponent
#
.IF "$(TESTAPP)" == "testcomponent"

CXXFILES=	testcomponent.cxx
OBJFILES=	$(OBJ)$/testcomponent.obj

APP2TARGET = testcomponent
APP2OBJS   = $(OBJ)$/testcomponent.obj
APP2STDLIBS = $(UNOLIB) \
              $(VOSLIB) \
              $(USRLIB) \
              $(TOOLSLIB)	\
              $(SALLIB) \
              $(RTLLIB) \
              $(ONELIB)

.ENDIF # testcomponent

#
# test python.
#
.IF "$(TESTAPP)" == "phytontest"

CXXFILES=	phytontest.cxx
OBJFILES=	$(OBJ)$/pythontest.obj

#APP4TARGET=	pythontest
#APP4OBJS=	$(OBJ)$/pythontest.obj 
#APP4STDLIBS=$(TOOLSLIB)		\
#			$(USRLIB)		\
#			$(UNOLIB)		\
#			$(VOSLIB)		\
#			$(RTLLIB)

#APP1DEPN=	$(LB)$/one.lib
#APP4DEF=	$(MISC)$/$(APP4TARGET).def

.ENDIF # phytontest

#
# python auto test
#
.IF "$(TESTAPP)" == "phytonautotest"

CXXFILES=	phytonautotest.cxx
OBJFILES=	$(OBJ)$/pythonautotest.obj

#APP5TARGET= pythonautotest
#APP5OBJS = $(OBJ)$/pythonautotest.obj
#APP5STDLIBS=$(TOOLSLIB)		\
#			$(USRLIB)		\
#			$(UNOLIB)		\
#			$(VOSLIB)		\
#			$(RTLLIB)
#APP5DEF=	$(MISC)$/$(APP5TARGET).def

.ENDIF # phytonautotest

#
# testresource.
#
.IF "$(TESTAPP)" == "testresource"

CXXFILES=	testresource.cxx
OBJFILES=	$(OBJ)$/testresource.obj

SRS2NAME =		testresource
SRC2FILES=		testresource.src
RESLIB2SRSFILES= $(SRS)$/testresource.srs
RESLIB2NAME=	testresource

APP2TARGET=	testresource
APP2OBJS=	$(OBJ)$/testresource.obj
APP2STDLIBS=$(TOOLSLIB)		\
            $(VCLLIB)		\
            $(ONELIB)		\
            $(USRLIB)		\
            $(UNOLIB)		\
            $(SALLIB) \
            $(RTLLIB)

APP2DEF=	$(MISC)$/$(APP3TARGET).def

.ENDIF # testresource

#
# testframecontrol.
#
.IF "$(TESTAPP)" == "testframecontrol"

CXXFILES=	testframecontrol.cxx
OBJFILES=	$(OBJ)$/testframecontrol.obj

APP3TARGET=	testframecontrol
APP3OBJS=	$(OBJ)$/testframecontrol.obj
APP3STDLIBS=$(TOOLSLIB)		\
            $(ONELIB)		\
            $(USRLIB)		\
            $(UNOLIB)		\
            $(VOSLIB)		\
            $(OSLLIB)		\
            $(RTLLIB)		\
            $(SVTOOLLIB)	\
            $(VCLLIB)

APP3DEF=	$(MISC)$/$(APP3TARGET).def

.ENDIF # testframecontrol

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
