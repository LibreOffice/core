# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Dictionary_Dictionary,dict-fr,dictionaries/fr_FR))

$(eval $(call gb_Dictionary_add_root_files,dict-fr,\
	dictionaries/fr_FR/fr.aff \
	dictionaries/fr_FR/fr.dic \
	dictionaries/fr_FR/hyph_fr.dic \
	dictionaries/fr_FR/hyph-fr.tex \
	dictionaries/fr_FR/icon.png \
	dictionaries/fr_FR/package-description.txt \
	dictionaries/fr_FR/README_fr.txt \
	dictionaries/fr_FR/README_hyph_fr.txt \
	dictionaries/fr_FR/README_thes_fr.txt \
))

$(eval $(call gb_Dictionary_add_thesauri,dict-fr,\
	dictionaries/fr_FR/thes_fr.dat \
))

# vim: set noet sw=4 ts=4:
