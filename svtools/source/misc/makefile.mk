#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: jp $ $Date: 2000-09-27 09:38:32 $
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

PRJ=..$/..

PRJNAME=			svtools
TARGET=				misc
LIBTARGET=			NO
USE_LDUMP2=			TRUE
USE_DEFFILE=		TRUE
ENABLE_EXCEPTIONS=	TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk

.IF "$(GUI)"=="MAC"
CFLAGS += -nomapcr
.ENDIF

# --- Files --------------------------------------------------------

SRS1NAME=misc
SRC1FILES=\
    config.src	\
    demo.src	\
    pver.src	\
    iniman.src 	\
    ehdl.src

SRS2NAME=ehdl
SRC2FILES=\
    errtxt.src

SRS3NAME=mediatyp
SRC3FILES=\
    mediatyp.src

SLOFILES=\
    $(LIB2OBJFILES)	\
    $(LIB3OBJFILES) \
    $(SLO)$/svtdata.obj

OBJFILES=\
    $(OBJ)$/filearch.obj	\
    $(OBJ)$/svtdata.obj

LIB2TARGET=$(SLB)$/misc1.lib
LIB2OBJFILES=\
    $(SLO)$/adrparse.obj \
    $(SLO)$/inethist.obj \
    $(SLO)$/inettype.obj \
    $(SLO)$/iniadrtk.obj \
    $(SLO)$/loginerr.obj \
    $(SLO)$/strcrypt.obj \
    $(SLO)$/strmadpt.obj

LIB3TARGET=$(SLB)$/misc2.lib
LIB3OBJFILES=\
    $(SLO)$/agprop.obj \
    $(SLO)$/config.obj \
    $(SLO)$/confitem.obj \
    $(SLO)$/demo.obj \
    $(SLO)$/ehdl.obj \
    $(SLO)$/filearch.obj \
    $(SLO)$/flbytes.obj \
    $(SLO)$/fstathelper.obj \
    $(SLO)$/imap.obj \
    $(SLO)$/imap2.obj \
    $(SLO)$/imap3.obj \
    $(SLO)$/iniman.obj \
    $(SLO)$/iniprop.obj \
    $(SLO)$/ownlist.obj \
    $(SLO)$/pver.obj \
    $(SLO)$/urihelper.obj \
    $(SLO)$/vcldata.obj

EXCEPTIONSFILES=\
    $(SLO)$/strmadpt.obj	\
    $(SLO)$/iniman.obj

UNOUCRDEP=$(SOLARBINDIR)$/applicat.rdb
UNOUCRRDB=$(SOLARBINDIR)$/applicat.rdb
UNOUCROUT=$(OUT)$/inc
UNOTYPES=\
    com.sun.star.io.XInputStream					\
    com.sun.star.io.XOutputStream					\
    com.sun.star.io.XSeekable						\
    com.sun.star.uno.TypeClass						\
    com.sun.star.uno.XWeak							\
    com.sun.star.lang.XMultiServiceFactory			\
    com.sun.star.lang.XSingleServiceFactory			\
    com.sun.star.registry.MergeConflictException	\
    com.sun.star.registry.XSimpleRegistry			\
    com.sun.star.frame.XConfigManager


# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk
