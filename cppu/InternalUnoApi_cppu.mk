# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_InternalUnoApi_InternalUnoApi,cppu))

$(eval $(call gb_InternalUnoApi_use_api,cppu,\
    udkapi \
))

$(eval $(call gb_InternalUnoApi_set_include,cppu,\
    -I$(SRCDIR)/udkapi \
    $$(INCLUDE) \
))

$(eval $(call gb_InternalUnoApi_add_idlfiles,cppu,cppu/qa,\
    types \
))

$(eval $(call gb_InternalUnoApi_add_idlfiles,cppu,cppu/qa/cppumaker,\
    types \
))

# vim:set noet sw=4 ts=4:
