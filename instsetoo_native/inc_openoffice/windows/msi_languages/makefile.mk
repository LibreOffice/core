#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.7 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 18:24:03 $
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

PRJ=..$/..$/..

PRJNAME=instsetoo_native
TARGET=win_ulffiles

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# ------------------------------------------------------------------

ULFFILES = \
    ActionTe.ulf \
    Control.ulf  \
    CustomAc.ulf \
    Error.ulf    \
    Langpack.ulf \
    LaunchCo.ulf \
    Property.ulf \
    RadioBut.ulf \
    Nsis.ulf     \
    SIS.ulf      \
    UIText.ulf 
    
MLFFILES = $(foreach,i,$(ULFFILES) $(COMMONMISC)$/$(TARGET)$/$(i:b).mlf)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

.IF "$(GUI)"=="WNT"

ALLTAR : $(MLFFILES)

.ENDIF
