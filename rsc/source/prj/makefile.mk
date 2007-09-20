#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.7 $
#
#   last change: $Author: vg $ $Date: 2007-09-20 16:34:08 $
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

PRJNAME=rsc
TARGETTYPE=CUI
TARGET=rsc
LIBTARGET=NO

# --- Settings -----------------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------------

OBJFILES=   $(OBJ)$/gui.obj          \
            $(OBJ)$/start.obj

APP1TARGET= rsc
APP1STDLIBS=$(TOOLSLIB) $(I18NISOLANGLIB) $(VOSLIB) $(SALLIB) # $(RTLLIB)
APP1LIBS=   $(LB)$/rsctoo.lib
APP1OBJS=   $(OBJ)$/start.obj
.IF "$(GUI)" != "OS2"
# why not this way?
APP1STACK=64000
#APP1STACK=32768
.ENDIF

APP2TARGET= rsc2
.IF "$(OS)"=="SCO"
# SCO hat Probleme mit fork/exec und einigen shared libraries. 
# rsc2 muss daher statisch gelinkt werden
APP2STDLIBS=$(STATIC) -latools $(BPICONVLIB) $(VOSLIB) $(OSLLIB) $(RTLLIB) $(DYNAMIC)
.ELSE
APP2STDLIBS=$(TOOLSLIB) $(I18NISOLANGLIB) $(VOSLIB) $(SALLIB) #RTLLIB)
.ENDIF
APP2LIBS=   $(LB)$/rsctoo.lib \
            $(LB)$/rscres.lib \
            $(LB)$/rscpar.lib \
            $(LB)$/rscrsc.lib \
            $(LB)$/rscmis.lib
APP2OBJS=   $(OBJ)$/gui.obj
.IF "$(GUI)" != "OS2"
# why not this way?
APP2STACK=64000
#APP2STACK=32768
.ENDIF

# --- Targets ------------------------------------------------------------

.INCLUDE :  target.mk

