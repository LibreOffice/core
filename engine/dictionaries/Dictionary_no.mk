# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Dictionary_Dictionary,dict-no,dictionaries/no))

$(eval $(call gb_Dictionary_add_root_files,dict-no,\
	dictionaries/no/COPYING \
	dictionaries/no/hyph_nb_NO.dic \
	dictionaries/no/hyph_nn_NO.dic \
	dictionaries/no/nb_NO.aff \
	dictionaries/no/nb_NO.dic \
	dictionaries/no/nn_NO.aff \
	dictionaries/no/nn_NO.dic \
	dictionaries/no/README_hyph_NO.txt \
))

$(eval $(call gb_Dictionary_add_thesauri,dict-no,\
	dictionaries/no/th_nb_NO_v2.dat \
	dictionaries/no/th_nn_NO_v2.dat \
))

# vim: set noet sw=4 ts=4:
