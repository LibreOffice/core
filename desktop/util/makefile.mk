#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: obo $ $Date: 2000-10-31 16:19:47 $
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

PRJNAME=desktop
TARGET=soffice
GEN_HID=TRUE

# --- Settings -----------------------------------------------------------

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk

VERINFONAME=verinfo

#IENV=$(IENV);..\res

# --- Resourcen ----------------------------------------------------

.IF "$(GUI)" == "WNT"
RCFILES=verinfo.rc
.ENDIF

# --- Linken der Applikation ---------------------------------------

.IF "$(OS)" == "LINUX"
# #74158# linux needs sal/vos/tools at end of link list, solaris needs it first,
# winXX is handled like solaris for now
APP1_STDPRE=
APP1_STDPOST=$(CPPULIB) $(CPPUHELPERLIB) $(UNOLIB) $(TOOLSLIB) \
    $(VOSLIB) $(SALLIB)
.ELSE
APP1_STDPRE=$(SALLIB) $(VOSLIB) $(TOOLSLIB) $(UNOLIB) $(CPPULIB) \
    $(CPPUHELPERLIB)
APP1_STDPOST=
.ENDIF

APP1TARGET=$(TARGET)

APP1STDLIBS= 		\
    $(APP1_STDPRE)  \
    $(SFX2LIB) 		\
    $(BASICLIB) 	\
    $(SO2LIB) 		\
    $(SJLIB) 		\
    $(TKLIB) 		\
    $(SVTOOLLIB) 	\
    $(SETUPLIB) 	\
    $(SVLLIB) 		\
    $(SVMEMLIB) 	\
        $(OFALIB)               \
    $(ONELIB) 		\
    $(VCLLIB) 		\
    $(SOTLIB) 		\
        $(APP1_STDPOST)     \
                $(SCHLIB)               \
                $(SMLIB)                \
                $(SIMLIB)               \
                $(SWLIB)                \
                $(SDLIB)                \
                $(SCLIB)                \
                $(SVXLIB)               \
    $(UNOTOOLSLIB)						\
    $(COMPHELPERLIB)

.IF "$(OS)" == "SOLARIS"
# for Java applets
APP1STDLIBS+= -lXm
.ENDIF

.IF "$(GUI)" == "UNX"
.IF "$(OS)" == "LINUX"

APP1STDLIBS+= -lXt -lXmu -lXext -lX11 -lSM -lICE

.ENDIF
.ENDIF


.IF "$(GUI)" == "MAC"
APP1STDLIBS+=$(SOLARLIBDIR)$/SALMAIN.OBJ \
            $(SOLARLIBDIR)$/NOSHAREDMAIN.LIB \
            $(SOLARLIBDIR)$/aofa.lib
MACRES += writer.r
.ENDIF


APP1DEPN= \
        $(APP1RES) \
        verinfo.rc

APP1OBJS= \
                $(OBJ)$/app.obj \
                $(OBJ)$/wrapper.obj

#APP1STACK=64000

APP1DEF=    $(MISCX)$/$(TARGET).def

APP1RES=    $(RES)$/desktop.res

APP1ICON=soffice.ico
APP1VERINFO=verinfo.rc
APP1LINKRES=$(MISC)$/$(TARGET).res



# --- Targets -------------------------------------------------------------

.INCLUDE :  target.mk

# -------------------------------------------------------------------------
# MAC
# -------------------------------------------------------------------------

.IF "$(GUI)" == "MAC"

$(MISCX)$/$(APP1TARGET).def : makefile
    echo "kein Def-File bei Applikationen"

.ENDIF

.IF "$(GUI)" == "WNT"

$(MISCX)$/$(APP1TARGET).def : makefile
    echo  NAME			soffice								>$@
    echo  DESCRIPTION   'StarDesktop Version 5'           >>$@
    echo  DATA			READ WRITE NONSHARED		   >>$@
.ENDIF

