# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Dictionary_Dictionary,dict-sk,dictionaries/sk_SK))

$(eval $(call gb_Dictionary_add_root_files,dict-sk,\
	dictionaries/sk_SK/hyph_sk_SK.dic \
	dictionaries/sk_SK/LICENSE.txt \
	dictionaries/sk_SK/README_en.txt \
	dictionaries/sk_SK/README_sk.txt \
	dictionaries/sk_SK/README_th_sk_SK_v2.txt \
	dictionaries/sk_SK/release_en.txt \
	dictionaries/sk_SK/release_sk.txt \
	dictionaries/sk_SK/sk_SK.aff \
	dictionaries/sk_SK/sk_SK.dic \
))

$(eval $(call gb_Dictionary_add_thesauri,dict-sk,\
	dictionaries/sk_SK/th_sk_SK_v2.dat \
))

# vim: set noet sw=4 ts=4:
