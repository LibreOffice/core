#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.7 $
#
#   last change: $Author: rt $ $Date: 2005-09-08 14:16:58 $
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
PRJINC=$(PRJ)$/source
PRJNAME=dbaccess
LIBTARGET=NO
TARGET=shared
LIB1TARGET=$(SLB)$/dbashared.lib
LIB2TARGET=$(SLB)$/dbushared.lib
LIB3TARGET=$(SLB)$/fltshared.lib
LIB4TARGET=$(SLB)$/cfgshared.lib

#ENABLE_EXCEPTIONS=TRUE

# --- Settings ----------------------------------

.INCLUDE : settings.mk

# --- Types -------------------------------------

# --- Files -------------------------------------
EXCEPTIONSFILES=	\
        $(SLO)$/apitools.obj	\
        $(SLO)$/dba_reghelper.obj	\
        $(SLO)$/dbu_reghelper.obj	\
        $(SLO)$/cfg_reghelper.obj	\
        $(SLO)$/flt_reghelper.obj
        
SLOFILES=	\
        $(EXCEPTIONSFILES)			\
        $(SLO)$/cfgstrings.obj		\
        $(SLO)$/xmlstrings.obj		\
        $(SLO)$/dbastrings.obj		\
        $(SLO)$/dbustrings.obj


LIB1OBJFILES=	\
        $(SLO)$/dba_reghelper.obj	\
        $(SLO)$/dbastrings.obj	\
        $(SLO)$/apitools.obj
        
LIB2OBJFILES=	\
        $(SLO)$/dbu_reghelper.obj	\
        $(SLO)$/dbustrings.obj
        
LIB3OBJFILES=	\
        $(SLO)$/flt_reghelper.obj	\
        $(SLO)$/xmlstrings.obj
        
LIB4OBJFILES=	\
        $(SLO)$/cfg_reghelper.obj	\
        $(SLO)$/cfgstrings.obj

# --- Targets ----------------------------------

.INCLUDE : target.mk

