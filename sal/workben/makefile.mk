#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: mhu $ $Date: 2002-04-12 15:52:47 $
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

PRJNAME=sal
TARGET=workben
LIBTARGET=NO
TARGETTYPE=CUI

#TESTAPP=abbreviatetest
#TESTAPP=getlocaleinfotest
#TESTAPP=salstattest
#TESTAPP=saldyntest

TESTAPP=t_alloc
#TESTAPP=t_cipher
#TESTAPP=t_digest
#TESTAPP=t_random

#TESTAPP=testfile
#TESTAPP=testpipe
#TESTAPP=testpip2
#TESTAPP=testproc
#TESTAPP=tgetpwnam
#TESTAPP=salstattest

# --- Settings -----------------------------------------------------

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk

# --- Files --------------------------------------------------------

.IF "$(TESTAPP)" == "getlocaleinfotest"

    OBJFILES=$(OBJ)$/getlocaleinfotest.obj

    APP1TARGET=	getlitest
    APP1OBJS=$(OBJFILES)

    APP1STDLIBS=\
                kernel32.lib

    APP1LIBS=\
            $(LB)$/kernel9x.lib\
            $(LB)$/isal.lib

    APP1DEPN=$(LB)$/isal.lib

.ENDIF

.IF "$(TESTAPP)" == "abbreviatetest"

    OBJFILES=$(OBJ)$/abbreviatetest.obj

    APP1TARGET=	abbreviatetest
    APP1OBJS=$(OBJFILES)

    APP1STDLIBS=\
                kernel32.lib

    APP1LIBS=\
            $(LB)$/kernel9x.lib\
            $(LB)$/isal.lib

    APP1DEPN=$(LB)$/isal.lib

.ENDIF

.IF "$(TESTAPP)" == "salstattest"

    CFLAGS+= /DUSE_SAL_STATIC

    OBJFILES=	$(OBJ)$/salstattest.obj

    APP1TARGET=	salstattest
    APP1OBJS=	$(OBJFILES)
    APP1STDLIBS=\
                $(LB)$/asal.lib\
                shell32.lib\
                user32.lib\
                comdlg32.lib\
                advapi32.lib

    APP1DEPN=	$(LB)$/asal.lib	

.ENDIF # salstattest

.IF "$(TESTAPP)" == "saldyntest"

    OBJFILES=	$(OBJ)$/saldyntest.obj

    APP1TARGET=	saldyntest
    APP1OBJS=	$(OBJFILES)
    APP1STDLIBS=\
                $(LB)$/isal.lib\
                shell32.lib\
                user32.lib\
                comdlg32.lib\
                advapi32.lib

    APP1DEPN=	$(LB)$/isal.lib

.ENDIF # salstattest

#
# t_alloc.
#
.IF "$(TESTAPP)" == "t_alloc"

CFILES=		t_alloc.c
OBJFILES=	$(OBJ)$/t_alloc.obj

APP1TARGET=	t_alloc
APP1OBJS=	$(OBJFILES)
APP1STDLIBS=$(SALLIB)
APP1DEPN=	$(SLB)$/sal.lib

.ENDIF # t_layer

#
# t_cipher
#
.IF "$(TESTAPP)" == "t_cipher"

CFILES=		t_cipher.c
OBJFILES=	$(OBJ)$/t_cipher.obj

APP1TARGET=	t_cipher
APP1OBJS=	$(OBJFILES)
APP1STDLIBS=$(SALLIB)
APP1DEPN=	$(SLB)$/sal.lib

.ENDIF # t_cipher

#
# t_digest.
#
.IF "$(TESTAPP)" == "t_digest"

CFILES=		t_digest.c
OBJFILES=	$(OBJ)$/t_digest.obj

APP1TARGET=	t_digest
APP1OBJS=	$(OBJFILES)
APP1STDLIBS=$(SALLIB)
APP1DEPN=	$(SLB)$/sal.lib

.ENDIF # t_digest

#
# t_random.
#
.IF "$(TESTAPP)" == "t_random"

CFILES=		t_random.c
OBJFILES=	$(OBJ)$/t_random.obj

APP1TARGET=	t_random
APP1OBJS=	$(OBJFILES)
APP1STDLIBS=$(SALLIB)
APP1DEPN=	$(SLB)$/sal.lib

.ENDIF # t_random

#
# testfile
#
.IF "$(TESTAPP)" == "testfile"

CXXFILES=	testfile.cxx
OBJFILES=	$(OBJ)$/testfile.obj

APP1TARGET=	testfile
APP1OBJS=	$(OBJFILES)
APP1STDLIBS=$(SALLIB)
APP1DEPN=	$(SLB)$/sal.lib

.ENDIF # testfile

#
# testpipe
#
.IF "$(TESTAPP)" == "testpipe"

CXXFILES=	testpipe.cxx
OBJFILES=	$(OBJ)$/testpipe.obj

APP1TARGET= testpipe
APP1OBJS=   $(OBJFILES)
APP1STDLIBS=$(SALLIB)
APP1DEPN=	$(SLB)$/sal.lib

.ENDIF # testpipe

#
# testpip2
#
.IF "$(TESTAPP)" == "testpip2"

CXXFILES=	testpip2.cxx
OBJFILES=	$(OBJ)$/testpip2.obj

APP1TARGET=	testpip2
APP1OBJS=	$(OBJFILES)
APP1STDLIBS=$(SALLIB)
APP1DEPN=	$(SLB)$/sal.lib

.ENDIF # testpip2

#
# testproc
#
.IF "$(TESTAPP)" == "testproc"

CXXFILES=	testproc.cxx
OBJFILES=	$(OBJ)$/testproc.obj

APP1TARGET= testproc
APP1OBJS=	$(OBJFILES)
APP1STDLIBS=$(SALLIB)
APP1DEPN=	$(SLB)$/sal.lib

.ENDIF # testproc

#
# tgetpwnam
#
.IF "$(TESTAPP)" == "tgetpwnam"
.IF "$(OS)"=="SCO" || "$(OS)"=="NETBSD"

CXXFILES=	tgetpwnam.cxx
OBJFILES=	$(OBJ)$/tgetpwnam.obj

APP1TARGET= tgetpwnam
APP1OBJS=	$(OBJFILES)
APP1STDLIBS=$(SALLIB)
APP1DEPN=	$(SLB)$/sal.lib

.ENDIF # (sco | netbsd)
.ENDIF # tgetpwname

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk


