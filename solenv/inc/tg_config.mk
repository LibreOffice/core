#************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: tg_config.mk,v $
#
#   $Revision: 1.20 $
#
#   last change: $Author: kz $ $Date: 2008-03-05 16:33:26 $
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

PACKAGEDIR*:=$(subst,.,$/ $(PACKAGE))
XSLTPACKAGEDIR*:=$(subst,.,/ $(PACKAGE))
XCSROOTURL!:=$(ABSXCSROOT)
.IF $(GUI)==WNT
XCSROOTURL!:=file:///$(subst,\,/ $(XCSROOTURL))
.ENDIF
SYSXSLDIR*:=$(XSLDIR)$/

#
# --- XCS ---
#

.IF "$(XCSFILES)"!=""
# remove unnecessary info from the component schemas
XCS_TRIM=$(PROCESSOUT)$/registry$/schema$/$(PACKAGEDIR)$/{$(XCSFILES)}

# transform xcs format to properties format
# XCS_RESOURCES=$(foreach,i,$(XCSFILES) $(MISC)$/registry$/res$/{$(alllangiso)}$/$(PACKAGEDIR)$/$(i:b).properties)

ALLTAR: \
    $(XCS_TRIM) 
.ENDIF			# "$(XCSFILES)"!=""

.IF "$(XCS_TRIM)" != ""
$(XCS_TRIM) :   $(DTDDIR)$/registry$/component-schema.dtd \
                $(XSLDIR)$/schema_val.xsl \
                $(XSLDIR)$/sanity.xsl \
                $(XSLDIR)$/schema_trim.xsl 
.ENDIF

$(PROCESSOUT)$/registry$/schema$/$(PACKAGEDIR)$/%.xcs : %.xcs
    @echo -------------+ validating and stripping schema files
    -$(MKDIRHIER) $(@:d)
    $(XSLTPROC) --nonet -o $(@:d)$*.val \
                --stringparam componentName $(PACKAGE).$* \
                $(SYSXSLDIR)schema_val.xsl $<
    $(XSLTPROC) --nonet -o $(@:d)$*.san \
                $(SYSXSLDIR)sanity.xsl $<
    $(XSLTPROC) --nonet -o $@ \
                $(SYSXSLDIR)schema_trim.xsl $<
    +-$(RM) $(@:d)$*.val > $(NULLDEV)
    +-$(RM) $(@:d)$*.san > $(NULLDEV)

$(PROCESSOUT)$/merge$/$(PACKAGEDIR)$/%.xcs : %.xcs
# just a copy for now - insert "cfgex" commandline when required
    -$(MKDIRHIER) $(@:d)
    $(COPY) $< $@

# Create properties files for localized <info> contents (Currently not built)
.IF "$(XCS_RESOURCES)" != ""
$(XCS_RESOURCES) :   $(XSLDIR)$/resource.xsl 
.ENDIF

$(PROCESSOUT)$/registry$/res$/{$(alllangiso)}$/$(PACKAGEDIR)$/%.properties :| $(PROCESSOUT)$/merge$/$(PACKAGEDIR)$/%.xcs
    @echo -------------+ creating locale dependent resource bundles
    -$(MKDIRHIER) $(@:d)
    $(XSLTPROC) --nonet -o $@ \
                --stringparam locale {$(subst,$/$(PACKAGEDIR)$/$(@:f), $(subst,$(PROCESSOUT)$/registry$/res$/, $@))} \
                $(SYSXSLDIR)resource.xsl $<

# 
# --- XCU ---
#

.IF "$(XCUFILES)"!=""
XCU_DEFAULT =$(PROCESSOUT)$/registry$/data$/$(PACKAGEDIR)$/{$(XCUFILES)}
XCU_MODULES =$(foreach,i,$(MODULEFILES) $(PROCESSOUT)$/registry$/spool$/$(PACKAGEDIR)$/$(i))
XCU_LANG    =$(foreach,i,$(LOCALIZEDFILES) $(PROCESSOUT)$/registry$/res$/{$(alllangiso)}$/$(PACKAGEDIR)$/$(i))
.ENDIF			# "$(XCUFILES)"!=""

.IF "$(XCU_DEFAULT)" != ""
$(XCU_DEFAULT) : $(DTDDIR)$/registry$/component-update.dtd \
                 $(XSLDIR)$/data_val.xsl \
                 $(XSLDIR)$/alllang.xsl 
.ENDIF

$(PROCESSOUT)$/registry$/data$/$(PACKAGEDIR)$/%.xcu : %.xcu
    @echo -------------+ validating and creating a locale independent file
    -$(MKDIRHIER) $(@:d) 
    $(XSLTPROC) --nonet -o $(@:d)$*.val \
                --stringparam xcs $(XCSROOTURL)/registry/schema/$(XSLTPACKAGEDIR)/$*.xcs \
                --stringparam schemaRoot $(XCSROOTURL)/registry/schema \
                $(SYSXSLDIR)data_val.xsl $<
    $(XSLTPROC) --nonet -o $@ \
                --stringparam xcs $(XCSROOTURL)/registry/schema/$(XSLTPACKAGEDIR)/$*.xcs \
                --stringparam schemaRoot $(XCSROOTURL)/registry/schema \
                $(SYSXSLDIR)alllang.xsl $<
    +-$(RM) $(@:d)$*.val > $(NULLDEV)

# --- localizations ---
.IF "$(WITH_LANG)"!=""
.IF "$(XCU_LANG)"!=""
$(XCU_LANG) : localize.sdf
.ENDIF			# "$(XCU_LANG)"!=""

$(PROCESSOUT)$/merge$/$(PACKAGEDIR)$/%.xcu : %.xcu
    -$(MKDIRHIER) $(@:d)
    $(CFGEX) -p $(PRJNAME) -i $(@:f) -o $@ -m localize.sdf -l all

.IF "$(XCU_LANG)" != ""
$(XCU_LANG) : $(XSLDIR)$/alllang.xsl
.ENDIF

$(PROCESSOUT)$/registry$/res$/{$(alllangiso)}$/$(PACKAGEDIR)$/%.xcu :| $(PROCESSOUT)$/merge$/$(PACKAGEDIR)$/%.xcu
.ELSE			# "$(WITH_LANG)"!=""
$(PROCESSOUT)$/registry$/res$/{$(alllangiso)}$/$(PACKAGEDIR)$/%.xcu :| %.xcu
.ENDIF			# "$(WITH_LANG)"!=""
    @echo ------------- creating locale dependent entries
    -$(MKDIRHIER) $(@:d)
    $(XSLTPROC) --nonet -o $@ \
                --stringparam xcs $(XCSROOTURL)/registry/schema/$(XSLTPACKAGEDIR)/$*.xcs \
                --stringparam schemaRoot $(XCSROOTURL)/registry/schema \
                --stringparam locale {$(subst,$/$(PACKAGEDIR)$/$(@:f), $(subst,$(PROCESSOUT)$/registry$/res$/, $@))} \
                $(SYSXSLDIR)alllang.xsl $<

# --- languagepack tag modules ---
.IF "$(LANGUAGEPACKS)" != ""
$(LANGUAGEPACKS) : $(XSLDIR)$/delcomment.sed 
.ENDIF


$(PROCESSOUT)$/registry$/spool$/Langpack-%.xcu : Langpack.xcu.tmpl
    @echo -------------+ creating a Langpack module for locale $*
    -$(MKDIRHIER) $(@:d) 
    $(SED) -e "s/__LANGUAGE__/$*/" -f $(XSLDIR)$/delcomment.sed $< > $@


# --- modules ---
.IF "$(XCU_MODULES)" != ""
$(XCU_MODULES) : $(XSLDIR)$/alllang.xsl 
.ENDIF


$(PROCESSOUT)$/registry$/spool$/$(PACKAGEDIR)$/%.xcu :| $$(@:b:s/-/./:b).xcu
    @echo -------------+ creating a module file
    -$(MKDIRHIER) $(@:d) 
    $(XSLTPROC) --nonet -o $@ \
                --stringparam xcs $(XCSROOTURL)/registry/schema/$(XSLTPACKAGEDIR)/$(<:b).xcs \
                --stringparam schemaRoot $(XCSROOTURL)/registry/schema \
                --stringparam module $(subst,$(<:b)-, $(*)) \
                $(SYSXSLDIR)alllang.xsl $<

.IF "$(XCUFILES)"!=""
ALLTAR: \
    $(XCU_DEFAULT) \
    $(XCU_MODULES) \
    $(XCU_LANG)

$(XCU_DEFAULT) : $$(@:d:s!$(PROCESSOUT)$/registry$/data$/!$(ABSXCSROOT)$/registry$/schema$/!)$$(@:f:s/.xcu/.xcs/)

.ENDIF			# "$(XCUFILES)"!=""

