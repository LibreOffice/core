# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Dictionary_Dictionary,dict-lo,dictionaries/lo_LA))

$(eval $(call gb_Dictionary_add_root_files,dict-lo,\
	dictionaries/lo_LA/README_lo_LA.txt \
	dictionaries/lo_LA/lo_LA.aff \
	dictionaries/lo_LA/lo_LA.dic \
))

# vim: set noet sw=4 ts=4:
