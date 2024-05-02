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

$(eval $(call gb_CustomTarget_CustomTarget,jvmfwk/jvmfwk3_ini))

$(eval $(call gb_CustomTarget_register_targets,jvmfwk/jvmfwk3_ini, \
    $(call gb_Helper_get_rcfile,jvmfwk3) \
))

$(gb_CustomTarget_workdir)/jvmfwk/jvmfwk3_ini/$(call gb_Helper_get_rcfile,jvmfwk3): \
            $(SRCDIR)/jvmfwk/CustomTarget_jvmfwk_jvmfwk3_ini.mk
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ECH,1)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),ECH)
	(   printf '[Bootstrap]\n' && \
            printf 'UNO_JAVA_JFW_VENDOR_SETTINGS=$(if $(filter MACOSX,$(OS)),$${ORIGIN}/../share/misc/,$${ORIGIN}/)javavendors.xml\n' && \
            printf 'UNO_JAVA_JFW_SHARED_DATA=$${URE_OVERRIDE_JAVA_JFW_SHARED_DATA}\n' && \
            printf 'UNO_JAVA_JFW_USER_DATA=$${URE_OVERRIDE_JAVA_JFW_USER_DATA}\n' && \
            printf 'UNO_JAVA_JFW_CLASSPATH_URLS=$${URE_MORE_JAVA_CLASSPATH_URLS}\n' \
        ) > $@
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),ECH)

# vim: set noet sw=4 ts=4:
