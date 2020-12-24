# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,ucrt,$(UCRT_REDISTDIR)))

$(eval $(call gb_Package_add_files,ucrt,$(LIBO_ETC_FOLDER),\
    Windows6.1-KB3118401-x64.msu \
    Windows6.1-KB3118401-x86.msu \
    Windows8.1-KB3118401-x64.msu \
    Windows8.1-KB3118401-x86.msu \
    Windows8-RT-KB3118401-x64.msu \
    Windows8-RT-KB3118401-x86.msu \
))

# vim:set shiftwidth=4 tabstop=4 noexpandtab:
