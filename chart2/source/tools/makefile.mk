#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: iha $ $Date: 2003-11-10 19:32:44 $
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

PRJ=				..$/..
PRJNAME=			chart2
TARGET=				charttools

PRJINC=				$(PRJ)$/source

USE_DEFFILE=		TRUE
ENABLE_EXCEPTIONS=	TRUE

# --- Settings -----------------------------------------------------

.INCLUDE: $(PRJ)$/util$/makefile.pmk

# --- export library -------------------------------------------------

#You can use several library macros of this form to build libraries that
#do not consist of all object files in a directory or to merge different libraries.
#LIB1TARGET=		$(SLB)$/_$(TARGET).lib

#Specifies object files to bind into linked libraries.
SLOFILES=	\
                $(SLO)$/LifeTime.obj \
                $(SLO)$/MediaDescriptorHelper.obj \
                $(SLO)$/CommonConverters.obj \
                $(SLO)$/ContextHelper.obj \
                $(SLO)$/DataSeriesTreeHelper.obj \
                $(SLO)$/DiagramHelper.obj \
                $(SLO)$/ImplOPropertySet.obj \
                $(SLO)$/LayoutHelper.obj \
                $(SLO)$/OChartElementContainerHelper.obj \
                $(SLO)$/OEnumeration.obj \
                $(SLO)$/OIndexContainer.obj \
                $(SLO)$/OPropertySet.obj \
                $(SLO)$/OStyle.obj \
                $(SLO)$/Scaling.obj \
                $(SLO)$/ChartModelHelper.obj \
                $(SLO)$/ChartTypeHelper.obj \
                $(SLO)$/MeterHelper.obj \
                $(SLO)$/MutexContainer.obj \
                $(SLO)$/TitleHelper.obj \
                $(SLO)$/LegendHelper.obj

LIB1OBJFILES = $(SLOFILES)

#--------

#Indicates the filename of the shared library.
SHL1TARGET=		$(TARGET)$(UPD)$(DLLPOSTFIX)

#indicates dependencies:
SHL1DEPN=
#Specifies an import library to create. For Win32 only.
SHL1IMPLIB=		i$(TARGET)

#Specifies libraries from the same module to put into the shared library.
#was created above
SHL1LIBS= 		$(SLB)$/$(TARGET).lib

#Links import libraries.

SHL1STDLIBS=    \
                $(CPPULIB)			\
                $(CPPUHELPERLIB)	\
                $(SALLIB)			\
                $(TOOLSLIB)

#				$(COMPHELPERLIB)	\
#				$(GOODIESLIB)		\
#				$(SVLIB)			\
#				$(SVLLIB)			\
#				$(SVTOOLLIB)		\
#				$(SVXLIB)			\
#				$(TKLIB)			\
#				$(VCLLIB)           \
#			    $(SFXLIB)

#--------exports

#specifies the exported symbols for Windows only:
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def

#Specifies the library name to parse for symbols. For Win32 only.
#DEFLIB1NAME=	_$(TARGET)

#A file of symbols to export.
#DEF1EXPORTFILE=	$(PRJ)$/source$/inc$/exports.dxp

#--------definition file

#name of the definition file:
DEF1NAME=		$(SHL1TARGET)

# indicates definition file dependencies
DEF1DEPN=		$(MISC)$/$(SHL1TARGET).flt

#A comment on the definition file.
DEF1DES=		Chart Tools

# --- Targets -----------------------------------------------------------------

.INCLUDE: target.mk

# --- Filter -----------------------------------------------------------

$(MISC)$/$(SHL1TARGET).flt: makefile.mk \
                            exports.flt
    +$(TYPE) exports.flt > $@

