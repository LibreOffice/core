# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Dictionary_Dictionary,dict-sv,dictionaries/sv_SE))

$(eval $(call gb_Dictionary_add_root_files,dict-sv,\
	dictionaries/sv_SE/LICENSE_en_US.txt \
	dictionaries/sv_SE/LICENSE_sv_SE.txt \
	dictionaries/sv_SE/README_hyph_sv.txt \
	dictionaries/sv_SE/README_th_sv_SE.txt \
	dictionaries/sv_SE/hyph_sv.dic \
	dictionaries/sv_SE/sv_FI.aff \
	dictionaries/sv_SE/sv_FI.dic \
	dictionaries/sv_SE/sv_SE.aff \
	dictionaries/sv_SE/sv_SE.dic \
))

$(eval $(call gb_Dictionary_add_thesauri,dict-sv,\
	dictionaries/sv_SE/th_sv_SE.dat \
))

# vim: set noet sw=4 ts=4:
