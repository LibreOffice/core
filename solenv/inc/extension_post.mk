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

EXTENSION_TARGET:=$(ZIP9TARGETN)

.SOURCE.xcu : $(MISC)/$(EXTNAME)/merge $(MISC)/$(EXTNAME)/registry/data $(MISC)/$(COMPONENT_SHARED_CONFIG)_in/merge $(MISC)/$(COMPONENT_SHARED_CONFIG)_in/registry/data $(COMPONENT_CONFIGDIR) .
.SOURCE.xcs : $(MISC)/$(EXTNAME)/registry $(MISC)/$(EXTNAME)/registry/schema .

fixme=$(MISC)/$(EXTNAME)/merge/$(MISC)
fixme2=$(fixme:n)

fixme3=$(MISC)/$(EXTNAME)/$(MISC)
fixme4=$(fixme:n)

$(EXTENSIONDIR)/$(COMPONENT_CONFIGDEST)/%.xcu : %.xcu
# ugly hacking to workaround prepended first source path
    @@-$(MKDIRHIER) $(subst,$(fixme2),$(MISC) $(@:d))
    $(COMMAND_ECHO)$(GNUCOPY) $< $(subst,$(fixme2),$(MISC) $@)
    
$(EXTENSIONDIR)/$(COMPONENT_CONFIGDESTSCHEMA)/%.xcs : %.xcs
    @@-$(MKDIRHIER) $(subst,$(fixme4),$(MISC) $(@:d))
    $(COMMAND_ECHO)$(GNUCOPY) $< $(subst,$(fixme4),$(MISC) $@)

$(EXTENSIONDIR)/%.jar : $(SOLARBINDIR)/%.jar
    @@-$(MKDIRHIER) $(@:d)
    $(COMMAND_ECHO)$(GNUCOPY) $< $@

.IF "$(COMPONENT_FILES)"!=""
COMPONENT_FILES_SRC*=./
# ugly hacking to workaround prepended first source path - also hits
# here in case of "just copied" .xcu files
$(COMPONENT_FILES) : $$(@:s|$(fixme2)|$(MISC)|:s|$(EXTENSIONDIR)/|$(COMPONENT_FILES_SRC)|)
    @@-$(MKDIRHIER) $(@:s|$(fixme2)|$(MISC)|:d)
    $(COMMAND_ECHO)$(COPY) $< $(@:s|$(fixme2)|$(MISC)|)
.ENDIF			# "$(COMPONENT_FILES)"!=""

.IF "$(COMPONENT_JARFILES)"!=""
$(COMPONENT_JARFILES) : $(CLASSDIR)/$$(@:f)
    @@-$(MKDIRHIER) $(@:d)
    $(COMMAND_ECHO)$(COPY) $< $@
.ENDIF			# "$(COMPONENT_JARFILES)"!=""

.IF "$(COMPONENT_LIBRARIES)"!=""
# TODO(Q3): strip the binary?
$(COMPONENT_LIBRARIES) : $(DLLDEST)/$$(@:f)
    @@-$(MKDIRHIER) $(@:d)
    $(COMMAND_ECHO)$(COPY) $< $@
.ENDIF			# "$(COMPONENT_LIBRARIES)"!=""

IMPLEMENTATION_IDENTIFIER*="com.sun.star.$(EXTENSIONNAME)-$(PLATFORMID)"

.INCLUDE .IGNORE : $(MISC)/$(TARGET)_lang_track.mk
.IF "$(LAST_WITH_LANG)"!="$(WITH_LANG)"
PHONYDESC=.PHONY
.ENDIF			# "$(LAST_WITH_LANG)"!="$(WITH_LANG)"
.IF "$(DESCRIPTION)"!=""
$(DESCRIPTION) $(PHONYDESC) : $(DESCRIPTION_SRC)
    @@-$(MKDIRHIER) $(@:d)
    @echo LAST_WITH_LANG=$(WITH_LANG) > $(MISC)/$(TARGET)_lang_track.mk

    $(COMMAND_ECHO)$(TYPE) $(DESCRIPTION_SRC) | sed s/UPDATED_IDENTIFIER/$(IMPLEMENTATION_IDENTIFIER)/ >  $@.1.$(EXTNAME)

    $(COMMAND_ECHO)$(TYPE) $@.1.$(EXTNAME) | sed s/UPDATED_SUPPORTED_PLATFORM/$(PLATFORMID)/ > $@
    @@-$(RM) $@.1.$(EXTNAME)

.ENDIF			# "$(DESCRIPTION)"!=""
# default OOo license text!!!
# may not fit...
.IF "$(CUSTOM_LICENSE)"==""
.IF "$(OS)" == "WNT"
PACKLICDEPS=$(SOLARVER)/$(INPATH)/bin/osl/license.txt
.ELSE			# "$(OS)" == "WNT"
PACKLICDEPS=$(SOLARVER)/$(INPATH)/bin/osl/LICENSE
.ENDIF			# "$(OS)" == "WNT"
.ELSE			# "$(CUSTOM_LICENSE)" == ""
PACKLICDEPS=$(CUSTOM_LICENSE)
.ENDIF			# "$(CUSTOM_LICENSE)" == ""
.IF "$(PACKLICS)"!=""
$(PACKLICS) : $(PACKLICDEPS)
    @@-$(MKDIRHIER) $(@:d)
    $(COMMAND_ECHO)$(GNUCOPY) $< $@
.ENDIF			# "$(PACKLICS)"!=""

.IF "$(COMPONENT_MANIFEST)"!=""
#$(COMPONENT_MANIFEST) : $$(@:f)
$(COMPONENT_MANIFEST) : $(MANIFEST_SRC) $(MANIFEST_DEPS)
    @@-$(MKDIRHIER) $(@:d)
.IF "$(COMPONENT_MANIFEST_GENERIC)" == ""
    $(COMMAND_ECHO)$(TYPE) $(MANIFEST_SRC) | $(SED) "s/SHARED_EXTENSION/$(DLLPOST)/" | $(SED) "s/EXEC_EXTENSION/$(EXECPOST)/" > $@
.ELSE			# "$(COMPONENT_MANIFEST_GENERIC)" != ""
    $(COMMAND_ECHO)$(PERL) $(SOLARENV)/bin/makemani.pl $(PRJ)/util/manifest.xml $(EXTENSIONDIR) $(COMPONENT_MANIFEST_SEARCHDIR) $(@:d:d)
.ENDIF			# "$(COMPONENT_MANIFEST_GENERIC)" != ""
.ENDIF			# "$(COMPONENT_MANIFEST)"!=""

.IF "$(depend)" == ""
ALLTAR : $(EXTENSION_PACKDEPS)
.END
