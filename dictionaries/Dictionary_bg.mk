# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Dictionary_Dictionary,dict-bg,dictionaries/bg_BG))

$(eval $(call gb_Dictionary_add_root_files,dict-bg,\
	dictionaries/bg_BG/bg_BG.aff \
	dictionaries/bg_BG/bg_BG.dic \
	dictionaries/bg_BG/COPYING \
	dictionaries/bg_BG/hyph_bg_BG.dic \
	dictionaries/bg_BG/README_hyph_bg_BG.txt \
	dictionaries/bg_BG/README_th_bg_BG_v2.txt \
))

$(eval $(call gb_Dictionary_add_thesauri,dict-bg,\
	dictionaries/bg_BG/th_bg_BG_v2.dat \
))

# vim: set noet sw=4 ts=4:
