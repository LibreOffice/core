# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Dictionary_Dictionary,dict-uk,dictionaries/uk_UA))

$(eval $(call gb_Dictionary_add_root_files,dict-uk,\
	dictionaries/uk_UA/hyph_uk_UA.dic \
	dictionaries/uk_UA/README_hyph_uk_UA.txt \
	dictionaries/uk_UA/README_th_uk_UA.txt \
	dictionaries/uk_UA/README_uk_UA.txt \
	dictionaries/uk_UA/uk_UA.aff \
	dictionaries/uk_UA/uk_UA.dic \
))

$(eval $(call gb_Dictionary_add_thesauri,dict-uk,\
	dictionaries/uk_UA/th_uk_UA.dat \
))

# vim: set noet sw=4 ts=4:
