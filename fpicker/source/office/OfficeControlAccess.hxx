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

#ifndef INCLUDED_FPICKER_SOURCE_OFFICE_OFFICECONTROLACCESS_HXX
#define INCLUDED_FPICKER_SOURCE_OFFICE_OFFICECONTROLACCESS_HXX

#include <svtools/fileview.hxx>
#include <vcl/lstbox.hxx>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include "pickercallbacks.hxx"


namespace svt
{



    namespace InternalFilePickerElementIds
    {
        static const sal_Int16 PUSHBUTTON_HELP = (sal_Int16)0x1000;
        static const sal_Int16 TOOLBOXBUTOON_DEFAULT_LOCATION = (sal_Int16)0x1001;
        static const sal_Int16 TOOLBOXBUTOON_LEVEL_UP = (sal_Int16)0x1002;
        static const sal_Int16 TOOLBOXBUTOON_NEW_FOLDER = (sal_Int16)0x1003;
        static const sal_Int16 FIXEDTEXT_CURRENTFOLDER = (sal_Int16)0x1004;
    }


    /** implements the XControlAccess, XControlInformation and XFilePickerControlAccess for the file picker
    */
    class OControlAccess
    {
        IFilePickerController*  m_pFilePickerController;
        VclPtr<SvtFileView>     m_pFileView;

    public:
        OControlAccess( IFilePickerController* _pController, SvtFileView* _pFileView );

        // XControlAccess implementation
        void setControlProperty( const OUString& _rControlName, const OUString& _rControlProperty, const css::uno::Any& _rValue );
        css::uno::Any  getControlProperty( const OUString& _rControlName, const OUString& _rControlProperty );

        // XControlInformation implementation
        css::uno::Sequence< OUString >  getSupportedControls(  );
        css::uno::Sequence< OUString >  getSupportedControlProperties( const OUString& _rControlName );
        static bool                     isControlSupported( const OUString& _rControlName );
        bool                            isControlPropertySupported( const OUString& _rControlName, const OUString& _rControlProperty );

        // XFilePickerControlAccess
        void                        setValue( sal_Int16 _nId, sal_Int16 _nCtrlAction, const css::uno::Any& _rValue );
        css::uno::Any               getValue( sal_Int16 _nId, sal_Int16 _nCtrlAction ) const;
        void                        setLabel( sal_Int16 _nId, const OUString& _rValue );
        OUString                    getLabel( sal_Int16 _nId ) const;
        void                        enableControl( sal_Int16 _nId, bool _bEnable );

        static void             setHelpURL( vcl::Window* _pControl, const OUString& _rURL, bool _bFileView );
        static OUString  getHelpURL( vcl::Window* _pControl, bool _bFileView );

    private:
        /** implements the various methods for setting properties on controls

            @param _nControlId
                the id of the control
            @param _pControl
                the affected control. Must be the same as referred by <arg>_nControlId</arg>, or NULL.
            @param _nProperty
                the property to set
                See PROPERTY_FLAG_*
            @param _rValue
                the value to set
            @param _bIgnoreIllegalArgument
                if <FALSE/>, an exception will be thrown if the given value is of improper type
        */
        void                        implSetControlProperty(
                                        sal_Int16 _nControlId,
                                        Control* _pControl, sal_Int16 _nProperty, const css::uno::Any& _rValue,
                                        bool _bIgnoreIllegalArgument = true );

        Control* implGetControl( const OUString& _rControlName, sal_Int16* _pId = NULL, sal_Int32* _pPropertyMask = NULL ) const;

        /** implements the various methods for retrieving properties from controls

            @param _pControl
                the affected control
                @PRECOND not <NULL/>
            @param _nProperty
                the property to retrieve
                See PROPERTY_FLAG_*
            @return
        */
        css::uno::Any  implGetControlProperty( Control* _pControl, sal_Int16 _nProperty ) const;

        static void implDoListboxAction( ListBox* _pListbox, sal_Int16 _nCtrlAction, const css::uno::Any& _rValue );

    };


}   // namespace svt


#endif // INCLUDED_FPICKER_SOURCE_OFFICE_OFFICECONTROLACCESS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
