#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: fullcpp.mk,v $
#
# $Revision: 1.8 $
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
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

