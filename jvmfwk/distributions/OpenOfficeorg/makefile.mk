#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.8 $
#
#   last change: $Author: vg $ $Date: 2008-03-18 12:20:25 $
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

