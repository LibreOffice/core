# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Dictionary_Dictionary,dict-eo,dictionaries/eo))

$(eval $(call gb_Dictionary_add_root_files,dict-eo,\
	dictionaries/eo/desc_en.txt \
	dictionaries/eo/desc_eo.txt \
	dictionaries/eo/eo.aff \
	dictionaries/eo/eo.dic \
	dictionaries/eo/hyph_eo.dic \
	dictionaries/eo/license-en.txt \
	dictionaries/eo/package-description.txt \
))

$(eval $(call gb_Dictionary_add_thesauri,dict-eo,\
	dictionaries/eo/th_eo.dat \
))

# vim: set noet sw=4 ts=4:
