#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.1.1.1 $
#
#   last change: $Author: hr $ $Date: 2000-09-18 15:17:30 $
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

PRJNAME=osl
TARGET=workben
LIBTARGET=NO
TARGETTYPE=CUI

TESTAPP=

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

# --- Settings -----------------------------------------------------

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk

# --- Files --------------------------------------------------------

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
SALTLSLIB=		isaltls.lib
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
SALZIPLIB=	isalzip.lib
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
.IF "$(OS)"=="SCO" || "$(OS)"=="NETBSD"

CXXFILES=	tgetpwnam.cxx
OBJFILES=	$(OBJ)$/tgetpwnam.obj

APP5TARGET= tgetpwnam
APP5OBJS=	$(OBJFILES)
APP5STDLIBS=$(SALLIB)
APP5DEPN=	$(SLB)$/sal.lib

.ENDIF # (sco | netbsd)
.ENDIF # tgetpwname

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk


