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

#include <macros/generic.hxx>
#include <general.h>

namespace framework{

/** properties for "Frame" class */

#define FRAME_PROPNAME_ASCII_DISPATCHRECORDERSUPPLIER   "DispatchRecorderSupplier"
#define FRAME_PROPNAME_ASCII_ISHIDDEN                   "IsHidden"
#define FRAME_PROPNAME_ASCII_LAYOUTMANAGER              "LayoutManager"
#define FRAME_PROPNAME_ASCII_TITLE                      "Title"
#define FRAME_PROPNAME_ASCII_INDICATORINTERCEPTION      "IndicatorInterception"

#define FRAME_PROPNAME_DISPATCHRECORDERSUPPLIER         DECLARE_ASCII(FRAME_PROPNAME_ASCII_DISPATCHRECORDERSUPPLIER)
#define FRAME_PROPNAME_ISHIDDEN                         DECLARE_ASCII(FRAME_PROPNAME_ASCII_ISHIDDEN                )
#define FRAME_PROPNAME_LAYOUTMANAGER                    DECLARE_ASCII(FRAME_PROPNAME_ASCII_LAYOUTMANAGER           )
#define FRAME_PROPNAME_TITLE                            DECLARE_ASCII(FRAME_PROPNAME_ASCII_TITLE                   )
#define FRAME_PROPNAME_INDICATORINTERCEPTION            DECLARE_ASCII(FRAME_PROPNAME_ASCII_INDICATORINTERCEPTION   )

// Please add new entries alphabetical sorted and correct all other handles!
// Start counting with 0, so it can be used as direct index into an array too.
// Don't forget updating of define FRAME_PROPCOUNT below!!!
#define FRAME_PROPHANDLE_DISPATCHRECORDERSUPPLIER       0
#define FRAME_PROPHANDLE_ISHIDDEN                       1
#define FRAME_PROPHANDLE_LAYOUTMANAGER                  2
#define FRAME_PROPHANDLE_TITLE                          3
#define FRAME_PROPHANDLE_INDICATORINTERCEPTION          4

#define FRAME_PROPCOUNT                                 5

/** properties for "Desktop" class */

#define DESKTOP_PROPNAME_ASCII_ACTIVEFRAME              "ActiveFrame"
#define DESKTOP_PROPNAME_ASCII_DISPATCHRECORDERSUPPLIER "DispatchRecorderSupplier"
#define DESKTOP_PROPNAME_ASCII_ISPLUGGED                "IsPlugged"
#define DESKTOP_PROPNAME_ASCII_SUSPENDQUICKSTARTVETO    "SuspendQuickstartVeto"
#define DESKTOP_PROPNAME_ASCII_TITLE                    "Title"

#define DESKTOP_PROPNAME_ACTIVEFRAME                    DECLARE_ASCII(DESKTOP_PROPNAME_ASCII_ACTIVEFRAME             )
#define DESKTOP_PROPNAME_DISPATCHRECORDERSUPPLIER       DECLARE_ASCII(DESKTOP_PROPNAME_ASCII_DISPATCHRECORDERSUPPLIER)
#define DESKTOP_PROPNAME_ISPLUGGED                      DECLARE_ASCII(DESKTOP_PROPNAME_ASCII_ISPLUGGED               )
#define DESKTOP_PROPNAME_SUSPENDQUICKSTARTVETO          DECLARE_ASCII(DESKTOP_PROPNAME_ASCII_SUSPENDQUICKSTARTVETO   )
#define DESKTOP_PROPNAME_TITLE                          DECLARE_ASCII(DESKTOP_PROPNAME_ASCII_TITLE                   )

// Please add new entries alphabetical sorted and correct all other handles!
// Start counting with 0, so it can be used as direct index into an array too.
// Don't forget updating of define DESKTOP_PROPCOUNT below!!!
#define DESKTOP_PROPHANDLE_ACTIVEFRAME                  0
#define DESKTOP_PROPHANDLE_DISPATCHRECORDERSUPPLIER     1
#define DESKTOP_PROPHANDLE_ISPLUGGED                    2
#define DESKTOP_PROPHANDLE_SUSPENDQUICKSTARTVETO        3
#define DESKTOP_PROPHANDLE_TITLE                        4

#define DESKTOP_PROPCOUNT                               5

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

#define PATHSETTINGS_PROPNAME_ADDIN                     OUString(PATHSETTINGS_PROPNAME_ASCII_ADDIN)
#define PATHSETTINGS_PROPNAME_AUTOCORRECT               OUString(PATHSETTINGS_PROPNAME_ASCII_AUTOCORRECT)
#define PATHSETTINGS_PROPNAME_AUTOTEXT                  OUString(PATHSETTINGS_PROPNAME_ASCII_AUTOTEXT)
#define PATHSETTINGS_PROPNAME_BACKUP                    OUString(PATHSETTINGS_PROPNAME_ASCII_BACKUP)
#define PATHSETTINGS_PROPNAME_BASIC                     OUString(PATHSETTINGS_PROPNAME_ASCII_BASIC)
#define PATHSETTINGS_PROPNAME_BITMAP                    OUString(PATHSETTINGS_PROPNAME_ASCII_BITMAP)
#define PATHSETTINGS_PROPNAME_CONFIG                    OUString(PATHSETTINGS_PROPNAME_ASCII_CONFIG)
#define PATHSETTINGS_PROPNAME_DICTIONARY                OUString(PATHSETTINGS_PROPNAME_ASCII_DICTIONARY)
#define PATHSETTINGS_PROPNAME_FAVORITE                  OUString(PATHSETTINGS_PROPNAME_ASCII_FAVORITE)
#define PATHSETTINGS_PROPNAME_FILTER                    OUString(PATHSETTINGS_PROPNAME_ASCII_FILTER)
#define PATHSETTINGS_PROPNAME_GALLERY                   OUString(PATHSETTINGS_PROPNAME_ASCII_GALLERY)
#define PATHSETTINGS_PROPNAME_GRAPHIC                   OUString(PATHSETTINGS_PROPNAME_ASCII_GRAPHIC)
#define PATHSETTINGS_PROPNAME_HELP                      OUString(PATHSETTINGS_PROPNAME_ASCII_HELP)
#define PATHSETTINGS_PROPNAME_LINGUISTIC                OUString(PATHSETTINGS_PROPNAME_ASCII_LINGUISTIC)
#define PATHSETTINGS_PROPNAME_MODULE                    OUString(PATHSETTINGS_PROPNAME_ASCII_MODULE)
#define PATHSETTINGS_PROPNAME_PALETTE                   OUString(PATHSETTINGS_PROPNAME_ASCII_PALETTE)
#define PATHSETTINGS_PROPNAME_PLUGIN                    OUString(PATHSETTINGS_PROPNAME_ASCII_PLUGIN)
#define PATHSETTINGS_PROPNAME_STORAGE                   OUString(PATHSETTINGS_PROPNAME_ASCII_STORAGE)
#define PATHSETTINGS_PROPNAME_TEMP                      OUString(PATHSETTINGS_PROPNAME_ASCII_TEMP)
#define PATHSETTINGS_PROPNAME_TEMPLATE                  OUString(PATHSETTINGS_PROPNAME_ASCII_TEMPLATE)
#define PATHSETTINGS_PROPNAME_UICONFIG                  OUString(PATHSETTINGS_PROPNAME_ASCII_UICONFIG)
#define PATHSETTINGS_PROPNAME_USERCONFIG                OUString(PATHSETTINGS_PROPNAME_ASCII_USERCONFIG)
#define PATHSETTINGS_PROPNAME_USERDICTIONARY            OUString(PATHSETTINGS_PROPNAME_ASCII_USERDICTIONARY)
#define PATHSETTINGS_PROPNAME_WORK                      OUString(PATHSETTINGS_PROPNAME_ASCII_WORK)

// Please add new entries alphabetical sorted and correct all other handles!
// Start counting with 0, so it can be used as direct index into an array too.
// Don't forget updating of define PATHSETTINGS_PROPCOUNT below!!!
#define PATHSETTINGS_PROPHANDLE_ADDIN                    0
#define PATHSETTINGS_PROPHANDLE_AUTOCORRECT              1
#define PATHSETTINGS_PROPHANDLE_AUTOTEXT                 2
#define PATHSETTINGS_PROPHANDLE_BACKUP                   3
#define PATHSETTINGS_PROPHANDLE_BASIC                    4
#define PATHSETTINGS_PROPHANDLE_BITMAP                   5
#define PATHSETTINGS_PROPHANDLE_CONFIG                   6
#define PATHSETTINGS_PROPHANDLE_DICTIONARY               7
#define PATHSETTINGS_PROPHANDLE_FAVORITE                 8
#define PATHSETTINGS_PROPHANDLE_FILTER                   9
#define PATHSETTINGS_PROPHANDLE_GALLERY                 10
#define PATHSETTINGS_PROPHANDLE_GRAPHIC                 11
#define PATHSETTINGS_PROPHANDLE_HELP                    12
#define PATHSETTINGS_PROPHANDLE_LINGUISTIC              13
#define PATHSETTINGS_PROPHANDLE_MODULE                  14
#define PATHSETTINGS_PROPHANDLE_PALETTE                 15
#define PATHSETTINGS_PROPHANDLE_PLUGIN                  16
#define PATHSETTINGS_PROPHANDLE_STORAGE                 17
#define PATHSETTINGS_PROPHANDLE_TEMP                    18
#define PATHSETTINGS_PROPHANDLE_TEMPLATE                19
#define PATHSETTINGS_PROPHANDLE_UICONFIG                20
#define PATHSETTINGS_PROPHANDLE_USERCONFIG              21
#define PATHSETTINGS_PROPHANDLE_USERDICTIONARY          22
#define PATHSETTINGS_PROPHANDLE_WORK                    23

#define PATHSETTINGS_PROPCOUNT                          24

/** properties for "LayoutManager" class */

#define LAYOUTMANAGER_PROPNAME_ASCII_MENUBARCLOSER          "MenuBarCloser"
#define LAYOUTMANAGER_PROPNAME_ASCII_AUTOMATICTOOLBARS      "AutomaticToolbars"
#define LAYOUTMANAGER_PROPNAME_ASCII_REFRESHVISIBILITY      "RefreshContextToolbarVisibility"
#define LAYOUTMANAGER_PROPNAME_ASCII_HIDECURRENTUI          "HideCurrentUI"
#define LAYOUTMANAGER_PROPNAME_ASCII_LOCKCOUNT              "LockCount"
#define LAYOUTMANAGER_PROPNAME_ASCII_PRESERVE_CONTENT_SIZE  "PreserveContentSize"

#define LAYOUTMANAGER_PROPNAME_MENUBARCLOSER            OUString( LAYOUTMANAGER_PROPNAME_ASCII_MENUBARCLOSER )
#define LAYOUTMANAGER_PROPNAME_AUTOMATICTOOLBARS        OUString( LAYOUTMANAGER_PROPNAME_ASCII_AUTOMATICTOOLBARS )
#define LAYOUTMANAGER_PROPNAME_REFRESHVISIBILITY        OUString( LAYOUTMANAGER_PROPNAME_ASCII_REFRESHVISIBILITY )
#define LAYOUTMANAGER_PROPNAME_HIDECURRENTUI            OUString( LAYOUTMANAGER_PROPNAME_ASCII_HIDECURRENTUI )
#define LAYOUTMANAGER_PROPNAME_LOCKCOUNT                OUString( LAYOUTMANAGER_PROPNAME_ASCII_LOCKCOUNT )
#define LAYOUTMANAGER_PROPNAME_PRESERVE_CONTENT_SIZE    OUString( LAYOUTMANAGER_PROPNAME_ASCII_PRESERVE_CONTENT_SIZE )

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

#define UICOMMANDDESCRIPTION_PROPNAME_PROPERTIES                "Properties"
#define UICOMMANDDESCRIPTION_PROPNAME_POPUP                     "Popup"

#define UICOMMANDDESCRIPTION_PROPERTIES_IMAGE                   1
#define UICOMMANDDESCRIPTION_PROPERTIES_IMAGE_MIRRORED          2
#define UICOMMANDDESCRIPTION_PROPERTIES_IMAGE_ROTATED           4
#define UICOMMANDDESCRIPTION_PROPERTIES_TOGGLEBUTTON            8

/** properties for "AutoRecovery" class */

#define AUTORECOVERY_PROPNAME_ASCII_EXISTS_RECOVERYDATA "ExistsRecoveryData"
#define AUTORECOVERY_PROPNAME_ASCII_EXISTS_SESSIONDATA  "ExistsSessionData"
#define AUTORECOVERY_PROPNAME_ASCII_CRASHED             "Crashed"

#define AUTORECOVERY_PROPNAME_EXISTS_RECOVERYDATA       OUString( AUTORECOVERY_PROPNAME_ASCII_EXISTS_RECOVERYDATA )
#define AUTORECOVERY_PROPNAME_EXISTS_SESSIONDATA        OUString( AUTORECOVERY_PROPNAME_ASCII_EXISTS_SESSIONDATA )
#define AUTORECOVERY_PROPNAME_CRASHED                   OUString( AUTORECOVERY_PROPNAME_ASCII_CRASHED )

#define AUTORECOVERY_PROPHANDLE_EXISTS_RECOVERYDATA     0
#define AUTORECOVERY_PROPHANDLE_EXISTS_SESSIONDATA      1
#define AUTORECOVERY_PROPHANDLE_CRASHED                 2

#define AUTORECOVERY_PROPCOUNT                          3

/** properties for Filter config */

#define FILTER_PROPNAME_ASCII_DOCUMENTSERVICE           "DocumentService"

#define FILTER_PROPNAME_DOCUMENTSERVICE                 OUString( FILTER_PROPNAME_ASCII_DOCUMENTSERVICE )

/** properties for office module config (Setup.xcu) */

#define OFFICEFACTORY_PROPNAME_ASCII_WINDOWATTRIBUTES   "ooSetupFactoryWindowAttributes"
#define OFFICEFACTORY_PROPNAME_ASCII_UINAME             "ooSetupFactoryUIName"
#define OFFICEFACTORY_PROPNAME_ASCII_ICON               "ooSetupFactoryIcon"

#define OFFICEFACTORY_PROPNAME_WINDOWATTRIBUTES         OUString( OFFICEFACTORY_PROPNAME_ASCII_WINDOWATTRIBUTES )
#define OFFICEFACTORY_PROPNAME_UINAME                   OUString( OFFICEFACTORY_PROPNAME_ASCII_UINAME )
#define OFFICEFACTORY_PROPNAME_ICON                     OUString( OFFICEFACTORY_PROPNAME_ASCII_ICON )

/** properties for tab window (old) */
#define TABWINDOW_PROPNAME_ASCII_PARENTWINDOW           "ParentWindow"
#define TABWINDOW_PROPNAME_ASCII_TOPWINDOW              "TopWindow"
#define TABWINDOW_PROPNAME_PARENTWINDOW                 OUString( TABWINDOW_PROPNAME_ASCII_PARENTWINDOW )
#define TABWINDOW_PROPNAME_TOPWINDOW                    OUString( TABWINDOW_PROPNAME_ASCII_TOPWINDOW )

#define TABWINDOW_PROPHANDLE_PARENTWINDOW               0
#define TABWINDOW_PROPHANDLE_TOPWINDOW                  1
#define TABWINDOW_PROPCOUNT                             2

#define TABWINDOWSERVICE_PROPHANDLE_WINDOW              0
#define TABWINDOWSERVICE_PROPCOUNT                      1

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

    static sal_Bool willPropertyBeChanged( const css::uno::Any& aCurrentValue ,
                                           const css::uno::Any& aNewValue     ,
                                                 css::uno::Any& aOldValue     ,
                                                 css::uno::Any& aChangedValue )
    {
        sal_Bool bChanged = sal_False;

        // clear return parameter to be shure, to put out only valid values ...
        aOldValue.clear();
        aChangedValue.clear();

        // if value change ...
        bChanged = !(aCurrentValue==aNewValue);
        if (bChanged)
        {
            // ... set information of change.
            aOldValue     = aCurrentValue;
            aChangedValue = aNewValue    ;
        }

        return bChanged;
    }
};

} // namespace framework

#endif // INCLUDED_FRAMEWORK_INC_PROPERTIES_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
