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



EXTENSION_TARGET:=$(ZIP9TARGETN)

.SOURCE.xcu : $(MISC)/$(EXTNAME)/merge $(MISC)/$(EXTNAME)/registry/data $(MISC)/$(COMPONENT_SHARED_CONFIG)_in/merge $(MISC)/$(COMPONENT_SHARED_CONFIG)_in/registry/data $(COMPONENT_CONFIGDIR) .
.SOURCE.xcs : $(MISC)/$(EXTNAME)/registry $(MISC)/$(EXTNAME)/registry/schema .

fixme=$(MISC)/$(EXTNAME)/merge/$(MISC)
fixme2=$(fixme:n)

fixme3=$(MISC)/$(EXTNAME)/$(MISC)
fixme4=$(fixme:n)

$(EXTENSIONDIR)/$(COMPONENT_CONFIGDEST)/%.xcu : %.xcu
# ugly hacking to workaround prepended first source path
    @@-$(MKDIRHIER) $(subst,$(fixme2),$(MISC) $(@:d))
.IF "$(COMPONENT_XCU_REPLACEMENT)"!=""
    $(TYPE) $< | sed '$(COMPONENT_XCU_REPLACEMENT)' > $(subst,$(fixme2),$(MISC) $@)
.ELSE          # "$(COMPONENT_REPLACEMENT)"!=""
    $(GNUCOPY) $< $(subst,$(fixme2),$(MISC) $@)
.ENDIF          # "$(COMPONENT_REPLACEMENT)"!=""
    
$(EXTENSIONDIR)/$(COMPONENT_CONFIGDESTSCHEMA)/%.xcs : %.xcs
    @@-$(MKDIRHIER) $(subst,$(fixme4),$(MISC) $(@:d))
    $(GNUCOPY) $< $(subst,$(fixme4),$(MISC) $@)

$(EXTENSIONDIR)/%.jar : $(SOLARBINDIR)/%.jar
    @@-$(MKDIRHIER) $(@:d)
    $(GNUCOPY) $< $@

.IF "$(COMPONENT_FILES)"!=""
COMPONENT_FILES_SRC*=./
# ugly hacking to workaround prepended first source path - also hits
# here in case of "just copied" .xcu files
$(COMPONENT_FILES) : $$(@:s|$(fixme2)|$(MISC)|:s|$(EXTENSIONDIR)/|$(COMPONENT_FILES_SRC)|)
    @@-$(MKDIRHIER) $(@:s|$(fixme2)|$(MISC)|:d)
    $(COMMAND_ECHO)$(COPY) $< $(@:s|$(fixme2)|$(MISC)|)
.ENDIF			# "$(COMPONENT_FILES)"!=""

.IF "$(COMPONENT_JARFILES)"!=""
$(COMPONENT_JARFILES) : $(CLASSDIR)/$$(@:f)
    @@-$(MKDIRHIER) $(@:d)
    $(COMMAND_ECHO)$(COPY) $< $@
.ENDIF			# "$(COMPONENT_JARFILES)"!=""

.IF "$(COMPONENT_LIBRARIES)"!=""
PACK_RUNTIME!:=TRUE
# TODO(Q3): strip the binary?
$(COMPONENT_LIBRARIES) : $(DLLDEST)/$$(@:f)
    @@-$(MKDIRHIER) $(@:d)
    $(COMMAND_ECHO)$(COPY) $< $@
.ENDIF			# "$(COMPONENT_LIBRARIES)"!=""

.IF "$(PACK_RUNTIME)"!=""
PACK_RUNTIME_FLAG:=$(MISC)/$(TARGET)_pack_runtime.flag
$(EXTENSION_TARGET) : $(PACK_RUNTIME_FLAG)

$(EXTENSIONDIR) :
    @@-$(MKDIRHIER) $(@:d)

$(PACK_RUNTIME_FLAG) : $(EXTENSIONDIR)
    @@-$(MKDIRHIER) $(@:d)
.IF "$(OS)$(CPU)"=="WNTI"
.IF "$(COM)"=="GCC"
   $(GNUCOPY) $(SOLARBINDIR)/mingwm10.dll $(EXTENSIONDIR)
   .IF "$(MINGW_GCCDLL)"!=""
     $(GNUCOPY) $(SOLARBINDIR)/$(MINGW_GCCDLL) $(EXTENSIONDIR)
   .ENDIF
   .IF "$(MINGW_GXXDLL)"!=""
     $(GNUCOPY) $(SOLARBINDIR)/$(MINGW_GXXDLL) $(EXTENSIONDIR)
   .ENDIF
.ELSE
.IF "$(PACKMS)"!=""
.IF "$(CCNUMVER)" <= "001399999999"
    $(GNUCOPY) $(PACKMS)/msvcr71.dll $(EXTENSIONDIR)
    $(GNUCOPY) $(PACKMS)/msvcp71.dll $(EXTENSIONDIR)
.ELSE			# "$(CCNUMVER)" <= "001399999999"
.IF "$(CCNUMVER)" <= "001499999999"
    $(GNUCOPY) $(PACKMS)/msvcr80.dll $(EXTENSIONDIR)
    $(GNUCOPY) $(PACKMS)/msvcp80.dll $(EXTENSIONDIR)
    $(GNUCOPY) $(PACKMS)/msvcm80.dll $(EXTENSIONDIR)
    $(GNUCOPY) $(PACKMS)/Microsoft.VC80.CRT.manifest $(EXTENSIONDIR)
.ELSE			# "$(CCNUMVER)" <= "001499999999"
    $(GNUCOPY) $(PACKMS)/msvcr90.dll $(EXTENSIONDIR)
    $(GNUCOPY) $(PACKMS)/msvcp90.dll $(EXTENSIONDIR)
    $(GNUCOPY) $(PACKMS)/msvcm90.dll $(EXTENSIONDIR)
    $(GNUCOPY) $(PACKMS)/Microsoft.VC90.CRT.manifest $(EXTENSIONDIR)
.ENDIF 			# "$(CCNUMVER)" <= "001499999999"
.ENDIF			# "$(CCNUMVER)" <= "001399999999"
.ELSE        # "$(PACKMS)"!=""
.IF "$(CCNUMVER)" <= "001399999999"
    $(GNUCOPY) $(SOLARBINDIR)/msvcr71.dll $(EXTENSIONDIR)
    $(GNUCOPY) $(SOLARBINDIR)/msvcp71.dll $(EXTENSIONDIR)
.ELSE			# "$(CCNUMVER)" <= "001399999999"
.IF "$(CCNUMVER)" <= "001499999999"
    $(GNUCOPY) $(SOLARBINDIR)/msvcr80.dll $(EXTENSIONDIR)
    $(GNUCOPY) $(SOLARBINDIR)/msvcp80.dll $(EXTENSIONDIR)
    $(GNUCOPY) $(SOLARBINDIR)/msvcm80.dll $(EXTENSIONDIR)
    $(GNUCOPY) $(SOLARBINDIR)/Microsoft.VC80.CRT.manifest $(EXTENSIONDIR)
.ELSE    		# "$(CCNUMVER)" <= "001499999999"
    $(GNUCOPY) $(SOLARBINDIR)/msvcr90.dll $(EXTENSIONDIR)
    $(GNUCOPY) $(SOLARBINDIR)/msvcp90.dll $(EXTENSIONDIR)
    $(GNUCOPY) $(SOLARBINDIR)/msvcm90.dll $(EXTENSIONDIR)
    $(GNUCOPY) $(SOLARBINDIR)/Microsoft.VC90.CRT.manifest $(EXTENSIONDIR)
.ENDIF			# "$(CCNUMVER)" <= "001499999999"
.ENDIF			# "$(CCNUMVER)" <= "001399999999"
.ENDIF          # "$(PACKMS)"!=""
.ENDIF	#"$(COM)"=="GCC" 
.ENDIF 			# "$(OS)$(CPU)"=="WNTI"
    @$(TOUCH) $@
.ENDIF			# "$(PACK_RUNTIME)"!=""

IMPLEMENTATION_IDENTIFIER*="com.sun.star.$(EXTENSIONNAME)-$(PLATFORMID)"

.INCLUDE .IGNORE : $(MISC)/$(TARGET)_lang_track.mk
.IF "$(LAST_WITH_LANG)"!="$(WITH_LANG)"
PHONYDESC=.PHONY
.ENDIF			# "$(LAST_WITH_LANG)"!="$(WITH_LANG)"
.IF "$(DESCRIPTION)"!=""
$(DESCRIPTION) $(PHONYDESC) : $(DESCRIPTION_SRC)
    @@-$(MKDIRHIER) $(@:d)
    @echo LAST_WITH_LANG=$(WITH_LANG) > $(MISC)/$(TARGET)_lang_track.mk

    $(COMMAND_ECHO)$(PERL) $(SOLARENV)/bin/licinserter.pl $(DESCRIPTION_SRC) $(COMPONENT_LIC_TEMPL) $@.1.$(EXTNAME)

    $(COMMAND_ECHO)$(PERL) $(SOLARENV)/bin/transform_description.pl $@.1.$(EXTNAME) $@.2.$(EXTNAME)
    @@-$(RM) $@.1.$(EXTNAME)

    $(COMMAND_ECHO)$(TYPE) $@.2.$(EXTNAME) | sed s/UPDATED_IDENTIFIER/$(IMPLEMENTATION_IDENTIFIER)/ >  $@.3.$(EXTNAME)
    @@-$(RM) $@.2.$(EXTNAME)

    $(COMMAND_ECHO)$(TYPE) $@.3.$(EXTNAME) | sed s/UPDATED_SUPPORTED_PLATFORM/$(PLATFORMID)/ > $@
    @@-$(RM) $@.3.$(EXTNAME)

.ENDIF			# "$(DESCRIPTION)"!=""
# default OOo license text!!!
# may not fit...
.IF "$(CUSTOM_LICENSE)"==""
.IF "$(GUI)" == "WNT"
PACKLICDEPS=$(SOLARBINDIR)/osl/license$$(@:b:s/_/./:e:s/./_/)$$(@:e)
.ELSE			# "$(GUI)" == "WNT"
PACKLICDEPS=$(SOLARBINDIR)/osl/LICENSE$$(@:b:s/_/./:e:s/./_/)$$(@:e)
.ENDIF			# "$(GUI)" == "WNT"
.ELSE			# "$(CUSTOM_LICENSE)" == ""
PACKLICDEPS=$(CUSTOM_LICENSE)
.ENDIF			# "$(CUSTOM_LICENSE)" == ""
.IF "$(PACKLICS)"!=""
$(PACKLICS) : $(PACKLICDEPS)
    @@-$(MKDIRHIER) $(@:d)
    $(COMMAND_ECHO)$(GNUCOPY) $< $@
.ENDIF			# "$(PACKLICS)"!=""

.IF "$(COMPONENT_MANIFEST)"!=""
#$(COMPONENT_MANIFEST) : $$(@:f)
$(COMPONENT_MANIFEST) : $(MANIFEST_SRC) $(MANIFEST_DEPS)
    @@-$(MKDIRHIER) $(@:d)
.IF "$(COMPONENT_MANIFEST_GENERIC)" == ""
    $(COMMAND_ECHO)$(TYPE) $(MANIFEST_SRC) | $(SED) "s/SHARED_EXTENSION/$(DLLPOST)/" | $(SED) "s/EXEC_EXTENSION/$(EXECPOST)/" > $@
.ELSE			# "$(COMPONENT_MANIFEST_GENERIC)" != ""
    $(COMMAND_ECHO)$(PERL) $(SOLARENV)/bin/makemani.pl $(PRJ)/util/manifest.xml $(EXTENSIONDIR) $(COMPONENT_MANIFEST_SEARCHDIR) $(@:d:d)
.ENDIF			# "$(COMPONENT_MANIFEST_GENERIC)" != ""
.ENDIF			# "$(COMPONENT_MANIFEST)"!=""

ALLTAR : $(EXTENSION_PACKDEPS)
