# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# This executable is required only by sal_osl_process test.
# See qa/osl/process/osl_process.cxx:411
$(eval $(call gb_Executable_Executable,osl_process_child))

$(eval $(call gb_Executable_use_libraries,osl_process_child,\
	sal \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Executable_add_exception_objects,osl_process_child,\
	sal/qa/osl/process/osl_process_child \
))

# Also this batch file is required by sal_osl_process.
# See qa/osl/process/osl_process.cxx:675
ifeq ($(OS),WNT)
sal_BATCH_FILE := batch.bat
else
sal_BATCH_FILE := batch.sh
endif

$(call gb_Executable_get_target,osl_process_child) : \
	$(WORKDIR)/LinkTarget/Executable/$(sal_BATCH_FILE)

$(WORKDIR)/LinkTarget/Executable/$(sal_BATCH_FILE) : \
		$(SRCDIR)/sal/qa/osl/process/$(sal_BATCH_FILE)
	mkdir -p $(dir $@) && \
	$(call gb_Deliver_deliver,$<,$@)

$(call gb_Executable_get_clean_target,osl_process_child) :
	rm -f $(WORKDIR)/LinkTarget/Executable/$(sal_BATCH_FILE)

# vim: set noet sw=4 ts=4:
