#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.17 $
#
#   last change: $Author: obo $ $Date: 2008-02-25 15:34:50 $
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
PRJ=..$/..
PRJNAME=extensions
TARGET=scn
ENABLE_EXCEPTIONS=TRUE
PACKAGE=com$/sun$/star$/scanner
USE_DEFFILE=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :	$(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SLOFILES=\
                $(SLO)$/scnserv.obj		\
                $(SLO)$/scanner.obj

.IF "$(GUI)"=="WNT"
SLOFILES+=\
                $(SLO)$/scanwin.obj		
.ENDIF

.IF "$(GUI)"=="UNX" || "$(GUI)" =="OS2"
SLOFILES+=\
                $(SLO)$/sane.obj		\
                $(SLO)$/sanedlg.obj		\
                $(SLO)$/scanunx.obj		\
                $(SLO)$/grid.obj

.ENDIF

SRS1NAME=$(TARGET)
SRC1FILES=\
    sanedlg.src		\
    grid.src

RESLIB1NAME=san
RESLIB1IMAGES=$(PRJ)$/source$/scanner
RESLIB1SRSFILES= $(SRS)$/scn.srs
RESLIB1DEPN= sanedlg.src sanedlg.hrc grid.src grid.hrc

SHL1TARGET= $(TARGET)$(DLLPOSTFIX)
SHL1STDLIBS=\
    $(CPPULIB)			\
    $(CPPUHELPERLIB)	\
    $(COMPHELPERLIB)	\
    $(VOSLIB)			\
    $(SALLIB)			\
    $(ONELIB)			\
    $(TOOLSLIB)			\
    $(VCLLIB)			\
    $(SVTOOLLIB)		\
    $(GOODIESLIB)		

.IF "$(GUI)"=="UNX"
.IF "$(OS)"!="FREEBSD"
.IF "$(OS)"!="NETBSD"
SHL1STDLIBS+=$(SVTOOLLIB) -ldl
.ENDIF
.ENDIF
.ENDIF

SHL1DEF=$(MISC)$/$(SHL1TARGET).def
SHL1IMPLIB=i$(TARGET)
SHL1LIBS=$(SLB)$/$(TARGET).lib
SHL1VERSIONMAP=exports.map

DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=exports.dxp


# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

