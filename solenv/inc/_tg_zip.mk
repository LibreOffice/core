# unroll begin

.IF "$(GUI)"=="WNT"
command_seperator=&&
#command_seperator=^
avoid_cvs_dir=-x "*CVS*"
.ELSE
command_seperator=;
avoid_cvs_dir=-x "*CVS*"
.ENDIF

.IF "$(ZIP1TARGET)"!=""
ZIP1DIR*=$(ZIPDIR)
ZIP1FLAGS*=$(ZIPFLAGS)
.IF "$(GUI)"=="UNX" || "$(USE_SHELL)"!="4nt"
zip1langdirs:=$(shell +find {$(subst,$/$(LANGDIR), $(null,$(ZIP1DIR) . $(ZIP1DIR)))}/ -type d ! -name CVS ! -name "." | sed "s/\.\/\///" | sed "s/\.\///" )
.ELSE			# "$(GUI)"=="UNX"
zip1langdirs:=$(subst,CVS, $(shell +-dir {$(subst,$/$(LANGDIR), $(ZIP1DIR))} /ba:d ))
.ENDIF			# "$(GUI)"=="UNX"
zip1alllangext:=$(foreach,i,$(alllangext) $(foreach,j,$(zip1langdirs) $(eq,$(longlang_$i),$j  $i $(NULL))))
.ENDIF			# "$(ZIP1TARGET)"!=""

.IF "$(ZIP1TARGETN)"!=""

ZIP1TMP:=$(mktmp iii)

$(MISC)$/$(TARGET).$(PWD:f).$(ZIP1TARGET).dpzz : $(ZIP1TARGETN)

.IF "$(common_build_zip)"!=""
ZIP1HELPVAR={$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP1TARGET)}
.ELSE			# "$(common_build_zip)"!=""
ZIP1HELPVAR=$(BIN)$/$(ZIP1TARGET)
.ENDIF			# "$(common_build_zip)"!=""

.IF "$(make_zip_deps)" == ""
$(ZIP1TARGETN) : delzip
.ELSE			# "$(make_zip_deps)" == ""
$(ZIP1TARGETN) :
.ENDIF			# "$(make_zip_deps)" == ""
.IF "$(make_zip_deps)" == ""
    @+echo ------------------------------
    @+echo Making: $@
    @$(!eq,$?,$(?:s/delzip/zzz/) +-$(RM) +echo) $(uniq $@ $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)) >&$(NULLDEV)
    @+echo rebuilding zipfiles $(eq,$?,$(?:s/delzip/zzz/) >&$(NULLDEV) )
    @+echo ------------------------------ $(eq,$?,$(?:s/delzip/zzz/) >&$(NULLDEV) )
.IF "$(common_build_zip)"!=""
.IF "$(ZIP1DIR)" != ""
    @+-$(GNUCOPY) -p $@ $(ZIP1TMP).{$(subst,$(ZIP1HELPVAR),_ $(@:db))}.zip >& $(NULLDEV)
    +-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP1HELPVAR), $(@:db))}) $(ZIP1DIR)) $(command_seperator) zip $(ZIP1FLAGS) $(ZIP1TMP).{$(subst,$(ZIP1HELPVAR),_ $(@:db))}.zip $(ZIP1LIST:s/LANGDIR/./) -x delzip $(avoid_cvs_dir) 
    +$(COPY) $(ZIP1TMP).{$(subst,$(ZIP1HELPVAR),_ $(@:db))}.zip $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)    
    +$(RM) $(ZIP1TMP).{$(subst,$(ZIP1HELPVAR),_ $(@:db))}.zip
.ELSE			# "$(ZIP1DIR)" != ""
    @+-$(GNUCOPY) -p $@ $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) >& $(NULLDEV)
    +-zip $(ZIP1FLAGS) $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $(foreach,j,$(ZIP1LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP1HELPVAR), $(@:db))}) $j )) -x delzip $(avoid_cvs_dir)
.ENDIF			# "$(ZIP1DIR)" != ""
    @+-$(GNUCOPY) -p $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $@.$(INPATH) >& $(NULLDEV)
    @+-$(RM) $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)
    @+-$(RM) $@ >& $(NULLDEV)
.IF "$(GUI)"=="UNX"
    @+if ( -r $@.$(INPATH) ) $(RENAME) $@.$(INPATH) $@
#	@+if ( -r $@ ) $(TOUCH) $@
.ELSE			# "$(GUI)"=="UNX"
    @+if exist $@.$(INPATH) $(RENAME) $@.$(INPATH) $@
#	@+if exist $@ $(TOUCH) $@
.ENDIF			# "$(GUI)"=="UNX"
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIP1DIR)" != ""
    @+-$(GNUCOPY) -p $@ $(ZIP1TMP).{$(subst,$(ZIP1HELPVAR),_ $(@:db))}.zip >& $(NULLDEV)
    +-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP1HELPVAR), $(@:db))}) $(ZIP1DIR)) $(command_seperator) zip $(ZIP1FLAGS) $(ZIP1TMP).{$(subst,$(ZIP1HELPVAR),_ $(@:db))}.zip $(ZIP1LIST:s/LANGDIR/./) -x delzip  $(avoid_cvs_dir)
    +$(COPY) $(ZIP1TMP).{$(subst,$(ZIP1HELPVAR),_ $(@:db))}.zip  $@ 
    +$(RM) $(ZIP1TMP).{$(subst,$(ZIP1HELPVAR),_ $(@:db))}.zip
.ELSE			# "$(ZIP1DIR)" != ""
    +-zip $(ZIP1FLAGS) $@ $(foreach,j,$(ZIP1LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP1TARGET), $(@:db))}) $j )) -x delzip $(avoid_cvs_dir)
.ENDIF			# "$(ZIP1DIR)" != ""
.ENDIF			# "$(common_build_zip)"!=""
.ELSE			# "$(make_zip_deps)" == ""
    +echo # > $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.IF "$(common_build_zip)"!=""
.IF "$(ZIP1DIR)" != ""
    +-cd $(subst,LANGDIR,. $(subst,$/LANGDIR, $(ZIP1DIR))) $(command_seperator) $(ZIPDEP) $(ZIP1FLAGS) -prefix $(subst,LANGDIR,. $(subst,$/LANGDIR, $(ZIP1DIR)))$/ $@ $(foreach,j,$(ZIP1LIST) "{$(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP1TARGET), $(@:db))}) $j )}") $(avoid_cvs_dir) >> $(PWD)$/$(PRJ)$/$(ROUT)$/misc$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ELSE			# "$(ZIP1DIR)" != ""
    +-$(ZIPDEP) $(ZIP1FLAGS) $@ $(foreach,j,$(ZIP1LIST) "{$(subst,LANGDIR,$(longlang_{$(subst,$(ZIP1HELPVAR), $(@:db))}) $j )}") $(avoid_cvs_dir) >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ENDIF			# "$(ZIP1DIR)" != ""
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIP1DIR)" != ""
    +-cd $(subst,LANGDIR,. $(subst,$/LANGDIR, $(ZIP1DIR))) $(command_seperator) $(ZIPDEP) $(ZIP1FLAGS) -prefix $(subst,LANGDIR,. $(subst,$/LANGDIR, $(ZIP1DIR)))$/ $@ $(foreach,j,$(ZIP1LIST) "{$(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP1TARGET), $(@:db))}) $j )}") $(avoid_cvs_dir) >> $(PWD)$/$(PRJ)$/$(ROUT)$/misc$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ELSE			# "$(ZIP1DIR)" != ""
    +-$(ZIPDEP) $(ZIP1FLAGS) $@ $(foreach,j,$(ZIP1LIST) "{$(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP1TARGET), $(@:db))}) $j )}") $(avoid_cvs_dir) >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ENDIF			# "$(ZIP1DIR)" != ""
.ENDIF			# "$(common_build_zip)"!=""
    @+echo $@ : makefile.mk >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ENDIF			# "$(make_zip_deps)" == ""
.ENDIF

# Anweisungen fuer das Linken
# unroll begin

.IF "$(GUI)"=="WNT"
command_seperator=&&
#command_seperator=^
avoid_cvs_dir=-x "*CVS*"
.ELSE
command_seperator=;
avoid_cvs_dir=-x "*CVS*"
.ENDIF

.IF "$(ZIP2TARGET)"!=""
ZIP2DIR*=$(ZIPDIR)
ZIP2FLAGS*=$(ZIPFLAGS)
.IF "$(GUI)"=="UNX" || "$(USE_SHELL)"!="4nt"
zip2langdirs:=$(shell +find {$(subst,$/$(LANGDIR), $(null,$(ZIP2DIR) . $(ZIP2DIR)))}/ -type d ! -name CVS ! -name "." | sed "s/\.\/\///" | sed "s/\.\///" )
.ELSE			# "$(GUI)"=="UNX"
zip2langdirs:=$(subst,CVS, $(shell +-dir {$(subst,$/$(LANGDIR), $(ZIP2DIR))} /ba:d ))
.ENDIF			# "$(GUI)"=="UNX"
zip2alllangext:=$(foreach,i,$(alllangext) $(foreach,j,$(zip2langdirs) $(eq,$(longlang_$i),$j  $i $(NULL))))
.ENDIF			# "$(ZIP2TARGET)"!=""

.IF "$(ZIP2TARGETN)"!=""

ZIP2TMP:=$(mktmp iii)

$(MISC)$/$(TARGET).$(PWD:f).$(ZIP2TARGET).dpzz : $(ZIP2TARGETN)

.IF "$(common_build_zip)"!=""
ZIP2HELPVAR={$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP2TARGET)}
.ELSE			# "$(common_build_zip)"!=""
ZIP2HELPVAR=$(BIN)$/$(ZIP2TARGET)
.ENDIF			# "$(common_build_zip)"!=""

.IF "$(make_zip_deps)" == ""
$(ZIP2TARGETN) : delzip
.ELSE			# "$(make_zip_deps)" == ""
$(ZIP2TARGETN) :
.ENDIF			# "$(make_zip_deps)" == ""
.IF "$(make_zip_deps)" == ""
    @+echo ------------------------------
    @+echo Making: $@
    @$(!eq,$?,$(?:s/delzip/zzz/) +-$(RM) +echo) $(uniq $@ $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)) >&$(NULLDEV)
    @+echo rebuilding zipfiles $(eq,$?,$(?:s/delzip/zzz/) >&$(NULLDEV) )
    @+echo ------------------------------ $(eq,$?,$(?:s/delzip/zzz/) >&$(NULLDEV) )
.IF "$(common_build_zip)"!=""
.IF "$(ZIP2DIR)" != ""
    @+-$(GNUCOPY) -p $@ $(ZIP2TMP).{$(subst,$(ZIP2HELPVAR),_ $(@:db))}.zip >& $(NULLDEV)
    +-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP2HELPVAR), $(@:db))}) $(ZIP2DIR)) $(command_seperator) zip $(ZIP2FLAGS) $(ZIP2TMP).{$(subst,$(ZIP2HELPVAR),_ $(@:db))}.zip $(ZIP2LIST:s/LANGDIR/./) -x delzip $(avoid_cvs_dir) 
    +$(COPY) $(ZIP2TMP).{$(subst,$(ZIP2HELPVAR),_ $(@:db))}.zip $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)    
    +$(RM) $(ZIP2TMP).{$(subst,$(ZIP2HELPVAR),_ $(@:db))}.zip
.ELSE			# "$(ZIP2DIR)" != ""
    @+-$(GNUCOPY) -p $@ $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) >& $(NULLDEV)
    +-zip $(ZIP2FLAGS) $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $(foreach,j,$(ZIP2LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP2HELPVAR), $(@:db))}) $j )) -x delzip $(avoid_cvs_dir)
.ENDIF			# "$(ZIP2DIR)" != ""
    @+-$(GNUCOPY) -p $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $@.$(INPATH) >& $(NULLDEV)
    @+-$(RM) $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)
    @+-$(RM) $@ >& $(NULLDEV)
.IF "$(GUI)"=="UNX"
    @+if ( -r $@.$(INPATH) ) $(RENAME) $@.$(INPATH) $@
#	@+if ( -r $@ ) $(TOUCH) $@
.ELSE			# "$(GUI)"=="UNX"
    @+if exist $@.$(INPATH) $(RENAME) $@.$(INPATH) $@
#	@+if exist $@ $(TOUCH) $@
.ENDIF			# "$(GUI)"=="UNX"
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIP2DIR)" != ""
    @+-$(GNUCOPY) -p $@ $(ZIP2TMP).{$(subst,$(ZIP2HELPVAR),_ $(@:db))}.zip >& $(NULLDEV)
    +-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP2HELPVAR), $(@:db))}) $(ZIP2DIR)) $(command_seperator) zip $(ZIP2FLAGS) $(ZIP2TMP).{$(subst,$(ZIP2HELPVAR),_ $(@:db))}.zip $(ZIP2LIST:s/LANGDIR/./) -x delzip  $(avoid_cvs_dir)
    +$(COPY) $(ZIP2TMP).{$(subst,$(ZIP2HELPVAR),_ $(@:db))}.zip  $@ 
    +$(RM) $(ZIP2TMP).{$(subst,$(ZIP2HELPVAR),_ $(@:db))}.zip
.ELSE			# "$(ZIP2DIR)" != ""
    +-zip $(ZIP2FLAGS) $@ $(foreach,j,$(ZIP2LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP2TARGET), $(@:db))}) $j )) -x delzip $(avoid_cvs_dir)
.ENDIF			# "$(ZIP2DIR)" != ""
.ENDIF			# "$(common_build_zip)"!=""
.ELSE			# "$(make_zip_deps)" == ""
    +echo # > $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.IF "$(common_build_zip)"!=""
.IF "$(ZIP2DIR)" != ""
    +-cd $(subst,LANGDIR,. $(subst,$/LANGDIR, $(ZIP2DIR))) $(command_seperator) $(ZIPDEP) $(ZIP2FLAGS) -prefix $(subst,LANGDIR,. $(subst,$/LANGDIR, $(ZIP2DIR)))$/ $@ $(foreach,j,$(ZIP2LIST) "{$(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP2TARGET), $(@:db))}) $j )}") $(avoid_cvs_dir) >> $(PWD)$/$(PRJ)$/$(ROUT)$/misc$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ELSE			# "$(ZIP2DIR)" != ""
    +-$(ZIPDEP) $(ZIP2FLAGS) $@ $(foreach,j,$(ZIP2LIST) "{$(subst,LANGDIR,$(longlang_{$(subst,$(ZIP2HELPVAR), $(@:db))}) $j )}") $(avoid_cvs_dir) >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ENDIF			# "$(ZIP2DIR)" != ""
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIP2DIR)" != ""
    +-cd $(subst,LANGDIR,. $(subst,$/LANGDIR, $(ZIP2DIR))) $(command_seperator) $(ZIPDEP) $(ZIP2FLAGS) -prefix $(subst,LANGDIR,. $(subst,$/LANGDIR, $(ZIP2DIR)))$/ $@ $(foreach,j,$(ZIP2LIST) "{$(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP2TARGET), $(@:db))}) $j )}") $(avoid_cvs_dir) >> $(PWD)$/$(PRJ)$/$(ROUT)$/misc$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ELSE			# "$(ZIP2DIR)" != ""
    +-$(ZIPDEP) $(ZIP2FLAGS) $@ $(foreach,j,$(ZIP2LIST) "{$(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP2TARGET), $(@:db))}) $j )}") $(avoid_cvs_dir) >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ENDIF			# "$(ZIP2DIR)" != ""
.ENDIF			# "$(common_build_zip)"!=""
    @+echo $@ : makefile.mk >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ENDIF			# "$(make_zip_deps)" == ""
.ENDIF

# Anweisungen fuer das Linken
# unroll begin

.IF "$(GUI)"=="WNT"
command_seperator=&&
#command_seperator=^
avoid_cvs_dir=-x "*CVS*"
.ELSE
command_seperator=;
avoid_cvs_dir=-x "*CVS*"
.ENDIF

.IF "$(ZIP3TARGET)"!=""
ZIP3DIR*=$(ZIPDIR)
ZIP3FLAGS*=$(ZIPFLAGS)
.IF "$(GUI)"=="UNX" || "$(USE_SHELL)"!="4nt"
zip3langdirs:=$(shell +find {$(subst,$/$(LANGDIR), $(null,$(ZIP3DIR) . $(ZIP3DIR)))}/ -type d ! -name CVS ! -name "." | sed "s/\.\/\///" | sed "s/\.\///" )
.ELSE			# "$(GUI)"=="UNX"
zip3langdirs:=$(subst,CVS, $(shell +-dir {$(subst,$/$(LANGDIR), $(ZIP3DIR))} /ba:d ))
.ENDIF			# "$(GUI)"=="UNX"
zip3alllangext:=$(foreach,i,$(alllangext) $(foreach,j,$(zip3langdirs) $(eq,$(longlang_$i),$j  $i $(NULL))))
.ENDIF			# "$(ZIP3TARGET)"!=""

.IF "$(ZIP3TARGETN)"!=""

ZIP3TMP:=$(mktmp iii)

$(MISC)$/$(TARGET).$(PWD:f).$(ZIP3TARGET).dpzz : $(ZIP3TARGETN)

.IF "$(common_build_zip)"!=""
ZIP3HELPVAR={$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP3TARGET)}
.ELSE			# "$(common_build_zip)"!=""
ZIP3HELPVAR=$(BIN)$/$(ZIP3TARGET)
.ENDIF			# "$(common_build_zip)"!=""

.IF "$(make_zip_deps)" == ""
$(ZIP3TARGETN) : delzip
.ELSE			# "$(make_zip_deps)" == ""
$(ZIP3TARGETN) :
.ENDIF			# "$(make_zip_deps)" == ""
.IF "$(make_zip_deps)" == ""
    @+echo ------------------------------
    @+echo Making: $@
    @$(!eq,$?,$(?:s/delzip/zzz/) +-$(RM) +echo) $(uniq $@ $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)) >&$(NULLDEV)
    @+echo rebuilding zipfiles $(eq,$?,$(?:s/delzip/zzz/) >&$(NULLDEV) )
    @+echo ------------------------------ $(eq,$?,$(?:s/delzip/zzz/) >&$(NULLDEV) )
.IF "$(common_build_zip)"!=""
.IF "$(ZIP3DIR)" != ""
    @+-$(GNUCOPY) -p $@ $(ZIP3TMP).{$(subst,$(ZIP3HELPVAR),_ $(@:db))}.zip >& $(NULLDEV)
    +-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP3HELPVAR), $(@:db))}) $(ZIP3DIR)) $(command_seperator) zip $(ZIP3FLAGS) $(ZIP3TMP).{$(subst,$(ZIP3HELPVAR),_ $(@:db))}.zip $(ZIP3LIST:s/LANGDIR/./) -x delzip $(avoid_cvs_dir) 
    +$(COPY) $(ZIP3TMP).{$(subst,$(ZIP3HELPVAR),_ $(@:db))}.zip $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)    
    +$(RM) $(ZIP3TMP).{$(subst,$(ZIP3HELPVAR),_ $(@:db))}.zip
.ELSE			# "$(ZIP3DIR)" != ""
    @+-$(GNUCOPY) -p $@ $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) >& $(NULLDEV)
    +-zip $(ZIP3FLAGS) $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $(foreach,j,$(ZIP3LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP3HELPVAR), $(@:db))}) $j )) -x delzip $(avoid_cvs_dir)
.ENDIF			# "$(ZIP3DIR)" != ""
    @+-$(GNUCOPY) -p $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $@.$(INPATH) >& $(NULLDEV)
    @+-$(RM) $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)
    @+-$(RM) $@ >& $(NULLDEV)
.IF "$(GUI)"=="UNX"
    @+if ( -r $@.$(INPATH) ) $(RENAME) $@.$(INPATH) $@
#	@+if ( -r $@ ) $(TOUCH) $@
.ELSE			# "$(GUI)"=="UNX"
    @+if exist $@.$(INPATH) $(RENAME) $@.$(INPATH) $@
#	@+if exist $@ $(TOUCH) $@
.ENDIF			# "$(GUI)"=="UNX"
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIP3DIR)" != ""
    @+-$(GNUCOPY) -p $@ $(ZIP3TMP).{$(subst,$(ZIP3HELPVAR),_ $(@:db))}.zip >& $(NULLDEV)
    +-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP3HELPVAR), $(@:db))}) $(ZIP3DIR)) $(command_seperator) zip $(ZIP3FLAGS) $(ZIP3TMP).{$(subst,$(ZIP3HELPVAR),_ $(@:db))}.zip $(ZIP3LIST:s/LANGDIR/./) -x delzip  $(avoid_cvs_dir)
    +$(COPY) $(ZIP3TMP).{$(subst,$(ZIP3HELPVAR),_ $(@:db))}.zip  $@ 
    +$(RM) $(ZIP3TMP).{$(subst,$(ZIP3HELPVAR),_ $(@:db))}.zip
.ELSE			# "$(ZIP3DIR)" != ""
    +-zip $(ZIP3FLAGS) $@ $(foreach,j,$(ZIP3LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP3TARGET), $(@:db))}) $j )) -x delzip $(avoid_cvs_dir)
.ENDIF			# "$(ZIP3DIR)" != ""
.ENDIF			# "$(common_build_zip)"!=""
.ELSE			# "$(make_zip_deps)" == ""
    +echo # > $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.IF "$(common_build_zip)"!=""
.IF "$(ZIP3DIR)" != ""
    +-cd $(subst,LANGDIR,. $(subst,$/LANGDIR, $(ZIP3DIR))) $(command_seperator) $(ZIPDEP) $(ZIP3FLAGS) -prefix $(subst,LANGDIR,. $(subst,$/LANGDIR, $(ZIP3DIR)))$/ $@ $(foreach,j,$(ZIP3LIST) "{$(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP3TARGET), $(@:db))}) $j )}") $(avoid_cvs_dir) >> $(PWD)$/$(PRJ)$/$(ROUT)$/misc$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ELSE			# "$(ZIP3DIR)" != ""
    +-$(ZIPDEP) $(ZIP3FLAGS) $@ $(foreach,j,$(ZIP3LIST) "{$(subst,LANGDIR,$(longlang_{$(subst,$(ZIP3HELPVAR), $(@:db))}) $j )}") $(avoid_cvs_dir) >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ENDIF			# "$(ZIP3DIR)" != ""
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIP3DIR)" != ""
    +-cd $(subst,LANGDIR,. $(subst,$/LANGDIR, $(ZIP3DIR))) $(command_seperator) $(ZIPDEP) $(ZIP3FLAGS) -prefix $(subst,LANGDIR,. $(subst,$/LANGDIR, $(ZIP3DIR)))$/ $@ $(foreach,j,$(ZIP3LIST) "{$(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP3TARGET), $(@:db))}) $j )}") $(avoid_cvs_dir) >> $(PWD)$/$(PRJ)$/$(ROUT)$/misc$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ELSE			# "$(ZIP3DIR)" != ""
    +-$(ZIPDEP) $(ZIP3FLAGS) $@ $(foreach,j,$(ZIP3LIST) "{$(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP3TARGET), $(@:db))}) $j )}") $(avoid_cvs_dir) >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ENDIF			# "$(ZIP3DIR)" != ""
.ENDIF			# "$(common_build_zip)"!=""
    @+echo $@ : makefile.mk >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ENDIF			# "$(make_zip_deps)" == ""
.ENDIF

# Anweisungen fuer das Linken
# unroll begin

.IF "$(GUI)"=="WNT"
command_seperator=&&
#command_seperator=^
avoid_cvs_dir=-x "*CVS*"
.ELSE
command_seperator=;
avoid_cvs_dir=-x "*CVS*"
.ENDIF

.IF "$(ZIP4TARGET)"!=""
ZIP4DIR*=$(ZIPDIR)
ZIP4FLAGS*=$(ZIPFLAGS)
.IF "$(GUI)"=="UNX" || "$(USE_SHELL)"!="4nt"
zip4langdirs:=$(shell +find {$(subst,$/$(LANGDIR), $(null,$(ZIP4DIR) . $(ZIP4DIR)))}/ -type d ! -name CVS ! -name "." | sed "s/\.\/\///" | sed "s/\.\///" )
.ELSE			# "$(GUI)"=="UNX"
zip4langdirs:=$(subst,CVS, $(shell +-dir {$(subst,$/$(LANGDIR), $(ZIP4DIR))} /ba:d ))
.ENDIF			# "$(GUI)"=="UNX"
zip4alllangext:=$(foreach,i,$(alllangext) $(foreach,j,$(zip4langdirs) $(eq,$(longlang_$i),$j  $i $(NULL))))
.ENDIF			# "$(ZIP4TARGET)"!=""

.IF "$(ZIP4TARGETN)"!=""

ZIP4TMP:=$(mktmp iii)

$(MISC)$/$(TARGET).$(PWD:f).$(ZIP4TARGET).dpzz : $(ZIP4TARGETN)

.IF "$(common_build_zip)"!=""
ZIP4HELPVAR={$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP4TARGET)}
.ELSE			# "$(common_build_zip)"!=""
ZIP4HELPVAR=$(BIN)$/$(ZIP4TARGET)
.ENDIF			# "$(common_build_zip)"!=""

.IF "$(make_zip_deps)" == ""
$(ZIP4TARGETN) : delzip
.ELSE			# "$(make_zip_deps)" == ""
$(ZIP4TARGETN) :
.ENDIF			# "$(make_zip_deps)" == ""
.IF "$(make_zip_deps)" == ""
    @+echo ------------------------------
    @+echo Making: $@
    @$(!eq,$?,$(?:s/delzip/zzz/) +-$(RM) +echo) $(uniq $@ $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)) >&$(NULLDEV)
    @+echo rebuilding zipfiles $(eq,$?,$(?:s/delzip/zzz/) >&$(NULLDEV) )
    @+echo ------------------------------ $(eq,$?,$(?:s/delzip/zzz/) >&$(NULLDEV) )
.IF "$(common_build_zip)"!=""
.IF "$(ZIP4DIR)" != ""
    @+-$(GNUCOPY) -p $@ $(ZIP4TMP).{$(subst,$(ZIP4HELPVAR),_ $(@:db))}.zip >& $(NULLDEV)
    +-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP4HELPVAR), $(@:db))}) $(ZIP4DIR)) $(command_seperator) zip $(ZIP4FLAGS) $(ZIP4TMP).{$(subst,$(ZIP4HELPVAR),_ $(@:db))}.zip $(ZIP4LIST:s/LANGDIR/./) -x delzip $(avoid_cvs_dir) 
    +$(COPY) $(ZIP4TMP).{$(subst,$(ZIP4HELPVAR),_ $(@:db))}.zip $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)    
    +$(RM) $(ZIP4TMP).{$(subst,$(ZIP4HELPVAR),_ $(@:db))}.zip
.ELSE			# "$(ZIP4DIR)" != ""
    @+-$(GNUCOPY) -p $@ $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) >& $(NULLDEV)
    +-zip $(ZIP4FLAGS) $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $(foreach,j,$(ZIP4LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP4HELPVAR), $(@:db))}) $j )) -x delzip $(avoid_cvs_dir)
.ENDIF			# "$(ZIP4DIR)" != ""
    @+-$(GNUCOPY) -p $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $@.$(INPATH) >& $(NULLDEV)
    @+-$(RM) $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)
    @+-$(RM) $@ >& $(NULLDEV)
.IF "$(GUI)"=="UNX"
    @+if ( -r $@.$(INPATH) ) $(RENAME) $@.$(INPATH) $@
#	@+if ( -r $@ ) $(TOUCH) $@
.ELSE			# "$(GUI)"=="UNX"
    @+if exist $@.$(INPATH) $(RENAME) $@.$(INPATH) $@
#	@+if exist $@ $(TOUCH) $@
.ENDIF			# "$(GUI)"=="UNX"
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIP4DIR)" != ""
    @+-$(GNUCOPY) -p $@ $(ZIP4TMP).{$(subst,$(ZIP4HELPVAR),_ $(@:db))}.zip >& $(NULLDEV)
    +-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP4HELPVAR), $(@:db))}) $(ZIP4DIR)) $(command_seperator) zip $(ZIP4FLAGS) $(ZIP4TMP).{$(subst,$(ZIP4HELPVAR),_ $(@:db))}.zip $(ZIP4LIST:s/LANGDIR/./) -x delzip  $(avoid_cvs_dir)
    +$(COPY) $(ZIP4TMP).{$(subst,$(ZIP4HELPVAR),_ $(@:db))}.zip  $@ 
    +$(RM) $(ZIP4TMP).{$(subst,$(ZIP4HELPVAR),_ $(@:db))}.zip
.ELSE			# "$(ZIP4DIR)" != ""
    +-zip $(ZIP4FLAGS) $@ $(foreach,j,$(ZIP4LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP4TARGET), $(@:db))}) $j )) -x delzip $(avoid_cvs_dir)
.ENDIF			# "$(ZIP4DIR)" != ""
.ENDIF			# "$(common_build_zip)"!=""
.ELSE			# "$(make_zip_deps)" == ""
    +echo # > $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.IF "$(common_build_zip)"!=""
.IF "$(ZIP4DIR)" != ""
    +-cd $(subst,LANGDIR,. $(subst,$/LANGDIR, $(ZIP4DIR))) $(command_seperator) $(ZIPDEP) $(ZIP4FLAGS) -prefix $(subst,LANGDIR,. $(subst,$/LANGDIR, $(ZIP4DIR)))$/ $@ $(foreach,j,$(ZIP4LIST) "{$(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP4TARGET), $(@:db))}) $j )}") $(avoid_cvs_dir) >> $(PWD)$/$(PRJ)$/$(ROUT)$/misc$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ELSE			# "$(ZIP4DIR)" != ""
    +-$(ZIPDEP) $(ZIP4FLAGS) $@ $(foreach,j,$(ZIP4LIST) "{$(subst,LANGDIR,$(longlang_{$(subst,$(ZIP4HELPVAR), $(@:db))}) $j )}") $(avoid_cvs_dir) >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ENDIF			# "$(ZIP4DIR)" != ""
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIP4DIR)" != ""
    +-cd $(subst,LANGDIR,. $(subst,$/LANGDIR, $(ZIP4DIR))) $(command_seperator) $(ZIPDEP) $(ZIP4FLAGS) -prefix $(subst,LANGDIR,. $(subst,$/LANGDIR, $(ZIP4DIR)))$/ $@ $(foreach,j,$(ZIP4LIST) "{$(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP4TARGET), $(@:db))}) $j )}") $(avoid_cvs_dir) >> $(PWD)$/$(PRJ)$/$(ROUT)$/misc$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ELSE			# "$(ZIP4DIR)" != ""
    +-$(ZIPDEP) $(ZIP4FLAGS) $@ $(foreach,j,$(ZIP4LIST) "{$(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP4TARGET), $(@:db))}) $j )}") $(avoid_cvs_dir) >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ENDIF			# "$(ZIP4DIR)" != ""
.ENDIF			# "$(common_build_zip)"!=""
    @+echo $@ : makefile.mk >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ENDIF			# "$(make_zip_deps)" == ""
.ENDIF

# Anweisungen fuer das Linken
# unroll begin

.IF "$(GUI)"=="WNT"
command_seperator=&&
#command_seperator=^
avoid_cvs_dir=-x "*CVS*"
.ELSE
command_seperator=;
avoid_cvs_dir=-x "*CVS*"
.ENDIF

.IF "$(ZIP5TARGET)"!=""
ZIP5DIR*=$(ZIPDIR)
ZIP5FLAGS*=$(ZIPFLAGS)
.IF "$(GUI)"=="UNX" || "$(USE_SHELL)"!="4nt"
zip5langdirs:=$(shell +find {$(subst,$/$(LANGDIR), $(null,$(ZIP5DIR) . $(ZIP5DIR)))}/ -type d ! -name CVS ! -name "." | sed "s/\.\/\///" | sed "s/\.\///" )
.ELSE			# "$(GUI)"=="UNX"
zip5langdirs:=$(subst,CVS, $(shell +-dir {$(subst,$/$(LANGDIR), $(ZIP5DIR))} /ba:d ))
.ENDIF			# "$(GUI)"=="UNX"
zip5alllangext:=$(foreach,i,$(alllangext) $(foreach,j,$(zip5langdirs) $(eq,$(longlang_$i),$j  $i $(NULL))))
.ENDIF			# "$(ZIP5TARGET)"!=""

.IF "$(ZIP5TARGETN)"!=""

ZIP5TMP:=$(mktmp iii)

$(MISC)$/$(TARGET).$(PWD:f).$(ZIP5TARGET).dpzz : $(ZIP5TARGETN)

.IF "$(common_build_zip)"!=""
ZIP5HELPVAR={$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP5TARGET)}
.ELSE			# "$(common_build_zip)"!=""
ZIP5HELPVAR=$(BIN)$/$(ZIP5TARGET)
.ENDIF			# "$(common_build_zip)"!=""

.IF "$(make_zip_deps)" == ""
$(ZIP5TARGETN) : delzip
.ELSE			# "$(make_zip_deps)" == ""
$(ZIP5TARGETN) :
.ENDIF			# "$(make_zip_deps)" == ""
.IF "$(make_zip_deps)" == ""
    @+echo ------------------------------
    @+echo Making: $@
    @$(!eq,$?,$(?:s/delzip/zzz/) +-$(RM) +echo) $(uniq $@ $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)) >&$(NULLDEV)
    @+echo rebuilding zipfiles $(eq,$?,$(?:s/delzip/zzz/) >&$(NULLDEV) )
    @+echo ------------------------------ $(eq,$?,$(?:s/delzip/zzz/) >&$(NULLDEV) )
.IF "$(common_build_zip)"!=""
.IF "$(ZIP5DIR)" != ""
    @+-$(GNUCOPY) -p $@ $(ZIP5TMP).{$(subst,$(ZIP5HELPVAR),_ $(@:db))}.zip >& $(NULLDEV)
    +-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP5HELPVAR), $(@:db))}) $(ZIP5DIR)) $(command_seperator) zip $(ZIP5FLAGS) $(ZIP5TMP).{$(subst,$(ZIP5HELPVAR),_ $(@:db))}.zip $(ZIP5LIST:s/LANGDIR/./) -x delzip $(avoid_cvs_dir) 
    +$(COPY) $(ZIP5TMP).{$(subst,$(ZIP5HELPVAR),_ $(@:db))}.zip $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)    
    +$(RM) $(ZIP5TMP).{$(subst,$(ZIP5HELPVAR),_ $(@:db))}.zip
.ELSE			# "$(ZIP5DIR)" != ""
    @+-$(GNUCOPY) -p $@ $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) >& $(NULLDEV)
    +-zip $(ZIP5FLAGS) $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $(foreach,j,$(ZIP5LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP5HELPVAR), $(@:db))}) $j )) -x delzip $(avoid_cvs_dir)
.ENDIF			# "$(ZIP5DIR)" != ""
    @+-$(GNUCOPY) -p $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $@.$(INPATH) >& $(NULLDEV)
    @+-$(RM) $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)
    @+-$(RM) $@ >& $(NULLDEV)
.IF "$(GUI)"=="UNX"
    @+if ( -r $@.$(INPATH) ) $(RENAME) $@.$(INPATH) $@
#	@+if ( -r $@ ) $(TOUCH) $@
.ELSE			# "$(GUI)"=="UNX"
    @+if exist $@.$(INPATH) $(RENAME) $@.$(INPATH) $@
#	@+if exist $@ $(TOUCH) $@
.ENDIF			# "$(GUI)"=="UNX"
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIP5DIR)" != ""
    @+-$(GNUCOPY) -p $@ $(ZIP5TMP).{$(subst,$(ZIP5HELPVAR),_ $(@:db))}.zip >& $(NULLDEV)
    +-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP5HELPVAR), $(@:db))}) $(ZIP5DIR)) $(command_seperator) zip $(ZIP5FLAGS) $(ZIP5TMP).{$(subst,$(ZIP5HELPVAR),_ $(@:db))}.zip $(ZIP5LIST:s/LANGDIR/./) -x delzip  $(avoid_cvs_dir)
    +$(COPY) $(ZIP5TMP).{$(subst,$(ZIP5HELPVAR),_ $(@:db))}.zip  $@ 
    +$(RM) $(ZIP5TMP).{$(subst,$(ZIP5HELPVAR),_ $(@:db))}.zip
.ELSE			# "$(ZIP5DIR)" != ""
    +-zip $(ZIP5FLAGS) $@ $(foreach,j,$(ZIP5LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP5TARGET), $(@:db))}) $j )) -x delzip $(avoid_cvs_dir)
.ENDIF			# "$(ZIP5DIR)" != ""
.ENDIF			# "$(common_build_zip)"!=""
.ELSE			# "$(make_zip_deps)" == ""
    +echo # > $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.IF "$(common_build_zip)"!=""
.IF "$(ZIP5DIR)" != ""
    +-cd $(subst,LANGDIR,. $(subst,$/LANGDIR, $(ZIP5DIR))) $(command_seperator) $(ZIPDEP) $(ZIP5FLAGS) -prefix $(subst,LANGDIR,. $(subst,$/LANGDIR, $(ZIP5DIR)))$/ $@ $(foreach,j,$(ZIP5LIST) "{$(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP5TARGET), $(@:db))}) $j )}") $(avoid_cvs_dir) >> $(PWD)$/$(PRJ)$/$(ROUT)$/misc$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ELSE			# "$(ZIP5DIR)" != ""
    +-$(ZIPDEP) $(ZIP5FLAGS) $@ $(foreach,j,$(ZIP5LIST) "{$(subst,LANGDIR,$(longlang_{$(subst,$(ZIP5HELPVAR), $(@:db))}) $j )}") $(avoid_cvs_dir) >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ENDIF			# "$(ZIP5DIR)" != ""
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIP5DIR)" != ""
    +-cd $(subst,LANGDIR,. $(subst,$/LANGDIR, $(ZIP5DIR))) $(command_seperator) $(ZIPDEP) $(ZIP5FLAGS) -prefix $(subst,LANGDIR,. $(subst,$/LANGDIR, $(ZIP5DIR)))$/ $@ $(foreach,j,$(ZIP5LIST) "{$(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP5TARGET), $(@:db))}) $j )}") $(avoid_cvs_dir) >> $(PWD)$/$(PRJ)$/$(ROUT)$/misc$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ELSE			# "$(ZIP5DIR)" != ""
    +-$(ZIPDEP) $(ZIP5FLAGS) $@ $(foreach,j,$(ZIP5LIST) "{$(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP5TARGET), $(@:db))}) $j )}") $(avoid_cvs_dir) >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ENDIF			# "$(ZIP5DIR)" != ""
.ENDIF			# "$(common_build_zip)"!=""
    @+echo $@ : makefile.mk >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ENDIF			# "$(make_zip_deps)" == ""
.ENDIF

# Anweisungen fuer das Linken
# unroll begin

.IF "$(GUI)"=="WNT"
command_seperator=&&
#command_seperator=^
avoid_cvs_dir=-x "*CVS*"
.ELSE
command_seperator=;
avoid_cvs_dir=-x "*CVS*"
.ENDIF

.IF "$(ZIP6TARGET)"!=""
ZIP6DIR*=$(ZIPDIR)
ZIP6FLAGS*=$(ZIPFLAGS)
.IF "$(GUI)"=="UNX" || "$(USE_SHELL)"!="4nt"
zip6langdirs:=$(shell +find {$(subst,$/$(LANGDIR), $(null,$(ZIP6DIR) . $(ZIP6DIR)))}/ -type d ! -name CVS ! -name "." | sed "s/\.\/\///" | sed "s/\.\///" )
.ELSE			# "$(GUI)"=="UNX"
zip6langdirs:=$(subst,CVS, $(shell +-dir {$(subst,$/$(LANGDIR), $(ZIP6DIR))} /ba:d ))
.ENDIF			# "$(GUI)"=="UNX"
zip6alllangext:=$(foreach,i,$(alllangext) $(foreach,j,$(zip6langdirs) $(eq,$(longlang_$i),$j  $i $(NULL))))
.ENDIF			# "$(ZIP6TARGET)"!=""

.IF "$(ZIP6TARGETN)"!=""

ZIP6TMP:=$(mktmp iii)

$(MISC)$/$(TARGET).$(PWD:f).$(ZIP6TARGET).dpzz : $(ZIP6TARGETN)

.IF "$(common_build_zip)"!=""
ZIP6HELPVAR={$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP6TARGET)}
.ELSE			# "$(common_build_zip)"!=""
ZIP6HELPVAR=$(BIN)$/$(ZIP6TARGET)
.ENDIF			# "$(common_build_zip)"!=""

.IF "$(make_zip_deps)" == ""
$(ZIP6TARGETN) : delzip
.ELSE			# "$(make_zip_deps)" == ""
$(ZIP6TARGETN) :
.ENDIF			# "$(make_zip_deps)" == ""
.IF "$(make_zip_deps)" == ""
    @+echo ------------------------------
    @+echo Making: $@
    @$(!eq,$?,$(?:s/delzip/zzz/) +-$(RM) +echo) $(uniq $@ $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)) >&$(NULLDEV)
    @+echo rebuilding zipfiles $(eq,$?,$(?:s/delzip/zzz/) >&$(NULLDEV) )
    @+echo ------------------------------ $(eq,$?,$(?:s/delzip/zzz/) >&$(NULLDEV) )
.IF "$(common_build_zip)"!=""
.IF "$(ZIP6DIR)" != ""
    @+-$(GNUCOPY) -p $@ $(ZIP6TMP).{$(subst,$(ZIP6HELPVAR),_ $(@:db))}.zip >& $(NULLDEV)
    +-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP6HELPVAR), $(@:db))}) $(ZIP6DIR)) $(command_seperator) zip $(ZIP6FLAGS) $(ZIP6TMP).{$(subst,$(ZIP6HELPVAR),_ $(@:db))}.zip $(ZIP6LIST:s/LANGDIR/./) -x delzip $(avoid_cvs_dir) 
    +$(COPY) $(ZIP6TMP).{$(subst,$(ZIP6HELPVAR),_ $(@:db))}.zip $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)    
    +$(RM) $(ZIP6TMP).{$(subst,$(ZIP6HELPVAR),_ $(@:db))}.zip
.ELSE			# "$(ZIP6DIR)" != ""
    @+-$(GNUCOPY) -p $@ $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) >& $(NULLDEV)
    +-zip $(ZIP6FLAGS) $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $(foreach,j,$(ZIP6LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP6HELPVAR), $(@:db))}) $j )) -x delzip $(avoid_cvs_dir)
.ENDIF			# "$(ZIP6DIR)" != ""
    @+-$(GNUCOPY) -p $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $@.$(INPATH) >& $(NULLDEV)
    @+-$(RM) $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)
    @+-$(RM) $@ >& $(NULLDEV)
.IF "$(GUI)"=="UNX"
    @+if ( -r $@.$(INPATH) ) $(RENAME) $@.$(INPATH) $@
#	@+if ( -r $@ ) $(TOUCH) $@
.ELSE			# "$(GUI)"=="UNX"
    @+if exist $@.$(INPATH) $(RENAME) $@.$(INPATH) $@
#	@+if exist $@ $(TOUCH) $@
.ENDIF			# "$(GUI)"=="UNX"
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIP6DIR)" != ""
    @+-$(GNUCOPY) -p $@ $(ZIP6TMP).{$(subst,$(ZIP6HELPVAR),_ $(@:db))}.zip >& $(NULLDEV)
    +-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP6HELPVAR), $(@:db))}) $(ZIP6DIR)) $(command_seperator) zip $(ZIP6FLAGS) $(ZIP6TMP).{$(subst,$(ZIP6HELPVAR),_ $(@:db))}.zip $(ZIP6LIST:s/LANGDIR/./) -x delzip  $(avoid_cvs_dir)
    +$(COPY) $(ZIP6TMP).{$(subst,$(ZIP6HELPVAR),_ $(@:db))}.zip  $@ 
    +$(RM) $(ZIP6TMP).{$(subst,$(ZIP6HELPVAR),_ $(@:db))}.zip
.ELSE			# "$(ZIP6DIR)" != ""
    +-zip $(ZIP6FLAGS) $@ $(foreach,j,$(ZIP6LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP6TARGET), $(@:db))}) $j )) -x delzip $(avoid_cvs_dir)
.ENDIF			# "$(ZIP6DIR)" != ""
.ENDIF			# "$(common_build_zip)"!=""
.ELSE			# "$(make_zip_deps)" == ""
    +echo # > $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.IF "$(common_build_zip)"!=""
.IF "$(ZIP6DIR)" != ""
    +-cd $(subst,LANGDIR,. $(subst,$/LANGDIR, $(ZIP6DIR))) $(command_seperator) $(ZIPDEP) $(ZIP6FLAGS) -prefix $(subst,LANGDIR,. $(subst,$/LANGDIR, $(ZIP6DIR)))$/ $@ $(foreach,j,$(ZIP6LIST) "{$(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP6TARGET), $(@:db))}) $j )}") $(avoid_cvs_dir) >> $(PWD)$/$(PRJ)$/$(ROUT)$/misc$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ELSE			# "$(ZIP6DIR)" != ""
    +-$(ZIPDEP) $(ZIP6FLAGS) $@ $(foreach,j,$(ZIP6LIST) "{$(subst,LANGDIR,$(longlang_{$(subst,$(ZIP6HELPVAR), $(@:db))}) $j )}") $(avoid_cvs_dir) >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ENDIF			# "$(ZIP6DIR)" != ""
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIP6DIR)" != ""
    +-cd $(subst,LANGDIR,. $(subst,$/LANGDIR, $(ZIP6DIR))) $(command_seperator) $(ZIPDEP) $(ZIP6FLAGS) -prefix $(subst,LANGDIR,. $(subst,$/LANGDIR, $(ZIP6DIR)))$/ $@ $(foreach,j,$(ZIP6LIST) "{$(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP6TARGET), $(@:db))}) $j )}") $(avoid_cvs_dir) >> $(PWD)$/$(PRJ)$/$(ROUT)$/misc$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ELSE			# "$(ZIP6DIR)" != ""
    +-$(ZIPDEP) $(ZIP6FLAGS) $@ $(foreach,j,$(ZIP6LIST) "{$(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP6TARGET), $(@:db))}) $j )}") $(avoid_cvs_dir) >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ENDIF			# "$(ZIP6DIR)" != ""
.ENDIF			# "$(common_build_zip)"!=""
    @+echo $@ : makefile.mk >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ENDIF			# "$(make_zip_deps)" == ""
.ENDIF

# Anweisungen fuer das Linken
# unroll begin

.IF "$(GUI)"=="WNT"
command_seperator=&&
#command_seperator=^
avoid_cvs_dir=-x "*CVS*"
.ELSE
command_seperator=;
avoid_cvs_dir=-x "*CVS*"
.ENDIF

.IF "$(ZIP7TARGET)"!=""
ZIP7DIR*=$(ZIPDIR)
ZIP7FLAGS*=$(ZIPFLAGS)
.IF "$(GUI)"=="UNX" || "$(USE_SHELL)"!="4nt"
zip7langdirs:=$(shell +find {$(subst,$/$(LANGDIR), $(null,$(ZIP7DIR) . $(ZIP7DIR)))}/ -type d ! -name CVS ! -name "." | sed "s/\.\/\///" | sed "s/\.\///" )
.ELSE			# "$(GUI)"=="UNX"
zip7langdirs:=$(subst,CVS, $(shell +-dir {$(subst,$/$(LANGDIR), $(ZIP7DIR))} /ba:d ))
.ENDIF			# "$(GUI)"=="UNX"
zip7alllangext:=$(foreach,i,$(alllangext) $(foreach,j,$(zip7langdirs) $(eq,$(longlang_$i),$j  $i $(NULL))))
.ENDIF			# "$(ZIP7TARGET)"!=""

.IF "$(ZIP7TARGETN)"!=""

ZIP7TMP:=$(mktmp iii)

$(MISC)$/$(TARGET).$(PWD:f).$(ZIP7TARGET).dpzz : $(ZIP7TARGETN)

.IF "$(common_build_zip)"!=""
ZIP7HELPVAR={$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP7TARGET)}
.ELSE			# "$(common_build_zip)"!=""
ZIP7HELPVAR=$(BIN)$/$(ZIP7TARGET)
.ENDIF			# "$(common_build_zip)"!=""

.IF "$(make_zip_deps)" == ""
$(ZIP7TARGETN) : delzip
.ELSE			# "$(make_zip_deps)" == ""
$(ZIP7TARGETN) :
.ENDIF			# "$(make_zip_deps)" == ""
.IF "$(make_zip_deps)" == ""
    @+echo ------------------------------
    @+echo Making: $@
    @$(!eq,$?,$(?:s/delzip/zzz/) +-$(RM) +echo) $(uniq $@ $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)) >&$(NULLDEV)
    @+echo rebuilding zipfiles $(eq,$?,$(?:s/delzip/zzz/) >&$(NULLDEV) )
    @+echo ------------------------------ $(eq,$?,$(?:s/delzip/zzz/) >&$(NULLDEV) )
.IF "$(common_build_zip)"!=""
.IF "$(ZIP7DIR)" != ""
    @+-$(GNUCOPY) -p $@ $(ZIP7TMP).{$(subst,$(ZIP7HELPVAR),_ $(@:db))}.zip >& $(NULLDEV)
    +-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP7HELPVAR), $(@:db))}) $(ZIP7DIR)) $(command_seperator) zip $(ZIP7FLAGS) $(ZIP7TMP).{$(subst,$(ZIP7HELPVAR),_ $(@:db))}.zip $(ZIP7LIST:s/LANGDIR/./) -x delzip $(avoid_cvs_dir) 
    +$(COPY) $(ZIP7TMP).{$(subst,$(ZIP7HELPVAR),_ $(@:db))}.zip $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)    
    +$(RM) $(ZIP7TMP).{$(subst,$(ZIP7HELPVAR),_ $(@:db))}.zip
.ELSE			# "$(ZIP7DIR)" != ""
    @+-$(GNUCOPY) -p $@ $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) >& $(NULLDEV)
    +-zip $(ZIP7FLAGS) $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $(foreach,j,$(ZIP7LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP7HELPVAR), $(@:db))}) $j )) -x delzip $(avoid_cvs_dir)
.ENDIF			# "$(ZIP7DIR)" != ""
    @+-$(GNUCOPY) -p $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $@.$(INPATH) >& $(NULLDEV)
    @+-$(RM) $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)
    @+-$(RM) $@ >& $(NULLDEV)
.IF "$(GUI)"=="UNX"
    @+if ( -r $@.$(INPATH) ) $(RENAME) $@.$(INPATH) $@
#	@+if ( -r $@ ) $(TOUCH) $@
.ELSE			# "$(GUI)"=="UNX"
    @+if exist $@.$(INPATH) $(RENAME) $@.$(INPATH) $@
#	@+if exist $@ $(TOUCH) $@
.ENDIF			# "$(GUI)"=="UNX"
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIP7DIR)" != ""
    @+-$(GNUCOPY) -p $@ $(ZIP7TMP).{$(subst,$(ZIP7HELPVAR),_ $(@:db))}.zip >& $(NULLDEV)
    +-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP7HELPVAR), $(@:db))}) $(ZIP7DIR)) $(command_seperator) zip $(ZIP7FLAGS) $(ZIP7TMP).{$(subst,$(ZIP7HELPVAR),_ $(@:db))}.zip $(ZIP7LIST:s/LANGDIR/./) -x delzip  $(avoid_cvs_dir)
    +$(COPY) $(ZIP7TMP).{$(subst,$(ZIP7HELPVAR),_ $(@:db))}.zip  $@ 
    +$(RM) $(ZIP7TMP).{$(subst,$(ZIP7HELPVAR),_ $(@:db))}.zip
.ELSE			# "$(ZIP7DIR)" != ""
    +-zip $(ZIP7FLAGS) $@ $(foreach,j,$(ZIP7LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP7TARGET), $(@:db))}) $j )) -x delzip $(avoid_cvs_dir)
.ENDIF			# "$(ZIP7DIR)" != ""
.ENDIF			# "$(common_build_zip)"!=""
.ELSE			# "$(make_zip_deps)" == ""
    +echo # > $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.IF "$(common_build_zip)"!=""
.IF "$(ZIP7DIR)" != ""
    +-cd $(subst,LANGDIR,. $(subst,$/LANGDIR, $(ZIP7DIR))) $(command_seperator) $(ZIPDEP) $(ZIP7FLAGS) -prefix $(subst,LANGDIR,. $(subst,$/LANGDIR, $(ZIP7DIR)))$/ $@ $(foreach,j,$(ZIP7LIST) "{$(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP7TARGET), $(@:db))}) $j )}") $(avoid_cvs_dir) >> $(PWD)$/$(PRJ)$/$(ROUT)$/misc$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ELSE			# "$(ZIP7DIR)" != ""
    +-$(ZIPDEP) $(ZIP7FLAGS) $@ $(foreach,j,$(ZIP7LIST) "{$(subst,LANGDIR,$(longlang_{$(subst,$(ZIP7HELPVAR), $(@:db))}) $j )}") $(avoid_cvs_dir) >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ENDIF			# "$(ZIP7DIR)" != ""
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIP7DIR)" != ""
    +-cd $(subst,LANGDIR,. $(subst,$/LANGDIR, $(ZIP7DIR))) $(command_seperator) $(ZIPDEP) $(ZIP7FLAGS) -prefix $(subst,LANGDIR,. $(subst,$/LANGDIR, $(ZIP7DIR)))$/ $@ $(foreach,j,$(ZIP7LIST) "{$(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP7TARGET), $(@:db))}) $j )}") $(avoid_cvs_dir) >> $(PWD)$/$(PRJ)$/$(ROUT)$/misc$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ELSE			# "$(ZIP7DIR)" != ""
    +-$(ZIPDEP) $(ZIP7FLAGS) $@ $(foreach,j,$(ZIP7LIST) "{$(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP7TARGET), $(@:db))}) $j )}") $(avoid_cvs_dir) >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ENDIF			# "$(ZIP7DIR)" != ""
.ENDIF			# "$(common_build_zip)"!=""
    @+echo $@ : makefile.mk >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ENDIF			# "$(make_zip_deps)" == ""
.ENDIF

# Anweisungen fuer das Linken
# unroll begin

.IF "$(GUI)"=="WNT"
command_seperator=&&
#command_seperator=^
avoid_cvs_dir=-x "*CVS*"
.ELSE
command_seperator=;
avoid_cvs_dir=-x "*CVS*"
.ENDIF

.IF "$(ZIP8TARGET)"!=""
ZIP8DIR*=$(ZIPDIR)
ZIP8FLAGS*=$(ZIPFLAGS)
.IF "$(GUI)"=="UNX" || "$(USE_SHELL)"!="4nt"
zip8langdirs:=$(shell +find {$(subst,$/$(LANGDIR), $(null,$(ZIP8DIR) . $(ZIP8DIR)))}/ -type d ! -name CVS ! -name "." | sed "s/\.\/\///" | sed "s/\.\///" )
.ELSE			# "$(GUI)"=="UNX"
zip8langdirs:=$(subst,CVS, $(shell +-dir {$(subst,$/$(LANGDIR), $(ZIP8DIR))} /ba:d ))
.ENDIF			# "$(GUI)"=="UNX"
zip8alllangext:=$(foreach,i,$(alllangext) $(foreach,j,$(zip8langdirs) $(eq,$(longlang_$i),$j  $i $(NULL))))
.ENDIF			# "$(ZIP8TARGET)"!=""

.IF "$(ZIP8TARGETN)"!=""

ZIP8TMP:=$(mktmp iii)

$(MISC)$/$(TARGET).$(PWD:f).$(ZIP8TARGET).dpzz : $(ZIP8TARGETN)

.IF "$(common_build_zip)"!=""
ZIP8HELPVAR={$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP8TARGET)}
.ELSE			# "$(common_build_zip)"!=""
ZIP8HELPVAR=$(BIN)$/$(ZIP8TARGET)
.ENDIF			# "$(common_build_zip)"!=""

.IF "$(make_zip_deps)" == ""
$(ZIP8TARGETN) : delzip
.ELSE			# "$(make_zip_deps)" == ""
$(ZIP8TARGETN) :
.ENDIF			# "$(make_zip_deps)" == ""
.IF "$(make_zip_deps)" == ""
    @+echo ------------------------------
    @+echo Making: $@
    @$(!eq,$?,$(?:s/delzip/zzz/) +-$(RM) +echo) $(uniq $@ $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)) >&$(NULLDEV)
    @+echo rebuilding zipfiles $(eq,$?,$(?:s/delzip/zzz/) >&$(NULLDEV) )
    @+echo ------------------------------ $(eq,$?,$(?:s/delzip/zzz/) >&$(NULLDEV) )
.IF "$(common_build_zip)"!=""
.IF "$(ZIP8DIR)" != ""
    @+-$(GNUCOPY) -p $@ $(ZIP8TMP).{$(subst,$(ZIP8HELPVAR),_ $(@:db))}.zip >& $(NULLDEV)
    +-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP8HELPVAR), $(@:db))}) $(ZIP8DIR)) $(command_seperator) zip $(ZIP8FLAGS) $(ZIP8TMP).{$(subst,$(ZIP8HELPVAR),_ $(@:db))}.zip $(ZIP8LIST:s/LANGDIR/./) -x delzip $(avoid_cvs_dir) 
    +$(COPY) $(ZIP8TMP).{$(subst,$(ZIP8HELPVAR),_ $(@:db))}.zip $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)    
    +$(RM) $(ZIP8TMP).{$(subst,$(ZIP8HELPVAR),_ $(@:db))}.zip
.ELSE			# "$(ZIP8DIR)" != ""
    @+-$(GNUCOPY) -p $@ $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) >& $(NULLDEV)
    +-zip $(ZIP8FLAGS) $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $(foreach,j,$(ZIP8LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP8HELPVAR), $(@:db))}) $j )) -x delzip $(avoid_cvs_dir)
.ENDIF			# "$(ZIP8DIR)" != ""
    @+-$(GNUCOPY) -p $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $@.$(INPATH) >& $(NULLDEV)
    @+-$(RM) $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)
    @+-$(RM) $@ >& $(NULLDEV)
.IF "$(GUI)"=="UNX"
    @+if ( -r $@.$(INPATH) ) $(RENAME) $@.$(INPATH) $@
#	@+if ( -r $@ ) $(TOUCH) $@
.ELSE			# "$(GUI)"=="UNX"
    @+if exist $@.$(INPATH) $(RENAME) $@.$(INPATH) $@
#	@+if exist $@ $(TOUCH) $@
.ENDIF			# "$(GUI)"=="UNX"
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIP8DIR)" != ""
    @+-$(GNUCOPY) -p $@ $(ZIP8TMP).{$(subst,$(ZIP8HELPVAR),_ $(@:db))}.zip >& $(NULLDEV)
    +-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP8HELPVAR), $(@:db))}) $(ZIP8DIR)) $(command_seperator) zip $(ZIP8FLAGS) $(ZIP8TMP).{$(subst,$(ZIP8HELPVAR),_ $(@:db))}.zip $(ZIP8LIST:s/LANGDIR/./) -x delzip  $(avoid_cvs_dir)
    +$(COPY) $(ZIP8TMP).{$(subst,$(ZIP8HELPVAR),_ $(@:db))}.zip  $@ 
    +$(RM) $(ZIP8TMP).{$(subst,$(ZIP8HELPVAR),_ $(@:db))}.zip
.ELSE			# "$(ZIP8DIR)" != ""
    +-zip $(ZIP8FLAGS) $@ $(foreach,j,$(ZIP8LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP8TARGET), $(@:db))}) $j )) -x delzip $(avoid_cvs_dir)
.ENDIF			# "$(ZIP8DIR)" != ""
.ENDIF			# "$(common_build_zip)"!=""
.ELSE			# "$(make_zip_deps)" == ""
    +echo # > $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.IF "$(common_build_zip)"!=""
.IF "$(ZIP8DIR)" != ""
    +-cd $(subst,LANGDIR,. $(subst,$/LANGDIR, $(ZIP8DIR))) $(command_seperator) $(ZIPDEP) $(ZIP8FLAGS) -prefix $(subst,LANGDIR,. $(subst,$/LANGDIR, $(ZIP8DIR)))$/ $@ $(foreach,j,$(ZIP8LIST) "{$(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP8TARGET), $(@:db))}) $j )}") $(avoid_cvs_dir) >> $(PWD)$/$(PRJ)$/$(ROUT)$/misc$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ELSE			# "$(ZIP8DIR)" != ""
    +-$(ZIPDEP) $(ZIP8FLAGS) $@ $(foreach,j,$(ZIP8LIST) "{$(subst,LANGDIR,$(longlang_{$(subst,$(ZIP8HELPVAR), $(@:db))}) $j )}") $(avoid_cvs_dir) >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ENDIF			# "$(ZIP8DIR)" != ""
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIP8DIR)" != ""
    +-cd $(subst,LANGDIR,. $(subst,$/LANGDIR, $(ZIP8DIR))) $(command_seperator) $(ZIPDEP) $(ZIP8FLAGS) -prefix $(subst,LANGDIR,. $(subst,$/LANGDIR, $(ZIP8DIR)))$/ $@ $(foreach,j,$(ZIP8LIST) "{$(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP8TARGET), $(@:db))}) $j )}") $(avoid_cvs_dir) >> $(PWD)$/$(PRJ)$/$(ROUT)$/misc$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ELSE			# "$(ZIP8DIR)" != ""
    +-$(ZIPDEP) $(ZIP8FLAGS) $@ $(foreach,j,$(ZIP8LIST) "{$(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP8TARGET), $(@:db))}) $j )}") $(avoid_cvs_dir) >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ENDIF			# "$(ZIP8DIR)" != ""
.ENDIF			# "$(common_build_zip)"!=""
    @+echo $@ : makefile.mk >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ENDIF			# "$(make_zip_deps)" == ""
.ENDIF

# Anweisungen fuer das Linken
# unroll begin

.IF "$(GUI)"=="WNT"
command_seperator=&&
#command_seperator=^
avoid_cvs_dir=-x "*CVS*"
.ELSE
command_seperator=;
avoid_cvs_dir=-x "*CVS*"
.ENDIF

.IF "$(ZIP9TARGET)"!=""
ZIP9DIR*=$(ZIPDIR)
ZIP9FLAGS*=$(ZIPFLAGS)
.IF "$(GUI)"=="UNX" || "$(USE_SHELL)"!="4nt"
zip9langdirs:=$(shell +find {$(subst,$/$(LANGDIR), $(null,$(ZIP9DIR) . $(ZIP9DIR)))}/ -type d ! -name CVS ! -name "." | sed "s/\.\/\///" | sed "s/\.\///" )
.ELSE			# "$(GUI)"=="UNX"
zip9langdirs:=$(subst,CVS, $(shell +-dir {$(subst,$/$(LANGDIR), $(ZIP9DIR))} /ba:d ))
.ENDIF			# "$(GUI)"=="UNX"
zip9alllangext:=$(foreach,i,$(alllangext) $(foreach,j,$(zip9langdirs) $(eq,$(longlang_$i),$j  $i $(NULL))))
.ENDIF			# "$(ZIP9TARGET)"!=""

.IF "$(ZIP9TARGETN)"!=""

ZIP9TMP:=$(mktmp iii)

$(MISC)$/$(TARGET).$(PWD:f).$(ZIP9TARGET).dpzz : $(ZIP9TARGETN)

.IF "$(common_build_zip)"!=""
ZIP9HELPVAR={$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP9TARGET)}
.ELSE			# "$(common_build_zip)"!=""
ZIP9HELPVAR=$(BIN)$/$(ZIP9TARGET)
.ENDIF			# "$(common_build_zip)"!=""

.IF "$(make_zip_deps)" == ""
$(ZIP9TARGETN) : delzip
.ELSE			# "$(make_zip_deps)" == ""
$(ZIP9TARGETN) :
.ENDIF			# "$(make_zip_deps)" == ""
.IF "$(make_zip_deps)" == ""
    @+echo ------------------------------
    @+echo Making: $@
    @$(!eq,$?,$(?:s/delzip/zzz/) +-$(RM) +echo) $(uniq $@ $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)) >&$(NULLDEV)
    @+echo rebuilding zipfiles $(eq,$?,$(?:s/delzip/zzz/) >&$(NULLDEV) )
    @+echo ------------------------------ $(eq,$?,$(?:s/delzip/zzz/) >&$(NULLDEV) )
.IF "$(common_build_zip)"!=""
.IF "$(ZIP9DIR)" != ""
    @+-$(GNUCOPY) -p $@ $(ZIP9TMP).{$(subst,$(ZIP9HELPVAR),_ $(@:db))}.zip >& $(NULLDEV)
    +-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP9HELPVAR), $(@:db))}) $(ZIP9DIR)) $(command_seperator) zip $(ZIP9FLAGS) $(ZIP9TMP).{$(subst,$(ZIP9HELPVAR),_ $(@:db))}.zip $(ZIP9LIST:s/LANGDIR/./) -x delzip $(avoid_cvs_dir) 
    +$(COPY) $(ZIP9TMP).{$(subst,$(ZIP9HELPVAR),_ $(@:db))}.zip $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)    
    +$(RM) $(ZIP9TMP).{$(subst,$(ZIP9HELPVAR),_ $(@:db))}.zip
.ELSE			# "$(ZIP9DIR)" != ""
    @+-$(GNUCOPY) -p $@ $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) >& $(NULLDEV)
    +-zip $(ZIP9FLAGS) $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $(foreach,j,$(ZIP9LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP9HELPVAR), $(@:db))}) $j )) -x delzip $(avoid_cvs_dir)
.ENDIF			# "$(ZIP9DIR)" != ""
    @+-$(GNUCOPY) -p $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $@.$(INPATH) >& $(NULLDEV)
    @+-$(RM) $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)
    @+-$(RM) $@ >& $(NULLDEV)
.IF "$(GUI)"=="UNX"
    @+if ( -r $@.$(INPATH) ) $(RENAME) $@.$(INPATH) $@
#	@+if ( -r $@ ) $(TOUCH) $@
.ELSE			# "$(GUI)"=="UNX"
    @+if exist $@.$(INPATH) $(RENAME) $@.$(INPATH) $@
#	@+if exist $@ $(TOUCH) $@
.ENDIF			# "$(GUI)"=="UNX"
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIP9DIR)" != ""
    @+-$(GNUCOPY) -p $@ $(ZIP9TMP).{$(subst,$(ZIP9HELPVAR),_ $(@:db))}.zip >& $(NULLDEV)
    +-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP9HELPVAR), $(@:db))}) $(ZIP9DIR)) $(command_seperator) zip $(ZIP9FLAGS) $(ZIP9TMP).{$(subst,$(ZIP9HELPVAR),_ $(@:db))}.zip $(ZIP9LIST:s/LANGDIR/./) -x delzip  $(avoid_cvs_dir)
    +$(COPY) $(ZIP9TMP).{$(subst,$(ZIP9HELPVAR),_ $(@:db))}.zip  $@ 
    +$(RM) $(ZIP9TMP).{$(subst,$(ZIP9HELPVAR),_ $(@:db))}.zip
.ELSE			# "$(ZIP9DIR)" != ""
    +-zip $(ZIP9FLAGS) $@ $(foreach,j,$(ZIP9LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP9TARGET), $(@:db))}) $j )) -x delzip $(avoid_cvs_dir)
.ENDIF			# "$(ZIP9DIR)" != ""
.ENDIF			# "$(common_build_zip)"!=""
.ELSE			# "$(make_zip_deps)" == ""
    +echo # > $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.IF "$(common_build_zip)"!=""
.IF "$(ZIP9DIR)" != ""
    +-cd $(subst,LANGDIR,. $(subst,$/LANGDIR, $(ZIP9DIR))) $(command_seperator) $(ZIPDEP) $(ZIP9FLAGS) -prefix $(subst,LANGDIR,. $(subst,$/LANGDIR, $(ZIP9DIR)))$/ $@ $(foreach,j,$(ZIP9LIST) "{$(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP9TARGET), $(@:db))}) $j )}") $(avoid_cvs_dir) >> $(PWD)$/$(PRJ)$/$(ROUT)$/misc$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ELSE			# "$(ZIP9DIR)" != ""
    +-$(ZIPDEP) $(ZIP9FLAGS) $@ $(foreach,j,$(ZIP9LIST) "{$(subst,LANGDIR,$(longlang_{$(subst,$(ZIP9HELPVAR), $(@:db))}) $j )}") $(avoid_cvs_dir) >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ENDIF			# "$(ZIP9DIR)" != ""
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIP9DIR)" != ""
    +-cd $(subst,LANGDIR,. $(subst,$/LANGDIR, $(ZIP9DIR))) $(command_seperator) $(ZIPDEP) $(ZIP9FLAGS) -prefix $(subst,LANGDIR,. $(subst,$/LANGDIR, $(ZIP9DIR)))$/ $@ $(foreach,j,$(ZIP9LIST) "{$(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP9TARGET), $(@:db))}) $j )}") $(avoid_cvs_dir) >> $(PWD)$/$(PRJ)$/$(ROUT)$/misc$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ELSE			# "$(ZIP9DIR)" != ""
    +-$(ZIPDEP) $(ZIP9FLAGS) $@ $(foreach,j,$(ZIP9LIST) "{$(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP9TARGET), $(@:db))}) $j )}") $(avoid_cvs_dir) >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ENDIF			# "$(ZIP9DIR)" != ""
.ENDIF			# "$(common_build_zip)"!=""
    @+echo $@ : makefile.mk >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ENDIF			# "$(make_zip_deps)" == ""
.ENDIF

# Anweisungen fuer das Linken
# unroll begin

.IF "$(GUI)"=="WNT"
command_seperator=&&
#command_seperator=^
avoid_cvs_dir=-x "*CVS*"
.ELSE
command_seperator=;
avoid_cvs_dir=-x "*CVS*"
.ENDIF

.IF "$(ZIP10TARGET)"!=""
ZIP10DIR*=$(ZIPDIR)
ZIP10FLAGS*=$(ZIPFLAGS)
.IF "$(GUI)"=="UNX" || "$(USE_SHELL)"!="4nt"
zip10langdirs:=$(shell +find {$(subst,$/$(LANGDIR), $(null,$(ZIP10DIR) . $(ZIP10DIR)))}/ -type d ! -name CVS ! -name "." | sed "s/\.\/\///" | sed "s/\.\///" )
.ELSE			# "$(GUI)"=="UNX"
zip10langdirs:=$(subst,CVS, $(shell +-dir {$(subst,$/$(LANGDIR), $(ZIP10DIR))} /ba:d ))
.ENDIF			# "$(GUI)"=="UNX"
zip10alllangext:=$(foreach,i,$(alllangext) $(foreach,j,$(zip10langdirs) $(eq,$(longlang_$i),$j  $i $(NULL))))
.ENDIF			# "$(ZIP10TARGET)"!=""

.IF "$(ZIP10TARGETN)"!=""

ZIP10TMP:=$(mktmp iii)

$(MISC)$/$(TARGET).$(PWD:f).$(ZIP10TARGET).dpzz : $(ZIP10TARGETN)

.IF "$(common_build_zip)"!=""
ZIP10HELPVAR={$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP10TARGET)}
.ELSE			# "$(common_build_zip)"!=""
ZIP10HELPVAR=$(BIN)$/$(ZIP10TARGET)
.ENDIF			# "$(common_build_zip)"!=""

.IF "$(make_zip_deps)" == ""
$(ZIP10TARGETN) : delzip
.ELSE			# "$(make_zip_deps)" == ""
$(ZIP10TARGETN) :
.ENDIF			# "$(make_zip_deps)" == ""
.IF "$(make_zip_deps)" == ""
    @+echo ------------------------------
    @+echo Making: $@
    @$(!eq,$?,$(?:s/delzip/zzz/) +-$(RM) +echo) $(uniq $@ $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)) >&$(NULLDEV)
    @+echo rebuilding zipfiles $(eq,$?,$(?:s/delzip/zzz/) >&$(NULLDEV) )
    @+echo ------------------------------ $(eq,$?,$(?:s/delzip/zzz/) >&$(NULLDEV) )
.IF "$(common_build_zip)"!=""
.IF "$(ZIP10DIR)" != ""
    @+-$(GNUCOPY) -p $@ $(ZIP10TMP).{$(subst,$(ZIP10HELPVAR),_ $(@:db))}.zip >& $(NULLDEV)
    +-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP10HELPVAR), $(@:db))}) $(ZIP10DIR)) $(command_seperator) zip $(ZIP10FLAGS) $(ZIP10TMP).{$(subst,$(ZIP10HELPVAR),_ $(@:db))}.zip $(ZIP10LIST:s/LANGDIR/./) -x delzip $(avoid_cvs_dir) 
    +$(COPY) $(ZIP10TMP).{$(subst,$(ZIP10HELPVAR),_ $(@:db))}.zip $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)    
    +$(RM) $(ZIP10TMP).{$(subst,$(ZIP10HELPVAR),_ $(@:db))}.zip
.ELSE			# "$(ZIP10DIR)" != ""
    @+-$(GNUCOPY) -p $@ $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) >& $(NULLDEV)
    +-zip $(ZIP10FLAGS) $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $(foreach,j,$(ZIP10LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP10HELPVAR), $(@:db))}) $j )) -x delzip $(avoid_cvs_dir)
.ENDIF			# "$(ZIP10DIR)" != ""
    @+-$(GNUCOPY) -p $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $@.$(INPATH) >& $(NULLDEV)
    @+-$(RM) $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)
    @+-$(RM) $@ >& $(NULLDEV)
.IF "$(GUI)"=="UNX"
    @+if ( -r $@.$(INPATH) ) $(RENAME) $@.$(INPATH) $@
#	@+if ( -r $@ ) $(TOUCH) $@
.ELSE			# "$(GUI)"=="UNX"
    @+if exist $@.$(INPATH) $(RENAME) $@.$(INPATH) $@
#	@+if exist $@ $(TOUCH) $@
.ENDIF			# "$(GUI)"=="UNX"
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIP10DIR)" != ""
    @+-$(GNUCOPY) -p $@ $(ZIP10TMP).{$(subst,$(ZIP10HELPVAR),_ $(@:db))}.zip >& $(NULLDEV)
    +-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP10HELPVAR), $(@:db))}) $(ZIP10DIR)) $(command_seperator) zip $(ZIP10FLAGS) $(ZIP10TMP).{$(subst,$(ZIP10HELPVAR),_ $(@:db))}.zip $(ZIP10LIST:s/LANGDIR/./) -x delzip  $(avoid_cvs_dir)
    +$(COPY) $(ZIP10TMP).{$(subst,$(ZIP10HELPVAR),_ $(@:db))}.zip  $@ 
    +$(RM) $(ZIP10TMP).{$(subst,$(ZIP10HELPVAR),_ $(@:db))}.zip
.ELSE			# "$(ZIP10DIR)" != ""
    +-zip $(ZIP10FLAGS) $@ $(foreach,j,$(ZIP10LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP10TARGET), $(@:db))}) $j )) -x delzip $(avoid_cvs_dir)
.ENDIF			# "$(ZIP10DIR)" != ""
.ENDIF			# "$(common_build_zip)"!=""
.ELSE			# "$(make_zip_deps)" == ""
    +echo # > $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.IF "$(common_build_zip)"!=""
.IF "$(ZIP10DIR)" != ""
    +-cd $(subst,LANGDIR,. $(subst,$/LANGDIR, $(ZIP10DIR))) $(command_seperator) $(ZIPDEP) $(ZIP10FLAGS) -prefix $(subst,LANGDIR,. $(subst,$/LANGDIR, $(ZIP10DIR)))$/ $@ $(foreach,j,$(ZIP10LIST) "{$(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP10TARGET), $(@:db))}) $j )}") $(avoid_cvs_dir) >> $(PWD)$/$(PRJ)$/$(ROUT)$/misc$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ELSE			# "$(ZIP10DIR)" != ""
    +-$(ZIPDEP) $(ZIP10FLAGS) $@ $(foreach,j,$(ZIP10LIST) "{$(subst,LANGDIR,$(longlang_{$(subst,$(ZIP10HELPVAR), $(@:db))}) $j )}") $(avoid_cvs_dir) >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ENDIF			# "$(ZIP10DIR)" != ""
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIP10DIR)" != ""
    +-cd $(subst,LANGDIR,. $(subst,$/LANGDIR, $(ZIP10DIR))) $(command_seperator) $(ZIPDEP) $(ZIP10FLAGS) -prefix $(subst,LANGDIR,. $(subst,$/LANGDIR, $(ZIP10DIR)))$/ $@ $(foreach,j,$(ZIP10LIST) "{$(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP10TARGET), $(@:db))}) $j )}") $(avoid_cvs_dir) >> $(PWD)$/$(PRJ)$/$(ROUT)$/misc$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ELSE			# "$(ZIP10DIR)" != ""
    +-$(ZIPDEP) $(ZIP10FLAGS) $@ $(foreach,j,$(ZIP10LIST) "{$(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP10TARGET), $(@:db))}) $j )}") $(avoid_cvs_dir) >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ENDIF			# "$(ZIP10DIR)" != ""
.ENDIF			# "$(common_build_zip)"!=""
    @+echo $@ : makefile.mk >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ENDIF			# "$(make_zip_deps)" == ""
.ENDIF

# Anweisungen fuer das Linken
