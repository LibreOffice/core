#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.1.1.1 $
#
#   last change: $Author: hr $ $Date: 2000-09-18 17:03:00 $
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

PRJ=..$/..

PRJNAME=tools
TARGET=addexes
TARGETTYPE=CUI
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk
.INCLUDE : 	static.mk

CDEFS+=-D_TOOLS_STRINGLIST

# --- Files --------------------------------------------------------

LIB1TARGET= $(LB)$/btstrp.lib
LIB1ARCHIV= $(LB)$/libbtstrp.a
LIB1FILES=  $(LB)$/bootstrp.lib

APP1TARGET=	dirsync
APP1STACK=	16000
APP1OBJS=   $(OBJ)$/dirsync.obj
APP1STDLIBS=$(STATIC_LIBS)
APP1LIBS=	$(LB)$/bootstrp.lib
APP1DEPN=   $(LB)$/atools.lib $(LB)$/bootstrp.lib

APP2TARGET=	include
APP2STACK=	16000
APP2OBJS=   $(OBJ)$/include.obj
APP2STDLIBS=$(STATIC_LIBS)
APP2LIBS=	$(LB)$/bootstrp.lib
APP2DEPN=   $(LB)$/atools.lib $(LB)$/bootstrp.lib

APP3TARGET=	ldump
APP3STACK=	16000
APP3OBJS=   $(OBJ)$/ldump.obj
APP3STDLIBS=$(STATIC_LIBS) $(LIBCIMT)
APP3LIBS=	$(LB)$/bootstrp.lib
APP3DEPN=   $(LB)$/atools.lib $(LB)$/bootstrp.lib

APP4TARGET=	readmap
APP4STACK=	16000
APP4OBJS=   $(OBJ)$/readmap.obj $(OBJ)$/deco.obj
APP4STDLIBS=$(STATIC_LIBS) $(LIBCIMT)
APP4LIBS=	$(LB)$/bootstrp.lib
APP4DEPN=   $(LB)$/atools.lib $(LB)$/bootstrp.lib

APP5TARGET=	deco
APP5STACK=	16000
APP5OBJS=   $(OBJ)$/deco.obj $(OBJ)$/deco2.obj
APP5STDLIBS=$(STATIC_LIBS) $(LIBCIMT)
APP5LIBS=	$(LB)$/bootstrp.lib
APP5DEPN=   $(LB)$/atools.lib $(LB)$/bootstrp.lib

APP6TARGET= urlconv
APP6OBJS= $(OBJ)$/urlconv.obj
APP6STDLIBS=\
    $(TOOLSLIB)	\
    $(SALLIB)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
