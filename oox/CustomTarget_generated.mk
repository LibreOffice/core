# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,oox/generated))

oox_SRC := $(SRCDIR)/oox/source/token
oox_MISC := $(call gb_CustomTarget_get_workdir,oox/generated)/misc
oox_INC := $(call gb_CustomTarget_get_workdir,oox/generated)/inc
oox_GENHEADERPATH := $(oox_INC)/oox/token

$(oox_MISC)/vmlexport-shape-types.cxx : \
		$(SRCDIR)/oox/source/export/preset-definitions-to-shape-types.pl \
		$(SRCDIR)/oox/source/export/presetShapeDefinitions.xml \
		$(SRCDIR)/oox/source/export/presetTextWarpDefinitions.xml
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),build,PRL,1)
	mkdir -p $(dir $@)
	perl $^ > $@.in_progress 2> $@.log && mv $@.in_progress $@

$(oox_INC)/tokenhash.inc : $(oox_MISC)/tokenhash.gperf
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),build,GPF,1)
	$(GPERF) --compare-strncmp $< | sed -e 's/(char\*)0/(char\*)0, 0/g' | grep -v '^#line' > $@

oox_GenTarget_get_target = $(oox_MISC)/$(1)

define oox_GenTarget
$(oox_GENHEADERPATH)/$(1).hxx $(oox_MISC)/$(2)ids.inc $(oox_INC)/$(2)names.inc \
		$(if $(3),$(oox_MISC)/$(3)) : $(call oox_GenTarget_get_target,$(1))
	@touch $$@

$(call oox_GenTarget_get_target,$(1)) : $(oox_SRC)/$(1).pl $(oox_SRC)/$(1).txt \
		$(oox_SRC)/$(1).hxx.head $(oox_SRC)/$(1).hxx.tail
	$$(call gb_Output_announce,$$(subst $(WORKDIR)/,,$$@),build,PRL,1)
	mkdir -p $(oox_MISC) $(oox_INC) $(oox_GENHEADERPATH)
	perl $(oox_SRC)/$(1).pl $(oox_SRC)/$(1).txt $(oox_MISC)/$(2)ids.inc \
			$(oox_INC)/$(2)names.inc $(if $(3),$(oox_MISC)/$(3)) \
		&& cat $(oox_SRC)/$(1).hxx.head $(oox_MISC)/$(2)ids.inc \
			   $(oox_SRC)/$(1).hxx.tail > $(oox_GENHEADERPATH)/$(1).hxx \
		&& touch $$@
endef

$(eval $(call oox_GenTarget,namespaces,namespace,namespaces.txt))
$(eval $(call oox_GenTarget,properties,property,))
$(eval $(call oox_GenTarget,tokens,token,tokenhash.gperf))

$(call gb_CustomTarget_get_target,oox/generated) : \
	$(oox_MISC)/vmlexport-shape-types.cxx \
	$(oox_INC)/tokenhash.inc \
	$(oox_INC)/tokennames.inc \
	$(oox_INC)/namespacenames.inc \
	$(oox_INC)/propertynames.inc \
	$(oox_GENHEADERPATH)/tokens.hxx \
	$(oox_GENHEADERPATH)/namespaces.hxx \
	$(oox_GENHEADERPATH)/properties.hxx \
	$(oox_MISC)/namespaces.txt \

# vim: set noet sw=4 ts=4:
