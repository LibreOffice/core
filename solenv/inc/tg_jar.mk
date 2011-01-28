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


.IF "$(VERBOSE)"=="FALSE"
ZIP_VERBOSITY=-q
.ENDIF

.IF "$(JARTARGETN)"!=""

.IF "$(JARCOMPRESS)"==""
JARCOMPRESS_FLAG=0
.ENDIF

.IF "$(NEW_JAR_PACK)"!=""
$(JARTARGETN) : $(JARMANIFEST) $(JAVACLASSFILES) $(JAVATARGET)
.ENDIF			# "$(NEW_JAR_PACK)"!=""

.IF "$(NEW_JAR_PACK)"!=""
.IF "$(JARMANIFEST)"!=""

.IF "$(CUSTOMMANIFESTFILE)"!=""

CUSTOMMANIFESTFILEDEP:=$(MISC)/$(TARGET)_$(CUSTOMMANIFESTFILE:f)

$(MISC)/$(TARGET)_$(CUSTOMMANIFESTFILE:f) : $(subst,/,/ $(DMAKE_WORK_DIR))/$(CUSTOMMANIFESTFILE)
    $(COMMAND_ECHO)-$(RM) $@
    $(COMMAND_ECHO)$(COPY) $< $@
.ENDIF			# "$(CUSTOMMANIFESTFILE)"!=""

$(JARMANIFEST) .PHONY : $(CUSTOMMANIFESTFILEDEP)
    @@-$(MKDIRHIER) $(@:d)
    @@-$(RM) $@
    $(COMMAND_ECHO)echo Manifest-Version: 1.0 > $@
.IF "$(JARCLASSPATH)" != ""
    $(COMMAND_ECHO)echo $(USQ)Class-Path: $(JARCLASSPATH)$(USQ) >> $@
.ENDIF
# $(RSCREVISION) contains chars that must be quoted (for *NIX shells)
    $(COMMAND_ECHO)echo $(USQ)Solar-Version: $(RSCREVISION)$(USQ) >> $@
.IF "$(CUSTOMMANIFESTFILE)"!=""
    $(COMMAND_ECHO)$(TYPE) $(MISC)/$(TARGET)_$(CUSTOMMANIFESTFILE:f) >> $@
.ENDIF			# "$(CUSTOMMANIFESTFILE)"!=""
.ENDIF			# "$(JARMANIFEST)"!=""
.ENDIF			# "$(NEW_JAR_PACK)"!=""

.IF "$(JARCLASSEXCLUDES)" == ""
ZIPJAR_EXCLUDE=
.ELSE   # "$(JARCLASSEXCLUDES)" == ""
ZIPJAR_EXCLUDE=$(foreach,i,$(JARCLASSEXCLUDES) -x "$i")
# format:
# JARCLASSEXCLUDES=foo/* bar/*
.ENDIF  # "$(JARCLASSEXCLUDES)" == ""

#
# build jar 
#
.IF "$(NOJARDEP)"!="" || "$(NEW_JAR_PACK)"!=""
$(JARTARGETN) .PHONY :
#  $(JARMANIFEST)
.ELSE			# "$(NOJARDEP)"!="" || "$(NEW_JAR_PACK)"!=""
.DIRCACHE = no
$(JARTARGETN) :
#$(JARTARGETN) .SETDIR=$(CLASSDIR) .SEQUENTIAL : $(JARTARGETDEP) $(shell @-cat -s $(MISC)/$(JARTARGETN).dep )
.ENDIF			# "$(NOJARDEP)"!="" || "$(NEW_JAR_PACK)"!=""
.IF "$(OS)$(CPU)"=="SOLARISS"
    @-find . -type d -user $(USER) ! -perm -5 -print | xargs test "$$1" != "" && chmod +r $$1 
.ENDIF
.IF "$(JARMANIFEST)"!=""
    $(COMMAND_ECHO)cd $(CLASSDIR)/$(TARGET) && zip $(ZIP_VERBOSITY) -u -rX ../$(@:f) $(subst,$(CLASSDIR)/$(TARGET)/, $(JARMANIFEST)) $(ZIPJAR_EXCLUDE) $(CHECKZIPRESULT)
.ENDIF			# "$(JARMANIFEST)"!=""
    $(COMMAND_ECHO)cd $(CLASSDIR) && zip $(ZIP_VERBOSITY) -u -rX $(@:f) $(subst,\,/ $(JARCLASSDIRS)) $(ZIPJAR_EXCLUDE) $(CHECKZIPRESULT)
.ENDIF

