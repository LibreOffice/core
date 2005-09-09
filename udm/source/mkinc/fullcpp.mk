#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: fullcpp.mk,v $
#
#   $Revision: 1.7 $
#
#   last change: $Author: rt $ $Date: 2005-09-09 14:34:53 $
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



# --- Settings -----------------------------------------------------
# Has to be included AFTER settings.mk !


# Precompiled header:


.IF "$(GUI)"=="WNT"

# RTTI
CFLAGS+= -GR

.IF "$(NP_LOCALBUILD)"!=""


# Precompiled Headers
PCH_NAME=  udm
.IF "$(DBG_LEVEL)"<"2"
CFLAGS+= -YX"precomp.h" /Fp$(PRJ)$/$(INPATH)$/misc$/$(PCH_NAME).pch
.ELSE
CFLAGS+= -YX"precomp.h" /Fp$(PRJ)$/$(INPATH)$/misc$/$(PCH_NAME).pcd -DNP_LOCALBUILD
.ENDIF

.ENDIF

.ENDIF

.IF "$(OS)"=="LINUX" || "$(OS)"=="FREEBSD" || "$(OS)"=="NETBSD"
CFLAGSCXX+= -frtti
.ENDIF

