#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.9 $
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

PRJ = ..$/..
PRJNAME = jvmfwk
TARGET = vendors_ooo

.INCLUDE: settings.mk

.IF "$(SOLAR_JAVA)"==""
nojava:
    @echo "Not building jvmfwk  because Java is disabled"
.ENDIF

.IF "$(SOLAR_JAVA)"!=""
$(BIN)$/javavendors.xml: javavendors_unx.xml javavendors_wnt.xml javavendors_macosx.xml javavendors_linux.xml
.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="MACOSX"
    -$(COPY) javavendors_macosx.xml $(BIN)$/javavendors.xml
.ELIF "$(OS)"=="LINUX" || "$(OS)"=="FREEBSD"
    -$(COPY) javavendors_linux.xml $(BIN)$/javavendors.xml
.ELSE
    -$(COPY) javavendors_unx.xml $(BIN)$/javavendors.xml
.ENDIF
.ELIF "$(GUI)"=="WNT"
    -$(COPY) javavendors_wnt.xml $(BIN)$/javavendors.xml	
.ELIF "$(GUI)"=="OS2"
    -$(COPY) javavendors_os2.xml $(BIN)$/javavendors.xml	
.ELSE
    @echo Unsupported platform.
.ENDIF

.ENDIF          # "$(SOLAR_JAVA)"!=""




.INCLUDE: target.mk

