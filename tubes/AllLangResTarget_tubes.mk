# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
$(eval $(call gb_AllLangResTarget_AllLangResTarget,tubes))

$(eval $(call gb_AllLangResTarget_add_srs,tubes,tubes/res))

$(eval $(call gb_SrsTarget_SrsTarget,tubes/res))

$(eval $(call gb_SrsTarget_add_files,tubes/res,\
	tubes/source/contacts.src \
))

# vim: set noet sw=4 ts=4:
