#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.9 $
#
#   last change: $Author: fs $ $Date: 2000-10-30 13:46:58 $
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

PRJ=..
PRJNAME=dbaccess
TARGET=dba
TARGET2=dbu
USE_LDUMP2=TRUE

USE_DEFFILE=TRUE

# --- Settings ----------------------------------

.INCLUDE :	svpre.mk
.INCLUDE :	settings.mk
.INCLUDE :	sv.mk

LDUMP=ldump2.exe

# --- database core (dba) -----------------------------------

LIB1TARGET=$(SLB)$/dbaccess.lib
LIB1FILES=\
        $(SLB)$/api.lib	\
        $(SLB)$/dataaccess.lib	\
        $(SLB)$/misc.lib	\
        $(SLB)$/core_resource.lib	\
        $(SLB)$/dbashared.lib	

SHL1TARGET=$(TARGET)$(UPD)$(DLLPOSTFIX)
SHL1VERSIONMAP= $(TARGET).map

SHL1STDLIBS= \
        $(SALLIB) \
        $(OSLLIB) \
        $(ONELIB) \
        $(STORELIB) \
        $(TOOLSLIB) \
        $(VOSLIB) \
        $(CPPUHELPERLIB) \
        $(CPPULIB) \
        $(UNOTOOLSLIB) \
        $(VCLLIB) \
        $(SVLLIB) \
        $(COMPHELPERLIB) \
        $(SVTOOLLIB) \
        $(DBTOOLSLIB) \
        $(COMPHELPERLIB)	\

.IF "$(DBTOOLSLIB)" == ""
SHL1STDLIBS+=idbt$(UPD)$(DLLPOSTFIX).lib
.ENDIF

.IF "$(COMPHELPERLIB)" == ""
SHL1STDLIBS+= icomphelp2.lib
.ENDIF

SHL1LIBS=$(LIB1TARGET)
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=$(TARGET).dxp 

#SHL1DEPN=\
        makefile.mk

ALL: \
    $(LIB1TARGET)	\
    ALLTAR

# --- .res file ----------------------------------------------------------
RES1FILELIST=\
    $(SRS)$/core_strings.srs

RESLIB1NAME=$(TARGET)
RESLIB1SRSFILES=$(RES1FILELIST)

# --- database ui (dbu) -----------------------------------

LIB2TARGET=$(SLB)$/$(TARGET2).lib
LIB2FILES=\
        $(SLB)$/uimisc.lib		\
        $(SLB)$/uidlg.lib		\
        $(SLB)$/dbushared.lib	\
        $(SLB)$/browser.lib		\
        $(SLB)$/uiuno.lib		\
        $(SLB)$/uicontrols.lib

SHL2TARGET=$(TARGET2)$(UPD)$(DLLPOSTFIX)
SHL2VERSIONMAP= $(TARGET2).map

SHL2STDLIBS= \
        $(SALLIB)				\
        $(OSLLIB)				\
        $(ONELIB)				\
        $(VOSLIB)				\
        $(CPPUHELPERLIB)		\
        $(CPPULIB)				\
        $(UNOTOOLSLIB)			\
        $(VCLLIB)				\
        $(TOOLSLIB)				\
        $(SFXLIB)				\
        $(SVLLIB)				\
        $(SVTOOLLIB)			\
        $(TKLIB)				\
        $(COMPHELPERLIB)		\
        $(SVXLIB)				\
        $(SOTLIB)				\
        $(SO2LIB)				\
        $(UCBHELPERLIB) 		\
        $(DBTOOLSLIB)

.IF "$(DBTOOLSLIB)" == ""
SHL2STDLIBS+=idbt$(UPD)$(DLLPOSTFIX).lib
.ENDIF

.IF "$(COMPHELPERLIB)" == ""
SHL2STDLIBS+= icomphelp2.lib
.ENDIF

SHL2LIBS=$(LIB2TARGET)
SHL2DEF=$(MISC)$/$(SHL2TARGET).def
DEF2NAME=$(SHL2TARGET)
DEF2EXPORTFILE=$(TARGET2).dxp 

ALL: \
    $(LIB2TARGET)	\
    $(SRS)$/hidother.hid	\
    ALLTAR

# --- .res file ----------------------------------------------------------
RES2FILELIST=\
    $(SRS)$/uidlg.srs			\
    $(SRS)$/uicontrols.srs		\
    $(SRS)$/browser.srs			\
    $(SRS)$/uiuno.srs			\
    $(SRS)$/uimisc.srs

RESLIB2NAME=$(TARGET2)
RESLIB2SRSFILES=$(RES2FILELIST)

# --- Targets ----------------------------------

.INCLUDE : target.mk



$(SRS)$/hidother.hid: hidother.src
.IF "$(GUI)$(CPU)"=="WNTI"
.IF "$(BUILD_SOSL)"==""
    +-mhids hidother.src ..\$(INPATH)$/srs dbu hidother $(INCLUDE)
.ENDIF
.ELSE
    @+echo nix
.ENDIF

