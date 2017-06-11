# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

include $(SRCDIR)/vcl/commonfuzzer.mk

$(eval $(call gb_Executable_Executable,tgafuzzer))

$(eval $(call gb_Executable_use_api,tgafuzzer,\
    offapi \
    udkapi \
))

$(eval $(call gb_Executable_use_externals,tgafuzzer,\
	$(fuzzer_externals) \
))

$(eval $(call gb_Executable_set_include,tgafuzzer,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
))

$(eval $(call gb_Executable_use_libraries,tgafuzzer,\
    $(fuzzer_core_libraries) \
))

$(eval $(call gb_Executable_use_static_libraries,tgafuzzer,\
    findsofficepath \
    ulingu \
    fuzzer_core \
    fuzzerstubs \
))

$(eval $(call gb_Executable_add_exception_objects,tgafuzzer,\
	vcl/workben/tgafuzzer \
))

$(eval $(call gb_Executable_add_libs,tgafuzzer,\
	-lFuzzingEngine \
))

# vim: set noet sw=4 ts=4:
