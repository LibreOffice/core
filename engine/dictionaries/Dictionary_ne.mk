# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Dictionary_Dictionary,dict-ne,dictionaries/ne_NP))

$(eval $(call gb_Dictionary_add_root_files,dict-ne,\
	dictionaries/ne_NP/ne_NP.aff \
	dictionaries/ne_NP/ne_NP.dic \
	dictionaries/ne_NP/README_ne_NP.txt \
	dictionaries/ne_NP/README_th_ne_NP_v2.txt \
))

$(eval $(call gb_Dictionary_add_thesauri,dict-ne,\
	dictionaries/ne_NP/th_ne_NP_v2.dat \
))

# vim: set noet sw=4 ts=4:
