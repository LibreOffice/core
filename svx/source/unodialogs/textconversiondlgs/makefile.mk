#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.10 $
#
#   last change: $Author: obo $ $Date: 2008-02-25 16:21:15 $
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

PRJNAME=svx
TARGET=textconversiondlgs
ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE

LOCALIZE_ME=chinese_direction_tmpl.hrc

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

SLOFILES=           \
                    $(SLO)$/services.obj\
                    $(SLO)$/resid.obj\
                    $(SLO)$/chinese_dictionarydialog.obj\
                    $(SLO)$/chinese_translationdialog.obj\
                    $(SLO)$/chinese_translation_unodialog.obj

SRS1NAME=$(TARGET)
SRC1FILES=          \
                    chinese_translationdialog.src\
                    chinese_dictionarydialog.src

RESLIB1NAME=$(TARGET)
RESLIB1SRSFILES= $(SRS)$/$(TARGET).srs

.IF "$(GUI)" == "OS2"
SHL1TARGET= txcnvdlg
.ELSE
SHL1TARGET= $(TARGET)$(DLLPOSTFIX)
.ENDIF
SHL1VERSIONMAP= export.map

SHL1STDLIBS= \
        $(CPPULIB)  \
        $(SALLIB)   \
        $(CPPUHELPERLIB) \
        $(UNOTOOLSLIB) \
        $(COMPHELPERLIB) \
        $(TOOLSLIB)  \
        $(TKLIB)	 \
        $(SVTOOLLIB) \
        $(SVLLIB) \
        $(VCLLIB)

SHL1LIBS=		$(SLB)$/$(TARGET).lib
SHL1IMPLIB=		i$(TARGET)
SHL1DEPN=		$(SHL1LIBS)
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def

DEF1NAME=		$(SHL1TARGET)


# --- Targets ------------------------------------------------------
.INCLUDE :	target.mk

$(SRS)$/$(TARGET).srs: $(INCCOM)$/chinese_direction.hrc
$(SLO)$/chinese_dictionarydialog.obj: $(INCCOM)$/chinese_direction.hrc
$(SLO)$/chinese_translationdialog.obj: $(INCCOM)$/chinese_direction.hrc

