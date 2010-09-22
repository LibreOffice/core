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

my_cp:=$(CLASSPATH)$(PATH_SEPERATOR)$(SOLARBINDIR)/jaxp.jar$(PATH_SEPERATOR)$(SOLARBINDIR)/juh.jar$(PATH_SEPERATOR)$(SOLARBINDIR)/parser.jar$(PATH_SEPERATOR)$(SOLARBINDIR)/xt.jar$(PATH_SEPERATOR)$(SOLARBINDIR)/unoil.jar$(PATH_SEPERATOR)$(SOLARBINDIR)/ridl.jar$(PATH_SEPERATOR)$(SOLARBINDIR)/jurt.jar$(PATH_SEPERATOR)$(SOLARBINDIR)/xmlsearch.jar$(PATH_SEPERATOR)$(SOLARBINDIR)/LuceneHelpWrapper.jar$(PATH_SEPERATOR)$(SOLARBINDIR)/HelpIndexerTool.jar$

.IF "$(SYSTEM_LUCENE)" == "YES"
my_cp!:=$(my_cp)$(PATH_SEPERATOR)$(LUCENE_CORE_JAR)$(PATH_SEPERATOR)$(LUCENE_ANALYZERS_JAR)
.ELSE
my_cp!:=$(my_cp)$(PATH_SEPERATOR)$(SOLARBINDIR)/lucene-core-2.3.jar$(PATH_SEPERATOR)$(SOLARBINDIR)/lucene-analyzers-2.3.jar
.ENDIF
 
.IF "$(SYSTEM_DB)" != "YES"
JAVA_LIBRARY_PATH= -Djava.library.path=$(SOLARSHAREDBIN)
.ENDIF 

#aux_alllangiso*:=$(foreach,i,$(alllangiso) $(foreach,j,$(aux_langdirs) $(eq,$i,$j  $i $(NULL))))
aux_alllangiso*:=$(alllangiso)

#########################################################


HELPLINKALLTARGETS=$(foreach,i,$(aux_alllangiso) $(MISC)/$(TARGET)_$(LINKNAME)_$i.done)
HELPLINKALLADDEDDEPS=$(foreach,i,$(aux_alllangiso) $(subst,LANGUAGE,$i $(LINKADDEDDPES)))

ALLTAR : $(HELPLINKALLTARGETS)

.IF "$(SYSTEM_DB)" != "YES"
JAVA_LIBRARY_PATH= -Djava.library.path=$(SOLARSHAREDBIN)
.ENDIF

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

