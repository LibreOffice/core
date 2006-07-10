#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.17 $
#
#   last change: $Author: obo $ $Date: 2006-07-10 15:24:15 $
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
PRJINC=$(PRJ)$/source
PRJNAME=dbaccess
TARGET=browser

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
# --- Files --------------------------------------------------------

EXCEPTIONSFILES=\
            $(SLO)$/genericcontroller.obj		\
            $(SLO)$/dsbrowserDnD.obj			\
            $(SLO)$/dataview.obj				\
            $(SLO)$/dbexchange.obj				\
            $(SLO)$/dbloader.obj				\
            $(SLO)$/sbagrid.obj					\
            $(SLO)$/formadapter.obj				\
            $(SLO)$/brwview.obj					\
            $(SLO)$/brwctrlr.obj				\
            $(SLO)$/unodatbr.obj				\
            $(SLO)$/sbamultiplex.obj			\
            $(SLO)$/exsrcbrw.obj                \
            $(SLO)$/dbtreemodel.obj				\
            $(SLO)$/dsEntriesNoExp.obj			\
            $(SLO)$/dbtreeview.obj

SLOFILES =\
            $(EXCEPTIONSFILES)\
            $(SLO)$/AsyncronousLink.obj

SRS1NAME=$(TARGET)
SRC1FILES =  sbabrw.src	\
            sbagrid.src

# --- Targets -------------------------------------------------------


.INCLUDE :  target.mk

$(SRS)$/$(TARGET).srs: $(SOLARINCDIR)$/svx$/globlmn.hrc


