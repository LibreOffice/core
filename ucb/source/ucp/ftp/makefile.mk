#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.11 $
#
#   last change: $Author: vg $ $Date: 2003-07-25 11:38:46 $
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

PRJ=..$/..$/..
PRJNAME=ucb
TARGET=ucpftp
ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE
NO_BSYMBOLIC=TRUE

# Version
UCPFTP_MAJOR=1


# --- Settings ---------------------------------------------------------

.INCLUDE: settings.mk

#CFLAGS +=-fno-inline

# --- General -----------------------------------------------------

# first target ( shared library )

SLOFILES1=\
    $(SLO)$/ftpservices.obj  \
    $(SLO)$/ftpcontentprovider.obj  \
    $(SLO)$/ftpcontent.obj   \
    $(SLO)$/ftpcontentidentifier.obj   \
    $(SLO)$/ftpcontentcaps.obj \
    $(SLO)$/ftpdynresultset.obj  \
    $(SLO)$/ftpresultsetbase.obj \
    $(SLO)$/ftpresultsetI.obj \
    $(SLO)$/ftploaderthread.obj  \
    $(SLO)$/ftpdownloadthread.obj  \
    $(SLO)$/ftpinpstr.obj	\
    $(SLO)$/ftpdirp.obj     \
    $(SLO)$/ftpcfunc.obj     \
    $(SLO)$/ftpurl.obj     \
    $(SLO)$/ftpintreq.obj     \
    $(SLO)$/debughelper.obj

LIB1TARGET=$(SLB)$/_$(TARGET).lib
LIB1OBJFILES=$(SLOFILES1)

# --- Shared-Library 1 ---------------------------------------------------

SHL1TARGET=$(TARGET)$(UCPFTP_MAJOR)
SHL1IMPLIB=i$(TARGET)
.IF "$(OS)"=="MACOSX"
.ELSE
SHL1VERSIONMAP=	$(TARGET).map
.ENDIF

SHL1STDLIBS=\
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(SALLIB)  \
    $(UCBHELPERLIB) \
    $(CURLLIB)

SHL1DEF=$(MISC)$/$(SHL1TARGET).def
SHL1LIBS= \
    $(LIB1TARGET)

# Make symbol renaming match library name for Mac OS X
.IF "$(OS)"=="MACOSX"
SYMBOLPREFIX=$(TARGET)$(UCPFTP_MAJOR)
.ENDIF

# --- Def-File ---------------------------------------------------------

DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=	$(TARGET).dxp
DEF1DES=UCB Ftp Content Provider

.IF "$(COMPHELPERLIB)"==""
.IF "$(GUI)" == "UNX"
COMPHELPERLIB=-licomphelp2
.ENDIF # unx
.IF "$(GUI)"=="WNT"
COMPHELPERLIB=icomphelp2.lib
.ENDIF # wnt
.ENDIF

.INCLUDE: target.mk













