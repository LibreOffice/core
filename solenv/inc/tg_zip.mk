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



#######################################################
# unroll begin

.IF "$(GUI)"=="WNT"
command_seperator=&&
#command_seperator=^
avoid_cvs_dir=-x "*CVS*" -x "*.svn*"
.ELIF "$(GUI)"=="OS2"
command_seperator=&&
.ELSE
command_seperator=;
avoid_cvs_dir=-x "*CVS*" -x "*.svn*"
 
.ENDIF

.IF "$(ZIP$(TNR)TARGET)"!=""

ZIP$(TNR)EXT*=.zip
.IF "$(common_build_zip)"!=""
.IF "$(ZIP$(TNR)LIST:s/LANGDIR//)" == "$(ZIP$(TNR)LIST)"
ZIP$(TNR)TARGETN=$(COMMONBIN)/$(ZIP$(TNR)TARGET)$(ZIP$(TNR)EXT)
.ELSE
ZIP$(TNR)TARGETN=$(foreach,i,$(zip$(TNR)alllangiso) $(COMMONBIN)/$(ZIP$(TNR)TARGET)_$i$(ZIP$(TNR)EXT) )
.ENDIF
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIP$(TNR)LIST:s/LANGDIR//)" == "$(ZIP$(TNR)LIST)"
ZIP$(TNR)TARGETN=$(BIN)/$(ZIP$(TNR)TARGET)$(ZIP$(TNR)EXT)
.ELSE
ZIP$(TNR)TARGETN=$(foreach,i,$(zip$(TNR)alllangiso) $(BIN)/$(ZIP$(TNR)TARGET)_$i$(ZIP$(TNR)EXT) )
.ENDIF
.ENDIF			# "$(common_build_zip)"!=""
ZIP$(TNR)DEPFILE=$(subst,$(COMMON_OUTDIR),$(OUTPATH) $(subst,/bin/,/misc/ $(ZIP$(TNR)TARGETN:s/$(ZIP$(TNR)EXT)/.dpzz/)))
ZIPDEPFILES+=$(ZIP$(TNR)DEPFILE)

ZIP$(TNR)DIR*=$(ZIPDIR)
ZIP$(TNR)FLAGS*=$(ZIPFLAGS)
.IF "$(zip$(TNR)generatedlangs)"!=""
zip$(TNR)langdirs*=$(alllangiso)
.ELSE           # "$(zip$(TNR)generatedlangs)"!=""
zip$(TNR)langdirs:=$(shell @-test -d {$(subst,/$(LANGDIR), $(null,$(ZIP$(TNR)DIR) . $(ZIP$(TNR)DIR)))}/ && find {$(subst,/$(LANGDIR), $(null,$(ZIP$(TNR)DIR) . $(ZIP$(TNR)DIR)))}/ -type d ! -name CVS ! -name ".svn" ! -name "." | sed "s/\.\/\///" | sed "s/\.\///" )
.ENDIF          # "$(zip$(TNR)generatedlangs)"!=""
.IF "$(ZIP$(TNR)FORCEALLLANG)"!=""
zip$(TNR)alllangiso:=$(foreach,i,$(completelangiso) $(foreach,j,$(zip$(TNR)langdirs) $(eq,$i,$j  $i $(NULL))))
.ELSE          # "$(ZIP$(TNR)ALLLANG)" != ""
zip$(TNR)alllangiso*:=$(foreach,i,$(alllangiso) $(foreach,j,$(zip$(TNR)langdirs) $(eq,$i,$j  $i $(NULL))))
.ENDIF          # "$(ZIP$(TNR)ALLLANG)" != ""
.ENDIF			# "$(ZIP$(TNR)TARGET)"!=""

.IF "$(ZIP$(TNR)TARGETN)"!=""

ZIP$(TNR)TMP:=$(mktmp iii)

$(MISC)/$(TARGET).$(PWD:f).$(ZIP$(TNR)TARGET).dpzz : $(ZIP$(TNR)TARGETN)

.IF "$(common_build_zip)"!=""
ZIP$(TNR)HELPVAR=$(COMMONBIN)/$(ZIP$(TNR)TARGET)
.ELSE			# "$(common_build_zip)"!=""
ZIP$(TNR)HELPVAR=$(BIN)/$(ZIP$(TNR)TARGET)
.ENDIF			# "$(common_build_zip)"!=""

$(ZIP$(TNR)DEPFILE) :
    @echo # > $(MISC)/$(@:f)
.IF "$(common_build_zip)"!=""
.IF "$(ZIP$(TNR)DIR)" != ""
    $(COMMAND_ECHO)-$(MKDIRHIER) $(ZIP$(TNR)DIR)
    $(COMMAND_ECHO)$(CDD) $(subst,LANGDIR,. $(subst,/LANGDIR, $(ZIP$(TNR)DIR))) $(command_seperator) $(ZIPDEP) $(ZIP$(TNR)FLAGS) -prefix $(subst,LANGDIR,. $(subst,/LANGDIR, $(ZIP$(TNR)DIR)))/ $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(subst,/misc/,/bin/ $(@:s/.dpzz/$(ZIP$(TNR)EXT)/))) $(foreach,j,$(ZIP$(TNR)LIST) "{$(subst,LANGDIR,{$(subst,$(ZIP$(TNR)TARGET)_, $(@:f:b))} $j )}") $(avoid_cvs_dir) >> $(PWD)/$(PRJ)/$(ROUT)/misc/$(@:f)
.ELSE			# "$(ZIP$(TNR)DIR)" != ""
    $(COMMAND_ECHO)-$(ZIPDEP) $(ZIP$(TNR)FLAGS) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(subst,/misc/,/bin/ $(@:s/.dpzz/$(ZIP$(TNR)EXT)/))) $(foreach,j,$(ZIP$(TNR)LIST) "{$(subst,LANGDIR,{$(subst,$(ZIP$(TNR)TARGET)_, $(@:f:b))} $j )}") $(avoid_cvs_dir) >> $(MISC)/$(@:f)
.ENDIF			# "$(ZIP$(TNR)DIR)" != ""
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIP$(TNR)DIR)" != ""
    $(COMMAND_ECHO)-$(MKDIRHIER) $(ZIP$(TNR)DIR)
    $(COMMAND_ECHO)$(CDD) $(subst,LANGDIR,. $(subst,/LANGDIR, $(ZIP$(TNR)DIR))) $(command_seperator) $(ZIPDEP) $(ZIP$(TNR)FLAGS) -prefix $(subst,LANGDIR,. $(subst,/LANGDIR, $(ZIP$(TNR)DIR)))/ $(subst,/misc/,/bin/ $(@:s/.dpzz/$(ZIP$(TNR)EXT)/)) $(foreach,j,$(ZIP$(TNR)LIST) "{$(subst,LANGDIR,{$(subst,$(BIN)/$(ZIP$(TNR)TARGET)_, $(subst,/misc/,/bin/ $(@:db)))} $j )}") $(avoid_cvs_dir) >> $(PWD)/$(PRJ)/$(ROUT)/misc/$(@:f)
.ELSE			# "$(ZIP$(TNR)DIR)" != ""
    $(COMMAND_ECHO)-$(ZIPDEP) $(ZIP$(TNR)FLAGS) $(subst,/misc/,/bin/ $(@:s/.dpzz/$(ZIP$(TNR)EXT)/)) $(foreach,j,$(ZIP$(TNR)LIST) "{$(subst,LANGDIR,{$(subst,$(BIN)/$(ZIP$(TNR)TARGET)_, $(subst,/misc/,/bin/ $(@:db)))} $j )}") $(avoid_cvs_dir) >> $(MISC)/$(@:f)
.ENDIF			# "$(ZIP$(TNR)DIR)" != ""
.ENDIF			# "$(common_build_zip)"!=""
    @echo $(@:n:s#\#/#) : makefile.mk >> $(MISC)/$(@:f)


$(ZIP$(TNR)TARGETN) : delzip $(ZIP$(TNR)DEPS)
    @echo "Making:   " $(@:f)
    @@$(!eq,$?,$(?:s/delzip/zzz/) -$(RM) echo) $(uniq $@ $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@))
    @$(eq,$?,$(?:s/delzip/zzz/) noop echo ) rebuilding zipfiles
    @echo ------------------------------ $(eq,$?,$(?:s/delzip/zzz/) >&$(NULLDEV) )
.IF "$(common_build_zip)"!=""
.IF "$(ZIP$(TNR)DIR)" != ""
    @@-$(GNUCOPY) -p $@ $(ZIP$(TNR)TMP).$(ZIP$(TNR)TARGET){$(subst,$(ZIP$(TNR)HELPVAR),_ $(@:db))}$(ZIP$(TNR)EXT)
    $(COMMAND_ECHO)-$(CDD) $(subst,LANGDIR,{$(subst,$(ZIP$(TNR)HELPVAR)_, $(@:db))} $(ZIP$(TNR)DIR)) $(command_seperator) zip $(ZIP_VERBOSITY) $(ZIP$(TNR)FLAGS) $(ZIP$(TNR)TMP).$(ZIP$(TNR)TARGET){$(subst,$(ZIP$(TNR)HELPVAR),_ $(@:db))}$(ZIP$(TNR)EXT) $(subst,LANGDIR_away/, $(ZIP$(TNR)LIST:s/LANGDIR/LANGDIR_away/)) -x delzip $(avoid_cvs_dir) $(CHECKZIPRESULT)
    $(COMMAND_ECHO)$(IFEXIST) $(ZIP$(TNR)TMP).$(ZIP$(TNR)TARGET){$(subst,$(ZIP$(TNR)HELPVAR),_ $(@:db))}$(ZIP$(TNR)EXT) $(THEN) \
        $(PERL) -w $(SOLARENV)/bin/cleanzip.pl $(ZIP$(TNR)TMP).$(ZIP$(TNR)TARGET){$(subst,$(ZIP$(TNR)HELPVAR),_ $(@:db))}$(ZIP$(TNR)EXT) \
        $(FI)
    $(COMMAND_ECHO)$(COPY) $(ZIP$(TNR)TMP).$(ZIP$(TNR)TARGET){$(subst,$(ZIP$(TNR)HELPVAR),_ $(@:db))}$(ZIP$(TNR)EXT) $@.$(INPATH)
    $(COMMAND_ECHO)$(RM) $(ZIP$(TNR)TMP).$(ZIP$(TNR)TARGET){$(subst,$(ZIP$(TNR)HELPVAR),_ $(@:db))}$(ZIP$(TNR)EXT)
.ELSE			# "$(ZIP$(TNR)DIR)" != ""
    @@-$(GNUCOPY) -p $@ $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)
    $(COMMAND_ECHO)zip $(ZIP_VERBOSITY) $(ZIP$(TNR)FLAGS) $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $(foreach,j,$(ZIP$(TNR)LIST) $(subst,LANGDIR,{$(subst,$(ZIP$(TNR)HELPVAR)_, $(@:db))} $j )) -x delzip $(avoid_cvs_dir) $(CHECKZIPRESULT)
    $(COMMAND_ECHO)$(IFEXIST) $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $(THEN) \
        $(PERL) -w $(SOLARENV)/bin/cleanzip.pl $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) \
        $(FI)
    @@-$(GNUCOPY) -p $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $@.$(INPATH)
    @-$(RM) $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)
.ENDIF			# "$(ZIP$(TNR)DIR)" != ""
    @@-$(RM) $@
    @$(IFEXIST) $@.$(INPATH) $(THEN) $(RENAME:s/+//) $@.$(INPATH) $@ $(FI)
#	@$(IFEXIST) $@ $(THEN) $(TOUCH) $@ $(FI)  # even if it's not used...
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIP$(TNR)DIR)" != ""
    @@-$(GNUCOPY) -p $@ $(ZIP$(TNR)TMP).$(ZIP$(TNR)TARGET){$(subst,$(ZIP$(TNR)HELPVAR),_ $(@:db))}$(ZIP$(TNR)EXT)
    $(COMMAND_ECHO)-$(CDD) $(subst,LANGDIR,{$(subst,$(ZIP$(TNR)HELPVAR)_, $(@:db))} $(ZIP$(TNR)DIR)) $(command_seperator) zip $(ZIP_VERBOSITY) $(ZIP$(TNR)FLAGS) $(ZIP$(TNR)TMP).$(ZIP$(TNR)TARGET){$(subst,$(ZIP$(TNR)HELPVAR),_ $(@:db))}$(ZIP$(TNR)EXT) $(subst,LANGDIR_away/, $(ZIP$(TNR)LIST:s/LANGDIR/LANGDIR_away/)) -x delzip  $(avoid_cvs_dir) $(CHECKZIPRESULT)
    $(COMMAND_ECHO)$(IFEXIST) $(ZIP$(TNR)TMP).$(ZIP$(TNR)TARGET){$(subst,$(ZIP$(TNR)HELPVAR),_ $(@:db))}$(ZIP$(TNR)EXT) $(THEN) \
        $(PERL) -w $(SOLARENV)/bin/cleanzip.pl $(ZIP$(TNR)TMP).$(ZIP$(TNR)TARGET){$(subst,$(ZIP$(TNR)HELPVAR),_ $(@:db))}$(ZIP$(TNR)EXT) \
        $(FI)
    $(COMMAND_ECHO)$(COPY) $(ZIP$(TNR)TMP).$(ZIP$(TNR)TARGET){$(subst,$(ZIP$(TNR)HELPVAR),_ $(@:db))}$(ZIP$(TNR)EXT)  $@ 
    $(COMMAND_ECHO)$(RM) $(ZIP$(TNR)TMP).$(ZIP$(TNR)TARGET){$(subst,$(ZIP$(TNR)HELPVAR),_ $(@:db))}$(ZIP$(TNR)EXT)
.ELSE			# "$(ZIP$(TNR)DIR)" != ""
    $(COMMAND_ECHO)zip $(ZIP_VERBOSITY) $(ZIP$(TNR)FLAGS) $@ $(foreach,j,$(ZIP$(TNR)LIST) $(subst,LANGDIR,{$(subst,$(BIN)/$(ZIP$(TNR)TARGET)_, $(@:db))} $j )) -x delzip $(avoid_cvs_dir) $(CHECKZIPRESULT)
    $(COMMAND_ECHO)$(IFEXIST) $@ $(THEN) \
        $(PERL) -w $(SOLARENV)/bin/cleanzip.pl $@ \
        $(FI)
.ENDIF			# "$(ZIP$(TNR)DIR)" != ""
.ENDIF			# "$(common_build_zip)"!=""
.ENDIF

# unroll end
#######################################################

