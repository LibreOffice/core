#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: wnt.mk,v $
#
# $Revision: 1.84 $
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

.IF "$(OS)$(COM)$(CPU)" == "WNTMSCI"
.IF "$(COMEX)" == "10"
.INCLUDE : wntmsci10.mk
.ELSE
# for wntmsci11 (.Net 2005) and wntmsci12 (.Net 2008)
.INCLUDE : wntmsci11.mk
.ENDIF # "$(COMEX)" == "10"
.ENDIF # "$(OS)$(COM)$(CPU)" == "WNTMSCI"

.IF "$(COM)$(CVER)$(OS)$(CPU)" == "GCCC341WNTI"
.INCLUDE : wntgcci6.mk
.ENDIF

# --- changes for W32-tcsh - should move into settings.mk ---
.IF "$(USE_SHELL)"!="4nt"
JAVAC=javac
JAVA=java
JAVAI!:=java
PATH_SEPERATOR*=:
.ELSE # "$(USE_SHELL)"!="4nt"
PATH_SEPERATOR*=;
.ENDIF # "$(USE_SHELL)"!="4nt"

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

.IF "$(USE_SHELL)" == "bash"
AUGMENT_LIBRARY_PATH *= : && \
    PATH=$${{PATH}}:$(SOLARBINDIR:s/://:^"/cygdrive/")
.ELSE
AUGMENT_LIBRARY_PATH *= echos && PATH=%PATH%;$(SOLARBINDIR) &&
.ENDIF
