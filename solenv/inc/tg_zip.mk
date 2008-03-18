#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: tg_zip.mk,v $
#
#   $Revision: 1.40 $
#
#   last change: $Author: vg $ $Date: 2008-03-18 14:45:34 $
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
# unroll begin

.IF "$(GUI)"=="WNT"
command_seperator=&&
#command_seperator=^
avoid_cvs_dir=-x "*CVS*"
.ELSE
command_seperator=;
avoid_cvs_dir=-x "*CVS*"
.ENDIF

.IF "$(ZIP$(TNR)TARGET)"!=""

ZIP$(TNR)EXT*=.zip
.IF "$(common_build_zip)"!=""
.IF "$(ZIP$(TNR)LIST:s/LANGDIR//)" == "$(ZIP$(TNR)LIST)"
ZIP$(TNR)TARGETN=$(COMMONBIN)$/$(ZIP$(TNR)TARGET)$(ZIP$(TNR)EXT)
.ELSE
ZIP$(TNR)TARGETN=$(foreach,i,$(zip$(TNR)alllangiso) $(COMMONBIN)$/$(ZIP$(TNR)TARGET)_$i$(ZIP$(TNR)EXT) )
.ENDIF
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIP$(TNR)LIST:s/LANGDIR//)" == "$(ZIP$(TNR)LIST)"
ZIP$(TNR)TARGETN=$(BIN)$/$(ZIP$(TNR)TARGET)$(ZIP$(TNR)EXT)
.ELSE
ZIP$(TNR)TARGETN=$(foreach,i,$(zip$(TNR)alllangiso) $(BIN)$/$(ZIP$(TNR)TARGET)_$i$(ZIP$(TNR)EXT) )
.ENDIF
.ENDIF			# "$(common_build_zip)"!=""
ZIP$(TNR)DEPFILE=$(subst,$(COMMON_OUTDIR),$(OUTPATH) $(subst,$/bin$/,$/misc$/ $(ZIP$(TNR)TARGETN:s/$(ZIP$(TNR)EXT)/.dpzz/)))
ZIPDEPFILES+=$(ZIP$(TNR)DEPFILE)

ZIP$(TNR)DIR*=$(ZIPDIR)
ZIP$(TNR)FLAGS*=$(ZIPFLAGS)
.IF "$(zip$(TNR)generatedlangs)"!=""
zip$(TNR)langdirs*=$(alllangiso)
.ELSE           # "$(zip$(TNR)generatedlangs)"!=""
.IF "$(GUI)"=="UNX" || "$(USE_SHELL)"!="4nt"
zip$(TNR)langdirs:=$(shell @-test -d {$(subst,$/$(LANGDIR), $(null,$(ZIP$(TNR)DIR) . $(ZIP$(TNR)DIR)))}/ && find {$(subst,$/$(LANGDIR), $(null,$(ZIP$(TNR)DIR) . $(ZIP$(TNR)DIR)))}/ -type d ! -name CVS ! -name "." | sed "s/\.\/\///" | sed "s/\.\///" )
.ELSE			# "$(GUI)"=="UNX"
zip$(TNR)langdirs:=$(subst,CVS, $(shell @$(4nt_force_shell)-dir {$(subst,$/$(LANGDIR), $(ZIP$(TNR)DIR))} /ba:d ))
.ENDIF			# "$(GUI)"=="UNX"
.ENDIF          # "$(zip$(TNR)generatedlangs)"!=""
.IF "$(ZIP$(TNR)FORCEALLLANG)"!=""
zip$(TNR)alllangiso:=$(foreach,i,$(completelangiso) $(foreach,j,$(zip$(TNR)langdirs) $(eq,$i,$j  $i $(NULL))))
.ELSE          # "$(ZIP$(TNR)ALLLANG)" != ""
zip$(TNR)alllangiso*:=$(foreach,i,$(alllangiso) $(foreach,j,$(zip$(TNR)langdirs) $(eq,$i,$j  $i $(NULL))))
.ENDIF          # "$(ZIP$(TNR)ALLLANG)" != ""
.ENDIF			# "$(ZIP$(TNR)TARGET)"!=""

.IF "$(ZIP$(TNR)TARGETN)"!=""

ZIP$(TNR)TMP:=$(mktmp iii)

$(MISC)$/$(TARGET).$(PWD:f).$(ZIP$(TNR)TARGET).dpzz : $(ZIP$(TNR)TARGETN)

.IF "$(common_build_zip)"!=""
ZIP$(TNR)HELPVAR=$(COMMONBIN)$/$(ZIP$(TNR)TARGET)
.ELSE			# "$(common_build_zip)"!=""
ZIP$(TNR)HELPVAR=$(BIN)$/$(ZIP$(TNR)TARGET)
.ENDIF			# "$(common_build_zip)"!=""

$(ZIP$(TNR)DEPFILE) :
    echo # > $(MISC)$/$(@:f)
.IF "$(common_build_zip)"!=""
.IF "$(ZIP$(TNR)DIR)" != ""
    @echo type 1
    -$(MKDIRHIER) $(ZIP$(TNR)DIR)
    $(CDD) $(subst,LANGDIR,. $(subst,$/LANGDIR, $(ZIP$(TNR)DIR))) $(command_seperator) $(ZIPDEP) $(ZIP$(TNR)FLAGS) -prefix $(subst,LANGDIR,. $(subst,$/LANGDIR, $(ZIP$(TNR)DIR)))$/ $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(subst,$/misc$/,$/bin$/ $(@:s/.dpzz/$(ZIP$(TNR)EXT)/))) $(foreach,j,$(ZIP$(TNR)LIST) "{$(subst,LANGDIR,{$(subst,$(ZIP$(TNR)TARGET)_, $(@:f:b))} $j )}") $(avoid_cvs_dir) >> $(PWD)$/$(PRJ)$/$(ROUT)$/misc$/$(@:f)
.ELSE			# "$(ZIP$(TNR)DIR)" != ""
    @echo type 2
    -$(ZIPDEP) $(ZIP$(TNR)FLAGS) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(subst,$/misc$/,$/bin$/ $(@:s/.dpzz/$(ZIP$(TNR)EXT)/))) $(foreach,j,$(ZIP$(TNR)LIST) "{$(subst,LANGDIR,{$(subst,$(ZIP$(TNR)TARGET)_, $(@:f:b))} $j )}") $(avoid_cvs_dir) >> $(MISC)$/$(@:f)
.ENDIF			# "$(ZIP$(TNR)DIR)" != ""
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIP$(TNR)DIR)" != ""
    @echo type 3
    -$(MKDIRHIER) $(ZIP$(TNR)DIR)
    $(CDD) $(subst,LANGDIR,. $(subst,$/LANGDIR, $(ZIP$(TNR)DIR))) $(command_seperator) $(ZIPDEP) $(ZIP$(TNR)FLAGS) -prefix $(subst,LANGDIR,. $(subst,$/LANGDIR, $(ZIP$(TNR)DIR)))$/ $(subst,$/misc$/,$/bin$/ $(@:s/.dpzz/$(ZIP$(TNR)EXT)/)) $(foreach,j,$(ZIP$(TNR)LIST) "{$(subst,LANGDIR,{$(subst,$(BIN)$/$(ZIP$(TNR)TARGET)_, $(subst,$/misc$/,$/bin$/ $(@:db)))} $j )}") $(avoid_cvs_dir) >> $(PWD)$/$(PRJ)$/$(ROUT)$/misc$/$(@:f)
.ELSE			# "$(ZIP$(TNR)DIR)" != ""
    @echo type 4
    -$(ZIPDEP) $(ZIP$(TNR)FLAGS) $(subst,$/misc$/,$/bin$/ $(@:s/.dpzz/$(ZIP$(TNR)EXT)/)) $(foreach,j,$(ZIP$(TNR)LIST) "{$(subst,LANGDIR,{$(subst,$(BIN)$/$(ZIP$(TNR)TARGET)_, $(subst,$/misc$/,$/bin$/ $(@:db)))} $j )}") $(avoid_cvs_dir) >> $(MISC)$/$(@:f)
.ENDIF			# "$(ZIP$(TNR)DIR)" != ""
.ENDIF			# "$(common_build_zip)"!=""
    @echo $@ : makefile.mk >> $(MISC)$/$(@:f)


$(ZIP$(TNR)TARGETN) : delzip $(ZIP$(TNR)DEPS)
    @echo ------------------------------
    @echo Making: $@
    @@$(!eq,$?,$(?:s/delzip/zzz/) -$(RM) echo) $(uniq $@ $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@))
    @$(eq,$?,$(?:s/delzip/zzz/) noop echo ) rebuilding zipfiles
    @echo ------------------------------ $(eq,$?,$(?:s/delzip/zzz/) >&$(NULLDEV) )
.IF "$(common_build_zip)"!=""
.IF "$(ZIP$(TNR)DIR)" != ""
    @@-$(GNUCOPY) -p $@ $(ZIP$(TNR)TMP).$(ZIP$(TNR)TARGET){$(subst,$(ZIP$(TNR)HELPVAR),_ $(@:db))}$(ZIP$(TNR)EXT)
    -$(CDD) $(subst,LANGDIR,{$(subst,$(ZIP$(TNR)HELPVAR)_, $(@:db))} $(ZIP$(TNR)DIR)) $(command_seperator) zip $(ZIP$(TNR)FLAGS) $(ZIP$(TNR)TMP).$(ZIP$(TNR)TARGET){$(subst,$(ZIP$(TNR)HELPVAR),_ $(@:db))}$(ZIP$(TNR)EXT) $(subst,LANGDIR_away$/, $(ZIP$(TNR)LIST:s/LANGDIR/LANGDIR_away/)) -x delzip $(avoid_cvs_dir) $(CHECKZIPRESULT)
    $(COPY) $(ZIP$(TNR)TMP).$(ZIP$(TNR)TARGET){$(subst,$(ZIP$(TNR)HELPVAR),_ $(@:db))}$(ZIP$(TNR)EXT) $@.$(INPATH)
    $(RM) $(ZIP$(TNR)TMP).$(ZIP$(TNR)TARGET){$(subst,$(ZIP$(TNR)HELPVAR),_ $(@:db))}$(ZIP$(TNR)EXT)
.ELSE			# "$(ZIP$(TNR)DIR)" != ""
    @@-$(GNUCOPY) -p $@ $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)
    zip $(ZIP$(TNR)FLAGS) $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $(foreach,j,$(ZIP$(TNR)LIST) $(subst,LANGDIR,{$(subst,$(ZIP$(TNR)HELPVAR)_, $(@:db))} $j )) -x delzip $(avoid_cvs_dir) $(CHECKZIPRESULT)
    @@-$(GNUCOPY) -p $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $@.$(INPATH)
    @-$(RM) $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)
.ENDIF			# "$(ZIP$(TNR)DIR)" != ""
    @@-$(RM) $@
    @$(IFEXIST) $@.$(INPATH) $(THEN) $(RENAME:s/+//) $@.$(INPATH) $@ $(FI)
#	@$(IFEXIST) $@ $(THEN) $(TOUCH) $@ $(FI)  # even if it's not used...
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIP$(TNR)DIR)" != ""
    @@-$(GNUCOPY) -p $@ $(ZIP$(TNR)TMP).$(ZIP$(TNR)TARGET){$(subst,$(ZIP$(TNR)HELPVAR),_ $(@:db))}$(ZIP$(TNR)EXT)
    -$(CDD) $(subst,LANGDIR,{$(subst,$(ZIP$(TNR)HELPVAR)_, $(@:db))} $(ZIP$(TNR)DIR)) $(command_seperator) zip $(ZIP$(TNR)FLAGS) $(ZIP$(TNR)TMP).$(ZIP$(TNR)TARGET){$(subst,$(ZIP$(TNR)HELPVAR),_ $(@:db))}$(ZIP$(TNR)EXT) $(subst,LANGDIR_away$/, $(ZIP$(TNR)LIST:s/LANGDIR/LANGDIR_away/)) -x delzip  $(avoid_cvs_dir) $(CHECKZIPRESULT)
    $(COPY) $(ZIP$(TNR)TMP).$(ZIP$(TNR)TARGET){$(subst,$(ZIP$(TNR)HELPVAR),_ $(@:db))}$(ZIP$(TNR)EXT)  $@ 
    $(RM) $(ZIP$(TNR)TMP).$(ZIP$(TNR)TARGET){$(subst,$(ZIP$(TNR)HELPVAR),_ $(@:db))}$(ZIP$(TNR)EXT)
.ELSE			# "$(ZIP$(TNR)DIR)" != ""
    zip $(ZIP$(TNR)FLAGS) $@ $(foreach,j,$(ZIP$(TNR)LIST) $(subst,LANGDIR,{$(subst,$(BIN)$/$(ZIP$(TNR)TARGET)_, $(@:db))} $j )) -x delzip $(avoid_cvs_dir) $(CHECKZIPRESULT)
.ENDIF			# "$(ZIP$(TNR)DIR)" != ""
.ENDIF			# "$(common_build_zip)"!=""
.ENDIF

# unroll end
#######################################################

