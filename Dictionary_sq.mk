# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Dictionary_Dictionary,dict-sq,dictionaries/sq_AL))

$(eval $(call gb_Dictionary_add_root_files,dict-sq,\
	dictionaries/sq_AL/sq_AL.aff \
	dictionaries/sq_AL/sq_AL.dic \
	dictionaries/sq_AL/hyph_sq_AL.dic \
	dictionaries/sq_AL/README_hyph_sq_AL.txt \
	dictionaries/sq_AL/README.txt \
))

# vim: set noet sw=4 ts=4:
