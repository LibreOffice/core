# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,unoidl/unoidl-write_test))

# this target is phony to run it every time
.PHONY : $(call gb_CustomTarget_get_target,unoidl/unoidl-write_test)

$(call gb_CustomTarget_get_target,unoidl/unoidl-write_test) : \
        $(call gb_Executable_get_runtime_dependencies,unoidl-write) \
        $(SRCDIR)/solenv/bin/exectest.pl \
        $(SRCDIR)/idlc/test/parser/attribute.tests \
        $(SRCDIR)/idlc/test/parser/constant.tests \
        $(SRCDIR)/idlc/test/parser/constructor.tests \
        $(SRCDIR)/idlc/test/parser/conversion.tests \
        $(SRCDIR)/idlc/test/parser/interfaceinheritance.tests \
        $(SRCDIR)/idlc/test/parser/methodoverload.tests \
        $(SRCDIR)/idlc/test/parser/polystruct.tests \
        $(SRCDIR)/idlc/test/parser/published.tests \
        $(SRCDIR)/idlc/test/parser/struct.tests \
        $(SRCDIR)/idlc/test/parser/typedef.tests \
        | $(call gb_CustomTarget_get_workdir,unoidl/unoidl-write_test)/.dir
ifneq ($(gb_SUPPRESS_TESTS),)
	@true
else
	$(call gb_Helper_abbreviate_dirs,( \
        $(PERL) $(SRCDIR)/solenv/bin/exectest.pl \
            $(SRCDIR)/idlc/test/parser/attribute.tests \
            $(call gb_CustomTarget_get_workdir,unoidl/unoidl-write_test)/in.idl \
            1 $(call gb_Executable_get_command,unoidl-write) $(SRCDIR)/udkapi \
            {} \
            $(call gb_CustomTarget_get_workdir,unoidl/unoidl-write_test)/out.rdb \
        && $(PERL) $(SRCDIR)/solenv/bin/exectest.pl \
            $(SRCDIR)/idlc/test/parser/constant.tests \
            $(call gb_CustomTarget_get_workdir,unoidl/unoidl-write_test)/in.idl \
            1 $(call gb_Executable_get_command,unoidl-write) $(SRCDIR)/udkapi \
            {} \
            $(call gb_CustomTarget_get_workdir,unoidl/unoidl-write_test)/out.rdb \
        && $(PERL) $(SRCDIR)/solenv/bin/exectest.pl \
            $(SRCDIR)/idlc/test/parser/constructor.tests \
            $(call gb_CustomTarget_get_workdir,unoidl/unoidl-write_test)/in.idl \
            1 $(call gb_Executable_get_command,unoidl-write) $(SRCDIR)/udkapi \
            {} \
            $(call gb_CustomTarget_get_workdir,unoidl/unoidl-write_test)/out.rdb \
        && $(PERL) $(SRCDIR)/solenv/bin/exectest.pl \
            $(SRCDIR)/idlc/test/parser/conversion.tests \
            $(call gb_CustomTarget_get_workdir,unoidl/unoidl-write_test)/in.idl \
            1 $(call gb_Executable_get_command,unoidl-write) $(SRCDIR)/udkapi \
            {} \
            $(call gb_CustomTarget_get_workdir,unoidl/unoidl-write_test)/out.rdb \
        && $(PERL) $(SRCDIR)/solenv/bin/exectest.pl \
            $(SRCDIR)/idlc/test/parser/interfaceinheritance.tests \
            $(call gb_CustomTarget_get_workdir,unoidl/unoidl-write_test)/in.idl \
            1 $(call gb_Executable_get_command,unoidl-write) $(SRCDIR)/udkapi \
            {} \
            $(call gb_CustomTarget_get_workdir,unoidl/unoidl-write_test)/out.rdb \
        && $(PERL) $(SRCDIR)/solenv/bin/exectest.pl \
            $(SRCDIR)/idlc/test/parser/methodoverload.tests \
            $(call gb_CustomTarget_get_workdir,unoidl/unoidl-write_test)/in.idl \
            1 $(call gb_Executable_get_command,unoidl-write) $(SRCDIR)/udkapi \
            {} \
            $(call gb_CustomTarget_get_workdir,unoidl/unoidl-write_test)/out.rdb \
        && $(PERL) $(SRCDIR)/solenv/bin/exectest.pl \
            $(SRCDIR)/idlc/test/parser/oldstyle.tests \
            $(call gb_CustomTarget_get_workdir,unoidl/unoidl-write_test)/in.idl \
            1 $(call gb_Executable_get_command,unoidl-write) $(SRCDIR)/udkapi \
            {} \
            $(call gb_CustomTarget_get_workdir,unoidl/unoidl-write_test)/out.rdb \
        && $(PERL) $(SRCDIR)/solenv/bin/exectest.pl \
            $(SRCDIR)/idlc/test/parser/polystruct.tests \
            $(call gb_CustomTarget_get_workdir,unoidl/unoidl-write_test)/in.idl \
            1 $(call gb_Executable_get_command,unoidl-write) $(SRCDIR)/udkapi \
            {} \
            $(call gb_CustomTarget_get_workdir,unoidl/unoidl-write_test)/out.rdb \
        && $(PERL) $(SRCDIR)/solenv/bin/exectest.pl \
            $(SRCDIR)/idlc/test/parser/published.tests \
            $(call gb_CustomTarget_get_workdir,unoidl/unoidl-write_test)/in.idl \
            1 $(call gb_Executable_get_command,unoidl-write) $(SRCDIR)/udkapi \
            {} \
            $(call gb_CustomTarget_get_workdir,unoidl/unoidl-write_test)/out.rdb \
        && $(PERL) $(SRCDIR)/solenv/bin/exectest.pl \
            $(SRCDIR)/idlc/test/parser/struct.tests \
            $(call gb_CustomTarget_get_workdir,unoidl/unoidl-write_test)/in.idl \
            1 $(call gb_Executable_get_command,unoidl-write) $(SRCDIR)/udkapi \
            {} \
            $(call gb_CustomTarget_get_workdir,unoidl/unoidl-write_test)/out.rdb \
        && $(PERL) $(SRCDIR)/solenv/bin/exectest.pl \
            $(SRCDIR)/idlc/test/parser/typedef.tests \
            $(call gb_CustomTarget_get_workdir,unoidl/unoidl-write_test)/in.idl \
            1 $(call gb_Executable_get_command,unoidl-write) $(SRCDIR)/udkapi \
            {} \
            $(call gb_CustomTarget_get_workdir,unoidl/unoidl-write_test)/out.rdb) \
        > $@.log 2>&1 || (cat $@.log && false))
endif

# vim: set noet sw=4 ts=4:
