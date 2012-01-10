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


PRJNAME=postprocess
TARGET=signing

.INCLUDE : settings.mk

# PFXFILE	has to be set elsewhere
# PFXPASSWORD	has to be set elsewhere

EXCLUDELIST=no_signing.txt
LOGFILE=$(MISC)$/signing_log.txt
IMAGENAMES=$(SOLARBINDIR)$/*.dll $(SOLARBINDIR)$/so$/*.dll $(SOLARBINDIR)$/*.exe $(SOLARBINDIR)$/so$/*.exe
TIMESTAMPURL*="http://timestamp.verisign.com/scripts/timstamp.dll"

signing.done :
.IF "$(VISTA_SIGNING)"!=""
.IF "$(COM)"=="MSC"
.IF "$(product)"=="full"
    $(PERL) signing.pl -e $(EXCLUDELIST) -f $(PFXFILE) -p $(PFXPASSWORD) -t $(TIMESTAMPURL) $(IMAGENAMES) && $(TOUCH) $(MISC)$/signing.done
.ELSE  # "$(product)"=="full"
    @echo Doing nothing on non product builds ...
.ENDIF # "$(product)"=="full"
.ELSE  # "$(GUI)"=="MSC"
    @echo Nothing to do, signing is Windows \(MSC\) only.
.ENDIF # "$(GUI)"=="MSC"
.ELSE  # "$(VISTA_SIGNING)"!=""
    @echo Doing nothing. To switch on signing set VISTA_SIGNING=TRUE ...
.ENDIF # "$(VISTA_SIGNING)"!=""

.INCLUDE : target.mk

