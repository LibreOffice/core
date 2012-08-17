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

$(eval $(call gb_Jar_Jar,java_accessibility))

$(eval $(call gb_Jar_use_jars,java_accessibility,\
    jurt \
    ridl \
    unoil \
))

$(eval $(call gb_Jar_set_packageroot,java_accessibility,org))

$(eval $(call gb_Jar_add_sourcefiles,java_accessibility,\
    accessibility/bridge/org/openoffice/java/accessibility/AbstractButton \
    accessibility/bridge/org/openoffice/java/accessibility/AccessibleActionImpl \
    accessibility/bridge/org/openoffice/java/accessibility/AccessibleComponentImpl \
    accessibility/bridge/org/openoffice/java/accessibility/AccessibleEditableTextImpl \
    accessibility/bridge/org/openoffice/java/accessibility/AccessibleExtendedState \
    accessibility/bridge/org/openoffice/java/accessibility/AccessibleHypertextImpl \
    accessibility/bridge/org/openoffice/java/accessibility/AccessibleIconImpl \
    accessibility/bridge/org/openoffice/java/accessibility/AccessibleKeyBinding \
    accessibility/bridge/org/openoffice/java/accessibility/AccessibleObjectFactory \
    accessibility/bridge/org/openoffice/java/accessibility/AccessibleRoleAdapter \
    accessibility/bridge/org/openoffice/java/accessibility/AccessibleSelectionImpl \
    accessibility/bridge/org/openoffice/java/accessibility/AccessibleStateAdapter \
    accessibility/bridge/org/openoffice/java/accessibility/AccessibleTextImpl \
    accessibility/bridge/org/openoffice/java/accessibility/AccessibleValueImpl \
    accessibility/bridge/org/openoffice/java/accessibility/Alert \
    accessibility/bridge/org/openoffice/java/accessibility/Application \
    accessibility/bridge/org/openoffice/java/accessibility/Button \
    accessibility/bridge/org/openoffice/java/accessibility/CheckBox \
    accessibility/bridge/org/openoffice/java/accessibility/ComboBox \
    accessibility/bridge/org/openoffice/java/accessibility/Component \
    accessibility/bridge/org/openoffice/java/accessibility/Container \
    accessibility/bridge/org/openoffice/java/accessibility/DescendantManager \
    accessibility/bridge/org/openoffice/java/accessibility/Dialog \
    accessibility/bridge/org/openoffice/java/accessibility/FocusTraversalPolicy \
    accessibility/bridge/org/openoffice/java/accessibility/Frame \
    accessibility/bridge/org/openoffice/java/accessibility/Icon \
    accessibility/bridge/org/openoffice/java/accessibility/Label \
    accessibility/bridge/org/openoffice/java/accessibility/List \
    accessibility/bridge/org/openoffice/java/accessibility/Menu \
    accessibility/bridge/org/openoffice/java/accessibility/MenuContainer \
    accessibility/bridge/org/openoffice/java/accessibility/MenuItem \
    accessibility/bridge/org/openoffice/java/accessibility/NativeFrame \
    accessibility/bridge/org/openoffice/java/accessibility/Paragraph \
    accessibility/bridge/org/openoffice/java/accessibility/RadioButton \
    accessibility/bridge/org/openoffice/java/accessibility/ScrollBar \
    accessibility/bridge/org/openoffice/java/accessibility/Separator \
    accessibility/bridge/org/openoffice/java/accessibility/Table \
    accessibility/bridge/org/openoffice/java/accessibility/TextComponent \
    accessibility/bridge/org/openoffice/java/accessibility/ToggleButton \
    accessibility/bridge/org/openoffice/java/accessibility/ToolTip \
    accessibility/bridge/org/openoffice/java/accessibility/Tree \
    accessibility/bridge/org/openoffice/java/accessibility/Window \
    accessibility/bridge/org/openoffice/java/accessibility/logging/XAccessibleEventLog \
    accessibility/bridge/org/openoffice/java/accessibility/logging/XAccessibleHypertextLog \
    accessibility/bridge/org/openoffice/java/accessibility/logging/XAccessibleTextLog \
))

$(eval $(call gb_Jar_add_generated_sourcefiles,java_accessibility,\
    CustomTarget/accessibility/bridge/org/openoffice/java/accessibility/Build \
))

# vim:set shiftwidth=4 softtabstop=4 expandtab:
