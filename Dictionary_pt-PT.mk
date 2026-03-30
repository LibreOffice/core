# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Dictionary_Dictionary,dict-pt-PT,dictionaries/pt_PT))

$(eval $(call gb_Dictionary_add_root_files,dict-pt-PT,\
	dictionaries/pt_PT/hyph_pt_PT.dic \
	dictionaries/pt_PT/icon.png \
	dictionaries/pt_PT/LICENSES.txt \
	dictionaries/pt_PT/pt_PT.aff \
	dictionaries/pt_PT/pt_PT.dic \
	dictionaries/pt_PT/README_hyph_pt_PT.txt \
	dictionaries/pt_PT/README_pt_PT.txt \
	dictionaries/pt_PT/README_th_pt_PT.txt \
))

$(eval $(call gb_Dictionary_add_thesauri,dict-pt-PT,\
	dictionaries/pt_PT/th_pt_PT.dat \
))

# vim: set noet sw=4 ts=4:
