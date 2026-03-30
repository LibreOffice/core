# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Dictionary_Dictionary,dict-pl,dictionaries/pl_PL))

$(eval $(call gb_Dictionary_add_root_files,dict-pl,\
	dictionaries/pl_PL/hyph_pl_PL.dic \
	dictionaries/pl_PL/plhyph.tex \
	dictionaries/pl_PL/pl_PL.aff \
	dictionaries/pl_PL/pl_PL.dic \
	dictionaries/pl_PL/README_en.txt \
	dictionaries/pl_PL/README_pl.txt \
))

$(eval $(call gb_Dictionary_add_thesauri,dict-pl,\
	dictionaries/pl_PL/th_pl_PL_v2.dat \
))

# vim: set noet sw=4 ts=4:
