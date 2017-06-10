# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

$(eval $(call gb_StaticLibrary_StaticLibrary,fuzzer_draw))

$(eval $(call gb_StaticLibrary_set_include,fuzzer_draw,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
))

$(eval $(call gb_StaticLibrary_use_api,fuzzer_draw,\
    offapi \
    udkapi \
))

$(eval $(call gb_StaticLibrary_add_generated_exception_objects,fuzzer_draw,\
    CustomTarget/vcl/workben/native-draw \
))

# vim: set noet sw=4 ts=4:
