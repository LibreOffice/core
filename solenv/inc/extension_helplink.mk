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



my_cp:=$(CLASSPATH)$(PATH_SEPERATOR)$(SOLARBINDIR)/jaxp.jar$(PATH_SEPERATOR)$(SOLARBINDIR)/juh.jar$(PATH_SEPERATOR)$(SOLARBINDIR)/parser.jar$(PATH_SEPERATOR)$(SOLARBINDIR)/xt.jar$(PATH_SEPERATOR)$(SOLARBINDIR)/unoil.jar$(PATH_SEPERATOR)$(SOLARBINDIR)/ridl.jar$(PATH_SEPERATOR)$(SOLARBINDIR)/jurt.jar$(PATH_SEPERATOR)$(SOLARBINDIR)/xmlsearch.jar$(PATH_SEPERATOR)$(SOLARBINDIR)/LuceneHelpWrapper.jar$(PATH_SEPERATOR)$(SOLARBINDIR)/HelpIndexerTool.jar$

.IF "$(SYSTEM_LUCENE)" == "YES"
my_cp!:=$(my_cp)$(PATH_SEPERATOR)$(LUCENE_CORE_JAR)$(PATH_SEPERATOR)$(LUCENE_ANALYZERS_JAR)
.ELSE
my_cp!:=$(my_cp)$(PATH_SEPERATOR)$(SOLARBINDIR)/lucene-core-2.3.jar$(PATH_SEPERATOR)$(SOLARBINDIR)/lucene-analyzers-2.3.jar
.ENDIF
 
#aux_alllangiso*:=$(foreach,i,$(alllangiso) $(foreach,j,$(aux_langdirs) $(eq,$i,$j  $i $(NULL))))
aux_alllangiso*:=$(alllangiso)

#########################################################


HELPLINKALLTARGETS=$(foreach,i,$(aux_alllangiso) $(MISC)/$(TARGET)_$(LINKNAME)_$i.done)
HELPLINKALLADDEDDEPS=$(foreach,i,$(aux_alllangiso) $(subst,LANGUAGE,$i $(LINKADDEDDPES)))

ALLTAR : $(HELPLINKALLTARGETS)

XSL_DIR*:=$(SOLARBINDIR)
XHPLINKSRC*:=$(XHPDEST)
STY_SWITCH:= -sty $(XSL_DIR)/embed.xsl

.IF "$(XHPFILES)"!="" && "$(HELP_OUT)"!=""
$(foreach,i,$(LINKLINKFILES) $(XHPLINKSRC)/{$(aux_alllangiso)}/$i) : $(HELP_OUT)/$(TARGET).done
LINKDEPS+=$(HELP_OUT)/xhp_changed.flag
.ENDIF          # "$(XHPFILES)"!="" && "$(HELP_OUT)"!=""

$(HELPLINKALLTARGETS) : $(foreach,i,$(LINKLINKFILES) $(XHPLINKSRC)/$$(@:b:s/_/./:e:s/.//)/$i) $(subst,LANGUAGE,$$(@:b:s/_/./:e:s/.//) $(LINKDEPS))
    @echo Building help index for $(@:b:s/_/./:e:s/.//)
    $(COMMAND_ECHO)$(HELPLINKER) -mod $(LINKNAME) -extlangsrc $(XHPLINKSRC)/{$(subst,$(TARGET)_$(LINKNAME)_, $(@:b))} $(STY_SWITCH) -extlangdest $(XHPLINKSRC)/{$(subst,$(TARGET)_$(LINKNAME)_, $(@:b))} -idxcaption $(XSL_DIR)/idxcaption.xsl -idxcontent $(XSL_DIR)/idxcontent.xsl $(LINKLINKFILES)
    $(COMMAND_ECHO)cd $(XHPLINKSRC)/{$(subst,$(TARGET)_$(LINKNAME)_, $(@:b))} && zip -u -r $(LINKNAME).jar $(PACKAGE)/* $(CHECKZIPRESULT)
.IF "$(SOLAR_JAVA)" == "TRUE"
# cleanup index dir
    -$(RM) $(XHPLINKSRC)/{$(subst,$(TARGET)_$(LINKNAME)_, $(@:b))}/$(LINKNAME).idxl/*
.IF "$(CHECK_LUCENCE_INDEXER_OUTPUT)" == ""
    $(COMMAND_ECHO)$(JAVAI) $(JAVAIFLAGS) $(JAVA_LIBRARY_PATH) -cp "$(my_cp)" com.sun.star.help.HelpIndexerTool -extension -lang $(@:b:s/_/./:e:s/.//) -mod $(LINKNAME) -zipdir $(XHPLINKSRC)/{$(subst,$(TARGET)_$(LINKNAME)_, $(@:b))} && $(TOUCH) $@
.ELSE
    $(COMMAND_ECHO)$(JAVAI) $(JAVAIFLAGS) $(JAVA_LIBRARY_PATH) -cp "$(my_cp)" com.sun.star.help.HelpIndexerTool -extension -lang $(@:b:s/_/./:e:s/.//) -mod $(LINKNAME) -zipdir $(XHPLINKSRC)/{$(subst,$(TARGET)_$(LINKNAME)_, $(@:b))} -checkcfsandsegname _0 _3  && $(TOUCH) $@
.ENDIF
.ELSE
    -$(RM) $(XHPLINKSRC)/$(@:b)/content/*.*
    -$(RM) $(XHPLINKSRC)/$(@:b)/caption/*.*
.ENDIF

