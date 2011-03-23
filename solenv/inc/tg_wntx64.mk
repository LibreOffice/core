#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************
###############################################
#        Windows 64 bit special
#	only used for 64 bit shell extension
#          ( Windows Vista )
# ---------------------------------------------
# THIS FILE WILL BE DELETED when a fully ported
# Windows 64 bit version is available
###############################################

.IF "$(BUILD_X64)"!=""

.IF "$(SLOTARGET_X64)"!=""
$(SLOTARGET_X64): $(SLOFILES_X64) $(IDLSLOFILES_X64)
.IF "$(MDB)" != ""
    @echo $(SLOTARGET_X64)
    @echo $(&:+"\n")
.ENDIF
    @echo ------------------------------
    @echo Making: $@
    @-$(MKDIR) $(SLB_X64)
.IF "$(LIBTARGET)"!="NO"
    @-$(TYPE) $(mktmp $(&:+"\n")) > $(@:s/.lib/.lin/)
.ENDIF          # "$(LIBTARGET)"!="NO"
    $(LIBMGR_X64) $(LIBFLAGS_X64) /OUT:$@ @$(mktmp $(&:+"\n"))
.ENDIF			# "$(SLOTARGET_X64)"!=""


# decide how to link
.IF "$(SHL1CODETYPE)"=="C"
SHL1LINKFLAGS_X64+=$(LINKCFLAGS_X64)
.ELSE			# "$(SHL1CODETYPE)"=="C"
SHL1LINKFLAGS_X64+=$(LINKFLAGS_X64)
.ENDIF			# "$(SHL1CODETYPE)"=="C"

.IF "$(SHL2CODETYPE)"=="C"
SHL2LINKFLAGS_X64+=$(LINKCFLAGS_X64)
.ELSE			# "$(SHL2CODETYPE)"=="C"
SHL2LINKFLAGS_X64+=$(LINKFLAGS_X64)
.ENDIF			# "$(SHL2CODETYPE)"=="C"

.IF "$(SHL1IMPLIB_X64)" == ""
SHL1IMPLIB_X64=i$(TARGET)_t1
.ENDIF			# "$(SHL1IMPLIB_X64)" == ""
USE_1IMPLIB_X64=-implib:$(LB_X64)/$(SHL1IMPLIB_X64).lib
.IF "$(SHL1TARGET_X64)"!=""
SHL1IMPLIBN_X64=$(LB_X64)/$(SHL1IMPLIB_X64).lib
.ENDIF # "$(SLOFILES_X64)$(OBJFILES_X64)"!=""

make_x64_dirs :
    -$(MKDIR) $(BIN_X64)
    -$(MKDIR) $(MISC_X64)
    -$(MKDIR) $(LB_X64)
    -$(MKDIR) $(SLB_X64)


.IF "$(LIBTARGET)" == ""
ALLTAR : $(SHL1IMPLIBN_X64)
.ENDIF "$(LIBTARGET)" == ""

# ----------------------------- lib ------------------------------------------
.IF "$(LIB1TARGET_X64)" != ""
$(LIB1TARGET_X64) :	$(LIB1FILES_X64) \
                        $(LIB1OBJFILES_X64) \
                        $(LIB1DEPN_X64)
    @echo using: $(LIB1FILES_X64)
    @echo using: $(LIB1TARGET_X64)
    @echo ------------------------------
    @echo Making: $@
    -$(MKDIR) $(SLB_X64)
    @@-$(RM) $@
    $(LIBMGR_X64) $(LIBFLAGS_X64) /OUT:$@ @$(mktmp $(LIB1FILES_X64) $(LIB1OBJFILES_X64))
    @-$(RM) $(@:s/.lib/.lin/)
.IF "$(LIB1OBJFILES_X64)"!=""
    @$(TYPE) $(mktmp $(LIB1OBJFILES_X64)) > $(null,$(LIB1OBJFILES_X64) $(NULLDEV) $(@:s/.lib/.lin/))
.ENDIF          # "$(LIB1OBJFILES_X64)"!=""
.IF "$(LIB1FILES_X64)"!=""
    @-$(TYPE) $(foreach,i,$(LIB1FILES_X64) $(i:s/.lib/.lin/)) >> $(@:s/.lib/.lin/)
.ENDIF          # "$(LIB1FILES_X64)"!=""
    @$(ECHONL)
.ENDIF          # "$(LIB1TARGET_X64)" != ""

.IF "$(LIB2TARGET_X64)" != ""
$(LIB2TARGET_X64) :	$(LIB2FILES_X64) \
                        $(LIB2OBJFILES_X64) \
                        $(LIB2DEPN_X64)
    @echo using: $(LIB2FILES_X64)
    @echo using: $(LIB2TARGET_X64)
    @echo ------------------------------
    @echo Making: $@
    -$(MKDIR) $(SLB_X64)
    @@-$(RM) $@
    $(LIBMGR_X64) $(LIBFLAGS_X64) /OUT:$@ @$(mktmp $(LIB2FILES_X64) $(LIB2OBJFILES_X64))
    @-$(RM) $(@:s/.lib/.lin/)
.IF "$(LIB2OBJFILES_X64)"!=""
    @$(TYPE) $(mktmp $(LIB2OBJFILES_X64)) > $(null,$(LIB2OBJFILES_X64) $(NULLDEV) $(@:s/.lib/.lin/))
.ENDIF          # "$(LIB2OBJFILES_X64)"!=""
.IF "$(LIB2FILES_X64)"!=""
    @-$(TYPE) $(foreach,i,$(LIB2FILES_X64) $(i:s/.lib/.lin/)) >> $(@:s/.lib/.lin/)
.ENDIF          # "$(LIB2FILES_X64)"!=""
    @$(ECHONL)
.ENDIF          # "$(LIB2TARGET_X64)" != ""

# -------------- def ----------------------
.IF	"$(DEF1TARGETN_X64)"!=""
$(DEF1TARGETN_X64) : \
        $(DEF1DEPN_X64) \
        $(DEF1EXPORTFILE_X64)
        @-$(MKDIR) $(MISC_X64)
    @-$(RM) $@.tmpfile
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo LIBRARY	  $(EMQ)"$(SHL1TARGETN:f)$(EMQ)" 								 >$@.tmpfile
    @echo HEAPSIZE	  0 											>>$@.tmpfile
    @echo EXPORTS													>>$@.tmpfile
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@.tmpfile
.IF "$(DEFLIB1NAME_X64)"!=""
.IF "$(SHL1USE_EXPORTS_X64)"==""
    @-$(EXPORT1_PROTECT) $(RMHACK1) $(MISC_X64)/$(SHL1TARGET_X64).exp
    @$(EXPORT1_PROTECT) $(LIBMGR_X64) -EXTRACT:/ /OUT:$(MISC_X64)/$(SHL1TARGET_X64).exp $(SLB_X64)/$(DEFLIB1NAME_X64).lib
    @$(EXPORT1_PROTECT) $(LDUMP2) -E 20 -F $(MISC_X64)/$(SHL1TARGET_X64).flt $(MISC_X64)/$(SHL1TARGET_X64).exp			   >>$@.tmpfile
    $(EXPORT1_PROTECT) $(RMHACK1) $(MISC_X64)/$(SHL1TARGET_X64).exp
.ELSE			# "$(SHL1USE_EXPORTS_X64)"==""
    @$(EXPORT1_PROTECT) $(DUMPBIN) -DIRECTIVES  $(foreach,i,$(DEFLIB1NAME_X64) $(SLB_X64)/$(i).lib) | $(GREP) EXPORT: > $(MISC_X64)/$(SHL1TARGET_X64).direct
.IF "$(SHL1USE_EXPORTS_X64)"!="name"
    @$(EXPORT1_PROTECT) $(LDUMP2) -D -E 20 -F $(DEF1FILTER_X64) $(MISC_X64)/$(SHL1TARGET_X64).direct >>$@.tmpfile
.ELSE			# "$(SHL1USE_EXPORTS_X64)"!="name"
    @$(EXPORT1_PROTECT) $(LDUMP2) -N -D -E 20 -F $(DEF1FILTER_X64) $(MISC_X64)/$(SHL1TARGET_X64).direct >>$@.tmpfile
.ENDIF			# "$(SHL1USE_EXPORTS_X64)"!="name"
.ENDIF			# "$(SHL1USE_EXPORTS)"==""
.ENDIF				# "$(DEFLIB1NAME)"!=""
.IF "$(DEF1EXPORT1_X64)"!=""
    @echo $(DEF1EXPORT1_X64)										>>$@.tmpfile
.ENDIF
.IF "$(DEF1EXPORTFILE_X64)"!=""
    $(COMMAND_ECHO)$(TYPE) $(DEF1EXPORTFILE_X64) >> $@.tmpfile
.ENDIF
    @-$(RM) $@
    @$(RENAME) $@.tmpfile $@
.ENDIF 		# "$(DEF1TARGETN_X64)"!=""

.IF	"$(DEF2TARGETN_X64)"!=""
$(DEF2TARGETN_X64) : \
        $(DEF2DEPN_X64) \
        $(DEF2EXPORTFILE_X64)
    @-$(MKDIR) $(MISC_X64)
    @-$(RM) $@.tmpfile
    @echo Making Module-Definitionfile : $@
    @echo LIBRARY	  $(EMQ)"$(SHL2TARGETN:f)$(EMQ)" 								 >$@.tmpfile
    @echo HEAPSIZE	  0 											>>$@.tmpfile
    @echo EXPORTS													>>$@.tmpfile
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@.tmpfile
.IF "$(DEFLIB2NAME_X64)"!=""
.IF "$(SHL2USE_EXPORTS_X64)"==""
    @-$(EXPORT2_PROTECT) $(RMHACK1) $(MISC_X64)/$(SHL2TARGET_X64).exp
    @$(EXPORT2_PROTECT) $(LIBMGR_X64) -EXTRACT:/ /OUT:$(MISC_X64)/$(SHL2TARGET_X64).exp $(SLB_X64)/$(DEFLIB2NAME_X64).lib
    @$(EXPORT2_PROTECT) $(LDUMP2) -E 20 -F $(MISC_X64)/$(SHL2TARGET_X64).flt $(MISC_X64)/$(SHL2TARGET_X64).exp			   >>$@.tmpfile
    $(EXPORT2_PROTECT) $(RMHACK1) $(MISC_X64)/$(SHL2TARGET_X64).exp
.ELSE			# "$(SHL2USE_EXPORTS_X64)"==""
    @$(EXPORT2_PROTECT) $(DUMPBIN) -DIRECTIVES  $(foreach,i,$(DEFLIB2NAME_X64) $(SLB_X64)/$(i).lib) | $(GREP) EXPORT: > $(MISC_X64)/$(SHL2TARGET_X64).direct
.IF "$(SHL2USE_EXPORTS_X64)"!="name"
    @$(EXPORT2_PROTECT) $(LDUMP2) -D -E 20 -F $(DEF2FILTER_X64) $(MISC_X64)/$(SHL2TARGET_X64).direct >>$@.tmpfile
.ELSE			# "$(SHL2USE_EXPORTS_X64)"!="name"
    @$(EXPORT2_PROTECT) $(LDUMP2) -N -D -E 20 -F $(DEF2FILTER_X64) $(MISC_X64)/$(SHL2TARGET_X64).direct >>$@.tmpfile
.ENDIF			# "$(SHL2USE_EXPORTS_X64)"!="name"
.ENDIF			# "$(SHL2USE_EXPORTS)"==""
.ENDIF				# "$(DEFLIB1NAME)"!=""
.IF "$(DEF2EXPORT2_X64)"!=""
    @echo $(DEF2EXPORT2_X64)										>>$@.tmpfile
.ENDIF
.IF "$(DEF2EXPORTFILE_X64)"!=""
    $(COMMAND_ECHO)$(TYPE) $(DEF2EXPORTFILE_X64) >> $@.tmpfile
.ENDIF
    @-$(RM) $@
    @$(RENAME) $@.tmpfile $@
.ENDIF 		# "$(DEF2TARGETN_X64)"!=""
#-----------------------------------------------------------------------------

# -------------------------------- shl -----------------------------------------

.IF "$(SHL1TARGET_X64)"!=""
.IF "$(VERSIONOBJ_X64)"!=""
SHL1VERSIONOBJ_X64:=$(VERSIONOBJ_X64:d){$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL1TARGET_X64))}$(VERSIONOBJ_X64:f)
USE_VERSIONH_X64:=$(INCCOM)/$(SHL1VERSIONOBJ_X64:b).h
SHL1VERSIONOBJDEP_X64:=$(VERSIONOBJ_X64:d){$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL1TARGET_X64))}$(VERSIONOBJ_X64:f)
$(MISC_X64)/$(SHL1VERSIONOBJ_X64:b).c : make_x64_dirs $(SOLARENV)/src/version.c $(INCCOM)/$(SHL1VERSIONOBJ_X64:b).h
    $(COMMAND_ECHO)$(TYPE) $(SOLARENV)/src/version.c | $(SED) s/_version.h/$(SHL1VERSIONOBJ_X64:b).h/ > $@
.INIT : $(SHL1VERSIONOBJDEP_X64)
.ENDIF

.IF "$(USE_DEFFILE_X64)"==""
USE_1IMPLIB_DEPS_X64=$(LB_X64)/$(SHL1IMPLIB_X64).lib
.ENDIF			# "$(USE_DEFFILE_X64)"==""

.IF "$(SHL1RES_X64)"!=""
SHL1ALLRES_X64+=$(SHL1RES_X64)
SHL1LINKRES_X64*=$(MISC_X64)/$(SHL1TARGET_X64).res
SHL1LINKRESO_X64*=$(MISC_X64)/$(SHL1TARGET_X64)_res.o
.ENDIF			# "$(SHL1RES_X64)"!=""

.IF "$(SHL1DEFAULTRES_X64)$(use_shl_versions)"!=""
SHL1DEFAULTRES_X64*=$(MISC_X64)/$(SHL1TARGET_X64)_def.res
SHL1ALLRES_X64+=$(SHL1DEFAULTRES_X64)
SHL1LINKRES_X64*=$(MISC_X64)/$(SHL1TARGET_X64).res
SHL1LINKRESO_X64*=$(MISC_X64)/$(SHL1TARGET_X64)_res.o
.ENDIF			# "$(SHL1DEFAULTRES)$(use_shl_versions)"!=""
.ENDIF # "$(SHL1TARGET_X64)"=!""

.IF "$(SHL1TARGETN_X64)"!=""
$(SHL1TARGETN_X64) : \
                    make_x64_dirs \
                    $(SHL1OBJS_X64)\
                    $(SHL1LIBS_X64)\
                    $(USE_1IMPLIB_DEPS_X64)\
                    $(USE_SHL1DEF_X64)\
                    $(USE_SHL1VERSIONMAP_X64)\
                    $(SHL1RES)\
                    $(SHL1DEPN_X64) \
                    $(SHL1LINKLIST_X64)
    @echo Making: $(SHL1TARGETN_X64)
.IF "$(SHL1DEFAULTRES_X64)"!=""
    @@-$(RM) $(MISC_X64)/$(SHL1DEFAULTRES_X64:b).rc
.IF "$(use_shl_versions)" != ""
.IF "$(SHL1ADD_VERINFO_X64)"!=""
    @echo $(EMQ)#include $(EMQ)"$(SHL1ADD_VERINFO)$(EMQ)" >> $(MISC_X64)/$(SHL1DEFAULTRES_X64:b).rc
.ELSE			# "$(SHL1ADD_VERINFO_X64)"!=""
    @echo $(EMQ)#define ADDITIONAL_VERINFO1 >> $(MISC_X64)/$(SHL1DEFAULTRES_X64:b).rc
    @echo $(EMQ)#define ADDITIONAL_VERINFO2 >> $(MISC_X64)/$(SHL1DEFAULTRES_X64:b).rc
    @echo $(EMQ)#define ADDITIONAL_VERINFO3 >> $(MISC_X64)/$(SHL1DEFAULTRES_X64:b).rc
.ENDIF			# "$(SHL1ADD_VERINFO_X64)"!=""
    @echo $(EMQ)#define VERVARIANT	$(BUILD) >> $(MISC_X64)/$(SHL1DEFAULTRES_X64:b).rc
    @echo $(EMQ)#define ORG_NAME	$(SHL1TARGET_X64)$(DLLPOST) >> $(MISC_X64)/$(SHL1DEFAULTRES_X64:b).rc
    @echo $(EMQ)#define INTERNAL_NAME $(SHL1TARGET_X64:b) >> $(MISC_X64)/$(SHL1DEFAULTRES_X64:b).rc
    @echo $(EMQ)#include $(EMQ)"shlinfo.rc$(EMQ)" >> $(MISC_X64)/$(SHL1DEFAULTRES_X64:b).rc
.ENDIF			# "$(use_shl_versions)" != ""
    $(COMMAND_ECHO)$(RC) -DWIN32 $(INCLUDE) $(RCLINKFLAGS_X64) $(MISC_X64)/$(SHL1DEFAULTRES_X64:b).rc
.ENDIF # "$(SHL1DEFAULTRES_X64)"!=""
.IF "$(SHL1ALLRES_X64)"!=""
    $(COMMAND_ECHO)$(TYPE) $(SHL1ALLRES_X64) > $(SHL1LINKRES_X64)
.ENDIF			# "$(SHL1ALLRES)"!=""
.IF "$(USE_DEFFILE_X64)"!=""
    $(LINK_X64) @$(mktmp \
        $(SHL1LINKFLAGS_X64) \
        $(LINKFLAGSSHL_X64) \
        $(SHL1STACK_X64) $(SHL1BASEX_X64)	\
        -out:$@ \
        -map:$(MISC_X64)/$(@:b).map \
        -def:$(SHL1DEF_X64) \
        $(USE_1IMPLIB_X64) \
        $(STDOBJ_X64) \
        $(SHL1VERSIONOBJ_X64) $(SHL1OBJS_X64) \
        $(SHL1LIBS_X64) \
        $(SHL1STDLIBS_X64) \
        $(SHL1STDSHL_X64) $(STDSHL1_X64) \
        $(SHL1LINKRES_X64) \
    ) $(LINKOUTPUTFILTER_X64)
# double check if target was really written... still making sense?
    @@$(LS) $@
    @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ELSE			# "$(USE_DEFFILE)"!=""
    $(LINK_X64) @$(mktmp	$(SHL1LINKFLAGS_X64)			\
        $(LINKFLAGSSHL_X64) $(SHL1BASEX_X64)		\
        $(SHL1STACK_X64) -out:$(SHL1TARGETN_X64)	\
        -map:$(MISC_X64)/$(@:B).map				\
        $(LB_X64)/$(SHL1IMPLIB_X64).exp				\
        $(STDOBJ_X64)							\
        $(SHL1OBJS_X64) $(SHL1VERSIONOBJ_X64) \
        $(SHL1LIBS_X64)                         \
        $(SHL1STDLIBS_X64)                      \
        $(SHL1STDSHL_X64) $(STDSHL1_X64)                           \
        $(SHL1LINKRES_X64) \
    ) $(LINKOUTPUTFILTER_X64)
# double check if target was really written... still making sense?
    @@$(LS) $@
    @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ENDIF			# "$(USE_DEFFILE)"!=""
# ------------------------------------------------------------------------------
$(SHL1IMPLIBN_X64):	\
                    $(SHL1DEF_X64) \
                    $(USE_SHL1TARGET_X64) \
                    $(USELIB1DEPN_X64) \
                    $(USELIBDEPN_X64)
    @echo Making: $(SHL1IMPLIBN_X64)
# bei use_deffile implib von linker erstellt
    @-mkdir $(LB_X64)
.IF "$(USE_DEFFILE_X64)"==""
    $(IMPLIB_X64) $(IMPLIBFLAGS_X64) @$(mktmp -out:$(SHL1IMPLIBN_X64) \
    -def:$(SHL1DEF_X64) )
.ELSE			# "$(USE_DEFFILE_X64)==""
    @echo build of $(SHL1TARGETN_X64) creates $@
.ENDIF			# "$(USE_DEFFILE_X64)==""

.ENDIF 			# "$(SHL1TARGETN_X64)"!=""

.IF "$(SHL2IMPLIB_X64)" == ""
SHL2IMPLIB_X64=i$(TARGET)_t2
.ENDIF			# "$(SHL2IMPLIB_X64)" == ""
.IF "$(COM)" != "GCC"
USE_2IMPLIB_X64=-implib:$(LB_X64)/$(SHL2IMPLIB_X64).lib
.ENDIF			# "$(COM)" != "GCC"
.IF "$(SHL2TARGET_X64)"!=""
SHL2IMPLIBN_X64=$(LB_X64)/$(SHL2IMPLIB_X64).lib
.ENDIF # "$(SLOFILES_X64)$(OBJFILES_X64)"!=""

.IF "$(LIBTARGET)" == ""
ALLTAR : $(SHL2IMPLIBN_X64)
.ENDIF # "$(LIBTARGET)" == ""

.IF "$(SHL2TARGET_X64)"!=""
.IF "$(VERSIONOBJ_X64)"!=""
SHL2VERSIONOBJ_X64:=$(VERSIONOBJ_X64:d){$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL2TARGET_X64))}$(VERSIONOBJ_X64:f)
USE_VERSIONH_X64:=$(INCCOM)/$(SHL2VERSIONOBJ_X64:b).h
SHL2VERSIONOBJDEP_X64:=$(VERSIONOBJ_X64:d){$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL2TARGET_X64))}$(VERSIONOBJ_X64:f)
$(MISC_X64)/$(SHL2VERSIONOBJ_X64:b).c : make_x64_dirs $(SOLARENV)/src/version.c $(INCCOM)/$(SHL2VERSIONOBJ_X64:b).h
    $(COMMAND_ECHO)$(TYPE) $(SOLARENV)/src/version.c | $(SED) s/_version.h/$(SHL2VERSIONOBJ_X64:b).h/ > $@
.INIT : $(SHL2VERSIONOBJDEP_X64)
.ENDIF

.IF "$(USE_DEFFILE_X64)"==""
USE_2IMPLIB_DEPS_X64=$(LB_X64)/$(SHL2IMPLIB_X64).lib
.ENDIF			# "$(USE_DEFFILE_X64)"==""

.IF "$(SHL2RES_X64)"!=""
SHL2ALLRES_X64+=$(SHL2RES_X64)
SHL2LINKRES_X64*=$(MISC_X64)/$(SHL2TARGET_X64).res
SHL2LINKRESO_X64*=$(MISC_X64)/$(SHL2TARGET_X64)_res.o
.ENDIF			# "$(SHL2RES_X64)"!=""

.IF "$(SHL2DEFAULTRES_X64)$(use_shl_versions)"!=""
SHL2DEFAULTRES_X64*=$(MISC_X64)/$(SHL2TARGET_X64)_def.res
SHL2ALLRES_X64+=$(SHL2DEFAULTRES_X64)
SHL2LINKRES_X64*=$(MISC_X64)/$(SHL2TARGET_X64).res
SHL2LINKRESO_X64*=$(MISC_X64)/$(SHL2TARGET_X64)_res.o
.ENDIF			# "$(SHL2DEFAULTRES)$(use_shl_versions)"!=""
.ENDIF # "$(SHL2TARGET_X64)"=!""

.IF "$(SHL2TARGETN_X64)"!=""
$(SHL2TARGETN_X64) : \
                    make_x64_dirs \
                    $(SHL2OBJS_X64)\
                    $(SHL2LIBS_X64)\
                    $(USE_2IMPLIB_DEPS_X64)\
                    $(USE_SHL2DEF_X64)\
                    $(USE_SHL2VERSIONMAP_X64)\
                    $(SHL2RES)\
                    $(SHL2DEPN_X64) \
                    $(SHL2LINKLIST_X64)
    @echo Making: $(SHL2TARGETN_X64)
.IF "$(SHL2DEFAULTRES_X64)"!=""
    @@-$(RM) $(MISC_X64)/$(SHL2DEFAULTRES_X64:b).rc
.IF "$(use_shl_versions)" != ""
.IF "$(SHL2ADD_VERINFO_X64)"!=""
    @echo $(EMQ)#include $(EMQ)"$(SHL2ADD_VERINFO)$(EMQ)" >> $(MISC_X64)/$(SHL2DEFAULTRES_X64:b).rc
.ELSE			# "$(SHL2ADD_VERINFO_X64)"!=""
    @echo $(EMQ)#define ADDITIONAL_VERINFO1 >> $(MISC_X64)/$(SHL2DEFAULTRES_X64:b).rc
    @echo $(EMQ)#define ADDITIONAL_VERINFO2 >> $(MISC_X64)/$(SHL2DEFAULTRES_X64:b).rc
    @echo $(EMQ)#define ADDITIONAL_VERINFO3 >> $(MISC_X64)/$(SHL2DEFAULTRES_X64:b).rc
.ENDIF			# "$(SHL1ADD_VERINFO_X64)"!=""
    @echo $(EMQ)#define VERVARIANT	$(BUILD) >> $(MISC_X64)/$(SHL2DEFAULTRES_X64:b).rc
    @echo $(EMQ)#define ORG_NAME	$(SHL2TARGET_X64)$(DLLPOST) >> $(MISC_X64)/$(SHL2DEFAULTRES_X64:b).rc
    @echo $(EMQ)#define INTERNAL_NAME $(SHL1TARGET_X64:b) >> $(MISC_X64)/$(SHL2DEFAULTRES_X64:b).rc
    @echo $(EMQ)#include $(EMQ)"shlinfo.rc$(EMQ)" >> $(MISC_X64)/$(SHL2DEFAULTRES_X64:b).rc
.ENDIF			# "$(use_shl_versions)" != ""
    $(COMMAND_ECHO)$(RC) -DWIN32 $(INCLUDE) $(RCLINKFLAGS_X64) $(MISC_X64)/$(SHL2DEFAULTRES_X64:b).rc
.ENDIF # "$(SHL2DEFAULTRES_X64)"!=""
.IF "$(SHL2ALLRES_X64)"!=""
    $(COMMAND_ECHO)$(TYPE) $(SHL2ALLRES_X64) > $(SHL2LINKRES_X64)
.ENDIF			# "$(SHL2ALLRES)"!=""
.IF "$(USE_DEFFILE_X64)"!=""
    $(LINK_X64) @$(mktmp \
        $(SHL2LINKFLAGS_X64) \
        $(LINKFLAGSSHL_X64) \
        $(SHL2STACK_X64) $(SHL2BASEX_X64)	\
        -out:$@ \
        -map:$(MISC_X64)/$(@:b).map \
        -def:$(SHL2DEF_X64) \
        $(USE_2IMPLIB_X64) \
        $(STDOBJ_X64) \
        $(SHL2VERSIONOBJ_X64) $(SHL2OBJS_X64) \
        $(SHL2LIBS_X64) \
        $(SHL2STDLIBS_X64) \
        $(SHL2STDSHL_X64) $(STDSHL2_X64) \
        $(SHL2LINKRES_X64) \
    ) $(LINKOUTPUTFILTER_X64)
# double check if target was really written... still making sense?
    @@$(LS) $@
    @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ELSE			# "$(USE_DEFFILE)"!=""
    $(LINK_X64) @$(mktmp	$(SHL2LINKFLAGS_X64)			\
        $(LINKFLAGSSHL_X64) $(SHL2BASEX_X64)		\
        $(SHL2STACK_X64) -out:$(SHL2TARGETN_X64)	\
        -map:$(MISC_X64)/$(@:B).map				\
        $(LB_X64)/$(SHL2IMPLIB_X64).exp				\
        $(STDOBJ_X64)							\
        $(SHL2OBJS_X64) $(SHL2VERSIONOBJ_X64) \
        $(SHL2LIBS_X64)                         \
        $(SHL2STDLIBS_X64)                      \
        $(SHL2STDSHL_X64) $(STDSHL2_X64)                           \
        $(SHL2LINKRES_X64) \
    ) $(LINKOUTPUTFILTER_X64)
# double check if target was really written... still making sense?
    @@$(LS) $@
    @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ENDIF			# "$(USE_DEFFILE)"!=""

$(SHL2IMPLIBN_X64):	\
                    $(SHL2DEF_X64) \
                    $(USE_SHL2TARGET_X64) \
                    $(USELIB2DEPN_X64) \
                    $(USELIBDEPN_X64)
    @echo Making: $(SHL2IMPLIBN_X64)
# bei use_deffile implib von linker erstellt
    @-mkdir $(LB_X64)
.IF "$(USE_DEFFILE_X64)"==""
    $(IMPLIB_X64) $(IMPLIBFLAGS_X64) @$(mktmp -out:$(SHL2IMPLIBN_X64) \
    -def:$(SHL2DEF_X64) )
.ELSE			# "$(USE_DEFFILE_X64)==""
    @echo build of $(SHL2TARGETN_X64) creates $@
.ENDIF			# "$(USE_DEFFILE_X64)==""

.ENDIF 			# "$(SHL2TARGETN_X64)"!=""


$(SLO_X64)/%.obj : %.cpp
    @echo ------------------------------
    @echo Making: $@
    @@-$(RM) $@ >& $(NULLDEV)
    -$(MKDIR) $(@:d)
    @-$(MKDIR) $(MISC_X64)
    $(CAPTURE_COMMAND) $(CXX_X64) $(USE_CFLAGS_X64) $(INCLUDE_X64) $(CFLAGSCXX_X64) $(CFLAGSSLO_X64) $(USE_CDEFS_X64) $(CDEFSSLO_X64) $(CDEFSMT_X64) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(SLO_X64)/$*.obj $(CFLAGSINCXX)$(PWD)/$*.cpp $(CAPTURE_OUTPUT)

$(SLO_X64)/%.obj : %.cxx
    @echo Making: $@
    @@-$(RM) $@ >& $(NULLDEV)
    $(COMMAND_ECHO)-$(MKDIR) $(@:d)
    $(COMMAND_ECHO)@-$(MKDIR) $(MISC_X64)
    $(COMMAND_ECHO)$(CAPTURE_COMMAND) $(CXX_X64) $(USE_CFLAGS_X64) $(INCLUDE_X64) $(CFLAGSCXX_X64) $(CFLAGSSLO_X64) $(USE_CDEFS_X64) $(CDEFSSLO_X64) $(CDEFSMT_X64) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(SLO_X64)/$*.obj $(CFLAGSINCXX)$(PWD)/$*.cxx $(CAPTURE_OUTPUT)

$(SLO_X64)/%.obj : $(MISC)/%.c
    @echo Making: $@
    $(COMMAND_ECHO)-$(MKDIR) $(@:d)
    $(COMMAND_ECHO)@-$(MKDIR) $(MISC_X64)
    @@-$(RM) $@
    @$(TYPE) $(mktmp $(CC_X64) $(USE_CFLAGS_X64) $(INCLUDE_C) $(CFLAGSCC_X64) $(CFLAGSSLO_X64) $(USE_CDEFS_X64) $(CDEFSSLO_X64) $(CFLAGSAPPEND_X64) $(CFLAGSOUTOBJ)$(SLO_X64)/$*.obj $(MISC)/$*.c )
    @$(ECHONL)
    $(COMMAND_ECHO)$(CC_X64) @$(mktmp $(USE_CFLAGS_X64) $(INCLUDE_C) $(CFLAGSCC_X64) $(CFLAGSSLO_X64) $(USE_CDEFS_X64) $(CDEFSSLO_X64) $(CFLAGSAPPEND_X64) $(CFLAGSOUTOBJ)$(SLO_X64)/$*.obj $(MISC)/$*.c )

$(SLO_X64)/%.obj : %.c
    @echo Making: $@
    $(COMMAND_ECHO)-$(MKDIR) $(@:d)
    $(COMMAND_ECHO)@-$(MKDIR) $(MISC_X64)
    @@-$(RM) $@
    $(COMMAND_ECHO)$(CC_X64) @$(mktmp $(USE_CFLAGS_X64) $(INCLUDE_C) $(CFLAGSCC_X64) $(CFLAGSSLO_X64) $(USE_CDEFS_X64) $(CDEFSSLO_X64) $(CDEFSMT_X64) $(CFLAGSAPPEND_X64) $(CFLAGSOUTOBJ)$(SLO_X64)/$*.obj $*.c )

.ENDIF			# "$(BUILD_X64)"!=""
