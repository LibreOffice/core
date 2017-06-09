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

$(eval $(call gb_CustomTarget_CustomTarget,registry/regcompare_test))

.PHONY: $(call gb_CustomTarget_get_target,registry/regcompare_test)

$(call gb_CustomTarget_get_target,registry/regcompare_test): \
        $(call gb_Executable_get_runtime_dependencies,unoidl-check) \
        $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/other1.rdb \
        $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/other2.rdb \
        $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/pe.rdb \
        $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/psa.rdb \
        $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/psb.rdb \
        $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/ue.rdb \
        $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/usa.rdb \
        $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/usb.rdb
ifneq ($(gb_SUPPRESS_TESTS),)
	@true
else
	$(call gb_Helper_abbreviate_dirs,( \
        $(call gb_Executable_get_command,unoidl-check) --ignore-unpublished \
            $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/psa.rdb \
            -- $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/psa.rdb) \
        > $@.log 2>&1 || (cat $@.log && false))
	$(call gb_Helper_abbreviate_dirs,( \
        ! $(call gb_Executable_get_command,unoidl-check) --ignore-unpublished \
            $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/psa.rdb \
            -- $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/psb.rdb) \
        > $@.log 2>&1 || (cat $@.log && false))
	$(call gb_Helper_abbreviate_dirs,( \
        ! $(call gb_Executable_get_command,unoidl-check) --ignore-unpublished \
            $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/psa.rdb \
            -- $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/usa.rdb) \
        > $@.log 2>&1 || (cat $@.log && false))
	$(call gb_Helper_abbreviate_dirs,( \
        ! $(call gb_Executable_get_command,unoidl-check) --ignore-unpublished \
            $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/psa.rdb \
            -- $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/usb.rdb) \
        > $@.log 2>&1 || (cat $@.log && false))
	$(call gb_Helper_abbreviate_dirs,( \
        ! $(call gb_Executable_get_command,unoidl-check) --ignore-unpublished \
            $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/psa.rdb \
            -- $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/pe.rdb) \
        > $@.log 2>&1 || (cat $@.log && false))
	$(call gb_Helper_abbreviate_dirs,( \
        ! $(call gb_Executable_get_command,unoidl-check) --ignore-unpublished \
            $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/psa.rdb \
            -- $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/ue.rdb) \
        > $@.log 2>&1 || (cat $@.log && false))
	$(call gb_Helper_abbreviate_dirs,( \
        ! $(call gb_Executable_get_command,unoidl-check) --ignore-unpublished \
            $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/psa.rdb \
            -- $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/other1.rdb) \
        > $@.log 2>&1 || (cat $@.log && false))
	$(call gb_Helper_abbreviate_dirs,( \
        ! $(call gb_Executable_get_command,unoidl-check) --ignore-unpublished \
            $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/psa.rdb \
            -- $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/other2.rdb) \
        > $@.log 2>&1 || (cat $@.log && false))
	$(call gb_Helper_abbreviate_dirs,( \
        $(call gb_Executable_get_command,unoidl-check) \
            $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/psa.rdb \
            -- $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/psa.rdb) \
        > $@.log 2>&1 || (cat $@.log && false))
	$(call gb_Helper_abbreviate_dirs,( \
        ! $(call gb_Executable_get_command,unoidl-check) \
            $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/psa.rdb \
            -- $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/psb.rdb) \
        > $@.log 2>&1 || (cat $@.log && false))
	$(call gb_Helper_abbreviate_dirs,( \
        ! $(call gb_Executable_get_command,unoidl-check) \
            $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/psa.rdb \
            -- $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/usa.rdb) \
        > $@.log 2>&1 || (cat $@.log && false))
	$(call gb_Helper_abbreviate_dirs,( \
        ! $(call gb_Executable_get_command,unoidl-check) \
            $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/psa.rdb \
            -- $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/usb.rdb) \
        > $@.log 2>&1 || (cat $@.log && false))
	$(call gb_Helper_abbreviate_dirs,( \
        ! $(call gb_Executable_get_command,unoidl-check) \
            $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/psa.rdb \
            -- $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/pe.rdb) \
        > $@.log 2>&1 || (cat $@.log && false))
	$(call gb_Helper_abbreviate_dirs,( \
        ! $(call gb_Executable_get_command,unoidl-check) \
            $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/psa.rdb \
            -- $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/ue.rdb) \
        > $@.log 2>&1 || (cat $@.log && false))
	$(call gb_Helper_abbreviate_dirs,( \
        ! $(call gb_Executable_get_command,unoidl-check) \
            $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/psa.rdb \
            -- $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/other1.rdb) \
        > $@.log 2>&1 || (cat $@.log && false))
	$(call gb_Helper_abbreviate_dirs,( \
        ! $(call gb_Executable_get_command,unoidl-check) \
            $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/psa.rdb \
            -- $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/other2.rdb) \
        > $@.log 2>&1 || (cat $@.log && false))
	$(call gb_Helper_abbreviate_dirs,( \
        $(call gb_Executable_get_command,unoidl-check) --ignore-unpublished \
            $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/usa.rdb \
            -- $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/psa.rdb) \
        > $@.log 2>&1 || (cat $@.log && false))
	$(call gb_Helper_abbreviate_dirs,( \
        $(call gb_Executable_get_command,unoidl-check) --ignore-unpublished \
            $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/usa.rdb \
            -- $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/psb.rdb) \
        > $@.log 2>&1 || (cat $@.log && false))
	$(call gb_Helper_abbreviate_dirs,( \
        $(call gb_Executable_get_command,unoidl-check) --ignore-unpublished \
            $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/usa.rdb \
            -- $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/usa.rdb) \
        > $@.log 2>&1 || (cat $@.log && false))
	$(call gb_Helper_abbreviate_dirs,( \
        $(call gb_Executable_get_command,unoidl-check) --ignore-unpublished \
            $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/usa.rdb \
            -- $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/usb.rdb) \
        > $@.log 2>&1 || (cat $@.log && false))
	$(call gb_Helper_abbreviate_dirs,( \
        $(call gb_Executable_get_command,unoidl-check) --ignore-unpublished \
            $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/usa.rdb \
            -- $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/pe.rdb) \
        > $@.log 2>&1 || (cat $@.log && false))
	$(call gb_Helper_abbreviate_dirs,( \
        $(call gb_Executable_get_command,unoidl-check) --ignore-unpublished \
            $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/usa.rdb \
            -- $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/ue.rdb) \
        > $@.log 2>&1 || (cat $@.log && false))
	$(call gb_Helper_abbreviate_dirs,( \
        $(call gb_Executable_get_command,unoidl-check) --ignore-unpublished \
            $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/usa.rdb \
            -- $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/other1.rdb) \
        > $@.log 2>&1 || (cat $@.log && false))
	$(call gb_Helper_abbreviate_dirs,( \
        $(call gb_Executable_get_command,unoidl-check) --ignore-unpublished \
            $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/usa.rdb \
            -- $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/other2.rdb) \
        > $@.log 2>&1 || (cat $@.log && false))
	$(call gb_Helper_abbreviate_dirs,( \
        $(call gb_Executable_get_command,unoidl-check) \
            $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/usa.rdb \
            -- $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/psa.rdb) \
        > $@.log 2>&1 || (cat $@.log && false))
	$(call gb_Helper_abbreviate_dirs,( \
        ! $(call gb_Executable_get_command,unoidl-check) \
            $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/usa.rdb \
            -- $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/psb.rdb) \
        > $@.log 2>&1 || (cat $@.log && false))
	$(call gb_Helper_abbreviate_dirs,( \
        $(call gb_Executable_get_command,unoidl-check) \
            $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/usa.rdb \
            -- $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/usa.rdb) \
        > $@.log 2>&1 || (cat $@.log && false))
	$(call gb_Helper_abbreviate_dirs,( \
        ! $(call gb_Executable_get_command,unoidl-check) \
            $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/usa.rdb \
            -- $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/usb.rdb) \
        > $@.log 2>&1 || (cat $@.log && false))
	$(call gb_Helper_abbreviate_dirs,( \
        ! $(call gb_Executable_get_command,unoidl-check) \
            $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/usa.rdb \
            -- $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/pe.rdb) \
        > $@.log 2>&1 || (cat $@.log && false))
	$(call gb_Helper_abbreviate_dirs,( \
        ! $(call gb_Executable_get_command,unoidl-check) \
            $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/usa.rdb \
            -- $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/ue.rdb) \
        > $@.log 2>&1 || (cat $@.log && false))
	$(call gb_Helper_abbreviate_dirs,( \
        ! $(call gb_Executable_get_command,unoidl-check) \
            $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/usa.rdb \
            -- $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/other1.rdb) \
        > $@.log 2>&1 || (cat $@.log && false))
	$(call gb_Helper_abbreviate_dirs,( \
        ! $(call gb_Executable_get_command,unoidl-check) \
            $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/usa.rdb \
            -- $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/other2.rdb) \
        > $@.log 2>&1 || (cat $@.log && false))
endif

$(call gb_CustomTarget_get_workdir,registry/regcompare_test)/%.rdb: \
        $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/%.urd \
        $(call gb_Executable_get_runtime_dependencies,regmerge)
	$(call gb_Helper_abbreviate_dirs,( \
        $(call gb_Executable_get_command,regmerge) $@ /UCR $<))

$(call gb_CustomTarget_get_workdir,registry/regcompare_test)/%.urd: \
        $(SRCDIR)/registry/test/regcompare/%.idl \
        $(call gb_Executable_get_runtime_dependencies,idlc)
	$(call gb_Helper_abbreviate_dirs,( \
        $(call gb_Executable_get_command,idlc) \
            -O$(call gb_CustomTarget_get_workdir,registry/regcompare_test) \
            -cid -we $<))

# vim: set noet sw=4 ts=4:
