# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Dictionary_Dictionary,dict-fa,dictionaries/fa_IR))

$(eval $(call gb_Dictionary_add_root_files,dict-fa,\
	dictionaries/fa_IR/fa-IR.aff \
	dictionaries/fa_IR/fa-IR.dic \
	dictionaries/fa_IR/LICENSE \
	dictionaries/fa_IR/README_fa_IR.txt \
	dictionaries/fa_IR/icon.png \
	dictionaries/fa_IR/package-description.txt \
))

# vim: set noet sw=4 ts=4:
