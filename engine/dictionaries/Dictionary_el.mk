# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Dictionary_Dictionary,dict-el,dictionaries/el_GR))

$(eval $(call gb_Dictionary_add_root_files,dict-el,\
	dictionaries/el_GR/el_GR.aff \
	dictionaries/el_GR/el_GR.dic \
	dictionaries/el_GR/hyph_el_GR.dic \
	dictionaries/el_GR/README_el_GR.txt \
	dictionaries/el_GR/README_hyph_el_GR.txt \
))

# vim: set noet sw=4 ts=4:
