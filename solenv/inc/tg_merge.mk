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



MKFILENAME:=tg_merge.mk

# look for the according rules in "rules.mk"

.IF "$(WITH_LANG)"!=""
.IF "$(ULFFILES)"!=""
$(foreach,i,$(ULFFILES) $(COMMONMISC)/$(TARGET)/$i) : $$(@:f) $(LOCALIZESDF) 
.ENDIF          # "$(ULFFILES)"!=""

# *.xrb merge
.IF "$(XMLPROPERTIES)"!=""
$(foreach,i,$(XMLPROPERTIES) $(COMMONMISC)/$(TARGET)/$i) : $$(@:f) $(LOCALIZESDF) 
.ENDIF          # "$(ULFFILES)"!=""

# *.xrm merge
.IF "$(READMEFILES)"!=""
$(foreach,i,$(READMEFILES) $(COMMONMISC)/$(TARGET)/$(i:b).xrm) : $$(@:f) $(LOCALIZESDF) 
.ENDIF          # "$(ULFFILES)"!=""

# *.xrm merge
#.IF "$(APOCHELPFILES)"!=""
#$(foreach,i,$(APOCHELPFILES) $(COMMONMISC)/$(MYPATH)/$(i:b).xrm) : $$(@:f) $(LOCALIZESDF) 
#.ENDIF          # "$(ULFFILES)"!=""
# *.xcu merge
.IF "$(LOCALIZEDFILES)"!=""
$(foreach,i,$(LOCALIZEDFILES) $(PROCESSOUT)/merge/{$(subst,.,/ $(PACKAGE))}/$(i:b).xcu) : $$(@:f) $(LOCALIZESDF) 
.ENDIF          # "$(ULFFILES)"!=""
.ENDIF			# "$(WITH_LANG)"!=""

# dependencies from *.ulf to par-files
.IF "$(ULFPARFILES)"!=""
$(ULFPARFILES) : $(COMMONMISC)/$(TARGET)/$$(@:b).$(LANGFILEEXT)
.ENDIF          # "$(ULFPARFILES)"!=""

# *.xrm merge
#.IF "$(READMEFILES)"!=""
#$(uniq $(foreach,i,$(READMEFILES) $(COMMONMISC)/$(TARGET)/$(i:b).xrm)) : $$(@:f) $(LOCALIZESDF)
#.ENDIF          # "$(ULFFILES)"!=""

