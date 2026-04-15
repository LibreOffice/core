# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Dictionary_Dictionary,dict-lv,dictionaries/lv_LV))

$(eval $(call gb_Dictionary_add_root_files,dict-lv,\
	dictionaries/lv_LV/hyph_lv_LV.dic \
	dictionaries/lv_LV/license.txt \
	dictionaries/lv_LV/lv_LV.aff \
	dictionaries/lv_LV/lv_LV.dic \
	dictionaries/lv_LV/README_hyph_lv_LV.txt \
	dictionaries/lv_LV/README_lv_LV.txt \
	dictionaries/lv_LV/README_th_lv_LV_v2.txt \
))

$(eval $(call gb_Dictionary_add_thesauri,dict-lv,\
	dictionaries/lv_LV/th_lv_LV_v2.dat \
))

# vim: set noet sw=4 ts=4:
