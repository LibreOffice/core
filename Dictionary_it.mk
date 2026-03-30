# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Dictionary_Dictionary,dict-it,dictionaries/it_IT))

$(eval $(call gb_Dictionary_add_root_files,dict-it,\
	dictionaries/it_IT/hyph_it_IT.dic \
	dictionaries/it_IT/it_IT.aff \
	dictionaries/it_IT/it_IT.dic \
	dictionaries/it_IT/README_hyph_it_IT.txt \
	dictionaries/it_IT/README_it_IT.txt \
	dictionaries/it_IT/README_th_it_IT.txt \
	dictionaries/it_IT/CHANGELOG.txt \
))

$(eval $(call gb_Dictionary_add_thesauri,dict-it,\
	dictionaries/it_IT/th_it_IT_v2.dat \
))

# vim: set noet sw=4 ts=4:
