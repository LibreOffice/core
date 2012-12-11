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

.IF "$(OS)"=="WNT"
command_seperator=&&
#command_seperator=^
.ELSE
command_seperator=;
 
.ENDIF

.IF "$(ZIP$(TNR)TARGET)"!=""

.IF "$(ZIP$(TNR)STRIPLANGUAGETAGS)" != ""
CALLXSLTPROC:=$(XSLTPROC)
.EXPORT: CALLXSLTPROC
.ENDIF

ZIP$(TNR)EXT*=.zip
.IF "$(ZIP$(TNR)LIST:s/LANGDIR//)" == "$(ZIP$(TNR)LIST)"
ZIP$(TNR)TARGETN=$(BIN)/$(ZIP$(TNR)TARGET)$(ZIP$(TNR)EXT)
.ELSE
ZIP$(TNR)TARGETN=$(foreach,i,$(zip$(TNR)alllangiso) $(BIN)/$(ZIP$(TNR)TARGET)_$i$(ZIP$(TNR)EXT) )
.ENDIF
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

ZIP$(TNR)HELPVAR=$(BIN)/$(ZIP$(TNR)TARGET)

$(ZIP$(TNR)DEPFILE) :
.IF "$(ZIP$(TNR)DIR)" != ""
    $(COMMAND_ECHO)-$(MKDIRHIER) $(ZIP$(TNR)DIR)
    $(COMMAND_ECHO)$(CDD) $(subst,LANGDIR,. $(subst,/LANGDIR, $(ZIP$(TNR)DIR))) $(command_seperator) $(ZIPDEP) $(ZIP$(TNR)FLAGS) -prefix $(subst,LANGDIR,. $(subst,/LANGDIR, $(ZIP$(TNR)DIR)))/ $(subst,/misc/,/bin/ $(@:s/.dpzz/$(ZIP$(TNR)EXT)/)) $(foreach,j,$(ZIP$(TNR)LIST) "{$(subst,LANGDIR,{$(subst,$(BIN)/$(ZIP$(TNR)TARGET)_, $(subst,/misc/,/bin/ $(@:db)))} $j )}") >> $(PWD)/$(PRJ)/$(ROUT)/misc/$(@:f)
.ELSE			# "$(ZIP$(TNR)DIR)" != ""
    $(COMMAND_ECHO)-$(ZIPDEP) $(ZIP$(TNR)FLAGS) $(subst,/misc/,/bin/ $(@:s/.dpzz/$(ZIP$(TNR)EXT)/)) $(foreach,j,$(ZIP$(TNR)LIST) "{$(subst,LANGDIR,{$(subst,$(BIN)/$(ZIP$(TNR)TARGET)_, $(subst,/misc/,/bin/ $(@:db)))} $j )}") >> $(MISC)/$(@:f)
.ENDIF			# "$(ZIP$(TNR)DIR)" != ""
    @echo $(@:n:s#\#/#) : makefile.mk >> $(MISC)/$(@:f)


$(ZIP$(TNR)TARGETN) : delzip $(ZIP$(TNR)DEPS)
    @echo "[ building ZIP ]" $(@:f)
    @@$(!eq,$?,$(?:s/delzip/zzz/) -$(RM) echo) $(uniq $@ $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@))
.IF "$(ZIP$(TNR)DIR)" != ""
    @@-$(GNUCOPY) -p $@ $(ZIP$(TNR)TMP).$(ZIP$(TNR)TARGET){$(subst,$(ZIP$(TNR)HELPVAR),_ $(@:db))}$(ZIP$(TNR)EXT)
    $(COMMAND_ECHO)-$(CDD) $(subst,LANGDIR,{$(subst,$(ZIP$(TNR)HELPVAR)_, $(@:db))} $(ZIP$(TNR)DIR)) $(command_seperator) zip $(ZIP_VERBOSITY) $(ZIP$(TNR)FLAGS) $(ZIP$(TNR)TMP).$(ZIP$(TNR)TARGET){$(subst,$(ZIP$(TNR)HELPVAR),_ $(@:db))}$(ZIP$(TNR)EXT) $(subst,LANGDIR_away/, $(ZIP$(TNR)LIST:s/LANGDIR/LANGDIR_away/)) -x delzip  $(CHECKZIPRESULT)
    $(COMMAND_ECHO)$(COPY) $(ZIP$(TNR)TMP).$(ZIP$(TNR)TARGET){$(subst,$(ZIP$(TNR)HELPVAR),_ $(@:db))}$(ZIP$(TNR)EXT)  $@ 
    $(COMMAND_ECHO)$(RM) $(ZIP$(TNR)TMP).$(ZIP$(TNR)TARGET){$(subst,$(ZIP$(TNR)HELPVAR),_ $(@:db))}$(ZIP$(TNR)EXT)
.ELSE			# "$(ZIP$(TNR)DIR)" != ""
    $(COMMAND_ECHO)zip $(ZIP_VERBOSITY) $(ZIP$(TNR)FLAGS) $@ $(foreach,j,$(ZIP$(TNR)LIST) $(subst,LANGDIR,{$(subst,$(BIN)/$(ZIP$(TNR)TARGET)_, $(@:db))} $j )) -x delzip $(CHECKZIPRESULT)
.IF "$(ZIP$(TNR)STRIPLANGUAGETAGS)" != ""
    $(COMMAND_ECHO)$(IFEXIST) $@ $(THEN) \
        $(SOLARENV)/bin/striplanguagetags.sh $@ \
        $(FI)
.ENDIF
.ENDIF			# "$(ZIP$(TNR)DIR)" != ""
.ENDIF

# unroll end
#######################################################

