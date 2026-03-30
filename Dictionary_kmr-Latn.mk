# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Dictionary_Dictionary,dict-kmr-Latn,dictionaries/kmr_Latn))

$(eval $(call gb_Dictionary_add_root_files,dict-kmr-Latn,\
	dictionaries/kmr_Latn/ferheng.org.png \
	dictionaries/kmr_Latn/gpl-3.0.txt \
	dictionaries/kmr_Latn/kmr_Latn.aff \
	dictionaries/kmr_Latn/kmr_Latn.dic \
	dictionaries/kmr_Latn/lgpl-2.1.txt \
	dictionaries/kmr_Latn/license.txt \
	dictionaries/kmr_Latn/MPL-1.1.txt \
	dictionaries/kmr_Latn/README_kmr_Latn.txt \
))

# vim: set noet sw=4 ts=4:
