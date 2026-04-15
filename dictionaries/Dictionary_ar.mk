# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Dictionary_Dictionary,dict-ar,dictionaries/ar))

$(eval $(call gb_Dictionary_add_root_files,dict-ar,\
	dictionaries/ar/ar.aff \
	dictionaries/ar/ar.dic \
	dictionaries/ar/AUTHORS.txt \
	dictionaries/ar/COPYING.txt \
	dictionaries/ar/README_ar.txt \
))

$(eval $(call gb_Dictionary_add_thesauri,dict-ar,\
	dictionaries/ar/th_ar.dat \
))

# vim: set noet sw=4 ts=4:
