# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,idlc/parser_test))

# this target is phony to run it every time
.PHONY : $(call gb_CustomTarget_get_target,idlc/parser_test)

$(call gb_CustomTarget_get_target,idlc/parser_test) : \
            $(call gb_Executable_get_runtime_dependencies,idlc) \
            $(SRCDIR)/solenv/bin/exectest.pl \
            $(SRCDIR)/idlc/test/parser/attribute.tests \
            $(SRCDIR)/idlc/test/parser/constant.tests \
            $(SRCDIR)/idlc/test/parser/constructor.tests \
            $(SRCDIR)/idlc/test/parser/interfaceinheritance.tests \
            $(SRCDIR)/idlc/test/parser/methodoverload.tests \
            $(SRCDIR)/idlc/test/parser/polystruct.tests \
            $(SRCDIR)/idlc/test/parser/published.tests \
            $(SRCDIR)/idlc/test/parser/struct.tests \
            $(SRCDIR)/idlc/test/parser/typedef.tests \
            | $(call gb_CustomTarget_get_workdir,idlc/parser_test)/.dir
	$(call gb_Helper_abbreviate_dirs,( \
            $(PERL) $(SRCDIR)/solenv/bin/exectest.pl \
                $(SRCDIR)/idlc/test/parser/attribute.tests \
                $(call gb_CustomTarget_get_workdir,idlc/parser_test)/in.idl \
                $(call gb_Executable_get_command,idlc) \
                -O $(call gb_CustomTarget_get_workdir,idlc/parser_test) {} && \
            $(PERL) $(SRCDIR)/solenv/bin/exectest.pl \
                $(SRCDIR)/idlc/test/parser/constant.tests \
                $(call gb_CustomTarget_get_workdir,idlc/parser_test)/in.idl \
                $(call gb_Executable_get_command,idlc) \
                -O $(call gb_CustomTarget_get_workdir,idlc/parser_test) {} && \
            $(PERL) $(SRCDIR)/solenv/bin/exectest.pl \
                $(SRCDIR)/idlc/test/parser/constructor.tests \
                $(call gb_CustomTarget_get_workdir,idlc/parser_test)/in.idl \
                $(call gb_Executable_get_command,idlc) \
                -O $(call gb_CustomTarget_get_workdir,idlc/parser_test) {} && \
            $(PERL) $(SRCDIR)/solenv/bin/exectest.pl \
                $(SRCDIR)/idlc/test/parser/interfaceinheritance.tests \
                $(call gb_CustomTarget_get_workdir,idlc/parser_test)/in.idl \
                $(call gb_Executable_get_command,idlc) \
                -O $(call gb_CustomTarget_get_workdir,idlc/parser_test) {} && \
            $(PERL) $(SRCDIR)/solenv/bin/exectest.pl \
                $(SRCDIR)/idlc/test/parser/methodoverload.tests \
                $(call gb_CustomTarget_get_workdir,idlc/parser_test)/in.idl \
                $(call gb_Executable_get_command,idlc) \
                -O $(call gb_CustomTarget_get_workdir,idlc/parser_test) {} && \
            $(PERL) $(SRCDIR)/solenv/bin/exectest.pl \
                $(SRCDIR)/idlc/test/parser/oldstyle.tests \
                $(call gb_CustomTarget_get_workdir,idlc/parser_test)/in.idl \
                $(call gb_Executable_get_command,idlc) \
                -O $(call gb_CustomTarget_get_workdir,idlc/parser_test) {} && \
            $(PERL) $(SRCDIR)/solenv/bin/exectest.pl \
                $(SRCDIR)/idlc/test/parser/polystruct.tests \
                $(call gb_CustomTarget_get_workdir,idlc/parser_test)/in.idl \
                $(call gb_Executable_get_command,idlc) \
                -O $(call gb_CustomTarget_get_workdir,idlc/parser_test) {} && \
            $(PERL) $(SRCDIR)/solenv/bin/exectest.pl \
                $(SRCDIR)/idlc/test/parser/published.tests \
                $(call gb_CustomTarget_get_workdir,idlc/parser_test)/in.idl \
                $(call gb_Executable_get_command,idlc) \
                -O $(call gb_CustomTarget_get_workdir,idlc/parser_test) {} && \
            $(PERL) $(SRCDIR)/solenv/bin/exectest.pl \
                $(SRCDIR)/idlc/test/parser/struct.tests \
                $(call gb_CustomTarget_get_workdir,idlc/parser_test)/in.idl \
                $(call gb_Executable_get_command,idlc) \
                -O $(call gb_CustomTarget_get_workdir,idlc/parser_test) {} && \
            $(PERL) $(SRCDIR)/solenv/bin/exectest.pl \
                $(SRCDIR)/idlc/test/parser/typedef.tests \
                $(call gb_CustomTarget_get_workdir,idlc/parser_test)/in.idl \
                $(call gb_Executable_get_command,idlc) \
                -O $(call gb_CustomTarget_get_workdir,idlc/parser_test) {}) \
            > ${}.log 2>&1 || (cat ${}.log && false))

# vim: set noet sw=4 ts=4:
