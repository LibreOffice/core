#*************************************************************************
#
#   $RCSfile: tg_jar.mk,v $
#
#   $Revision: 1.10 $
#
#   last change: $Author: hr $ $Date: 2003-04-28 16:44:42 $
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
.IF "$(use_starjar)"!=""
XSTARJARPATH=$(strip $(STARJARPATH))
.ENDIF			# "$(use_starjar)"!=""
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

.IF "$(USE_EXTENDED_MANIFESTFILE)"!=""
EXTENDEDMANIFESTFILE=$(MISC)$/$(JARTARGET:b)
.IF "$(JARMANIFEST)"!=""
$(JARMANIFEST) : $(EXTENDEDMANIFESTFILE)
.ENDIF			# "$(JARMANIFEST)"!=""
$(MISC)$/$(JARTARGET:b) : $(SOLARINCDIR)$/$(UPD)minor.mk
    @+echo Specification-Title: $(SPECTITLE) > $@
    @+echo Specification-Version: $(VERSION) >> $@
    @+echo Specification-Vendor: $(VENDOR) >> $@
    @+echo Implementation-Title: $(IMPLTITLE) >> $@
.IF "$(GUI)"=="UNX" || "$(USE_SHELL)"!="4nt"
    @+echo "Implementation-Version: $(RSCREVISION)" >> $@
.ELSE			# "$(GUI)"=="UNX"" || "$(USE_SHELL)"!="4nt"
    @+echo Implementation-Version: $(RSCREVISION) >> $@
.ENDIF			# "$(GUI)"=="UNX"" || "$(USE_SHELL)"!="4nt"
    @+echo Implementation-Vendor: $(VENDOR) >> $@
.ENDIF			# "$(USE_EXTENDED_MANIFESTFILE)"!=""

.IF "$(EXTENDEDMANIFESTFILE)"!=""

EXTENDEDMANIFESTFILEDEP:=$(MISC)$/$(TARGET)_$(EXTENDEDMANIFESTFILE:f)

$(MISC)$/$(TARGET)_$(EXTENDEDMANIFESTFILE:f) : $(EXTENDEDMANIFESTFILE)
    +-$(RM) $@
    +$(COPY) $(EXTENDEDMANIFESTFILE) $@
.ENDIF			# "$(EXTENDEDMANIFESTFILE)"!=""

$(JARMANIFEST) .PHONY : $(CUSTOMMANIFESTFILEDEP) $(EXTENDEDMANIFESTFILEDEP)
    +-$(MKDIR) $(@:d) >& $(NULLDEV)
    +-$(RM) $@ >& $(NULLDEV)
    +echo Manifest-Version: 1.0 > $@
.IF "$(GUI)"=="UNX" || "$(USE_SHELL)"!="4nt"
    +echo "Solar-Version: $(RSCREVISION)" >> $@
.ELSE			# "$(GUI)"=="UNX"" || "$(USE_SHELL)"!="4nt"
    +echo Solar-Version: $(RSCREVISION) >> $@
.ENDIF			# "$(GUI)"=="UNX"" || "$(USE_SHELL)"!="4nt"
.IF "$(EXTENDEDMANIFESTFILE)"!=""
    +$(TYPE) $(MISC)$/$(TARGET)_$(EXTENDEDMANIFESTFILE:f) >> $@
.ENDIF			# "$(EXTENDEDMANIFESTFILE)"!=""
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
.IF "$(use_starjar)"!=""
    +-$(RM) $@
    @+-$(COPY) $(DMAKE_WORK_DIR)$/$(JARFLT) $(TARGET)_$(JARFLT) >& $(NULLDEV)
    cd $(CLASSDIR) && $(STARJAR) $@ $(JARMANIFEST) $(TARGET)_$(JARFLT) + $(XSTARJARPATH:s/ /+/)
.ELSE			# "$(use_starjar)"!=""
    +cd $(CLASSDIR) && zip -u -r $(@:f) $(subst,$(CLASSDIR)$/, $(JARMANIFEST)) $(subst,\,/ $(JARCLASSDIRS))
.ENDIF			# "$(use_starjar)"!=""
.ENDIF

