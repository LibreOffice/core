# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Dictionary_Dictionary,dict-th,dictionaries/th_TH))

$(eval $(call gb_Dictionary_add_root_files,dict-th,\
	dictionaries/th_TH/README_th_TH.txt \
	dictionaries/th_TH/th_TH.aff \
	dictionaries/th_TH/th_TH.dic \
	dictionaries/th_TH/README_hyph_th_TH.txt \
	dictionaries/th_TH/hyph_th_TH.dic \
))

# vim: set noet sw=4 ts=4:
