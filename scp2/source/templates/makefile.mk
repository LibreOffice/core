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



PRJ=..$/..

PRJNAME=scp2
TARGET=templates

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

.INCLUDE :  target.mk

# force expanding
COMPLETELANGISO_VAR:=$(uniq $(completelangiso) $(alllangiso))
.EXPORT : COMPLETELANGISO_VAR

ALLTAR : $(INCCOM)$/alllangmodules.inc $(INCCOM)$/alllangmodules_root.inc $(INCCOM)$/alllangmodules_base.inc $(INCCOM)$/alllangmodules_calc.inc $(INCCOM)$/alllangmodules_draw.inc $(INCCOM)$/alllangmodules_impress.inc $(INCCOM)$/alllangmodules_math.inc $(INCCOM)$/alllangmodules_writer.inc

.INCLUDE .IGNORE : $(MISC)$/$(TARGET)_lang_track.mk
.IF "$(LAST_COMPLETELANGISO_VAR)"!="$(COMPLETELANGISO_VAR)"
PHONYTEMPL=.PHONY
.ENDIF			# "$(LAST_COMPLETELANGISO_VAR)"!="$(COMPLETELANGISO_VAR)"
$(INCCOM)$/alllangmodules%.inc $(PHONYTEMPL) : module_langpack%.sct
    @@-$(RENAME) $@ $@.tmp
    $(COMMAND_ECHO)$(PERL) -w modules.pl -i $< -o $@.tmp && $(RENAME:s/+//) $@.tmp $@
    @echo LAST_COMPLETELANGISO_VAR=$(COMPLETELANGISO_VAR) > $(MISC)$/$(TARGET)_lang_track.mk
