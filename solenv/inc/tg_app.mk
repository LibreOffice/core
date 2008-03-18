#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: tg_app.mk,v $
#
#   $Revision: 1.71 $
#
#   last change: $Author: vg $ $Date: 2008-03-18 13:08:55 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
#
#*************************************************************************

#######################################################
# instructions for linking
# unroll begin

.IF "$(APP$(TNR)LINKTYPE)" != ""
#must be either STATIC or SHARED
APP$(TNR)LINKTYPEFLAG=$(APPLINK$(APP$(TNR)LINKTYPE))
.ENDIF

# decide how to link
.IF "$(APP$(TNR)CODETYPE)"=="C"
APP$(TNR)LINKER=$(LINKC)
APP$(TNR)STDLIB=$(subst,CPPRUNTIME, $(STDLIB))
APP$(TNR)LINKFLAGS+=$(LINKCFLAGS)
.ELSE			# "$(APP$(TNR)CODETYPE)"=="C"
APP$(TNR)LINKER=$(LINK)
APP$(TNR)STDLIB=$(subst,CPPRUNTIME,$(STDLIBCPP) $(STDLIB))
APP$(TNR)LINKFLAGS+=$(LINKFLAGS)
.ENDIF			# "$(APP$(TNR)CODETYPE)"=="C"

APP$(TNR)RPATH*=OOO
LINKFLAGSRUNPATH_$(APP$(TNR)RPATH)*=/ERROR:/Bad_APP$(TNR)RPATH_value
.IF "$(OS)" != "MACOSX"
APP$(TNR)LINKFLAGS+=$(LINKFLAGSRUNPATH_$(APP$(TNR)RPATH))
.ENDIF

.IF "$(APP$(TNR)STACK)" != ""
.IF "$(LINKFLAGSTACK)" != ""
APP$(TNR)STACKN=$(LINKFLAGSTACK)$(APP$(TNR)STACK)
.ENDIF
.ELSE
APP$(TNR)STACKN=
.ENDIF

.IF "$(APP$(TNR)NOSAL)"==""
.IF "$(TARGETTYPE)" == "GUI"
APP$(TNR)OBJS+= $(STDOBJVCL)
.ENDIF
.ENDIF

.IF "$(GUI)$(COM)" == "WNTGCC"
APP$(TNR)RESO=
.IF "$(APP$(TNR)LINKRES)" != "" || "$(APP$(TNR)RES)" != ""
APP$(TNR)RESO=$(MISC)$/$(APP$(TNR)TARGET:b)_res.o
.ENDIF
.ENDIF

.IF "$(GUI)" == "UNX"
APP$(TNR)DEPN+:=$(APP$(TNR)DEPNU)
USE_APP$(TNR)DEF=
.ENDIF

.IF "$(APP$(TNR)TARGETN)"!=""

.IF "$(APP$(TNR)PRODUCTNAME)"!=""
APP$(TNR)PRODUCTDEF:=-DPRODUCT_NAME=\"$(APP$(TNR)PRODUCTNAME)\"
.ENDIF			# "$(APP$(TNR)PRODUCTNAME)"!=""

.IF "$(linkinc)"!=""
.IF "$(GUI)"=="WNT"
$(MISC)$/$(APP$(TNR)TARGET)_linkinc.ls .PHONY:
    @@-$(RM) $@
    sed -f $(COMMON_ENV_TOOLS)$/chrel.sed $(foreach,i,$(APP$(TNR)LIBS) $(i:s/.lib/.lin/)) >> $@
.ENDIF

LINKINCTARGETS+=$(MISC)$/$(APP$(TNR)TARGETN:b)_linkinc.ls
$(APP$(TNR)TARGETN) : $(LINKINCTARGETS)
.ENDIF          # "$(linkinc)"!=""

# Allow for target specific LIBSALCPPRT override
APP$(TNR)LIBSALCPPRT*=$(LIBSALCPPRT)

$(APP$(TNR)TARGETN): $(APP$(TNR)OBJS) $(APP$(TNR)LIBS) \
    $(APP$(TNR)RES) \
    $(APP$(TNR)ICON) $(APP$(TNR)DEPN) $(USE_APP$(TNR)DEF)
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="MACOSX"
    @echo unx
    @-$(RM) $(MISC)$/$(@:b).list
    @-$(RM) $(MISC)$/$(TARGET).$(@:b)_$(TNR).cmd
    @-$(RM) $(MISC)$/$(@:b).strip
    @echo $(STDSLO) $(APP$(TNR)OBJS:s/.obj/.o/) \
    `cat /dev/null $(APP$(TNR)LIBS) | sed s\#$(ROUT)\#$(OUT)\#g` | tr -s " " "\n" > $(MISC)$/$(@:b).list
    @echo -n $(APP$(TNR)LINKER) $(APP$(TNR)LINKFLAGS) $(LINKFLAGSAPP) -L$(PRJ)$/$(INPATH)$/lib $(SOLARLIB) -o $@ \
    $(APP$(TNR)LINKTYPEFLAG) $(APP$(TNR)STDLIBS) $(APP$(TNR)STDLIB) $(STDLIB$(TNR)) -filelist $(MISC)$/$(@:b).list > $(MISC)$/$(TARGET).$(@:b)_$(TNR).cmd
    @$(PERL) $(SOLARENV)$/bin$/macosx-dylib-link-list.pl \
        `cat $(MISC)$/$(TARGET).$(@:b)_$(TNR).cmd` \
        >> $(MISC)$/$(TARGET).$(@:b)_$(TNR).cmd
    @cat $(MISC)$/$(TARGET).$(@:b)_$(TNR).cmd
    @+source $(MISC)$/$(TARGET).$(@:b)_$(TNR).cmd
# Need to strip __objcInit symbol to avoid duplicate symbols when loading
# libraries at runtime
    @-nm $@ | grep -v ' U ' | $(AWK) '{ print $$NF }' | grep -F -x '__objcInit' > $(MISC)$/$(@:b).strip
    @strip -i -R $(MISC)$/$(@:b).strip -X $@
    @ls -l $@
    @$(PERL) $(SOLARENV)$/bin$/macosx-change-install-names.pl \
        app $(APP$(TNR)RPATH) $@
.IF "$(TARGETTYPE)"=="GUI"
    @echo "Making: $@.app"
    @macosx-create-bundle $@
.ENDIF		# "$(TARGETTYPE)"=="GUI"
.ELSE		# "$(OS)"=="MACOSX"
    @echo unx
    @-$(RM) $(MISC)$/$(TARGET).$(@:b)_$(TNR).cmd
    @echo $(APP$(TNR)LINKER) $(APP$(TNR)LINKFLAGS) $(LINKFLAGSAPP) -L$(PRJ)$/$(INPATH)$/lib $(SOLARLIB) $(STDSLO) \
    $(APP$(TNR)OBJS:s/.obj/.o/) '\' >  $(MISC)$/$(TARGET).$(@:b)_$(TNR).cmd
    @cat $(mktmp /dev/null $(APP$(TNR)LIBS)) | xargs -n 1 cat | sed s\#$(ROUT)\#$(OUT)\#g | sed 's#$$# \\#'  >> $(MISC)$/$(TARGET).$(@:b)_$(TNR).cmd
    @echo $(APP$(TNR)LINKTYPEFLAG) $(APP$(TNR)LIBSALCPPRT) $(APP$(TNR)STDLIBS) $(APP$(TNR)STDLIB) $(STDLIB$(TNR)) -o $@ >> $(MISC)$/$(TARGET).$(@:b)_$(TNR).cmd
    cat $(MISC)$/$(TARGET).$(@:b)_$(TNR).cmd
    @+source $(MISC)$/$(TARGET).$(@:b)_$(TNR).cmd
    @ls -l $@
.ENDIF		# "$(OS)"=="MACOSX"
.ENDIF
.IF "$(GUI)" == "WNT"
    @@-$(MKDIR) $(@:d:d)
.IF "$(APP$(TNR)LINKRES)" != ""
    @@-$(RM) $(MISC)$/$(APP$(TNR)LINKRES:b).rc
.IF "$(APP$(TNR)ICON)" != ""
    @-echo 1 ICON $(EMQ)"$(APP$(TNR)ICON:s/\/\\/)$(EMQ)" >> $(MISC)$/$(APP$(TNR)LINKRES:b).rc
.ENDIF		# "$(APP$(TNR)ICON)" != ""
.IF "$(APP$(TNR)VERINFO)" != ""
    @-echo $(EMQ)#define VERVARIANT	$(BUILD) >> $(MISC)$/$(APP$(TNR)LINKRES:b).rc
    @-echo $(EMQ)#include  $(EMQ)"$(APP$(TNR)VERINFO)$(EMQ)" >> $(MISC)$/$(APP$(TNR)LINKRES:b).rc
.ENDIF		# "$(APP$(TNR)VERINFO)" != ""
    $(RC) -DWIN32 $(APP$(TNR)PRODUCTDEF) -I$(SOLARRESDIR) $(INCLUDE) $(RCLINKFLAGS) $(MISC)$/$(APP$(TNR)LINKRES:b).rc
.ENDIF			# "$(APP$(TNR)LINKRES)" != ""
.IF "$(COM)" == "GCC"
    @echo mingw
.IF "$(APP$(TNR)LINKRES)" != "" || "$(APP$(TNR)RES)" != ""
    @cat $(APP$(TNR)LINKRES) $(subst,$/res$/,$/res{$(subst,$(BIN), $(@:d))} $(APP$(TNR)RES)) >  $(MISC)$/$(@:b)_all.res
    windres $(MISC)$/$(@:b)_all.res $(APP$(TNR)RESO)
.ENDIF
    @echo $(LINK) $(LINKFLAGS) $(LINKFLAGSAPP) -L$(PRJ)$/$(INPATH)$/lib $(SOLARLIB) $(STDSLO) \
        $(APP$(TNR)BASEX) $(APP$(TNR)STACKN) -o $@ $(APP$(TNR)OBJS) \
        -Wl,-Map,$(MISC)$/$(@:b).map $(STDOBJ) $(APP$(TNR)RESO) \
        `$(TYPE) /dev/null $(APP$(TNR)LIBS) | sed s#$(ROUT)#$(OUT)#g` \
        $(APP_LINKTYPE) $(APP$(TNR)LIBSALCPPRT) $(APP$(TNR)STDLIBS) $(APP$(TNR)STDLIB) $(STDLIB$(TNR)) > $(MISC)$/$(TARGET).$(@:b)_$(TNR).cmd
    @$(TYPE)  $(MISC)$/$(TARGET).$(@:b)_$(TNR).cmd
    @+source $(MISC)$/$(TARGET).$(@:b)_$(TNR).cmd
    @ls -l $@
.ELSE	# "$(COM)" == "GCC"
.IF "$(linkinc)" == ""
    $(APP$(TNR)LINKER) @$(mktmp \
        $(APP$(TNR)LINKFLAGS) \
        $(LINKFLAGSAPP) $(APP$(TNR)BASEX) \
        $(APP$(TNR)STACKN) \
        -out:$@ \
        -map:$(MISC)$/{$(subst,$/,_ $(APP$(TNR)TARGET)).map} \
        $(STDOBJ) \
        $(APP$(TNR)LINKRES) \
        $(APP$(TNR)RES) \
        $(APP$(TNR)OBJS) \
        $(APP$(TNR)LIBS) \
        $(APP$(TNR)STDLIBS) \
        $(APP$(TNR)STDLIB) $(STDLIB$(TNR)) \
        )
    @-echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)$/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.tmanifest -outputresource:$@$(EMQ);1 $(FI)
.ELSE
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -outputresource:$@$(EMQ);1 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ELSE
        -$(RM) $(MISC)\$(APP$(TNR)TARGET).lnk
        -$(RM) $(MISC)\$(APP$(TNR)TARGET).lst
        -$(RM) $(MISC)\linkobj.lst
        for %_i in ($(MISC)\*.obj) do type %_i >> $(MISC)\linkobj.lst
    type $(mktmp,$(MISC)\$(APP$(TNR)TARGET).lst
        $(APP$(TNR)LINKFLAGS) \
        $(LINKFLAGSAPP) $(APP$(TNR)BASEX) \
        $(APP$(TNR)STACKN) \
        -out:$@ \
        $(STDOBJ) \
        $(APP$(TNR)LINKRES) \
        $(APP$(TNR)RES) \
        $(APP$(TNR)OBJS) \
        $(APP$(TNR)LIBS) \
        $(APP$(TNR)STDLIBS) \
        $(APP$(TNR)STDLIB) $(STDLIB$(TNR)))
        $(SED) -e 's/\(\.\.\\\)\{2,4\}/..\\/g' $(MISC)\$(APP$(TNR)TARGETN:b)_linkobj.lst >> $(MISC)\$(APP$(TNR)TARGET).lst
        $(IFEXIST) $(MISC)$/$(APP$(TNR)TARGET).lst $(THEN) type $(MISC)$/$(APP$(TNR)TARGET).lst  >> $(MISC)$/$(APP$(TNR)TARGET).lnk $(FI)
        $(APP$(TNR)LINKER) @$(MISC)\$(APP$(TNR)TARGET).lnk
.ENDIF		# "$(linkinc)" == ""
.ENDIF		# "$(COM)" == "GCC"
.IF "$(APP$(TNR)TARGET)" == "loader"
    $(PERL) loader.pl $@
.IF "$(USE_SHELL)"=="4nt"
    $(COPY) /b $(@)+$(@:d)unloader.exe $(@:d)_new.exe
.ELSE			# "$(USE_SHELL)"=="4nt"
    $(TYPE) $(@) $(@:d)unloader.exe > $(@:d)_new.exe
.ENDIF			# "$(USE_SHELL)"=="4nt"
    $(RM) $@
    $(RENAME) $(@:d)_new.exe $(@:d)loader.exe
.ENDIF			# "$(TARGET)" == "setup"

.ENDIF			# "$(GUI)" == "WNT"

.ENDIF			# "$(APP$(TNR)TARGETN)"!=""


# Instruction for linking
# unroll end
#######################################################

