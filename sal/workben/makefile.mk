#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

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
# measure_oustrings
#
OBJFILES+=\
    $(OBJ)$/measure_oustrings.obj

.IF "$(ENABLE_VALGRIND)" == "TRUE"
CFLAGSCXX+=-DHAVE_VALGRIND_HEADERS
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

.IF "$(OS)" == "WNT"
APP10STDLIBS=$(KERNEL32LIB)
SHL10STDLIBS=$(LB)$/isal.lib
SHL10STDLIBS+=$(SOLARLIBDIR)$/cppunit.lib
.ENDIF

.IF "$(OS)" != "WNT"
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

.IF "$(OS)" != "WNT"
APP10STDLIBS=$(LB)$/libsal.so
.ENDIF

SHL10OBJS=$(SLO)$/t_osl_joinProcess.obj
SHL10TARGET=tojp

.IF "$(OS)" == "WNT"
APP10STDLIBS=$(KERNEL32LIB)
SHL10STDLIBS=$(LB)$/isal.lib
SHL10STDLIBS+=$(SOLARLIBDIR)$/cppunit.lib
.ENDIF

.IF "$(OS)" != "WNT"
SHL10STDLIBS=$(LB)$/libsal.so
SHL10STDLIBS+=$(SOLARLIBDIR)$/libcppunit$(DLLPOSTFIX).a
.ENDIF

SHL10IMPLIB=i$(SHL1TARGET)
SHL10DEF=$(MISC)$/$(SHL1TARGET).def
DEF10NAME=$(SHL1TARGET)
DEF10EXPORTFILE=export.exp
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
