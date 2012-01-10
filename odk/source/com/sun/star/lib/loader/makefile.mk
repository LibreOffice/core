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



PRJ=..$/..$/..$/..$/..$/..

PRJNAME = odk
PACKAGE = com$/sun$/star$/lib$/loader
TARGET  = com_sun_star_lib_loader


# --- Settings -----------------------------------------------------

.INCLUDE : settings.mk

.IF "$(SOLAR_JAVA)"==""
nojava:
    @echo "Not building javaunohelper because Java is disabled"
.ENDIF

# Files --------------------------------------------------------

JAVAFILES= \
    Loader.java\
    InstallationFinder.java\
    WinRegKey.java\
    WinRegKeyException.java

JAVACLASSFILES=	$(foreach,i,$(JAVAFILES) $(CLASSDIR)$/$(PACKAGE)$/$(i:b).class)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
