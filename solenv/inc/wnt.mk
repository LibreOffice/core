#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
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

# --- Windows-NT-Environment ---------------------------------------
# Used if "$(GUI)" == "WNT"

# --- Compiler ---

.IF "$(OS)$(COM)" == "WNTMSC"
# for wntmsci12, wntmscx12, wntmsci13 and wntmscx13
# (MSVC 2008 and MSVC 2010, 32- and 64-bit)
.INCLUDE : wntmsc.mk
.ENDIF # "$(OS)$(COM)" == "WNTMSC"

.IF "$(COM)$(OS)$(CPU)" == "GCCWNTI"
.INCLUDE : wntgcci.mk
.ENDIF

# --- changes for W32-tcsh - should move into settings.mk ---
JAVAC=javac
JAVA=java
JAVAI!:=java
PATH_SEPERATOR*=:

# --- general WNT settings ---

HC=hc
HCFLAGS=
DLLPRE=
DLLPOST=.dll
EXECPOST=.exe
SCPPOST=.inf
DLLDEST=$(BIN)
SOLARSHAREDBIN=$(SOLARBINDIR)

.IF "$(SOLAR_JAVA)"!=""
JAVADEF=-DSOLAR_JAVA
.IF "$(debug)"==""
JAVA_RUNTIME=javai.lib
.ELSE
JAVA_RUNTIME=javai_g.lib
.ENDIF
.ENDIF

OOO_LIBRARY_PATH_VAR = PATH
