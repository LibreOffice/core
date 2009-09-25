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

# wx mktyplib nur unter Windows aufrufbar
# VMB.386 und WXSRVR.EXE muessen geladen sein! (n:\mscwini\bin)

PRJ=..

PRJNAME=sw
TARGET=swslots

# --- Settings -----------------------------------------------------
.IF "$(L10N_framework)"==""
SVSDIINC=$(PRJ)$/source$/ui$/inc
.ENDIF

.INCLUDE :  $(PRJ)$/inc$/swpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/inc$/sw.mk

.IF "$(L10N_framework)"==""
SDI1NAME=$(TARGET)
SDI1EXPORT=swriter
#SIDHRCNAME=swslots.hrc

# --- Files --------------------------------------------------------
SVSDI1DEPEND= \
        $(SOLARINCXDIR)$/sfx2/sfx.sdi \
        $(SOLARINCXDIR)$/sfx2/sfxitems.sdi \
        $(SOLARINCXDIR)$/svx/svx.sdi \
        $(SOLARINCXDIR)$/svx/svxitems.sdi \
        $(SOLARINCXDIR)$/svx/xoitems.sdi \
                switems.sdi\
        swriter.sdi\
        _docsh.sdi\
        docsh.sdi\
        wdocsh.sdi\
        wrtapp.sdi\
        _basesh.sdi\
        basesh.sdi\
        _textsh.sdi\
        textsh.sdi\
        wtextsh.sdi\
        _drwbase.sdi\
        drwbassh.sdi\
        wdrwbase.sdi\
        formsh.sdi\
        _formsh.sdi\
        wformsh.sdi\
        drawsh.sdi\
        drwtxtsh.sdi\
        _listsh.sdi\
        listsh.sdi\
        wlistsh.sdi\
        _frmsh.sdi\
        frmsh.sdi\
        wfrmsh.sdi\
        _tabsh.sdi\
        tabsh.sdi\
        wtabsh.sdi\
        _grfsh.sdi\
        grfsh.sdi\
        wgrfsh.sdi\
        _olesh.sdi\
        olesh.sdi\
        wolesh.sdi\
        beziersh.sdi\
        _viewsh.sdi\
        viewsh.sdi\
        wviewsh.sdi\
        _mediash.sdi\
        mediash.sdi\
        _annotsh.sdi\
        annotsh.sdi\
        swslots.hrc \
        $(INC)$/globals.hrc \
                $(INC)$/cmdid.h \
                $(SOLARINCXDIR)$/svxslots.ilb
.ENDIF

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk
