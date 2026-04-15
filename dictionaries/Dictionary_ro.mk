# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Dictionary_Dictionary,dict-ro,dictionaries/ro))

$(eval $(call gb_Dictionary_add_root_files,dict-ro,\
	dictionaries/ro/COPYING.GPL \
	dictionaries/ro/COPYING.LGPL \
	dictionaries/ro/COPYING.MPL \
	dictionaries/ro/hyph_ro_RO.dic \
	dictionaries/ro/README_EN.txt \
	dictionaries/ro/README_RO.txt \
	dictionaries/ro/ro_RO.aff \
	dictionaries/ro/ro_RO.dic \
))

$(eval $(call gb_Dictionary_add_thesauri,dict-ro,\
	dictionaries/ro/th_ro_RO_v2.dat \
))

# vim: set noet sw=4 ts=4:
