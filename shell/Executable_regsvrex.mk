# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,regsvrex))

$(eval $(call gb_Executable_add_exception_objects,regsvrex,\
    shell/source/tools/regsvrex/regsvrex \
))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
