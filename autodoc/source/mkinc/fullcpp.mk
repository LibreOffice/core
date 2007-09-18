#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: fullcpp.mk,v $
#
#   $Revision: 1.9 $
#
#   last change: $Author: vg $ $Date: 2007-09-18 14:06:38 $
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


# RTTI
.IF "$(GUI)"=="WNT"
CFLAGS+= -GR
.ENDIF
.IF "$(OS)"=="LINUX" || "$(OS)"=="FREEBSD" || "$(OS)"=="NETBSD" || $(COM) == "GCC"
CFLAGSCXX+= -frtti
.ENDIF



# Precompiled Headers
.IF "$(NP_LOCALBUILD)"!="" && "$(GUI)"=="WNT"

PCH_NAME=autodoc
.IF "$(debug)"==""
CFLAGS+= -YX"precomp.h" -Fp$(PRJ)$/$(INPATH)$/misc$/$(PCH_NAME).pch
.ELSE
CFLAGS+= -YX"precomp.h" -Fp$(PRJ)$/$(INPATH)$/misc$/$(PCH_NAME).pcd
.ENDIF

.ENDIF  # "$(NP_LOCALBUILD)"!="" && "$(GUI)"=="WNT"
