#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: john.marmion $ $Date: 2000-09-26 14:05:51 $
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

PRJNAME=BASIC
TARGET=app

# --- Settings ------------------------------------------------------------

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk

# --- Allgemein ------------------------------------------------------------

.INCLUDE :  minor.mk
RSCUPDVER=$(RSCREVISION)SV$(UPD)$(UPDMINOR)


CXXFILES = \
    ttstrhlp.cxx			\
    ttbasic.cxx				\
    basicrt.cxx				\
    processw.cxx            \
    process.cxx             \
    brkpnts.cxx             \
    app.cxx                 \
    mybasic.cxx             \
    status.cxx              \
    printer.cxx             \
    appwin.cxx              \
    appedit.cxx             \
    appbased.cxx            \
    apperror.cxx            \
    textedit.cxx            \
    msgedit.cxx             \
    dialogs.cxx             \
    comm_bas.cxx            \
    testbasi.cxx            


OBJFILES = \
    $(OBJ)$/ttstrhlp.obj	\
    $(OBJ)$/ttbasic.obj	\
    $(OBJ)$/basicrt.obj	\
    $(OBJ)$/processw.obj    \
    $(OBJ)$/process.obj     \
    $(OBJ)$/brkpnts.obj     \
    $(OBJ)$/app.obj         \
    $(OBJ)$/mybasic.obj     \
    $(OBJ)$/status.obj      \
    $(OBJ)$/printer.obj     \
    $(OBJ)$/appwin.obj      \
    $(OBJ)$/appedit.obj     \
    $(OBJ)$/appbased.obj    \
    $(OBJ)$/apperror.obj    \
    $(OBJ)$/textedit.obj    \
    $(OBJ)$/msgedit.obj     \
    $(OBJ)$/dialogs.obj     \
    $(OBJ)$/comm_bas.obj	\
    $(SLO)$/sbintern.obj


SRCFILES = \
    basic.src				\
    ttmsg.src				\
    basmsg.src				\
    svtmsg.src				\
    testtool.src

LIBTARGET = NO

LIB1TARGET=$(LB)$/app.lib
LIB1ARCHIV=$(LB)$/libapp.a
LIB1OBJFILES = \
        $(OBJ)$/ttstrhlp.obj	\
        $(OBJ)$/basicrt.obj	\
        $(OBJ)$/processw.obj     \
              $(OBJ)$/process.obj      \
              $(OBJ)$/brkpnts.obj      \
              $(OBJ)$/app.obj          \
        $(OBJ)$/mybasic.obj	\
              $(OBJ)$/status.obj       \
              $(OBJ)$/printer.obj      \
              $(OBJ)$/appwin.obj       \
              $(OBJ)$/appedit.obj      \
              $(OBJ)$/appbased.obj     \
              $(OBJ)$/apperror.obj     \
              $(OBJ)$/textedit.obj     \
              $(OBJ)$/msgedit.obj      \
              $(OBJ)$/dialogs.obj      \
              $(OBJ)$/comm_bas.obj	\
        $(SLO)$/sbintern.obj

.IF "$(depend)" == ""

ALL:    $(LB)$/app.lib  \
        ALLTAR

.ENDIF

# --- Targets ------------------------------------------------------------

.INCLUDE :  target.mk
