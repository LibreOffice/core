# *************************************************************
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
# *************************************************************

CC_PATH*=""

# unroll begin

.IF "$(GUI)" == "OS2" && "$(TARGETTYPE)" == "GUI" 
APP1DEF = $(MISC)/$(APP1TARGET).def
.ENDIF

.IF "$(APP1LINKTYPE)" != ""
#must be either STATIC or SHARED
APP1LINKTYPEFLAG=$(APPLINK$(APP1LINKTYPE))
.ENDIF

# decide how to link
.IF "$(APP1CODETYPE)"=="C"
APP1LINKER=$(LINKC)
APP1STDLIB=$(subst,CPPRUNTIME, $(STDLIB))
APP1LINKFLAGS+=$(LINKCFLAGS)
.ELSE			# "$(APP1CODETYPE)"=="C"
APP1LINKER=$(LINK)
APP1STDLIB=$(subst,CPPRUNTIME,$(STDLIBCPP) $(STDLIB))
APP1LINKFLAGS+=$(LINKFLAGS)
.ENDIF			# "$(APP1CODETYPE)"=="C"

APP1RPATH*=OOO
LINKFLAGSRUNPATH_$(APP1RPATH)*=/ERROR:/Bad_APP1RPATH_value
.IF "$(OS)" != "MACOSX"
APP1LINKFLAGS+=$(LINKFLAGSRUNPATH_$(APP1RPATH))
.ENDIF

.IF "$(APP1STACK)" != ""
.IF "$(LINKFLAGSTACK)" != ""
APP1STACKN=$(LINKFLAGSTACK)$(APP1STACK)
.ENDIF
.ELSE
APP1STACKN=
.ENDIF

.IF "$(APP1NOSAL)"==""
.IF "$(TARGETTYPE)" == "GUI"
APP1OBJS+= $(STDOBJVCL)
.ENDIF
.ENDIF

.IF "$(GUI)$(COM)" == "WNTGCC"
APP1RESO=
.IF "$(APP1LINKRES)" != "" || "$(APP1RES)" != ""
APP1RESO=$(MISC)/$(APP1TARGET:b)_res.o
.ENDIF
.ENDIF

.IF "$(GUI)" == "UNX"
APP1DEPN+:=$(APP1DEPNU)
USE_APP1DEF=
.ENDIF

.IF "$(APP1TARGETN)"!=""

.IF "$(APP1PRODUCTNAME)"!=""
APP1PRODUCTDEF+:=-DPRODUCT_NAME=\"$(APP1PRODUCTNAME)\"
.ENDIF			# "$(APP1PRODUCTNAME)"!=""

.IF "$(linkinc)"!=""
.IF "$(GUI)"=="WNT" || "$(GUI)"=="OS2"
.IF "$(APP1LIBS)"!=""
$(MISC)/$(APP1TARGET)_linkinc.ls .PHONY:
    @@-$(RM) $@
    sed -f $(SOLARENV)/bin/chrel.sed $(foreach,i,$(APP1LIBS) $(i:s/.lib/.lin/)) >> $@
.ENDIF          #"$(APP1LIBS)"!="" 
.ENDIF

LINKINCTARGETS+=$(MISC)/$(APP1TARGETN:b)_linkinc.ls
$(APP1TARGETN) : $(LINKINCTARGETS)
.ENDIF          # "$(linkinc)"!=""

# Allow for target specific LIBSALCPPRT override
APP1LIBSALCPPRT*=$(LIBSALCPPRT)

$(APP1TARGETN): $(APP1OBJS) $(APP1LIBS) \
    $(APP1RES) \
    $(APP1ICON) $(APP1DEPN) $(USE_APP1DEF)
    @echo "Making:   " $(@:f)
.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="MACOSX"
    @echo unx
    @-$(RM) $(MISC)/$(@:b).list
    @-$(RM) $(MISC)/$(TARGET).$(@:b)_1.cmd
    @-$(RM) $(MISC)/$(@:b).strip
    @echo $(STDSLO) $(APP1OBJS:s/.obj/.o/) \
    `cat /dev/null $(APP1LIBS) | sed s\#$(ROUT)\#$(OUT)\#g` | tr -s " " "\n" > $(MISC)/$(@:b).list
    @echo -n $(APP1LINKER) $(APP1LINKFLAGS) $(LINKFLAGSAPP) -L$(PRJ)/$(INPATH)/lib $(SOLARLIB) -o $@ \
    $(APP1LINKTYPEFLAG) $(APP1STDLIBS) $(APP1STDLIB) $(STDLIB1) -filelist $(MISC)/$(@:b).list > $(MISC)/$(TARGET).$(@:b)_1.cmd
    @$(PERL) $(SOLARENV)/bin/macosx-dylib-link-list.pl \
        `cat $(MISC)/$(TARGET).$(@:b)_1.cmd` \
        >> $(MISC)/$(TARGET).$(@:b)_1.cmd
  .IF "$(VERBOSE)" == "TRUE"
    @cat $(MISC)/$(TARGET).$(@:b)_1.cmd
  .ENDIF
    @+source $(MISC)/$(TARGET).$(@:b)_1.cmd
# Need to strip __objcInit symbol to avoid duplicate symbols when loading
# libraries at runtime
    @-$(CC_PATH)nm $@ | grep -v ' U ' | $(AWK) '{ print $$NF }' | grep -F -x '__objcInit' > $(MISC)/$(@:b).strip
    @$(CC_PATH)strip -i -R $(MISC)/$(@:b).strip -X $@
    @ls -l $@
    @$(PERL) $(SOLARENV)/bin/macosx-change-install-names.pl \
        app $(APP1RPATH) $@
.IF "$(TARGETTYPE)"=="GUI"
    @echo "Making:   " $(@:f).app
    @macosx-create-bundle $@
.ENDIF		# "$(TARGETTYPE)"=="GUI"
.ELSE		# "$(OS)"=="MACOSX"
    @echo unx
    @-$(RM) $(MISC)/$(TARGET).$(@:b)_1.cmd
    @echo $(APP1LINKER) $(APP1LINKFLAGS) $(LINKFLAGSAPP) -L$(PRJ)/$(INPATH)/lib $(SOLARLIB) $(STDSLO) \
    $(APP1OBJS:s/.obj/.o/) '\' >  $(MISC)/$(TARGET).$(@:b)_1.cmd
    @cat $(mktmp /dev/null $(APP1LIBS)) | xargs -n 1 cat | sed s\#$(ROUT)\#$(OUT)\#g | sed 's#$$# \\#'  >> $(MISC)/$(TARGET).$(@:b)_1.cmd
    @echo $(APP1LINKTYPEFLAG) $(APP1LIBSALCPPRT) $(APP1STDLIBS) $(APP1STDLIB) $(STDLIB1) -o $@ >> $(MISC)/$(TARGET).$(@:b)_1.cmd
  .IF "$(VERBOSE)" == "TRUE"
    @cat $(MISC)/$(TARGET).$(@:b)_1.cmd
  .ENDIF
    @+source $(MISC)/$(TARGET).$(@:b)_1.cmd
  .IF "$(VERBOSE)" == "TRUE"
    @ls -l $@
  .ENDIF
.ENDIF		# "$(OS)"=="MACOSX"
.ENDIF
.IF "$(GUI)" == "WNT"
    @@-$(MKDIR) $(@:d:d)
.IF "$(APP1LINKRES)" != ""
    @@-$(RM) $(MISC)/$(APP1LINKRES:b).rc
.IF "$(APP1ICON)" != ""
    @-echo 1 ICON $(EMQ)"$(APP1ICON:s/\/\\/)$(EMQ)" >> $(MISC)/$(APP1LINKRES:b).rc
.ENDIF		# "$(APP1ICON)" != ""
.IF "$(APP1VERINFO)" != ""
    @-echo $(EMQ)#define VERVARIANT	$(BUILD) >> $(MISC)/$(APP1LINKRES:b).rc
    @-echo $(EMQ)#include  $(EMQ)"$(APP1VERINFO)$(EMQ)" >> $(MISC)/$(APP1LINKRES:b).rc
.ENDIF		# "$(APP1VERINFO)" != ""
    $(COMMAND_ECHO)$(RC) -DWIN32 $(APP1PRODUCTDEF) -I$(SOLARRESDIR) $(INCLUDE) $(RCLINKFLAGS) $(MISC)/$(APP1LINKRES:b).rc
.ENDIF			# "$(APP1LINKRES)" != ""
.IF "$(COM)" == "GCC"
    @echo mingw
.IF "$(APP1LINKRES)" != "" || "$(APP1RES)" != ""
    @cat $(APP1LINKRES) $(subst,/res/,/res{$(subst,$(BIN), $(@:d))} $(APP1RES)) >  $(MISC)/$(@:b)_all.res
    windres $(MISC)/$(@:b)_all.res $(APP1RESO)
.ENDIF
    @echo $(LINK) $(LINKFLAGS) $(LINKFLAGSAPP) $(MINGWSSTDOBJ) -L$(PRJ)/$(INPATH)/lib $(SOLARLIB) $(STDSLO) \
        $(APP1BASEX) $(APP1STACKN) -o $@ $(APP1OBJS) \
        -Wl,-Map,$(MISC)/$(@:b).map $(STDOBJ) $(APP1RESO) \
        `$(TYPE) /dev/null $(APP1LIBS) | sed s#$(ROUT)#$(OUT)#g` \
        $(APP_LINKTYPE) $(APP1LIBSALCPPRT) \
        -Wl,--start-group $(APP1STDLIBS) -Wl,--end-group $(APP1STDLIB) \
        $(STDLIB1) $(MINGWSSTDENDOBJ) > $(MISC)/$(TARGET).$(@:b)_1.cmd
  # need this empty line, else dmake somehow gets confused by the .IFs and .ENDIFs
  .IF "$(VERBOSE)" == "TRUE"
    @$(TYPE)  $(MISC)/$(TARGET).$(@:b)_1.cmd
  .ENDIF
    @+source $(MISC)/$(TARGET).$(@:b)_1.cmd
    @ls -l $@
.ELSE	# "$(COM)" == "GCC"
.IF "$(linkinc)" == ""
    $(COMMAND_ECHO)$(APP1LINKER) @$(mktmp \
        $(APP1LINKFLAGS) \
        $(LINKFLAGSAPP) $(APP1BASEX) \
        $(APP1STACKN) \
        -out:$@ \
        -map:$(MISC)/{$(subst,/,_ $(APP1TARGET)).map} \
        $(STDOBJ) \
        $(APP1LINKRES) \
        $(APP1RES) \
        $(APP1OBJS) \
        $(APP1LIBS) \
        $(APP1STDLIBS) \
        $(APP1STDLIB) $(STDLIB1) \
        )
    @-echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.tmanifest -outputresource:$@$(EMQ);1 $(FI)
.ELSE
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -outputresource:$@$(EMQ);1 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ELSE
        $(COMMAND_ECHO)-$(RM) $(MISC)\$(APP1TARGET).lnk
        $(COMMAND_ECHO)-$(RM) $(MISC)\$(APP1TARGET).lst
        $(COMMAND_ECHO)-$(RM) $(MISC)\linkobj.lst
        for %_i in ($(MISC)\*.obj) do type %_i >> $(MISC)\linkobj.lst
    type $(mktmp,$(MISC)\$(APP1TARGET).lst
        $(APP1LINKFLAGS) \
        $(LINKFLAGSAPP) $(APP1BASEX) \
        $(APP1STACKN) \
        -out:$@ \
        $(STDOBJ) \
        $(APP1LINKRES) \
        $(APP1RES) \
        $(APP1OBJS) \
        $(APP1LIBS) \
        $(APP1STDLIBS) \
        $(APP1STDLIB) $(STDLIB1))
        $(COMMAND_ECHO)$(SED)$(SED) -e 's/\(\.\.\\\)\{2,4\}/..\\/g' $(MISC)\$(APP1TARGETN:b)_linkobj.lst >> $(MISC)\$(APP1TARGET).lst
        $(COMMAND_ECHO)$(SED)$(IFEXIST) $(MISC)/$(APP1TARGET).lst $(THEN) type $(MISC)/$(APP1TARGET).lst  >> $(MISC)/$(APP1TARGET).lnk $(FI)
        $(COMMAND_ECHO)$(SED)$(APP1LINKER) @$(MISC)\$(APP1TARGET).lnk
.ENDIF		# "$(linkinc)" == ""
.ENDIF		# "$(COM)" == "GCC"
.IF "$(APP1TARGET)" == "loader"
    $(COMMAND_ECHO)$(PERL) loader.pl $@
    $(COMMAND_ECHO)$(TYPE) $(@) $(@:d)unloader.exe > $(@:d)_new.exe
    $(COMMAND_ECHO)$(RM) $@
    $(COMMAND_ECHO)$(RENAME) $(@:d)_new.exe $(@:d)loader.exe
.ENDIF			# "$(TARGET)" == "setup"

.ENDIF			# "$(GUI)" == "WNT"

.IF "$(GUI)" == "OS2"
    @+-$(MKDIR) $(@:d:d) > $(NULLDEV)
.IF "$(APP1LINKRES)" != ""
    @+-$(RM) $(MISC)/$(APP1LINKRES:b).rc > $(NULLDEV)
.IF "$(APP1ICON)" != ""
    @-+echo ICON 1 $(EMQ)"$(APP1ICON:s#/#\\\\#)$(EMQ)" >> $(MISC)$/$(APP1LINKRES:b).rc
.ENDIF		# "$(APP1ICON)" != ""
.IF "$(APP1VERINFO)" != ""
    @-+echo $(EMQ)#define VERVARIANT	$(BUILD) >> $(MISC)/$(APP1LINKRES:b).rc
    @-+echo $(EMQ)#include  $(EMQ)"$(APP1VERINFO)$(EMQ)" >> $(MISC)/$(APP1LINKRES:b).rc
.ENDIF		# "$(APP1VERINFO)" != ""
    $(COMMAND_ECHO)$(RC) -r -DOS2 $(APP1PRODUCTDEF) -I$(SOLARRESDIR) $(INCLUDE) $(RCLINKFLAGS) $(MISC)/$(APP1LINKRES:b).rc
.ENDIF			# "$(APP1LINKRES)" != ""

.IF "$(TARGETTYPE)" == "GUI" 
    @echo NAME $(APP1TARGET) WINDOWAPI > $(MISC)/$(APP1TARGET).def
.ENDIF

    $(COMMAND_ECHO)$(APP1LINKER) -v \
        $(APP1LINKFLAGS) \
        $(LINKFLAGSAPP) $(APP1BASEX) \
        $(APP1STACKN) \
        -o $@ \
        -Zmap -L$(LB) \
        -L$(SOLARVERSION)/$(INPATH)/lib \
        $(STDOBJ) \
        $(APP1LINKRES) \
        $(APP1RES) \
        $(APP1DEF) \
        $(APP1OBJS) \
        $(APP1LIBS) \
        $(APP1STDLIBS) \
        $(APP1STDLIB) $(STDLIB1) 


.IF "$(APP1TARGET)" == "loader"
    $(COMMAND_ECHO)+$(PERL) loader.pl $@
    $(COMMAND_ECHO)+$(TYPE) $(@) $(@:d)unloader.exe > $(@:d)_new.exe
    $(COMMAND_ECHO)+$(RM) $@
    $(COMMAND_ECHO)+$(RENAME) $(@:d)_new.exe $(@:d)loader.exe
.ENDIF			# "$(TARGET)" == "setup"

.ENDIF			# "$(GUI)" == "OS2"

.ENDIF			# "$(APP1TARGETN)"!=""


# Instruction for linking
# unroll begin

.IF "$(GUI)" == "OS2" && "$(TARGETTYPE)" == "GUI" 
APP2DEF = $(MISC)/$(APP2TARGET).def
.ENDIF

.IF "$(APP2LINKTYPE)" != ""
#must be either STATIC or SHARED
APP2LINKTYPEFLAG=$(APPLINK$(APP2LINKTYPE))
.ENDIF

# decide how to link
.IF "$(APP2CODETYPE)"=="C"
APP2LINKER=$(LINKC)
APP2STDLIB=$(subst,CPPRUNTIME, $(STDLIB))
APP2LINKFLAGS+=$(LINKCFLAGS)
.ELSE			# "$(APP2CODETYPE)"=="C"
APP2LINKER=$(LINK)
APP2STDLIB=$(subst,CPPRUNTIME,$(STDLIBCPP) $(STDLIB))
APP2LINKFLAGS+=$(LINKFLAGS)
.ENDIF			# "$(APP2CODETYPE)"=="C"

APP2RPATH*=OOO
LINKFLAGSRUNPATH_$(APP2RPATH)*=/ERROR:/Bad_APP2RPATH_value
.IF "$(OS)" != "MACOSX"
APP2LINKFLAGS+=$(LINKFLAGSRUNPATH_$(APP2RPATH))
.ENDIF

.IF "$(APP2STACK)" != ""
.IF "$(LINKFLAGSTACK)" != ""
APP2STACKN=$(LINKFLAGSTACK)$(APP2STACK)
.ENDIF
.ELSE
APP2STACKN=
.ENDIF

.IF "$(APP2NOSAL)"==""
.IF "$(TARGETTYPE)" == "GUI"
APP2OBJS+= $(STDOBJVCL)
.ENDIF
.ENDIF

.IF "$(GUI)$(COM)" == "WNTGCC"
APP2RESO=
.IF "$(APP2LINKRES)" != "" || "$(APP2RES)" != ""
APP2RESO=$(MISC)/$(APP2TARGET:b)_res.o
.ENDIF
.ENDIF

.IF "$(GUI)" == "UNX"
APP2DEPN+:=$(APP2DEPNU)
USE_APP2DEF=
.ENDIF

.IF "$(APP2TARGETN)"!=""

.IF "$(APP2PRODUCTNAME)"!=""
APP2PRODUCTDEF+:=-DPRODUCT_NAME=\"$(APP2PRODUCTNAME)\"
.ENDIF			# "$(APP2PRODUCTNAME)"!=""

.IF "$(linkinc)"!=""
.IF "$(GUI)"=="WNT" || "$(GUI)"=="OS2"
.IF "$(APP2LIBS)"!=""
$(MISC)/$(APP2TARGET)_linkinc.ls .PHONY:
    @@-$(RM) $@
    sed -f $(SOLARENV)/bin/chrel.sed $(foreach,i,$(APP2LIBS) $(i:s/.lib/.lin/)) >> $@
.ENDIF          #"$(APP2LIBS)"!="" 
.ENDIF

LINKINCTARGETS+=$(MISC)/$(APP2TARGETN:b)_linkinc.ls
$(APP2TARGETN) : $(LINKINCTARGETS)
.ENDIF          # "$(linkinc)"!=""

# Allow for target specific LIBSALCPPRT override
APP2LIBSALCPPRT*=$(LIBSALCPPRT)

$(APP2TARGETN): $(APP2OBJS) $(APP2LIBS) \
    $(APP2RES) \
    $(APP2ICON) $(APP2DEPN) $(USE_APP2DEF)
    @echo "Making:   " $(@:f)
.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="MACOSX"
    @echo unx
    @-$(RM) $(MISC)/$(@:b).list
    @-$(RM) $(MISC)/$(TARGET).$(@:b)_2.cmd
    @-$(RM) $(MISC)/$(@:b).strip
    @echo $(STDSLO) $(APP2OBJS:s/.obj/.o/) \
    `cat /dev/null $(APP2LIBS) | sed s\#$(ROUT)\#$(OUT)\#g` | tr -s " " "\n" > $(MISC)/$(@:b).list
    @echo -n $(APP2LINKER) $(APP2LINKFLAGS) $(LINKFLAGSAPP) -L$(PRJ)/$(INPATH)/lib $(SOLARLIB) -o $@ \
    $(APP2LINKTYPEFLAG) $(APP2STDLIBS) $(APP2STDLIB) $(STDLIB2) -filelist $(MISC)/$(@:b).list > $(MISC)/$(TARGET).$(@:b)_2.cmd
    @$(PERL) $(SOLARENV)/bin/macosx-dylib-link-list.pl \
        `cat $(MISC)/$(TARGET).$(@:b)_2.cmd` \
        >> $(MISC)/$(TARGET).$(@:b)_2.cmd
  .IF "$(VERBOSE)" == "TRUE"
    @cat $(MISC)/$(TARGET).$(@:b)_2.cmd
  .ENDIF
    @+source $(MISC)/$(TARGET).$(@:b)_2.cmd
# Need to strip __objcInit symbol to avoid duplicate symbols when loading
# libraries at runtime
    @-$(CC_PATH)nm $@ | grep -v ' U ' | $(AWK) '{ print $$NF }' | grep -F -x '__objcInit' > $(MISC)/$(@:b).strip
    @$(CC_PATH)strip -i -R $(MISC)/$(@:b).strip -X $@
    @ls -l $@
    @$(PERL) $(SOLARENV)/bin/macosx-change-install-names.pl \
        app $(APP2RPATH) $@
.IF "$(TARGETTYPE)"=="GUI"
    @echo "Making:   " $(@:f).app
    @macosx-create-bundle $@
.ENDIF		# "$(TARGETTYPE)"=="GUI"
.ELSE		# "$(OS)"=="MACOSX"
    @echo unx
    @-$(RM) $(MISC)/$(TARGET).$(@:b)_2.cmd
    @echo $(APP2LINKER) $(APP2LINKFLAGS) $(LINKFLAGSAPP) -L$(PRJ)/$(INPATH)/lib $(SOLARLIB) $(STDSLO) \
    $(APP2OBJS:s/.obj/.o/) '\' >  $(MISC)/$(TARGET).$(@:b)_2.cmd
    @cat $(mktmp /dev/null $(APP2LIBS)) | xargs -n 1 cat | sed s\#$(ROUT)\#$(OUT)\#g | sed 's#$$# \\#'  >> $(MISC)/$(TARGET).$(@:b)_2.cmd
    @echo $(APP2LINKTYPEFLAG) $(APP2LIBSALCPPRT) $(APP2STDLIBS) $(APP2STDLIB) $(STDLIB2) -o $@ >> $(MISC)/$(TARGET).$(@:b)_2.cmd
  .IF "$(VERBOSE)" == "TRUE"
    @cat $(MISC)/$(TARGET).$(@:b)_2.cmd
  .ENDIF
    @+source $(MISC)/$(TARGET).$(@:b)_2.cmd
  .IF "$(VERBOSE)" == "TRUE"
    @ls -l $@
  .ENDIF
.ENDIF		# "$(OS)"=="MACOSX"
.ENDIF
.IF "$(GUI)" == "WNT"
    @@-$(MKDIR) $(@:d:d)
.IF "$(APP2LINKRES)" != ""
    @@-$(RM) $(MISC)/$(APP2LINKRES:b).rc
.IF "$(APP2ICON)" != ""
    @-echo 1 ICON $(EMQ)"$(APP2ICON:s/\/\\/)$(EMQ)" >> $(MISC)/$(APP2LINKRES:b).rc
.ENDIF		# "$(APP2ICON)" != ""
.IF "$(APP2VERINFO)" != ""
    @-echo $(EMQ)#define VERVARIANT	$(BUILD) >> $(MISC)/$(APP2LINKRES:b).rc
    @-echo $(EMQ)#include  $(EMQ)"$(APP2VERINFO)$(EMQ)" >> $(MISC)/$(APP2LINKRES:b).rc
.ENDIF		# "$(APP2VERINFO)" != ""
    $(COMMAND_ECHO)$(RC) -DWIN32 $(APP2PRODUCTDEF) -I$(SOLARRESDIR) $(INCLUDE) $(RCLINKFLAGS) $(MISC)/$(APP2LINKRES:b).rc
.ENDIF			# "$(APP2LINKRES)" != ""
.IF "$(COM)" == "GCC"
    @echo mingw
.IF "$(APP2LINKRES)" != "" || "$(APP2RES)" != ""
    @cat $(APP2LINKRES) $(subst,/res/,/res{$(subst,$(BIN), $(@:d))} $(APP2RES)) >  $(MISC)/$(@:b)_all.res
    windres $(MISC)/$(@:b)_all.res $(APP2RESO)
.ENDIF
    @echo $(LINK) $(LINKFLAGS) $(LINKFLAGSAPP) $(MINGWSSTDOBJ) -L$(PRJ)/$(INPATH)/lib $(SOLARLIB) $(STDSLO) \
        $(APP2BASEX) $(APP2STACKN) -o $@ $(APP2OBJS) \
        -Wl,-Map,$(MISC)/$(@:b).map $(STDOBJ) $(APP2RESO) \
        `$(TYPE) /dev/null $(APP2LIBS) | sed s#$(ROUT)#$(OUT)#g` \
        $(APP_LINKTYPE) $(APP2LIBSALCPPRT) \
        -Wl,--start-group $(APP2STDLIBS) -Wl,--end-group $(APP2STDLIB) \
        $(STDLIB2) $(MINGWSSTDENDOBJ) > $(MISC)/$(TARGET).$(@:b)_2.cmd
  # need this empty line, else dmake somehow gets confused by the .IFs and .ENDIFs
  .IF "$(VERBOSE)" == "TRUE"
    @$(TYPE)  $(MISC)/$(TARGET).$(@:b)_2.cmd
  .ENDIF
    @+source $(MISC)/$(TARGET).$(@:b)_2.cmd
    @ls -l $@
.ELSE	# "$(COM)" == "GCC"
.IF "$(linkinc)" == ""
    $(COMMAND_ECHO)$(APP2LINKER) @$(mktmp \
        $(APP2LINKFLAGS) \
        $(LINKFLAGSAPP) $(APP2BASEX) \
        $(APP2STACKN) \
        -out:$@ \
        -map:$(MISC)/{$(subst,/,_ $(APP2TARGET)).map} \
        $(STDOBJ) \
        $(APP2LINKRES) \
        $(APP2RES) \
        $(APP2OBJS) \
        $(APP2LIBS) \
        $(APP2STDLIBS) \
        $(APP2STDLIB) $(STDLIB2) \
        )
    @-echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.tmanifest -outputresource:$@$(EMQ);1 $(FI)
.ELSE
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -outputresource:$@$(EMQ);1 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ELSE
        $(COMMAND_ECHO)-$(RM) $(MISC)\$(APP2TARGET).lnk
        $(COMMAND_ECHO)-$(RM) $(MISC)\$(APP2TARGET).lst
        $(COMMAND_ECHO)-$(RM) $(MISC)\linkobj.lst
        for %_i in ($(MISC)\*.obj) do type %_i >> $(MISC)\linkobj.lst
    type $(mktmp,$(MISC)\$(APP2TARGET).lst
        $(APP2LINKFLAGS) \
        $(LINKFLAGSAPP) $(APP2BASEX) \
        $(APP2STACKN) \
        -out:$@ \
        $(STDOBJ) \
        $(APP2LINKRES) \
        $(APP2RES) \
        $(APP2OBJS) \
        $(APP2LIBS) \
        $(APP2STDLIBS) \
        $(APP2STDLIB) $(STDLIB2))
        $(COMMAND_ECHO)$(SED)$(SED) -e 's/\(\.\.\\\)\{2,4\}/..\\/g' $(MISC)\$(APP2TARGETN:b)_linkobj.lst >> $(MISC)\$(APP2TARGET).lst
        $(COMMAND_ECHO)$(SED)$(IFEXIST) $(MISC)/$(APP2TARGET).lst $(THEN) type $(MISC)/$(APP2TARGET).lst  >> $(MISC)/$(APP2TARGET).lnk $(FI)
        $(COMMAND_ECHO)$(SED)$(APP2LINKER) @$(MISC)\$(APP2TARGET).lnk
.ENDIF		# "$(linkinc)" == ""
.ENDIF		# "$(COM)" == "GCC"
.IF "$(APP2TARGET)" == "loader"
    $(COMMAND_ECHO)$(PERL) loader.pl $@
    $(COMMAND_ECHO)$(TYPE) $(@) $(@:d)unloader.exe > $(@:d)_new.exe
    $(COMMAND_ECHO)$(RM) $@
    $(COMMAND_ECHO)$(RENAME) $(@:d)_new.exe $(@:d)loader.exe
.ENDIF			# "$(TARGET)" == "setup"

.ENDIF			# "$(GUI)" == "WNT"

.IF "$(GUI)" == "OS2"
    @+-$(MKDIR) $(@:d:d) > $(NULLDEV)
.IF "$(APP2LINKRES)" != ""
    @+-$(RM) $(MISC)/$(APP2LINKRES:b).rc > $(NULLDEV)
.IF "$(APP2ICON)" != ""
    @-+echo ICON 1 $(EMQ)"$(APP2ICON:s#/#\\\\#)$(EMQ)" >> $(MISC)$/$(APP2LINKRES:b).rc
.ENDIF		# "$(APP2ICON)" != ""
.IF "$(APP2VERINFO)" != ""
    @-+echo $(EMQ)#define VERVARIANT	$(BUILD) >> $(MISC)/$(APP2LINKRES:b).rc
    @-+echo $(EMQ)#include  $(EMQ)"$(APP2VERINFO)$(EMQ)" >> $(MISC)/$(APP2LINKRES:b).rc
.ENDIF		# "$(APP2VERINFO)" != ""
    $(COMMAND_ECHO)$(RC) -r -DOS2 $(APP2PRODUCTDEF) -I$(SOLARRESDIR) $(INCLUDE) $(RCLINKFLAGS) $(MISC)/$(APP2LINKRES:b).rc
.ENDIF			# "$(APP2LINKRES)" != ""

.IF "$(TARGETTYPE)" == "GUI" 
    @echo NAME $(APP2TARGET) WINDOWAPI > $(MISC)/$(APP2TARGET).def
.ENDIF

    $(COMMAND_ECHO)$(APP2LINKER) -v \
        $(APP2LINKFLAGS) \
        $(LINKFLAGSAPP) $(APP2BASEX) \
        $(APP2STACKN) \
        -o $@ \
        -Zmap -L$(LB) \
        -L$(SOLARVERSION)/$(INPATH)/lib \
        $(STDOBJ) \
        $(APP2LINKRES) \
        $(APP2RES) \
        $(APP2DEF) \
        $(APP2OBJS) \
        $(APP2LIBS) \
        $(APP2STDLIBS) \
        $(APP2STDLIB) $(STDLIB2) 


.IF "$(APP2TARGET)" == "loader"
    $(COMMAND_ECHO)+$(PERL) loader.pl $@
    $(COMMAND_ECHO)+$(TYPE) $(@) $(@:d)unloader.exe > $(@:d)_new.exe
    $(COMMAND_ECHO)+$(RM) $@
    $(COMMAND_ECHO)+$(RENAME) $(@:d)_new.exe $(@:d)loader.exe
.ENDIF			# "$(TARGET)" == "setup"

.ENDIF			# "$(GUI)" == "OS2"

.ENDIF			# "$(APP2TARGETN)"!=""


# Instruction for linking
# unroll begin

.IF "$(GUI)" == "OS2" && "$(TARGETTYPE)" == "GUI" 
APP3DEF = $(MISC)/$(APP3TARGET).def
.ENDIF

.IF "$(APP3LINKTYPE)" != ""
#must be either STATIC or SHARED
APP3LINKTYPEFLAG=$(APPLINK$(APP3LINKTYPE))
.ENDIF

# decide how to link
.IF "$(APP3CODETYPE)"=="C"
APP3LINKER=$(LINKC)
APP3STDLIB=$(subst,CPPRUNTIME, $(STDLIB))
APP3LINKFLAGS+=$(LINKCFLAGS)
.ELSE			# "$(APP3CODETYPE)"=="C"
APP3LINKER=$(LINK)
APP3STDLIB=$(subst,CPPRUNTIME,$(STDLIBCPP) $(STDLIB))
APP3LINKFLAGS+=$(LINKFLAGS)
.ENDIF			# "$(APP3CODETYPE)"=="C"

APP3RPATH*=OOO
LINKFLAGSRUNPATH_$(APP3RPATH)*=/ERROR:/Bad_APP3RPATH_value
.IF "$(OS)" != "MACOSX"
APP3LINKFLAGS+=$(LINKFLAGSRUNPATH_$(APP3RPATH))
.ENDIF

.IF "$(APP3STACK)" != ""
.IF "$(LINKFLAGSTACK)" != ""
APP3STACKN=$(LINKFLAGSTACK)$(APP3STACK)
.ENDIF
.ELSE
APP3STACKN=
.ENDIF

.IF "$(APP3NOSAL)"==""
.IF "$(TARGETTYPE)" == "GUI"
APP3OBJS+= $(STDOBJVCL)
.ENDIF
.ENDIF

.IF "$(GUI)$(COM)" == "WNTGCC"
APP3RESO=
.IF "$(APP3LINKRES)" != "" || "$(APP3RES)" != ""
APP3RESO=$(MISC)/$(APP3TARGET:b)_res.o
.ENDIF
.ENDIF

.IF "$(GUI)" == "UNX"
APP3DEPN+:=$(APP3DEPNU)
USE_APP3DEF=
.ENDIF

.IF "$(APP3TARGETN)"!=""

.IF "$(APP3PRODUCTNAME)"!=""
APP3PRODUCTDEF+:=-DPRODUCT_NAME=\"$(APP3PRODUCTNAME)\"
.ENDIF			# "$(APP3PRODUCTNAME)"!=""

.IF "$(linkinc)"!=""
.IF "$(GUI)"=="WNT" || "$(GUI)"=="OS2"
.IF "$(APP3LIBS)"!=""
$(MISC)/$(APP3TARGET)_linkinc.ls .PHONY:
    @@-$(RM) $@
    sed -f $(SOLARENV)/bin/chrel.sed $(foreach,i,$(APP3LIBS) $(i:s/.lib/.lin/)) >> $@
.ENDIF          #"$(APP3LIBS)"!="" 
.ENDIF

LINKINCTARGETS+=$(MISC)/$(APP3TARGETN:b)_linkinc.ls
$(APP3TARGETN) : $(LINKINCTARGETS)
.ENDIF          # "$(linkinc)"!=""

# Allow for target specific LIBSALCPPRT override
APP3LIBSALCPPRT*=$(LIBSALCPPRT)

$(APP3TARGETN): $(APP3OBJS) $(APP3LIBS) \
    $(APP3RES) \
    $(APP3ICON) $(APP3DEPN) $(USE_APP3DEF)
    @echo "Making:   " $(@:f)
.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="MACOSX"
    @echo unx
    @-$(RM) $(MISC)/$(@:b).list
    @-$(RM) $(MISC)/$(TARGET).$(@:b)_3.cmd
    @-$(RM) $(MISC)/$(@:b).strip
    @echo $(STDSLO) $(APP3OBJS:s/.obj/.o/) \
    `cat /dev/null $(APP3LIBS) | sed s\#$(ROUT)\#$(OUT)\#g` | tr -s " " "\n" > $(MISC)/$(@:b).list
    @echo -n $(APP3LINKER) $(APP3LINKFLAGS) $(LINKFLAGSAPP) -L$(PRJ)/$(INPATH)/lib $(SOLARLIB) -o $@ \
    $(APP3LINKTYPEFLAG) $(APP3STDLIBS) $(APP3STDLIB) $(STDLIB3) -filelist $(MISC)/$(@:b).list > $(MISC)/$(TARGET).$(@:b)_3.cmd
    @$(PERL) $(SOLARENV)/bin/macosx-dylib-link-list.pl \
        `cat $(MISC)/$(TARGET).$(@:b)_3.cmd` \
        >> $(MISC)/$(TARGET).$(@:b)_3.cmd
  .IF "$(VERBOSE)" == "TRUE"
    @cat $(MISC)/$(TARGET).$(@:b)_3.cmd
  .ENDIF
    @+source $(MISC)/$(TARGET).$(@:b)_3.cmd
# Need to strip __objcInit symbol to avoid duplicate symbols when loading
# libraries at runtime
    @-$(CC_PATH)nm $@ | grep -v ' U ' | $(AWK) '{ print $$NF }' | grep -F -x '__objcInit' > $(MISC)/$(@:b).strip
    @$(CC_PATH)strip -i -R $(MISC)/$(@:b).strip -X $@
    @ls -l $@
    @$(PERL) $(SOLARENV)/bin/macosx-change-install-names.pl \
        app $(APP3RPATH) $@
.IF "$(TARGETTYPE)"=="GUI"
    @echo "Making:   " $(@:f).app
    @macosx-create-bundle $@
.ENDIF		# "$(TARGETTYPE)"=="GUI"
.ELSE		# "$(OS)"=="MACOSX"
    @echo unx
    @-$(RM) $(MISC)/$(TARGET).$(@:b)_3.cmd
    @echo $(APP3LINKER) $(APP3LINKFLAGS) $(LINKFLAGSAPP) -L$(PRJ)/$(INPATH)/lib $(SOLARLIB) $(STDSLO) \
    $(APP3OBJS:s/.obj/.o/) '\' >  $(MISC)/$(TARGET).$(@:b)_3.cmd
    @cat $(mktmp /dev/null $(APP3LIBS)) | xargs -n 1 cat | sed s\#$(ROUT)\#$(OUT)\#g | sed 's#$$# \\#'  >> $(MISC)/$(TARGET).$(@:b)_3.cmd
    @echo $(APP3LINKTYPEFLAG) $(APP3LIBSALCPPRT) $(APP3STDLIBS) $(APP3STDLIB) $(STDLIB3) -o $@ >> $(MISC)/$(TARGET).$(@:b)_3.cmd
  .IF "$(VERBOSE)" == "TRUE"
    @cat $(MISC)/$(TARGET).$(@:b)_3.cmd
  .ENDIF
    @+source $(MISC)/$(TARGET).$(@:b)_3.cmd
  .IF "$(VERBOSE)" == "TRUE"
    @ls -l $@
  .ENDIF
.ENDIF		# "$(OS)"=="MACOSX"
.ENDIF
.IF "$(GUI)" == "WNT"
    @@-$(MKDIR) $(@:d:d)
.IF "$(APP3LINKRES)" != ""
    @@-$(RM) $(MISC)/$(APP3LINKRES:b).rc
.IF "$(APP3ICON)" != ""
    @-echo 1 ICON $(EMQ)"$(APP3ICON:s/\/\\/)$(EMQ)" >> $(MISC)/$(APP3LINKRES:b).rc
.ENDIF		# "$(APP3ICON)" != ""
.IF "$(APP3VERINFO)" != ""
    @-echo $(EMQ)#define VERVARIANT	$(BUILD) >> $(MISC)/$(APP3LINKRES:b).rc
    @-echo $(EMQ)#include  $(EMQ)"$(APP3VERINFO)$(EMQ)" >> $(MISC)/$(APP3LINKRES:b).rc
.ENDIF		# "$(APP3VERINFO)" != ""
    $(COMMAND_ECHO)$(RC) -DWIN32 $(APP3PRODUCTDEF) -I$(SOLARRESDIR) $(INCLUDE) $(RCLINKFLAGS) $(MISC)/$(APP3LINKRES:b).rc
.ENDIF			# "$(APP3LINKRES)" != ""
.IF "$(COM)" == "GCC"
    @echo mingw
.IF "$(APP3LINKRES)" != "" || "$(APP3RES)" != ""
    @cat $(APP3LINKRES) $(subst,/res/,/res{$(subst,$(BIN), $(@:d))} $(APP3RES)) >  $(MISC)/$(@:b)_all.res
    windres $(MISC)/$(@:b)_all.res $(APP3RESO)
.ENDIF
    @echo $(LINK) $(LINKFLAGS) $(LINKFLAGSAPP) $(MINGWSSTDOBJ) -L$(PRJ)/$(INPATH)/lib $(SOLARLIB) $(STDSLO) \
        $(APP3BASEX) $(APP3STACKN) -o $@ $(APP3OBJS) \
        -Wl,-Map,$(MISC)/$(@:b).map $(STDOBJ) $(APP3RESO) \
        `$(TYPE) /dev/null $(APP3LIBS) | sed s#$(ROUT)#$(OUT)#g` \
        $(APP_LINKTYPE) $(APP3LIBSALCPPRT) \
        -Wl,--start-group $(APP3STDLIBS) -Wl,--end-group $(APP3STDLIB) \
        $(STDLIB3) $(MINGWSSTDENDOBJ) > $(MISC)/$(TARGET).$(@:b)_3.cmd
  # need this empty line, else dmake somehow gets confused by the .IFs and .ENDIFs
  .IF "$(VERBOSE)" == "TRUE"
    @$(TYPE)  $(MISC)/$(TARGET).$(@:b)_3.cmd
  .ENDIF
    @+source $(MISC)/$(TARGET).$(@:b)_3.cmd
    @ls -l $@
.ELSE	# "$(COM)" == "GCC"
.IF "$(linkinc)" == ""
    $(COMMAND_ECHO)$(APP3LINKER) @$(mktmp \
        $(APP3LINKFLAGS) \
        $(LINKFLAGSAPP) $(APP3BASEX) \
        $(APP3STACKN) \
        -out:$@ \
        -map:$(MISC)/{$(subst,/,_ $(APP3TARGET)).map} \
        $(STDOBJ) \
        $(APP3LINKRES) \
        $(APP3RES) \
        $(APP3OBJS) \
        $(APP3LIBS) \
        $(APP3STDLIBS) \
        $(APP3STDLIB) $(STDLIB3) \
        )
    @-echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.tmanifest -outputresource:$@$(EMQ);1 $(FI)
.ELSE
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -outputresource:$@$(EMQ);1 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ELSE
        $(COMMAND_ECHO)-$(RM) $(MISC)\$(APP3TARGET).lnk
        $(COMMAND_ECHO)-$(RM) $(MISC)\$(APP3TARGET).lst
        $(COMMAND_ECHO)-$(RM) $(MISC)\linkobj.lst
        for %_i in ($(MISC)\*.obj) do type %_i >> $(MISC)\linkobj.lst
    type $(mktmp,$(MISC)\$(APP3TARGET).lst
        $(APP3LINKFLAGS) \
        $(LINKFLAGSAPP) $(APP3BASEX) \
        $(APP3STACKN) \
        -out:$@ \
        $(STDOBJ) \
        $(APP3LINKRES) \
        $(APP3RES) \
        $(APP3OBJS) \
        $(APP3LIBS) \
        $(APP3STDLIBS) \
        $(APP3STDLIB) $(STDLIB3))
        $(COMMAND_ECHO)$(SED)$(SED) -e 's/\(\.\.\\\)\{2,4\}/..\\/g' $(MISC)\$(APP3TARGETN:b)_linkobj.lst >> $(MISC)\$(APP3TARGET).lst
        $(COMMAND_ECHO)$(SED)$(IFEXIST) $(MISC)/$(APP3TARGET).lst $(THEN) type $(MISC)/$(APP3TARGET).lst  >> $(MISC)/$(APP3TARGET).lnk $(FI)
        $(COMMAND_ECHO)$(SED)$(APP3LINKER) @$(MISC)\$(APP3TARGET).lnk
.ENDIF		# "$(linkinc)" == ""
.ENDIF		# "$(COM)" == "GCC"
.IF "$(APP3TARGET)" == "loader"
    $(COMMAND_ECHO)$(PERL) loader.pl $@
    $(COMMAND_ECHO)$(TYPE) $(@) $(@:d)unloader.exe > $(@:d)_new.exe
    $(COMMAND_ECHO)$(RM) $@
    $(COMMAND_ECHO)$(RENAME) $(@:d)_new.exe $(@:d)loader.exe
.ENDIF			# "$(TARGET)" == "setup"

.ENDIF			# "$(GUI)" == "WNT"

.IF "$(GUI)" == "OS2"
    @+-$(MKDIR) $(@:d:d) > $(NULLDEV)
.IF "$(APP3LINKRES)" != ""
    @+-$(RM) $(MISC)/$(APP3LINKRES:b).rc > $(NULLDEV)
.IF "$(APP3ICON)" != ""
    @-+echo ICON 1 $(EMQ)"$(APP3ICON:s#/#\\\\#)$(EMQ)" >> $(MISC)$/$(APP3LINKRES:b).rc
.ENDIF		# "$(APP3ICON)" != ""
.IF "$(APP3VERINFO)" != ""
    @-+echo $(EMQ)#define VERVARIANT	$(BUILD) >> $(MISC)/$(APP3LINKRES:b).rc
    @-+echo $(EMQ)#include  $(EMQ)"$(APP3VERINFO)$(EMQ)" >> $(MISC)/$(APP3LINKRES:b).rc
.ENDIF		# "$(APP3VERINFO)" != ""
    $(COMMAND_ECHO)$(RC) -r -DOS2 $(APP3PRODUCTDEF) -I$(SOLARRESDIR) $(INCLUDE) $(RCLINKFLAGS) $(MISC)/$(APP3LINKRES:b).rc
.ENDIF			# "$(APP3LINKRES)" != ""

.IF "$(TARGETTYPE)" == "GUI" 
    @echo NAME $(APP3TARGET) WINDOWAPI > $(MISC)/$(APP3TARGET).def
.ENDIF

    $(COMMAND_ECHO)$(APP3LINKER) -v \
        $(APP3LINKFLAGS) \
        $(LINKFLAGSAPP) $(APP3BASEX) \
        $(APP3STACKN) \
        -o $@ \
        -Zmap -L$(LB) \
        -L$(SOLARVERSION)/$(INPATH)/lib \
        $(STDOBJ) \
        $(APP3LINKRES) \
        $(APP3RES) \
        $(APP3DEF) \
        $(APP3OBJS) \
        $(APP3LIBS) \
        $(APP3STDLIBS) \
        $(APP3STDLIB) $(STDLIB3) 


.IF "$(APP3TARGET)" == "loader"
    $(COMMAND_ECHO)+$(PERL) loader.pl $@
    $(COMMAND_ECHO)+$(TYPE) $(@) $(@:d)unloader.exe > $(@:d)_new.exe
    $(COMMAND_ECHO)+$(RM) $@
    $(COMMAND_ECHO)+$(RENAME) $(@:d)_new.exe $(@:d)loader.exe
.ENDIF			# "$(TARGET)" == "setup"

.ENDIF			# "$(GUI)" == "OS2"

.ENDIF			# "$(APP3TARGETN)"!=""


# Instruction for linking
# unroll begin

.IF "$(GUI)" == "OS2" && "$(TARGETTYPE)" == "GUI" 
APP4DEF = $(MISC)/$(APP4TARGET).def
.ENDIF

.IF "$(APP4LINKTYPE)" != ""
#must be either STATIC or SHARED
APP4LINKTYPEFLAG=$(APPLINK$(APP4LINKTYPE))
.ENDIF

# decide how to link
.IF "$(APP4CODETYPE)"=="C"
APP4LINKER=$(LINKC)
APP4STDLIB=$(subst,CPPRUNTIME, $(STDLIB))
APP4LINKFLAGS+=$(LINKCFLAGS)
.ELSE			# "$(APP4CODETYPE)"=="C"
APP4LINKER=$(LINK)
APP4STDLIB=$(subst,CPPRUNTIME,$(STDLIBCPP) $(STDLIB))
APP4LINKFLAGS+=$(LINKFLAGS)
.ENDIF			# "$(APP4CODETYPE)"=="C"

APP4RPATH*=OOO
LINKFLAGSRUNPATH_$(APP4RPATH)*=/ERROR:/Bad_APP4RPATH_value
.IF "$(OS)" != "MACOSX"
APP4LINKFLAGS+=$(LINKFLAGSRUNPATH_$(APP4RPATH))
.ENDIF

.IF "$(APP4STACK)" != ""
.IF "$(LINKFLAGSTACK)" != ""
APP4STACKN=$(LINKFLAGSTACK)$(APP4STACK)
.ENDIF
.ELSE
APP4STACKN=
.ENDIF

.IF "$(APP4NOSAL)"==""
.IF "$(TARGETTYPE)" == "GUI"
APP4OBJS+= $(STDOBJVCL)
.ENDIF
.ENDIF

.IF "$(GUI)$(COM)" == "WNTGCC"
APP4RESO=
.IF "$(APP4LINKRES)" != "" || "$(APP4RES)" != ""
APP4RESO=$(MISC)/$(APP4TARGET:b)_res.o
.ENDIF
.ENDIF

.IF "$(GUI)" == "UNX"
APP4DEPN+:=$(APP4DEPNU)
USE_APP4DEF=
.ENDIF

.IF "$(APP4TARGETN)"!=""

.IF "$(APP4PRODUCTNAME)"!=""
APP4PRODUCTDEF+:=-DPRODUCT_NAME=\"$(APP4PRODUCTNAME)\"
.ENDIF			# "$(APP4PRODUCTNAME)"!=""

.IF "$(linkinc)"!=""
.IF "$(GUI)"=="WNT" || "$(GUI)"=="OS2"
.IF "$(APP4LIBS)"!=""
$(MISC)/$(APP4TARGET)_linkinc.ls .PHONY:
    @@-$(RM) $@
    sed -f $(SOLARENV)/bin/chrel.sed $(foreach,i,$(APP4LIBS) $(i:s/.lib/.lin/)) >> $@
.ENDIF          #"$(APP4LIBS)"!="" 
.ENDIF

LINKINCTARGETS+=$(MISC)/$(APP4TARGETN:b)_linkinc.ls
$(APP4TARGETN) : $(LINKINCTARGETS)
.ENDIF          # "$(linkinc)"!=""

# Allow for target specific LIBSALCPPRT override
APP4LIBSALCPPRT*=$(LIBSALCPPRT)

$(APP4TARGETN): $(APP4OBJS) $(APP4LIBS) \
    $(APP4RES) \
    $(APP4ICON) $(APP4DEPN) $(USE_APP4DEF)
    @echo "Making:   " $(@:f)
.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="MACOSX"
    @echo unx
    @-$(RM) $(MISC)/$(@:b).list
    @-$(RM) $(MISC)/$(TARGET).$(@:b)_4.cmd
    @-$(RM) $(MISC)/$(@:b).strip
    @echo $(STDSLO) $(APP4OBJS:s/.obj/.o/) \
    `cat /dev/null $(APP4LIBS) | sed s\#$(ROUT)\#$(OUT)\#g` | tr -s " " "\n" > $(MISC)/$(@:b).list
    @echo -n $(APP4LINKER) $(APP4LINKFLAGS) $(LINKFLAGSAPP) -L$(PRJ)/$(INPATH)/lib $(SOLARLIB) -o $@ \
    $(APP4LINKTYPEFLAG) $(APP4STDLIBS) $(APP4STDLIB) $(STDLIB4) -filelist $(MISC)/$(@:b).list > $(MISC)/$(TARGET).$(@:b)_4.cmd
    @$(PERL) $(SOLARENV)/bin/macosx-dylib-link-list.pl \
        `cat $(MISC)/$(TARGET).$(@:b)_4.cmd` \
        >> $(MISC)/$(TARGET).$(@:b)_4.cmd
  .IF "$(VERBOSE)" == "TRUE"
    @cat $(MISC)/$(TARGET).$(@:b)_4.cmd
  .ENDIF
    @+source $(MISC)/$(TARGET).$(@:b)_4.cmd
# Need to strip __objcInit symbol to avoid duplicate symbols when loading
# libraries at runtime
    @-$(CC_PATH)nm $@ | grep -v ' U ' | $(AWK) '{ print $$NF }' | grep -F -x '__objcInit' > $(MISC)/$(@:b).strip
    @$(CC_PATH)strip -i -R $(MISC)/$(@:b).strip -X $@
    @ls -l $@
    @$(PERL) $(SOLARENV)/bin/macosx-change-install-names.pl \
        app $(APP4RPATH) $@
.IF "$(TARGETTYPE)"=="GUI"
    @echo "Making:   " $(@:f).app
    @macosx-create-bundle $@
.ENDIF		# "$(TARGETTYPE)"=="GUI"
.ELSE		# "$(OS)"=="MACOSX"
    @echo unx
    @-$(RM) $(MISC)/$(TARGET).$(@:b)_4.cmd
    @echo $(APP4LINKER) $(APP4LINKFLAGS) $(LINKFLAGSAPP) -L$(PRJ)/$(INPATH)/lib $(SOLARLIB) $(STDSLO) \
    $(APP4OBJS:s/.obj/.o/) '\' >  $(MISC)/$(TARGET).$(@:b)_4.cmd
    @cat $(mktmp /dev/null $(APP4LIBS)) | xargs -n 1 cat | sed s\#$(ROUT)\#$(OUT)\#g | sed 's#$$# \\#'  >> $(MISC)/$(TARGET).$(@:b)_4.cmd
    @echo $(APP4LINKTYPEFLAG) $(APP4LIBSALCPPRT) $(APP4STDLIBS) $(APP4STDLIB) $(STDLIB4) -o $@ >> $(MISC)/$(TARGET).$(@:b)_4.cmd
  .IF "$(VERBOSE)" == "TRUE"
    @cat $(MISC)/$(TARGET).$(@:b)_4.cmd
  .ENDIF
    @+source $(MISC)/$(TARGET).$(@:b)_4.cmd
  .IF "$(VERBOSE)" == "TRUE"
    @ls -l $@
  .ENDIF
.ENDIF		# "$(OS)"=="MACOSX"
.ENDIF
.IF "$(GUI)" == "WNT"
    @@-$(MKDIR) $(@:d:d)
.IF "$(APP4LINKRES)" != ""
    @@-$(RM) $(MISC)/$(APP4LINKRES:b).rc
.IF "$(APP4ICON)" != ""
    @-echo 1 ICON $(EMQ)"$(APP4ICON:s/\/\\/)$(EMQ)" >> $(MISC)/$(APP4LINKRES:b).rc
.ENDIF		# "$(APP4ICON)" != ""
.IF "$(APP4VERINFO)" != ""
    @-echo $(EMQ)#define VERVARIANT	$(BUILD) >> $(MISC)/$(APP4LINKRES:b).rc
    @-echo $(EMQ)#include  $(EMQ)"$(APP4VERINFO)$(EMQ)" >> $(MISC)/$(APP4LINKRES:b).rc
.ENDIF		# "$(APP4VERINFO)" != ""
    $(COMMAND_ECHO)$(RC) -DWIN32 $(APP4PRODUCTDEF) -I$(SOLARRESDIR) $(INCLUDE) $(RCLINKFLAGS) $(MISC)/$(APP4LINKRES:b).rc
.ENDIF			# "$(APP4LINKRES)" != ""
.IF "$(COM)" == "GCC"
    @echo mingw
.IF "$(APP4LINKRES)" != "" || "$(APP4RES)" != ""
    @cat $(APP4LINKRES) $(subst,/res/,/res{$(subst,$(BIN), $(@:d))} $(APP4RES)) >  $(MISC)/$(@:b)_all.res
    windres $(MISC)/$(@:b)_all.res $(APP4RESO)
.ENDIF
    @echo $(LINK) $(LINKFLAGS) $(LINKFLAGSAPP) $(MINGWSSTDOBJ) -L$(PRJ)/$(INPATH)/lib $(SOLARLIB) $(STDSLO) \
        $(APP4BASEX) $(APP4STACKN) -o $@ $(APP4OBJS) \
        -Wl,-Map,$(MISC)/$(@:b).map $(STDOBJ) $(APP4RESO) \
        `$(TYPE) /dev/null $(APP4LIBS) | sed s#$(ROUT)#$(OUT)#g` \
        $(APP_LINKTYPE) $(APP4LIBSALCPPRT) \
        -Wl,--start-group $(APP4STDLIBS) -Wl,--end-group $(APP4STDLIB) \
        $(STDLIB4) $(MINGWSSTDENDOBJ) > $(MISC)/$(TARGET).$(@:b)_4.cmd
  # need this empty line, else dmake somehow gets confused by the .IFs and .ENDIFs
  .IF "$(VERBOSE)" == "TRUE"
    @$(TYPE)  $(MISC)/$(TARGET).$(@:b)_4.cmd
  .ENDIF
    @+source $(MISC)/$(TARGET).$(@:b)_4.cmd
    @ls -l $@
.ELSE	# "$(COM)" == "GCC"
.IF "$(linkinc)" == ""
    $(COMMAND_ECHO)$(APP4LINKER) @$(mktmp \
        $(APP4LINKFLAGS) \
        $(LINKFLAGSAPP) $(APP4BASEX) \
        $(APP4STACKN) \
        -out:$@ \
        -map:$(MISC)/{$(subst,/,_ $(APP4TARGET)).map} \
        $(STDOBJ) \
        $(APP4LINKRES) \
        $(APP4RES) \
        $(APP4OBJS) \
        $(APP4LIBS) \
        $(APP4STDLIBS) \
        $(APP4STDLIB) $(STDLIB4) \
        )
    @-echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.tmanifest -outputresource:$@$(EMQ);1 $(FI)
.ELSE
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -outputresource:$@$(EMQ);1 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ELSE
        $(COMMAND_ECHO)-$(RM) $(MISC)\$(APP4TARGET).lnk
        $(COMMAND_ECHO)-$(RM) $(MISC)\$(APP4TARGET).lst
        $(COMMAND_ECHO)-$(RM) $(MISC)\linkobj.lst
        for %_i in ($(MISC)\*.obj) do type %_i >> $(MISC)\linkobj.lst
    type $(mktmp,$(MISC)\$(APP4TARGET).lst
        $(APP4LINKFLAGS) \
        $(LINKFLAGSAPP) $(APP4BASEX) \
        $(APP4STACKN) \
        -out:$@ \
        $(STDOBJ) \
        $(APP4LINKRES) \
        $(APP4RES) \
        $(APP4OBJS) \
        $(APP4LIBS) \
        $(APP4STDLIBS) \
        $(APP4STDLIB) $(STDLIB4))
        $(COMMAND_ECHO)$(SED)$(SED) -e 's/\(\.\.\\\)\{2,4\}/..\\/g' $(MISC)\$(APP4TARGETN:b)_linkobj.lst >> $(MISC)\$(APP4TARGET).lst
        $(COMMAND_ECHO)$(SED)$(IFEXIST) $(MISC)/$(APP4TARGET).lst $(THEN) type $(MISC)/$(APP4TARGET).lst  >> $(MISC)/$(APP4TARGET).lnk $(FI)
        $(COMMAND_ECHO)$(SED)$(APP4LINKER) @$(MISC)\$(APP4TARGET).lnk
.ENDIF		# "$(linkinc)" == ""
.ENDIF		# "$(COM)" == "GCC"
.IF "$(APP4TARGET)" == "loader"
    $(COMMAND_ECHO)$(PERL) loader.pl $@
    $(COMMAND_ECHO)$(TYPE) $(@) $(@:d)unloader.exe > $(@:d)_new.exe
    $(COMMAND_ECHO)$(RM) $@
    $(COMMAND_ECHO)$(RENAME) $(@:d)_new.exe $(@:d)loader.exe
.ENDIF			# "$(TARGET)" == "setup"

.ENDIF			# "$(GUI)" == "WNT"

.IF "$(GUI)" == "OS2"
    @+-$(MKDIR) $(@:d:d) > $(NULLDEV)
.IF "$(APP4LINKRES)" != ""
    @+-$(RM) $(MISC)/$(APP4LINKRES:b).rc > $(NULLDEV)
.IF "$(APP4ICON)" != ""
    @-+echo ICON 1 $(EMQ)"$(APP4ICON:s#/#\\\\#)$(EMQ)" >> $(MISC)$/$(APP4LINKRES:b).rc
.ENDIF		# "$(APP4ICON)" != ""
.IF "$(APP4VERINFO)" != ""
    @-+echo $(EMQ)#define VERVARIANT	$(BUILD) >> $(MISC)/$(APP4LINKRES:b).rc
    @-+echo $(EMQ)#include  $(EMQ)"$(APP4VERINFO)$(EMQ)" >> $(MISC)/$(APP4LINKRES:b).rc
.ENDIF		# "$(APP4VERINFO)" != ""
    $(COMMAND_ECHO)$(RC) -r -DOS2 $(APP4PRODUCTDEF) -I$(SOLARRESDIR) $(INCLUDE) $(RCLINKFLAGS) $(MISC)/$(APP4LINKRES:b).rc
.ENDIF			# "$(APP4LINKRES)" != ""

.IF "$(TARGETTYPE)" == "GUI" 
    @echo NAME $(APP4TARGET) WINDOWAPI > $(MISC)/$(APP4TARGET).def
.ENDIF

    $(COMMAND_ECHO)$(APP4LINKER) -v \
        $(APP4LINKFLAGS) \
        $(LINKFLAGSAPP) $(APP4BASEX) \
        $(APP4STACKN) \
        -o $@ \
        -Zmap -L$(LB) \
        -L$(SOLARVERSION)/$(INPATH)/lib \
        $(STDOBJ) \
        $(APP4LINKRES) \
        $(APP4RES) \
        $(APP4DEF) \
        $(APP4OBJS) \
        $(APP4LIBS) \
        $(APP4STDLIBS) \
        $(APP4STDLIB) $(STDLIB4) 


.IF "$(APP4TARGET)" == "loader"
    $(COMMAND_ECHO)+$(PERL) loader.pl $@
    $(COMMAND_ECHO)+$(TYPE) $(@) $(@:d)unloader.exe > $(@:d)_new.exe
    $(COMMAND_ECHO)+$(RM) $@
    $(COMMAND_ECHO)+$(RENAME) $(@:d)_new.exe $(@:d)loader.exe
.ENDIF			# "$(TARGET)" == "setup"

.ENDIF			# "$(GUI)" == "OS2"

.ENDIF			# "$(APP4TARGETN)"!=""


# Instruction for linking
# unroll begin

.IF "$(GUI)" == "OS2" && "$(TARGETTYPE)" == "GUI" 
APP5DEF = $(MISC)/$(APP5TARGET).def
.ENDIF

.IF "$(APP5LINKTYPE)" != ""
#must be either STATIC or SHARED
APP5LINKTYPEFLAG=$(APPLINK$(APP5LINKTYPE))
.ENDIF

# decide how to link
.IF "$(APP5CODETYPE)"=="C"
APP5LINKER=$(LINKC)
APP5STDLIB=$(subst,CPPRUNTIME, $(STDLIB))
APP5LINKFLAGS+=$(LINKCFLAGS)
.ELSE			# "$(APP5CODETYPE)"=="C"
APP5LINKER=$(LINK)
APP5STDLIB=$(subst,CPPRUNTIME,$(STDLIBCPP) $(STDLIB))
APP5LINKFLAGS+=$(LINKFLAGS)
.ENDIF			# "$(APP5CODETYPE)"=="C"

APP5RPATH*=OOO
LINKFLAGSRUNPATH_$(APP5RPATH)*=/ERROR:/Bad_APP5RPATH_value
.IF "$(OS)" != "MACOSX"
APP5LINKFLAGS+=$(LINKFLAGSRUNPATH_$(APP5RPATH))
.ENDIF

.IF "$(APP5STACK)" != ""
.IF "$(LINKFLAGSTACK)" != ""
APP5STACKN=$(LINKFLAGSTACK)$(APP5STACK)
.ENDIF
.ELSE
APP5STACKN=
.ENDIF

.IF "$(APP5NOSAL)"==""
.IF "$(TARGETTYPE)" == "GUI"
APP5OBJS+= $(STDOBJVCL)
.ENDIF
.ENDIF

.IF "$(GUI)$(COM)" == "WNTGCC"
APP5RESO=
.IF "$(APP5LINKRES)" != "" || "$(APP5RES)" != ""
APP5RESO=$(MISC)/$(APP5TARGET:b)_res.o
.ENDIF
.ENDIF

.IF "$(GUI)" == "UNX"
APP5DEPN+:=$(APP5DEPNU)
USE_APP5DEF=
.ENDIF

.IF "$(APP5TARGETN)"!=""

.IF "$(APP5PRODUCTNAME)"!=""
APP5PRODUCTDEF+:=-DPRODUCT_NAME=\"$(APP5PRODUCTNAME)\"
.ENDIF			# "$(APP5PRODUCTNAME)"!=""

.IF "$(linkinc)"!=""
.IF "$(GUI)"=="WNT" || "$(GUI)"=="OS2"
.IF "$(APP5LIBS)"!=""
$(MISC)/$(APP5TARGET)_linkinc.ls .PHONY:
    @@-$(RM) $@
    sed -f $(SOLARENV)/bin/chrel.sed $(foreach,i,$(APP5LIBS) $(i:s/.lib/.lin/)) >> $@
.ENDIF          #"$(APP5LIBS)"!="" 
.ENDIF

LINKINCTARGETS+=$(MISC)/$(APP5TARGETN:b)_linkinc.ls
$(APP5TARGETN) : $(LINKINCTARGETS)
.ENDIF          # "$(linkinc)"!=""

# Allow for target specific LIBSALCPPRT override
APP5LIBSALCPPRT*=$(LIBSALCPPRT)

$(APP5TARGETN): $(APP5OBJS) $(APP5LIBS) \
    $(APP5RES) \
    $(APP5ICON) $(APP5DEPN) $(USE_APP5DEF)
    @echo "Making:   " $(@:f)
.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="MACOSX"
    @echo unx
    @-$(RM) $(MISC)/$(@:b).list
    @-$(RM) $(MISC)/$(TARGET).$(@:b)_5.cmd
    @-$(RM) $(MISC)/$(@:b).strip
    @echo $(STDSLO) $(APP5OBJS:s/.obj/.o/) \
    `cat /dev/null $(APP5LIBS) | sed s\#$(ROUT)\#$(OUT)\#g` | tr -s " " "\n" > $(MISC)/$(@:b).list
    @echo -n $(APP5LINKER) $(APP5LINKFLAGS) $(LINKFLAGSAPP) -L$(PRJ)/$(INPATH)/lib $(SOLARLIB) -o $@ \
    $(APP5LINKTYPEFLAG) $(APP5STDLIBS) $(APP5STDLIB) $(STDLIB5) -filelist $(MISC)/$(@:b).list > $(MISC)/$(TARGET).$(@:b)_5.cmd
    @$(PERL) $(SOLARENV)/bin/macosx-dylib-link-list.pl \
        `cat $(MISC)/$(TARGET).$(@:b)_5.cmd` \
        >> $(MISC)/$(TARGET).$(@:b)_5.cmd
  .IF "$(VERBOSE)" == "TRUE"
    @cat $(MISC)/$(TARGET).$(@:b)_5.cmd
  .ENDIF
    @+source $(MISC)/$(TARGET).$(@:b)_5.cmd
# Need to strip __objcInit symbol to avoid duplicate symbols when loading
# libraries at runtime
    @-$(CC_PATH)nm $@ | grep -v ' U ' | $(AWK) '{ print $$NF }' | grep -F -x '__objcInit' > $(MISC)/$(@:b).strip
    @$(CC_PATH)strip -i -R $(MISC)/$(@:b).strip -X $@
    @ls -l $@
    @$(PERL) $(SOLARENV)/bin/macosx-change-install-names.pl \
        app $(APP5RPATH) $@
.IF "$(TARGETTYPE)"=="GUI"
    @echo "Making:   " $(@:f).app
    @macosx-create-bundle $@
.ENDIF		# "$(TARGETTYPE)"=="GUI"
.ELSE		# "$(OS)"=="MACOSX"
    @echo unx
    @-$(RM) $(MISC)/$(TARGET).$(@:b)_5.cmd
    @echo $(APP5LINKER) $(APP5LINKFLAGS) $(LINKFLAGSAPP) -L$(PRJ)/$(INPATH)/lib $(SOLARLIB) $(STDSLO) \
    $(APP5OBJS:s/.obj/.o/) '\' >  $(MISC)/$(TARGET).$(@:b)_5.cmd
    @cat $(mktmp /dev/null $(APP5LIBS)) | xargs -n 1 cat | sed s\#$(ROUT)\#$(OUT)\#g | sed 's#$$# \\#'  >> $(MISC)/$(TARGET).$(@:b)_5.cmd
    @echo $(APP5LINKTYPEFLAG) $(APP5LIBSALCPPRT) $(APP5STDLIBS) $(APP5STDLIB) $(STDLIB5) -o $@ >> $(MISC)/$(TARGET).$(@:b)_5.cmd
  .IF "$(VERBOSE)" == "TRUE"
    @cat $(MISC)/$(TARGET).$(@:b)_5.cmd
  .ENDIF
    @+source $(MISC)/$(TARGET).$(@:b)_5.cmd
  .IF "$(VERBOSE)" == "TRUE"
    @ls -l $@
  .ENDIF
.ENDIF		# "$(OS)"=="MACOSX"
.ENDIF
.IF "$(GUI)" == "WNT"
    @@-$(MKDIR) $(@:d:d)
.IF "$(APP5LINKRES)" != ""
    @@-$(RM) $(MISC)/$(APP5LINKRES:b).rc
.IF "$(APP5ICON)" != ""
    @-echo 1 ICON $(EMQ)"$(APP5ICON:s/\/\\/)$(EMQ)" >> $(MISC)/$(APP5LINKRES:b).rc
.ENDIF		# "$(APP5ICON)" != ""
.IF "$(APP5VERINFO)" != ""
    @-echo $(EMQ)#define VERVARIANT	$(BUILD) >> $(MISC)/$(APP5LINKRES:b).rc
    @-echo $(EMQ)#include  $(EMQ)"$(APP5VERINFO)$(EMQ)" >> $(MISC)/$(APP5LINKRES:b).rc
.ENDIF		# "$(APP5VERINFO)" != ""
    $(COMMAND_ECHO)$(RC) -DWIN32 $(APP5PRODUCTDEF) -I$(SOLARRESDIR) $(INCLUDE) $(RCLINKFLAGS) $(MISC)/$(APP5LINKRES:b).rc
.ENDIF			# "$(APP5LINKRES)" != ""
.IF "$(COM)" == "GCC"
    @echo mingw
.IF "$(APP5LINKRES)" != "" || "$(APP5RES)" != ""
    @cat $(APP5LINKRES) $(subst,/res/,/res{$(subst,$(BIN), $(@:d))} $(APP5RES)) >  $(MISC)/$(@:b)_all.res
    windres $(MISC)/$(@:b)_all.res $(APP5RESO)
.ENDIF
    @echo $(LINK) $(LINKFLAGS) $(LINKFLAGSAPP) $(MINGWSSTDOBJ) -L$(PRJ)/$(INPATH)/lib $(SOLARLIB) $(STDSLO) \
        $(APP5BASEX) $(APP5STACKN) -o $@ $(APP5OBJS) \
        -Wl,-Map,$(MISC)/$(@:b).map $(STDOBJ) $(APP5RESO) \
        `$(TYPE) /dev/null $(APP5LIBS) | sed s#$(ROUT)#$(OUT)#g` \
        $(APP_LINKTYPE) $(APP5LIBSALCPPRT) \
        -Wl,--start-group $(APP5STDLIBS) -Wl,--end-group $(APP5STDLIB) \
        $(STDLIB5) $(MINGWSSTDENDOBJ) > $(MISC)/$(TARGET).$(@:b)_5.cmd
  # need this empty line, else dmake somehow gets confused by the .IFs and .ENDIFs
  .IF "$(VERBOSE)" == "TRUE"
    @$(TYPE)  $(MISC)/$(TARGET).$(@:b)_5.cmd
  .ENDIF
    @+source $(MISC)/$(TARGET).$(@:b)_5.cmd
    @ls -l $@
.ELSE	# "$(COM)" == "GCC"
.IF "$(linkinc)" == ""
    $(COMMAND_ECHO)$(APP5LINKER) @$(mktmp \
        $(APP5LINKFLAGS) \
        $(LINKFLAGSAPP) $(APP5BASEX) \
        $(APP5STACKN) \
        -out:$@ \
        -map:$(MISC)/{$(subst,/,_ $(APP5TARGET)).map} \
        $(STDOBJ) \
        $(APP5LINKRES) \
        $(APP5RES) \
        $(APP5OBJS) \
        $(APP5LIBS) \
        $(APP5STDLIBS) \
        $(APP5STDLIB) $(STDLIB5) \
        )
    @-echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.tmanifest -outputresource:$@$(EMQ);1 $(FI)
.ELSE
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -outputresource:$@$(EMQ);1 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ELSE
        $(COMMAND_ECHO)-$(RM) $(MISC)\$(APP5TARGET).lnk
        $(COMMAND_ECHO)-$(RM) $(MISC)\$(APP5TARGET).lst
        $(COMMAND_ECHO)-$(RM) $(MISC)\linkobj.lst
        for %_i in ($(MISC)\*.obj) do type %_i >> $(MISC)\linkobj.lst
    type $(mktmp,$(MISC)\$(APP5TARGET).lst
        $(APP5LINKFLAGS) \
        $(LINKFLAGSAPP) $(APP5BASEX) \
        $(APP5STACKN) \
        -out:$@ \
        $(STDOBJ) \
        $(APP5LINKRES) \
        $(APP5RES) \
        $(APP5OBJS) \
        $(APP5LIBS) \
        $(APP5STDLIBS) \
        $(APP5STDLIB) $(STDLIB5))
        $(COMMAND_ECHO)$(SED)$(SED) -e 's/\(\.\.\\\)\{2,4\}/..\\/g' $(MISC)\$(APP5TARGETN:b)_linkobj.lst >> $(MISC)\$(APP5TARGET).lst
        $(COMMAND_ECHO)$(SED)$(IFEXIST) $(MISC)/$(APP5TARGET).lst $(THEN) type $(MISC)/$(APP5TARGET).lst  >> $(MISC)/$(APP5TARGET).lnk $(FI)
        $(COMMAND_ECHO)$(SED)$(APP5LINKER) @$(MISC)\$(APP5TARGET).lnk
.ENDIF		# "$(linkinc)" == ""
.ENDIF		# "$(COM)" == "GCC"
.IF "$(APP5TARGET)" == "loader"
    $(COMMAND_ECHO)$(PERL) loader.pl $@
    $(COMMAND_ECHO)$(TYPE) $(@) $(@:d)unloader.exe > $(@:d)_new.exe
    $(COMMAND_ECHO)$(RM) $@
    $(COMMAND_ECHO)$(RENAME) $(@:d)_new.exe $(@:d)loader.exe
.ENDIF			# "$(TARGET)" == "setup"

.ENDIF			# "$(GUI)" == "WNT"

.IF "$(GUI)" == "OS2"
    @+-$(MKDIR) $(@:d:d) > $(NULLDEV)
.IF "$(APP5LINKRES)" != ""
    @+-$(RM) $(MISC)/$(APP5LINKRES:b).rc > $(NULLDEV)
.IF "$(APP5ICON)" != ""
    @-+echo ICON 1 $(EMQ)"$(APP5ICON:s#/#\\\\#)$(EMQ)" >> $(MISC)$/$(APP5LINKRES:b).rc
.ENDIF		# "$(APP5ICON)" != ""
.IF "$(APP5VERINFO)" != ""
    @-+echo $(EMQ)#define VERVARIANT	$(BUILD) >> $(MISC)/$(APP5LINKRES:b).rc
    @-+echo $(EMQ)#include  $(EMQ)"$(APP5VERINFO)$(EMQ)" >> $(MISC)/$(APP5LINKRES:b).rc
.ENDIF		# "$(APP5VERINFO)" != ""
    $(COMMAND_ECHO)$(RC) -r -DOS2 $(APP5PRODUCTDEF) -I$(SOLARRESDIR) $(INCLUDE) $(RCLINKFLAGS) $(MISC)/$(APP5LINKRES:b).rc
.ENDIF			# "$(APP5LINKRES)" != ""

.IF "$(TARGETTYPE)" == "GUI" 
    @echo NAME $(APP5TARGET) WINDOWAPI > $(MISC)/$(APP5TARGET).def
.ENDIF

    $(COMMAND_ECHO)$(APP5LINKER) -v \
        $(APP5LINKFLAGS) \
        $(LINKFLAGSAPP) $(APP5BASEX) \
        $(APP5STACKN) \
        -o $@ \
        -Zmap -L$(LB) \
        -L$(SOLARVERSION)/$(INPATH)/lib \
        $(STDOBJ) \
        $(APP5LINKRES) \
        $(APP5RES) \
        $(APP5DEF) \
        $(APP5OBJS) \
        $(APP5LIBS) \
        $(APP5STDLIBS) \
        $(APP5STDLIB) $(STDLIB5) 


.IF "$(APP5TARGET)" == "loader"
    $(COMMAND_ECHO)+$(PERL) loader.pl $@
    $(COMMAND_ECHO)+$(TYPE) $(@) $(@:d)unloader.exe > $(@:d)_new.exe
    $(COMMAND_ECHO)+$(RM) $@
    $(COMMAND_ECHO)+$(RENAME) $(@:d)_new.exe $(@:d)loader.exe
.ENDIF			# "$(TARGET)" == "setup"

.ENDIF			# "$(GUI)" == "OS2"

.ENDIF			# "$(APP5TARGETN)"!=""


# Instruction for linking
# unroll begin

.IF "$(GUI)" == "OS2" && "$(TARGETTYPE)" == "GUI" 
APP6DEF = $(MISC)/$(APP6TARGET).def
.ENDIF

.IF "$(APP6LINKTYPE)" != ""
#must be either STATIC or SHARED
APP6LINKTYPEFLAG=$(APPLINK$(APP6LINKTYPE))
.ENDIF

# decide how to link
.IF "$(APP6CODETYPE)"=="C"
APP6LINKER=$(LINKC)
APP6STDLIB=$(subst,CPPRUNTIME, $(STDLIB))
APP6LINKFLAGS+=$(LINKCFLAGS)
.ELSE			# "$(APP6CODETYPE)"=="C"
APP6LINKER=$(LINK)
APP6STDLIB=$(subst,CPPRUNTIME,$(STDLIBCPP) $(STDLIB))
APP6LINKFLAGS+=$(LINKFLAGS)
.ENDIF			# "$(APP6CODETYPE)"=="C"

APP6RPATH*=OOO
LINKFLAGSRUNPATH_$(APP6RPATH)*=/ERROR:/Bad_APP6RPATH_value
.IF "$(OS)" != "MACOSX"
APP6LINKFLAGS+=$(LINKFLAGSRUNPATH_$(APP6RPATH))
.ENDIF

.IF "$(APP6STACK)" != ""
.IF "$(LINKFLAGSTACK)" != ""
APP6STACKN=$(LINKFLAGSTACK)$(APP6STACK)
.ENDIF
.ELSE
APP6STACKN=
.ENDIF

.IF "$(APP6NOSAL)"==""
.IF "$(TARGETTYPE)" == "GUI"
APP6OBJS+= $(STDOBJVCL)
.ENDIF
.ENDIF

.IF "$(GUI)$(COM)" == "WNTGCC"
APP6RESO=
.IF "$(APP6LINKRES)" != "" || "$(APP6RES)" != ""
APP6RESO=$(MISC)/$(APP6TARGET:b)_res.o
.ENDIF
.ENDIF

.IF "$(GUI)" == "UNX"
APP6DEPN+:=$(APP6DEPNU)
USE_APP6DEF=
.ENDIF

.IF "$(APP6TARGETN)"!=""

.IF "$(APP6PRODUCTNAME)"!=""
APP6PRODUCTDEF+:=-DPRODUCT_NAME=\"$(APP6PRODUCTNAME)\"
.ENDIF			# "$(APP6PRODUCTNAME)"!=""

.IF "$(linkinc)"!=""
.IF "$(GUI)"=="WNT" || "$(GUI)"=="OS2"
.IF "$(APP6LIBS)"!=""
$(MISC)/$(APP6TARGET)_linkinc.ls .PHONY:
    @@-$(RM) $@
    sed -f $(SOLARENV)/bin/chrel.sed $(foreach,i,$(APP6LIBS) $(i:s/.lib/.lin/)) >> $@
.ENDIF          #"$(APP6LIBS)"!="" 
.ENDIF

LINKINCTARGETS+=$(MISC)/$(APP6TARGETN:b)_linkinc.ls
$(APP6TARGETN) : $(LINKINCTARGETS)
.ENDIF          # "$(linkinc)"!=""

# Allow for target specific LIBSALCPPRT override
APP6LIBSALCPPRT*=$(LIBSALCPPRT)

$(APP6TARGETN): $(APP6OBJS) $(APP6LIBS) \
    $(APP6RES) \
    $(APP6ICON) $(APP6DEPN) $(USE_APP6DEF)
    @echo "Making:   " $(@:f)
.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="MACOSX"
    @echo unx
    @-$(RM) $(MISC)/$(@:b).list
    @-$(RM) $(MISC)/$(TARGET).$(@:b)_6.cmd
    @-$(RM) $(MISC)/$(@:b).strip
    @echo $(STDSLO) $(APP6OBJS:s/.obj/.o/) \
    `cat /dev/null $(APP6LIBS) | sed s\#$(ROUT)\#$(OUT)\#g` | tr -s " " "\n" > $(MISC)/$(@:b).list
    @echo -n $(APP6LINKER) $(APP6LINKFLAGS) $(LINKFLAGSAPP) -L$(PRJ)/$(INPATH)/lib $(SOLARLIB) -o $@ \
    $(APP6LINKTYPEFLAG) $(APP6STDLIBS) $(APP6STDLIB) $(STDLIB6) -filelist $(MISC)/$(@:b).list > $(MISC)/$(TARGET).$(@:b)_6.cmd
    @$(PERL) $(SOLARENV)/bin/macosx-dylib-link-list.pl \
        `cat $(MISC)/$(TARGET).$(@:b)_6.cmd` \
        >> $(MISC)/$(TARGET).$(@:b)_6.cmd
  .IF "$(VERBOSE)" == "TRUE"
    @cat $(MISC)/$(TARGET).$(@:b)_6.cmd
  .ENDIF
    @+source $(MISC)/$(TARGET).$(@:b)_6.cmd
# Need to strip __objcInit symbol to avoid duplicate symbols when loading
# libraries at runtime
    @-$(CC_PATH)nm $@ | grep -v ' U ' | $(AWK) '{ print $$NF }' | grep -F -x '__objcInit' > $(MISC)/$(@:b).strip
    @$(CC_PATH)strip -i -R $(MISC)/$(@:b).strip -X $@
    @ls -l $@
    @$(PERL) $(SOLARENV)/bin/macosx-change-install-names.pl \
        app $(APP6RPATH) $@
.IF "$(TARGETTYPE)"=="GUI"
    @echo "Making:   " $(@:f).app
    @macosx-create-bundle $@
.ENDIF		# "$(TARGETTYPE)"=="GUI"
.ELSE		# "$(OS)"=="MACOSX"
    @echo unx
    @-$(RM) $(MISC)/$(TARGET).$(@:b)_6.cmd
    @echo $(APP6LINKER) $(APP6LINKFLAGS) $(LINKFLAGSAPP) -L$(PRJ)/$(INPATH)/lib $(SOLARLIB) $(STDSLO) \
    $(APP6OBJS:s/.obj/.o/) '\' >  $(MISC)/$(TARGET).$(@:b)_6.cmd
    @cat $(mktmp /dev/null $(APP6LIBS)) | xargs -n 1 cat | sed s\#$(ROUT)\#$(OUT)\#g | sed 's#$$# \\#'  >> $(MISC)/$(TARGET).$(@:b)_6.cmd
    @echo $(APP6LINKTYPEFLAG) $(APP6LIBSALCPPRT) $(APP6STDLIBS) $(APP6STDLIB) $(STDLIB6) -o $@ >> $(MISC)/$(TARGET).$(@:b)_6.cmd
  .IF "$(VERBOSE)" == "TRUE"
    @cat $(MISC)/$(TARGET).$(@:b)_6.cmd
  .ENDIF
    @+source $(MISC)/$(TARGET).$(@:b)_6.cmd
  .IF "$(VERBOSE)" == "TRUE"
    @ls -l $@
  .ENDIF
.ENDIF		# "$(OS)"=="MACOSX"
.ENDIF
.IF "$(GUI)" == "WNT"
    @@-$(MKDIR) $(@:d:d)
.IF "$(APP6LINKRES)" != ""
    @@-$(RM) $(MISC)/$(APP6LINKRES:b).rc
.IF "$(APP6ICON)" != ""
    @-echo 1 ICON $(EMQ)"$(APP6ICON:s/\/\\/)$(EMQ)" >> $(MISC)/$(APP6LINKRES:b).rc
.ENDIF		# "$(APP6ICON)" != ""
.IF "$(APP6VERINFO)" != ""
    @-echo $(EMQ)#define VERVARIANT	$(BUILD) >> $(MISC)/$(APP6LINKRES:b).rc
    @-echo $(EMQ)#include  $(EMQ)"$(APP6VERINFO)$(EMQ)" >> $(MISC)/$(APP6LINKRES:b).rc
.ENDIF		# "$(APP6VERINFO)" != ""
    $(COMMAND_ECHO)$(RC) -DWIN32 $(APP6PRODUCTDEF) -I$(SOLARRESDIR) $(INCLUDE) $(RCLINKFLAGS) $(MISC)/$(APP6LINKRES:b).rc
.ENDIF			# "$(APP6LINKRES)" != ""
.IF "$(COM)" == "GCC"
    @echo mingw
.IF "$(APP6LINKRES)" != "" || "$(APP6RES)" != ""
    @cat $(APP6LINKRES) $(subst,/res/,/res{$(subst,$(BIN), $(@:d))} $(APP6RES)) >  $(MISC)/$(@:b)_all.res
    windres $(MISC)/$(@:b)_all.res $(APP6RESO)
.ENDIF
    @echo $(LINK) $(LINKFLAGS) $(LINKFLAGSAPP) $(MINGWSSTDOBJ) -L$(PRJ)/$(INPATH)/lib $(SOLARLIB) $(STDSLO) \
        $(APP6BASEX) $(APP6STACKN) -o $@ $(APP6OBJS) \
        -Wl,-Map,$(MISC)/$(@:b).map $(STDOBJ) $(APP6RESO) \
        `$(TYPE) /dev/null $(APP6LIBS) | sed s#$(ROUT)#$(OUT)#g` \
        $(APP_LINKTYPE) $(APP6LIBSALCPPRT) \
        -Wl,--start-group $(APP6STDLIBS) -Wl,--end-group $(APP6STDLIB) \
        $(STDLIB6) $(MINGWSSTDENDOBJ) > $(MISC)/$(TARGET).$(@:b)_6.cmd
  # need this empty line, else dmake somehow gets confused by the .IFs and .ENDIFs
  .IF "$(VERBOSE)" == "TRUE"
    @$(TYPE)  $(MISC)/$(TARGET).$(@:b)_6.cmd
  .ENDIF
    @+source $(MISC)/$(TARGET).$(@:b)_6.cmd
    @ls -l $@
.ELSE	# "$(COM)" == "GCC"
.IF "$(linkinc)" == ""
    $(COMMAND_ECHO)$(APP6LINKER) @$(mktmp \
        $(APP6LINKFLAGS) \
        $(LINKFLAGSAPP) $(APP6BASEX) \
        $(APP6STACKN) \
        -out:$@ \
        -map:$(MISC)/{$(subst,/,_ $(APP6TARGET)).map} \
        $(STDOBJ) \
        $(APP6LINKRES) \
        $(APP6RES) \
        $(APP6OBJS) \
        $(APP6LIBS) \
        $(APP6STDLIBS) \
        $(APP6STDLIB) $(STDLIB6) \
        )
    @-echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.tmanifest -outputresource:$@$(EMQ);1 $(FI)
.ELSE
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -outputresource:$@$(EMQ);1 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ELSE
        $(COMMAND_ECHO)-$(RM) $(MISC)\$(APP6TARGET).lnk
        $(COMMAND_ECHO)-$(RM) $(MISC)\$(APP6TARGET).lst
        $(COMMAND_ECHO)-$(RM) $(MISC)\linkobj.lst
        for %_i in ($(MISC)\*.obj) do type %_i >> $(MISC)\linkobj.lst
    type $(mktmp,$(MISC)\$(APP6TARGET).lst
        $(APP6LINKFLAGS) \
        $(LINKFLAGSAPP) $(APP6BASEX) \
        $(APP6STACKN) \
        -out:$@ \
        $(STDOBJ) \
        $(APP6LINKRES) \
        $(APP6RES) \
        $(APP6OBJS) \
        $(APP6LIBS) \
        $(APP6STDLIBS) \
        $(APP6STDLIB) $(STDLIB6))
        $(COMMAND_ECHO)$(SED)$(SED) -e 's/\(\.\.\\\)\{2,4\}/..\\/g' $(MISC)\$(APP6TARGETN:b)_linkobj.lst >> $(MISC)\$(APP6TARGET).lst
        $(COMMAND_ECHO)$(SED)$(IFEXIST) $(MISC)/$(APP6TARGET).lst $(THEN) type $(MISC)/$(APP6TARGET).lst  >> $(MISC)/$(APP6TARGET).lnk $(FI)
        $(COMMAND_ECHO)$(SED)$(APP6LINKER) @$(MISC)\$(APP6TARGET).lnk
.ENDIF		# "$(linkinc)" == ""
.ENDIF		# "$(COM)" == "GCC"
.IF "$(APP6TARGET)" == "loader"
    $(COMMAND_ECHO)$(PERL) loader.pl $@
    $(COMMAND_ECHO)$(TYPE) $(@) $(@:d)unloader.exe > $(@:d)_new.exe
    $(COMMAND_ECHO)$(RM) $@
    $(COMMAND_ECHO)$(RENAME) $(@:d)_new.exe $(@:d)loader.exe
.ENDIF			# "$(TARGET)" == "setup"

.ENDIF			# "$(GUI)" == "WNT"

.IF "$(GUI)" == "OS2"
    @+-$(MKDIR) $(@:d:d) > $(NULLDEV)
.IF "$(APP6LINKRES)" != ""
    @+-$(RM) $(MISC)/$(APP6LINKRES:b).rc > $(NULLDEV)
.IF "$(APP6ICON)" != ""
    @-+echo ICON 1 $(EMQ)"$(APP6ICON:s#/#\\\\#)$(EMQ)" >> $(MISC)$/$(APP6LINKRES:b).rc
.ENDIF		# "$(APP6ICON)" != ""
.IF "$(APP6VERINFO)" != ""
    @-+echo $(EMQ)#define VERVARIANT	$(BUILD) >> $(MISC)/$(APP6LINKRES:b).rc
    @-+echo $(EMQ)#include  $(EMQ)"$(APP6VERINFO)$(EMQ)" >> $(MISC)/$(APP6LINKRES:b).rc
.ENDIF		# "$(APP6VERINFO)" != ""
    $(COMMAND_ECHO)$(RC) -r -DOS2 $(APP6PRODUCTDEF) -I$(SOLARRESDIR) $(INCLUDE) $(RCLINKFLAGS) $(MISC)/$(APP6LINKRES:b).rc
.ENDIF			# "$(APP6LINKRES)" != ""

.IF "$(TARGETTYPE)" == "GUI" 
    @echo NAME $(APP6TARGET) WINDOWAPI > $(MISC)/$(APP6TARGET).def
.ENDIF

    $(COMMAND_ECHO)$(APP6LINKER) -v \
        $(APP6LINKFLAGS) \
        $(LINKFLAGSAPP) $(APP6BASEX) \
        $(APP6STACKN) \
        -o $@ \
        -Zmap -L$(LB) \
        -L$(SOLARVERSION)/$(INPATH)/lib \
        $(STDOBJ) \
        $(APP6LINKRES) \
        $(APP6RES) \
        $(APP6DEF) \
        $(APP6OBJS) \
        $(APP6LIBS) \
        $(APP6STDLIBS) \
        $(APP6STDLIB) $(STDLIB6) 


.IF "$(APP6TARGET)" == "loader"
    $(COMMAND_ECHO)+$(PERL) loader.pl $@
    $(COMMAND_ECHO)+$(TYPE) $(@) $(@:d)unloader.exe > $(@:d)_new.exe
    $(COMMAND_ECHO)+$(RM) $@
    $(COMMAND_ECHO)+$(RENAME) $(@:d)_new.exe $(@:d)loader.exe
.ENDIF			# "$(TARGET)" == "setup"

.ENDIF			# "$(GUI)" == "OS2"

.ENDIF			# "$(APP6TARGETN)"!=""


# Instruction for linking
# unroll begin

.IF "$(GUI)" == "OS2" && "$(TARGETTYPE)" == "GUI" 
APP7DEF = $(MISC)/$(APP7TARGET).def
.ENDIF

.IF "$(APP7LINKTYPE)" != ""
#must be either STATIC or SHARED
APP7LINKTYPEFLAG=$(APPLINK$(APP7LINKTYPE))
.ENDIF

# decide how to link
.IF "$(APP7CODETYPE)"=="C"
APP7LINKER=$(LINKC)
APP7STDLIB=$(subst,CPPRUNTIME, $(STDLIB))
APP7LINKFLAGS+=$(LINKCFLAGS)
.ELSE			# "$(APP7CODETYPE)"=="C"
APP7LINKER=$(LINK)
APP7STDLIB=$(subst,CPPRUNTIME,$(STDLIBCPP) $(STDLIB))
APP7LINKFLAGS+=$(LINKFLAGS)
.ENDIF			# "$(APP7CODETYPE)"=="C"

APP7RPATH*=OOO
LINKFLAGSRUNPATH_$(APP7RPATH)*=/ERROR:/Bad_APP7RPATH_value
.IF "$(OS)" != "MACOSX"
APP7LINKFLAGS+=$(LINKFLAGSRUNPATH_$(APP7RPATH))
.ENDIF

.IF "$(APP7STACK)" != ""
.IF "$(LINKFLAGSTACK)" != ""
APP7STACKN=$(LINKFLAGSTACK)$(APP7STACK)
.ENDIF
.ELSE
APP7STACKN=
.ENDIF

.IF "$(APP7NOSAL)"==""
.IF "$(TARGETTYPE)" == "GUI"
APP7OBJS+= $(STDOBJVCL)
.ENDIF
.ENDIF

.IF "$(GUI)$(COM)" == "WNTGCC"
APP7RESO=
.IF "$(APP7LINKRES)" != "" || "$(APP7RES)" != ""
APP7RESO=$(MISC)/$(APP7TARGET:b)_res.o
.ENDIF
.ENDIF

.IF "$(GUI)" == "UNX"
APP7DEPN+:=$(APP7DEPNU)
USE_APP7DEF=
.ENDIF

.IF "$(APP7TARGETN)"!=""

.IF "$(APP7PRODUCTNAME)"!=""
APP7PRODUCTDEF+:=-DPRODUCT_NAME=\"$(APP7PRODUCTNAME)\"
.ENDIF			# "$(APP7PRODUCTNAME)"!=""

.IF "$(linkinc)"!=""
.IF "$(GUI)"=="WNT" || "$(GUI)"=="OS2"
.IF "$(APP7LIBS)"!=""
$(MISC)/$(APP7TARGET)_linkinc.ls .PHONY:
    @@-$(RM) $@
    sed -f $(SOLARENV)/bin/chrel.sed $(foreach,i,$(APP7LIBS) $(i:s/.lib/.lin/)) >> $@
.ENDIF          #"$(APP7LIBS)"!="" 
.ENDIF

LINKINCTARGETS+=$(MISC)/$(APP7TARGETN:b)_linkinc.ls
$(APP7TARGETN) : $(LINKINCTARGETS)
.ENDIF          # "$(linkinc)"!=""

# Allow for target specific LIBSALCPPRT override
APP7LIBSALCPPRT*=$(LIBSALCPPRT)

$(APP7TARGETN): $(APP7OBJS) $(APP7LIBS) \
    $(APP7RES) \
    $(APP7ICON) $(APP7DEPN) $(USE_APP7DEF)
    @echo "Making:   " $(@:f)
.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="MACOSX"
    @echo unx
    @-$(RM) $(MISC)/$(@:b).list
    @-$(RM) $(MISC)/$(TARGET).$(@:b)_7.cmd
    @-$(RM) $(MISC)/$(@:b).strip
    @echo $(STDSLO) $(APP7OBJS:s/.obj/.o/) \
    `cat /dev/null $(APP7LIBS) | sed s\#$(ROUT)\#$(OUT)\#g` | tr -s " " "\n" > $(MISC)/$(@:b).list
    @echo -n $(APP7LINKER) $(APP7LINKFLAGS) $(LINKFLAGSAPP) -L$(PRJ)/$(INPATH)/lib $(SOLARLIB) -o $@ \
    $(APP7LINKTYPEFLAG) $(APP7STDLIBS) $(APP7STDLIB) $(STDLIB7) -filelist $(MISC)/$(@:b).list > $(MISC)/$(TARGET).$(@:b)_7.cmd
    @$(PERL) $(SOLARENV)/bin/macosx-dylib-link-list.pl \
        `cat $(MISC)/$(TARGET).$(@:b)_7.cmd` \
        >> $(MISC)/$(TARGET).$(@:b)_7.cmd
  .IF "$(VERBOSE)" == "TRUE"
    @cat $(MISC)/$(TARGET).$(@:b)_7.cmd
  .ENDIF
    @+source $(MISC)/$(TARGET).$(@:b)_7.cmd
# Need to strip __objcInit symbol to avoid duplicate symbols when loading
# libraries at runtime
    @-$(CC_PATH)nm $@ | grep -v ' U ' | $(AWK) '{ print $$NF }' | grep -F -x '__objcInit' > $(MISC)/$(@:b).strip
    @$(CC_PATH)strip -i -R $(MISC)/$(@:b).strip -X $@
    @ls -l $@
    @$(PERL) $(SOLARENV)/bin/macosx-change-install-names.pl \
        app $(APP7RPATH) $@
.IF "$(TARGETTYPE)"=="GUI"
    @echo "Making:   " $(@:f).app
    @macosx-create-bundle $@
.ENDIF		# "$(TARGETTYPE)"=="GUI"
.ELSE		# "$(OS)"=="MACOSX"
    @echo unx
    @-$(RM) $(MISC)/$(TARGET).$(@:b)_7.cmd
    @echo $(APP7LINKER) $(APP7LINKFLAGS) $(LINKFLAGSAPP) -L$(PRJ)/$(INPATH)/lib $(SOLARLIB) $(STDSLO) \
    $(APP7OBJS:s/.obj/.o/) '\' >  $(MISC)/$(TARGET).$(@:b)_7.cmd
    @cat $(mktmp /dev/null $(APP7LIBS)) | xargs -n 1 cat | sed s\#$(ROUT)\#$(OUT)\#g | sed 's#$$# \\#'  >> $(MISC)/$(TARGET).$(@:b)_7.cmd
    @echo $(APP7LINKTYPEFLAG) $(APP7LIBSALCPPRT) $(APP7STDLIBS) $(APP7STDLIB) $(STDLIB7) -o $@ >> $(MISC)/$(TARGET).$(@:b)_7.cmd
  .IF "$(VERBOSE)" == "TRUE"
    @cat $(MISC)/$(TARGET).$(@:b)_7.cmd
  .ENDIF
    @+source $(MISC)/$(TARGET).$(@:b)_7.cmd
  .IF "$(VERBOSE)" == "TRUE"
    @ls -l $@
  .ENDIF
.ENDIF		# "$(OS)"=="MACOSX"
.ENDIF
.IF "$(GUI)" == "WNT"
    @@-$(MKDIR) $(@:d:d)
.IF "$(APP7LINKRES)" != ""
    @@-$(RM) $(MISC)/$(APP7LINKRES:b).rc
.IF "$(APP7ICON)" != ""
    @-echo 1 ICON $(EMQ)"$(APP7ICON:s/\/\\/)$(EMQ)" >> $(MISC)/$(APP7LINKRES:b).rc
.ENDIF		# "$(APP7ICON)" != ""
.IF "$(APP7VERINFO)" != ""
    @-echo $(EMQ)#define VERVARIANT	$(BUILD) >> $(MISC)/$(APP7LINKRES:b).rc
    @-echo $(EMQ)#include  $(EMQ)"$(APP7VERINFO)$(EMQ)" >> $(MISC)/$(APP7LINKRES:b).rc
.ENDIF		# "$(APP7VERINFO)" != ""
    $(COMMAND_ECHO)$(RC) -DWIN32 $(APP7PRODUCTDEF) -I$(SOLARRESDIR) $(INCLUDE) $(RCLINKFLAGS) $(MISC)/$(APP7LINKRES:b).rc
.ENDIF			# "$(APP7LINKRES)" != ""
.IF "$(COM)" == "GCC"
    @echo mingw
.IF "$(APP7LINKRES)" != "" || "$(APP7RES)" != ""
    @cat $(APP7LINKRES) $(subst,/res/,/res{$(subst,$(BIN), $(@:d))} $(APP7RES)) >  $(MISC)/$(@:b)_all.res
    windres $(MISC)/$(@:b)_all.res $(APP7RESO)
.ENDIF
    @echo $(LINK) $(LINKFLAGS) $(LINKFLAGSAPP) $(MINGWSSTDOBJ) -L$(PRJ)/$(INPATH)/lib $(SOLARLIB) $(STDSLO) \
        $(APP7BASEX) $(APP7STACKN) -o $@ $(APP7OBJS) \
        -Wl,-Map,$(MISC)/$(@:b).map $(STDOBJ) $(APP7RESO) \
        `$(TYPE) /dev/null $(APP7LIBS) | sed s#$(ROUT)#$(OUT)#g` \
        $(APP_LINKTYPE) $(APP7LIBSALCPPRT) \
        -Wl,--start-group $(APP7STDLIBS) -Wl,--end-group $(APP7STDLIB) \
        $(STDLIB7) $(MINGWSSTDENDOBJ) > $(MISC)/$(TARGET).$(@:b)_7.cmd
  # need this empty line, else dmake somehow gets confused by the .IFs and .ENDIFs
  .IF "$(VERBOSE)" == "TRUE"
    @$(TYPE)  $(MISC)/$(TARGET).$(@:b)_7.cmd
  .ENDIF
    @+source $(MISC)/$(TARGET).$(@:b)_7.cmd
    @ls -l $@
.ELSE	# "$(COM)" == "GCC"
.IF "$(linkinc)" == ""
    $(COMMAND_ECHO)$(APP7LINKER) @$(mktmp \
        $(APP7LINKFLAGS) \
        $(LINKFLAGSAPP) $(APP7BASEX) \
        $(APP7STACKN) \
        -out:$@ \
        -map:$(MISC)/{$(subst,/,_ $(APP7TARGET)).map} \
        $(STDOBJ) \
        $(APP7LINKRES) \
        $(APP7RES) \
        $(APP7OBJS) \
        $(APP7LIBS) \
        $(APP7STDLIBS) \
        $(APP7STDLIB) $(STDLIB7) \
        )
    @-echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.tmanifest -outputresource:$@$(EMQ);1 $(FI)
.ELSE
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -outputresource:$@$(EMQ);1 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ELSE
        $(COMMAND_ECHO)-$(RM) $(MISC)\$(APP7TARGET).lnk
        $(COMMAND_ECHO)-$(RM) $(MISC)\$(APP7TARGET).lst
        $(COMMAND_ECHO)-$(RM) $(MISC)\linkobj.lst
        for %_i in ($(MISC)\*.obj) do type %_i >> $(MISC)\linkobj.lst
    type $(mktmp,$(MISC)\$(APP7TARGET).lst
        $(APP7LINKFLAGS) \
        $(LINKFLAGSAPP) $(APP7BASEX) \
        $(APP7STACKN) \
        -out:$@ \
        $(STDOBJ) \
        $(APP7LINKRES) \
        $(APP7RES) \
        $(APP7OBJS) \
        $(APP7LIBS) \
        $(APP7STDLIBS) \
        $(APP7STDLIB) $(STDLIB7))
        $(COMMAND_ECHO)$(SED)$(SED) -e 's/\(\.\.\\\)\{2,4\}/..\\/g' $(MISC)\$(APP7TARGETN:b)_linkobj.lst >> $(MISC)\$(APP7TARGET).lst
        $(COMMAND_ECHO)$(SED)$(IFEXIST) $(MISC)/$(APP7TARGET).lst $(THEN) type $(MISC)/$(APP7TARGET).lst  >> $(MISC)/$(APP7TARGET).lnk $(FI)
        $(COMMAND_ECHO)$(SED)$(APP7LINKER) @$(MISC)\$(APP7TARGET).lnk
.ENDIF		# "$(linkinc)" == ""
.ENDIF		# "$(COM)" == "GCC"
.IF "$(APP7TARGET)" == "loader"
    $(COMMAND_ECHO)$(PERL) loader.pl $@
    $(COMMAND_ECHO)$(TYPE) $(@) $(@:d)unloader.exe > $(@:d)_new.exe
    $(COMMAND_ECHO)$(RM) $@
    $(COMMAND_ECHO)$(RENAME) $(@:d)_new.exe $(@:d)loader.exe
.ENDIF			# "$(TARGET)" == "setup"

.ENDIF			# "$(GUI)" == "WNT"

.IF "$(GUI)" == "OS2"
    @+-$(MKDIR) $(@:d:d) > $(NULLDEV)
.IF "$(APP7LINKRES)" != ""
    @+-$(RM) $(MISC)/$(APP7LINKRES:b).rc > $(NULLDEV)
.IF "$(APP7ICON)" != ""
    @-+echo ICON 1 $(EMQ)"$(APP7ICON:s#/#\\\\#)$(EMQ)" >> $(MISC)$/$(APP7LINKRES:b).rc
.ENDIF		# "$(APP7ICON)" != ""
.IF "$(APP7VERINFO)" != ""
    @-+echo $(EMQ)#define VERVARIANT	$(BUILD) >> $(MISC)/$(APP7LINKRES:b).rc
    @-+echo $(EMQ)#include  $(EMQ)"$(APP7VERINFO)$(EMQ)" >> $(MISC)/$(APP7LINKRES:b).rc
.ENDIF		# "$(APP7VERINFO)" != ""
    $(COMMAND_ECHO)$(RC) -r -DOS2 $(APP7PRODUCTDEF) -I$(SOLARRESDIR) $(INCLUDE) $(RCLINKFLAGS) $(MISC)/$(APP7LINKRES:b).rc
.ENDIF			# "$(APP7LINKRES)" != ""

.IF "$(TARGETTYPE)" == "GUI" 
    @echo NAME $(APP7TARGET) WINDOWAPI > $(MISC)/$(APP7TARGET).def
.ENDIF

    $(COMMAND_ECHO)$(APP7LINKER) -v \
        $(APP7LINKFLAGS) \
        $(LINKFLAGSAPP) $(APP7BASEX) \
        $(APP7STACKN) \
        -o $@ \
        -Zmap -L$(LB) \
        -L$(SOLARVERSION)/$(INPATH)/lib \
        $(STDOBJ) \
        $(APP7LINKRES) \
        $(APP7RES) \
        $(APP7DEF) \
        $(APP7OBJS) \
        $(APP7LIBS) \
        $(APP7STDLIBS) \
        $(APP7STDLIB) $(STDLIB7) 


.IF "$(APP7TARGET)" == "loader"
    $(COMMAND_ECHO)+$(PERL) loader.pl $@
    $(COMMAND_ECHO)+$(TYPE) $(@) $(@:d)unloader.exe > $(@:d)_new.exe
    $(COMMAND_ECHO)+$(RM) $@
    $(COMMAND_ECHO)+$(RENAME) $(@:d)_new.exe $(@:d)loader.exe
.ENDIF			# "$(TARGET)" == "setup"

.ENDIF			# "$(GUI)" == "OS2"

.ENDIF			# "$(APP7TARGETN)"!=""


# Instruction for linking
# unroll begin

.IF "$(GUI)" == "OS2" && "$(TARGETTYPE)" == "GUI" 
APP8DEF = $(MISC)/$(APP8TARGET).def
.ENDIF

.IF "$(APP8LINKTYPE)" != ""
#must be either STATIC or SHARED
APP8LINKTYPEFLAG=$(APPLINK$(APP8LINKTYPE))
.ENDIF

# decide how to link
.IF "$(APP8CODETYPE)"=="C"
APP8LINKER=$(LINKC)
APP8STDLIB=$(subst,CPPRUNTIME, $(STDLIB))
APP8LINKFLAGS+=$(LINKCFLAGS)
.ELSE			# "$(APP8CODETYPE)"=="C"
APP8LINKER=$(LINK)
APP8STDLIB=$(subst,CPPRUNTIME,$(STDLIBCPP) $(STDLIB))
APP8LINKFLAGS+=$(LINKFLAGS)
.ENDIF			# "$(APP8CODETYPE)"=="C"

APP8RPATH*=OOO
LINKFLAGSRUNPATH_$(APP8RPATH)*=/ERROR:/Bad_APP8RPATH_value
.IF "$(OS)" != "MACOSX"
APP8LINKFLAGS+=$(LINKFLAGSRUNPATH_$(APP8RPATH))
.ENDIF

.IF "$(APP8STACK)" != ""
.IF "$(LINKFLAGSTACK)" != ""
APP8STACKN=$(LINKFLAGSTACK)$(APP8STACK)
.ENDIF
.ELSE
APP8STACKN=
.ENDIF

.IF "$(APP8NOSAL)"==""
.IF "$(TARGETTYPE)" == "GUI"
APP8OBJS+= $(STDOBJVCL)
.ENDIF
.ENDIF

.IF "$(GUI)$(COM)" == "WNTGCC"
APP8RESO=
.IF "$(APP8LINKRES)" != "" || "$(APP8RES)" != ""
APP8RESO=$(MISC)/$(APP8TARGET:b)_res.o
.ENDIF
.ENDIF

.IF "$(GUI)" == "UNX"
APP8DEPN+:=$(APP8DEPNU)
USE_APP8DEF=
.ENDIF

.IF "$(APP8TARGETN)"!=""

.IF "$(APP8PRODUCTNAME)"!=""
APP8PRODUCTDEF+:=-DPRODUCT_NAME=\"$(APP8PRODUCTNAME)\"
.ENDIF			# "$(APP8PRODUCTNAME)"!=""

.IF "$(linkinc)"!=""
.IF "$(GUI)"=="WNT" || "$(GUI)"=="OS2"
.IF "$(APP8LIBS)"!=""
$(MISC)/$(APP8TARGET)_linkinc.ls .PHONY:
    @@-$(RM) $@
    sed -f $(SOLARENV)/bin/chrel.sed $(foreach,i,$(APP8LIBS) $(i:s/.lib/.lin/)) >> $@
.ENDIF          #"$(APP8LIBS)"!="" 
.ENDIF

LINKINCTARGETS+=$(MISC)/$(APP8TARGETN:b)_linkinc.ls
$(APP8TARGETN) : $(LINKINCTARGETS)
.ENDIF          # "$(linkinc)"!=""

# Allow for target specific LIBSALCPPRT override
APP8LIBSALCPPRT*=$(LIBSALCPPRT)

$(APP8TARGETN): $(APP8OBJS) $(APP8LIBS) \
    $(APP8RES) \
    $(APP8ICON) $(APP8DEPN) $(USE_APP8DEF)
    @echo "Making:   " $(@:f)
.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="MACOSX"
    @echo unx
    @-$(RM) $(MISC)/$(@:b).list
    @-$(RM) $(MISC)/$(TARGET).$(@:b)_8.cmd
    @-$(RM) $(MISC)/$(@:b).strip
    @echo $(STDSLO) $(APP8OBJS:s/.obj/.o/) \
    `cat /dev/null $(APP8LIBS) | sed s\#$(ROUT)\#$(OUT)\#g` | tr -s " " "\n" > $(MISC)/$(@:b).list
    @echo -n $(APP8LINKER) $(APP8LINKFLAGS) $(LINKFLAGSAPP) -L$(PRJ)/$(INPATH)/lib $(SOLARLIB) -o $@ \
    $(APP8LINKTYPEFLAG) $(APP8STDLIBS) $(APP8STDLIB) $(STDLIB8) -filelist $(MISC)/$(@:b).list > $(MISC)/$(TARGET).$(@:b)_8.cmd
    @$(PERL) $(SOLARENV)/bin/macosx-dylib-link-list.pl \
        `cat $(MISC)/$(TARGET).$(@:b)_8.cmd` \
        >> $(MISC)/$(TARGET).$(@:b)_8.cmd
  .IF "$(VERBOSE)" == "TRUE"
    @cat $(MISC)/$(TARGET).$(@:b)_8.cmd
  .ENDIF
    @+source $(MISC)/$(TARGET).$(@:b)_8.cmd
# Need to strip __objcInit symbol to avoid duplicate symbols when loading
# libraries at runtime
    @-$(CC_PATH)nm $@ | grep -v ' U ' | $(AWK) '{ print $$NF }' | grep -F -x '__objcInit' > $(MISC)/$(@:b).strip
    @$(CC_PATH)strip -i -R $(MISC)/$(@:b).strip -X $@
    @ls -l $@
    @$(PERL) $(SOLARENV)/bin/macosx-change-install-names.pl \
        app $(APP8RPATH) $@
.IF "$(TARGETTYPE)"=="GUI"
    @echo "Making:   " $(@:f).app
    @macosx-create-bundle $@
.ENDIF		# "$(TARGETTYPE)"=="GUI"
.ELSE		# "$(OS)"=="MACOSX"
    @echo unx
    @-$(RM) $(MISC)/$(TARGET).$(@:b)_8.cmd
    @echo $(APP8LINKER) $(APP8LINKFLAGS) $(LINKFLAGSAPP) -L$(PRJ)/$(INPATH)/lib $(SOLARLIB) $(STDSLO) \
    $(APP8OBJS:s/.obj/.o/) '\' >  $(MISC)/$(TARGET).$(@:b)_8.cmd
    @cat $(mktmp /dev/null $(APP8LIBS)) | xargs -n 1 cat | sed s\#$(ROUT)\#$(OUT)\#g | sed 's#$$# \\#'  >> $(MISC)/$(TARGET).$(@:b)_8.cmd
    @echo $(APP8LINKTYPEFLAG) $(APP8LIBSALCPPRT) $(APP8STDLIBS) $(APP8STDLIB) $(STDLIB8) -o $@ >> $(MISC)/$(TARGET).$(@:b)_8.cmd
  .IF "$(VERBOSE)" == "TRUE"
    @cat $(MISC)/$(TARGET).$(@:b)_8.cmd
  .ENDIF
    @+source $(MISC)/$(TARGET).$(@:b)_8.cmd
  .IF "$(VERBOSE)" == "TRUE"
    @ls -l $@
  .ENDIF
.ENDIF		# "$(OS)"=="MACOSX"
.ENDIF
.IF "$(GUI)" == "WNT"
    @@-$(MKDIR) $(@:d:d)
.IF "$(APP8LINKRES)" != ""
    @@-$(RM) $(MISC)/$(APP8LINKRES:b).rc
.IF "$(APP8ICON)" != ""
    @-echo 1 ICON $(EMQ)"$(APP8ICON:s/\/\\/)$(EMQ)" >> $(MISC)/$(APP8LINKRES:b).rc
.ENDIF		# "$(APP8ICON)" != ""
.IF "$(APP8VERINFO)" != ""
    @-echo $(EMQ)#define VERVARIANT	$(BUILD) >> $(MISC)/$(APP8LINKRES:b).rc
    @-echo $(EMQ)#include  $(EMQ)"$(APP8VERINFO)$(EMQ)" >> $(MISC)/$(APP8LINKRES:b).rc
.ENDIF		# "$(APP8VERINFO)" != ""
    $(COMMAND_ECHO)$(RC) -DWIN32 $(APP8PRODUCTDEF) -I$(SOLARRESDIR) $(INCLUDE) $(RCLINKFLAGS) $(MISC)/$(APP8LINKRES:b).rc
.ENDIF			# "$(APP8LINKRES)" != ""
.IF "$(COM)" == "GCC"
    @echo mingw
.IF "$(APP8LINKRES)" != "" || "$(APP8RES)" != ""
    @cat $(APP8LINKRES) $(subst,/res/,/res{$(subst,$(BIN), $(@:d))} $(APP8RES)) >  $(MISC)/$(@:b)_all.res
    windres $(MISC)/$(@:b)_all.res $(APP8RESO)
.ENDIF
    @echo $(LINK) $(LINKFLAGS) $(LINKFLAGSAPP) $(MINGWSSTDOBJ) -L$(PRJ)/$(INPATH)/lib $(SOLARLIB) $(STDSLO) \
        $(APP8BASEX) $(APP8STACKN) -o $@ $(APP8OBJS) \
        -Wl,-Map,$(MISC)/$(@:b).map $(STDOBJ) $(APP8RESO) \
        `$(TYPE) /dev/null $(APP8LIBS) | sed s#$(ROUT)#$(OUT)#g` \
        $(APP_LINKTYPE) $(APP8LIBSALCPPRT) \
        -Wl,--start-group $(APP8STDLIBS) -Wl,--end-group $(APP8STDLIB) \
        $(STDLIB8) $(MINGWSSTDENDOBJ) > $(MISC)/$(TARGET).$(@:b)_8.cmd
  # need this empty line, else dmake somehow gets confused by the .IFs and .ENDIFs
  .IF "$(VERBOSE)" == "TRUE"
    @$(TYPE)  $(MISC)/$(TARGET).$(@:b)_8.cmd
  .ENDIF
    @+source $(MISC)/$(TARGET).$(@:b)_8.cmd
    @ls -l $@
.ELSE	# "$(COM)" == "GCC"
.IF "$(linkinc)" == ""
    $(COMMAND_ECHO)$(APP8LINKER) @$(mktmp \
        $(APP8LINKFLAGS) \
        $(LINKFLAGSAPP) $(APP8BASEX) \
        $(APP8STACKN) \
        -out:$@ \
        -map:$(MISC)/{$(subst,/,_ $(APP8TARGET)).map} \
        $(STDOBJ) \
        $(APP8LINKRES) \
        $(APP8RES) \
        $(APP8OBJS) \
        $(APP8LIBS) \
        $(APP8STDLIBS) \
        $(APP8STDLIB) $(STDLIB8) \
        )
    @-echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.tmanifest -outputresource:$@$(EMQ);1 $(FI)
.ELSE
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -outputresource:$@$(EMQ);1 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ELSE
        $(COMMAND_ECHO)-$(RM) $(MISC)\$(APP8TARGET).lnk
        $(COMMAND_ECHO)-$(RM) $(MISC)\$(APP8TARGET).lst
        $(COMMAND_ECHO)-$(RM) $(MISC)\linkobj.lst
        for %_i in ($(MISC)\*.obj) do type %_i >> $(MISC)\linkobj.lst
    type $(mktmp,$(MISC)\$(APP8TARGET).lst
        $(APP8LINKFLAGS) \
        $(LINKFLAGSAPP) $(APP8BASEX) \
        $(APP8STACKN) \
        -out:$@ \
        $(STDOBJ) \
        $(APP8LINKRES) \
        $(APP8RES) \
        $(APP8OBJS) \
        $(APP8LIBS) \
        $(APP8STDLIBS) \
        $(APP8STDLIB) $(STDLIB8))
        $(COMMAND_ECHO)$(SED)$(SED) -e 's/\(\.\.\\\)\{2,4\}/..\\/g' $(MISC)\$(APP8TARGETN:b)_linkobj.lst >> $(MISC)\$(APP8TARGET).lst
        $(COMMAND_ECHO)$(SED)$(IFEXIST) $(MISC)/$(APP8TARGET).lst $(THEN) type $(MISC)/$(APP8TARGET).lst  >> $(MISC)/$(APP8TARGET).lnk $(FI)
        $(COMMAND_ECHO)$(SED)$(APP8LINKER) @$(MISC)\$(APP8TARGET).lnk
.ENDIF		# "$(linkinc)" == ""
.ENDIF		# "$(COM)" == "GCC"
.IF "$(APP8TARGET)" == "loader"
    $(COMMAND_ECHO)$(PERL) loader.pl $@
    $(COMMAND_ECHO)$(TYPE) $(@) $(@:d)unloader.exe > $(@:d)_new.exe
    $(COMMAND_ECHO)$(RM) $@
    $(COMMAND_ECHO)$(RENAME) $(@:d)_new.exe $(@:d)loader.exe
.ENDIF			# "$(TARGET)" == "setup"

.ENDIF			# "$(GUI)" == "WNT"

.IF "$(GUI)" == "OS2"
    @+-$(MKDIR) $(@:d:d) > $(NULLDEV)
.IF "$(APP8LINKRES)" != ""
    @+-$(RM) $(MISC)/$(APP8LINKRES:b).rc > $(NULLDEV)
.IF "$(APP8ICON)" != ""
    @-+echo ICON 1 $(EMQ)"$(APP8ICON:s#/#\\\\#)$(EMQ)" >> $(MISC)$/$(APP8LINKRES:b).rc
.ENDIF		# "$(APP8ICON)" != ""
.IF "$(APP8VERINFO)" != ""
    @-+echo $(EMQ)#define VERVARIANT	$(BUILD) >> $(MISC)/$(APP8LINKRES:b).rc
    @-+echo $(EMQ)#include  $(EMQ)"$(APP8VERINFO)$(EMQ)" >> $(MISC)/$(APP8LINKRES:b).rc
.ENDIF		# "$(APP8VERINFO)" != ""
    $(COMMAND_ECHO)$(RC) -r -DOS2 $(APP8PRODUCTDEF) -I$(SOLARRESDIR) $(INCLUDE) $(RCLINKFLAGS) $(MISC)/$(APP8LINKRES:b).rc
.ENDIF			# "$(APP8LINKRES)" != ""

.IF "$(TARGETTYPE)" == "GUI" 
    @echo NAME $(APP8TARGET) WINDOWAPI > $(MISC)/$(APP8TARGET).def
.ENDIF

    $(COMMAND_ECHO)$(APP8LINKER) -v \
        $(APP8LINKFLAGS) \
        $(LINKFLAGSAPP) $(APP8BASEX) \
        $(APP8STACKN) \
        -o $@ \
        -Zmap -L$(LB) \
        -L$(SOLARVERSION)/$(INPATH)/lib \
        $(STDOBJ) \
        $(APP8LINKRES) \
        $(APP8RES) \
        $(APP8DEF) \
        $(APP8OBJS) \
        $(APP8LIBS) \
        $(APP8STDLIBS) \
        $(APP8STDLIB) $(STDLIB8) 


.IF "$(APP8TARGET)" == "loader"
    $(COMMAND_ECHO)+$(PERL) loader.pl $@
    $(COMMAND_ECHO)+$(TYPE) $(@) $(@:d)unloader.exe > $(@:d)_new.exe
    $(COMMAND_ECHO)+$(RM) $@
    $(COMMAND_ECHO)+$(RENAME) $(@:d)_new.exe $(@:d)loader.exe
.ENDIF			# "$(TARGET)" == "setup"

.ENDIF			# "$(GUI)" == "OS2"

.ENDIF			# "$(APP8TARGETN)"!=""


# Instruction for linking
# unroll begin

.IF "$(GUI)" == "OS2" && "$(TARGETTYPE)" == "GUI" 
APP9DEF = $(MISC)/$(APP9TARGET).def
.ENDIF

.IF "$(APP9LINKTYPE)" != ""
#must be either STATIC or SHARED
APP9LINKTYPEFLAG=$(APPLINK$(APP9LINKTYPE))
.ENDIF

# decide how to link
.IF "$(APP9CODETYPE)"=="C"
APP9LINKER=$(LINKC)
APP9STDLIB=$(subst,CPPRUNTIME, $(STDLIB))
APP9LINKFLAGS+=$(LINKCFLAGS)
.ELSE			# "$(APP9CODETYPE)"=="C"
APP9LINKER=$(LINK)
APP9STDLIB=$(subst,CPPRUNTIME,$(STDLIBCPP) $(STDLIB))
APP9LINKFLAGS+=$(LINKFLAGS)
.ENDIF			# "$(APP9CODETYPE)"=="C"

APP9RPATH*=OOO
LINKFLAGSRUNPATH_$(APP9RPATH)*=/ERROR:/Bad_APP9RPATH_value
.IF "$(OS)" != "MACOSX"
APP9LINKFLAGS+=$(LINKFLAGSRUNPATH_$(APP9RPATH))
.ENDIF

.IF "$(APP9STACK)" != ""
.IF "$(LINKFLAGSTACK)" != ""
APP9STACKN=$(LINKFLAGSTACK)$(APP9STACK)
.ENDIF
.ELSE
APP9STACKN=
.ENDIF

.IF "$(APP9NOSAL)"==""
.IF "$(TARGETTYPE)" == "GUI"
APP9OBJS+= $(STDOBJVCL)
.ENDIF
.ENDIF

.IF "$(GUI)$(COM)" == "WNTGCC"
APP9RESO=
.IF "$(APP9LINKRES)" != "" || "$(APP9RES)" != ""
APP9RESO=$(MISC)/$(APP9TARGET:b)_res.o
.ENDIF
.ENDIF

.IF "$(GUI)" == "UNX"
APP9DEPN+:=$(APP9DEPNU)
USE_APP9DEF=
.ENDIF

.IF "$(APP9TARGETN)"!=""

.IF "$(APP9PRODUCTNAME)"!=""
APP9PRODUCTDEF+:=-DPRODUCT_NAME=\"$(APP9PRODUCTNAME)\"
.ENDIF			# "$(APP9PRODUCTNAME)"!=""

.IF "$(linkinc)"!=""
.IF "$(GUI)"=="WNT" || "$(GUI)"=="OS2"
.IF "$(APP9LIBS)"!=""
$(MISC)/$(APP9TARGET)_linkinc.ls .PHONY:
    @@-$(RM) $@
    sed -f $(SOLARENV)/bin/chrel.sed $(foreach,i,$(APP9LIBS) $(i:s/.lib/.lin/)) >> $@
.ENDIF          #"$(APP9LIBS)"!="" 
.ENDIF

LINKINCTARGETS+=$(MISC)/$(APP9TARGETN:b)_linkinc.ls
$(APP9TARGETN) : $(LINKINCTARGETS)
.ENDIF          # "$(linkinc)"!=""

# Allow for target specific LIBSALCPPRT override
APP9LIBSALCPPRT*=$(LIBSALCPPRT)

$(APP9TARGETN): $(APP9OBJS) $(APP9LIBS) \
    $(APP9RES) \
    $(APP9ICON) $(APP9DEPN) $(USE_APP9DEF)
    @echo "Making:   " $(@:f)
.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="MACOSX"
    @echo unx
    @-$(RM) $(MISC)/$(@:b).list
    @-$(RM) $(MISC)/$(TARGET).$(@:b)_9.cmd
    @-$(RM) $(MISC)/$(@:b).strip
    @echo $(STDSLO) $(APP9OBJS:s/.obj/.o/) \
    `cat /dev/null $(APP9LIBS) | sed s\#$(ROUT)\#$(OUT)\#g` | tr -s " " "\n" > $(MISC)/$(@:b).list
    @echo -n $(APP9LINKER) $(APP9LINKFLAGS) $(LINKFLAGSAPP) -L$(PRJ)/$(INPATH)/lib $(SOLARLIB) -o $@ \
    $(APP9LINKTYPEFLAG) $(APP9STDLIBS) $(APP9STDLIB) $(STDLIB9) -filelist $(MISC)/$(@:b).list > $(MISC)/$(TARGET).$(@:b)_9.cmd
    @$(PERL) $(SOLARENV)/bin/macosx-dylib-link-list.pl \
        `cat $(MISC)/$(TARGET).$(@:b)_9.cmd` \
        >> $(MISC)/$(TARGET).$(@:b)_9.cmd
  .IF "$(VERBOSE)" == "TRUE"
    @cat $(MISC)/$(TARGET).$(@:b)_9.cmd
  .ENDIF
    @+source $(MISC)/$(TARGET).$(@:b)_9.cmd
# Need to strip __objcInit symbol to avoid duplicate symbols when loading
# libraries at runtime
    @-$(CC_PATH)nm $@ | grep -v ' U ' | $(AWK) '{ print $$NF }' | grep -F -x '__objcInit' > $(MISC)/$(@:b).strip
    @$(CC_PATH)strip -i -R $(MISC)/$(@:b).strip -X $@
    @ls -l $@
    @$(PERL) $(SOLARENV)/bin/macosx-change-install-names.pl \
        app $(APP9RPATH) $@
.IF "$(TARGETTYPE)"=="GUI"
    @echo "Making:   " $(@:f).app
    @macosx-create-bundle $@
.ENDIF		# "$(TARGETTYPE)"=="GUI"
.ELSE		# "$(OS)"=="MACOSX"
    @echo unx
    @-$(RM) $(MISC)/$(TARGET).$(@:b)_9.cmd
    @echo $(APP9LINKER) $(APP9LINKFLAGS) $(LINKFLAGSAPP) -L$(PRJ)/$(INPATH)/lib $(SOLARLIB) $(STDSLO) \
    $(APP9OBJS:s/.obj/.o/) '\' >  $(MISC)/$(TARGET).$(@:b)_9.cmd
    @cat $(mktmp /dev/null $(APP9LIBS)) | xargs -n 1 cat | sed s\#$(ROUT)\#$(OUT)\#g | sed 's#$$# \\#'  >> $(MISC)/$(TARGET).$(@:b)_9.cmd
    @echo $(APP9LINKTYPEFLAG) $(APP9LIBSALCPPRT) $(APP9STDLIBS) $(APP9STDLIB) $(STDLIB9) -o $@ >> $(MISC)/$(TARGET).$(@:b)_9.cmd
  .IF "$(VERBOSE)" == "TRUE"
    @cat $(MISC)/$(TARGET).$(@:b)_9.cmd
  .ENDIF
    @+source $(MISC)/$(TARGET).$(@:b)_9.cmd
  .IF "$(VERBOSE)" == "TRUE"
    @ls -l $@
  .ENDIF
.ENDIF		# "$(OS)"=="MACOSX"
.ENDIF
.IF "$(GUI)" == "WNT"
    @@-$(MKDIR) $(@:d:d)
.IF "$(APP9LINKRES)" != ""
    @@-$(RM) $(MISC)/$(APP9LINKRES:b).rc
.IF "$(APP9ICON)" != ""
    @-echo 1 ICON $(EMQ)"$(APP9ICON:s/\/\\/)$(EMQ)" >> $(MISC)/$(APP9LINKRES:b).rc
.ENDIF		# "$(APP9ICON)" != ""
.IF "$(APP9VERINFO)" != ""
    @-echo $(EMQ)#define VERVARIANT	$(BUILD) >> $(MISC)/$(APP9LINKRES:b).rc
    @-echo $(EMQ)#include  $(EMQ)"$(APP9VERINFO)$(EMQ)" >> $(MISC)/$(APP9LINKRES:b).rc
.ENDIF		# "$(APP9VERINFO)" != ""
    $(COMMAND_ECHO)$(RC) -DWIN32 $(APP9PRODUCTDEF) -I$(SOLARRESDIR) $(INCLUDE) $(RCLINKFLAGS) $(MISC)/$(APP9LINKRES:b).rc
.ENDIF			# "$(APP9LINKRES)" != ""
.IF "$(COM)" == "GCC"
    @echo mingw
.IF "$(APP9LINKRES)" != "" || "$(APP9RES)" != ""
    @cat $(APP9LINKRES) $(subst,/res/,/res{$(subst,$(BIN), $(@:d))} $(APP9RES)) >  $(MISC)/$(@:b)_all.res
    windres $(MISC)/$(@:b)_all.res $(APP9RESO)
.ENDIF
    @echo $(LINK) $(LINKFLAGS) $(LINKFLAGSAPP) $(MINGWSSTDOBJ) -L$(PRJ)/$(INPATH)/lib $(SOLARLIB) $(STDSLO) \
        $(APP9BASEX) $(APP9STACKN) -o $@ $(APP9OBJS) \
        -Wl,-Map,$(MISC)/$(@:b).map $(STDOBJ) $(APP9RESO) \
        `$(TYPE) /dev/null $(APP9LIBS) | sed s#$(ROUT)#$(OUT)#g` \
        $(APP_LINKTYPE) $(APP9LIBSALCPPRT) \
        -Wl,--start-group $(APP9STDLIBS) -Wl,--end-group $(APP9STDLIB) \
        $(STDLIB9) $(MINGWSSTDENDOBJ) > $(MISC)/$(TARGET).$(@:b)_9.cmd
  # need this empty line, else dmake somehow gets confused by the .IFs and .ENDIFs
  .IF "$(VERBOSE)" == "TRUE"
    @$(TYPE)  $(MISC)/$(TARGET).$(@:b)_9.cmd
  .ENDIF
    @+source $(MISC)/$(TARGET).$(@:b)_9.cmd
    @ls -l $@
.ELSE	# "$(COM)" == "GCC"
.IF "$(linkinc)" == ""
    $(COMMAND_ECHO)$(APP9LINKER) @$(mktmp \
        $(APP9LINKFLAGS) \
        $(LINKFLAGSAPP) $(APP9BASEX) \
        $(APP9STACKN) \
        -out:$@ \
        -map:$(MISC)/{$(subst,/,_ $(APP9TARGET)).map} \
        $(STDOBJ) \
        $(APP9LINKRES) \
        $(APP9RES) \
        $(APP9OBJS) \
        $(APP9LIBS) \
        $(APP9STDLIBS) \
        $(APP9STDLIB) $(STDLIB9) \
        )
    @-echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.tmanifest -outputresource:$@$(EMQ);1 $(FI)
.ELSE
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -outputresource:$@$(EMQ);1 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ELSE
        $(COMMAND_ECHO)-$(RM) $(MISC)\$(APP9TARGET).lnk
        $(COMMAND_ECHO)-$(RM) $(MISC)\$(APP9TARGET).lst
        $(COMMAND_ECHO)-$(RM) $(MISC)\linkobj.lst
        for %_i in ($(MISC)\*.obj) do type %_i >> $(MISC)\linkobj.lst
    type $(mktmp,$(MISC)\$(APP9TARGET).lst
        $(APP9LINKFLAGS) \
        $(LINKFLAGSAPP) $(APP9BASEX) \
        $(APP9STACKN) \
        -out:$@ \
        $(STDOBJ) \
        $(APP9LINKRES) \
        $(APP9RES) \
        $(APP9OBJS) \
        $(APP9LIBS) \
        $(APP9STDLIBS) \
        $(APP9STDLIB) $(STDLIB9))
        $(COMMAND_ECHO)$(SED)$(SED) -e 's/\(\.\.\\\)\{2,4\}/..\\/g' $(MISC)\$(APP9TARGETN:b)_linkobj.lst >> $(MISC)\$(APP9TARGET).lst
        $(COMMAND_ECHO)$(SED)$(IFEXIST) $(MISC)/$(APP9TARGET).lst $(THEN) type $(MISC)/$(APP9TARGET).lst  >> $(MISC)/$(APP9TARGET).lnk $(FI)
        $(COMMAND_ECHO)$(SED)$(APP9LINKER) @$(MISC)\$(APP9TARGET).lnk
.ENDIF		# "$(linkinc)" == ""
.ENDIF		# "$(COM)" == "GCC"
.IF "$(APP9TARGET)" == "loader"
    $(COMMAND_ECHO)$(PERL) loader.pl $@
    $(COMMAND_ECHO)$(TYPE) $(@) $(@:d)unloader.exe > $(@:d)_new.exe
    $(COMMAND_ECHO)$(RM) $@
    $(COMMAND_ECHO)$(RENAME) $(@:d)_new.exe $(@:d)loader.exe
.ENDIF			# "$(TARGET)" == "setup"

.ENDIF			# "$(GUI)" == "WNT"

.IF "$(GUI)" == "OS2"
    @+-$(MKDIR) $(@:d:d) > $(NULLDEV)
.IF "$(APP9LINKRES)" != ""
    @+-$(RM) $(MISC)/$(APP9LINKRES:b).rc > $(NULLDEV)
.IF "$(APP9ICON)" != ""
    @-+echo ICON 1 $(EMQ)"$(APP9ICON:s#/#\\\\#)$(EMQ)" >> $(MISC)$/$(APP9LINKRES:b).rc
.ENDIF		# "$(APP9ICON)" != ""
.IF "$(APP9VERINFO)" != ""
    @-+echo $(EMQ)#define VERVARIANT	$(BUILD) >> $(MISC)/$(APP9LINKRES:b).rc
    @-+echo $(EMQ)#include  $(EMQ)"$(APP9VERINFO)$(EMQ)" >> $(MISC)/$(APP9LINKRES:b).rc
.ENDIF		# "$(APP9VERINFO)" != ""
    $(COMMAND_ECHO)$(RC) -r -DOS2 $(APP9PRODUCTDEF) -I$(SOLARRESDIR) $(INCLUDE) $(RCLINKFLAGS) $(MISC)/$(APP9LINKRES:b).rc
.ENDIF			# "$(APP9LINKRES)" != ""

.IF "$(TARGETTYPE)" == "GUI" 
    @echo NAME $(APP9TARGET) WINDOWAPI > $(MISC)/$(APP9TARGET).def
.ENDIF

    $(COMMAND_ECHO)$(APP9LINKER) -v \
        $(APP9LINKFLAGS) \
        $(LINKFLAGSAPP) $(APP9BASEX) \
        $(APP9STACKN) \
        -o $@ \
        -Zmap -L$(LB) \
        -L$(SOLARVERSION)/$(INPATH)/lib \
        $(STDOBJ) \
        $(APP9LINKRES) \
        $(APP9RES) \
        $(APP9DEF) \
        $(APP9OBJS) \
        $(APP9LIBS) \
        $(APP9STDLIBS) \
        $(APP9STDLIB) $(STDLIB9) 


.IF "$(APP9TARGET)" == "loader"
    $(COMMAND_ECHO)+$(PERL) loader.pl $@
    $(COMMAND_ECHO)+$(TYPE) $(@) $(@:d)unloader.exe > $(@:d)_new.exe
    $(COMMAND_ECHO)+$(RM) $@
    $(COMMAND_ECHO)+$(RENAME) $(@:d)_new.exe $(@:d)loader.exe
.ENDIF			# "$(TARGET)" == "setup"

.ENDIF			# "$(GUI)" == "OS2"

.ENDIF			# "$(APP9TARGETN)"!=""


# Instruction for linking
# unroll begin

.IF "$(GUI)" == "OS2" && "$(TARGETTYPE)" == "GUI" 
APP10DEF = $(MISC)/$(APP10TARGET).def
.ENDIF

.IF "$(APP10LINKTYPE)" != ""
#must be either STATIC or SHARED
APP10LINKTYPEFLAG=$(APPLINK$(APP10LINKTYPE))
.ENDIF

# decide how to link
.IF "$(APP10CODETYPE)"=="C"
APP10LINKER=$(LINKC)
APP10STDLIB=$(subst,CPPRUNTIME, $(STDLIB))
APP10LINKFLAGS+=$(LINKCFLAGS)
.ELSE			# "$(APP10CODETYPE)"=="C"
APP10LINKER=$(LINK)
APP10STDLIB=$(subst,CPPRUNTIME,$(STDLIBCPP) $(STDLIB))
APP10LINKFLAGS+=$(LINKFLAGS)
.ENDIF			# "$(APP10CODETYPE)"=="C"

APP10RPATH*=OOO
LINKFLAGSRUNPATH_$(APP10RPATH)*=/ERROR:/Bad_APP10RPATH_value
.IF "$(OS)" != "MACOSX"
APP10LINKFLAGS+=$(LINKFLAGSRUNPATH_$(APP10RPATH))
.ENDIF

.IF "$(APP10STACK)" != ""
.IF "$(LINKFLAGSTACK)" != ""
APP10STACKN=$(LINKFLAGSTACK)$(APP10STACK)
.ENDIF
.ELSE
APP10STACKN=
.ENDIF

.IF "$(APP10NOSAL)"==""
.IF "$(TARGETTYPE)" == "GUI"
APP10OBJS+= $(STDOBJVCL)
.ENDIF
.ENDIF

.IF "$(GUI)$(COM)" == "WNTGCC"
APP10RESO=
.IF "$(APP10LINKRES)" != "" || "$(APP10RES)" != ""
APP10RESO=$(MISC)/$(APP10TARGET:b)_res.o
.ENDIF
.ENDIF

.IF "$(GUI)" == "UNX"
APP10DEPN+:=$(APP10DEPNU)
USE_APP10DEF=
.ENDIF

.IF "$(APP10TARGETN)"!=""

.IF "$(APP10PRODUCTNAME)"!=""
APP10PRODUCTDEF+:=-DPRODUCT_NAME=\"$(APP10PRODUCTNAME)\"
.ENDIF			# "$(APP10PRODUCTNAME)"!=""

.IF "$(linkinc)"!=""
.IF "$(GUI)"=="WNT" || "$(GUI)"=="OS2"
.IF "$(APP10LIBS)"!=""
$(MISC)/$(APP10TARGET)_linkinc.ls .PHONY:
    @@-$(RM) $@
    sed -f $(SOLARENV)/bin/chrel.sed $(foreach,i,$(APP10LIBS) $(i:s/.lib/.lin/)) >> $@
.ENDIF          #"$(APP10LIBS)"!="" 
.ENDIF

LINKINCTARGETS+=$(MISC)/$(APP10TARGETN:b)_linkinc.ls
$(APP10TARGETN) : $(LINKINCTARGETS)
.ENDIF          # "$(linkinc)"!=""

# Allow for target specific LIBSALCPPRT override
APP10LIBSALCPPRT*=$(LIBSALCPPRT)

$(APP10TARGETN): $(APP10OBJS) $(APP10LIBS) \
    $(APP10RES) \
    $(APP10ICON) $(APP10DEPN) $(USE_APP10DEF)
    @echo "Making:   " $(@:f)
.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="MACOSX"
    @echo unx
    @-$(RM) $(MISC)/$(@:b).list
    @-$(RM) $(MISC)/$(TARGET).$(@:b)_10.cmd
    @-$(RM) $(MISC)/$(@:b).strip
    @echo $(STDSLO) $(APP10OBJS:s/.obj/.o/) \
    `cat /dev/null $(APP10LIBS) | sed s\#$(ROUT)\#$(OUT)\#g` | tr -s " " "\n" > $(MISC)/$(@:b).list
    @echo -n $(APP10LINKER) $(APP10LINKFLAGS) $(LINKFLAGSAPP) -L$(PRJ)/$(INPATH)/lib $(SOLARLIB) -o $@ \
    $(APP10LINKTYPEFLAG) $(APP10STDLIBS) $(APP10STDLIB) $(STDLIB10) -filelist $(MISC)/$(@:b).list > $(MISC)/$(TARGET).$(@:b)_10.cmd
    @$(PERL) $(SOLARENV)/bin/macosx-dylib-link-list.pl \
        `cat $(MISC)/$(TARGET).$(@:b)_10.cmd` \
        >> $(MISC)/$(TARGET).$(@:b)_10.cmd
  .IF "$(VERBOSE)" == "TRUE"
    @cat $(MISC)/$(TARGET).$(@:b)_10.cmd
  .ENDIF
    @+source $(MISC)/$(TARGET).$(@:b)_10.cmd
# Need to strip __objcInit symbol to avoid duplicate symbols when loading
# libraries at runtime
    @-$(CC_PATH)nm $@ | grep -v ' U ' | $(AWK) '{ print $$NF }' | grep -F -x '__objcInit' > $(MISC)/$(@:b).strip
    @$(CC_PATH)strip -i -R $(MISC)/$(@:b).strip -X $@
    @ls -l $@
    @$(PERL) $(SOLARENV)/bin/macosx-change-install-names.pl \
        app $(APP10RPATH) $@
.IF "$(TARGETTYPE)"=="GUI"
    @echo "Making:   " $(@:f).app
    @macosx-create-bundle $@
.ENDIF		# "$(TARGETTYPE)"=="GUI"
.ELSE		# "$(OS)"=="MACOSX"
    @echo unx
    @-$(RM) $(MISC)/$(TARGET).$(@:b)_10.cmd
    @echo $(APP10LINKER) $(APP10LINKFLAGS) $(LINKFLAGSAPP) -L$(PRJ)/$(INPATH)/lib $(SOLARLIB) $(STDSLO) \
    $(APP10OBJS:s/.obj/.o/) '\' >  $(MISC)/$(TARGET).$(@:b)_10.cmd
    @cat $(mktmp /dev/null $(APP10LIBS)) | xargs -n 1 cat | sed s\#$(ROUT)\#$(OUT)\#g | sed 's#$$# \\#'  >> $(MISC)/$(TARGET).$(@:b)_10.cmd
    @echo $(APP10LINKTYPEFLAG) $(APP10LIBSALCPPRT) $(APP10STDLIBS) $(APP10STDLIB) $(STDLIB10) -o $@ >> $(MISC)/$(TARGET).$(@:b)_10.cmd
  .IF "$(VERBOSE)" == "TRUE"
    @cat $(MISC)/$(TARGET).$(@:b)_10.cmd
  .ENDIF
    @+source $(MISC)/$(TARGET).$(@:b)_10.cmd
  .IF "$(VERBOSE)" == "TRUE"
    @ls -l $@
  .ENDIF
.ENDIF		# "$(OS)"=="MACOSX"
.ENDIF
.IF "$(GUI)" == "WNT"
    @@-$(MKDIR) $(@:d:d)
.IF "$(APP10LINKRES)" != ""
    @@-$(RM) $(MISC)/$(APP10LINKRES:b).rc
.IF "$(APP10ICON)" != ""
    @-echo 1 ICON $(EMQ)"$(APP10ICON:s/\/\\/)$(EMQ)" >> $(MISC)/$(APP10LINKRES:b).rc
.ENDIF		# "$(APP10ICON)" != ""
.IF "$(APP10VERINFO)" != ""
    @-echo $(EMQ)#define VERVARIANT	$(BUILD) >> $(MISC)/$(APP10LINKRES:b).rc
    @-echo $(EMQ)#include  $(EMQ)"$(APP10VERINFO)$(EMQ)" >> $(MISC)/$(APP10LINKRES:b).rc
.ENDIF		# "$(APP10VERINFO)" != ""
    $(COMMAND_ECHO)$(RC) -DWIN32 $(APP10PRODUCTDEF) -I$(SOLARRESDIR) $(INCLUDE) $(RCLINKFLAGS) $(MISC)/$(APP10LINKRES:b).rc
.ENDIF			# "$(APP10LINKRES)" != ""
.IF "$(COM)" == "GCC"
    @echo mingw
.IF "$(APP10LINKRES)" != "" || "$(APP10RES)" != ""
    @cat $(APP10LINKRES) $(subst,/res/,/res{$(subst,$(BIN), $(@:d))} $(APP10RES)) >  $(MISC)/$(@:b)_all.res
    windres $(MISC)/$(@:b)_all.res $(APP10RESO)
.ENDIF
    @echo $(LINK) $(LINKFLAGS) $(LINKFLAGSAPP) $(MINGWSSTDOBJ) -L$(PRJ)/$(INPATH)/lib $(SOLARLIB) $(STDSLO) \
        $(APP10BASEX) $(APP10STACKN) -o $@ $(APP10OBJS) \
        -Wl,-Map,$(MISC)/$(@:b).map $(STDOBJ) $(APP10RESO) \
        `$(TYPE) /dev/null $(APP10LIBS) | sed s#$(ROUT)#$(OUT)#g` \
        $(APP_LINKTYPE) $(APP10LIBSALCPPRT) \
        -Wl,--start-group $(APP10STDLIBS) -Wl,--end-group $(APP10STDLIB) \
        $(STDLIB10) $(MINGWSSTDENDOBJ) > $(MISC)/$(TARGET).$(@:b)_10.cmd
  # need this empty line, else dmake somehow gets confused by the .IFs and .ENDIFs
  .IF "$(VERBOSE)" == "TRUE"
    @$(TYPE)  $(MISC)/$(TARGET).$(@:b)_10.cmd
  .ENDIF
    @+source $(MISC)/$(TARGET).$(@:b)_10.cmd
    @ls -l $@
.ELSE	# "$(COM)" == "GCC"
.IF "$(linkinc)" == ""
    $(COMMAND_ECHO)$(APP10LINKER) @$(mktmp \
        $(APP10LINKFLAGS) \
        $(LINKFLAGSAPP) $(APP10BASEX) \
        $(APP10STACKN) \
        -out:$@ \
        -map:$(MISC)/{$(subst,/,_ $(APP10TARGET)).map} \
        $(STDOBJ) \
        $(APP10LINKRES) \
        $(APP10RES) \
        $(APP10OBJS) \
        $(APP10LIBS) \
        $(APP10STDLIBS) \
        $(APP10STDLIB) $(STDLIB10) \
        )
    @-echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.tmanifest -outputresource:$@$(EMQ);1 $(FI)
.ELSE
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -outputresource:$@$(EMQ);1 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ELSE
        $(COMMAND_ECHO)-$(RM) $(MISC)\$(APP10TARGET).lnk
        $(COMMAND_ECHO)-$(RM) $(MISC)\$(APP10TARGET).lst
        $(COMMAND_ECHO)-$(RM) $(MISC)\linkobj.lst
        for %_i in ($(MISC)\*.obj) do type %_i >> $(MISC)\linkobj.lst
    type $(mktmp,$(MISC)\$(APP10TARGET).lst
        $(APP10LINKFLAGS) \
        $(LINKFLAGSAPP) $(APP10BASEX) \
        $(APP10STACKN) \
        -out:$@ \
        $(STDOBJ) \
        $(APP10LINKRES) \
        $(APP10RES) \
        $(APP10OBJS) \
        $(APP10LIBS) \
        $(APP10STDLIBS) \
        $(APP10STDLIB) $(STDLIB10))
        $(COMMAND_ECHO)$(SED)$(SED) -e 's/\(\.\.\\\)\{2,4\}/..\\/g' $(MISC)\$(APP10TARGETN:b)_linkobj.lst >> $(MISC)\$(APP10TARGET).lst
        $(COMMAND_ECHO)$(SED)$(IFEXIST) $(MISC)/$(APP10TARGET).lst $(THEN) type $(MISC)/$(APP10TARGET).lst  >> $(MISC)/$(APP10TARGET).lnk $(FI)
        $(COMMAND_ECHO)$(SED)$(APP10LINKER) @$(MISC)\$(APP10TARGET).lnk
.ENDIF		# "$(linkinc)" == ""
.ENDIF		# "$(COM)" == "GCC"
.IF "$(APP10TARGET)" == "loader"
    $(COMMAND_ECHO)$(PERL) loader.pl $@
    $(COMMAND_ECHO)$(TYPE) $(@) $(@:d)unloader.exe > $(@:d)_new.exe
    $(COMMAND_ECHO)$(RM) $@
    $(COMMAND_ECHO)$(RENAME) $(@:d)_new.exe $(@:d)loader.exe
.ENDIF			# "$(TARGET)" == "setup"

.ENDIF			# "$(GUI)" == "WNT"

.IF "$(GUI)" == "OS2"
    @+-$(MKDIR) $(@:d:d) > $(NULLDEV)
.IF "$(APP10LINKRES)" != ""
    @+-$(RM) $(MISC)/$(APP10LINKRES:b).rc > $(NULLDEV)
.IF "$(APP10ICON)" != ""
    @-+echo ICON 1 $(EMQ)"$(APP10ICON:s#/#\\\\#)$(EMQ)" >> $(MISC)$/$(APP10LINKRES:b).rc
.ENDIF		# "$(APP10ICON)" != ""
.IF "$(APP10VERINFO)" != ""
    @-+echo $(EMQ)#define VERVARIANT	$(BUILD) >> $(MISC)/$(APP10LINKRES:b).rc
    @-+echo $(EMQ)#include  $(EMQ)"$(APP10VERINFO)$(EMQ)" >> $(MISC)/$(APP10LINKRES:b).rc
.ENDIF		# "$(APP10VERINFO)" != ""
    $(COMMAND_ECHO)$(RC) -r -DOS2 $(APP10PRODUCTDEF) -I$(SOLARRESDIR) $(INCLUDE) $(RCLINKFLAGS) $(MISC)/$(APP10LINKRES:b).rc
.ENDIF			# "$(APP10LINKRES)" != ""

.IF "$(TARGETTYPE)" == "GUI" 
    @echo NAME $(APP10TARGET) WINDOWAPI > $(MISC)/$(APP10TARGET).def
.ENDIF

    $(COMMAND_ECHO)$(APP10LINKER) -v \
        $(APP10LINKFLAGS) \
        $(LINKFLAGSAPP) $(APP10BASEX) \
        $(APP10STACKN) \
        -o $@ \
        -Zmap -L$(LB) \
        -L$(SOLARVERSION)/$(INPATH)/lib \
        $(STDOBJ) \
        $(APP10LINKRES) \
        $(APP10RES) \
        $(APP10DEF) \
        $(APP10OBJS) \
        $(APP10LIBS) \
        $(APP10STDLIBS) \
        $(APP10STDLIB) $(STDLIB10) 


.IF "$(APP10TARGET)" == "loader"
    $(COMMAND_ECHO)+$(PERL) loader.pl $@
    $(COMMAND_ECHO)+$(TYPE) $(@) $(@:d)unloader.exe > $(@:d)_new.exe
    $(COMMAND_ECHO)+$(RM) $@
    $(COMMAND_ECHO)+$(RENAME) $(@:d)_new.exe $(@:d)loader.exe
.ENDIF			# "$(TARGET)" == "setup"

.ENDIF			# "$(GUI)" == "OS2"

.ENDIF			# "$(APP10TARGETN)"!=""


# Instruction for linking
