#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: hr $ $Date: 2003-08-07 15:08:19 $
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
PRJINC=

PRJNAME=testshl2
TARGET=test_autoptr
TARGETTYPE=CUI

ENABLE_EXCEPTIONS=TRUE
#USE_LDUMP2=TRUE
#LDUMP2=LDUMP3

# --- Settings -----------------------------------------------------
.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk

# ------------------------------------------------------------------
#--------------------------------- Objectfiles ---------------------------------
OBJFILES=\
    $(OBJ)$/test_autoptr.obj \
    $(OBJ)$/test_ostringstream.obj

#----------------------------- prog with *.lib file -----------------------------

APP1TARGET= $(TARGET)
APP1OBJS=$(OBJ)$/test_autoptr.obj 

APP1STDLIBS=$(SALLIB)

APP1DEPN= $(APP1OBJS)
APP1LIBS= 

#-------------------------------------------------------------------------------

APP2TARGET= test_ostringstream
APP2OBJS=$(OBJ)$/test_ostringstream.obj

APP2STDLIBS=$(SALLIB)

APP2DEPN= $(APP1OBJS)
APP2LIBS= 
#-------------------------------------------------------------------------------

APP3TARGET= test_filter
APP3OBJS=$(OBJ)$/test_filter.obj

APP3STDLIBS=$(SALLIB)

APP3DEPN= $(APP1OBJS)
APP3LIBS= 
#-------------------------------------------------------------------------------

# APP4TARGET= test_member
# APP4OBJS=$(OBJ)$/test_member.obj
# 
# APP4STDLIBS=$(SALLIB)
# 
# APP4DEPN=
 # APP4LIBS= 


#-------------------------------------------------------------------------------

APP5TARGET= test_very_long_names
APP5OBJS=$(OBJ)$/test_very_long_names.obj

APP5STDLIBS=$(SALLIB)

APP5DEPN=
APP5LIBS= 
# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
