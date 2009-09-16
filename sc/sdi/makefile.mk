#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.11 $
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

PRJNAME=sc
TARGET=scslots
SVSDIINC=$(PRJ)$/source$/ui$/inc

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

.IF "$(L10N_framework)"==""
SDI1NAME=$(TARGET)
#SIDHRCNAME=SCSLOTS.HRC
SDI1EXPORT=scalc

SVSDI1DEPEND= \
    scalc.sdi \
    app.sdi \
    auditsh.sdi \
    docsh.sdi \
    drawsh.sdi \
    drtxtob.sdi \
    drformsh.sdi \
    editsh.sdi \
    pivotsh.sdi \
    prevwsh.sdi \
    tabvwsh.sdi \
    formatsh.sdi\
    cellsh.sdi  \
    oleobjsh.sdi\
    chartsh.sdi \
    graphsh.sdi \
    mediash.sdi \
    pgbrksh.sdi \
    scslots.hrc \
    $(INC)$/sc.hrc
.ENDIF

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk
