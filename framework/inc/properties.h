/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_FRAMEWORK_INC_PROPERTIES_H
#define INCLUDED_FRAMEWORK_INC_PROPERTIES_H

#include "general.h"

namespace framework{

/** properties for "Frame" class */

#define FRAME_PROPNAME_ASCII_DISPATCHRECORDERSUPPLIER   "DispatchRecorderSupplier"
#define FRAME_PROPNAME_ASCII_ISHIDDEN                   "IsHidden"
#define FRAME_PROPNAME_ASCII_LAYOUTMANAGER              "LayoutManager"
#define FRAME_PROPNAME_ASCII_TITLE                      "Title"
#define FRAME_PROPNAME_ASCII_INDICATORINTERCEPTION      "IndicatorInterception"

// Please add new entries alphabetical sorted and correct all other handles!
// Start counting with 0, so it can be used as direct index into an array too.
// Don't forget updating of define FRAME_PROPCOUNT below!!!
#define FRAME_PROPHANDLE_DISPATCHRECORDERSUPPLIER       0
#define FRAME_PROPHANDLE_ISHIDDEN                       1
#define FRAME_PROPHANDLE_LAYOUTMANAGER                  2
#define FRAME_PROPHANDLE_TITLE                          3
#define FRAME_PROPHANDLE_INDICATORINTERCEPTION          4

#define FRAME_PROPCOUNT                                 5

/** properties for "PathSettings" class */

#define PATHSETTINGS_PROPNAME_ASCII_ADDIN               "Addin"
#define PATHSETTINGS_PROPNAME_ASCII_AUTOCORRECT         "AutoCorrect"
#define PATHSETTINGS_PROPNAME_ASCII_AUTOTEXT            "AutoText"
#define PATHSETTINGS_PROPNAME_ASCII_BACKUP              "Backup"
#define PATHSETTINGS_PROPNAME_ASCII_BASIC               "Basic"
#define PATHSETTINGS_PROPNAME_ASCII_BITMAP              "Bitmap"
#define PATHSETTINGS_PROPNAME_ASCII_CONFIG              "Config"
#define PATHSETTINGS_PROPNAME_ASCII_DICTIONARY          "Dictionary"
#define PATHSETTINGS_PROPNAME_ASCII_FAVORITE            "Favorite"
#define PATHSETTINGS_PROPNAME_ASCII_FILTER              "Filter"
#define PATHSETTINGS_PROPNAME_ASCII_GALLERY             "Gallery"
#define PATHSETTINGS_PROPNAME_ASCII_GRAPHIC             "Graphic"
#define PATHSETTINGS_PROPNAME_ASCII_HELP                "Help"
#define PATHSETTINGS_PROPNAME_ASCII_LINGUISTIC          "Linguistic"
#define PATHSETTINGS_PROPNAME_ASCII_MODULE              "Module"
#define PATHSETTINGS_PROPNAME_ASCII_PALETTE             "Palette"
#define PATHSETTINGS_PROPNAME_ASCII_PLUGIN              "Plugin"
#define PATHSETTINGS_PROPNAME_ASCII_STORAGE             "Storage"
#define PATHSETTINGS_PROPNAME_ASCII_TEMP                "Temp"
#define PATHSETTINGS_PROPNAME_ASCII_TEMPLATE            "Template"
#define PATHSETTINGS_PROPNAME_ASCII_UICONFIG            "UIConfig"
#define PATHSETTINGS_PROPNAME_ASCII_USERCONFIG          "UserConfig"
#define PATHSETTINGS_PROPNAME_ASCII_USERDICTIONARY      "UserDictionary"
#define PATHSETTINGS_PROPNAME_ASCII_WORK                "Work"


/** properties for "LayoutManager" class */

#define LAYOUTMANAGER_PROPNAME_ASCII_MENUBARCLOSER          "MenuBarCloser"
#define LAYOUTMANAGER_PROPNAME_ASCII_AUTOMATICTOOLBARS      "AutomaticToolbars"
#define LAYOUTMANAGER_PROPNAME_ASCII_REFRESHVISIBILITY      "RefreshContextToolbarVisibility"
#define LAYOUTMANAGER_PROPNAME_ASCII_HIDECURRENTUI          "HideCurrentUI"
#define LAYOUTMANAGER_PROPNAME_ASCII_LOCKCOUNT              "LockCount"
#define LAYOUTMANAGER_PROPNAME_ASCII_PRESERVE_CONTENT_SIZE  "PreserveContentSize"

#define LAYOUTMANAGER_PROPNAME_MENUBARCLOSER            LAYOUTMANAGER_PROPNAME_ASCII_MENUBARCLOSER

#define LAYOUTMANAGER_PROPHANDLE_MENUBARCLOSER          0
#define LAYOUTMANAGER_PROPHANDLE_AUTOMATICTOOLBARS      1
#define LAYOUTMANAGER_PROPHANDLE_REFRESHVISIBILITY      2
#define LAYOUTMANAGER_PROPHANDLE_HIDECURRENTUI          3
#define LAYOUTMANAGER_PROPHANDLE_LOCKCOUNT              4
#define LAYOUTMANAGER_PROPHANDLE_PRESERVE_CONTENT_SIZE  5

/** properties for "UICommandDescription" class */
#define UICOMMANDDESCRIPTION_NAMEACCESS_COMMANDIMAGELIST        "private:resource/image/commandimagelist"
#define UICOMMANDDESCRIPTION_NAMEACCESS_COMMANDROTATEIMAGELIST  "private:resource/image/commandrotateimagelist"
#define UICOMMANDDESCRIPTION_NAMEACCESS_COMMANDMIRRORIMAGELIST  "private:resource/image/commandmirrorimagelist"


#define UICOMMANDDESCRIPTION_PROPERTIES_TOGGLEBUTTON            8

/** properties for "AutoRecovery" class */

#define AUTORECOVERY_PROPNAME_ASCII_EXISTS_RECOVERYDATA "ExistsRecoveryData"
#define AUTORECOVERY_PROPNAME_ASCII_EXISTS_SESSIONDATA  "ExistsSessionData"
#define AUTORECOVERY_PROPNAME_ASCII_CRASHED             "Crashed"

#define AUTORECOVERY_PROPNAME_EXISTS_RECOVERYDATA       AUTORECOVERY_PROPNAME_ASCII_EXISTS_RECOVERYDATA
#define AUTORECOVERY_PROPNAME_EXISTS_SESSIONDATA        AUTORECOVERY_PROPNAME_ASCII_EXISTS_SESSIONDATA
#define AUTORECOVERY_PROPNAME_CRASHED                   AUTORECOVERY_PROPNAME_ASCII_CRASHED

#define AUTORECOVERY_PROPHANDLE_EXISTS_RECOVERYDATA     0
#define AUTORECOVERY_PROPHANDLE_EXISTS_SESSIONDATA      1
#define AUTORECOVERY_PROPHANDLE_CRASHED                 2

#define AUTORECOVERY_PROPCOUNT                          3

/** properties for Filter config */

#define FILTER_PROPNAME_ASCII_DOCUMENTSERVICE           "DocumentService"

/** properties for office module config (Setup.xcu) */

#define OFFICEFACTORY_PROPNAME_ASCII_WINDOWATTRIBUTES   "ooSetupFactoryWindowAttributes"
#define OFFICEFACTORY_PROPNAME_ASCII_UINAME             "ooSetupFactoryUIName"
#define OFFICEFACTORY_PROPNAME_ASCII_ICON               "ooSetupFactoryIcon"

/** properties for tab window (old) */
#define TABWINDOW_PROPNAME_ASCII_PARENTWINDOW           "ParentWindow"
#define TABWINDOW_PROPNAME_ASCII_TOPWINDOW              "TopWindow"
#define TABWINDOW_PROPNAME_PARENTWINDOW                 TABWINDOW_PROPNAME_ASCII_PARENTWINDOW
#define TABWINDOW_PROPNAME_TOPWINDOW                    TABWINDOW_PROPNAME_ASCII_TOPWINDOW

#define TABWINDOW_PROPHANDLE_PARENTWINDOW               0
#define TABWINDOW_PROPHANDLE_TOPWINDOW                  1
#define TABWINDOW_PROPCOUNT                             2

#define TABWINDOWSERVICE_PROPHANDLE_WINDOW              0

/** provides some helper methods to implement property sets. */

class PropHelper
{
    public:

    /** checks if given property will be changed by this settings.
     *  We compare the content of the given any values. If they are different we return sal_True - sal_False otherwise.
     *
     *  @param  aCurrentValue   contains the current value for this property
     *  @param  aNewValue       contains the new value for this property
     *  @param  aOldValue       returns the current value, in case something will be changed
     *  @param  aChangedValue   returns the new value, in case something will be changed
     *
     *  @return <True/> if value of this property will be changed; <False/> otherwise.
     */

    static bool willPropertyBeChanged( const css::uno::Any& aCurrentValue ,
                                           const css::uno::Any& aNewValue     ,
                                                 css::uno::Any& aOldValue     ,
                                                 css::uno::Any& aChangedValue )
    {
        bool bChanged = false;

        // clear return parameter to be sure, to put out only valid values ...
        aOldValue.clear();
        aChangedValue.clear();

        // if value change ...
        bChanged = aCurrentValue != aNewValue;
        if (bChanged)
        {
            // ... set information of change.
            aOldValue     = aCurrentValue;
            aChangedValue = aNewValue;
        }

        return bChanged;
    }
};

} // namespace framework

#endif // INCLUDED_FRAMEWORK_INC_PROPERTIES_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
