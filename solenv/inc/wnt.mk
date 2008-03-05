#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: wnt.mk,v $
#
#   $Revision: 1.83 $
#
#   last change: $Author: kz $ $Date: 2008-03-05 16:34:54 $
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
STARDEP=javadep
JAVAC=javac
JAVA=java
JAVAI!:=java
.ENDIF # "$(USE_SHELL)"!="4nt"

# --- general WNT settings ---
CLIMAKER*=climaker

HC=hc
HCFLAGS=
PATH_SEPERATOR*=;
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

