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
NO_DEFAULT_STL=YES


# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

CFLAGS+=$(LFS_CFLAGS)
CXXFLAGS+=$(LFS_CFLAGS)

# --- Files --------------------------------------------------------

#
# t_readline
#
OBJFILES+=\
    $(OBJ)$/t_readline.obj

APP1TARGET=t_readline
APP1OBJS=$(OBJ)$/t_readline.obj
APP1STDLIBS=$(SALLIB)
APP1DEPN=$(SLB)$/sal.lib
APP1RPATH=UREBIN

#
# testfile
#
OBJFILES+=\
    $(OBJ)$/testfile.obj

APP1TARGET=testfile
APP1OBJS=$(OBJ)$/testfile.obj
APP1STDLIBS=$(SALLIB)
APP1DEPN=$(SLB)$/sal.lib

#
# testpipe
#
OBJFILES+=\
    $(OBJ)$/testpipe.obj

APP2TARGET=testpipe
APP2OBJS=$(OBJ)$/testpipe.obj
APP2STDLIBS=$(SALLIB)
APP2DEPN=$(SLB)$/sal.lib

#
# testpip2
#
OBJFILES+=\
    $(OBJ)$/testpip2.obj

APP3TARGET=testpip2
APP3OBJS=$(OBJ)$/testpip2.obj
APP3STDLIBS=$(SALLIB)
APP3DEPN=$(SLB)$/sal.lib

#
# testproc
#
OBJFILES+=\
    $(OBJ)$/testproc.obj

APP4TARGET=testproc
APP4OBJS=$(OBJ)$/testproc.obj
APP4STDLIBS=$(SALLIB)
APP4DEPN=$(SLB)$/sal.lib

#
# tgetpwnam
#
.IF "$(OS)"=="NETBSD" || "$(OS)"=="FREEBSD" || "$(OS)"=="OPENBSD"
OBJFILES+=\
    $(OBJ)$/tgetpwnam.obj

APP5TARGET=tgetpwnam
APP5OBJS=$(OBJ)$/tgetpwnam.obj
APP5STDLIBS=$(SALLIB)
APP5DEPN=$(SLB)$/sal.lib
.ENDIF # (netbsd | freebsd)

#
# measure_oustrings
#
OBJFILES+=\
    $(OBJ)$/measure_oustrings.obj

.IF "$(HAVE_CALLGRIND)" == "YES"
CFLAGSCXX+=-DHAVE_CALLGRIND
.ENDIF

APP6TARGET=measure_oustrings
APP6OBJS=$(OBJ)$/measure_oustrings.obj
APP6STDLIBS=$(SALLIB)
APP6DEPN=$(SLB)$/sal.lib

#
# t_random.
#
OBJFILES+=\
    $(OBJ)$/t_random.obj

APP7TARGET=t_random
APP7OBJS=$(OBJ)$/t_random.obj
APP7STDLIBS=$(SALLIB)
APP7DEPN=$(SLB)$/sal.lib

#
# t_cipher
#
OBJFILES+=\
    $(OBJ)$/t_cipher.obj

APP8TARGET=t_cipher
APP8OBJS=$(OBJ)$/t_cipher.obj
APP8STDLIBS=$(SALLIB)
APP8DEPN=$(SLB)$/sal.lib

#
# t_digest.
#
OBJFILES+=\
    $(OBJ)$/t_digest.obj

APP9TARGET=t_digest
APP9OBJS=$(OBJ)$/t_digest.obj
APP9STDLIBS=$(SALLIB)
APP9DEPN=$(SLB)$/sal.lib

#
# getlitest
#
.IF "$(TESTAPP)" == "getlocaleinfotest"
OBJFILES+=\
    $(OBJ)$/getlocaleinfotest.obj

APP10TARGET=getlitest
APP10OBJS=$(OBJ)$/getlocaleinfotest.obj

APP10STDLIBS=\
    $(KERNEL32LIB)

APP10LIBS=\
    $(LB)$/kernel9x.lib\
    $(LB)$/isal.lib

APP10DEPN=$(LB)$/isal.lib
.ENDIF

.IF "$(TESTAPP)" == "test_osl_getVolInfo"

SHL10OBJS=$(SLO)$/t_osl_getVolInfo.obj
SHL10TARGET=togvi

.IF "$(GUI)" == "WNT"
APP10STDLIBS=$(KERNEL32LIB)
SHL10STDLIBS=$(LB)$/isal.lib
SHL10STDLIBS+=$(SOLARLIBDIR)$/cppunit.lib
.ENDIF

.IF "$(GUI)" == "UNX"
SHL10STDLIBS=$(LB)$/libsal.so
SHL10STDLIBS+=$(SOLARLIBDIR)$/libcppunit$(DLLPOSTFIX).a
.ENDIF

SHL10IMPLIB=i$(SHL1TARGET)
SHL10DEF=$(MISC)$/$(SHL1TARGET).def
DEF10NAME=$(SHL1TARGET)
DEF10EXPORTFILE=export.exp
.ENDIF

.IF "$(TESTAPP)" == "test_osl_joinProcess"
OBJFILES+=\
    $(OBJ)$/t_ojp_exe.obj
APP10TARGET=ojpx
APP10OBJS=$(OBJ)$/t_ojp_exe.obj

.IF "$(GUI)" == "UNX"
APP10STDLIBS=$(LB)$/libsal.so
.ENDIF

SHL10OBJS=$(SLO)$/t_osl_joinProcess.obj
SHL10TARGET=tojp

.IF "$(GUI)" == "WNT"
APP10STDLIBS=$(KERNEL32LIB)
SHL10STDLIBS=$(LB)$/isal.lib
SHL10STDLIBS+=$(SOLARLIBDIR)$/cppunit.lib
.ENDIF

.IF "$(GUI)" == "UNX"
SHL10STDLIBS=$(LB)$/libsal.so
SHL10STDLIBS+=$(SOLARLIBDIR)$/libcppunit$(DLLPOSTFIX).a
.ENDIF

SHL10IMPLIB=i$(SHL1TARGET)
SHL10DEF=$(MISC)$/$(SHL1TARGET).def
DEF10NAME=$(SHL1TARGET)
DEF10EXPORTFILE=export.exp
.ENDIF

.IF "$(TESTAPP)" == "salstattest"

    CFLAGS+= -DUSE_SAL_STATIC

    OBJFILES+=$(OBJ)$/salstattest.obj

    APP10TARGET=salstattest
    APP10OBJS=$(OBJ)$/salstattest.obj
    APP10STDLIBS=\
                $(LB)$/asal.lib\
                $(SHELL32LIB)\
                $(USER32LIB)\
                $(COMDLG32LIB)\
                $(ADVAPI32LIB)

    APP10DEPN=$(LB)$/asal.lib

.ENDIF # salstattest

.IF "$(TESTAPP)" == "saldyntest"

    OBJFILES+=$(OBJ)$/saldyntest.obj

    APP10TARGET=saldyntest
    APP10OBJS=$(OBJ)$/saldyntest.obj
    APP10STDLIBS=\
                $(LB)$/isal.lib\
                $(SHELL32LIB)\
                $(USER32LIB)\
                $(COMDLG32LIB)\
                $(ADVAPI32LIB)

    APP10DEPN=$(LB)$/isal.lib

.ENDIF # salstattest



# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
