# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Dictionary_Dictionary,dict-be,dictionaries/be_BY))

$(eval $(call gb_Dictionary_add_root_files,dict-be,\
	dictionaries/be_BY/be-official.aff \
	dictionaries/be_BY/be-official.dic \
	dictionaries/be_BY/hyph_be_BY.dic \
	dictionaries/be_BY/README_be_BY.txt \
))

# vim: set noet sw=4 ts=4:
