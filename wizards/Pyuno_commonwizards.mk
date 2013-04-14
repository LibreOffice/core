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
# Copyright (C) 2011 Red Hat, Inc., David Tardon <dtardon@redhat.com>
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

$(eval $(call gb_Pyuno_Pyuno,commonwizards,$(SRCDIR)/wizards/com/sun/star/wizards))

$(eval $(call gb_Pyuno_add_files,commonwizards,wizards,\
    __init__.py \
    common/ConfigGroup.py \
    common/ConfigSet.py \
    common/Configuration.py \
    common/Desktop.py \
    common/FileAccess.py \
    common/HelpIds.py \
    common/NoValidPathException.py \
    common/NumberFormatter.py \
    common/Properties.py \
    common/PropertyNames.py \
    common/Resource.py \
    common/SystemDialog.py \
    common/__init__.py \
    document/OfficeDocument.py \
    document/__init__.py \
    text/TextDocument.py \
    text/TextElement.py \
    text/TextFieldHandler.py \
    text/TextSectionHandler.py \
    text/__init__.py \
    ui/ControlScroller.py \
    ui/DocumentPreview.py \
    ui/ImageList.py \
    ui/PathSelection.py \
    ui/PeerConfig.py \
    ui/UIConsts.py \
    ui/UnoDialog.py \
    ui/UnoDialog2.py \
    ui/WizardDialog.py \
    ui/__init__.py \
    ui/event/CommonListener.py \
    ui/event/DataAware.py \
    ui/event/ListModelBinder.py \
    ui/event/RadioDataAware.py \
    ui/event/UnoDataAware.py \
    ui/event/__init__.py \
))

# vim:set noet sw=4 ts=4:
