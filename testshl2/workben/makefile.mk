#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.7 $
#
#   last change: $Author: vg $ $Date: 2007-02-06 13:39:51 $
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
PRJINC=

PRJNAME=testshl2
TARGET=test_autoptr
TARGETTYPE=CUI

ENABLE_EXCEPTIONS=TRUE
#USE_LDUMP2=TRUE
#LDUMP2=LDUMP3

# --- Settings -----------------------------------------------------
.INCLUDE :  settings.mk

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

# APP5TARGET= test_very_long_names
# APP5OBJS=$(OBJ)$/test_very_long_names.obj
# 
# APP5STDLIBS=$(SALLIB)
# 
# APP5DEPN=
# APP5LIBS= 


#-------------------------------------------------------------------------------

APP6TARGET= test_string
APP6OBJS=$(OBJ)$/test_string.obj

APP6STDLIBS=$(SALLIB)

APP6DEPN=
APP6LIBS= 
#-------------------------------------------------------------------------------

APP7TARGET= test_printf
APP7OBJS=$(OBJ)$/test_printf.obj

APP7STDLIBS=$(SALLIB)

APP7DEPN=
APP7LIBS= 
#-------------------------------------------------------------------------------

APP8TARGET= test_preproc
APP8OBJS=$(OBJ)$/test_preproc.obj

APP8STDLIBS=$(SALLIB)

APP8DEPN=
APP8LIBS= 
# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
