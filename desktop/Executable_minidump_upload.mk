# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,minidump_upload))

$(eval $(call gb_Executable_use_libraries,minidump_upload,\
	crashreport \
    sal \
))

$(eval $(call gb_Executable_add_defs,minidump_upload,\
	-DUNICODE \
))

$(eval $(call gb_Executable_use_external,minidump_upload,curl))

$(eval $(call gb_Executable_add_exception_objects,minidump_upload,\
    desktop/source/minidump/minidump_upload \
))

# vim: set noet sw=4 ts=4:
