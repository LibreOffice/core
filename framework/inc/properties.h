/*************************************************************************
 *
 *  $RCSfile: properties.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 18:19:28 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef __FRAMEWORK_PROPERTIES_H_
#define __FRAMEWORK_PROPERTIES_H_

//_______________________________________________
// includes

#ifndef __FRAMEWORK_MACROS_GENERIC_HXX_
#include <macros/generic.hxx>
#endif

//_______________________________________________
//  namespace

namespace framework{

//_______________________________________________
/** properties for "Frame" class */

#define FRAME_PROPNAME_ASCII_DISPATCHRECORDERSUPPLIER   "DispatchRecorderSupplier"
#define FRAME_PROPNAME_ASCII_ISBACKINGMODE              "IsBackingMode"
#define FRAME_PROPNAME_ASCII_ISHIDDEN                   "IsHidden"
#define FRAME_PROPNAME_ASCII_TITLE                      "Title"

#define FRAME_PROPNAME_DISPATCHRECORDERSUPPLIER         DECLARE_ASCII(FRAME_PROPNAME_ASCII_DISPATCHRECORDERSUPPLIER)
#define FRAME_PROPNAME_ISBACKINGMODE                    DECLARE_ASCII(FRAME_PROPNAME_ASCII_ISBACKINGMODE           )
#define FRAME_PROPNAME_ISHIDDEN                         DECLARE_ASCII(FRAME_PROPNAME_ASCII_ISHIDDEN                )
#define FRAME_PROPNAME_TITLE                            DECLARE_ASCII(FRAME_PROPNAME_ASCII_TITLE                   )

// Please add new entries alphabetical sorted and correct all other handles!
// Start counting with 0, so it can be used as direct index into an array too.
// Don't forget updating of define FRAME_PROPCOUNT below!!!
#define FRAME_PROPHANDLE_DISPATCHRECORDERSUPPLIER       0
#define FRAME_PROPHANDLE_ISBACKINGMODE                  1
#define FRAME_PROPHANDLE_ISHIDDEN                       2
#define FRAME_PROPHANDLE_TITLE                          3

#define FRAME_PROPCOUNT                                 4

//_______________________________________________
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

//_______________________________________________
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

#define PATHSETTINGS_PROPNAME_ADDIN                     ::rtl::OUString::createFromAscii(PATHSETTINGS_PROPNAME_ASCII_ADDIN         )
#define PATHSETTINGS_PROPNAME_AUTOCORRECT               ::rtl::OUString::createFromAscii(PATHSETTINGS_PROPNAME_ASCII_AUTOCORRECT   )
#define PATHSETTINGS_PROPNAME_AUTOTEXT                  ::rtl::OUString::createFromAscii(PATHSETTINGS_PROPNAME_ASCII_AUTOTEXT      )
#define PATHSETTINGS_PROPNAME_BACKUP                    ::rtl::OUString::createFromAscii(PATHSETTINGS_PROPNAME_ASCII_BACKUP        )
#define PATHSETTINGS_PROPNAME_BASIC                     ::rtl::OUString::createFromAscii(PATHSETTINGS_PROPNAME_ASCII_BASIC         )
#define PATHSETTINGS_PROPNAME_BITMAP                    ::rtl::OUString::createFromAscii(PATHSETTINGS_PROPNAME_ASCII_BITMAP        )
#define PATHSETTINGS_PROPNAME_CONFIG                    ::rtl::OUString::createFromAscii(PATHSETTINGS_PROPNAME_ASCII_CONFIG        )
#define PATHSETTINGS_PROPNAME_DICTIONARY                ::rtl::OUString::createFromAscii(PATHSETTINGS_PROPNAME_ASCII_DICTIONARY    )
#define PATHSETTINGS_PROPNAME_FAVORITE                  ::rtl::OUString::createFromAscii(PATHSETTINGS_PROPNAME_ASCII_FAVORITE      )
#define PATHSETTINGS_PROPNAME_FILTER                    ::rtl::OUString::createFromAscii(PATHSETTINGS_PROPNAME_ASCII_FILTER        )
#define PATHSETTINGS_PROPNAME_GALLERY                   ::rtl::OUString::createFromAscii(PATHSETTINGS_PROPNAME_ASCII_GALLERY       )
#define PATHSETTINGS_PROPNAME_GRAPHIC                   ::rtl::OUString::createFromAscii(PATHSETTINGS_PROPNAME_ASCII_GRAPHIC       )
#define PATHSETTINGS_PROPNAME_HELP                      ::rtl::OUString::createFromAscii(PATHSETTINGS_PROPNAME_ASCII_HELP          )
#define PATHSETTINGS_PROPNAME_LINGUISTIC                ::rtl::OUString::createFromAscii(PATHSETTINGS_PROPNAME_ASCII_LINGUISTIC    )
#define PATHSETTINGS_PROPNAME_MODULE                    ::rtl::OUString::createFromAscii(PATHSETTINGS_PROPNAME_ASCII_MODULE        )
#define PATHSETTINGS_PROPNAME_PALETTE                   ::rtl::OUString::createFromAscii(PATHSETTINGS_PROPNAME_ASCII_PALETTE       )
#define PATHSETTINGS_PROPNAME_PLUGIN                    ::rtl::OUString::createFromAscii(PATHSETTINGS_PROPNAME_ASCII_PLUGIN        )
#define PATHSETTINGS_PROPNAME_STORAGE                   ::rtl::OUString::createFromAscii(PATHSETTINGS_PROPNAME_ASCII_STORAGE       )
#define PATHSETTINGS_PROPNAME_TEMP                      ::rtl::OUString::createFromAscii(PATHSETTINGS_PROPNAME_ASCII_TEMP          )
#define PATHSETTINGS_PROPNAME_TEMPLATE                  ::rtl::OUString::createFromAscii(PATHSETTINGS_PROPNAME_ASCII_TEMPLATE      )
#define PATHSETTINGS_PROPNAME_UICONFIG                  ::rtl::OUString::createFromAscii(PATHSETTINGS_PROPNAME_ASCII_UICONFIG      )
#define PATHSETTINGS_PROPNAME_USERCONFIG                ::rtl::OUString::createFromAscii(PATHSETTINGS_PROPNAME_ASCII_USERCONFIG    )
#define PATHSETTINGS_PROPNAME_USERDICTIONARY            ::rtl::OUString::createFromAscii(PATHSETTINGS_PROPNAME_ASCII_USERDICTIONARY)
#define PATHSETTINGS_PROPNAME_WORK                      ::rtl::OUString::createFromAscii(PATHSETTINGS_PROPNAME_ASCII_WORK          )

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

//_______________________________________________
/** provides some helper methods to implement property sets. */

class PropHelper
{
    public:

    //___________________________________________

    /** checks if given property will be changed by this settings.
     *  We compare the content of the given any values. If they are different we return TRUE - FALSE otherwhise.
     *
     *  @param  aCurrentValue   contains the current value for this property
     *  @param  aNewValue       contains the new value for this property
     *  @param  aOldValue       returns the current value, in case somthing will be changed
     *  @param  aChangedValue   returns the new value, in case somthing will be changed
     *
     *  @return <True/> if value of this property will be changed; <False/> otherwhise.
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

    //___________________________________________

    /** calculates the count of items inside the given ascii array.
     *
     *  @param  pArray  must point to an array of items from type sal_Char
     *
     *  @return The count of items inside this array, calculated by knowing the size of sal_Char.
     */

    static sal_Int32 calculatePropArrayCount( const sal_Char* pArray[] )
    {
        return (sal_Int32)(sizeof(pArray)/sizeof(sal_Char*));
    }
};

} // namespace framework

#endif // #ifndef __FRAMEWORK_PROPERTIES_H_
