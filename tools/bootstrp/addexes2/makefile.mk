#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: svesik $ $Date: 2000-11-20 01:31:10 $
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
TARGET=addexes2
TARGETTYPE=CUI
TARGETTHREAD=MT

# --- Settings -----------------------------------------------------

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk
.INCLUDE :  ../static.mk

# --- Files --------------------------------------------------------

APP1TARGET= javadep
APP1OBJS=	$(OBJ)$/javadep.obj
APP1STDLIBS=$(STATIC_LIBS) 

.IF "$(GUI)"=="UNX"
APP2TARGET= checkdll
APP2OBJS=	$(OBJ)$/checkdll.obj
.ENDIF

APP3TARGET=	mkunroll
APP3OBJS=   $(OBJ)$/mkfilt.obj
APP3STDLIBS=$(STATIC_LIBS) 
.IF "$(OS)"=="LINUX"
APP3STDLIBS+=-lpthread
.ENDIF
.IF "$(OS)"=="NETBSD"
APP3STDLIBS+=-lpthread
.ENDIF
APP3LIBS=	$(LB)$/bootstrp.lib
APP3DEPN=   $(LB)$/atools.lib $(LB)$/bootstrp.lib


APP4TARGET= zipdep
APP4OBJS=	$(OBJ)$/zipdep.obj
APP4LIBS=	$(LB)$/bootstrp.lib
APP4STDLIBS=$(STATIC_LIBS) 

DEPOBJFILES		=	$(APP1OBJS) $(APP2OBJS) $(APP3OBJS) $(APP4OBJS) $(APP5OBJS) $(APP6OBJS) $(APP7OBJS) $(APP8OBJS) $(APP9OBJS)
# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
