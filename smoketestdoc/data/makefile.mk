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



PRJ = ..
PRJNAME = smoketestdoc
TARGET = data

.INCLUDE: settings.mk

$(BIN)/smoketestdoc.sxw: $(MISC)/zip/smoketestdoc.sxw
    $(COPY) $< $@

$(MISC)/zip/smoketestdoc.sxw: \
        Events.xml \
        Global.xml \
        OptionsDlg.xml \
        Test_10er.xml \
        Test_DB.xml \
        Test_Ext.xml \
        content.xml \
        dialog-lb.xml \
        dialog-lc.xml \
        manifest.xml \
        meta.xml \
        script-lb.xml \
        script-lc.xml \
        settings.xml \
        styles.xml
    $(COMMAND_ECHO)$(RM) -r $(MISC)/zip
    $(COMMAND_ECHO)$(MKDIR) $(MISC)/zip
    $(COMMAND_ECHO)$(COPY) content.xml meta.xml settings.xml styles.xml $(MISC)/zip/
    $(COMMAND_ECHO)printf application/vnd.sun.xml.writer > $(MISC)/zip/mimetype
    $(COMMAND_ECHO)$(MKDIR) $(MISC)/zip/META-INF
    $(COMMAND_ECHO)$(COPY) manifest.xml $(MISC)/zip/META-INF/
    $(COMMAND_ECHO)$(MKDIR) $(MISC)/zip/Basic
    $(COMMAND_ECHO)$(COPY) script-lc.xml $(MISC)/zip/Basic/
    $(COMMAND_ECHO)$(MKDIR) $(MISC)/zip/Basic/Standard
    $(COMMAND_ECHO)$(COPY) script-lb.xml Events.xml Global.xml Test_10er.xml Test_DB.xml \
        Test_Ext.xml $(MISC)/zip/Basic/Standard/
    $(COMMAND_ECHO)$(MKDIR) $(MISC)/zip/Dialogs
    $(COMMAND_ECHO)$(COPY) dialog-lc.xml $(MISC)/zip/Dialogs/
    $(COMMAND_ECHO)$(MKDIR) $(MISC)/zip/Dialogs/Standard
    $(COMMAND_ECHO)$(COPY) dialog-lb.xml OptionsDlg.xml $(MISC)/zip/Dialogs/Standard/
    $(COMMAND_ECHO)cd $(MISC)/zip && zip -rD $(@:f) .

.INCLUDE: target.mk
