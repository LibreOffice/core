#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
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

PRJ=..

PRJNAME=sal
TARGET=workben
LIBTARGET=NO
TARGETTYPE=CUI
#ENABLE_EXCEPTIONS=TRUE
NO_DEFAULT_STL=YES

#TESTAPP=test_osl_getVolInfo
#TESTAPP=test_osl_joinProcess
#TESTAPP=getlocaleinfotest
#TESTAPP=test_osl_joinProcess
#TESTAPP=getlocaleinfotest
#TESTAPP=salstattest
#TESTAPP=saldyntest

TESTAPP=t_readline
#TESTAPP=t_cipher
#TESTAPP=t_digest
#TESTAPP=t_random
#TESTAPP=t_layer
#TESTAPP=t_tls
#TESTAPP=t_zip

#TESTAPP=testfile
#TESTAPP=testpipe
#TESTAPP=testpip2
#TESTAPP=testproc
#TESTAPP=tgetpwnam
#TESTAPP=salstattest

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

CFLAGS+= $(LFS_CFLAGS)
CXXFLAGS+= $(LFS_CFLAGS)

# --- Files --------------------------------------------------------

.IF "$(TESTAPP)" == "test_osl_getVolInfo"    

SHL1OBJS=$(SLO)$/t_osl_getVolInfo.obj
SHL1TARGET=togvi

.IF "$(GUI)" == "WNT"
APP1STDLIBS=$(KERNEL32LIB)
SHL1STDLIBS=$(LB)$/isal.lib
SHL1STDLIBS+=$(SOLARLIBDIR)$/cppunit.lib
.ENDIF

.IF "$(GUI)" == "UNX"
SHL1STDLIBS=$(LB)$/libsal.so
SHL1STDLIBS+=$(SOLARLIBDIR)$/libcppunit$(DLLPOSTFIX).a
.ENDIF
    
SHL1IMPLIB=i$(SHL1TARGET) 
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET) 
DEF1EXPORTFILE=export.exp
.ENDIF

.IF "$(TESTAPP)" == "test_osl_joinProcess"    
OBJFILES=$(OBJ)$/t_ojp_exe.obj 
APP1TARGET=ojpx
APP1OBJS=$(OBJFILES)

.IF "$(GUI)" == "UNX"
APP1STDLIBS=$(LB)$/libsal.so
.ENDIF

SHL1OBJS=$(SLO)$/t_osl_joinProcess.obj
SHL1TARGET=tojp

.IF "$(GUI)" == "WNT"
APP1STDLIBS=$(KERNEL32LIB)
SHL1STDLIBS=$(LB)$/isal.lib
SHL1STDLIBS+=$(SOLARLIBDIR)$/cppunit.lib
.ENDIF

.IF "$(GUI)" == "UNX"
SHL1STDLIBS=$(LB)$/libsal.so
SHL1STDLIBS+=$(SOLARLIBDIR)$/libcppunit$(DLLPOSTFIX).a
.ENDIF
    
SHL1IMPLIB=i$(SHL1TARGET) 
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET) 
DEF1EXPORTFILE=export.exp
.ENDIF

.IF "$(TESTAPP)" == "getlocaleinfotest"

    OBJFILES=$(OBJ)$/getlocaleinfotest.obj

    APP1TARGET=	getlitest
    APP1OBJS=$(OBJFILES)

    APP1STDLIBS=\
                $(KERNEL32LIB)

    APP1LIBS=\
            $(LB)$/kernel9x.lib\
            $(LB)$/isal.lib

    APP1DEPN=$(LB)$/isal.lib

.ENDIF

.IF "$(TESTAPP)" == "salstattest"

    CFLAGS+= -DUSE_SAL_STATIC

    OBJFILES=	$(OBJ)$/salstattest.obj

    APP1TARGET=	salstattest
    APP1OBJS=	$(OBJFILES)
    APP1STDLIBS=\
                $(LB)$/asal.lib\
                $(SHELL32LIB)\
                $(USER32LIB)\
                $(COMDLG32LIB)\
                $(ADVAPI32LIB)

    APP1DEPN=	$(LB)$/asal.lib	

.ENDIF # salstattest

.IF "$(TESTAPP)" == "saldyntest"

    OBJFILES=	$(OBJ)$/saldyntest.obj

    APP1TARGET=	saldyntest
    APP1OBJS=	$(OBJFILES)
    APP1STDLIBS=\
                $(LB)$/isal.lib\
                $(SHELL32LIB)\
                $(USER32LIB)\
                $(COMDLG32LIB)\
                $(ADVAPI32LIB)

    APP1DEPN=	$(LB)$/isal.lib

.ENDIF # salstattest

#
# t_readline
#
.IF "$(TESTAPP)" == "t_readline"

CFILES=		t_readline.c
OBJFILES=	$(OBJ)$/t_readline.obj

APP1TARGET=	t_readline
APP1OBJS=	$(OBJFILES)
APP1STDLIBS=$(SALLIB)
APP1DEPN=	$(SLB)$/sal.lib

APP1RPATH=UREBIN

.ENDIF # t_readline

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

APP2TARGET=	t_digest
APP2OBJS=	$(OBJFILES)
APP2STDLIBS=$(SALLIB)
APP2DEPN=	$(SLB)$/sal.lib

.ENDIF # t_digest

#
# t_random.
#
.IF "$(TESTAPP)" == "t_random"

CFILES=		t_random.c
OBJFILES=	$(OBJ)$/t_random.obj

APP3TARGET=	t_random
APP3OBJS=	$(OBJFILES)
APP3STDLIBS=$(SALLIB)
APP3DEPN=	$(SLB)$/sal.lib

.ENDIF # t_random

#
# t_layer.
#
.IF "$(TESTAPP)" == "t_layer"

CFILES=		t_layer.c
OBJFILES=	$(OBJ)$/t_layer.obj

APP4TARGET=	t_layer
APP4OBJS=	$(OBJFILES)
APP4STDLIBS=$(SALLIB)
APP4DEPN=	$(SLB)$/sal.lib

.ENDIF # t_layer

#
# t_tls.
#
.IF "$(TESTAPP)" == "t_tls"

CFILES=		t_tls.c
OBJFILES=	$(OBJ)$/t_tls.obj

.IF "$(SALTLSLIB)" == ""

.IF "$(GUI)" == "UNX"
SALTLSLIB=		-lsaltls2
.ENDIF # unx

.IF "$(GUI)" == "WNT"
SALTLSLIB=		$(SALTLSLIB)
.ENDIF # wnt

.ENDIF # saltlslib

APP5TARGET=	t_tls
APP5OBJS=	$(OBJFILES)
APP5STDLIBS=$(SALTLSLIB) $(SALLIB)
APP5DEPN=	$(SLB)$/sal.lib

.ENDIF # t_tls

#
# t_zip.
#
.IF "$(TESTAPP)" == "t_zip"

CFILES=		t_zip.c
OBJFILES=	$(OBJ)$/t_zip.obj

.IF "$(SALZIPLIB)" == ""

.IF "$(GUI)" == "UNX"
SALZIPLIB=	-lsalzip2
.ENDIF # unx

.IF "$(GUI)" == "WNT"
SALZIPLIB=	$(SALZIPLIB)
.ENDIF # wnt

.ENDIF # salziplib

APP6TARGET=	t_zip
APP6OBJS=	$(OBJFILES)
APP6STDLIBS=$(SALZIPLIB) $(SALLIB)
APP6DEPN=	$(SLB)$/sal.lib

.ENDIF # t_zip

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

APP2TARGET= testpipe
APP2OBJS=   $(OBJFILES)
APP2STDLIBS=$(SALLIB)
APP2DEPN=	$(SLB)$/sal.lib

.ENDIF # testpipe

#
# testpip2
#
.IF "$(TESTAPP)" == "testpip2"

CXXFILES=	testpip2.cxx
OBJFILES=	$(OBJ)$/testpip2.obj

APP3TARGET=	testpip2
APP3OBJS=	$(OBJFILES)
APP3STDLIBS=$(SALLIB)
APP3DEPN=	$(SLB)$/sal.lib

.ENDIF # testpip2

#
# testproc
#
.IF "$(TESTAPP)" == "testproc"

CXXFILES=	testproc.cxx
OBJFILES=	$(OBJ)$/testproc.obj

APP4TARGET= testproc
APP4OBJS=	$(OBJFILES)
APP4STDLIBS=$(SALLIB)
APP4DEPN=	$(SLB)$/sal.lib

.ENDIF # testproc

#
# tgetpwnam
#
.IF "$(TESTAPP)" == "tgetpwnam"
.IF "$(OS)"=="SCO" || "$(OS)"=="NETBSD" || "$(OS)"=="FREEBSD" || \
	"$(OS)"=="OPENBSD"

CXXFILES=	tgetpwnam.cxx
OBJFILES=	$(OBJ)$/tgetpwnam.obj

APP5TARGET= tgetpwnam
APP5OBJS=	$(OBJFILES)
APP5STDLIBS=$(SALLIB)
APP5DEPN=	$(SLB)$/sal.lib

.ENDIF # (sco | netbsd | freebsd)
.ENDIF # tgetpwname

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk


