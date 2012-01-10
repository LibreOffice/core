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





$(CLASSDIR)/com/sun/star/upd/$(VERSIONINFOFILE)_updversion.class .PHONY: $(MISC)/java/com/sun/star/upd/$(VERSIONINFOFILE)_updversion.java 

$(MISC)/java/com/sun/star/upd/$(VERSIONINFOFILE)_updversion.java .PHONY:
    @echo "Making:   " $(@:f)
    @-$(MKDIRHIER) $(MISC)/java/com/sun/star/upd
.IF "$(GUI)"=="UNX"
    @echo package com.sun.star.upd\; > $@
    @echo 'public class $(VERSIONINFOFILE)_updversion { public static String Version = "$(RSCREVISION)";}' >> $@
.ELSE			# "$(GUI)"=="UNX"
    @echo package com.sun.star.upd; > $@
    @echo public class $(VERSIONINFOFILE)_updversion { public static String Version = "$(RSCREVISION)";} >> $@
.ENDIF			# "$(GUI)"=="UNX"

.IF "$(JAVATARGET)"!=""
.IF "$(PACKAGE)"!=""
$(CLASSDIR)/$(PACKAGE)/%.class .NOINFER .IGNORE : %.java
    $(COMMAND_ECHO)-$(RM) $(JAVATARGET)
.ELSE			# "$(PACKAGE)"!=""
$(CLASSDIR)/%.class .NOINFER .IGNORE : %.java
    $(COMMAND_ECHO)-$(RM) $(JAVATARGET)
.ENDIF			# "$(PACKAGE)"!=""

$(JAVATARGET) :	$(JAVAFILES) $(JAVACLASSFILES) 
.IF "$(JAVARESPONSE)"!=""
    $(JAVAC) @<<
    $(JAVACPS) $(CLASSPATH) -d $(CLASSDIR) $(JAVAFLAGS) $(JAVAFILES)
    <<keep
.ELSE			# "$(JAVARESPONSE)"!=""
.IF "$(use_jdep)"!=""
    $(JAVAC) -depend $(JAVACPS) $(CLASSPATH) -d $(CLASSDIR) $(JAVAFLAGS) $(JAVAFILES)
.ELSE			# "$(use_jdep)"!=""
.IF "$(javauno)"!=""
.IF "$(JAVAFILES:d)"==""
    $(JAVAC) $(JAVACPS) $(CLASSPATH) -d $(CLASSDIR) $(JAVAFLAGS) $(JAVAFILES)
.ELSE			# "$(JAVAFILES:d)"==""
    @@$(TOUCH) $(INPATH)_$(VCSID)_a_dummy.java
    $(JAVAC) $(JAVACPS) $(CLASSPATH) -d $(CLASSDIR) $(JAVAFLAGS) ./*.java $(uniq $(JAVAFILES:d:+"*.java"))
    @@-$(RM) $(INPATH)_$(VCSID)_a_dummy.java
.ENDIF			# "$(JAVAFILES:d)"==""
.ELSE			# "$(javauno)"!=""
    $(JAVAC) $(JAVACPS) "$(CLASSPATH)" -d $(CLASSDIR) $(JAVAFLAGS) @$(mktmp $(strip $(JAVAFILES)))
.ENDIF			# "$(javauno)"!=""
.ENDIF			# "$(use_jdep)"!=""
.ENDIF			# "$(JAVARESPONSE)"!=""
.IF "$(UPDATER)"!=""
.IF "$(OS)$(CPU)"=="SOLARISS"
    @@-find $(CLASSDIR) -type d -user $(USER) \! -perm -5 -print | xargs chmod a+r $$1
.ENDIF
.ENDIF
    @$(TOUCH) $@

.ENDIF			# "$(JAVATARGET)"!=""
