#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: tg_jar.mk,v $
#
#   $Revision: 1.24 $
#
#   last change: $Author: rt $ $Date: 2007-11-06 15:46:38 $
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

CUSTOMMANIFESTFILEDEP:=$(MISC)$/$(TARGET)_$(CUSTOMMANIFESTFILE:f)

$(MISC)$/$(TARGET)_$(CUSTOMMANIFESTFILE:f) : $(subst,/,$/ $(DMAKE_WORK_DIR))$/$(CUSTOMMANIFESTFILE)
    -$(RM) $@
    $(COPY) $< $@
.ENDIF			# "$(CUSTOMMANIFESTFILE)"!=""

$(JARMANIFEST) .PHONY : $(CUSTOMMANIFESTFILEDEP)
    @@-$(MKDIRHIER) $(@:d)
    @@-$(RM) $@
    echo Manifest-Version: 1.0 > $@
.IF "$(JARCLASSPATH)" != ""
    echo $(USQ)Class-Path: $(JARCLASSPATH)$(USQ) >> $@
.ENDIF
# $(RSCREVISION) contains chars that must be quoted (for *NIX shells)
    echo $(USQ)Solar-Version: $(RSCREVISION)$(USQ) >> $@
.IF "$(CUSTOMMANIFESTFILE)"!=""
    $(TYPE) $(MISC)$/$(TARGET)_$(CUSTOMMANIFESTFILE:f) >> $@
.ENDIF			# "$(CUSTOMMANIFESTFILE)"!=""
.ENDIF			# "$(JARMANIFEST)"!=""
.ENDIF			# "$(NEW_JAR_PACK)"!=""

#
# build jar 
#
.IF "$(NOJARDEP)"!="" || "$(NEW_JAR_PACK)"!=""
$(JARTARGETN) .PHONY :
#  $(JARMANIFEST)
.ELSE			# "$(NOJARDEP)"!="" || "$(NEW_JAR_PACK)"!=""
.DIRCACHE = no
$(JARTARGETN) :
#$(JARTARGETN) .SETDIR=$(CLASSDIR) .SEQUENTIAL : $(JARTARGETDEP) $(shell @-cat -s $(MISC)$/$(JARTARGETN).dep )
.ENDIF			# "$(NOJARDEP)"!="" || "$(NEW_JAR_PACK)"!=""
.IF "$(OS)$(CPU)"=="SOLARISS"
    @-find . -type d -user $(USER) ! -perm -5 -print | xargs test "$$1" != "" && chmod +r $$1 
.ENDIF
.IF "$(JARMANIFEST)"!=""
    cd $(CLASSDIR)$/$(TARGET) && zip -u -rX ..$/$(@:f) $(subst,$(CLASSDIR)$/$(TARGET)$/, $(JARMANIFEST)) $(CHECKZIPRESULT)
.ENDIF			# "$(JARMANIFEST)"!=""
    cd $(CLASSDIR) && zip -u -rX $(@:f) $(subst,\,/ $(JARCLASSDIRS)) $(CHECKZIPRESULT)
.ENDIF

