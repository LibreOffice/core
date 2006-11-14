#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.8 $
#
#   last change: $Author: ihi $ $Date: 2006-11-14 15:30:23 $
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

PRJ=				..$/..
PRJNAME=			chart2
TARGET=				chartcontroller

USE_DEFFILE=		TRUE
ENABLE_EXCEPTIONS=	TRUE

# --- Settings -----------------------------------------------------

.INCLUDE: $(PRJ)$/util$/makefile.pmk

# --- export library -------------------------------------------------

#You can use several library macros of this form to build libraries that
#do not consist of all object files in a directory or to merge different libraries.
LIB1TARGET=		$(SLB)$/$(TARGET).lib

LIB1FILES=		\
                $(SLB)$/chcontroller.lib \
                $(SLB)$/chcdrawinglayer.lib \
                $(SLB)$/chcitemsetwrapper.lib \
                $(SLB)$/chcdialogs.lib \
                $(SLB)$/chchartapiwrapper.lib

#--------

#Indicates the filename of the shared library.
SHL1TARGET=		$(TARGET)$(UPD)$(DLLPOSTFIX)

#indicates dependencies:
.IF "$(COM)" == "MSC"
SHL1DEPN = \
        $(SLB)$/charttools.lib \
        $(SLB)$/chartview.lib
.ELSE
SHL1DEPN =
.ENDIF

#Specifies an import library to create. For Win32 only.
SHL1IMPLIB=		i$(TARGET)

#Specifies libraries from the same module to put into the shared library.
#was created above
SHL1LIBS= 		$(LIB1TARGET)

#Links import libraries.

SHL1STDLIBS=	$(CHARTTOOLS)		\
                $(CHARTVIEW)		\
                $(CPPULIB)			\
                $(CPPUHELPERLIB)	\
                $(COMPHELPERLIB)	\
                $(BASEGFXLIB)		\
                $(GOODIESLIB)		\
                $(SALLIB)			\
                $(SVLIB)			\
                $(SVLLIB)			\
                $(SVTOOLLIB)		\
                $(SVXLIB)			\
                $(TKLIB)			\
                $(TOOLSLIB) 		\
                $(I18NISOLANGLIB)   \
                $(VCLLIB)           \
                $(SFXLIB)			\
                $(UNOTOOLSLIB)

#--------exports

#specifies the exported symbols for Windows only:
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def

SHL1VERSIONMAP = controller.map

#--------definition file

#name of the definition file:
DEF1NAME=		$(SHL1TARGET)

# --- Resources ---------------------------------------------------------------

RESLIB1LIST=\
    $(SRS)$/chcmenu.srs	\
    $(SRS)$/chcdialogs.srs

RESLIB1NAME=	$(TARGET)
RESLIB1IMAGES=$(PRJ)$/res
RESLIB1SRSFILES=$(RESLIB1LIST)
RESLIB1DEPN=$(RESLIB1LIST)

#RESLIB1SRSFILES=$(SRS)$/$(TARGET).srs
#RESLIB1DEPN=	SRCFILES

# --- Targets -----------------------------------------------------------------

.INCLUDE: target.mk
