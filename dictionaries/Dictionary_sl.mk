# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Dictionary_Dictionary,dict-sl,dictionaries/sl_SI))

$(eval $(call gb_Dictionary_add_root_files,dict-sl,\
	dictionaries/sl_SI/hyph_sl_SI.dic \
	dictionaries/sl_SI/icon.png \
	dictionaries/sl_SI/README_hyph_sl_SI.txt \
	dictionaries/sl_SI/README_sl_SI.txt \
	dictionaries/sl_SI/README_th_sl_SI.txt \
	dictionaries/sl_SI/sl_SI.aff \
	dictionaries/sl_SI/sl_SI.dic \
	dictionaries/sl_SI/package-description.txt \
))

$(eval $(call gb_Dictionary_add_thesauri,dict-sl,\
	dictionaries/sl_SI/th_sl_SI_v2.dat \
))

# vim: set noet sw=4 ts=4:
