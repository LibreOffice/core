# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Dictionary_Dictionary,dict-oc,dictionaries/oc_FR))

$(eval $(call gb_Dictionary_add_root_files,dict-oc,\
	dictionaries/oc_FR/LICENCES-fr.txt \
	dictionaries/oc_FR/LICENSES-en.txt \
	dictionaries/oc_FR/oc_FR.aff \
	dictionaries/oc_FR/oc_FR.dic \
	dictionaries/oc_FR/README_oc_FR.txt \
))

# vim: set noet sw=4 ts=4:
