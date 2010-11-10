#************************************************************************
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

PACKAGEDIR*:=$(subst,.,/ $(PACKAGE))
XSLTPACKAGEDIR*:=$(subst,.,/ $(PACKAGE))
XCSROOTURL!:=$(ABSXCSROOT)
.IF $(GUI)==WNT
XCSROOTURL!:=file:///$(subst,\,/ $(XCSROOTURL))
.ENDIF
SYSXSLDIR*:=$(XSLDIR)/
.IF $(GUI)==OS2
XCSROOTURL!:=$(subst,\,/ $(XCSROOTURL))
SYSXSLDIR!:=$(subst,\,/ $(SYSXSLDIR))/
.ENDIF

#
# --- XCS ---
#

.IF "$(XCSFILES)"!=""
# remove unnecessary info from the component schemas
XCS_TRIM=$(PROCESSOUT)/registry/schema/$(PACKAGEDIR)/{$(XCSFILES)}

# transform xcs format to properties format
# XCS_RESOURCES=$(foreach,i,$(XCSFILES) $(MISC)/registry/res/{$(alllangiso)}/$(PACKAGEDIR)/$(i:b).properties)

ALLTAR: \
    $(XCS_TRIM) 
.ENDIF			# "$(XCSFILES)"!=""

.IF "$(XCS_TRIM)" != ""
$(XCS_TRIM) :   $(DTDDIR)/registry/component-schema.dtd \
                $(XSLDIR)/schema_val.xsl \
                $(XSLDIR)/sanity.xsl \
                $(XSLDIR)/schema_trim.xsl 
.ENDIF

$(PROCESSOUT)/registry/schema/$(PACKAGEDIR)/%.xcs : %.xcs
    @echo "Making:   " $(@:f)
.IF "$(VERBOSE)" != "FALSE"
    @echo --- validating and stripping schema files
.ENDIF
    @-$(MKDIRHIER) $(@:d)
    $(COMMAND_ECHO)$(XSLTPROC) --nonet -o $(normpath $(subst,$(PATH_IN_MODULE), $(PWD))/$(subst,$(PRJ), $(@:d))$*.val) \
                --stringparam componentName $(PACKAGE).$* \
                $(SYSXSLDIR)schema_val.xsl $<
    $(COMMAND_ECHO)$(XSLTPROC) --nonet -o $(normpath $(subst,$(PATH_IN_MODULE), $(PWD))/$(subst,$(PRJ), $(@:d))$*.san) \
                $(SYSXSLDIR)sanity.xsl $<
    $(COMMAND_ECHO)$(XSLTPROC) --nonet -o $(normpath $(subst,$(PATH_IN_MODULE), $(PWD))/$(subst,$(PRJ), $@)) \
                $(SYSXSLDIR)schema_trim.xsl $<
    @+-$(RM) $(@:d)$*.val > $(NULLDEV)
    @+-$(RM) $(@:d)$*.san > $(NULLDEV)

$(PROCESSOUT)/merge/$(PACKAGEDIR)/%.xcs : %.xcs
# just a copy for now - insert "cfgex" commandline when required
    $(COMMAND_ECHO)-$(MKDIRHIER) $(@:d)
    $(COMMAND_ECHO)$(COPY) $< $@

# Create properties files for localized <info> contents (Currently not built)
.IF "$(XCS_RESOURCES)" != ""
$(XCS_RESOURCES) :   $(XSLDIR)/resource.xsl 
.ENDIF

$(PROCESSOUT)/registry/res/{$(alllangiso)}/$(PACKAGEDIR)/%.properties :| $(PROCESSOUT)/merge/$(PACKAGEDIR)/%.xcs
    @echo "Making:   " $(@:f)
.IF "$(VERBOSE)" != "FALSE"
    @echo --- creating locale dependent resource bundles
.ENDIF
    @-$(MKDIRHIER) $(@:d)
    $(COMMAND_ECHO)$(XSLTPROC) --nonet -o $(normpath $(subst,$(PATH_IN_MODULE), $(PWD))/$(subst,$(PRJ), $@)) \
                --stringparam locale {$(subst,/$(PACKAGEDIR)/$(@:f), $(subst,$(PROCESSOUT)/registry/res/, $@))} \
                $(SYSXSLDIR)resource.xsl $<

# 
# --- XCU ---
#

.IF "$(XCUFILES)"!=""
XCU_DEFAULT =$(PROCESSOUT)/registry/data/$(PACKAGEDIR)/{$(XCUFILES)}
XCU_MODULES =$(foreach,i,$(MODULEFILES) $(PROCESSOUT)/registry/spool/$(PACKAGEDIR)/$(i))
XCU_LANG    =$(foreach,i,$(LOCALIZEDFILES) $(PROCESSOUT)/registry/res/{$(alllangiso)}/$(PACKAGEDIR)/$(i))
.ENDIF			# "$(XCUFILES)"!=""

.IF "$(XCU_DEFAULT)" != ""
$(XCU_DEFAULT) : $(DTDDIR)/registry/component-update.dtd \
                 $(XSLDIR)/data_val.xsl \
                 $(XSLDIR)/alllang.xsl 
.ENDIF

$(PROCESSOUT)/registry/data/$(PACKAGEDIR)/%.xcu : %.xcu
    @echo "Making:   " $(@:f)
.IF "$(VERBOSE)" != "FALSE"
    @echo --- validating and creating a locale independent file
.ENDIF
    @-$(MKDIRHIER) $(@:d) 
    $(COMMAND_ECHO)$(XSLTPROC) --nonet -o $(normpath $(subst,$(PATH_IN_MODULE), $(PWD))/$(subst,$(PRJ), $(@:d))$*.val) \
                --stringparam xcs $(XCSROOTURL)/registry/schema/$(XSLTPACKAGEDIR)/$*.xcs \
                --stringparam schemaRoot $(XCSROOTURL)/registry/schema \
            --path $(DTDDIR)/registry \
                $(SYSXSLDIR)data_val.xsl $<
    $(COMMAND_ECHO)$(XSLTPROC) --nonet -o $(normpath $(subst,$(PATH_IN_MODULE), $(PWD))/$(subst,$(PRJ), $@)) \
                --stringparam xcs $(XCSROOTURL)/registry/schema/$(XSLTPACKAGEDIR)/$*.xcs \
                --stringparam schemaRoot $(XCSROOTURL)/registry/schema \
            --path $(DTDDIR)/registry \
                $(SYSXSLDIR)alllang.xsl $<
    @+-$(RM) $(@:d)$*.val > $(NULLDEV)

# --- localizations ---
.IF "$(WITH_LANG)"!=""
.IF "$(XCU_LANG)"!=""
$(XCU_LANG) : $(LOCALIZESDF)
.ENDIF			# "$(XCU_LANG)"!=""

$(PROCESSOUT)/merge/$(PACKAGEDIR)/%.xcu : %.xcu
    @-$(MKDIRHIER) $(@:d)
    $(COMMAND_ECHO)$(CFGEX)  -p $(PRJNAME) -i $(@:f) -o $@ -m $(LOCALIZESDF) -l all

.IF "$(XCU_LANG)" != ""
$(XCU_LANG) : $(XSLDIR)/alllang.xsl
.ENDIF

$(PROCESSOUT)/registry/res/{$(alllangiso)}/$(PACKAGEDIR)/%.xcu :| $(PROCESSOUT)/merge/$(PACKAGEDIR)/%.xcu
.ELSE			# "$(WITH_LANG)"!=""
$(PROCESSOUT)/registry/res/{$(alllangiso)}/$(PACKAGEDIR)/%.xcu :| %.xcu
.ENDIF			# "$(WITH_LANG)"!=""
    @echo "Making:   " $(@:f)
.IF "$(VERBOSE)" != "FALSE"
    @echo --- creating locale dependent entries
.ENDIF
    @-$(MKDIRHIER) $(@:d)
    $(COMMAND_ECHO)$(XSLTPROC) --nonet -o $(normpath $(subst,$(PATH_IN_MODULE), $(PWD))/$(subst,$(PRJ), $@)) \
                --stringparam xcs $(XCSROOTURL)/registry/schema/$(XSLTPACKAGEDIR)/$*.xcs \
                --stringparam schemaRoot $(XCSROOTURL)/registry/schema \
                --stringparam locale {$(subst,/$(PACKAGEDIR)/$(@:f), $(subst,$(PROCESSOUT)/registry/res/, $@))} \
                --path $(DTDDIR)/registry \
                $(SYSXSLDIR)alllang.xsl $<

# --- languagepack tag modules ---
.IF "$(LANGUAGEPACKS)" != ""
$(LANGUAGEPACKS) : $(XSLDIR)/delcomment.sed 
.ENDIF


$(PROCESSOUT)/registry/spool/Langpack-%.xcu : Langpack.xcu.tmpl
.IF "$(VERBOSE)" != "FALSE"
    @echo --- creating a Langpack module for locale $*
.ENDIF
    -$(MKDIRHIER) $(@:d) 
    $(COMMAND_ECHO)$(SED) -e "s/__LANGUAGE__/$*/" -f $(XSLDIR)/delcomment.sed $< > $@


# --- modules ---
.IF "$(XCU_MODULES)" != ""
$(XCU_MODULES) : $(XSLDIR)/alllang.xsl 
.ENDIF


$(PROCESSOUT)/registry/spool/$(PACKAGEDIR)/%.xcu :| $$(@:b:s/-/./:b).xcu
    @echo "Making:   " $(@:f)
.IF "$(VERBOSE)" != "FALSE"
    @echo --- creating a module file
.ENDIF
    @-$(MKDIRHIER) $(@:d) 
    $(COMMAND_ECHO)$(XSLTPROC) --nonet -o $(normpath $(subst,$(PATH_IN_MODULE), $(PWD))/$(subst,$(PRJ), $@)) \
                --stringparam xcs $(XCSROOTURL)/registry/schema/$(XSLTPACKAGEDIR)/$(<:b).xcs \
                --stringparam schemaRoot $(XCSROOTURL)/registry/schema \
                --stringparam module $(subst,$(<:b)-, $(*)) \
                $(SYSXSLDIR)alllang.xsl $<

.IF "$(XCUFILES)"!=""
ALLTAR: \
    $(XCU_DEFAULT) \
    $(XCU_MODULES) \
    $(XCU_LANG)

$(XCU_DEFAULT) : $$(@:d:s!$(PROCESSOUT)/registry/data/!$(ABSXCSROOT)/registry/schema/!)$$(@:f:s/.xcu/.xcs/)

.ENDIF			# "$(XCUFILES)"!=""

