#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: obo $ $Date: 2006-04-20 13:53:34 $
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

PRJNAME=soldep
TARGET=soldep


# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

SLOFILES        = \
    $(SLO)$/soldep.obj		\
    $(SLO)$/soldlg.obj		\
    $(SLO)$/depper.obj		\
     $(SLO)$/hashtbl.obj		\
    $(SLO)$/hashobj.obj		\
    $(SLO)$/connctr.obj		\
    $(SLO)$/depwin.obj		\
    $(SLO)$/graphwin.obj	\
    $(SLO)$/tbox.obj		\
    $(SLO)$/sdresid.obj		\
    $(SLO)$/objwin.obj

SHL1TARGET	=$(TARGET)$(UPD)$(DLLPOSTFIX)
SHL1IMPLIB	=i$(TARGET)
SHL1LIBS	=$(SLB)$/$(TARGET).lib
SHL1DEF		=$(MISC)$/$(SHL1TARGET).def
SHL1DEPN	=$(SHL1LIBS)
SHL1STDLIBS     = $(SVTOOLLIB)		\
                $(CPPUHELPERLIB)	\
                $(COMPHELPERLIB)	\
                $(SVLIB)			\
                $(SOTLIB)			\
                $(BTSTRPLIB)		\
                $(TOOLSLIB) 		\
                $(VOSLIB)			\
                $(UNOLIB)			\
                $(OSLLIB)			\
                $(SALLIB)           \
                $(CPPULIB)  \
                $(LB)$/ibootstrpdt.lib \
                   $(PERL_LIB)

.IF "$(GUI)" == "UNX"
SHL1STDLIBS+=\
        $(SALLIB)
.ENDIF


DEF1NAME    =$(SHL1TARGET)
DEF1DEPN	=$(MISC)$/$(SHL1TARGET).flt
DEFLIB1NAME	=$(TARGET)

#------------- Application ---------------
APP1TARGET=soldepl
APP1OBJS= \
    $(SLO)$/depapp.obj

APP1LIBS=   $(LB)$/ibootstrpdt.lib \
            $(LB)$/isoldep.lib

APP1ICON=soldep.ico
APP1STDLIBS= \
            $(SVTOOLLIB)		\
            $(CPPUHELPERLIB)	\
            $(COMPHELPERLIB)	\
            $(SVLIB)			\
            $(SOTLIB)			\
            $(BTSTRPLIB)		\
            $(TOOLSLIB) 		\
            $(VOSLIB)			\
            $(UNOLIB)			\
            $(OSLLIB)			\
            $(SALLIB)           \
               $(CPPULIB)  \
               $(PERL_LIB)


# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

ALLTAR : $(BIN)$/applicat.rdb

$(BIN)$/applicat.rdb : makefile.mk $(UNOUCRRDB)
    rm -f $@
    $(GNUCOPY) $(UNOUCRRDB) $@
     +cd $(BIN) && \
    regcomp -register -r applicat.rdb \
             -c i18nsearch.uno$(DLLPOST) \
             -c i18npool.uno$(DLLPOST)

$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo WEP > $@
    @echo LIBMAIN >> $@
    @echo LibMain >> $@
    @echo __CT >> $@