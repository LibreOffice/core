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
.IF "$(COMPONENT_XCU_REPLACEMENT)"!=""
    $(TYPE) $< | sed '$(COMPONENT_XCU_REPLACEMENT)' > $(subst,$(fixme2),$(MISC) $@)
.ELSE          # "$(COMPONENT_REPLACEMENT)"!=""
    $(GNUCOPY) $< $(subst,$(fixme2),$(MISC) $@)
.ENDIF          # "$(COMPONENT_REPLACEMENT)"!=""
    
$(EXTENSIONDIR)/$(COMPONENT_CONFIGDESTSCHEMA)/%.xcs : %.xcs
    @@-$(MKDIRHIER) $(subst,$(fixme4),$(MISC) $(@:d))
    $(GNUCOPY) $< $(subst,$(fixme4),$(MISC) $@)

$(EXTENSIONDIR)/%.jar : $(SOLARBINDIR)/%.jar
    @@-$(MKDIRHIER) $(@:d)
    $(GNUCOPY) $< $@

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
PACK_RUNTIME!:=TRUE
# TODO(Q3): strip the binary?
$(COMPONENT_LIBRARIES) : $(DLLDEST)/$$(@:f)
    @@-$(MKDIRHIER) $(@:d)
    $(COMMAND_ECHO)$(COPY) $< $@
.ENDIF			# "$(COMPONENT_LIBRARIES)"!=""

.IF "$(PACK_RUNTIME)"!=""
PACK_RUNTIME_FLAG:=$(MISC)/$(TARGET)_pack_runtime.flag
$(EXTENSION_TARGET) : $(PACK_RUNTIME_FLAG)

$(EXTENSIONDIR) :
    @@-$(MKDIRHIER) $(@:d)

$(PACK_RUNTIME_FLAG) : $(EXTENSIONDIR)
    @@-$(MKDIRHIER) $(@:d)
.IF "$(OS)$(CPU)"=="WNTI" && "$(WITH_EXTENSION_INTEGRATION)"!="YES"
.IF "$(COM)"=="GCC"
   $(GNUCOPY) $(SOLARBINDIR)/mingwm10.dll $(EXTENSIONDIR)
   .IF "$(MINGW_GCCDLL)"!=""
     $(GNUCOPY) $(SOLARBINDIR)/$(MINGW_GCCDLL) $(EXTENSIONDIR)
   .ENDIF
   .IF "$(MINGW_GXXDLL)"!=""
     $(GNUCOPY) $(SOLARBINDIR)/$(MINGW_GXXDLL) $(EXTENSIONDIR)
   .ENDIF
.ELSE
.IF "$(PACKMS)"!=""
.IF "$(CCNUMVER)" <= "001399999999"
    $(GNUCOPY) $(PACKMS)/msvcr71.dll $(EXTENSIONDIR)
    $(GNUCOPY) $(PACKMS)/msvcp71.dll $(EXTENSIONDIR)
.ELSE			# "$(CCNUMVER)" <= "001399999999"
.IF "$(CCNUMVER)" <= "001499999999"
    $(GNUCOPY) $(PACKMS)/msvcr80.dll $(EXTENSIONDIR)
    $(GNUCOPY) $(PACKMS)/msvcp80.dll $(EXTENSIONDIR)
    $(GNUCOPY) $(PACKMS)/msvcm80.dll $(EXTENSIONDIR)
    $(GNUCOPY) $(PACKMS)/Microsoft.VC80.CRT.manifest $(EXTENSIONDIR)
.ELSE			# "$(CCNUMVER)" <= "001499999999"
    $(GNUCOPY) $(PACKMS)/msvcr90.dll $(EXTENSIONDIR)
    $(GNUCOPY) $(PACKMS)/msvcp90.dll $(EXTENSIONDIR)
    $(GNUCOPY) $(PACKMS)/msvcm90.dll $(EXTENSIONDIR)
    $(GNUCOPY) $(PACKMS)/Microsoft.VC90.CRT.manifest $(EXTENSIONDIR)
.ENDIF 			# "$(CCNUMVER)" <= "001499999999"
.ENDIF			# "$(CCNUMVER)" <= "001399999999"
.ELSE        # "$(PACKMS)"!=""
.IF "$(CCNUMVER)" <= "001399999999"
    $(GNUCOPY) $(SOLARBINDIR)/msvcr71.dll $(EXTENSIONDIR)
    $(GNUCOPY) $(SOLARBINDIR)/msvcp71.dll $(EXTENSIONDIR)
.ELSE			# "$(CCNUMVER)" <= "001399999999"
.IF "$(CCNUMVER)" <= "001499999999"
    $(GNUCOPY) $(SOLARBINDIR)/msvcr80.dll $(EXTENSIONDIR)
    $(GNUCOPY) $(SOLARBINDIR)/msvcp80.dll $(EXTENSIONDIR)
    $(GNUCOPY) $(SOLARBINDIR)/msvcm80.dll $(EXTENSIONDIR)
    $(GNUCOPY) $(SOLARBINDIR)/Microsoft.VC80.CRT.manifest $(EXTENSIONDIR)
.ELSE    		# "$(CCNUMVER)" <= "001499999999"
    $(GNUCOPY) $(SOLARBINDIR)/msvcr90.dll $(EXTENSIONDIR)
    $(GNUCOPY) $(SOLARBINDIR)/msvcp90.dll $(EXTENSIONDIR)
    $(GNUCOPY) $(SOLARBINDIR)/msvcm90.dll $(EXTENSIONDIR)
    $(GNUCOPY) $(SOLARBINDIR)/Microsoft.VC90.CRT.manifest $(EXTENSIONDIR)
.ENDIF			# "$(CCNUMVER)" <= "001499999999"
.ENDIF			# "$(CCNUMVER)" <= "001399999999"
.ENDIF          # "$(PACKMS)"!=""
.ENDIF	#"$(COM)"=="GCC" 
.ENDIF 			# "$(OS)$(CPU)"=="WNTI" && "$(WITH_EXTENSION_INTEGRATION)"!="YES"
    @$(TOUCH) $@
.ENDIF			# "$(PACK_RUNTIME)"!=""

IMPLEMENTATION_IDENTIFIER*="com.sun.star.$(EXTENSIONNAME)-$(PLATFORMID)"

.INCLUDE .IGNORE : $(MISC)/$(TARGET)_lang_track.mk
.IF "$(LAST_WITH_LANG)"!="$(WITH_LANG)"
PHONYDESC=.PHONY
.ENDIF			# "$(LAST_WITH_LANG)"!="$(WITH_LANG)"
.IF "$(DESCRIPTION)"!=""
$(DESCRIPTION) $(PHONYDESC) : $(DESCRIPTION_SRC)
    @@-$(MKDIRHIER) $(@:d)
    @echo LAST_WITH_LANG=$(WITH_LANG) > $(MISC)/$(TARGET)_lang_track.mk

    $(COMMAND_ECHO)$(PERL) $(SOLARENV)/bin/transform_description.pl $(DESCRIPTION_SRC) $@.1.$(EXTNAME)

    $(COMMAND_ECHO)$(TYPE) $@.1.$(EXTNAME) | sed s/UPDATED_IDENTIFIER/$(IMPLEMENTATION_IDENTIFIER)/ >  $@.2.$(EXTNAME)
    @@-$(RM) $@.1.$(EXTNAME)

    $(COMMAND_ECHO)$(TYPE) $@.2.$(EXTNAME) | sed s/UPDATED_SUPPORTED_PLATFORM/$(PLATFORMID)/ > $@
    @@-$(RM) $@.2.$(EXTNAME)

.ENDIF			# "$(DESCRIPTION)"!=""
# default OOo license text!!!
# may not fit...
.IF "$(CUSTOM_LICENSE)"==""
.IF "$(GUI)" == "WNT"
PACKLICDEPS=$(SOLARBINDIR)/osl/license.txt
.ELSE			# "$(GUI)" == "WNT"
PACKLICDEPS=$(SOLARBINDIR)/osl/LICENSE
.ENDIF			# "$(GUI)" == "WNT"
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

ALLTAR : $(EXTENSION_PACKDEPS)
