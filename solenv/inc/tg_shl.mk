#*************************************************************************
#
#   $RCSfile: tg_shl.mk,v $
#
#   $Revision: 1.75 $
#
#   last change: $Author: hr $ $Date: 2003-07-16 18:20:44 $
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

MKFILENAME:=tg_shl.mk

#######################################################
# Anweisungen fuer Rekursion

.IF "$(MULTI_SHL_FLAG)" == ""
$(SHL1TARGETN) .NULL : SHL1

$(SHL2TARGETN) .NULL : SHL2

$(SHL3TARGETN) .NULL : SHL3

$(SHL4TARGETN) .NULL : SHL4

$(SHL5TARGETN) .NULL : SHL5

$(SHL6TARGETN) .NULL : SHL6

$(SHL7TARGETN) .NULL : SHL7

$(SHL8TARGETN) .NULL : SHL8

$(SHL9TARGETN) .NULL : SHL9
.ENDIF



.IF "$(MULTI_SHL_FLAG)"==""
SHL1 SHL2 SHL3 SHL4 SHL5 SHL6 SHL7 SHL8 SHL9:
.IF "$(GUI)"=="UNX"
    @dmake $(SHL$(TNR)TARGETN) MULTI_SHL_FLAG=true TNR:=$(TNR) $(MFLAGS) $(CALLMACROS)
.ELSE
    @dmake $(SHL$(TNR)TARGETN) MULTI_SHL_FLAG=true TNR:=$(TNR) $(MFLAGS) $(CALLMACROS)
.ENDIF
.ELSE

#######################################################
# Anweisungen fuer das Linken
# unroll begin

.IF "$(SHL$(TNR)TARGETN)"!=""

.IF "$(OS)"=="AIX"
SHL$(TNR)STDLIBS=
.ENDIF

# Link in static data members for template classes
.IF "$(OS)$(CVER)"=="MACOSXC295"
SHL$(TNR)STDLIBS+=$(STATICLIB)
# Allow certain libraries to not link to libstatic*.dylib. This is only used
# by libraries that cannot be linked to other libraries.
.IF "$(NOSHAREDSTATICLIB)"==""
SHL$(TNR)STDLIBS+=$(STATICLIB)
.ENDIF
.ENDIF

.IF "$(SHLLINKARCONLY)" != ""
SHL$(TNR)STDLIBS=
STDSHL=
.ELSE
SHL$(TNR)ARCHIVES=
.ENDIF

SHL$(TNR)DEF*=$(MISC)$/$(SHL$(TNR)TARGET).def

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#+++++++++++	description fallbak	++++++++++++++++++++++++++++++++++++++++
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

.IF "$(SHL$(TNR)TARGET)"!=""
.IF "$(COMP$(TNR)TYPELIST)"==""

#fallback
LOCAL$(TNR)DESC:=$(subst,/,$/ $(shell $(FIND) . -name "{$(subst,$($(WINVERSIONNAMES)_MAJOR),* $(subst,$(UPD)$(DLLPOSTFIX), $(SHL$(TNR)TARGET)))}.xml"))
.IF "$(LOCAL$(TNR)DESC)"==""
$(MISC)$/%{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL$(TNR)TARGET))}.xml : $(SOLARENV)$/src$/default_description.xml
    +$(COPY) $< $@
.ELSE           # "$(LOCALDESC$(TNR))"==""
SHL$(TNR)DESCRIPTIONOBJ*=$(SLO)$/$(LOCAL$(TNR)DESC:b)$($(WINVERSIONNAMES)_MAJOR)_description.obj
.ENDIF          # "$(LOCALDESC$(TNR))"==""

.ENDIF          # "$(COMP$(TNR)TYPELIST)"==""
.ENDIF			# "$(SHL$(TNR)TARGET)"!="

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#+++++++++++    version object      ++++++++++++++++++++++++++++++++++++++++
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

.IF "$(VERSIONOBJ)"!=""
SHL$(TNR)VERSIONOBJ:=$(VERSIONOBJ:d){$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL$(TNR)TARGET))}$(VERSIONOBJ:f)
.IF "$(UPDATER)"=="YES"
USE_VERSIONH:=$(INCCOM)$/_version.h
.ELSE			# "$(UPDATER)"=="YES"
.IF "$(GUI)" == "UNX"
SHL$(TNR)DEPN+=$(SHL$(TNR)VERSIONOBJ:s/.o/.obj/)
.ELSE           # "$(GUI)" == "UNX"
SHL$(TNR)DEPN+=$(SHL$(TNR)VERSIONOBJ)
.ENDIF          # "$(GUI)" == "UNX"
$(MISC)$/$(SHL$(TNR)VERSIONOBJ:b).c : $(SOLARENV)$/src$/version.c $(INCCOM)$/_version.h
    +$(COPY) $(SOLARENV)$/src$/version.c $@

.ENDIF			# "$(UPDATER)"=="YES"
.ENDIF			# "$(VERSIONOBJ)"!=""

.IF "$(GUI)" != "UNX"
.IF "$(GUI)" == "WNT"
.IF "$(COM)" == "MSC"
.IF "$(SHL$(TNR)IMPLIB)" != ""
USE_$(TNR)IMPLIB=-implib:$(LB)$/$(SHL$(TNR)IMPLIB).lib
.ELSE			# "$(SHL$(TNR)IMPLIB)" != ""
USE_$(TNR)IMPLIB=-implib:$(LB)$/i$(TARGET)$(TNR).lib
.ENDIF			# "$(SHL$(TNR)IMPLIB)" != ""
.ENDIF			# "$(COM)" == "MSC"

.IF "$(USE_DEFFILE)"==""
USE_$(TNR)IMPLIB_DEPS=$(LB)$/$(SHL$(TNR)IMPLIB).lib
.ENDIF			# "$(USE_DEFFILE)"==""
.ENDIF			# "$(GUI)" == "WNT"
USE_SHL$(TNR)DEF=$(SHL$(TNR)DEF)
.ELSE			# "$(GUI)" != "UNX"
USE_SHL$(TNR)DEF=
SHL$(TNR)DEPN+:=$(SHL$(TNR)DEPNU)

.IF "$(SHL$(TNR)VERSIONMAP)"==""
# to activate vmaps remove "#"
USE_SHL$(TNR)VERSIONMAP=$(MISC)$/$(SHL$(TNR)TARGET).vmap
.ENDIF			# "$(SHL$(TNR)VERSIONMAP)"==""

.IF "$(USE_SHL$(TNR)VERSIONMAP)"!=""

.IF "$(SHL$(TNR)FILTERFILE)"!=""
.IF "$(SHL$(TNR)VERSIONMAP)"!=""
#eine von beiden ist zuviel
USE_SHL$(TNR)VERSIONMAP=$(MISC)$/$(SHL$(TNR)TARGET).vmap
$(USE_SHL$(TNR)VERSIONMAP) .PHONY: 
    @+echo -----------------------------
    @+echo you should only use versionmap OR exportfile
    @+echo -----------------------------
#	force_dmake_to_error

.ENDIF			# "$(SHL$(TNR)VERSIONMAP)"!=""

.IF "$(OS)"!="MACOSX"
.IF "$(OS)"!="IRIX"
SHL$(TNR)VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL$(TNR)VERSIONMAP)
.ENDIF
.ENDIF

$(USE_SHL$(TNR)VERSIONMAP): \
                    $(SHL$(TNR)OBJS)\
                    $(SHL$(TNR)LIBS)\
                    $(SHL$(TNR)FILTERFILE)
    @+$(RM) $@.dump
.IF "$(SHL$(TNR)OBJS)"!=""
# dump remaining objects on the fly
.IF "$(OS)"=="MACOSX"
    @-+nm $(SHL$(TNR)OBJS:s/.obj/.o/) > $@.dump
.ELSE
    @+nm $(SHL$(TNR)OBJS:s/.obj/.o/) > $@.dump
.ENDIF
.ENDIF			# "$(SHL$(TNR)OBJS)!"=""
    @+$(TYPE) /dev/null $(SHL$(TNR)LIBS:s/.lib/.dump/) >> $@.dump
    +$(PERL) $(SOLARENV)$/bin$/mapgen.pl -d $@.dump -s $(SHL$(TNR)INTERFACE) -f $(SHL$(TNR)FILTERFILE) -m $@

.ELSE			# "$(SHL$(TNR)FILTERFILE)"!=""
USE_SHL$(TNR)VERSIONMAP=$(MISC)$/$(SHL$(TNR)TARGET).vmap
$(USE_SHL$(TNR)VERSIONMAP) : 
    @+echo -----------------------------
    @+echo SHL$(TNR)FILTERFILE not set!
    @+echo -----------------------------
    @$(TOUCH) $@
    @+echo dummy file to keep the dependencies for later use.
#	force_dmake_to_error
.ENDIF			# "$(SHL$(TNR)FILTERFILE)"!=""
.ELSE			# "$(USE_SHL$(TNR)VERSIONMAP)"!=""

#and now for the plain non-generic way...
.IF "$(SHL$(TNR)VERSIONMAP)"!=""
USE_SHL$(TNR)VERSIONMAP=$(MISC)$/$(SHL$(TNR)VERSIONMAP:b)_$(SHL$(TNR)TARGET)$(SHL$(TNR)VERSIONMAP:e)
.IF "$(OS)"!="MACOSX"
.IF "$(OS)"!="IRIX"
SHL$(TNR)VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL$(TNR)VERSIONMAP)
.ENDIF
.ENDIF

$(USE_SHL$(TNR)VERSIONMAP): $(SHL$(TNR)VERSIONMAP)
    @+-$(RM) -f $@ >& $(NULLDEV)
.IF "$(COMID)"=="gcc3"
    +tr -d "\015" < $(SHL$(TNR)VERSIONMAP) | $(AWK) -f $(SOLARENV)$/bin$/addsym.awk > $@
.ELSE           # "$(COMID)"=="gcc3"
    +tr -d "\015" < $(SHL$(TNR)VERSIONMAP) > $@
.ENDIF          # "$(COMID)"=="gcc3"
    @+chmod a+w $@
    
.ENDIF			# "$(SHL$(TNR)VERSIONMAP)"!=""
.ENDIF			# "$(USE_SHL$(TNR)VERSIONMAP)"!=""
.ENDIF			# "$(GUI)" != "UNX"

.IF "$(UNIXVERSIONNAMES)"!=""
.IF "$(OS)"!="MACOSX"
.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="IRIX"
SHL$(TNR)SONAME=$(SONAME_SWITCH)$(SHL$(TNR)TARGETN:b:b)
.ELSE
SHL$(TNR)SONAME=\"$(SONAME_SWITCH)$(SHL$(TNR)TARGETN:b:b)\"
.ENDIF
.ENDIF			# "$(GUI)"!="UNX"
.ENDIF			# "$(OS)"!="MACOSX"
.ENDIF			# "$(UNIXVERSIONNAMES)"!=""

.IF "$(NO_REC_RES)"!=""
.IF "$(SHL$(TNR)RES)"!=""
SHL$(TNR)RES!:=$(subst,$(RES)$/,$(RES)$/$(defaultlangext)$/ $(SHL$(TNR)RES))
SHL$(TNR)ALLRES+=$(SHL$(TNR)RES)
SHL$(TNR)LINKRES*=$(MISC)$/$(SHL$(TNR)TARGET).res
.ENDIF			# "$(SHL$(TNR)RES)"!=""
.ENDIF

.IF "$(SHL$(TNR)DEFAULTRES)$(use_shl_versions)"!=""
SHL$(TNR)DEFAULTRES*=$(MISC)$/$(SHL$(TNR)TARGET)_def.res
SHL$(TNR)ALLRES+=$(SHL$(TNR)DEFAULTRES)
SHL$(TNR)LINKRES*=$(MISC)$/$(SHL$(TNR)TARGET).res
.ENDIF			# "$(SHL$(TNR)DEFAULTRES)$(use_shl_versions)"!=""

.IF "$(NO_SHL$(TNR)DESCRIPTION)"==""
#SHL$(TNR)DESCRIPTIONOBJ*=$(SLO)$/default_description.obj
SHL$(TNR)DESCRIPTIONOBJ*=$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL$(TNR)TARGET))}_description.obj
.ENDIF			# "$(NO_SHL$(TNR)DESCRIPTION)"==""

#.IF "$(SHL$(TNR)TARGETN)"!=""

.IF "$(linkinc)"!=""
.IF "$(GUI)"=="WNT"
$(MISC)$/$(SHL$(TNR)TARGET)_linkinc.ls .PHONY:
    @+-$(RM) $@ >& $(NULLDEV)
    +sed -f $(COMMON_ENV_TOOLS)\chrel.sed $(foreach,i,$(SHL$(TNR)LIBS) $(i:s/.lib/.lin/)) >> $@
.ENDIF

LINKINCTARGETS+=$(MISC)$/$(SHL$(TNR)TARGETN:b)_linkinc.ls
$(SHL$(TNR)TARGETN) : $(LINKINCTARGETS)

.ELSE
$(MISC)$/%linkinc.ls:
    echo . > $@
.ENDIF          # "$(linkinc)"!=""

$(SHL$(TNR)TARGETN) : \
                    $(SHL$(TNR)OBJS)\
                    $(SHL$(TNR)DESCRIPTIONOBJ)\
                    $(SHL$(TNR)LIBS)\
                    $(USE_$(TNR)IMPLIB_DEPS)\
                    $(USE_SHL$(TNR)DEF)\
                    $(USE_SHL$(TNR)VERSIONMAP)\
                    $(SHL$(TNR)RES)\
                    $(SHL$(TNR)VERSIONH)\
                    $(SHL$(TNR)DEPN)
    @echo ------------------------------
    @echo Making: $(SHL$(TNR)TARGETN)
.IF "$(UPDATER)"=="YES"
        @-+$(RM) $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL$(TNR)TARGET))}_version.obj 
.ENDIF
.IF "$(GUI)" == "WNT"
.IF "$(UPDATER)"=="YES"
.IF "$(COM)"=="GCC"
            $(CXX) -c -o$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL$(TNR)TARGET))}_version.obj -DWNT $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
.ELSE
            $(CXX) -c -Fo$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL$(TNR)TARGET))}_version.obj -DWNT $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(UPDATER)"=="YES"
.IF "$(SHL$(TNR)DEFAULTRES)"!=""
    @+-$(RM) $(MISC)$/$(SHL$(TNR)DEFAULTRES:b).rc >& $(NULLDEV)
.IF "$(SHL$(TNR)ICON)" != ""
    @-+echo 1 ICON $(SHL$(TNR)ICON) >> $(MISC)$/$(SHL$(TNR)DEFAULTRES:b).rc
.ENDIF
.IF "$(use_shl_versions)" != ""
.IF "$(USE_SHELL)"!="4nt"
    @-+echo \#define VERVARIANT	$(BUILD) >> $(MISC)$/$(SHL$(TNR)DEFAULTRES:b).rc
    @-+echo \#define ORG_NAME	$(SHL$(TNR)TARGET)$(DLLPOST) >> $(MISC)$/$(SHL$(TNR)DEFAULTRES:b).rc
    @-+echo \#define INTERNAL_NAME $(SHL$(TNR)TARGET:b) >> $(MISC)$/$(SHL$(TNR)DEFAULTRES:b).rc
     @-+echo \#include \"shlinfo.rc\" >> $(MISC)$/$(SHL$(TNR)DEFAULTRES:b).rc
.ELSE			# "$(USE_SHELL)"!="4nt"
    @-+echo #define VERVARIANT	$(BUILD) >> $(MISC)$/$(SHL$(TNR)DEFAULTRES:b).rc
    @-+echo #define ORG_NAME	$(SHL$(TNR)TARGET)$(DLLPOST) >> $(MISC)$/$(SHL$(TNR)DEFAULTRES:b).rc
    @-+echo #define INTERNAL_NAME $(SHL$(TNR)TARGET:b) >> $(MISC)$/$(SHL$(TNR)DEFAULTRES:b).rc
     @-+echo #include "shlinfo.rc" >> $(MISC)$/$(SHL$(TNR)DEFAULTRES:b).rc
.ENDIF			# "$(USE_SHELL)"!="4nt"
.ENDIF			# "$(use_shl_versions)" != ""
    $(RC) -DWIN32 -I$(SOLARTESDIR) $(INCLUDE) $(RCLINKFLAGS) $(MISC)$/$(SHL$(TNR)DEFAULTRES:b).rc
.ENDIF			# "$(SHL$(TNR)DEFAULTRES)"!=""
.IF "$(SHL$(TNR)ALLRES)"!=""
.IF "$(USE_SHELL)"=="4nt"
    +$(COPY) /b $(SHL$(TNR)ALLRES:s/res /res+/) $(SHL$(TNR)LINKRES)
.ELSE			# "$(USE_SHELL)"=="4nt"
    +$(TYPE) $(SHL$(TNR)ALLRES) > $(SHL$(TNR)LINKRES)
.ENDIF			# "$(USE_SHELL)"=="4nt"
.ENDIF			# "$(SHL$(TNR)ALLRES)"!=""
.IF "$(linkinc)"==""
.IF "$(USE_DEFFILE)"!=""
.IF "$(COM)"=="GCC"
    @+echo $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) -o$@ \
        $(STDOBJ) $(SHL$(TNR)VERSIONOBJ) $(SHL$(TNR)DESCRIPTIONOBJ) | tr -d ï\r\nï > $(MISC)$/$(@:b).cmd
    @+$(TYPE) $(SHL$(TNR)LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$/$(ROUT)\#g | tr -d ï\r\nï >> $(MISC)$/$(@:b).cmd
    @+echo  $(SHL$(TNR)STDLIBS) $(STDSHL) $(STDSHL$(TNR)) $(SHL$(TNR)RES) >> $(MISC)$/$(@:b).cmd
    $(MISC)$/$(@:b).cmd
.ELSE
    $(LINK) @$(mktmp \
        $(LINKFLAGS) \
        $(LINKFLAGSSHL) \
        $(SHL$(TNR)STACK) $(SHL$(TNR)BASEX)	\
        -out:$@ \
        -map:$(MISC)$/$(@:b).map \
        -def:$(SHL$(TNR)DEF) \
        $(USE_$(TNR)IMPLIB) \
        $(STDOBJ) \
        $(SHL$(TNR)VERSIONOBJ) $(SHL$(TNR)DESCRIPTIONOBJ) $(SHL$(TNR)OBJS) \
        $(SHL$(TNR)LIBS) \
        $(SHL$(TNR)STDLIBS) \
        $(STDSHL) $(STDSHL$(TNR)) \
        $(SHL$(TNR)LINKRES) \
    )
.ENDIF			# "$(COM)"=="GCC"
.ELSE			# "$(USE_DEFFILE)"!=""
    $(LINK) @$(mktmp	$(LINKFLAGS)			\
        $(LINKFLAGSSHL) $(SHL$(TNR)BASEX)		\
        $(SHL$(TNR)STACK) -out:$(SHL$(TNR)TARGETN)	\
        -map:$(MISC)$/$(@:B).map				\
        $(LB)$/$(SHL$(TNR)IMPLIB).exp				\
        $(STDOBJ)							\
        $(SHL$(TNR)OBJS) $(SHL$(TNR)VERSIONOBJ) $(SHL$(TNR)DESCRIPTIONOBJ)   \
        $(SHL$(TNR)LIBS)                         \
        $(SHL$(TNR)STDLIBS)                      \
        $(STDSHL) $(STDSHL$(TNR))                           \
        $(SHL$(TNR)LINKRES) \
    )
.ENDIF			# "$(USE_DEFFILE)"!=""
.ELSE			# "$(linkinc)"==""
        +-$(RM) del $(MISC)$/$(SHL$(TNR)TARGET).lnk
        +-$(RM) $(MISC)$/$(SHL$(TNR)TARGET).lst
        +$(TYPE) $(mktmp \
        $(LINKFLAGS) \
        $(LINKFLAGSSHL) $(SHL$(TNR)BASEX) \
        $(SHL$(TNR)STACK) $(MAPFILE) \
        -out:$@ \
        $(LB)$/$(SHL$(TNR)IMPLIB).exp \
        $(STDOBJ) \
        $(SHL$(TNR)OBJS) \
        $(SHL$(TNR)STDLIBS) \
        $(STDSHL) $(STDSHL$(TNR)) \
        $(SHL$(TNR)LINKRES) \
        ) >> $(MISC)$/$(SHL$(TNR)TARGET).lnk
        +$(TYPE) $(MISC)$/$(SHL$(TNR)TARGETN:b)_linkinc.ls  >> $(MISC)$/$(SHL$(TNR)TARGET).lnk
        $(LINK) @$(MISC)$/$(SHL$(TNR)TARGET).lnk
.ENDIF			# "$(linkinc)"==""
.ENDIF			# "$(GUI)" == "WNT"
.IF "$(GUI)"=="UNX"
.IF "$(UPDATER)"=="YES"
.IF "$(OS)"=="SOLARIS"
.IF "$(COM)"=="GCC"
        $(CC) -c -fPIC -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL$(TNR)TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
.ELSE		
        $(CC) -c -KPIC -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL$(TNR)TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
.ENDIF		
.ENDIF
.IF "$(OS)"=="MACOSX"
        $(CC) -c -dynamic -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL$(TNR)TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
    .IF "$(CVER)"=="C295"
        @echo "------------------------------"
        @echo "Updating static data member initializations"
        @+dmake -f $(SOLARENV)$/$(OUTPATH)$/inc/makefile.mk $(MFLAGS) $(CALLMACROS) "PRJ=$(PRJ)" "PRJNAME=$(PRJNAME)" "TARGET=$(TARGET)"
    .ENDIF
.ENDIF
.IF "$(OS)"=="LINUX" || "$(OS)"=="NETBSD" || "$(OS)"=="FREEBSD"
        $(CC) -c -fPIC -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL$(TNR)TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
.ENDIF
.IF "$(OS)"=="IRIX"
        $(CC) -c fpic -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL$(TNR)TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
        @+if ( ! -e $(SOLARLIBDIR) ) mkdir $(SOLARLIBDIR)
        @+if ( ! -e $(SOLARLIBDIR)/so_locations ) touch $(SOLARLIBDIR)/so_locations
.ENDIF			# "$(OS)"=="IRIX"
.ENDIF
.IF "$(OS)"=="MACOSX"
    @+-$(RM) $(MISC)$/$(@:b).list
    @+-$(RM) $(MISC)$/$(@:b).cmd
    @+echo $(STDSLO) $(SHL$(TNR)OBJS:s/.obj/.o/) \
    $(SHL$(TNR)VERSIONOBJ) $(SHL$(TNR)DESCRIPTIONOBJ:s/.obj/.o/) \
    `cat /dev/null $(SHL$(TNR)LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` | tr -s " " "\n" > $(MISC)$/$(@:b).list
    @+echo $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) -L$(PRJ)$/$(ROUT)$/lib $(SOLARLIB) -o $@ \
    `dylib-link-list $(PRJNAME) $(SOLARVERSION)$/$(INPATH)$/lib $(PRJ)$/$(INPATH)$/lib $(SHL$(TNR)STDLIBS)` \
    $(SHL$(TNR)STDLIBS) $(SHL$(TNR)ARCHIVES) $(STDSHL) $(STDSHL$(TNR)) -filelist $(MISC)$/$(@:b).list $(LINKOUTPUT_FILTER) > $(MISC)$/$(@:b).cmd
    @cat $(MISC)$/$(@:b).cmd
    @+source $(MISC)$/$(@:b).cmd
    .IF "$(CVER)"=="C295"
        # This is a hack as libstatic and libcppuhelper have a circular dependency
        .IF "$(PRJNAME)"=="cppuhelper"
            @echo "------------------------------"
            @echo "Rerunning static data member initializations"
            @+dmake -u -f $(SOLARENV)$/$(OUTPATH)$/inc/makefile.mk $(MFLAGS) $(CALLMACROS) "PRJ=$(PRJ)" "PRJNAME=$(PRJNAME)" "TARGET=$(TARGET)"
        .ENDIF
    .ENDIF
.IF "$(SHL$(TNR)VERSIONMAP)"!=""
.IF "$(DEBUG)"==""
    @strip -i -r -u -S -s $(SHL$(TNR)VERSIONMAP) $@
.ENDIF
.ENDIF
    @echo "Making: $@.jnilib"
    @create-bundle $@
.IF "$(UPDATER)"=="YES"
.IF "$(SHL$(TNR)NOCHECK)"==""
    +$(SOLARENV)$/bin$/checkdll.sh -L$(LB) $(SOLARLIB) $(SHL$(TNR)TARGETN)
.ENDIF				# "$(SHL$(TNR)NOCHECK)"!=""
.ENDIF
.ELSE			# "$(OS)"=="MACOSX"
    @+-$(RM) $(MISC)$/$(@:b).cmd
    @+echo $(LINK) $(LINKFLAGS) $(SHL$(TNR)SONAME) $(LINKFLAGSSHL) $(SHL$(TNR)VERSIONMAPPARA) -L$(PRJ)$/$(ROUT)$/lib $(SOLARLIB) $(STDSLO) $(SHL$(TNR)OBJS:s/.obj/.o/) \
    $(SHL$(TNR)VERSIONOBJ) $(SHL$(TNR)DESCRIPTIONOBJ:s/.obj/.o/) -o $@ \
    `cat /dev/null $(SHL$(TNR)LIBS) | tr -s " " "\n" | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` \
    $(SHL$(TNR)STDLIBS) $(SHL$(TNR)ARCHIVES) $(STDSHL) $(STDSHL$(TNR)) $(LINKOUTPUT_FILTER) > $(MISC)$/$(@:b).cmd
    @cat $(MISC)$/$(@:b).cmd
    @+source $(MISC)$/$(@:b).cmd
.IF "$(UPDATER)"=="YES"
.IF "$(SHL$(TNR)NOCHECK)"==""
    +-$(RM) $(SHL$(TNR)TARGETN:d)check_$(SHL$(TNR)TARGETN:f)
    +$(RENAME) $(SHL$(TNR)TARGETN) $(SHL$(TNR)TARGETN:d)check_$(SHL$(TNR)TARGETN:f)
    +$(SOLARENV)$/bin$/checkdll.sh -L$(LB) $(SOLARLIB:s/2.6//) $(SHL$(TNR)TARGETN:d)check_$(SHL$(TNR)TARGETN:f)
.ENDIF				# "$(SHL$(TNR)NOCHECK)"!=""
.ENDIF			# "$(UPDATER)"=="YES"
.ENDIF			# "$(OS)"=="MACOSX"
.IF "$(UNIXVERSIONNAMES)"!=""
    +$(RM) $(LB)$/$(SHL$(TNR)TARGETN:b:b:b)
    +$(RM) $(LB)$/$(SHL$(TNR)TARGETN:b:b)
    +cd $(LB) && ln -s $(SHL$(TNR)TARGETN:f) $(SHL$(TNR)TARGETN:b:b)
    +cd $(LB) && ln -s $(SHL$(TNR)TARGETN:f:b:b) $(SHL$(TNR)TARGETN:b:b:b)
.ENDIF			# "$(UNIXVERSIONNAMES)"!=""
    @ls -l $@
.ENDIF			# "$(GUI)" == "UNX"
.IF "$(GUI)"=="MAC"
    @+-$(RM) $@ $@.xSYM
    $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) $(foreach,i,$(shell $(UNIX2MACPATH) $(PRJ)$/$(ROUT)$/lib $(SOLARLIB:s/-L//)) -L"$i") $(shell $(UNIX2MACPATH) $(STDSLO) $(SHL$(TNR)OBJS) `cat /dev/null $(SHL$(TNR)LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` $(SHL$(TNR)VERSIONOBJ) $(SHL$(TNR)DESCRIPTIONOBJ)) $(SHL$(TNR)STDLIBS) $(SHL$(TNR)ARCHIVES) $(STDSHL) $(STDSHL$(TNR)) $(LINKOUTPUT_FILTER) -o $(shell $(UNIX2MACPATH) $@)
.ENDIF			# "$(GUI)"=="MAC"
.IF "$(TARGETTHREAD)"!="MT"
    @+echo ----------------------------------------------------------
    @+echo -
    @+echo - THREAD WARNING! - this library was linked single threaded 
    @+echo - and must not be used in any office installation!
    @+echo -
    @+echo ----------------------------------------------------------
.ENDIF			# "$(TARGETTHREAD)"!="MT"

.IF "$(TESTDIR)"!=""
.IF "$(NO_TESTS)"==""

ALLTAR : runtest_$(SHL$(TNR)TARGET)

runtest_$(SHL$(TNR)TARGET) : $(SHL$(TNR)TARGETN)
    testshl $(SHL$(TNR)TARGETN) sce$/$(SHL$(TNR)TARGET).sce -msg -skip
.ENDIF			# "$(NO_TESTS)"==""
.ENDIF			# "$(TESTDIR)"!=""
.ENDIF			# "$(SHL$(TNR)TARGETN)"!=""

# Anweisungen fuer das Linken
# unroll end
#######################################################

.ENDIF	# MULTI_SHL_FLAG

#-------------------------------------------------------------------------


.IF "$(MULTI_SHLIMP_FLAG)"==""
# MULTITARGET: SHLNIMP --- hier einfuegen

$(SHL1IMPLIBN) .NULL : SHL1IMP

$(SHL2IMPLIBN) .NULL : SHL2IMP

$(SHL3IMPLIBN) .NULL : SHL3IMP

$(SHL4IMPLIBN) .NULL : SHL4IMP

$(SHL5IMPLIBN) .NULL : SHL5IMP

$(SHL6IMPLIBN) .NULL : SHL6IMP

$(SHL7IMPLIBN) .NULL : SHL7IMP

$(SHL8IMPLIBN) .NULL : SHL8IMP

$(SHL9IMPLIBN) .NULL : SHL9IMP
.ENDIF



.IF "$(MULTI_SHLIMP_FLAG)"==""

# MULTITARGET: SHLNIMP --- hier einfuegen

SHL1IMP SHL2IMP SHL3IMP SHL4IMP SHL5IMP SHL6IMP SHL7IMP SHL8IMP SHL9IMP:
.IF "$(GUI)" != "MAC"
    @dmake $(SHL$(TNR)IMPLIBN) MULTI_SHLIMP_FLAG=true TNR:=$(TNR) $(MFLAGS) $(CALLMACROS)
.ELSE
    @dmake "$(SHL$(TNR)IMPLIBN)" MULTI_SHLIMP_FLAG=true TNR:=$(TNR) $(MFLAGS) $(CALLMACROS)
.ENDIF
.ELSE


#######################################################
# Anweisungen fuer das Linken
# unroll begin

.IF "$(SHL$(TNR)IMPLIBN)" != ""

.IF "$(UPDATER)"=="YES"
USELIB$(TNR)DEPN+=$(SHL$(TNR)LIBS)
.ELSE
.ENDIF

.IF "$(USE_DEFFILE)"!=""
USE_SHL$(TNR)TARGET=$(SHL$(TNR)TARGETN)
.ENDIF

.IF "$(GUI)" != "UNX"
$(SHL$(TNR)IMPLIBN):	\
                    $(SHL$(TNR)DEF) \
                    $(USE_SHL$(TNR)TARGET) \
                    $(USELIB$(TNR)DEPN) \
                    $(USELIBDEPN)
.ELSE
$(SHL$(TNR)IMPLIBN):	\
                    $(SHL$(TNR)LIBS)
.ENDIF
    @echo ------------------------------
    @echo Making: $(SHL$(TNR)IMPLIBN)
.IF "$(GUI)" == "WNT"
# bei use_deffile implib von linker erstellt
.IF "$(USE_DEFFILE)"==""
    $(IMPLIB) $(IMPLIBFLAGS) @$(mktmp -out:$(SHL$(TNR)IMPLIBN) \
    -def:$(SHL$(TNR)DEF) )
.ELSE			# "$(GUI)" == "WNT"
    +$(TOUCH) $@
    +@echo build of $(SHL$(TNR)TARGETN) creates $@
.ENDIF			# "$(GUI)" == "WNT"
.ELSE
.IF "$(GUI)" == "WIN" || "$(GUI)" == "OS2"
.IF "$(USE_DEFFILE)"==""
    $(IMPLIB) $(IMPLIBFLAGS) $@ $(SHL$(TNR)DEF)
.ELSE
    $(IMPLIB) $(IMPLIBFLAGS) $@ $(SHL$(TNR)TARGETN)
.ENDIF
.ELSE
    @echo no ImportLibs on Mac and *ix
    @+-$(RM) $@
    @$(TOUCH) $@
.ENDIF
.ENDIF
.ENDIF

# Anweisungen fuer das Linken
# unroll end
#######################################################

.ENDIF	# MULTI_SHLIMP_FLAG

#------------------------------------------------------------------------


