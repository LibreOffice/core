# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

ifeq ($(gb_Side),)
gb_Side:=host
endif

ifeq (,$(BUILDDIR))
gb_partial_build__makefile_dir=$(dir $(abspath $(firstword $(MAKEFILE_LIST))))
BUILDDIR := $(if $(wildcard $(gb_partial_build__makefile_dir)../Module_external.mk), \
  $(gb_partial_build__makefile_dir)../.., \
  $(gb_partial_build__makefile_dir)..)
endif

ifeq ($(BUILD_TYPE),)
include $(BUILDDIR)/config_$(gb_Side).mk
endif

gb_PARTIAL_BUILD := T
include $(SRCDIR)/solenv/gbuild/gbuild.mk

$(eval $(call gb_Module_make_global_targets,$(wildcard $(module_directory)Module*.mk)))

ifeq ($(DISABLE_DYNLOADING),TRUE)
$(if $(gb_LinkTarget__Lock),$(shell rm -f $(gb_LinkTarget__Lock)))
include $(SRCDIR)/solenv/gbuild/static.mk
$(if $(filter a,$(gb_DEBUG_STATIC)),$(error Abort after static.mk))
endif

# vim: set noet sw=4 ts=4:
