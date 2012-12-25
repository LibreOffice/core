# unroll begin

.IF "$(OS)"=="WNT"
command_seperator=&&
#command_seperator=^
.ELSE
command_seperator=;
 
.ENDIF

.IF "$(ZIP1TARGET)"!=""

.IF "$(ZIP1STRIPLANGUAGETAGS)" != ""
CALLXSLTPROC:=$(XSLTPROC)
.EXPORT: CALLXSLTPROC
.ENDIF

ZIP1EXT*=.zip
.IF "$(ZIP1LIST:s/LANGDIR//)" == "$(ZIP1LIST)"
ZIP1TARGETN=$(BIN)/$(ZIP1TARGET)$(ZIP1EXT)
.ELSE
ZIP1TARGETN=$(foreach,i,$(zip1alllangiso) $(BIN)/$(ZIP1TARGET)_$i$(ZIP1EXT) )
.ENDIF
ZIP1DEPFILE=$(subst,$(COMMON_OUTDIR),$(OUTPATH) $(subst,/bin/,/misc/ $(ZIP1TARGETN:s/$(ZIP1EXT)/.dpzz/)))
ZIPDEPFILES+=$(ZIP1DEPFILE)

ZIP1DIR*=$(ZIPDIR)
ZIP1FLAGS*=$(ZIPFLAGS)
.IF "$(zip1generatedlangs)"!=""
zip1langdirs*=$(alllangiso)
.ELSE           # "$(zip1generatedlangs)"!=""
zip1langdirs:=$(shell @-test -d {$(subst,/$(LANGDIR), $(null,$(ZIP1DIR) . $(ZIP1DIR)))}/ && find {$(subst,/$(LANGDIR), $(null,$(ZIP1DIR) . $(ZIP1DIR)))}/ -type d ! -name CVS ! -name ".svn" ! -name "." | sed "s/\.\/\///" | sed "s/\.\///" )
.ENDIF          # "$(zip1generatedlangs)"!=""
.IF "$(ZIP1FORCEALLLANG)"!=""
zip1alllangiso:=$(foreach,i,$(completelangiso) $(foreach,j,$(zip1langdirs) $(eq,$i,$j  $i $(NULL))))
.ELSE          # "$(ZIP1ALLLANG)" != ""
zip1alllangiso*:=$(foreach,i,$(alllangiso) $(foreach,j,$(zip1langdirs) $(eq,$i,$j  $i $(NULL))))
.ENDIF          # "$(ZIP1ALLLANG)" != ""
.ENDIF			# "$(ZIP1TARGET)"!=""

.IF "$(ZIP1TARGETN)"!=""

ZIP1TMP:=$(mktmp iii)

$(MISC)/$(TARGET).$(PWD:f).$(ZIP1TARGET).dpzz : $(ZIP1TARGETN)

ZIP1HELPVAR=$(BIN)/$(ZIP1TARGET)

$(ZIP1DEPFILE) :
.IF "$(ZIP1DIR)" != ""
    $(COMMAND_ECHO)-$(MKDIRHIER) $(ZIP1DIR)
    $(COMMAND_ECHO)$(CDD) $(subst,LANGDIR,. $(subst,/LANGDIR, $(ZIP1DIR))) $(command_seperator) $(ZIPDEP) $(ZIP1FLAGS) -prefix $(subst,LANGDIR,. $(subst,/LANGDIR, $(ZIP1DIR)))/ $(subst,/misc/,/bin/ $(@:s/.dpzz/$(ZIP1EXT)/)) $(foreach,j,$(ZIP1LIST) "{$(subst,LANGDIR,{$(subst,$(BIN)/$(ZIP1TARGET)_, $(subst,/misc/,/bin/ $(@:db)))} $j )}") >> $(PWD)/$(PRJ)/$(ROUT)/misc/$(@:f)
.ELSE			# "$(ZIP1DIR)" != ""
    $(COMMAND_ECHO)-$(ZIPDEP) $(ZIP1FLAGS) $(subst,/misc/,/bin/ $(@:s/.dpzz/$(ZIP1EXT)/)) $(foreach,j,$(ZIP1LIST) "{$(subst,LANGDIR,{$(subst,$(BIN)/$(ZIP1TARGET)_, $(subst,/misc/,/bin/ $(@:db)))} $j )}") >> $(MISC)/$(@:f)
.ENDIF			# "$(ZIP1DIR)" != ""
    @echo $(@:n:s#\#/#) : makefile.mk >> $(MISC)/$(@:f)


$(ZIP1TARGETN) : delzip $(ZIP1DEPS)
    @echo "[ building ZIP ]" $(@:f)
    @@$(!eq,$?,$(?:s/delzip/zzz/) -$(RM) echo) $(uniq $@ $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@))
.IF "$(ZIP1DIR)" != ""
    @@-$(GNUCOPY) -p $@ $(ZIP1TMP).$(ZIP1TARGET){$(subst,$(ZIP1HELPVAR),_ $(@:db))}$(ZIP1EXT)
    $(COMMAND_ECHO)-$(CDD) $(subst,LANGDIR,{$(subst,$(ZIP1HELPVAR)_, $(@:db))} $(ZIP1DIR)) $(command_seperator) zip $(ZIP_VERBOSITY) $(ZIP1FLAGS) $(ZIP1TMP).$(ZIP1TARGET){$(subst,$(ZIP1HELPVAR),_ $(@:db))}$(ZIP1EXT) $(subst,LANGDIR_away/, $(ZIP1LIST:s/LANGDIR/LANGDIR_away/)) -x delzip  $(CHECKZIPRESULT)
    $(COMMAND_ECHO)$(COPY) $(ZIP1TMP).$(ZIP1TARGET){$(subst,$(ZIP1HELPVAR),_ $(@:db))}$(ZIP1EXT)  $@ 
    $(COMMAND_ECHO)$(RM) $(ZIP1TMP).$(ZIP1TARGET){$(subst,$(ZIP1HELPVAR),_ $(@:db))}$(ZIP1EXT)
.ELSE			# "$(ZIP1DIR)" != ""
    $(COMMAND_ECHO)zip $(ZIP_VERBOSITY) $(ZIP1FLAGS) $@ $(foreach,j,$(ZIP1LIST) $(subst,LANGDIR,{$(subst,$(BIN)/$(ZIP1TARGET)_, $(@:db))} $j )) -x delzip $(CHECKZIPRESULT)
.IF "$(ZIP1STRIPLANGUAGETAGS)" != ""
    $(COMMAND_ECHO)$(IFEXIST) $@ $(THEN) \
        $(SOLARENV)/bin/striplanguagetags.sh $@ \
        $(FI)
.ENDIF
.ENDIF			# "$(ZIP1DIR)" != ""
.ENDIF

# unroll begin

.IF "$(OS)"=="WNT"
command_seperator=&&
#command_seperator=^
.ELSE
command_seperator=;
 
.ENDIF

.IF "$(ZIP2TARGET)"!=""

.IF "$(ZIP2STRIPLANGUAGETAGS)" != ""
CALLXSLTPROC:=$(XSLTPROC)
.EXPORT: CALLXSLTPROC
.ENDIF

ZIP2EXT*=.zip
.IF "$(ZIP2LIST:s/LANGDIR//)" == "$(ZIP2LIST)"
ZIP2TARGETN=$(BIN)/$(ZIP2TARGET)$(ZIP2EXT)
.ELSE
ZIP2TARGETN=$(foreach,i,$(zip2alllangiso) $(BIN)/$(ZIP2TARGET)_$i$(ZIP2EXT) )
.ENDIF
ZIP2DEPFILE=$(subst,$(COMMON_OUTDIR),$(OUTPATH) $(subst,/bin/,/misc/ $(ZIP2TARGETN:s/$(ZIP2EXT)/.dpzz/)))
ZIPDEPFILES+=$(ZIP2DEPFILE)

ZIP2DIR*=$(ZIPDIR)
ZIP2FLAGS*=$(ZIPFLAGS)
.IF "$(zip2generatedlangs)"!=""
zip2langdirs*=$(alllangiso)
.ELSE           # "$(zip2generatedlangs)"!=""
zip2langdirs:=$(shell @-test -d {$(subst,/$(LANGDIR), $(null,$(ZIP2DIR) . $(ZIP2DIR)))}/ && find {$(subst,/$(LANGDIR), $(null,$(ZIP2DIR) . $(ZIP2DIR)))}/ -type d ! -name CVS ! -name ".svn" ! -name "." | sed "s/\.\/\///" | sed "s/\.\///" )
.ENDIF          # "$(zip2generatedlangs)"!=""
.IF "$(ZIP2FORCEALLLANG)"!=""
zip2alllangiso:=$(foreach,i,$(completelangiso) $(foreach,j,$(zip2langdirs) $(eq,$i,$j  $i $(NULL))))
.ELSE          # "$(ZIP2ALLLANG)" != ""
zip2alllangiso*:=$(foreach,i,$(alllangiso) $(foreach,j,$(zip2langdirs) $(eq,$i,$j  $i $(NULL))))
.ENDIF          # "$(ZIP2ALLLANG)" != ""
.ENDIF			# "$(ZIP2TARGET)"!=""

.IF "$(ZIP2TARGETN)"!=""

ZIP2TMP:=$(mktmp iii)

$(MISC)/$(TARGET).$(PWD:f).$(ZIP2TARGET).dpzz : $(ZIP2TARGETN)

ZIP2HELPVAR=$(BIN)/$(ZIP2TARGET)

$(ZIP2DEPFILE) :
.IF "$(ZIP2DIR)" != ""
    $(COMMAND_ECHO)-$(MKDIRHIER) $(ZIP2DIR)
    $(COMMAND_ECHO)$(CDD) $(subst,LANGDIR,. $(subst,/LANGDIR, $(ZIP2DIR))) $(command_seperator) $(ZIPDEP) $(ZIP2FLAGS) -prefix $(subst,LANGDIR,. $(subst,/LANGDIR, $(ZIP2DIR)))/ $(subst,/misc/,/bin/ $(@:s/.dpzz/$(ZIP2EXT)/)) $(foreach,j,$(ZIP2LIST) "{$(subst,LANGDIR,{$(subst,$(BIN)/$(ZIP2TARGET)_, $(subst,/misc/,/bin/ $(@:db)))} $j )}") >> $(PWD)/$(PRJ)/$(ROUT)/misc/$(@:f)
.ELSE			# "$(ZIP2DIR)" != ""
    $(COMMAND_ECHO)-$(ZIPDEP) $(ZIP2FLAGS) $(subst,/misc/,/bin/ $(@:s/.dpzz/$(ZIP2EXT)/)) $(foreach,j,$(ZIP2LIST) "{$(subst,LANGDIR,{$(subst,$(BIN)/$(ZIP2TARGET)_, $(subst,/misc/,/bin/ $(@:db)))} $j )}") >> $(MISC)/$(@:f)
.ENDIF			# "$(ZIP2DIR)" != ""
    @echo $(@:n:s#\#/#) : makefile.mk >> $(MISC)/$(@:f)


$(ZIP2TARGETN) : delzip $(ZIP2DEPS)
    @echo "[ building ZIP ]" $(@:f)
    @@$(!eq,$?,$(?:s/delzip/zzz/) -$(RM) echo) $(uniq $@ $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@))
.IF "$(ZIP2DIR)" != ""
    @@-$(GNUCOPY) -p $@ $(ZIP2TMP).$(ZIP2TARGET){$(subst,$(ZIP2HELPVAR),_ $(@:db))}$(ZIP2EXT)
    $(COMMAND_ECHO)-$(CDD) $(subst,LANGDIR,{$(subst,$(ZIP2HELPVAR)_, $(@:db))} $(ZIP2DIR)) $(command_seperator) zip $(ZIP_VERBOSITY) $(ZIP2FLAGS) $(ZIP2TMP).$(ZIP2TARGET){$(subst,$(ZIP2HELPVAR),_ $(@:db))}$(ZIP2EXT) $(subst,LANGDIR_away/, $(ZIP2LIST:s/LANGDIR/LANGDIR_away/)) -x delzip  $(CHECKZIPRESULT)
    $(COMMAND_ECHO)$(COPY) $(ZIP2TMP).$(ZIP2TARGET){$(subst,$(ZIP2HELPVAR),_ $(@:db))}$(ZIP2EXT)  $@ 
    $(COMMAND_ECHO)$(RM) $(ZIP2TMP).$(ZIP2TARGET){$(subst,$(ZIP2HELPVAR),_ $(@:db))}$(ZIP2EXT)
.ELSE			# "$(ZIP2DIR)" != ""
    $(COMMAND_ECHO)zip $(ZIP_VERBOSITY) $(ZIP2FLAGS) $@ $(foreach,j,$(ZIP2LIST) $(subst,LANGDIR,{$(subst,$(BIN)/$(ZIP2TARGET)_, $(@:db))} $j )) -x delzip $(CHECKZIPRESULT)
.IF "$(ZIP2STRIPLANGUAGETAGS)" != ""
    $(COMMAND_ECHO)$(IFEXIST) $@ $(THEN) \
        $(SOLARENV)/bin/striplanguagetags.sh $@ \
        $(FI)
.ENDIF
.ENDIF			# "$(ZIP2DIR)" != ""
.ENDIF

# unroll begin

.IF "$(OS)"=="WNT"
command_seperator=&&
#command_seperator=^
.ELSE
command_seperator=;
 
.ENDIF

.IF "$(ZIP3TARGET)"!=""

.IF "$(ZIP3STRIPLANGUAGETAGS)" != ""
CALLXSLTPROC:=$(XSLTPROC)
.EXPORT: CALLXSLTPROC
.ENDIF

ZIP3EXT*=.zip
.IF "$(ZIP3LIST:s/LANGDIR//)" == "$(ZIP3LIST)"
ZIP3TARGETN=$(BIN)/$(ZIP3TARGET)$(ZIP3EXT)
.ELSE
ZIP3TARGETN=$(foreach,i,$(zip3alllangiso) $(BIN)/$(ZIP3TARGET)_$i$(ZIP3EXT) )
.ENDIF
ZIP3DEPFILE=$(subst,$(COMMON_OUTDIR),$(OUTPATH) $(subst,/bin/,/misc/ $(ZIP3TARGETN:s/$(ZIP3EXT)/.dpzz/)))
ZIPDEPFILES+=$(ZIP3DEPFILE)

ZIP3DIR*=$(ZIPDIR)
ZIP3FLAGS*=$(ZIPFLAGS)
.IF "$(zip3generatedlangs)"!=""
zip3langdirs*=$(alllangiso)
.ELSE           # "$(zip3generatedlangs)"!=""
zip3langdirs:=$(shell @-test -d {$(subst,/$(LANGDIR), $(null,$(ZIP3DIR) . $(ZIP3DIR)))}/ && find {$(subst,/$(LANGDIR), $(null,$(ZIP3DIR) . $(ZIP3DIR)))}/ -type d ! -name CVS ! -name ".svn" ! -name "." | sed "s/\.\/\///" | sed "s/\.\///" )
.ENDIF          # "$(zip3generatedlangs)"!=""
.IF "$(ZIP3FORCEALLLANG)"!=""
zip3alllangiso:=$(foreach,i,$(completelangiso) $(foreach,j,$(zip3langdirs) $(eq,$i,$j  $i $(NULL))))
.ELSE          # "$(ZIP3ALLLANG)" != ""
zip3alllangiso*:=$(foreach,i,$(alllangiso) $(foreach,j,$(zip3langdirs) $(eq,$i,$j  $i $(NULL))))
.ENDIF          # "$(ZIP3ALLLANG)" != ""
.ENDIF			# "$(ZIP3TARGET)"!=""

.IF "$(ZIP3TARGETN)"!=""

ZIP3TMP:=$(mktmp iii)

$(MISC)/$(TARGET).$(PWD:f).$(ZIP3TARGET).dpzz : $(ZIP3TARGETN)

ZIP3HELPVAR=$(BIN)/$(ZIP3TARGET)

$(ZIP3DEPFILE) :
.IF "$(ZIP3DIR)" != ""
    $(COMMAND_ECHO)-$(MKDIRHIER) $(ZIP3DIR)
    $(COMMAND_ECHO)$(CDD) $(subst,LANGDIR,. $(subst,/LANGDIR, $(ZIP3DIR))) $(command_seperator) $(ZIPDEP) $(ZIP3FLAGS) -prefix $(subst,LANGDIR,. $(subst,/LANGDIR, $(ZIP3DIR)))/ $(subst,/misc/,/bin/ $(@:s/.dpzz/$(ZIP3EXT)/)) $(foreach,j,$(ZIP3LIST) "{$(subst,LANGDIR,{$(subst,$(BIN)/$(ZIP3TARGET)_, $(subst,/misc/,/bin/ $(@:db)))} $j )}") >> $(PWD)/$(PRJ)/$(ROUT)/misc/$(@:f)
.ELSE			# "$(ZIP3DIR)" != ""
    $(COMMAND_ECHO)-$(ZIPDEP) $(ZIP3FLAGS) $(subst,/misc/,/bin/ $(@:s/.dpzz/$(ZIP3EXT)/)) $(foreach,j,$(ZIP3LIST) "{$(subst,LANGDIR,{$(subst,$(BIN)/$(ZIP3TARGET)_, $(subst,/misc/,/bin/ $(@:db)))} $j )}") >> $(MISC)/$(@:f)
.ENDIF			# "$(ZIP3DIR)" != ""
    @echo $(@:n:s#\#/#) : makefile.mk >> $(MISC)/$(@:f)


$(ZIP3TARGETN) : delzip $(ZIP3DEPS)
    @echo "[ building ZIP ]" $(@:f)
    @@$(!eq,$?,$(?:s/delzip/zzz/) -$(RM) echo) $(uniq $@ $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@))
.IF "$(ZIP3DIR)" != ""
    @@-$(GNUCOPY) -p $@ $(ZIP3TMP).$(ZIP3TARGET){$(subst,$(ZIP3HELPVAR),_ $(@:db))}$(ZIP3EXT)
    $(COMMAND_ECHO)-$(CDD) $(subst,LANGDIR,{$(subst,$(ZIP3HELPVAR)_, $(@:db))} $(ZIP3DIR)) $(command_seperator) zip $(ZIP_VERBOSITY) $(ZIP3FLAGS) $(ZIP3TMP).$(ZIP3TARGET){$(subst,$(ZIP3HELPVAR),_ $(@:db))}$(ZIP3EXT) $(subst,LANGDIR_away/, $(ZIP3LIST:s/LANGDIR/LANGDIR_away/)) -x delzip  $(CHECKZIPRESULT)
    $(COMMAND_ECHO)$(COPY) $(ZIP3TMP).$(ZIP3TARGET){$(subst,$(ZIP3HELPVAR),_ $(@:db))}$(ZIP3EXT)  $@ 
    $(COMMAND_ECHO)$(RM) $(ZIP3TMP).$(ZIP3TARGET){$(subst,$(ZIP3HELPVAR),_ $(@:db))}$(ZIP3EXT)
.ELSE			# "$(ZIP3DIR)" != ""
    $(COMMAND_ECHO)zip $(ZIP_VERBOSITY) $(ZIP3FLAGS) $@ $(foreach,j,$(ZIP3LIST) $(subst,LANGDIR,{$(subst,$(BIN)/$(ZIP3TARGET)_, $(@:db))} $j )) -x delzip $(CHECKZIPRESULT)
.IF "$(ZIP3STRIPLANGUAGETAGS)" != ""
    $(COMMAND_ECHO)$(IFEXIST) $@ $(THEN) \
        $(SOLARENV)/bin/striplanguagetags.sh $@ \
        $(FI)
.ENDIF
.ENDIF			# "$(ZIP3DIR)" != ""
.ENDIF

# unroll begin

.IF "$(OS)"=="WNT"
command_seperator=&&
#command_seperator=^
.ELSE
command_seperator=;
 
.ENDIF

.IF "$(ZIP4TARGET)"!=""

.IF "$(ZIP4STRIPLANGUAGETAGS)" != ""
CALLXSLTPROC:=$(XSLTPROC)
.EXPORT: CALLXSLTPROC
.ENDIF

ZIP4EXT*=.zip
.IF "$(ZIP4LIST:s/LANGDIR//)" == "$(ZIP4LIST)"
ZIP4TARGETN=$(BIN)/$(ZIP4TARGET)$(ZIP4EXT)
.ELSE
ZIP4TARGETN=$(foreach,i,$(zip4alllangiso) $(BIN)/$(ZIP4TARGET)_$i$(ZIP4EXT) )
.ENDIF
ZIP4DEPFILE=$(subst,$(COMMON_OUTDIR),$(OUTPATH) $(subst,/bin/,/misc/ $(ZIP4TARGETN:s/$(ZIP4EXT)/.dpzz/)))
ZIPDEPFILES+=$(ZIP4DEPFILE)

ZIP4DIR*=$(ZIPDIR)
ZIP4FLAGS*=$(ZIPFLAGS)
.IF "$(zip4generatedlangs)"!=""
zip4langdirs*=$(alllangiso)
.ELSE           # "$(zip4generatedlangs)"!=""
zip4langdirs:=$(shell @-test -d {$(subst,/$(LANGDIR), $(null,$(ZIP4DIR) . $(ZIP4DIR)))}/ && find {$(subst,/$(LANGDIR), $(null,$(ZIP4DIR) . $(ZIP4DIR)))}/ -type d ! -name CVS ! -name ".svn" ! -name "." | sed "s/\.\/\///" | sed "s/\.\///" )
.ENDIF          # "$(zip4generatedlangs)"!=""
.IF "$(ZIP4FORCEALLLANG)"!=""
zip4alllangiso:=$(foreach,i,$(completelangiso) $(foreach,j,$(zip4langdirs) $(eq,$i,$j  $i $(NULL))))
.ELSE          # "$(ZIP4ALLLANG)" != ""
zip4alllangiso*:=$(foreach,i,$(alllangiso) $(foreach,j,$(zip4langdirs) $(eq,$i,$j  $i $(NULL))))
.ENDIF          # "$(ZIP4ALLLANG)" != ""
.ENDIF			# "$(ZIP4TARGET)"!=""

.IF "$(ZIP4TARGETN)"!=""

ZIP4TMP:=$(mktmp iii)

$(MISC)/$(TARGET).$(PWD:f).$(ZIP4TARGET).dpzz : $(ZIP4TARGETN)

ZIP4HELPVAR=$(BIN)/$(ZIP4TARGET)

$(ZIP4DEPFILE) :
.IF "$(ZIP4DIR)" != ""
    $(COMMAND_ECHO)-$(MKDIRHIER) $(ZIP4DIR)
    $(COMMAND_ECHO)$(CDD) $(subst,LANGDIR,. $(subst,/LANGDIR, $(ZIP4DIR))) $(command_seperator) $(ZIPDEP) $(ZIP4FLAGS) -prefix $(subst,LANGDIR,. $(subst,/LANGDIR, $(ZIP4DIR)))/ $(subst,/misc/,/bin/ $(@:s/.dpzz/$(ZIP4EXT)/)) $(foreach,j,$(ZIP4LIST) "{$(subst,LANGDIR,{$(subst,$(BIN)/$(ZIP4TARGET)_, $(subst,/misc/,/bin/ $(@:db)))} $j )}") >> $(PWD)/$(PRJ)/$(ROUT)/misc/$(@:f)
.ELSE			# "$(ZIP4DIR)" != ""
    $(COMMAND_ECHO)-$(ZIPDEP) $(ZIP4FLAGS) $(subst,/misc/,/bin/ $(@:s/.dpzz/$(ZIP4EXT)/)) $(foreach,j,$(ZIP4LIST) "{$(subst,LANGDIR,{$(subst,$(BIN)/$(ZIP4TARGET)_, $(subst,/misc/,/bin/ $(@:db)))} $j )}") >> $(MISC)/$(@:f)
.ENDIF			# "$(ZIP4DIR)" != ""
    @echo $(@:n:s#\#/#) : makefile.mk >> $(MISC)/$(@:f)


$(ZIP4TARGETN) : delzip $(ZIP4DEPS)
    @echo "[ building ZIP ]" $(@:f)
    @@$(!eq,$?,$(?:s/delzip/zzz/) -$(RM) echo) $(uniq $@ $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@))
.IF "$(ZIP4DIR)" != ""
    @@-$(GNUCOPY) -p $@ $(ZIP4TMP).$(ZIP4TARGET){$(subst,$(ZIP4HELPVAR),_ $(@:db))}$(ZIP4EXT)
    $(COMMAND_ECHO)-$(CDD) $(subst,LANGDIR,{$(subst,$(ZIP4HELPVAR)_, $(@:db))} $(ZIP4DIR)) $(command_seperator) zip $(ZIP_VERBOSITY) $(ZIP4FLAGS) $(ZIP4TMP).$(ZIP4TARGET){$(subst,$(ZIP4HELPVAR),_ $(@:db))}$(ZIP4EXT) $(subst,LANGDIR_away/, $(ZIP4LIST:s/LANGDIR/LANGDIR_away/)) -x delzip  $(CHECKZIPRESULT)
    $(COMMAND_ECHO)$(COPY) $(ZIP4TMP).$(ZIP4TARGET){$(subst,$(ZIP4HELPVAR),_ $(@:db))}$(ZIP4EXT)  $@ 
    $(COMMAND_ECHO)$(RM) $(ZIP4TMP).$(ZIP4TARGET){$(subst,$(ZIP4HELPVAR),_ $(@:db))}$(ZIP4EXT)
.ELSE			# "$(ZIP4DIR)" != ""
    $(COMMAND_ECHO)zip $(ZIP_VERBOSITY) $(ZIP4FLAGS) $@ $(foreach,j,$(ZIP4LIST) $(subst,LANGDIR,{$(subst,$(BIN)/$(ZIP4TARGET)_, $(@:db))} $j )) -x delzip $(CHECKZIPRESULT)
.IF "$(ZIP4STRIPLANGUAGETAGS)" != ""
    $(COMMAND_ECHO)$(IFEXIST) $@ $(THEN) \
        $(SOLARENV)/bin/striplanguagetags.sh $@ \
        $(FI)
.ENDIF
.ENDIF			# "$(ZIP4DIR)" != ""
.ENDIF

# unroll begin

.IF "$(OS)"=="WNT"
command_seperator=&&
#command_seperator=^
.ELSE
command_seperator=;
 
.ENDIF

.IF "$(ZIP5TARGET)"!=""

.IF "$(ZIP5STRIPLANGUAGETAGS)" != ""
CALLXSLTPROC:=$(XSLTPROC)
.EXPORT: CALLXSLTPROC
.ENDIF

ZIP5EXT*=.zip
.IF "$(ZIP5LIST:s/LANGDIR//)" == "$(ZIP5LIST)"
ZIP5TARGETN=$(BIN)/$(ZIP5TARGET)$(ZIP5EXT)
.ELSE
ZIP5TARGETN=$(foreach,i,$(zip5alllangiso) $(BIN)/$(ZIP5TARGET)_$i$(ZIP5EXT) )
.ENDIF
ZIP5DEPFILE=$(subst,$(COMMON_OUTDIR),$(OUTPATH) $(subst,/bin/,/misc/ $(ZIP5TARGETN:s/$(ZIP5EXT)/.dpzz/)))
ZIPDEPFILES+=$(ZIP5DEPFILE)

ZIP5DIR*=$(ZIPDIR)
ZIP5FLAGS*=$(ZIPFLAGS)
.IF "$(zip5generatedlangs)"!=""
zip5langdirs*=$(alllangiso)
.ELSE           # "$(zip5generatedlangs)"!=""
zip5langdirs:=$(shell @-test -d {$(subst,/$(LANGDIR), $(null,$(ZIP5DIR) . $(ZIP5DIR)))}/ && find {$(subst,/$(LANGDIR), $(null,$(ZIP5DIR) . $(ZIP5DIR)))}/ -type d ! -name CVS ! -name ".svn" ! -name "." | sed "s/\.\/\///" | sed "s/\.\///" )
.ENDIF          # "$(zip5generatedlangs)"!=""
.IF "$(ZIP5FORCEALLLANG)"!=""
zip5alllangiso:=$(foreach,i,$(completelangiso) $(foreach,j,$(zip5langdirs) $(eq,$i,$j  $i $(NULL))))
.ELSE          # "$(ZIP5ALLLANG)" != ""
zip5alllangiso*:=$(foreach,i,$(alllangiso) $(foreach,j,$(zip5langdirs) $(eq,$i,$j  $i $(NULL))))
.ENDIF          # "$(ZIP5ALLLANG)" != ""
.ENDIF			# "$(ZIP5TARGET)"!=""

.IF "$(ZIP5TARGETN)"!=""

ZIP5TMP:=$(mktmp iii)

$(MISC)/$(TARGET).$(PWD:f).$(ZIP5TARGET).dpzz : $(ZIP5TARGETN)

ZIP5HELPVAR=$(BIN)/$(ZIP5TARGET)

$(ZIP5DEPFILE) :
.IF "$(ZIP5DIR)" != ""
    $(COMMAND_ECHO)-$(MKDIRHIER) $(ZIP5DIR)
    $(COMMAND_ECHO)$(CDD) $(subst,LANGDIR,. $(subst,/LANGDIR, $(ZIP5DIR))) $(command_seperator) $(ZIPDEP) $(ZIP5FLAGS) -prefix $(subst,LANGDIR,. $(subst,/LANGDIR, $(ZIP5DIR)))/ $(subst,/misc/,/bin/ $(@:s/.dpzz/$(ZIP5EXT)/)) $(foreach,j,$(ZIP5LIST) "{$(subst,LANGDIR,{$(subst,$(BIN)/$(ZIP5TARGET)_, $(subst,/misc/,/bin/ $(@:db)))} $j )}") >> $(PWD)/$(PRJ)/$(ROUT)/misc/$(@:f)
.ELSE			# "$(ZIP5DIR)" != ""
    $(COMMAND_ECHO)-$(ZIPDEP) $(ZIP5FLAGS) $(subst,/misc/,/bin/ $(@:s/.dpzz/$(ZIP5EXT)/)) $(foreach,j,$(ZIP5LIST) "{$(subst,LANGDIR,{$(subst,$(BIN)/$(ZIP5TARGET)_, $(subst,/misc/,/bin/ $(@:db)))} $j )}") >> $(MISC)/$(@:f)
.ENDIF			# "$(ZIP5DIR)" != ""
    @echo $(@:n:s#\#/#) : makefile.mk >> $(MISC)/$(@:f)


$(ZIP5TARGETN) : delzip $(ZIP5DEPS)
    @echo "[ building ZIP ]" $(@:f)
    @@$(!eq,$?,$(?:s/delzip/zzz/) -$(RM) echo) $(uniq $@ $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@))
.IF "$(ZIP5DIR)" != ""
    @@-$(GNUCOPY) -p $@ $(ZIP5TMP).$(ZIP5TARGET){$(subst,$(ZIP5HELPVAR),_ $(@:db))}$(ZIP5EXT)
    $(COMMAND_ECHO)-$(CDD) $(subst,LANGDIR,{$(subst,$(ZIP5HELPVAR)_, $(@:db))} $(ZIP5DIR)) $(command_seperator) zip $(ZIP_VERBOSITY) $(ZIP5FLAGS) $(ZIP5TMP).$(ZIP5TARGET){$(subst,$(ZIP5HELPVAR),_ $(@:db))}$(ZIP5EXT) $(subst,LANGDIR_away/, $(ZIP5LIST:s/LANGDIR/LANGDIR_away/)) -x delzip  $(CHECKZIPRESULT)
    $(COMMAND_ECHO)$(COPY) $(ZIP5TMP).$(ZIP5TARGET){$(subst,$(ZIP5HELPVAR),_ $(@:db))}$(ZIP5EXT)  $@ 
    $(COMMAND_ECHO)$(RM) $(ZIP5TMP).$(ZIP5TARGET){$(subst,$(ZIP5HELPVAR),_ $(@:db))}$(ZIP5EXT)
.ELSE			# "$(ZIP5DIR)" != ""
    $(COMMAND_ECHO)zip $(ZIP_VERBOSITY) $(ZIP5FLAGS) $@ $(foreach,j,$(ZIP5LIST) $(subst,LANGDIR,{$(subst,$(BIN)/$(ZIP5TARGET)_, $(@:db))} $j )) -x delzip $(CHECKZIPRESULT)
.IF "$(ZIP5STRIPLANGUAGETAGS)" != ""
    $(COMMAND_ECHO)$(IFEXIST) $@ $(THEN) \
        $(SOLARENV)/bin/striplanguagetags.sh $@ \
        $(FI)
.ENDIF
.ENDIF			# "$(ZIP5DIR)" != ""
.ENDIF

# unroll begin

.IF "$(OS)"=="WNT"
command_seperator=&&
#command_seperator=^
.ELSE
command_seperator=;
 
.ENDIF

.IF "$(ZIP6TARGET)"!=""

.IF "$(ZIP6STRIPLANGUAGETAGS)" != ""
CALLXSLTPROC:=$(XSLTPROC)
.EXPORT: CALLXSLTPROC
.ENDIF

ZIP6EXT*=.zip
.IF "$(ZIP6LIST:s/LANGDIR//)" == "$(ZIP6LIST)"
ZIP6TARGETN=$(BIN)/$(ZIP6TARGET)$(ZIP6EXT)
.ELSE
ZIP6TARGETN=$(foreach,i,$(zip6alllangiso) $(BIN)/$(ZIP6TARGET)_$i$(ZIP6EXT) )
.ENDIF
ZIP6DEPFILE=$(subst,$(COMMON_OUTDIR),$(OUTPATH) $(subst,/bin/,/misc/ $(ZIP6TARGETN:s/$(ZIP6EXT)/.dpzz/)))
ZIPDEPFILES+=$(ZIP6DEPFILE)

ZIP6DIR*=$(ZIPDIR)
ZIP6FLAGS*=$(ZIPFLAGS)
.IF "$(zip6generatedlangs)"!=""
zip6langdirs*=$(alllangiso)
.ELSE           # "$(zip6generatedlangs)"!=""
zip6langdirs:=$(shell @-test -d {$(subst,/$(LANGDIR), $(null,$(ZIP6DIR) . $(ZIP6DIR)))}/ && find {$(subst,/$(LANGDIR), $(null,$(ZIP6DIR) . $(ZIP6DIR)))}/ -type d ! -name CVS ! -name ".svn" ! -name "." | sed "s/\.\/\///" | sed "s/\.\///" )
.ENDIF          # "$(zip6generatedlangs)"!=""
.IF "$(ZIP6FORCEALLLANG)"!=""
zip6alllangiso:=$(foreach,i,$(completelangiso) $(foreach,j,$(zip6langdirs) $(eq,$i,$j  $i $(NULL))))
.ELSE          # "$(ZIP6ALLLANG)" != ""
zip6alllangiso*:=$(foreach,i,$(alllangiso) $(foreach,j,$(zip6langdirs) $(eq,$i,$j  $i $(NULL))))
.ENDIF          # "$(ZIP6ALLLANG)" != ""
.ENDIF			# "$(ZIP6TARGET)"!=""

.IF "$(ZIP6TARGETN)"!=""

ZIP6TMP:=$(mktmp iii)

$(MISC)/$(TARGET).$(PWD:f).$(ZIP6TARGET).dpzz : $(ZIP6TARGETN)

ZIP6HELPVAR=$(BIN)/$(ZIP6TARGET)

$(ZIP6DEPFILE) :
.IF "$(ZIP6DIR)" != ""
    $(COMMAND_ECHO)-$(MKDIRHIER) $(ZIP6DIR)
    $(COMMAND_ECHO)$(CDD) $(subst,LANGDIR,. $(subst,/LANGDIR, $(ZIP6DIR))) $(command_seperator) $(ZIPDEP) $(ZIP6FLAGS) -prefix $(subst,LANGDIR,. $(subst,/LANGDIR, $(ZIP6DIR)))/ $(subst,/misc/,/bin/ $(@:s/.dpzz/$(ZIP6EXT)/)) $(foreach,j,$(ZIP6LIST) "{$(subst,LANGDIR,{$(subst,$(BIN)/$(ZIP6TARGET)_, $(subst,/misc/,/bin/ $(@:db)))} $j )}") >> $(PWD)/$(PRJ)/$(ROUT)/misc/$(@:f)
.ELSE			# "$(ZIP6DIR)" != ""
    $(COMMAND_ECHO)-$(ZIPDEP) $(ZIP6FLAGS) $(subst,/misc/,/bin/ $(@:s/.dpzz/$(ZIP6EXT)/)) $(foreach,j,$(ZIP6LIST) "{$(subst,LANGDIR,{$(subst,$(BIN)/$(ZIP6TARGET)_, $(subst,/misc/,/bin/ $(@:db)))} $j )}") >> $(MISC)/$(@:f)
.ENDIF			# "$(ZIP6DIR)" != ""
    @echo $(@:n:s#\#/#) : makefile.mk >> $(MISC)/$(@:f)


$(ZIP6TARGETN) : delzip $(ZIP6DEPS)
    @echo "[ building ZIP ]" $(@:f)
    @@$(!eq,$?,$(?:s/delzip/zzz/) -$(RM) echo) $(uniq $@ $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@))
.IF "$(ZIP6DIR)" != ""
    @@-$(GNUCOPY) -p $@ $(ZIP6TMP).$(ZIP6TARGET){$(subst,$(ZIP6HELPVAR),_ $(@:db))}$(ZIP6EXT)
    $(COMMAND_ECHO)-$(CDD) $(subst,LANGDIR,{$(subst,$(ZIP6HELPVAR)_, $(@:db))} $(ZIP6DIR)) $(command_seperator) zip $(ZIP_VERBOSITY) $(ZIP6FLAGS) $(ZIP6TMP).$(ZIP6TARGET){$(subst,$(ZIP6HELPVAR),_ $(@:db))}$(ZIP6EXT) $(subst,LANGDIR_away/, $(ZIP6LIST:s/LANGDIR/LANGDIR_away/)) -x delzip  $(CHECKZIPRESULT)
    $(COMMAND_ECHO)$(COPY) $(ZIP6TMP).$(ZIP6TARGET){$(subst,$(ZIP6HELPVAR),_ $(@:db))}$(ZIP6EXT)  $@ 
    $(COMMAND_ECHO)$(RM) $(ZIP6TMP).$(ZIP6TARGET){$(subst,$(ZIP6HELPVAR),_ $(@:db))}$(ZIP6EXT)
.ELSE			# "$(ZIP6DIR)" != ""
    $(COMMAND_ECHO)zip $(ZIP_VERBOSITY) $(ZIP6FLAGS) $@ $(foreach,j,$(ZIP6LIST) $(subst,LANGDIR,{$(subst,$(BIN)/$(ZIP6TARGET)_, $(@:db))} $j )) -x delzip $(CHECKZIPRESULT)
.IF "$(ZIP6STRIPLANGUAGETAGS)" != ""
    $(COMMAND_ECHO)$(IFEXIST) $@ $(THEN) \
        $(SOLARENV)/bin/striplanguagetags.sh $@ \
        $(FI)
.ENDIF
.ENDIF			# "$(ZIP6DIR)" != ""
.ENDIF

# unroll begin

.IF "$(OS)"=="WNT"
command_seperator=&&
#command_seperator=^
.ELSE
command_seperator=;
 
.ENDIF

.IF "$(ZIP7TARGET)"!=""

.IF "$(ZIP7STRIPLANGUAGETAGS)" != ""
CALLXSLTPROC:=$(XSLTPROC)
.EXPORT: CALLXSLTPROC
.ENDIF

ZIP7EXT*=.zip
.IF "$(ZIP7LIST:s/LANGDIR//)" == "$(ZIP7LIST)"
ZIP7TARGETN=$(BIN)/$(ZIP7TARGET)$(ZIP7EXT)
.ELSE
ZIP7TARGETN=$(foreach,i,$(zip7alllangiso) $(BIN)/$(ZIP7TARGET)_$i$(ZIP7EXT) )
.ENDIF
ZIP7DEPFILE=$(subst,$(COMMON_OUTDIR),$(OUTPATH) $(subst,/bin/,/misc/ $(ZIP7TARGETN:s/$(ZIP7EXT)/.dpzz/)))
ZIPDEPFILES+=$(ZIP7DEPFILE)

ZIP7DIR*=$(ZIPDIR)
ZIP7FLAGS*=$(ZIPFLAGS)
.IF "$(zip7generatedlangs)"!=""
zip7langdirs*=$(alllangiso)
.ELSE           # "$(zip7generatedlangs)"!=""
zip7langdirs:=$(shell @-test -d {$(subst,/$(LANGDIR), $(null,$(ZIP7DIR) . $(ZIP7DIR)))}/ && find {$(subst,/$(LANGDIR), $(null,$(ZIP7DIR) . $(ZIP7DIR)))}/ -type d ! -name CVS ! -name ".svn" ! -name "." | sed "s/\.\/\///" | sed "s/\.\///" )
.ENDIF          # "$(zip7generatedlangs)"!=""
.IF "$(ZIP7FORCEALLLANG)"!=""
zip7alllangiso:=$(foreach,i,$(completelangiso) $(foreach,j,$(zip7langdirs) $(eq,$i,$j  $i $(NULL))))
.ELSE          # "$(ZIP7ALLLANG)" != ""
zip7alllangiso*:=$(foreach,i,$(alllangiso) $(foreach,j,$(zip7langdirs) $(eq,$i,$j  $i $(NULL))))
.ENDIF          # "$(ZIP7ALLLANG)" != ""
.ENDIF			# "$(ZIP7TARGET)"!=""

.IF "$(ZIP7TARGETN)"!=""

ZIP7TMP:=$(mktmp iii)

$(MISC)/$(TARGET).$(PWD:f).$(ZIP7TARGET).dpzz : $(ZIP7TARGETN)

ZIP7HELPVAR=$(BIN)/$(ZIP7TARGET)

$(ZIP7DEPFILE) :
.IF "$(ZIP7DIR)" != ""
    $(COMMAND_ECHO)-$(MKDIRHIER) $(ZIP7DIR)
    $(COMMAND_ECHO)$(CDD) $(subst,LANGDIR,. $(subst,/LANGDIR, $(ZIP7DIR))) $(command_seperator) $(ZIPDEP) $(ZIP7FLAGS) -prefix $(subst,LANGDIR,. $(subst,/LANGDIR, $(ZIP7DIR)))/ $(subst,/misc/,/bin/ $(@:s/.dpzz/$(ZIP7EXT)/)) $(foreach,j,$(ZIP7LIST) "{$(subst,LANGDIR,{$(subst,$(BIN)/$(ZIP7TARGET)_, $(subst,/misc/,/bin/ $(@:db)))} $j )}") >> $(PWD)/$(PRJ)/$(ROUT)/misc/$(@:f)
.ELSE			# "$(ZIP7DIR)" != ""
    $(COMMAND_ECHO)-$(ZIPDEP) $(ZIP7FLAGS) $(subst,/misc/,/bin/ $(@:s/.dpzz/$(ZIP7EXT)/)) $(foreach,j,$(ZIP7LIST) "{$(subst,LANGDIR,{$(subst,$(BIN)/$(ZIP7TARGET)_, $(subst,/misc/,/bin/ $(@:db)))} $j )}") >> $(MISC)/$(@:f)
.ENDIF			# "$(ZIP7DIR)" != ""
    @echo $(@:n:s#\#/#) : makefile.mk >> $(MISC)/$(@:f)


$(ZIP7TARGETN) : delzip $(ZIP7DEPS)
    @echo "[ building ZIP ]" $(@:f)
    @@$(!eq,$?,$(?:s/delzip/zzz/) -$(RM) echo) $(uniq $@ $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@))
.IF "$(ZIP7DIR)" != ""
    @@-$(GNUCOPY) -p $@ $(ZIP7TMP).$(ZIP7TARGET){$(subst,$(ZIP7HELPVAR),_ $(@:db))}$(ZIP7EXT)
    $(COMMAND_ECHO)-$(CDD) $(subst,LANGDIR,{$(subst,$(ZIP7HELPVAR)_, $(@:db))} $(ZIP7DIR)) $(command_seperator) zip $(ZIP_VERBOSITY) $(ZIP7FLAGS) $(ZIP7TMP).$(ZIP7TARGET){$(subst,$(ZIP7HELPVAR),_ $(@:db))}$(ZIP7EXT) $(subst,LANGDIR_away/, $(ZIP7LIST:s/LANGDIR/LANGDIR_away/)) -x delzip  $(CHECKZIPRESULT)
    $(COMMAND_ECHO)$(COPY) $(ZIP7TMP).$(ZIP7TARGET){$(subst,$(ZIP7HELPVAR),_ $(@:db))}$(ZIP7EXT)  $@ 
    $(COMMAND_ECHO)$(RM) $(ZIP7TMP).$(ZIP7TARGET){$(subst,$(ZIP7HELPVAR),_ $(@:db))}$(ZIP7EXT)
.ELSE			# "$(ZIP7DIR)" != ""
    $(COMMAND_ECHO)zip $(ZIP_VERBOSITY) $(ZIP7FLAGS) $@ $(foreach,j,$(ZIP7LIST) $(subst,LANGDIR,{$(subst,$(BIN)/$(ZIP7TARGET)_, $(@:db))} $j )) -x delzip $(CHECKZIPRESULT)
.IF "$(ZIP7STRIPLANGUAGETAGS)" != ""
    $(COMMAND_ECHO)$(IFEXIST) $@ $(THEN) \
        $(SOLARENV)/bin/striplanguagetags.sh $@ \
        $(FI)
.ENDIF
.ENDIF			# "$(ZIP7DIR)" != ""
.ENDIF

# unroll begin

.IF "$(OS)"=="WNT"
command_seperator=&&
#command_seperator=^
.ELSE
command_seperator=;
 
.ENDIF

.IF "$(ZIP8TARGET)"!=""

.IF "$(ZIP8STRIPLANGUAGETAGS)" != ""
CALLXSLTPROC:=$(XSLTPROC)
.EXPORT: CALLXSLTPROC
.ENDIF

ZIP8EXT*=.zip
.IF "$(ZIP8LIST:s/LANGDIR//)" == "$(ZIP8LIST)"
ZIP8TARGETN=$(BIN)/$(ZIP8TARGET)$(ZIP8EXT)
.ELSE
ZIP8TARGETN=$(foreach,i,$(zip8alllangiso) $(BIN)/$(ZIP8TARGET)_$i$(ZIP8EXT) )
.ENDIF
ZIP8DEPFILE=$(subst,$(COMMON_OUTDIR),$(OUTPATH) $(subst,/bin/,/misc/ $(ZIP8TARGETN:s/$(ZIP8EXT)/.dpzz/)))
ZIPDEPFILES+=$(ZIP8DEPFILE)

ZIP8DIR*=$(ZIPDIR)
ZIP8FLAGS*=$(ZIPFLAGS)
.IF "$(zip8generatedlangs)"!=""
zip8langdirs*=$(alllangiso)
.ELSE           # "$(zip8generatedlangs)"!=""
zip8langdirs:=$(shell @-test -d {$(subst,/$(LANGDIR), $(null,$(ZIP8DIR) . $(ZIP8DIR)))}/ && find {$(subst,/$(LANGDIR), $(null,$(ZIP8DIR) . $(ZIP8DIR)))}/ -type d ! -name CVS ! -name ".svn" ! -name "." | sed "s/\.\/\///" | sed "s/\.\///" )
.ENDIF          # "$(zip8generatedlangs)"!=""
.IF "$(ZIP8FORCEALLLANG)"!=""
zip8alllangiso:=$(foreach,i,$(completelangiso) $(foreach,j,$(zip8langdirs) $(eq,$i,$j  $i $(NULL))))
.ELSE          # "$(ZIP8ALLLANG)" != ""
zip8alllangiso*:=$(foreach,i,$(alllangiso) $(foreach,j,$(zip8langdirs) $(eq,$i,$j  $i $(NULL))))
.ENDIF          # "$(ZIP8ALLLANG)" != ""
.ENDIF			# "$(ZIP8TARGET)"!=""

.IF "$(ZIP8TARGETN)"!=""

ZIP8TMP:=$(mktmp iii)

$(MISC)/$(TARGET).$(PWD:f).$(ZIP8TARGET).dpzz : $(ZIP8TARGETN)

ZIP8HELPVAR=$(BIN)/$(ZIP8TARGET)

$(ZIP8DEPFILE) :
.IF "$(ZIP8DIR)" != ""
    $(COMMAND_ECHO)-$(MKDIRHIER) $(ZIP8DIR)
    $(COMMAND_ECHO)$(CDD) $(subst,LANGDIR,. $(subst,/LANGDIR, $(ZIP8DIR))) $(command_seperator) $(ZIPDEP) $(ZIP8FLAGS) -prefix $(subst,LANGDIR,. $(subst,/LANGDIR, $(ZIP8DIR)))/ $(subst,/misc/,/bin/ $(@:s/.dpzz/$(ZIP8EXT)/)) $(foreach,j,$(ZIP8LIST) "{$(subst,LANGDIR,{$(subst,$(BIN)/$(ZIP8TARGET)_, $(subst,/misc/,/bin/ $(@:db)))} $j )}") >> $(PWD)/$(PRJ)/$(ROUT)/misc/$(@:f)
.ELSE			# "$(ZIP8DIR)" != ""
    $(COMMAND_ECHO)-$(ZIPDEP) $(ZIP8FLAGS) $(subst,/misc/,/bin/ $(@:s/.dpzz/$(ZIP8EXT)/)) $(foreach,j,$(ZIP8LIST) "{$(subst,LANGDIR,{$(subst,$(BIN)/$(ZIP8TARGET)_, $(subst,/misc/,/bin/ $(@:db)))} $j )}") >> $(MISC)/$(@:f)
.ENDIF			# "$(ZIP8DIR)" != ""
    @echo $(@:n:s#\#/#) : makefile.mk >> $(MISC)/$(@:f)


$(ZIP8TARGETN) : delzip $(ZIP8DEPS)
    @echo "[ building ZIP ]" $(@:f)
    @@$(!eq,$?,$(?:s/delzip/zzz/) -$(RM) echo) $(uniq $@ $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@))
.IF "$(ZIP8DIR)" != ""
    @@-$(GNUCOPY) -p $@ $(ZIP8TMP).$(ZIP8TARGET){$(subst,$(ZIP8HELPVAR),_ $(@:db))}$(ZIP8EXT)
    $(COMMAND_ECHO)-$(CDD) $(subst,LANGDIR,{$(subst,$(ZIP8HELPVAR)_, $(@:db))} $(ZIP8DIR)) $(command_seperator) zip $(ZIP_VERBOSITY) $(ZIP8FLAGS) $(ZIP8TMP).$(ZIP8TARGET){$(subst,$(ZIP8HELPVAR),_ $(@:db))}$(ZIP8EXT) $(subst,LANGDIR_away/, $(ZIP8LIST:s/LANGDIR/LANGDIR_away/)) -x delzip  $(CHECKZIPRESULT)
    $(COMMAND_ECHO)$(COPY) $(ZIP8TMP).$(ZIP8TARGET){$(subst,$(ZIP8HELPVAR),_ $(@:db))}$(ZIP8EXT)  $@ 
    $(COMMAND_ECHO)$(RM) $(ZIP8TMP).$(ZIP8TARGET){$(subst,$(ZIP8HELPVAR),_ $(@:db))}$(ZIP8EXT)
.ELSE			# "$(ZIP8DIR)" != ""
    $(COMMAND_ECHO)zip $(ZIP_VERBOSITY) $(ZIP8FLAGS) $@ $(foreach,j,$(ZIP8LIST) $(subst,LANGDIR,{$(subst,$(BIN)/$(ZIP8TARGET)_, $(@:db))} $j )) -x delzip $(CHECKZIPRESULT)
.IF "$(ZIP8STRIPLANGUAGETAGS)" != ""
    $(COMMAND_ECHO)$(IFEXIST) $@ $(THEN) \
        $(SOLARENV)/bin/striplanguagetags.sh $@ \
        $(FI)
.ENDIF
.ENDIF			# "$(ZIP8DIR)" != ""
.ENDIF

# unroll begin

.IF "$(OS)"=="WNT"
command_seperator=&&
#command_seperator=^
.ELSE
command_seperator=;
 
.ENDIF

.IF "$(ZIP9TARGET)"!=""

.IF "$(ZIP9STRIPLANGUAGETAGS)" != ""
CALLXSLTPROC:=$(XSLTPROC)
.EXPORT: CALLXSLTPROC
.ENDIF

ZIP9EXT*=.zip
.IF "$(ZIP9LIST:s/LANGDIR//)" == "$(ZIP9LIST)"
ZIP9TARGETN=$(BIN)/$(ZIP9TARGET)$(ZIP9EXT)
.ELSE
ZIP9TARGETN=$(foreach,i,$(zip9alllangiso) $(BIN)/$(ZIP9TARGET)_$i$(ZIP9EXT) )
.ENDIF
ZIP9DEPFILE=$(subst,$(COMMON_OUTDIR),$(OUTPATH) $(subst,/bin/,/misc/ $(ZIP9TARGETN:s/$(ZIP9EXT)/.dpzz/)))
ZIPDEPFILES+=$(ZIP9DEPFILE)

ZIP9DIR*=$(ZIPDIR)
ZIP9FLAGS*=$(ZIPFLAGS)
.IF "$(zip9generatedlangs)"!=""
zip9langdirs*=$(alllangiso)
.ELSE           # "$(zip9generatedlangs)"!=""
zip9langdirs:=$(shell @-test -d {$(subst,/$(LANGDIR), $(null,$(ZIP9DIR) . $(ZIP9DIR)))}/ && find {$(subst,/$(LANGDIR), $(null,$(ZIP9DIR) . $(ZIP9DIR)))}/ -type d ! -name CVS ! -name ".svn" ! -name "." | sed "s/\.\/\///" | sed "s/\.\///" )
.ENDIF          # "$(zip9generatedlangs)"!=""
.IF "$(ZIP9FORCEALLLANG)"!=""
zip9alllangiso:=$(foreach,i,$(completelangiso) $(foreach,j,$(zip9langdirs) $(eq,$i,$j  $i $(NULL))))
.ELSE          # "$(ZIP9ALLLANG)" != ""
zip9alllangiso*:=$(foreach,i,$(alllangiso) $(foreach,j,$(zip9langdirs) $(eq,$i,$j  $i $(NULL))))
.ENDIF          # "$(ZIP9ALLLANG)" != ""
.ENDIF			# "$(ZIP9TARGET)"!=""

.IF "$(ZIP9TARGETN)"!=""

ZIP9TMP:=$(mktmp iii)

$(MISC)/$(TARGET).$(PWD:f).$(ZIP9TARGET).dpzz : $(ZIP9TARGETN)

ZIP9HELPVAR=$(BIN)/$(ZIP9TARGET)

$(ZIP9DEPFILE) :
.IF "$(ZIP9DIR)" != ""
    $(COMMAND_ECHO)-$(MKDIRHIER) $(ZIP9DIR)
    $(COMMAND_ECHO)$(CDD) $(subst,LANGDIR,. $(subst,/LANGDIR, $(ZIP9DIR))) $(command_seperator) $(ZIPDEP) $(ZIP9FLAGS) -prefix $(subst,LANGDIR,. $(subst,/LANGDIR, $(ZIP9DIR)))/ $(subst,/misc/,/bin/ $(@:s/.dpzz/$(ZIP9EXT)/)) $(foreach,j,$(ZIP9LIST) "{$(subst,LANGDIR,{$(subst,$(BIN)/$(ZIP9TARGET)_, $(subst,/misc/,/bin/ $(@:db)))} $j )}") >> $(PWD)/$(PRJ)/$(ROUT)/misc/$(@:f)
.ELSE			# "$(ZIP9DIR)" != ""
    $(COMMAND_ECHO)-$(ZIPDEP) $(ZIP9FLAGS) $(subst,/misc/,/bin/ $(@:s/.dpzz/$(ZIP9EXT)/)) $(foreach,j,$(ZIP9LIST) "{$(subst,LANGDIR,{$(subst,$(BIN)/$(ZIP9TARGET)_, $(subst,/misc/,/bin/ $(@:db)))} $j )}") >> $(MISC)/$(@:f)
.ENDIF			# "$(ZIP9DIR)" != ""
    @echo $(@:n:s#\#/#) : makefile.mk >> $(MISC)/$(@:f)


$(ZIP9TARGETN) : delzip $(ZIP9DEPS)
    @echo "[ building ZIP ]" $(@:f)
    @@$(!eq,$?,$(?:s/delzip/zzz/) -$(RM) echo) $(uniq $@ $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@))
.IF "$(ZIP9DIR)" != ""
    @@-$(GNUCOPY) -p $@ $(ZIP9TMP).$(ZIP9TARGET){$(subst,$(ZIP9HELPVAR),_ $(@:db))}$(ZIP9EXT)
    $(COMMAND_ECHO)-$(CDD) $(subst,LANGDIR,{$(subst,$(ZIP9HELPVAR)_, $(@:db))} $(ZIP9DIR)) $(command_seperator) zip $(ZIP_VERBOSITY) $(ZIP9FLAGS) $(ZIP9TMP).$(ZIP9TARGET){$(subst,$(ZIP9HELPVAR),_ $(@:db))}$(ZIP9EXT) $(subst,LANGDIR_away/, $(ZIP9LIST:s/LANGDIR/LANGDIR_away/)) -x delzip  $(CHECKZIPRESULT)
    $(COMMAND_ECHO)$(COPY) $(ZIP9TMP).$(ZIP9TARGET){$(subst,$(ZIP9HELPVAR),_ $(@:db))}$(ZIP9EXT)  $@ 
    $(COMMAND_ECHO)$(RM) $(ZIP9TMP).$(ZIP9TARGET){$(subst,$(ZIP9HELPVAR),_ $(@:db))}$(ZIP9EXT)
.ELSE			# "$(ZIP9DIR)" != ""
    $(COMMAND_ECHO)zip $(ZIP_VERBOSITY) $(ZIP9FLAGS) $@ $(foreach,j,$(ZIP9LIST) $(subst,LANGDIR,{$(subst,$(BIN)/$(ZIP9TARGET)_, $(@:db))} $j )) -x delzip $(CHECKZIPRESULT)
.IF "$(ZIP9STRIPLANGUAGETAGS)" != ""
    $(COMMAND_ECHO)$(IFEXIST) $@ $(THEN) \
        $(SOLARENV)/bin/striplanguagetags.sh $@ \
        $(FI)
.ENDIF
.ENDIF			# "$(ZIP9DIR)" != ""
.ENDIF

# unroll begin

.IF "$(OS)"=="WNT"
command_seperator=&&
#command_seperator=^
.ELSE
command_seperator=;
 
.ENDIF

.IF "$(ZIP10TARGET)"!=""

.IF "$(ZIP10STRIPLANGUAGETAGS)" != ""
CALLXSLTPROC:=$(XSLTPROC)
.EXPORT: CALLXSLTPROC
.ENDIF

ZIP10EXT*=.zip
.IF "$(ZIP10LIST:s/LANGDIR//)" == "$(ZIP10LIST)"
ZIP10TARGETN=$(BIN)/$(ZIP10TARGET)$(ZIP10EXT)
.ELSE
ZIP10TARGETN=$(foreach,i,$(zip10alllangiso) $(BIN)/$(ZIP10TARGET)_$i$(ZIP10EXT) )
.ENDIF
ZIP10DEPFILE=$(subst,$(COMMON_OUTDIR),$(OUTPATH) $(subst,/bin/,/misc/ $(ZIP10TARGETN:s/$(ZIP10EXT)/.dpzz/)))
ZIPDEPFILES+=$(ZIP10DEPFILE)

ZIP10DIR*=$(ZIPDIR)
ZIP10FLAGS*=$(ZIPFLAGS)
.IF "$(zip10generatedlangs)"!=""
zip10langdirs*=$(alllangiso)
.ELSE           # "$(zip10generatedlangs)"!=""
zip10langdirs:=$(shell @-test -d {$(subst,/$(LANGDIR), $(null,$(ZIP10DIR) . $(ZIP10DIR)))}/ && find {$(subst,/$(LANGDIR), $(null,$(ZIP10DIR) . $(ZIP10DIR)))}/ -type d ! -name CVS ! -name ".svn" ! -name "." | sed "s/\.\/\///" | sed "s/\.\///" )
.ENDIF          # "$(zip10generatedlangs)"!=""
.IF "$(ZIP10FORCEALLLANG)"!=""
zip10alllangiso:=$(foreach,i,$(completelangiso) $(foreach,j,$(zip10langdirs) $(eq,$i,$j  $i $(NULL))))
.ELSE          # "$(ZIP10ALLLANG)" != ""
zip10alllangiso*:=$(foreach,i,$(alllangiso) $(foreach,j,$(zip10langdirs) $(eq,$i,$j  $i $(NULL))))
.ENDIF          # "$(ZIP10ALLLANG)" != ""
.ENDIF			# "$(ZIP10TARGET)"!=""

.IF "$(ZIP10TARGETN)"!=""

ZIP10TMP:=$(mktmp iii)

$(MISC)/$(TARGET).$(PWD:f).$(ZIP10TARGET).dpzz : $(ZIP10TARGETN)

ZIP10HELPVAR=$(BIN)/$(ZIP10TARGET)

$(ZIP10DEPFILE) :
.IF "$(ZIP10DIR)" != ""
    $(COMMAND_ECHO)-$(MKDIRHIER) $(ZIP10DIR)
    $(COMMAND_ECHO)$(CDD) $(subst,LANGDIR,. $(subst,/LANGDIR, $(ZIP10DIR))) $(command_seperator) $(ZIPDEP) $(ZIP10FLAGS) -prefix $(subst,LANGDIR,. $(subst,/LANGDIR, $(ZIP10DIR)))/ $(subst,/misc/,/bin/ $(@:s/.dpzz/$(ZIP10EXT)/)) $(foreach,j,$(ZIP10LIST) "{$(subst,LANGDIR,{$(subst,$(BIN)/$(ZIP10TARGET)_, $(subst,/misc/,/bin/ $(@:db)))} $j )}") >> $(PWD)/$(PRJ)/$(ROUT)/misc/$(@:f)
.ELSE			# "$(ZIP10DIR)" != ""
    $(COMMAND_ECHO)-$(ZIPDEP) $(ZIP10FLAGS) $(subst,/misc/,/bin/ $(@:s/.dpzz/$(ZIP10EXT)/)) $(foreach,j,$(ZIP10LIST) "{$(subst,LANGDIR,{$(subst,$(BIN)/$(ZIP10TARGET)_, $(subst,/misc/,/bin/ $(@:db)))} $j )}") >> $(MISC)/$(@:f)
.ENDIF			# "$(ZIP10DIR)" != ""
    @echo $(@:n:s#\#/#) : makefile.mk >> $(MISC)/$(@:f)


$(ZIP10TARGETN) : delzip $(ZIP10DEPS)
    @echo "[ building ZIP ]" $(@:f)
    @@$(!eq,$?,$(?:s/delzip/zzz/) -$(RM) echo) $(uniq $@ $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@))
.IF "$(ZIP10DIR)" != ""
    @@-$(GNUCOPY) -p $@ $(ZIP10TMP).$(ZIP10TARGET){$(subst,$(ZIP10HELPVAR),_ $(@:db))}$(ZIP10EXT)
    $(COMMAND_ECHO)-$(CDD) $(subst,LANGDIR,{$(subst,$(ZIP10HELPVAR)_, $(@:db))} $(ZIP10DIR)) $(command_seperator) zip $(ZIP_VERBOSITY) $(ZIP10FLAGS) $(ZIP10TMP).$(ZIP10TARGET){$(subst,$(ZIP10HELPVAR),_ $(@:db))}$(ZIP10EXT) $(subst,LANGDIR_away/, $(ZIP10LIST:s/LANGDIR/LANGDIR_away/)) -x delzip  $(CHECKZIPRESULT)
    $(COMMAND_ECHO)$(COPY) $(ZIP10TMP).$(ZIP10TARGET){$(subst,$(ZIP10HELPVAR),_ $(@:db))}$(ZIP10EXT)  $@ 
    $(COMMAND_ECHO)$(RM) $(ZIP10TMP).$(ZIP10TARGET){$(subst,$(ZIP10HELPVAR),_ $(@:db))}$(ZIP10EXT)
.ELSE			# "$(ZIP10DIR)" != ""
    $(COMMAND_ECHO)zip $(ZIP_VERBOSITY) $(ZIP10FLAGS) $@ $(foreach,j,$(ZIP10LIST) $(subst,LANGDIR,{$(subst,$(BIN)/$(ZIP10TARGET)_, $(@:db))} $j )) -x delzip $(CHECKZIPRESULT)
.IF "$(ZIP10STRIPLANGUAGETAGS)" != ""
    $(COMMAND_ECHO)$(IFEXIST) $@ $(THEN) \
        $(SOLARENV)/bin/striplanguagetags.sh $@ \
        $(FI)
.ENDIF
.ENDIF			# "$(ZIP10DIR)" != ""
.ENDIF

