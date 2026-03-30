# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Dictionary_Dictionary,dict-de,dictionaries/de))

$(eval $(call gb_Dictionary_add_root_files,dict-de,\
	dictionaries/de/COPYING_GPLv2 \
	dictionaries/de/COPYING_GPLv3 \
	dictionaries/de/COPYING_OASIS.txt \
	dictionaries/de/de_AT_frami.aff \
	dictionaries/de/de_AT_frami.dic \
	dictionaries/de/de_CH_frami.aff \
	dictionaries/de/de_CH_frami.dic \
	dictionaries/de/de_DE_frami.aff \
	dictionaries/de/de_DE_frami.dic \
	dictionaries/de/hyph_de_AT.dic \
	dictionaries/de/hyph_de_CH.dic \
	dictionaries/de/hyph_de_DE.dic \
	dictionaries/de/README_de_DE_frami.txt \
	dictionaries/de/README_extension_owner.txt \
	dictionaries/de/README_hyph_de.txt \
	dictionaries/de/README_thesaurus.txt \
))

$(eval $(call gb_Dictionary_add_thesauri,dict-de,\
	dictionaries/de/th_de_DE_v2.dat \
	dictionaries/de/th_de_CH_v2.dat \
))

# vim: set noet sw=4 ts=4:
