#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.14 $
#
#   last change: $Author: obo $ $Date: 2008-02-25 17:30:44 $
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

PRJNAME=soldep
TARGET=soldep


# --- Settings -----------------------------------------------------
.INCLUDE :  $(PRJ)$/util$/perl.mk
.INCLUDE :  settings.mk


# --- Files --------------------------------------------------------

SLOFILES        = \
    $(SLO)$/soldep.obj		\
    $(SLO)$/soldlg.obj		\
    $(SLO)$/depper.obj		\
    $(SLO)$/hashobj.obj		\
    $(SLO)$/connctr.obj		\
    $(SLO)$/depwin.obj		\
    $(SLO)$/graphwin.obj	\
    $(SLO)$/tbox.obj		\
    $(SLO)$/sdtresid.obj		\
    $(SLO)$/objwin.obj

SHL1TARGET	=$(TARGET)$(DLLPOSTFIX)
SHL1IMPLIB	=$(TARGET)
SHL1LIBS	=$(SLB)$/$(TARGET).lib \
            $(SLB)$/bootstrpdt.lib
SHL1DEF		=$(MISC)$/$(SHL1TARGET).def
SHL1DEPN	=$(SHL1LIBS)
SHL1STDLIBS     = $(SVTOOLLIB)		\
                $(CPPUHELPERLIB)	\
                $(COMPHELPERLIB)	\
                $(VCLLIB)			\
                $(SOTLIB)			\
                $(BTSTRPLIB)		\
                $(TOOLSLIB) 		\
                $(VOSLIB)			\
                $(SALLIB)           \
                $(CPPULIB)  \
                   $(PERL_LIB)

#.IF "$(GUI)" == "UNX"
#SHL1STDLIBS+=\
#		$(SALLIB)
#.ENDIF


DEF1NAME    =$(SHL1TARGET)
DEF1DEPN	=$(MISC)$/$(SHL1TARGET).flt
DEFLIB1NAME	=$(TARGET)

SRC1FILES = \
    soldlg.src
RES1TARGET = dep
SRS1NAME=$(TARGET)
SRS1FILES = \
    $(SRS)$/$(TARGET).srs

RESLIB1NAME = dep
RESLIB1SRSFILES = \
    $(SRS)$/$(TARGET).srs


#------------- Application ---------------
APP1TARGET=soldepl

APP1ICON=soldep.ico

APP1STDLIBS= \
            $(SVTOOLLIB)		\
            $(CPPUHELPERLIB)	\
            $(COMPHELPERLIB)	\
            $(VCLLIB)			\
            $(SOTLIB)			\
            $(BTSTRPLIB)		\
            $(TOOLSLIB) 		\
            $(VOSLIB)			\
            $(SALLIB)           \
               $(CPPULIB)  \
               $(PERL_LIB)

APP1LIBS=\
    $(SLB)$/bootstrpdt.lib \
    $(SLB)$/soldep.lib

APP1OBJS= \
    $(SLO)$/depapp.obj

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

ALLTAR : $(BIN)$/applicat.rdb

$(BIN)$/applicat.rdb : makefile.mk $(UNOUCRRDB)
    rm -f $@
    $(GNUCOPY) $(UNOUCRRDB) $@
     cd $(BIN) && \
    $(REGCOMP) -register -r applicat.rdb \
             -c i18nsearch.uno$(DLLPOST) \
             -c i18npool.uno$(DLLPOST)

$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo WEP > $@
    @echo LIBMAIN >> $@
    @echo LibMain >> $@
    @echo __CT >> $@
