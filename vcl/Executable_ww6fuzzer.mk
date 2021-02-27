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

$(eval $(call gb_Executable_Executable,ww6fuzzer))

$(eval $(call gb_Executable_use_api,ww6fuzzer,\
    offapi \
    udkapi \
))

$(eval $(call gb_Executable_use_externals,ww6fuzzer,\
	$(fuzzer_externals) \
))

$(eval $(call gb_Executable_set_include,ww6fuzzer,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
))

$(eval $(call gb_Executable_use_libraries,ww6fuzzer,\
    $(fuzzer_writer_libraries) \
    $(fuzzer_core_libraries) \
))

$(eval $(call gb_Executable_use_static_libraries,ww6fuzzer,\
    findsofficepath \
    ulingu \
    fuzzerstubs \
))

$(eval $(call gb_Executable_add_exception_objects,ww6fuzzer,\
	vcl/workben/ww6fuzzer \
))

$(eval $(call gb_Executable_add_libs,ww6fuzzer,\
	$(LIB_FUZZING_ENGINE) \
))

# vim: set noet sw=4 ts=4:
