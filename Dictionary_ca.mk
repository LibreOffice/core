# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Dictionary_Dictionary,dict-ca,dictionaries/ca))

$(eval $(call gb_Dictionary_add_root_files,dict-ca,\
	dictionaries/ca/dictionaries/ca.aff \
	dictionaries/ca/dictionaries/ca.dic \
	dictionaries/ca/dictionaries/ca-valencia.aff \
	dictionaries/ca/dictionaries/ca-valencia.dic \
	dictionaries/ca/dictionaries/hyph_ca.dic \
	dictionaries/ca/LICENSES-en.txt \
	dictionaries/ca/LLICENCIES-ca.txt \
))

$(eval $(call gb_Dictionary_add_files,dict-ca,images,\
    dictionaries/ca/images/SC-logo.png \
))

$(eval $(call gb_Dictionary_add_thesauri,dict-ca,\
	dictionaries/ca/dictionaries/th_ca_ES_v3.dat \
))

# vim: set noet sw=4 ts=4:
