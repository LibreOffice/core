# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Dictionary_Dictionary,dict-da,dictionaries/da_DK))

$(eval $(call gb_Dictionary_add_root_files,dict-da,\
	dictionaries/da_DK/da_DK.aff \
	dictionaries/da_DK/da_DK.dic \
	dictionaries/da_DK/desc_da_DK.txt \
	dictionaries/da_DK/desc_en_US.txt \
	dictionaries/da_DK/hyph_da_DK.dic \
	dictionaries/da_DK/HYPH_da_DK_README.txt \
	dictionaries/da_DK/README_da_DK.txt \
	dictionaries/da_DK/README_th_da_DK.txt \
	dictionaries/da_DK/Trold_42x42.png \
))

$(eval $(call gb_Dictionary_add_files,dict-da,help/en,\
	dictionaries/da_DK/help/en/help.tree \
))

$(eval $(call gb_Dictionary_add_files,dict-da,help/da,\
	dictionaries/da_DK/help/da/help.tree \
))

$(eval $(call gb_Dictionary_add_files,dict-da,help/da/org.openoffice.da.hunspell.dictionaries,\
	dictionaries/da_DK/help/da/org.openoffice.da.hunspell.dictionaries/page1.xhp \
	dictionaries/da_DK/help/da/org.openoffice.da.hunspell.dictionaries/page2.xhp \
	dictionaries/da_DK/help/da/org.openoffice.da.hunspell.dictionaries/Trold.png \
))

$(eval $(call gb_Dictionary_add_thesauri,dict-da,\
	dictionaries/da_DK/th_da_DK.dat \
))

# vim: set noet sw=4 ts=4:
