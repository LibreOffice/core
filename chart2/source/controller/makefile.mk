#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.11 $
#
#   last change: $Author: obo $ $Date: 2008-02-25 16:57:55 $
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
                $(SLB)$/chchartapiwrapper.lib \
                $(SLB)$/chcaccessibility.lib

#--------

#Indicates the filename of the shared library.
SHL1TARGET=		$(TARGET)$(DLLPOSTFIX)

#indicates dependencies:
.IF "$(COM)" == "MSC"
SHL1DEPN = \
        $(LB)$/icharttools.lib \
        $(LB)$/ichartview.lib
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
                $(BASEGFXLIB) 		\
                $(SALLIB)			\
                $(SVLLIB)			\
                $(SVTOOLLIB)		\
                $(SVXLIB)			\
                $(TKLIB)			\
                $(TOOLSLIB) 		\
                $(I18NISOLANGLIB)   \
                $(VCLLIB)           \
                $(SFXLIB)			\
                $(UNOTOOLSLIB)		\
                $(SOTLIB)

#--------exports

#specifies the exported symbols for Windows only:
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def

SHL1VERSIONMAP = controller.map

#--------definition file

#name of the definition file:
DEF1NAME=		$(SHL1TARGET)

# --- Resources ---------------------------------------------------------------

# sfx.srs is needed for the strings for UNDO and REDO in the UndoCommandDispatch
RESLIB1LIST=\
    $(SRS)$/chcdialogs.srs \
    $(SOLARCOMMONRESDIR)$/sfx.srs

RESLIB1NAME=	$(TARGET)
RESLIB1IMAGES=$(PRJ)$/res
RESLIB1SRSFILES=$(RESLIB1LIST)
RESLIB1DEPN=$(RESLIB1LIST)

#RESLIB1SRSFILES=$(SRS)$/$(TARGET).srs
#RESLIB1DEPN=	SRCFILES

# --- Targets -----------------------------------------------------------------

.INCLUDE: target.mk
