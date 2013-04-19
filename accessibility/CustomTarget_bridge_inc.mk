# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,accessibility/bridge/inc))

$(call gb_CustomTarget_get_target,accessibility/bridge/inc) : \
    $(call gb_CustomTarget_get_workdir,accessibility/bridge/inc)/WindowsAccessBridgeAdapter.h

$(call gb_CustomTarget_get_workdir,accessibility/bridge/inc)/WindowsAccessBridgeAdapter.h :| \
        $(call gb_CustomTarget_get_workdir,accessibility/bridge/inc)/.dir \
        $(call gb_Jar_get_target,java_uno_accessbridge)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),JVH,1)
	cd $(call gb_JavaClassSet_get_classdir,$(call gb_Jar_get_classsetname,java_uno_accessbridge)) && \
    javah -classpath "$(OUTDIR)/bin/ridl.jar$(gb_CLASSPATHSEP)$(OUTDIR)/bin/unoil.jar$(gb_CLASSPATHSEP)." -o $@ org.openoffice.accessibility.WindowsAccessBridgeAdapter

# vim: set ts=4 sw=4 et:
