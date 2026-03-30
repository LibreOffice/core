# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Dictionary_Dictionary,dict-id,dictionaries/id))

$(eval $(call gb_Dictionary_add_root_files,dict-id,\
	dictionaries/id/LICENSE-dict \
	dictionaries/id/LICENSE-thes\
	dictionaries/id/hyph_id_ID.dic \
	dictionaries/id/id_ID.aff \
	dictionaries/id/id_ID.dic \
	dictionaries/id/README-dict.adoc \
	dictionaries/id/README-thes \
))

$(eval $(call gb_Dictionary_add_thesaurus,dict-id,dictionaries/id/th_id_ID_v2.dat))

# vim: set noet sw=4 ts=4:
