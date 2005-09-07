#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.24 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 15:08:49 $
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

#MKDEPENDSOLVER=YES
PROJECTPCH4DLL=TRUE

PRJ=..

PRJNAME=starmath
TARGET=starmath

PROJECTPCH=math_pch
PDBTARGET=math_pch
PROJECTPCHSOURCE=math_pch
LIBTARGET=NO

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SMDLL=TRUE

SRS2NAME =smres
SRC2FILES = smres.src   \
            symbol.src	\
            commands.src

SLO1FILES =  \
        $(SLO)$/register.obj \
        $(SLO)$/typemap.obj \
        $(SLO)$/symbol.obj \
        $(SLO)$/toolbox.obj \
        $(SLO)$/action.obj \
        $(SLO)$/accessibility.obj \
        $(SLO)$/cfgitem.obj \
        $(SLO)$/config.obj \
        $(SLO)$/dialog.obj \
        $(SLO)$/document.obj \
        $(SLO)$/mathtype.obj \
        $(SLO)$/mathml.obj \
        $(SLO)$/format.obj \
        $(SLO)$/node.obj \
        $(SLO)$/parse.obj \
        $(SLO)$/utility.obj \
        $(SLO)$/smdll.obj \
        $(SLO)$/smmod.obj \
        $(SLO)$/view.obj \
        $(SLO)$/viewhdl.obj \
        $(SLO)$/edit.obj \
        $(SLO)$/rect.obj \
        $(SLO)$/unomodel.obj \
        $(SLO)$/unodoc.obj \
        $(SLO)$/eqnolefilehdr.obj

SLO2FILES =  \
        $(SLO)$/register.obj  \
        $(SLO)$/detreg.obj

SLOFILES = \
    $(SLO1FILES) \
    $(SLO2FILES)

EXCEPTIONSFILES =   \
        $(SLO)$/accessibility.obj \
        $(SLO)$/mathml.obj \
        $(SLO)$/viewhdl.obj \
        $(SLO)$/unomodel.obj \
        $(SLO)$/smdetect.obj \
        $(SLO)$/unodoc.obj

LIB1TARGET = \
    $(SLB)$/$(TARGET).lib

LIB1OBJFILES = \
    $(SLO1FILES)

# --- Targets -------------------------------------------------------

.IF "$(GUI)"=="UNX" || "$(GUI)"=="MAC"
CDEFS+=-DUSE_POLYGON
.ENDIF

LOCALIZE_ME =  menu_tmpl.src


.INCLUDE :  target.mk

$(INCCOM)$/dllname.hxx: makefile.mk
.IF "$(GUI)"=="UNX"
    $(RM) $@
    +echo \#define DLL_NAME \"$(DLLPRE)sm$(UPD)$(DLLPOSTFIX)$(DLLPOST)\" >$@
.ELSE
.IF "$(USE_SHELL)"!="4nt"
    +echo \#define DLL_NAME \"$(DLLPRE)sm$(UPD)$(DLLPOSTFIX)$(DLLPOST)\" >$@
.ELSE          # "$(USE_SHELL)"!="4nt"
    +echo #define DLL_NAME "sm$(UPD)$(DLLPOSTFIX)$(DLLPOST)" >$@
.ENDIF          # "$(USE_SHELL)"!="4nt"
.ENDIF

$(SRS)$/smres.srs: $(SOLARINCDIR)$/svx$/globlmn.hrc


$(SRS)$/$(SRS2NAME).srs  : $(LOCALIZE_ME_DEST)

