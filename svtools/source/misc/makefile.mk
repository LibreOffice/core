#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.28 $
#
#   last change: $Author: rt $ $Date: 2004-07-23 10:47:51 $
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
ENABLE_EXCEPTIONS=	TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

#use local "bmp" as it may not yet be delivered

SRS1NAME=misc
SRC1FILES=\
    config.src	\
    iniman.src 	\
    ehdl.src \
    imagemgr.src      \
    helpagent.src

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
    $(OBJ)$/svtdata.obj

LIB2TARGET=$(SLB)$/misc1.lib
LIB2OBJFILES=\
    $(SLO)$/adrparse.obj \
    $(SLO)$/inethist.obj \
    $(SLO)$/inettype.obj \
    $(SLO)$/iniadrtk.obj \
    $(SLO)$/loginerr.obj \
    $(SLO)$/strcrypt.obj \
    $(SLO)$/strmadpt.obj \
    $(SLO)$/PasswordHelper.obj

LIB3TARGET=$(SLB)$/misc2.lib
LIB3OBJFILES=\
    $(SLO)$/filenotation.obj    \
    $(SLO)$/productregistration.obj	\
    $(SLO)$/templatefoldercache.obj	\
    $(SLO)$/helpagentwindow.obj		\
    $(SLO)$/imagemgr.obj			\
    $(SLO)$/ehdl.obj		\
    $(SLO)$/flbytes.obj		\
    $(SLO)$/fstathelper.obj \
    $(SLO)$/imap.obj		\
    $(SLO)$/imap2.obj		\
    $(SLO)$/imap3.obj		\
    $(SLO)$/lingucfg.obj	\
    $(SLO)$/lngmisc.obj		\
    $(SLO)$/ownlist.obj		\
    $(SLO)$/pver.obj		\
    $(SLO)$/urihelper.obj	\
    $(SLO)$/vcldata.obj		\
    $(SLO)$/transfer.obj	\
    $(SLO)$/transfer2.obj	\
    $(SLO)$/cliplistener.obj \
    $(SLO)$/stringtransfer.obj \
    $(SLO)$/graphictools.obj \
    $(SLO)$/imageresourceaccess.obj

EXCEPTIONSFILES=\
    $(SLO)$/productregistration.obj	\
    $(SLO)$/templatefoldercache.obj	\
    $(SLO)$/strmadpt.obj			\
    $(SLO)$/transfer.obj			\
    $(SLO)$/transfer2.obj			\
    $(SLO)$/cliplistener.obj		\
    $(SLO)$/stringtransfer.obj		\
    $(SLO)$/imagemgr.obj

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk




