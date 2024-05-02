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

$(eval $(call gb_CustomTarget_CustomTarget,unoidl/unoidl-check_test))

.PHONY: $(call gb_CustomTarget_get_target,unoidl/unoidl-check_test)

$(call gb_CustomTarget_get_target,unoidl/unoidl-check_test): \
        $(call gb_Executable_get_runtime_dependencies,unoidl-check) \
        $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/other1.rdb \
        $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/other2.rdb \
        $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/pe.rdb \
        $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/psa.rdb \
        $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/psb.rdb \
        $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/ue.rdb \
        $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/usa.rdb \
        $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/usb.rdb
ifneq ($(gb_SUPPRESS_TESTS),)
	@true
else
	$(call gb_Helper_abbreviate_dirs,( \
        $(call gb_Executable_get_command,unoidl-check) --ignore-unpublished \
            $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/psa.rdb \
            -- $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/psa.rdb) \
        > $@.log 2>&1 || (cat $@.log && false))
	$(call gb_Helper_abbreviate_dirs,( \
        ! $(call gb_Executable_get_command,unoidl-check) --ignore-unpublished \
            $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/psa.rdb \
            -- $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/psb.rdb) \
        > $@.log 2>&1 || (cat $@.log && false))
	$(call gb_Helper_abbreviate_dirs,( \
        ! $(call gb_Executable_get_command,unoidl-check) --ignore-unpublished \
            $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/psa.rdb \
            -- $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/usa.rdb) \
        > $@.log 2>&1 || (cat $@.log && false))
	$(call gb_Helper_abbreviate_dirs,( \
        ! $(call gb_Executable_get_command,unoidl-check) --ignore-unpublished \
            $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/psa.rdb \
            -- $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/usb.rdb) \
        > $@.log 2>&1 || (cat $@.log && false))
	$(call gb_Helper_abbreviate_dirs,( \
        ! $(call gb_Executable_get_command,unoidl-check) --ignore-unpublished \
            $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/psa.rdb \
            -- $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/pe.rdb) \
        > $@.log 2>&1 || (cat $@.log && false))
	$(call gb_Helper_abbreviate_dirs,( \
        ! $(call gb_Executable_get_command,unoidl-check) --ignore-unpublished \
            $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/psa.rdb \
            -- $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/ue.rdb) \
        > $@.log 2>&1 || (cat $@.log && false))
	$(call gb_Helper_abbreviate_dirs,( \
        ! $(call gb_Executable_get_command,unoidl-check) --ignore-unpublished \
            $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/psa.rdb \
            -- $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/other1.rdb) \
        > $@.log 2>&1 || (cat $@.log && false))
	$(call gb_Helper_abbreviate_dirs,( \
        ! $(call gb_Executable_get_command,unoidl-check) --ignore-unpublished \
            $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/psa.rdb \
            -- $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/other2.rdb) \
        > $@.log 2>&1 || (cat $@.log && false))
	$(call gb_Helper_abbreviate_dirs,( \
        $(call gb_Executable_get_command,unoidl-check) \
            $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/psa.rdb \
            -- $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/psa.rdb) \
        > $@.log 2>&1 || (cat $@.log && false))
	$(call gb_Helper_abbreviate_dirs,( \
        ! $(call gb_Executable_get_command,unoidl-check) \
            $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/psa.rdb \
            -- $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/psb.rdb) \
        > $@.log 2>&1 || (cat $@.log && false))
	$(call gb_Helper_abbreviate_dirs,( \
        ! $(call gb_Executable_get_command,unoidl-check) \
            $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/psa.rdb \
            -- $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/usa.rdb) \
        > $@.log 2>&1 || (cat $@.log && false))
	$(call gb_Helper_abbreviate_dirs,( \
        ! $(call gb_Executable_get_command,unoidl-check) \
            $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/psa.rdb \
            -- $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/usb.rdb) \
        > $@.log 2>&1 || (cat $@.log && false))
	$(call gb_Helper_abbreviate_dirs,( \
        ! $(call gb_Executable_get_command,unoidl-check) \
            $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/psa.rdb \
            -- $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/pe.rdb) \
        > $@.log 2>&1 || (cat $@.log && false))
	$(call gb_Helper_abbreviate_dirs,( \
        ! $(call gb_Executable_get_command,unoidl-check) \
            $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/psa.rdb \
            -- $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/ue.rdb) \
        > $@.log 2>&1 || (cat $@.log && false))
	$(call gb_Helper_abbreviate_dirs,( \
        ! $(call gb_Executable_get_command,unoidl-check) \
            $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/psa.rdb \
            -- $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/other1.rdb) \
        > $@.log 2>&1 || (cat $@.log && false))
	$(call gb_Helper_abbreviate_dirs,( \
        ! $(call gb_Executable_get_command,unoidl-check) \
            $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/psa.rdb \
            -- $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/other2.rdb) \
        > $@.log 2>&1 || (cat $@.log && false))
	$(call gb_Helper_abbreviate_dirs,( \
        $(call gb_Executable_get_command,unoidl-check) --ignore-unpublished \
            $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/usa.rdb \
            -- $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/psa.rdb) \
        > $@.log 2>&1 || (cat $@.log && false))
	$(call gb_Helper_abbreviate_dirs,( \
        $(call gb_Executable_get_command,unoidl-check) --ignore-unpublished \
            $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/usa.rdb \
            -- $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/psb.rdb) \
        > $@.log 2>&1 || (cat $@.log && false))
	$(call gb_Helper_abbreviate_dirs,( \
        $(call gb_Executable_get_command,unoidl-check) --ignore-unpublished \
            $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/usa.rdb \
            -- $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/usa.rdb) \
        > $@.log 2>&1 || (cat $@.log && false))
	$(call gb_Helper_abbreviate_dirs,( \
        $(call gb_Executable_get_command,unoidl-check) --ignore-unpublished \
            $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/usa.rdb \
            -- $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/usb.rdb) \
        > $@.log 2>&1 || (cat $@.log && false))
	$(call gb_Helper_abbreviate_dirs,( \
        $(call gb_Executable_get_command,unoidl-check) --ignore-unpublished \
            $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/usa.rdb \
            -- $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/pe.rdb) \
        > $@.log 2>&1 || (cat $@.log && false))
	$(call gb_Helper_abbreviate_dirs,( \
        $(call gb_Executable_get_command,unoidl-check) --ignore-unpublished \
            $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/usa.rdb \
            -- $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/ue.rdb) \
        > $@.log 2>&1 || (cat $@.log && false))
	$(call gb_Helper_abbreviate_dirs,( \
        $(call gb_Executable_get_command,unoidl-check) --ignore-unpublished \
            $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/usa.rdb \
            -- $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/other1.rdb) \
        > $@.log 2>&1 || (cat $@.log && false))
	$(call gb_Helper_abbreviate_dirs,( \
        $(call gb_Executable_get_command,unoidl-check) --ignore-unpublished \
            $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/usa.rdb \
            -- $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/other2.rdb) \
        > $@.log 2>&1 || (cat $@.log && false))
	$(call gb_Helper_abbreviate_dirs,( \
        $(call gb_Executable_get_command,unoidl-check) \
            $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/usa.rdb \
            -- $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/psa.rdb) \
        > $@.log 2>&1 || (cat $@.log && false))
	$(call gb_Helper_abbreviate_dirs,( \
        ! $(call gb_Executable_get_command,unoidl-check) \
            $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/usa.rdb \
            -- $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/psb.rdb) \
        > $@.log 2>&1 || (cat $@.log && false))
	$(call gb_Helper_abbreviate_dirs,( \
        $(call gb_Executable_get_command,unoidl-check) \
            $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/usa.rdb \
            -- $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/usa.rdb) \
        > $@.log 2>&1 || (cat $@.log && false))
	$(call gb_Helper_abbreviate_dirs,( \
        ! $(call gb_Executable_get_command,unoidl-check) \
            $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/usa.rdb \
            -- $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/usb.rdb) \
        > $@.log 2>&1 || (cat $@.log && false))
	$(call gb_Helper_abbreviate_dirs,( \
        ! $(call gb_Executable_get_command,unoidl-check) \
            $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/usa.rdb \
            -- $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/pe.rdb) \
        > $@.log 2>&1 || (cat $@.log && false))
	$(call gb_Helper_abbreviate_dirs,( \
        ! $(call gb_Executable_get_command,unoidl-check) \
            $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/usa.rdb \
            -- $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/ue.rdb) \
        > $@.log 2>&1 || (cat $@.log && false))
	$(call gb_Helper_abbreviate_dirs,( \
        ! $(call gb_Executable_get_command,unoidl-check) \
            $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/usa.rdb \
            -- $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/other1.rdb) \
        > $@.log 2>&1 || (cat $@.log && false))
	$(call gb_Helper_abbreviate_dirs,( \
        ! $(call gb_Executable_get_command,unoidl-check) \
            $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/usa.rdb \
            -- $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/other2.rdb) \
        > $@.log 2>&1 || (cat $@.log && false))
endif

$(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/%.rdb: \
        $(SRCDIR)/unoidl/qa/unoidl-check/%.idl \
        $(call gb_Executable_get_runtime_dependencies,unoidl-write) \
        | $(gb_CustomTarget_workdir)/unoidl/unoidl-check_test/.dir
	$(call gb_Helper_abbreviate_dirs,( \
        $(call gb_Executable_get_command,unoidl-write) $< $@))

# vim: set noet sw=4 ts=4:
