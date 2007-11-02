#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.8 $
#
#   last change: $Author: hr $ $Date: 2007-11-02 12:18:54 $
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

OBJFILES=	$(OBJ)$/gengal.obj

.IF "$(GUI)"=="WNT" || "$(GUI)"=="OS2"
APP1TARGET= $(TARGET)
.ELSE			# .IF "$(GUI)"=="WNT"
APP1TARGET= $(TARGET).bin
.ENDIF			# .IF "$(GUI)"=="WNT"

APP1OBJS=   $(OBJFILES)

APP1STDLIBS=$(TOOLSLIB) 		\
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
            $(VCLLIB)			\
            $(UCBHELPERLIB)		\
            $(SVXLIB)

#.IF "$(COM)"=="GCC"
#ADDOPTFILES=$(OBJ)$/gengal.obj
#add_cflagscxx="-frtti -fexceptions"
#.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

ALLTAR : \
    $(SCRIPTFILES)

$(SCRIPTFILES) : $$(@:f:+".sh")
    @tr -d "\015" < $(@:f:+".sh") > $@

