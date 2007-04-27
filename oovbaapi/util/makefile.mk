#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: rt $ $Date: 2007-04-27 07:26:44 $
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
PRJPCH=

PRJNAME=oovbaapi
TARGET=oovbaapi_db


# --- Settings -----------------------------------------------------

.INCLUDE :  makefile.pmk

.IF "$(ENABLE_VBA)"!="YES"
dummy:
        @echo "not building vba..."
.ENDIF

# ------------------------------------------------------------------
UNOIDLDBFILES= \
    $(UCR)$/vba.db \
    $(UCR)$/excel.db \
    $(UCR)$/msforms.db

# --- Targets ------------------------------------------------------

ALLTAR : $(UCR)$/types.db 

$(UCR)$/types.db : $(UCR)$/oovbaapi.db 
    +-$(RM) $(REGISTRYCHECKFLAG)
    +$(GNUCOPY) -f $(UCR)$/oovbaapi.db $@

.INCLUDE :  target.mk
