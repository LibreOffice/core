#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: obo $ $Date: 2008-02-25 15:30:46 $
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

PRJNAME=UnoControls
TARGET=base
ENABLE_EXCEPTIONS=TRUE
#LIBTARGET=NO
#USE_LDUMP2=TRUE
#USE_DEFFILE=TRUE

# --- Settings -----------------------------------------------------
.INCLUDE :	$(PRJ)$/util$/makefile.pmk


# --- Files --------------------------------------------------------
SLOFILES=		$(SLO)$/multiplexer.obj				\
                $(SLO)$/basecontrol.obj				\
                $(SLO)$/basecontainercontrol.obj	\
                $(SLO)$/registercontrols.obj

#LIB1TARGET= 	$(SLB)$/$(TARGET).lib
#LIB1OBJFILES=	$(SLOFILES)

#SHL1TARGET= $(TARGET)$(DLLPOSTFIX)

#SHL1STDLIBS=\
#		$(ONELIB)	 \
#		$(USRLIB)	 \
#		$(UNOLIB)	 \
#		$(VOSLIB) 	 \
#		$(OSLLIB) 	 \
#		$(TOOLSLIB) 	 \
#		$(RTLLIB)

#SHL1DEPN=		makefile.mk
#SHL1LIBS=		$(LIB1TARGET)
#SHL1DEF=		$(MISC)$/$(SHL1TARGET).def

#DEF1NAME=		$(SHL1TARGET)
#DEF1EXPORTFILE=	exports.dxp

# --- Targets ------------------------------------------------------
.INCLUDE :	target.mk
#.INCLUDE :	$(PRJ)$/util$/target.pmk
