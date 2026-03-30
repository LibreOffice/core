# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Dictionary_Dictionary,dict-nl,dictionaries/nl_NL))

$(eval $(call gb_Dictionary_add_root_files,dict-nl,\
	dictionaries/nl_NL/hyph_nl_NL.dic \
	dictionaries/nl_NL/license_en_EN.txt \
	dictionaries/nl_NL/licentie_nl_NL.txt \
	dictionaries/nl_NL/nl_NL.aff \
	dictionaries/nl_NL/nl_NL.dic \
	dictionaries/nl_NL/OpenTaal.png \
	dictionaries/nl_NL/README_NL.txt \
))

# vim: set noet sw=4 ts=4:
