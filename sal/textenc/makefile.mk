#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.9 $
#
#   last change: $Author: hr $ $Date: 2003-03-26 16:47:12 $
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
#*************************************************************************

PRJ = ..
PRJNAME = sal
TARGET = textenc

.INCLUDE: settings.mk

SLOFILES = \
    $(SLO)$/context.obj \
    $(SLO)$/convertbig5hkscs.obj \
    $(SLO)$/converter.obj \
    $(SLO)$/converteuctw.obj \
    $(SLO)$/convertgb18030.obj \
    $(SLO)$/convertiso2022cn.obj \
    $(SLO)$/convertiso2022jp.obj \
    $(SLO)$/convertiso2022kr.obj \
    $(SLO)$/tcvtbyte.obj \
    $(SLO)$/tcvtmb.obj \
    $(SLO)$/tcvtutf7.obj \
    $(SLO)$/tcvtutf8.obj \
    $(SLO)$/tenchelp.obj \
    $(SLO)$/tencinfo.obj \
    $(SLO)$/textcvt.obj \
    $(SLO)$/textenc.obj \
    $(SLO)$/unichars.obj

#.IF "$(UPDATER)" != ""

OBJFILES = \
    $(OBJ)$/context.obj \
    $(OBJ)$/convertbig5hkscs.obj \
    $(OBJ)$/converter.obj \
    $(OBJ)$/converteuctw.obj \
    $(OBJ)$/convertgb18030.obj \
    $(OBJ)$/convertiso2022cn.obj \
    $(OBJ)$/convertiso2022jp.obj \
    $(OBJ)$/convertiso2022kr.obj \
    $(OBJ)$/tcvtbyte.obj \
    $(OBJ)$/tcvtmb.obj \
    $(OBJ)$/tcvtutf7.obj \
    $(OBJ)$/tcvtutf8.obj \
    $(OBJ)$/tenchelp.obj \
    $(OBJ)$/tencinfo.obj \
    $(OBJ)$/textcvt.obj \
    $(OBJ)$/textenc.obj \
    $(OBJ)$/unichars.obj

#.ENDIF # UPDATER

# Optimization off on Solaris Intel due to internal compiler error; to be
# reevaluated after compiler upgrade:
.IF "$(OS)$(CPU)" == "SOLARISI"

NOOPTFILES = \
    $(OBJ)$/textenc.obj \
    $(SLO)$/textenc.obj

.ENDIF # OS, CPU, SOLARISI

.INCLUDE: target.mk
