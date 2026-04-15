# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Dictionary_Dictionary,dict-te,dictionaries/te_IN))

$(eval $(call gb_Dictionary_add_root_files,dict-te,\
	dictionaries/te_IN/hyph_te_IN.dic \
	dictionaries/te_IN/README_hyph_te_IN.txt \
	dictionaries/te_IN/README_te_IN.txt \
	dictionaries/te_IN/te_IN.aff \
	dictionaries/te_IN/te_IN.dic \
))

# vim: set noet sw=4 ts=4:
