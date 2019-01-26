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


PRJNAME = OOoRunner
TARGET=qadevOOo
PRJ=.

# ------------------------------------------------
# NEVER REMOVE THIS!
# The OOoRunner.jar and OOoRunnerLight.jar should build with debug information all the time, also in the .pro builds.
# If you have any questions about this, ask the qadevOOo Maintainer.
debug=true
# ------------------------------------------------

.INCLUDE : ant.mk
TST:
    echo $(SOLAR_JAVA)

.IF "$(SOLAR_JAVA)"=="TRUE"	
.IF "$(ANT_HOME)"!="NO_ANT_HOME"
ANT_FLAGS+=-Dbuild.source=1.6
.IF "$(L10N_framework)"==""
ALLTAR: ANTBUILD
.ENDIF
.ENDIF
.ENDIF 
