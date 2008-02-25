#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.20 $
#
#   last change: $Author: obo $ $Date: 2008-02-25 15:54:20 $
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

PRJ=..$/..$/..

PRJNAME=sw
TARGET=app

LIBTARGET=NO

# future: DEMO\...

# --- Settings -----------------------------------------------------

.INCLUDE :  $(PRJ)$/inc$/swpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/inc$/sw.mk

# --- Files --------------------------------------------------------

SRS1NAME=$(TARGET)
SRC1FILES =\
        app.src     \
        mn.src      \
        error.src


SLOFILES = \
        $(SLO)$/appenv.obj   \
        $(SLO)$/apphdl.obj   \
        $(SLO)$/applab.obj   \
        $(SLO)$/appopt.obj   \
        $(SLO)$/docsh.obj    \
        $(SLO)$/docsh2.obj   \
        $(SLO)$/docshdrw.obj \
        $(SLO)$/docshini.obj \
        $(SLO)$/docst.obj    \
        $(SLO)$/docstyle.obj \
        $(SLO)$/mainwn.obj   \
        $(SLO)$/swmodule.obj \
        $(SLO)$/swmodul1.obj \
        $(SLO)$/swdll.obj	 \
        $(SLO)$/swwait.obj

EXCEPTIONSFILES= \
        $(SLO)$/docsh.obj    \
        $(SLO)$/docst.obj    \
        $(SLO)$/swmodule.obj \
        $(SLO)$/swmodul1.obj \
            $(SLO)$/apphdl.obj   \
        $(SLO)$/docsh2.obj

LIB1TARGET= $(SLB)$/app.lib

LIB1OBJFILES= \
        $(SLO)$/appenv.obj   \
        $(SLO)$/apphdl.obj   \
        $(SLO)$/applab.obj   \
        $(SLO)$/appopt.obj   \
        $(SLO)$/docsh.obj    \
        $(SLO)$/docsh2.obj   \
        $(SLO)$/docshdrw.obj \
        $(SLO)$/docshini.obj \
        $(SLO)$/docst.obj    \
        $(SLO)$/docstyle.obj \
        $(SLO)$/mainwn.obj   \
        $(SLO)$/swmodul1.obj \
        $(SLO)$/swwait.obj


# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

$(SRS)$/app.srs: $(SOLARINCDIR)$/svx$/globlmn.hrc

