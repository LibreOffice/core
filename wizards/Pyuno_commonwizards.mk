# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Pyuno_Pyuno,commonwizards,$(SRCDIR)/wizards/com/sun/star/wizards))

$(eval $(call gb_Pyuno_add_files,commonwizards,wizards,\
    __init__.py \
    common/strings.hrc \
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
    common/SystemDialog.py \
    common/IRenderer.py \
    common/UCB.py \
    common/XMLHelper.py \
    common/XMLProvider.py \
    common/ListModel.py \
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
    ui/event/SimpleDataAware.py \
    ui/event/Task.py \
    ui/event/TaskEvent.py \
    ui/event/TaskListener.py \
    ui/event/ListDataEvent.py \
    ui/event/ListDataListener.py \
    ui/event/EventListenerList.py \
    ui/event/__init__.py \
))

# vim:set noet sw=4 ts=4:
