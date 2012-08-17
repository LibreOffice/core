# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
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
# Copyright (C) 2010 Red Hat, Inc., David Tardon <dtardon@redhat.com>
#  (initial developer)
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

$(eval $(call gb_Jar_Jar,java_uno_accessbridge))

$(eval $(call gb_Jar_use_jars,java_uno_accessbridge,\
    java_accessibility \
    jurt \
    ridl \
    unoil \
))

$(eval $(call gb_Jar_set_packageroot,java_uno_accessbridge,org))

$(eval $(call gb_Jar_set_manifest,java_uno_accessbridge,$(SRCDIR)/accessibility/bridge/org/openoffice/accessibility/manifest))

$(eval $(call gb_Jar_set_componentfile,java_uno_accessbridge,accessibility/bridge/org/openoffice/accessibility/java_uno_accessbridge,OOO))

$(eval $(call gb_Jar_add_sourcefiles,java_uno_accessbridge,\
    accessibility/bridge/org/openoffice/accessibility/AccessBridge \
    accessibility/bridge/org/openoffice/accessibility/KeyHandler \
    accessibility/bridge/org/openoffice/accessibility/PopupWindow \
    accessibility/bridge/org/openoffice/accessibility/WindowsAccessBridgeAdapter \
))

# vim:set shiftwidth=4 softtabstop=4 expandtab:
