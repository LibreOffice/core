# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Dictionary_Dictionary,dict-et,dictionaries/et_EE))

$(eval $(call gb_Dictionary_add_root_files,dict-et,\
	dictionaries/et_EE/eehyph.tex \
	dictionaries/et_EE/et_EE.aff \
	dictionaries/et_EE/et_EE.dic \
	dictionaries/et_EE/hyph_et_EE.dic \
	dictionaries/et_EE/README_et_EE.txt \
	dictionaries/et_EE/README_hyph_et_EE.txt \
))

# vim: set noet sw=4 ts=4:
