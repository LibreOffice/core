# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# Copyright the Collabora Office contributors.
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

include $(SRCDIR)/vcl/commonfuzzer.mk

$(eval $(call gb_Executable_Executable,sftfuzzer))

$(eval $(call gb_Executable_use_api,sftfuzzer,\
    offapi \
    udkapi \
))

$(eval $(call gb_Executable_use_externals,sftfuzzer,\
	$(fuzzer_externals) \
))

$(eval $(call gb_Executable_set_include,sftfuzzer,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
))

$(eval $(call gb_Executable_use_libraries,sftfuzzer,\
    $(fuzzer_core_libraries) \
    guesslang \
    pdffilter \
))

$(eval $(call gb_Executable_use_static_libraries,sftfuzzer,\
    $(fuzzer_statics) \
    fuzzer_core \
))

$(eval $(call gb_Executable_add_exception_objects,sftfuzzer,\
    vcl/workben/sftfuzzer \
))

$(eval $(call gb_Executable_add_libs,sftfuzzer,\
    $(LIB_FUZZING_ENGINE) \
))

# vim: set noet sw=4 ts=4:
