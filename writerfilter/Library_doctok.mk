# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Initial Developer of the Original Code is
#       David Tardon, Red Hat Inc. <dtardon@redhat.com>
# Portions created by the Initial Developer are Copyright (C) 2010 the
# Initial Developer. All Rights Reserved.
#
# Major Contributor(s):
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_Library_Library,doctok))

$(eval $(call gb_Library_set_include,doctok,\
    $$(INCLUDE) \
    -I$(realpath $(SRCDIR)/writerfilter/inc) \
    -I$(realpath $(SRCDIR)/writerfilter/source/doctok) \
    -I$(WORKDIR)/writerfilter/inc \
    -I$(WORKDIR)/writerfilter/inc/doctok \
	$(if $(filter YES,$(SYSTEM_LIBXML)),$(filter -I%,$(LIBXML_CFLAGS))) \
    -I$(OUTDIR)/inc \
))

include $(realpath $(SRCDIR)/writerfilter/debug_setup.mk)

$(eval $(call gb_Library_set_defs,doctok,\
	$$(DEFS) \
	-DWRITERFILTER_DOCTOK_DLLIMPLEMENTATION \
	$(writerfilter_debug_flags) \
	$(if $(filter YES,$(SYSTEM_LIBXML)),$(filter-out -I%,$(LIBXML_CFLAGS))) \
))

$(eval $(call gb_Library_add_api,doctok,\
    offapi \
    udkapi \
))

$(eval $(call gb_Library_add_linked_libs,doctok,\
    cppu \
    cppuhelper \
    resourcemodel \
    sal \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,doctok,\
    writerfilter/source/doctok/Dff \
    writerfilter/source/doctok/DffImpl \
    writerfilter/source/doctok/WW8Annotation \
    writerfilter/source/doctok/WW8BinTableImpl \
    writerfilter/source/doctok/WW8Clx \
    writerfilter/source/doctok/WW8CpAndFc \
    writerfilter/source/doctok/WW8DocumentImpl \
    writerfilter/source/doctok/WW8FKPImpl \
    writerfilter/source/doctok/WW8FontTable \
    writerfilter/source/doctok/WW8LFOTable \
    writerfilter/source/doctok/WW8ListTable \
    writerfilter/source/doctok/WW8OutputWithDepth \
    writerfilter/source/doctok/WW8Picture \
    writerfilter/source/doctok/WW8PieceTableImpl \
    writerfilter/source/doctok/WW8PropertySetImpl \
    writerfilter/source/doctok/WW8ResourceModelImpl \
    writerfilter/source/doctok/WW8StreamImpl \
    writerfilter/source/doctok/WW8StructBase \
    writerfilter/source/doctok/WW8Sttbf \
    writerfilter/source/doctok/WW8StyleSheet \
    writerfilter/source/doctok/WW8Table \
    writerfilter/source/doctok/WW8Text \
))

$(eval $(call gb_Library_add_generated_exception_objects,doctok,\
	writerfilter/source/doctok/resources \
))

doctok_GENDIR := $(WORKDIR)/writerfilter
doctok_HXXOUTDIR := $(doctok_GENDIR)/inc/doctok
doctok_HXXOUTDIRCREATED := $(doctok_HXXOUTDIR)/created

ooxml_BASEDIR := $(realpath $(SRCDIR)/writerfilter)

doctok_QNAMETOSTRXSL := $(ooxml_BASEDIR)/source/doctok/qnametostr.xsl
doctok_RESOURCEIDSXSL := $(ooxml_BASEDIR)/source/doctok/resourceids.xsl
doctok_RESOURCESIMPLXSL := $(ooxml_BASEDIR)/source/doctok/resourcesimpl.xsl
doctok_RESOURCESXSL := $(ooxml_BASEDIR)/source/doctok/resources.xsl
doctok_RESOURCETOOLSXSL := $(ooxml_BASEDIR)/source/doctok/resourcetools.xsl
doctok_SPRMCODETOSTRXSL := $(ooxml_BASEDIR)/source/doctok/sprmcodetostr.xsl
doctok_SPRMIDSXSL := $(ooxml_BASEDIR)/source/doctok/sprmids.xsl

doctok_MODEL := $(ooxml_BASEDIR)/source/doctok/resources.xmi

doctok_RESOURCEIDSHXX := $(doctok_HXXOUTDIR)/resourceids.hxx
doctok_SPRMIDSHXX := $(doctok_HXXOUTDIR)/sprmids.hxx
doctok_RESOURCESHXX := $(doctok_HXXOUTDIR)/resources.hxx
doctok_RESOURCESCXX := $(call gb_GenCxxObject_get_source,writerfilter/source/doctok/resources)

doctok_QNAMETOSTRTMP := $(doctok_GENDIR)/doctok_qnameToStr.tmp
doctok_SPRPMCODETOSTRTMP := $(doctok_GENDIR)/sprmcodetostr.tmp

doctok_GENHEADERS = \
    $(doctok_RESOURCEIDSHXX) \
    $(doctok_SPRMIDSHXX) \
    $(doctok_RESOURCESHXX)

doctok_GENFILES = \
    $(doctok_GENHEADERS) \
    $(doctok_QNAMETOSTRTMP) \
    $(doctok_RESOURCESCXX) \
    $(doctok_SPRPMCODETOSTRTMP)

$(doctok_HXXOUTDIRCREATED) :
	mkdir -p $(dir $@) && touch $@

$(doctok_GENHEADERS) : $(doctok_HXXOUTDIRCREATED)

define doctok_xsl_process_model
$(1) : $(2) $(doctok_MODEL)
	mkdir -p $(dir $(1)) && $$(call gb_Helper_abbreviate_dirs_native,$(gb_XSLTPROC) --nonet $(2) $(doctok_MODEL)) > $(1)
endef

$(eval $(call doctok_xsl_process_model,$(doctok_RESOURCESHXX),$(doctok_RESOURCESXSL)))
$(eval $(call doctok_xsl_process_model,$(doctok_QNAMETOSTRTMP),$(doctok_QNAMETOSTRXSL)))
$(eval $(call doctok_xsl_process_model,$(doctok_SPRMIDSHXX),$(doctok_SPRMIDSXSL)))
$(eval $(call doctok_xsl_process_model,$(doctok_RESOURCESCXX),$(doctok_RESOURCESIMPLXSL)))
$(eval $(call doctok_xsl_process_model,$(doctok_RESOURCEIDSHXX),$(doctok_RESOURCEIDSXSL)))
$(eval $(call doctok_xsl_process_model,$(doctok_SPRPMCODETOSTRTMP),$(doctok_SPRMCODETOSTRXSL)))

$(doctok_RESOURCESCXX) : $(doctok_GENHEADERS) $(doctok_RESOURCETOOLSXSL)

doctok_clean:
	rm -rf $(doctok_GENFILES) $(doctok_HXXOUTDIR)
.PHONY: doctok_clean

$(call gb_Library_get_clean_target,doctok) : doctok_clean

# vim: set noet ts=4 sw=4:
