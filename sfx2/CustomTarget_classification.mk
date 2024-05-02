# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,sfx2/classification))

sfx2_classification_SRC := $(SRCDIR)/sfx2/classification
sfx2_classification_WORK := $(gb_CustomTarget_workdir)/sfx2/classification
sfx2_classification_GEN_example_validated=$(sfx2_classification_WORK)/example.validated

sfx2_classification_XMLLINTCOMMAND := $(call gb_ExternalExecutable_get_command,xmllint)

$(sfx2_classification_GEN_example_validated) : $(sfx2_classification_SRC)/baf.xsd $(sfx2_classification_SRC)/example.xml
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),build,VAL,1)
	$(call gb_Helper_abbreviate_dirs,\
		$(sfx2_classification_XMLLINTCOMMAND) --noout --schema $(sfx2_classification_SRC)/baf.xsd $(sfx2_classification_SRC)/example.xml > $@ 2>&1 \
		|| (cat $@; false))

$(call gb_CustomTarget_get_target,sfx2/classification) : $(sfx2_classification_GEN_example_validated)

$(sfx2_classification_GEN_example_validated) :| $(call gb_ExternalExecutable_get_dependencies,xmllint) $(sfx2_classification_WORK)/.dir

# vim: set noet sw=4 ts=4:
