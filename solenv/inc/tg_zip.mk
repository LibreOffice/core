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

#######################################################
# unroll begin

.IF "$(GUI)"=="WNT"
command_seperator=&&
#command_seperator=^
avoid_cvs_dir=-x "*CVS*" -x "*.svn*"
.ELSE
command_seperator=;
avoid_cvs_dir=-x "*CVS*" -x "*.svn*"
 
.ENDIF

.IF "$(ZIP$(TNR)TARGET)"!=""

.IF "$(ZIP$(TNR)STRIPLANGUAGETAGS)" != ""
CALLXSLTPROC:=$(XSLTPROC)
.EXPORT: CALLXSLTPROC
.ENDIF

ZIP$(TNR)EXT*=.zip
.IF "$(common_build_zip)"!=""
.IF "$(ZIP$(TNR)LIST:s/LANGDIR//)" == "$(ZIP$(TNR)LIST)"
ZIP$(TNR)TARGETN=$(COMMONBIN)/$(ZIP$(TNR)TARGET)$(ZIP$(TNR)EXT)
.ELSE
ZIP$(TNR)TARGETN=$(foreach,i,$(zip$(TNR)alllangiso) $(COMMONBIN)/$(ZIP$(TNR)TARGET)_$i$(ZIP$(TNR)EXT) )
.ENDIF
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIP$(TNR)LIST:s/LANGDIR//)" == "$(ZIP$(TNR)LIST)"
ZIP$(TNR)TARGETN=$(BIN)/$(ZIP$(TNR)TARGET)$(ZIP$(TNR)EXT)
.ELSE
ZIP$(TNR)TARGETN=$(foreach,i,$(zip$(TNR)alllangiso) $(BIN)/$(ZIP$(TNR)TARGET)_$i$(ZIP$(TNR)EXT) )
.ENDIF
.ENDIF			# "$(common_build_zip)"!=""
ZIP$(TNR)DEPFILE=$(subst,$(COMMON_OUTDIR),$(OUTPATH) $(subst,/bin/,/misc/ $(ZIP$(TNR)TARGETN:s/$(ZIP$(TNR)EXT)/.dpzz/)))
ZIPDEPFILES+=$(ZIP$(TNR)DEPFILE)

ZIP$(TNR)DIR*=$(ZIPDIR)
ZIP$(TNR)FLAGS*=$(ZIPFLAGS)
.IF "$(zip$(TNR)generatedlangs)"!=""
zip$(TNR)langdirs*=$(alllangiso)
.ELSE           # "$(zip$(TNR)generatedlangs)"!=""
zip$(TNR)langdirs:=$(shell @-test -d {$(subst,/$(LANGDIR), $(null,$(ZIP$(TNR)DIR) . $(ZIP$(TNR)DIR)))}/ && find {$(subst,/$(LANGDIR), $(null,$(ZIP$(TNR)DIR) . $(ZIP$(TNR)DIR)))}/ -type d ! -name CVS ! -name ".svn" ! -name "." | sed "s/\.\/\///" | sed "s/\.\///" )
.ENDIF          # "$(zip$(TNR)generatedlangs)"!=""
.IF "$(ZIP$(TNR)FORCEALLLANG)"!=""
zip$(TNR)alllangiso:=$(foreach,i,$(completelangiso) $(foreach,j,$(zip$(TNR)langdirs) $(eq,$i,$j  $i $(NULL))))
.ELSE          # "$(ZIP$(TNR)ALLLANG)" != ""
zip$(TNR)alllangiso*:=$(foreach,i,$(alllangiso) $(foreach,j,$(zip$(TNR)langdirs) $(eq,$i,$j  $i $(NULL))))
.ENDIF          # "$(ZIP$(TNR)ALLLANG)" != ""
.ENDIF			# "$(ZIP$(TNR)TARGET)"!=""

.IF "$(ZIP$(TNR)TARGETN)"!=""

ZIP$(TNR)TMP:=$(mktmp iii)

$(MISC)/$(TARGET).$(PWD:f).$(ZIP$(TNR)TARGET).dpzz : $(ZIP$(TNR)TARGETN)

.IF "$(common_build_zip)"!=""
ZIP$(TNR)HELPVAR=$(COMMONBIN)/$(ZIP$(TNR)TARGET)
.ELSE			# "$(common_build_zip)"!=""
ZIP$(TNR)HELPVAR=$(BIN)/$(ZIP$(TNR)TARGET)
.ENDIF			# "$(common_build_zip)"!=""

$(ZIP$(TNR)DEPFILE) :
    @echo # > $(MISC)/$(@:f)
.IF "$(common_build_zip)"!=""
.IF "$(ZIP$(TNR)DIR)" != ""
    $(COMMAND_ECHO)-$(MKDIRHIER) $(ZIP$(TNR)DIR)
    $(COMMAND_ECHO)$(CDD) $(subst,LANGDIR,. $(subst,/LANGDIR, $(ZIP$(TNR)DIR))) $(command_seperator) $(ZIPDEP) $(ZIP$(TNR)FLAGS) -prefix $(subst,LANGDIR,. $(subst,/LANGDIR, $(ZIP$(TNR)DIR)))/ $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(subst,/misc/,/bin/ $(@:s/.dpzz/$(ZIP$(TNR)EXT)/))) $(foreach,j,$(ZIP$(TNR)LIST) "{$(subst,LANGDIR,{$(subst,$(ZIP$(TNR)TARGET)_, $(@:f:b))} $j )}") $(avoid_cvs_dir) >> $(PWD)/$(PRJ)/$(ROUT)/misc/$(@:f)
.ELSE			# "$(ZIP$(TNR)DIR)" != ""
    $(COMMAND_ECHO)-$(ZIPDEP) $(ZIP$(TNR)FLAGS) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(subst,/misc/,/bin/ $(@:s/.dpzz/$(ZIP$(TNR)EXT)/))) $(foreach,j,$(ZIP$(TNR)LIST) "{$(subst,LANGDIR,{$(subst,$(ZIP$(TNR)TARGET)_, $(@:f:b))} $j )}") $(avoid_cvs_dir) >> $(MISC)/$(@:f)
.ENDIF			# "$(ZIP$(TNR)DIR)" != ""
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIP$(TNR)DIR)" != ""
    $(COMMAND_ECHO)-$(MKDIRHIER) $(ZIP$(TNR)DIR)
    $(COMMAND_ECHO)$(CDD) $(subst,LANGDIR,. $(subst,/LANGDIR, $(ZIP$(TNR)DIR))) $(command_seperator) $(ZIPDEP) $(ZIP$(TNR)FLAGS) -prefix $(subst,LANGDIR,. $(subst,/LANGDIR, $(ZIP$(TNR)DIR)))/ $(subst,/misc/,/bin/ $(@:s/.dpzz/$(ZIP$(TNR)EXT)/)) $(foreach,j,$(ZIP$(TNR)LIST) "{$(subst,LANGDIR,{$(subst,$(BIN)/$(ZIP$(TNR)TARGET)_, $(subst,/misc/,/bin/ $(@:db)))} $j )}") $(avoid_cvs_dir) >> $(PWD)/$(PRJ)/$(ROUT)/misc/$(@:f)
.ELSE			# "$(ZIP$(TNR)DIR)" != ""
    $(COMMAND_ECHO)-$(ZIPDEP) $(ZIP$(TNR)FLAGS) $(subst,/misc/,/bin/ $(@:s/.dpzz/$(ZIP$(TNR)EXT)/)) $(foreach,j,$(ZIP$(TNR)LIST) "{$(subst,LANGDIR,{$(subst,$(BIN)/$(ZIP$(TNR)TARGET)_, $(subst,/misc/,/bin/ $(@:db)))} $j )}") $(avoid_cvs_dir) >> $(MISC)/$(@:f)
.ENDIF			# "$(ZIP$(TNR)DIR)" != ""
.ENDIF			# "$(common_build_zip)"!=""
    @echo $(@:n:s#\#/#) : makefile.mk >> $(MISC)/$(@:f)


$(ZIP$(TNR)TARGETN) : delzip $(ZIP$(TNR)DEPS)
    @echo "Making:   " $(@:f)
    @@$(!eq,$?,$(?:s/delzip/zzz/) -$(RM) echo) $(uniq $@ $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@))
    @$(eq,$?,$(?:s/delzip/zzz/) noop echo ) rebuilding zipfiles
    @echo ------------------------------ $(eq,$?,$(?:s/delzip/zzz/) >&$(NULLDEV) )
.IF "$(common_build_zip)"!=""
.IF "$(ZIP$(TNR)DIR)" != ""
    @@-$(GNUCOPY) -p $@ $(ZIP$(TNR)TMP).$(ZIP$(TNR)TARGET){$(subst,$(ZIP$(TNR)HELPVAR),_ $(@:db))}$(ZIP$(TNR)EXT)
    $(COMMAND_ECHO)-$(CDD) $(subst,LANGDIR,{$(subst,$(ZIP$(TNR)HELPVAR)_, $(@:db))} $(ZIP$(TNR)DIR)) $(command_seperator) zip $(ZIP_VERBOSITY) $(ZIP$(TNR)FLAGS) $(ZIP$(TNR)TMP).$(ZIP$(TNR)TARGET){$(subst,$(ZIP$(TNR)HELPVAR),_ $(@:db))}$(ZIP$(TNR)EXT) $(subst,LANGDIR_away/, $(ZIP$(TNR)LIST:s/LANGDIR/LANGDIR_away/)) -x delzip $(avoid_cvs_dir) $(CHECKZIPRESULT)
    $(COMMAND_ECHO)$(IFEXIST) $(ZIP$(TNR)TMP).$(ZIP$(TNR)TARGET){$(subst,$(ZIP$(TNR)HELPVAR),_ $(@:db))}$(ZIP$(TNR)EXT) $(THEN) \
        $(PERL) -w $(SOLARENV)/bin/cleanzip.pl $(ZIP$(TNR)TMP).$(ZIP$(TNR)TARGET){$(subst,$(ZIP$(TNR)HELPVAR),_ $(@:db))}$(ZIP$(TNR)EXT) \
        $(FI)
    $(COMMAND_ECHO)$(COPY) $(ZIP$(TNR)TMP).$(ZIP$(TNR)TARGET){$(subst,$(ZIP$(TNR)HELPVAR),_ $(@:db))}$(ZIP$(TNR)EXT) $@.$(INPATH)
    $(COMMAND_ECHO)$(RM) $(ZIP$(TNR)TMP).$(ZIP$(TNR)TARGET){$(subst,$(ZIP$(TNR)HELPVAR),_ $(@:db))}$(ZIP$(TNR)EXT)
.ELSE			# "$(ZIP$(TNR)DIR)" != ""
    @@-$(GNUCOPY) -p $@ $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)
    $(COMMAND_ECHO)zip $(ZIP_VERBOSITY) $(ZIP$(TNR)FLAGS) $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $(foreach,j,$(ZIP$(TNR)LIST) $(subst,LANGDIR,{$(subst,$(ZIP$(TNR)HELPVAR)_, $(@:db))} $j )) -x delzip $(avoid_cvs_dir) $(CHECKZIPRESULT)
    $(COMMAND_ECHO)$(IFEXIST) $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $(THEN) \
        $(PERL) -w $(SOLARENV)/bin/cleanzip.pl $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) \
        $(FI)
    @@-$(GNUCOPY) -p $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $@.$(INPATH)
    @-$(RM) $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)
.ENDIF			# "$(ZIP$(TNR)DIR)" != ""
    @@-$(RM) $@
    @$(IFEXIST) $@.$(INPATH) $(THEN) $(RENAME:s/+//) $@.$(INPATH) $@ $(FI)
#	@$(IFEXIST) $@ $(THEN) $(TOUCH) $@ $(FI)  # even if it's not used...
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIP$(TNR)DIR)" != ""
    @@-$(GNUCOPY) -p $@ $(ZIP$(TNR)TMP).$(ZIP$(TNR)TARGET){$(subst,$(ZIP$(TNR)HELPVAR),_ $(@:db))}$(ZIP$(TNR)EXT)
    $(COMMAND_ECHO)-$(CDD) $(subst,LANGDIR,{$(subst,$(ZIP$(TNR)HELPVAR)_, $(@:db))} $(ZIP$(TNR)DIR)) $(command_seperator) zip $(ZIP_VERBOSITY) $(ZIP$(TNR)FLAGS) $(ZIP$(TNR)TMP).$(ZIP$(TNR)TARGET){$(subst,$(ZIP$(TNR)HELPVAR),_ $(@:db))}$(ZIP$(TNR)EXT) $(subst,LANGDIR_away/, $(ZIP$(TNR)LIST:s/LANGDIR/LANGDIR_away/)) -x delzip  $(avoid_cvs_dir) $(CHECKZIPRESULT)
    $(COMMAND_ECHO)$(IFEXIST) $(ZIP$(TNR)TMP).$(ZIP$(TNR)TARGET){$(subst,$(ZIP$(TNR)HELPVAR),_ $(@:db))}$(ZIP$(TNR)EXT) $(THEN) \
        $(PERL) -w $(SOLARENV)/bin/cleanzip.pl $(ZIP$(TNR)TMP).$(ZIP$(TNR)TARGET){$(subst,$(ZIP$(TNR)HELPVAR),_ $(@:db))}$(ZIP$(TNR)EXT) \
        $(FI)
    $(COMMAND_ECHO)$(COPY) $(ZIP$(TNR)TMP).$(ZIP$(TNR)TARGET){$(subst,$(ZIP$(TNR)HELPVAR),_ $(@:db))}$(ZIP$(TNR)EXT)  $@ 
    $(COMMAND_ECHO)$(RM) $(ZIP$(TNR)TMP).$(ZIP$(TNR)TARGET){$(subst,$(ZIP$(TNR)HELPVAR),_ $(@:db))}$(ZIP$(TNR)EXT)
.ELSE			# "$(ZIP$(TNR)DIR)" != ""
    $(COMMAND_ECHO)zip $(ZIP_VERBOSITY) $(ZIP$(TNR)FLAGS) $@ $(foreach,j,$(ZIP$(TNR)LIST) $(subst,LANGDIR,{$(subst,$(BIN)/$(ZIP$(TNR)TARGET)_, $(@:db))} $j )) -x delzip $(avoid_cvs_dir) $(CHECKZIPRESULT)
    $(COMMAND_ECHO)$(IFEXIST) $@ $(THEN) \
        $(PERL) -w $(SOLARENV)/bin/cleanzip.pl $@ \
        $(FI)
.IF "$(ZIP$(TNR)STRIPLANGUAGETAGS)" != ""
    $(COMMAND_ECHO)$(IFEXIST) $@ $(THEN) \
        $(SOLARENV)/bin/striplanguagetags.sh $@ \
        $(FI)
.ENDIF
.ENDIF			# "$(ZIP$(TNR)DIR)" != ""
.ENDIF			# "$(common_build_zip)"!=""
.ENDIF

# unroll end
#######################################################

