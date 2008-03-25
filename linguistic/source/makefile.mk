#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.25 $
#
#   last change: $Author: obo $ $Date: 2008-03-25 16:28:38 $
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

PRJNAME=linguistic
TARGET=lng
ENABLE_EXCEPTIONS=TRUE

#----- Settings ---------------------------------------------------------

.INCLUDE : settings.mk

# --- Files --------------------------------------------------------

SLOFILES = \
        $(SLO)$/convdiclist.obj\
        $(SLO)$/convdic.obj\
        $(SLO)$/convdicxml.obj\
        $(SLO)$/dicimp.obj\
        $(SLO)$/dlistimp.obj\
        $(SLO)$/hhconvdic.obj\
        $(SLO)$/hyphdsp.obj\
        $(SLO)$/hyphdta.obj\
        $(SLO)$/iprcache.obj\
        $(SLO)$/lngopt.obj\
        $(SLO)$/lngreg.obj\
        $(SLO)$/lngsvcmgr.obj\
        $(SLO)$/misc.obj\
        $(SLO)$/misc2.obj\
        $(SLO)$/prophelp.obj\
        $(SLO)$/spelldsp.obj\
        $(SLO)$/spelldta.obj\
        $(SLO)$/supplang.obj\
        $(SLO)$/thesdsp.obj\
        $(SLO)$/thesdta.obj\
        $(SLO)$/gciterator.obj\
        $(SLO)$/grammarchecker.obj

SHL1TARGET= $(TARGET)$(DLLPOSTFIX)

SHL1STDLIBS= \
        $(CPPULIB) 	 \
        $(CPPUHELPERLIB) 	 \
        $(I18NUTILLIB)  \
        $(VOSLIB)		\
        $(TOOLSLIB)		\
        $(I18NISOLANGLIB) \
        $(SVTOOLLIB)	\
        $(SVLLIB)		\
        $(VCLLIB)		\
        $(SFXLIB)		\
        $(SALLIB)		\
        $(UCBHELPERLIB)	\
        $(UNOTOOLSLIB)  \
        $(XMLOFFLIB)    \
        $(ICUUCLIB)
        
# build DLL
SHL1LIBS=		$(SLB)$/$(TARGET).lib
SHL1DEPN=		$(SLB)$/$(TARGET).lib
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def
#SHL1VERSIONMAP= $(TARGET).map
SHL1IMPLIB= ilng

# build DEF file
DEF1NAME	=$(SHL1TARGET)
DEF1DEPN    =$(MISC)$/$(SHL1TARGET).flt
DEFLIB1NAME =$(TARGET)
DEF1DES     =Linguistic main DLL
#DEF1EXPORTFILE=	exports.dxp


.IF "$(GUI)"=="WNT"
DEF1EXPORT1 = component_writeInfo
DEF1EXPORT2 = component_getFactory
DEF1EXPORT3 = component_getImplementationEnvironment
.ENDIF

# --- Targets ------------------------------------------------------


.INCLUDE : target.mk


$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo lcl > $@
    @echo component >> $@
    @echo __CT >> $@

