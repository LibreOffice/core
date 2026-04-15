# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Dictionary_Dictionary,dict-mn,dictionaries/mn_MN))

$(eval $(call gb_Dictionary_add_root_files,dict-mn,\
	dictionaries/mn_MN/hyph_mn_MN.dic \
	dictionaries/mn_MN/lppl.txt \
	dictionaries/mn_MN/README_mn_MN.txt \
	dictionaries/mn_MN/mn_MN.aff \
	dictionaries/mn_MN/mn_MN.dic \
        dictionaries/mn_MN/DictMN.png \
))

# vim: set noet sw=4 ts=4:
