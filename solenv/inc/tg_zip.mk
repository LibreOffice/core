
#*************************************************************************
#*
#*    $Workfile:   tg_zip.mk  $
#*
#*    Ersterstellung    XX  TT.MM.JJ
#*    Letzte Aenderung  $Author: hjs $ $Date: 2001-06-12 15:07:20 $
#*    $Revision: 1.5 $
#*
#*    $Logfile:   T:/solar/inc/tg_zip.mkv  $
#*
#*    Copyright (c) 1990-1997, STAR DIVISION
#*
#*************************************************************************

.IF "$(MULTI_ZIP_FLAG)" == ""
$(ZIP1TARGETN) .NULL : ZIP1

$(ZIP2TARGETN) .NULL : ZIP2

$(ZIP3TARGETN) .NULL : ZIP3

$(ZIP4TARGETN) .NULL : ZIP4

$(ZIP5TARGETN) .NULL : ZIP5

$(ZIP6TARGETN) .NULL : ZIP6

$(ZIP7TARGETN) .NULL : ZIP7

$(ZIP8TARGETN) .NULL : ZIP8

$(ZIP9TARGETN) .NULL : ZIP9
.ENDIF

.IF "$(MULTI_ZIP_FLAG)"==""
ZIP1 ZIP2 ZIP3 ZIP4 ZIP5 ZIP6 ZIP7 ZIP8 ZIP9:
    +@dmake $(ZIP$(TNR)TARGETN) MULTI_ZIP_FLAG=true TNR:=$(TNR) $(MFLAGS) $(CALLMACROS)
.ELSE


#######################################################
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

.IF "$(ZIP$(TNR)TARGETN)"!=""

$(MISC)$/$(TARGET).$(PWD:f).$(ZIP$(TNR)TARGET).dpzz : $(ZIP$(TNR)TARGETN)

.IF "$(common_build_zip)"!=""
ZIP$(TNR)HELPVAR={$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP$(TNR)TARGET)}
.ELSE			# "$(common_build_zip)"!=""
ZIP$(TNR)HELPVAR=$(BIN)$/$(ZIP$(TNR)TARGET)
.ENDIF			# "$(common_build_zip)"!=""

.IF "$(make_zip_deps)" == ""
$(ZIP$(TNR)TARGETN) : delzip
.ELSE			# "$(make_zip_deps)" == ""
$(ZIP$(TNR)TARGETN) :
.ENDIF			# "$(make_zip_deps)" == ""
.IF "$(make_zip_deps)" == ""
    @+echo ------------------------------
    @+echo Making: $@
.IF "$(common_build_zip)"!=""
.IF "$(make_zip_deps)" == ""
.IF "$?"!="$(subst,delzip,zzz $?)"
    @+-$(RM) $@  $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) >& $(NULLDEV)
    @+echo rebuilding zipfiles
    @+echo ------------------------------
.ENDIF		# "$?"!="$(subst,delzip,zzz $?)"
.ENDIF			# "$(make_zip_deps)" == ""
    @+-$(GNUCOPY) -p $@ $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) >& $(NULLDEV)
.IF "$(ZIPDIR)" != ""
#	+-echo $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP$(TNR)HELPVAR), $(@:db))}) $(ZIPDIR))
    +-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP$(TNR)HELPVAR), $(@:db))}) $(ZIPDIR)) $(command_seperator) zip $(ZIPFLAGS) ..$/{$(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)} $(ZIP$(TNR)LIST:s/LANGDIR/./) -x delzip $(avoid_cvs_dir) 
.ELSE			# "$(ZIPDIR)" != ""
    +-zip $(ZIPFLAGS) $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $(foreach,j,$(ZIP$(TNR)LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP$(TNR)HELPVAR), $(@:db))}) $j )) -x delzip $(avoid_cvs_dir)
.ENDIF			# "$(ZIPDIR)" != ""
    @+-$(GNUCOPY) -p $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $@.new >& $(NULLDEV)
    @+-$(RM) $@ >& $(NULLDEV)
.IF "$(GUI)"=="UNX"
    @+if ( -r $@.new ) $(RENAME) $@.new $@
#	@+if ( -r $@ ) $(TOUCH) $@
.ELSE			# "$(GUI)"=="UNX"
    @+if exist $@.new $(RENAME) $@.new $@
#	@+if exist $@ $(TOUCH) $@
.ENDIF			# "$(GUI)"=="UNX"
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIPDIR)" != ""
    +-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP$(TNR)HELPVAR), $(@:db))}) $(ZIPDIR)) $(command_seperator) zip $(ZIPFLAGS) ..$/{$@} $(ZIP$(TNR)LIST:s/LANGDIR/./) -x delzip  $(avoid_cvs_dir)
.ELSE			# "$(ZIPDIR)" != ""
    +-zip $(ZIPFLAGS) $@ $(foreach,j,$(ZIP$(TNR)LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP$(TNR)TARGET), $(@:db))}) $j )) -x delzip $(avoid_cvs_dir)
.ENDIF			# "$(ZIPDIR)" != ""
#	+$(TOUCH) $@
.ENDIF			# "$(common_build_zip)"!=""
.ELSE			# "$(make_zip_deps)" != ""
    @+echo $@ : makefile.mk >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.IF "$(common_build_zip)"!=""
.IF "$(ZIPDIR)" != ""
    @+-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP$(TNR)HELPVAR), $(@:db))}) $(ZIPDIR)) $(command_seperator) zipdep $(ZIPFLAGS) ..$/{$@} $(ZIP$(TNR)LIST:s/LANGDIR/./) >> ..$/$(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ELSE			# "$(ZIPDIR)" != ""
    @+-zipdep $(ZIPFLAGS) $@ $(foreach,j,$(ZIP$(TNR)LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP$(TNR)HELPVAR), $(@:db))}) $j )) >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ENDIF			# "$(ZIPDIR)" != ""
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIPDIR)" != ""
    +-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP$(TNR)HELPVAR), $(@:db))}) $(ZIPDIR)) $(command_seperator) zipdep $(ZIPFLAGS) ..$/{$@} $(ZIP$(TNR)LIST:s/LANGDIR/./) >> ..$/$(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ELSE			# "$(ZIPDIR)" != ""
    +-zipdep $(ZIPFLAGS) $@ $(foreach,j,$(ZIP$(TNR)LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP$(TNR)TARGET), $(@:db))}) $j )) >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ENDIF			# "$(ZIPDIR)" != ""
.ENDIF			# "$(common_build_zip)"!=""
.ENDIF			# "$(make_zip_deps)" != ""
.ENDIF

# Anweisungen fuer das Linken
# unroll end
#######################################################

.ENDIF		# "$(MULTI_ZIP_FLAG)"==""
