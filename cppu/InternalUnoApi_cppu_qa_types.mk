# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_InternalUnoApi_InternalUnoApi,cppu_qa_types))

$(eval $(call gb_InternalUnoApi_use_api,cppu_qa_types,\
    udkapi \
))

$(eval $(call gb_InternalUnoApi_set_include,cppu_qa_types,\
    -I$(SRCDIR)/udkapi \
    $$(INCLUDE) \
))

$(eval $(call gb_InternalUnoApi_add_idlfiles,cppu_qa_types,cppu/qa,\
    types \
))

# vim:set noet sw=4 ts=4:
