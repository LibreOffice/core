# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Dictionary_Dictionary,dict-vi,dictionaries/vi))

$(eval $(call gb_Dictionary_add_root_files,dict-vi,\
	dictionaries/vi/LICENSES-en.txt \
	dictionaries/vi/LICENSES-vi.txt \
	dictionaries/vi/vi_VN.aff \
	dictionaries/vi/vi_VN.dic \
))

# vim: set noet sw=4 ts=4:
