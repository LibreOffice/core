#*************************************************************************
#
#   $RCSfile: tg_jar.mk,v $
#
#   $Revision: 1.16 $
#
#   last change: $Author: obo $ $Date: 2005-06-17 09:49:55 $
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


.IF "$(JARTARGETN)"!=""

.IF "$(JARCOMPRESS)"==""
JARCOMPRESS_FLAG=0
.ENDIF

.IF "$(NEW_JAR_PACK)"!=""
$(JARTARGETN) : $(JARMANIFEST)
.ENDIF			# "$(NEW_JAR_PACK)"!=""

.IF "$(NEW_JAR_PACK)"!=""
.IF "$(JARMANIFEST)"!=""

.IF "$(CUSTOMMANIFESTFILE)"!=""

CUSTOMMANIFESTFILEDEP:=$(MISC)$/$(TARGET)_$(CUSTOMMANIFESTFILE:f)

$(MISC)$/$(TARGET)_$(CUSTOMMANIFESTFILE:f) : $(subst,/,$/ $(DMAKE_WORK_DIR))$/$(CUSTOMMANIFESTFILE)
    +-$(RM) $@
#>& $(NULLDEV)
    +$(COPY) $< $@
.ENDIF			# "$(CUSTOMMANIFESTFILE)"!=""

$(JARMANIFEST) .PHONY : $(CUSTOMMANIFESTFILEDEP)
    +-$(MKDIRHIER) $(@:d) >& $(NULLDEV)
    +-$(RM) $@ >& $(NULLDEV)
    +echo Manifest-Version: 1.0 > $@
.IF "$(JARCLASSPATH)" != ""
    +echo $(USQ)Class-Path: $(JARCLASSPATH)$(USQ) >> $@
.ENDIF
# $(RSCREVISION) contains chars that must be quoted (for *NIX shells)
    +echo $(USQ)Solar-Version: $(RSCREVISION)$(USQ) >> $@
.IF "$(CUSTOMMANIFESTFILE)"!=""
    +$(TYPE) $(MISC)$/$(TARGET)_$(CUSTOMMANIFESTFILE:f) >> $@
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
#$(JARTARGETN) .SETDIR=$(CLASSDIR) .SEQUENTIAL : $(JARTARGETDEP) $(shell -cat -s $(MISC)$/$(JARTARGETN).dep )
.ENDIF			# "$(NOJARDEP)"!="" || "$(NEW_JAR_PACK)"!=""
.IF "$(OS)$(CPU)"=="SOLARISS"
    @+-find . -type d -user $(USER) ! -perm -5 -print | xargs test "$$1" != "" && chmod +r $$1 
.ENDIF
.IF "$(JARMANIFEST)"!=""
    +cd $(CLASSDIR)$/$(TARGET) && zip -u -rX ..$/$(@:f) $(subst,$(CLASSDIR)$/$(TARGET)$/, $(JARMANIFEST)) $(CHECKZIPRESULT)
.ENDIF			# "$(JARMANIFEST)"!=""
    +cd $(CLASSDIR) && zip -u -rX $(@:f) $(subst,\,/ $(JARCLASSDIRS)) $(CHECKZIPRESULT)
.ENDIF

