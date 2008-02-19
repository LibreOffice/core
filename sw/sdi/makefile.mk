#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.10 $
#
#   last change: $Author: rt $ $Date: 2008-02-19 13:37:48 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
#
#*************************************************************************

# wx mktyplib nur unter Windows aufrufbar
# VMB.386 und WXSRVR.EXE muessen geladen sein! (n:\mscwini\bin)

PRJ=..

PRJNAME=sw
TARGET=swslots

# --- Settings -----------------------------------------------------

SVSDIINC=$(PRJ)$/source$/ui$/inc

.INCLUDE :  $(PRJ)$/inc$/swpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/inc$/sw.mk


SDI1NAME=$(TARGET)
SDI1EXPORT=swriter
#SIDHRCNAME=swslots.hrc

# --- Files --------------------------------------------------------

SVSDI1DEPEND= \
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
        $(INC)$/cmdid.h

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk
