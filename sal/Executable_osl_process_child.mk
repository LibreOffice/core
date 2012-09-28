# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# Major Contributor(s):
# Copyright (C) 2012 Matúš Kukan <matus.kukan@gmail.com> (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

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
