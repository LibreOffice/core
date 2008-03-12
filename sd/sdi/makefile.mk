#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.10 $
#
#   last change: $Author: rt $ $Date: 2008-03-12 11:24:12 $
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

PRJ=..

PRJNAME=sd
TARGET=sdslots
TARGET2=sdgslots
SDI1EXPORT=sdraw
SDI2EXPORT=sdraw
SVSDIINC=$(PRJ)$/source$/ui$/inc

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------


SDI1NAME=$(TARGET)
SDI2NAME=$(TARGET2)
SIDHRCNAME=SDSLOTS.HRC

SVSDI1DEPEND= \
    sdraw.sdi \
    app.sdi \
    _docsh.sdi \
    docshell.sdi \
    drtxtob.sdi \
    drbezob.sdi \
    drgrfob.sdi \
    _drvwsh.sdi \
    drviewsh.sdi \
    mediaob.sdi \
    tables.sdi\
    outlnvsh.sdi \
    slidvish.sdi \
    SlideSorterController.sdi \
    ViewShellBase.sdi \
    TaskPaneViewShell.sdi \
    sdslots.hrc \
    $(PRJ)$/inc$/app.hrc

SVSDI2DEPEND= \
    sdraw.sdi \
    app.sdi \
    _docsh.sdi \
    grdocsh.sdi \
    drtxtob.sdi \
    drbezob.sdi \
    drgrfob.sdi \
    _drvwsh.sdi \
    grviewsh.sdi \
    mediaob.sdi \
    outlnvsh.sdi \
    slidvish.sdi \
    ViewShellBase.sdi \
    TaskPaneViewShell.sdi \
    sdslots.hrc \
    $(PRJ)$/inc$/app.hrc

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk
