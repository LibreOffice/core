# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Dictionary_Dictionary,dict-lt,dictionaries/lt_LT))

$(eval $(call gb_Dictionary_add_root_files,dict-lt,\
	dictionaries/lt_LT/AUTHORS \
	dictionaries/lt_LT/COPYING \
	dictionaries/lt_LT/hyph_lt.dic \
	dictionaries/lt_LT/lt.aff \
	dictionaries/lt_LT/lt.dic \
	dictionaries/lt_LT/README \
	dictionaries/lt_LT/README_hyph \
))

# vim: set noet sw=4 ts=4:
