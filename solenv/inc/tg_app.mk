#*************************************************************************
#*
#*    $Workfile:   tg_app.mk  $
#*
#*    Ersterstellung    XX  TT.MM.JJ
#*    Letzte Aenderung  $Author: hr $ $Date: 2000-09-20 14:43:17 $
#*    $Revision: 1.1.1.1 $
#*
#*    $Logfile:   T:/solar/inc/tg_app.mkv  $
#*
#*    Copyright (c) 1990-1997, STAR DIVISION
#*
#*************************************************************************

.IF "$(MULTI_APP_FLAG)" == ""
$(APP1TARGETN) .NULL : APP1

$(APP2TARGETN) .NULL : APP2

$(APP3TARGETN) .NULL : APP3

$(APP4TARGETN) .NULL : APP4

$(APP5TARGETN) .NULL : APP5

$(APP6TARGETN) .NULL : APP6

$(APP7TARGETN) .NULL : APP7

$(APP8TARGETN) .NULL : APP8

$(APP9TARGETN) .NULL : APP9

.ENDIF

.IF "$(MULTI_APP_FLAG)"==""
APP1 APP2 APP3 APP4 APP5 APP6 APP7 APP8 APP9 :
.IF "$(GUI)" == "UNX"
    @echo $(SHELL) AUSE $(COMSPEC) MARTIN $(SHELLFLAGS)
.ENDIF
    +@dmake $(APP$(TNR)TARGETN) MULTI_APP_FLAG=true TNR:=$(TNR) $(MFLAGS) $(CALLMACROS)
.ELSE


#######################################################
# Anweisungen fuer das Linken
# unroll begin

.IF "$(APP$(TNR)LINKTYPE)" != ""
#darf nur STATIC oder SHARED sein
APP_LINKTYPE=$(APPLINK$(APP$(TNR)LINKTYPE))
.ENDIF

.IF "$(APP$(TNR)STACK)" != ""
.IF "$(LINKFLAGSTACK)" != ""
APP$(TNR)STACKN=$(LINKFLAGSTACK)$(APP$(TNR)STACK)
.ENDIF
.ELSE
APP$(TNR)STACKN=
.ENDIF
.IF "$(VCL)" != ""
.IF "$(APP$(TNR)NOSAL)"==""
.IF "$(TARGETTYPE)" == "GUI"
APP$(TNR)OBJS+= $(STDOBJVCL)
.ENDIF
.ENDIF
.ENDIF

.IF "$(GUI)" != "WNT" || "$(GUI)$(COM)"=="WNTBLC"
USE_APP$(TNR)DEF=$(APP$(TNR)DEF)
.ELSE
USE_APP$(TNR)DEF=
.ENDIF

.IF "$(GUI)" == "UNX"
APP$(TNR)DEPN=
USE_APP$(TNR)DEF=
.ENDIF

.IF "$(APP$(TNR)TARGETN)"!=""
$(APP$(TNR)TARGETN): $(APP$(TNR)OBJS) $(APP$(TNR)LIBS) \
    $(APP$(TNR)RES) \
    $(APP$(TNR)ICON) $(APP$(TNR)DEPN) $(USE_APP$(TNR)DEF)
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="UNX"
    @+echo unx
    @+-$(RM) $(MISC)$/$(@:b).cmd
    @+echo $(LINK) $(LINKFLAGS) $(LINKFLAGSAPP) -L$(PRJ)$/$(INPATH)$/lib $(SOLARLIB) $(STDSLO) \
    -o $@ $(APP$(TNR)OBJS:s/.obj/.o/)\
    `cat /dev/null $(APP$(TNR)LIBS) | sed s#$(ROUT)#$(OUT)#g` \
    $(APP_LINKTYPE) $(APP$(TNR)STDLIBS) $(STDLIB) > $(MISC)$/$(@:b).cmd
    @cat $(MISC)$/$(@:b).cmd
    @source $(MISC)$/$(@:b).cmd
    @ls -l $@
.ENDIF
.IF "$(GUI)"=="MAC"
    @+-$(RM) $@ $@.xSYM $@.idb
    $(LINK) $(LINKFLAGS) $(LINKFLAGSAPP) $(foreach,i,$(shell $(UNIX2MACPATH) $(PRJ)$/$(ROUT)$/lib $(SOLARLIB:s/-L//)) -L"$i") $(shell $(UNIX2MACPATH) $(STDSLO) $(APP$(TNR)OBJS) `cat /dev/null $(APP$(TNR)LIBS) | sed s#$(ROUT)#$(PRJ)$/$(ROUT)#g` $(VERSIONOBJ)) $(APP$(TNR)STDLIBS) $(APP$(TNR)ARCHIVES) $(STDSHL) $(LINKOUTPUT_FILTER) -o $(shell $(UNIX2MACPATH) $@)
.ENDIF                  # "$(GUI)"=="MAC"

.IF "$(GUI)" == "WNT"
    @+-$(MKDIR) $(@:d:d) >& $(NULLDEV)
    @+if not exist $(FUNCORD) $(TOUCH) $(FUNCORD)
.IF "$(APP$(TNR)LINKRES)" != ""
    @+-$(RM) $(MISC)$/$(APP$(TNR)LINKRES:b).rc >& $(NULLDEV)
.IF "$(APP$(TNR)ICON)" != ""
    @-+echo 1 ICON $(APP$(TNR)ICON) >> $(MISC)$/$(APP$(TNR)LINKRES:b).rc
.ENDIF
.IF "$(APP$(TNR)VERINFO)" != ""
    @-+echo #include "$(APP$(TNR)VERINFO)" >> $(MISC)$/$(APP$(TNR)LINKRES:b).rc
.ENDIF
    $(RC) -DWIN32 $(INCLUDE) $(RCLINKFLAGS) $(MISC)$/$(APP$(TNR)LINKRES:b).rc
.ENDIF			# "$(APP$(TNR)LINKRES)" != ""
.IF "$(linkinc)" == ""
    $(LINK) @$(mktmp \
        $(LINKFLAGS) \
        $(LINKFLAGSAPP) $(APP$(TNR)BASEX) \
        $(APP$(TNR)STACKN) \
        -out:$@ \
        -map:$(MISC)$/$(@:b).map \
        $(STDOBJ) \
        $(APP$(TNR)LINKRES) \
        $(subst,$/res$/,$/res{$(subst,$(BIN), $(@:d))} $(APP$(TNR)RES)) \
        $(APP$(TNR)OBJS) \
        $(APP$(TNR)LIBS) \
        $(APP$(TNR)STDLIBS) \
        $(STDLIB) \
        )
.ELSE
        +-$(RM) $(MISC)\$(APP$(TNR)TARGET).lnk
        +-$(RM) $(MISC)\$(APP$(TNR)TARGET).lst
        +-$(RM) $(MISC)\linkobj.lst
        +for %_i in ($(MISC)\*.obj) do type %_i >> $(MISC)\linkobj.lst
    type $(mktmp,$(MISC)\$(APP$(TNR)TARGET).lst
        $(LINKFLAGS) \
        $(LINKFLAGSAPP) $(APP$(TNR)BASEX) \
        $(APP$(TNR)STACKN) \
        -out:$@ \
        $(STDOBJ) \
        $(APP$(TNR)LINKRES) \
        $(subst,$/res$/,$/res{$(subst,$(BIN), $(@:d))} $(APP$(TNR)RES)) \
        $(APP$(TNR)OBJS) \
        $(APP$(TNR)LIBS) \
        $(APP$(TNR)STDLIBS) \
        $(STDLIB))
        sed -e 's/\(\.\.\\\)\{2,4\}/..\\/g' $(MISC)\linkobj.lst >> $(MISC)\$(APP$(TNR)TARGET).lst
        +if exist $(MISC)\$(APP$(TNR)TARGET).lst type $(MISC)\$(APP$(TNR)TARGET).lst  >> $(MISC)\$(APP$(TNR)TARGET).lnk
        $(LINK) @$(MISC)\$(APP$(TNR)TARGET).lnk
.ENDIF		# "$(linkinc)" == ""

.IF "$(APP$(TNR)TARGET)" == "loader"
    +perl5 loader.pl $@
    +$(COPY) /b $(@)+$(@:d)unloader.exe $(@:d)_new.exe
    +$(RM) $@
    +$(RENAME) $(@:d)_new.exe $(@:d)loader.exe
.ENDIF			# "$(TARGET)" == "setup"

.ENDIF			# "$(GUI)" == "WNT"

.IF "$(GUI)"=="WIN"
.IF "$(COM)"=="BLC"
    $(LINK) @$(mktmp$ $(LINKFLAGS) $(LINKFLAGSAPP) $(APP$(TNR)STACKN) $(STDOBJ) $(APP$(TNR)OBJS), $@, $(MISC)\$(APP$(TNR)TARGET).map, $(APP$(TNR)LIBS) $(APP$(TNR)STDLIBS) $(STDLIB), $(APP$(TNR)DEF)) >&  $(TMP)$/$(PRJNAME)$(USER).tmp
    @+$(TYPE) $(TMP)$/$(PRJNAME)$(USER).tmp
    @+$(RM) $(TMP)$/$(PRJNAME)$(USER).tmp
.ELSE
    @+echo ------------------------
    @+echo No valid Environment!!!
    @+echo ------------------------
    force_dmake_to_error
.ENDIF			# "$(COM)"=="BLC"
.IF "$(TARGETTYPE)" == "GUI"
.IF "$(APP$(TNR)RES)" != ""
    $(RCLINK) $(RCLINKFLAGS) $(subst,$/res$/,$/res{$(subst,$(BIN), $(@:d))} $(APP$(TNR)RES)) $@
.ELSE
    $(RCSETVERSION)
.ENDIF
.IF "$(MAPSYM)" != ""
    mapfix $(MISC)\$(@B).map
    $(MAPSYM) $(MAPSYMFLAGS) $(MISC)\$(APP$(TNR)TARGET).map
    @copy $(APP$(TNR)TARGET).sym $(BIN)\$(APP$(TNR)TARGET).sym
    @del $(APP$(TNR)TARGET).sym
.ENDIF			# "$(MAPSYM)" != ""
.ENDIF			# "$(TARGETTYPE)" == "GUI"
.ENDIF			# "$(GUI)" == "WIN"

.ENDIF			# "$(APP$(TNR)TARGETN)"!=""


# Anweisungen fuer das Linken
# unroll end
#######################################################

.ENDIF		# "$(MULTI_APP_FLAG)"==""
