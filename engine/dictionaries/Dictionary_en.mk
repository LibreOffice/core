# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Dictionary_Dictionary,dict-en,dictionaries/en))

$(eval $(call gb_Dictionary_add_root_files,dict-en,\
	dictionaries/en/affDescription.txt \
	dictionaries/en/changelog.txt \
	dictionaries/en/English.png \
	dictionaries/en/en_AU.aff \
	dictionaries/en/en_AU.dic \
	dictionaries/en/en_CA.aff \
	dictionaries/en/en_CA.dic \
	dictionaries/en/en_GB.aff \
	dictionaries/en/en_GB.dic \
	dictionaries/en/en_US.aff \
	dictionaries/en/en_US.dic \
	dictionaries/en/en_ZA.aff \
	dictionaries/en/en_ZA.dic \
	dictionaries/en/hyph_en_GB.dic \
	dictionaries/en/hyph_en_US.dic \
	dictionaries/en/license.txt \
	dictionaries/en/package-description.txt \
	dictionaries/en/README_en_AU.txt \
	dictionaries/en/README_en_CA.txt \
	dictionaries/en/README_en_GB_thes.txt \
	dictionaries/en/README_en_GB.txt \
	dictionaries/en/README_en_US.txt \
	dictionaries/en/README_en_ZA.txt \
	dictionaries/en/README_hyph_en_GB.txt \
	dictionaries/en/README_hyph_en_US.txt \
	dictionaries/en/README_lightproof_en.txt \
	dictionaries/en/README.txt \
	dictionaries/en/WordNet_license.txt \
))

$(eval $(call gb_Dictionary_add_thesaurus,dict-en,dictionaries/en/th_en_US_v2.dat))

# vim: set noet sw=4 ts=4:
