# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Dictionary_Dictionary,dict-cs,dictionaries/cs_CZ))

$(eval $(call gb_Dictionary_add_root_files,dict-cs,\
	dictionaries/cs_CZ/cs_CZ.aff \
	dictionaries/cs_CZ/cs_CZ.dic \
	dictionaries/cs_CZ/hyph_cs_CZ.dic \
	dictionaries/cs_CZ/README_cs.txt \
	dictionaries/cs_CZ/README_en.txt \
))

$(eval $(call gb_Dictionary_add_thesauri,dict-cs,\
	dictionaries/cs_CZ/thes_cs_CZ.dat \
))

# vim: set noet sw=4 ts=4:
