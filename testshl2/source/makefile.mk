#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: vg $ $Date: 2003-10-06 13:36:03 $
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

PRJNAME=testshl2
TARGET=testshl2
TARGETTYPE=CUI

ENABLE_EXCEPTIONS=TRUE
#USE_LDUMP2=TRUE
#LDUMP2=LDUMP3

# --- Settings -----------------------------------------------------
.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk

# ------------------------------------------------------------------

COMMON_OBJFILES=\
    $(OBJ)$/getopt.obj \
    $(OBJ)$/filehelper.obj \
    $(OBJ)$/dynamicregister.obj 

TESTSHL_OBJFILES=\
    $(OBJ)$/autoregisterhelper.obj \
    $(OBJ)$/testshl.obj 

VERSIONINFO_OBJFILES=\
    $(OBJ)$/versionhelper.obj \
    $(OBJ)$/versioner.obj 

BMSMA_OBJFILES=\
    $(OBJ)$/bmsma.obj

# for right dependencies
# OBJFILES= $(COMMON_OBJFILES) $(TESTSHL_OBJFILES) $(VERSIONINFO_OBJFILES)

# ------------------------------------------------------------------
APP1TARGET= $(TARGET)
APP1OBJS= $(COMMON_OBJFILES) $(TESTSHL_OBJFILES)

APP1STDLIBS=$(SALLIB)

APP1DEPN= \
    $(LB)$/cppunit_no_regallfkt.lib \
    $(SLB)$/testresult.lib

APP1LIBS= \
    $(LB)$/cppunit_no_regallfkt.lib \
    $(SLB)$/testresult.lib

#
# ------------------------------------------------------------------
APP2TARGET= versioninfo
APP2OBJS= $(COMMON_OBJFILES) $(VERSIONINFO_OBJFILES)

APP2STDLIBS=$(SALLIB)

APP2DEPN= 
APP2LIBS= 

# ------------------------------------------------------------------
APP3TARGET= $(TARGET)_timeout
APP3OBJS= 	$(OBJ)$/getopt.obj $(OBJ)$/filehelper.obj $(OBJ)$/terminate.obj

APP3STDLIBS=$(SALLIB)

APP3DEPN= 
APP3LIBS= 


# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
