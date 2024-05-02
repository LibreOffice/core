# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,xmloff/generated))

#Generates a hashtable for the odf-tags
xmloff_SRC := $(SRCDIR)/xmloff/source/token
xmloff_MISC := $(gb_CustomTarget_workdir)/xmloff/generated/misc
xmloff_INC := $(gb_CustomTarget_workdir)/xmloff/generated
xmloff_GENHEADERPATH := $(xmloff_INC)/xmloff/token

$(eval $(call gb_CustomTarget_token_hash,xmloff/generated,tokenhash.inc,tokenhash.gperf))
$(eval $(call gb_CustomTarget_generate_tokens,xmloff/generated,xmloff,xmloff/source/token,tokens,token,tokenhash.gperf))

$(call gb_CustomTarget_get_target,xmloff/generated) : \
	$(xmloff_INC)/tokenhash.inc \
	$(xmloff_INC)/tokennames.inc \
	$(xmloff_GENHEADERPATH)/tokens.hxx \

# vim: set noet sw=4 ts=4:
