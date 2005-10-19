#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: rt $ $Date: 2005-10-19 14:42:19 $
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

PRJ=..$/..

PRJNAME=svx

TARGET=gengal
TARGETTYPE=GUI
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SCRIPTFILES =	$(BIN)$/gengal

CXXFILES=	gengal.cxx

OBJFILES=	$(OBJ)$/gengal.obj

.IF "$(GUI)"=="WNT"
APP1TARGET= $(TARGET)
.ELSE			# .IF "$(GUI)"=="WNT"
APP1TARGET= $(TARGET).bin
.ENDIF			# .IF "$(GUI)"=="WNT"
APP1OBJS=	$(OBJFILES)
APP1STDLIBS=$(TOOLSLIB) 		\
            $(OSLLIB)			\
            $(ONELIB)			\
            $(VOSLIB)			\
            $(SO2LIB)			\
            $(SVTOOLLIB)		\
            $(SVLLIB)			\
            $(COMPHELPERLIB)		\
            $(CPPULIB)		\
            $(CPPUHELPERLIB)		\
            $(SALLIB)		\
            $(GOODIESLIB)		\
            $(SFXLIB)			\
            $(SOTLIB)			\
            $(DLLIB)			\
            $(SVLIB)			\
            $(UCBHELPERLIB)		\
            $(SVXLIB)

.IF "$(GUI)"!="UNX"
APP1DEF=	$(MISC)$/$(TARGET).def
.ENDIF


.IF "$(COM)"=="GCC"
ADDOPTFILES=$(OBJ)$/gengal.obj
add_cflagscxx="-frtti -fexceptions"
.ENDIF


# --- Targets ------------------------------------------------------

ALL : \
    ALLTAR \
    $(SCRIPTFILES) \
    $(BIN)$/gengal.rdb 

.INCLUDE :	target.mk

$(SCRIPTFILES) : $$(@:f:+".sh")
    +@tr -d "\015" < $(@:f:+".sh") > $@

$(BIN)$/gengal.rdb : makefile.mk $(UNOUCRRDB)
    rm -f $@
    $(GNUCOPY) $(UNOUCRRDB) $@
    +cd $(BIN) && \
        regcomp -register -r gengal.rdb \
            -c i18nsearch.uno$(DLLPOST) \
            -c i18npool.uno$(DLLPOST) \
            -c configmgr2.uno$(DLLPOST) \
            -c servicemgr.uno$(DLLPOST) \
            -c $(DLLPRE)fwl$(UPD)$(DLLSUFFIX)$(DLLPOST) \
            -c $(DLLPRE)ucpfile1$(DLLPOST) \
            -c $(DLLPRE)fileacc$(DLLPOST) \
            -c $(DLLPRE)ucb1$(DLLPOST)

