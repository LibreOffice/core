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

#pragma once

#include <sal/config.h>

#include <string_view>

#include "fileview.hxx"
#include "pickercallbacks.hxx"
#include <o3tl/typed_flags_set.hxx>

enum class PropFlags {
    Unknown           =     -1, // used as an error sentinel
    NONE              = 0x0000,
    Text              = 0x0001,
    Enabled           = 0x0002,
    Visible           = 0x0004,
    HelpUrl           = 0x0008,
    ListItems         = 0x0010,
    SelectedItem      = 0x0020,
    SelectedItemIndex = 0x0040,
    Checked           = 0x0080,
};
namespace o3tl {
    template<> struct typed_flags<PropFlags> : is_typed_flags<PropFlags, 0x00ff> {};
}


namespace svt
{


    namespace InternalFilePickerElementIds
    {
        const sal_Int16 PUSHBUTTON_HELP = sal_Int16(0x1000);
        const sal_Int16 TOOLBOXBUTOON_DEFAULT_LOCATION = sal_Int16(0x1001);
        const sal_Int16 TOOLBOXBUTOON_LEVEL_UP = sal_Int16(0x1002);
        const sal_Int16 TOOLBOXBUTOON_NEW_FOLDER = sal_Int16(0x1003);
        const sal_Int16 FIXEDTEXT_CURRENTFOLDER = sal_Int16(0x1004);
    }


    /** implements the XControlAccess, XControlInformation and XFilePickerControlAccess for the file picker
    */
    class OControlAccess
    {
        IFilePickerController*  m_pFilePickerController;
        SvtFileView*     m_pFileView;

    public:
        OControlAccess( IFilePickerController* pController, SvtFileView* pFileView );

        // XControlAccess implementation
        void setControlProperty( std::u16string_view rControlName, const OUString& rControlProperty, const css::uno::Any& rValue );
        css::uno::Any  getControlProperty( std::u16string_view rControlName, const OUString& rControlProperty );

        // XControlInformation implementation
        css::uno::Sequence< OUString >  getSupportedControls(  ) const;
        css::uno::Sequence< OUString >  getSupportedControlProperties( std::u16string_view rControlName );
        static bool                     isControlSupported( std::u16string_view rControlName );
        bool                            isControlPropertySupported( std::u16string_view rControlName, const OUString& rControlProperty );

        // XFilePickerControlAccess
        void                        setValue( sal_Int16 nId, sal_Int16 nCtrlAction, const css::uno::Any& rValue );
        css::uno::Any               getValue( sal_Int16 nId, sal_Int16 nCtrlAction ) const;
        void                        setLabel( sal_Int16 nId, const OUString& rValue );
        OUString                    getLabel( sal_Int16 nId ) const;
        void                        enableControl( sal_Int16 nId, bool bEnable );

        void setHelpURL(weld::Widget* pControl, const OUString& rURL);
        OUString getHelpURL(weld::Widget const* pControl) const;

    private:
        /** implements the various methods for setting properties on controls

            @param nControlId
                the id of the control
            @param pControl
                the affected control. Must be the same as referred by <arg>nControlId</arg>, or NULL.
            @param nProperty
                the property to set
                See PropFlags::*
            @param rValue
                the value to set
            @param bIgnoreIllegalArgument
                if <FALSE/>, an exception will be thrown if the given value is of improper type
        */
        void                        implSetControlProperty(
                                        sal_Int16 nControlId,
                                        weld::Widget* pControl, PropFlags nProperty, const css::uno::Any& rValue,
                                        bool bIgnoreIllegalArgument = true );

        weld::Widget* implGetControl( std::u16string_view rControlName, sal_Int16* pId, PropFlags* pPropertyMask = nullptr ) const;

        /** implements the various methods for retrieving properties from controls

            @param pControl
                the affected control
                @PRECOND not <NULL/>
            @param nProperty
                the property to retrieve
                See PropFlags::*
            @return
        */
        css::uno::Any  implGetControlProperty( weld::Widget const * pControl, PropFlags nProperty ) const;

        bool IsFileViewWidget(weld::Widget const * pControl) const;

        static void implDoListboxAction(weld::ComboBox* pListbox, sal_Int16 nCtrlAction, const css::uno::Any& rValue);

    };


}   // namespace svt


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
