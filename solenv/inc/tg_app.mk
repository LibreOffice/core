#*************************************************************************
#
#   $RCSfile: tg_app.mk,v $
#
#   $Revision: 1.41 $
#
#   last change: $Author: hr $ $Date: 2003-07-16 18:20:22 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
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
# instructions for linking
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
.IF "$(APP$(TNR)NOSAL)"==""
.IF "$(TARGETTYPE)" == "GUI"
APP$(TNR)OBJS+= $(STDOBJVCL)
.ENDIF
.ENDIF

.IF "$(GUI)" != "WNT" || "$(GUI)$(COM)"=="WNTBLC"
USE_APP$(TNR)DEF=$(APP$(TNR)DEF)
.ELSE
USE_APP$(TNR)DEF=
.ENDIF

.IF "$(GUI)" == "UNX"
APP$(TNR)DEPN+:=$(APP$(TNR)DEPNU)
USE_APP$(TNR)DEF=
.ENDIF

# Link in static data members for template classes
.IF "$(OS)$(CVER)"=="MACOSXC295"
# Allow certain executables to not link to libstatic*.dylib. This is only used
# by build tools that are built in the bootstrap process.
.IF "$(NOSHAREDSTATICLIB)"==""
APP$(TNR)STDLIBS+=$(STATICLIB)
.ENDIF
.ENDIF

.IF "$(APP$(TNR)TARGETN)"!=""

.IF "$(linkinc)"!=""
.IF "$(GUI)"=="WNT"
$(MISC)$/$(APP$(TNR)TARGET)_linkinc.ls .PHONY:
    @+-$(RM) $@ >& $(NULLDEV)
    +sed -f $(COMMON_ENV_TOOLS)$/chrel.sed $(foreach,i,$(APP$(TNR)LIBS) $(i:s/.lib/.lin/)) >> $@
.ENDIF

LINKINCTARGETS+=$(MISC)$/$(APP$(TNR)TARGETN:b)_linkinc.ls
$(APP$(TNR)TARGETN) : $(LINKINCTARGETS)
.ENDIF          # "$(linkinc)"!=""

# Allow for target specific LIBSALCPPRT override
APP$(TNR)LIBSALCPPRT*=$(LIBSALCPPRT)

$(APP$(TNR)TARGETN): $(APP$(TNR)OBJS) $(APP$(TNR)LIBS) \
    $(APP$(TNR)RES) \
    $(APP$(TNR)ICON) $(APP$(TNR)DEPN) $(USE_APP$(TNR)DEF)
.IF "$(OS)$(CVER)"=="MACOSXC295"
    @echo "------------------------------"
    @echo "Updating static data member initializations"
    @+dmake -f $(SOLARENV)$/$(OUTPATH)$/inc/makefile.mk $(MFLAGS) $(CALLMACROS) "PRJ=$(PRJ)" "PRJNAME=$(PRJNAME)" "TARGET=$(TARGET)"
.ENDIF
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="MACOSX"
    @+echo unx
    @+-$(RM) $(MISC)$/$(@:b).list
    @+-$(RM) $(MISC)$/$(@:b).cmd
    @+-$(RM) $(MISC)$/$(@:b).strip
    @+echo $(STDSLO) $(APP$(TNR)OBJS:s/.obj/.o/) \
    `cat /dev/null $(APP$(TNR)LIBS) | sed s\#$(ROUT)\#$(OUT)\#g` | tr -s " " "\n" > $(MISC)$/$(@:b).list
    @+echo $(LINK) $(LINKFLAGS) $(LINKFLAGSAPP) -L$(PRJ)$/$(INPATH)$/lib $(SOLARLIB) -o $@ \
    `dylib-link-list $(PRJNAME) $(SOLARVERSION)$/$(INPATH)$/lib $(PRJ)$/$(INPATH)$/lib $(APP$(TNR)STDLIBS) $(STDLIB) $(STDLIB$(TNR))` \
    $(APP_LINKTYPE) $(APP$(TNR)STDLIBS) $(STDLIB) $(STDLIB$(TNR)) -filelist $(MISC)$/$(@:b).list > $(MISC)$/$(@:b).cmd
    @cat $(MISC)$/$(@:b).cmd
    @source $(MISC)$/$(@:b).cmd
# Need to strip __objcInit symbol to avoid duplicate symbols when loading
# libraries at runtime
    @+-nm $@ | grep -v ' U ' | $(AWK) '{ print $$NF }' | grep -F -x '__objcInit' > $(MISC)$/$(@:b).strip
    @strip -i -R $(MISC)$/$(@:b).strip -X $@
    @ls -l $@
   .IF "$(CVER)"=="C295"
     # This is a hack as libstatic and libcppuhelper have a circular dependency
     .IF "$(PRJNAME)"=="cppuhelper"
         @echo "------------------------------"
         @echo "Rerunning static data member initializations"
         @+dmake -u -f $(SOLARENV)$/$(OUTPATH)$/inc/makefile.mk $(MFLAGS) $(CALLMACROS) "PRJ=$(PRJ)" "PRJNAME=$(PRJNAME)" "TARGET=$(TARGET)"
     .ENDIF
   .ENDIF
.IF "$(TARGETTYPE)"=="GUI"
    @echo "Making: $@.app"
.IF "$(STLPORT4)"!=""
.IF "$(STLPORT4)"!="NO_STLPORT4"
    @-ln -sf "$(STLPORT4)/lib/libstlport_gcc.dylib" "$(SOLARLIBDIR)"
.ENDIF
.ENDIF		# "$(STLPORT4)!=""
    @create-bundle $@
.ENDIF		# "$(TARGETTYPE)"=="GUI"
.ELSE		# "$(OS)"=="MACOSX"
    @+echo unx
    @+-$(RM) $(MISC)$/$(@:b).cmd
    @+echo $(LINK) $(LINKFLAGS) $(LINKFLAGSAPP) -L$(PRJ)$/$(INPATH)$/lib $(SOLARLIB) $(STDSLO) \
    -o $@ $(APP$(TNR)OBJS:s/.obj/.o/) "\" >  $(MISC)$/$(@:b).cmd
    @cat $(mktmp /dev/null $(APP$(TNR)LIBS)) | xargs -n 1 cat | sed s\#$(ROUT)\#$(OUT)\#g | sed 's#$$# \\#'  >> $(MISC)$/$(@:b).cmd
    @+echo $(APP_LINKTYPE) $(APP$(TNR)LIBSALCPPRT) $(APP$(TNR)STDLIBS) $(STDLIB) $(STDLIB$(TNR)) >> $(MISC)$/$(@:b).cmd
    cat $(MISC)$/$(@:b).cmd
    @source $(MISC)$/$(@:b).cmd
    @ls -l $@
.ENDIF		# "$(OS)"=="MACOSX"
.ENDIF
.IF "$(GUI)"=="MAC"
    @+-$(RM) $@ $@.xSYM $@.idb
    $(LINK) $(LINKFLAGS) $(LINKFLAGSAPP) $(foreach,i,$(shell $(UNIX2MACPATH) $(PRJ)$/$(ROUT)$/lib $(SOLARLIB:s/-L//)) -L"$i") $(shell $(UNIX2MACPATH) $(STDSLO) $(APP$(TNR)OBJS) `cat /dev/null $(APP$(TNR)LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` $(VERSIONOBJ)) $(APP$(TNR)STDLIBS) $(APP$(TNR)ARCHIVES) $(STDSHL) $(LINKOUTPUT_FILTER) -o $(shell $(UNIX2MACPATH) $@)
.ENDIF                  # "$(GUI)"=="MAC"
.IF "$(GUI)" == "WNT"
    @+-$(MKDIR) $(@:d:d) >& $(NULLDEV)
.IF "$(APP$(TNR)LINKRES)" != ""
    @+-$(RM) $(MISC)$/$(APP$(TNR)LINKRES:b).rc >& $(NULLDEV)
.IF "$(USE_SHELL)"=="4nt"
.IF "$(APP$(TNR)ICON)" != ""
    @-+echo 1 ICON "$(APP$(TNR)ICON:s/\/\\/)" >> $(MISC)$/$(APP$(TNR)LINKRES:b).rc
.ENDIF
.IF "$(APP$(TNR)VERINFO)" != ""
    @-+echo #define VERVARIANT	$(BUILD) >> $(MISC)$/$(APP$(TNR)LINKRES:b).rc
    @-+echo #include  "$(APP$(TNR)VERINFO)" >> $(MISC)$/$(APP$(TNR)LINKRES:b).rc
.ENDIF
.ELSE			# "$(USE_SHELL)"=="4nt"
.IF "$(APP$(TNR)ICON)" != ""
    @-+guw.pl echo 1 ICON \"$(APP$(TNR)ICON)\" | sed 'sX\\X\\\\Xg' >> $(MISC)$/$(APP$(TNR)LINKRES:b).rc
.ENDIF
.IF "$(APP$(TNR)VERINFO)" != ""
    @-+echo \#define VERVARIANT	$(BUILD) >> $(MISC)$/$(APP$(TNR)LINKRES:b).rc
    @-+echo \#include  \"$(APP$(TNR)VERINFO)\" >> $(MISC)$/$(APP$(TNR)LINKRES:b).rc
.ENDIF
.ENDIF			# "$(USE_SHELL)"=="4nt"
    $(RC) -DWIN32 -I$(SOLARRESDIR) $(INCLUDE) $(RCLINKFLAGS) $(MISC)$/$(APP$(TNR)LINKRES:b).rc
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
        $(STDLIB) $(STDLIB$(TNR)) \
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
        $(STDLIB) $(STDLIB$(TNR)))
        sed -e 's/\(\.\.\\\)\{2,4\}/..\\/g' $(MISC)\$(APP$(TNR)TARGETN:b)_linkobj.lst >> $(MISC)\$(APP$(TNR)TARGET).lst
        +if exist $(MISC)\$(APP$(TNR)TARGET).lst type $(MISC)\$(APP$(TNR)TARGET).lst  >> $(MISC)\$(APP$(TNR)TARGET).lnk
        $(LINK) @$(MISC)\$(APP$(TNR)TARGET).lnk
.ENDIF		# "$(linkinc)" == ""
.IF "$(APP$(TNR)TARGET)" == "loader"
    +$(PERL) loader.pl $@
.IF "$(USE_SHELL)"=="4nt"
    +$(COPY) /b $(@)+$(@:d)unloader.exe $(@:d)_new.exe
.ELSE			# "$(USE_SHELL)"=="4nt"
    +$(TYPE) $(@) $(@:d)unloader.exe > $(@:d)_new.exe
.ENDIF			# "$(USE_SHELL)"=="4nt"
    +$(RM) $@
    +$(RENAME) $(@:d)_new.exe $(@:d)loader.exe
.ENDIF			# "$(TARGET)" == "setup"

.ENDIF			# "$(GUI)" == "WNT"

.IF "$(GUI)"=="WIN"
.IF "$(COM)"=="BLC"
    $(LINK) @$(mktmp$ $(LINKFLAGS) $(LINKFLAGSAPP) $(APP$(TNR)STACKN) $(STDOBJ) $(APP$(TNR)OBJS), $@, $(MISC)\$(APP$(TNR)TARGET).map, $(APP$(TNR)LIBS) $(APP$(TNR)STDLIBS) $(STDLIB) $(STDLIB$(TNR)), $(APP$(TNR)DEF)) >&  $(TMP)$/$(PRJNAME)$(USER).tmp
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
    @$(COPY) $(APP$(TNR)TARGET).sym $(BIN)\$(APP$(TNR)TARGET).sym
    @$(RM) $(APP$(TNR)TARGET).sym
.ENDIF			# "$(MAPSYM)" != ""
.ENDIF			# "$(TARGETTYPE)" == "GUI"
.ENDIF			# "$(GUI)" == "WIN"

.ENDIF			# "$(APP$(TNR)TARGETN)"!=""


# Instruction for linking
# unroll end
#######################################################

.ENDIF		# "$(MULTI_APP_FLAG)"==""
