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
        $(call gb_Executable_get_runtime_dependencies,regcompare) \
        $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/other1.urd \
        $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/other2.urd \
        $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/pe.urd \
        $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/psa.urd \
        $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/psb.urd \
        $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/ue.urd \
        $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/usa.urd \
        $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/usb.urd
	$(call gb_Helper_abbreviate_dirs,( \
        $(call gb_Executable_get_command,regcompare) -f -t \
            -r1 $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/psa.urd \
            -r2 $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/psa.urd \
        && ! $(call gb_Executable_get_command,regcompare) -f -t \
            -r1 $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/psa.urd \
            -r2 $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/psb.urd \
        && ! $(call gb_Executable_get_command,regcompare) -f -t \
            -r1 $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/psa.urd \
            -r2 $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/usa.urd \
        && ! $(call gb_Executable_get_command,regcompare) -f -t \
            -r1 $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/psa.urd \
            -r2 $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/usb.urd \
        && ! $(call gb_Executable_get_command,regcompare) -f -t \
            -r1 $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/psa.urd \
            -r2 $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/pe.urd \
        && ! $(call gb_Executable_get_command,regcompare) -f -t \
            -r1 $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/psa.urd \
            -r2 $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/ue.urd \
        && ! $(call gb_Executable_get_command,regcompare) -f -t \
            -r1 $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/psa.urd \
            -r2 $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/other1.urd \
        && ! $(call gb_Executable_get_command,regcompare) -f -t \
            -r1 $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/psa.urd \
            -r2 $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/other2.urd \
        && $(call gb_Executable_get_command,regcompare) -f -t -u \
            -r1 $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/psa.urd \
            -r2 $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/psa.urd \
        && ! $(call gb_Executable_get_command,regcompare) -f -t -u \
            -r1 $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/psa.urd \
            -r2 $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/psb.urd \
        && ! $(call gb_Executable_get_command,regcompare) -f -t -u \
            -r1 $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/psa.urd \
            -r2 $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/usa.urd \
        && ! $(call gb_Executable_get_command,regcompare) -f -t -u \
            -r1 $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/psa.urd \
            -r2 $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/usb.urd \
        && ! $(call gb_Executable_get_command,regcompare) -f -t -u \
            -r1 $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/psa.urd \
            -r2 $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/pe.urd \
        && ! $(call gb_Executable_get_command,regcompare) -f -t -u \
            -r1 $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/psa.urd \
            -r2 $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/ue.urd \
        && ! $(call gb_Executable_get_command,regcompare) -f -t -u \
            -r1 $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/psa.urd \
            -r2 $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/other1.urd \
        && ! $(call gb_Executable_get_command,regcompare) -f -t -u \
            -r1 $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/psa.urd \
            -r2 $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/other2.urd \
        && $(call gb_Executable_get_command,regcompare) -f -t \
            -r1 $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/usa.urd \
            -r2 $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/psa.urd \
        && $(call gb_Executable_get_command,regcompare) -f -t \
            -r1 $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/usa.urd \
            -r2 $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/psb.urd \
        && $(call gb_Executable_get_command,regcompare) -f -t \
            -r1 $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/usa.urd \
            -r2 $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/usa.urd \
        && $(call gb_Executable_get_command,regcompare) -f -t \
            -r1 $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/usa.urd \
            -r2 $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/usb.urd \
        && $(call gb_Executable_get_command,regcompare) -f -t \
            -r1 $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/usa.urd \
            -r2 $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/pe.urd \
        && $(call gb_Executable_get_command,regcompare) -f -t \
            -r1 $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/usa.urd \
            -r2 $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/ue.urd \
        && $(call gb_Executable_get_command,regcompare) -f -t \
            -r1 $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/usa.urd \
            -r2 $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/other1.urd \
        && $(call gb_Executable_get_command,regcompare) -f -t \
            -r1 $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/usa.urd \
            -r2 $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/other2.urd \
        && $(call gb_Executable_get_command,regcompare) -f -t -u \
            -r1 $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/usa.urd \
            -r2 $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/psa.urd \
        && ! $(call gb_Executable_get_command,regcompare) -f -t -u \
            -r1 $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/usa.urd \
            -r2 $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/psb.urd \
        && $(call gb_Executable_get_command,regcompare) -f -t -u \
            -r1 $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/usa.urd \
            -r2 $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/usa.urd \
        && ! $(call gb_Executable_get_command,regcompare) -f -t -u \
            -r1 $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/usa.urd \
            -r2 $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/usb.urd \
        && ! $(call gb_Executable_get_command,regcompare) -f -t -u \
            -r1 $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/usa.urd \
            -r2 $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/pe.urd \
        && ! $(call gb_Executable_get_command,regcompare) -f -t -u \
            -r1 $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/usa.urd \
            -r2 $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/ue.urd \
        && ! $(call gb_Executable_get_command,regcompare) -f -t -u \
            -r1 $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/usa.urd \
            -r2 $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/other1.urd \
        && ! $(call gb_Executable_get_command,regcompare) -f -t -u \
            -r1 $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/usa.urd \
            -r2 $(call gb_CustomTarget_get_workdir,registry/regcompare_test)/other2.urd) \
        > $@.log 2>&1 || (cat $@.log && false))

$(call gb_CustomTarget_get_workdir,registry/regcompare_test)/%.urd: \
        $(SRCDIR)/registry/test/regcompare/%.idl \
        $(call gb_Executable_get_runtime_dependencies,idlc)
	$(call gb_Helper_abbreviate_dirs,( \
        $(call gb_Executable_get_command,idlc) \
            -O$(call gb_CustomTarget_get_workdir,registry/regcompare_test) \
            -cid -we $<))

# vim: set noet sw=4 ts=4:
