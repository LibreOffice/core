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


PRJ=..


PRJNAME=officecfg
TARGET=util

.INCLUDE : settings.mk
.INCLUDE : target.mk

# --- Targets ------------------------------------------------------

$(MISC)$/$(TARGET)_delzip :
    -$(RM) $(BIN)$/registry_{$(alllangiso)}.zip	

$(BIN)$/registry_{$(alllangiso)}.zip : $(MISC)$/$(TARGET)_delzip
    cd $(MISC)$/registry$/res$/$(@:b:s/registry_//) && zip -ru ..$/..$/..$/..$/bin$/registry_$(@:b:s/registry_//).zip org/*
    $(PERL) -w $(SOLARENV)$/bin$/cleanzip.pl $@

ALLTAR: \
    $(MISC)$/$(TARGET)_delzip \
    $(BIN)$/registry_{$(alllangiso)}.zip

