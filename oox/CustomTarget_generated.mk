# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,oox/generated))

oox_MISC := $(gb_CustomTarget_workdir)/oox/generated/misc

$(oox_MISC)/vml-shape-types : \
		$(SRCDIR)/oox/source/export/preset-definitions-to-shape-types.pl \
		$(SRCDIR)/oox/source/drawingml/customshapes/presetShapeDefinitions.xml \
		$(SRCDIR)/oox/source/export/presetTextWarpDefinitions.xml \
		$(SRCDIR)/oox/CustomTarget_generated.mk
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),build,PRL,1)
	mkdir -p $(dir $@)
	perl $< --vml-shape-types-data $(filter-out $<,$^) > $@.in_progress 2> $@.log && mv $@.in_progress $@

$(oox_MISC)/oox-drawingml-adj-names : \
		$(SRCDIR)/oox/source/export/preset-definitions-to-shape-types.pl \
		$(SRCDIR)/oox/source/drawingml/customshapes/presetShapeDefinitions.xml \
		$(SRCDIR)/oox/source/export/presetTextWarpDefinitions.xml \
		$(SRCDIR)/oox/CustomTarget_generated.mk
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),build,PRL,1)
	mkdir -p $(dir $@)
	perl $< --drawingml-adj-names-data $(filter-out $<,$^) > $@.in_progress 2> $@.log && mv $@.in_progress $@

# Generate tokens for oox
$(eval $(call gb_CustomTarget_token_hash,oox/generated,tokenhash.inc,tokenhash.gperf))

$(eval $(call gb_CustomTarget_generate_tokens,oox/generated,oox,oox/source/token,namespaces,namespace,namespaces.txt,namespaces-strict,namespaces.py))
$(eval $(call gb_CustomTarget_generate_tokens,oox/generated,oox,oox/source/token,properties,property,,,properties.py))
$(eval $(call gb_CustomTarget_generate_tokens,oox/generated,oox,oox/source/token,tokens,token,tokenhash.gperf))

$(call gb_CustomTarget_get_target,oox/generated) : \
	$(oox_MISC)/oox-drawingml-adj-names \
	$(oox_MISC)/vml-shape-types \

# vim: set noet sw=4 ts=4:
