#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



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
