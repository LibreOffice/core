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

#include "browserline.hxx"

#include <com/sun/star/inspection/XPropertyControl.hpp>
#include <com/sun/star/inspection/XPropertyHandler.hpp>
#include <vcl/weld.hxx>
#include <rtl/ref.hxx>

#include <memory>
#include <vector>

#define EDITOR_LIST_REPLACE_EXISTING \
    std::numeric_limits<ListBoxLines::size_type>::max()

namespace pcr
{


    class IPropertyLineListener;
    class IPropertyControlObserver;
    struct OLineDescriptor;
    class InspectorHelpWindow;
    class PropertyControlContext_Impl;


    // administrative structures for OBrowserListBox

    typedef std::shared_ptr< OBrowserLine > BrowserLinePointer;
    struct ListBoxLine
    {
        OUString                         aName;
        BrowserLinePointer                      pLine;
        css::uno::Reference< css::inspection::XPropertyHandler >
                                                xHandler;

        ListBoxLine( const OUString& rName, const BrowserLinePointer& _pLine, const css::uno::Reference< css::inspection::XPropertyHandler >& _rxHandler )
            : aName( rName ),
              pLine( _pLine ),
              xHandler( _rxHandler )
        {
        }
    };
    typedef std::vector< ListBoxLine > ListBoxLines;


    class OBrowserListBox final : public IButtonClickListener
    {
        std::unique_ptr<weld::ScrolledWindow> m_xScrolledWindow;
        std::unique_ptr<weld::Container> m_xLinesPlayground;
        std::unique_ptr<weld::SizeGroup> m_xSizeGroup;
        std::unique_ptr<InspectorHelpWindow> m_xHelpWindow;
        weld::Container*            m_pInitialControlParent;
        ListBoxLines                m_aLines;
        IPropertyLineListener*      m_pLineListener;
        IPropertyControlObserver*   m_pControlObserver;
        css::uno::Reference< css::inspection::XPropertyControl >
                                    m_xActiveControl;
        sal_uInt16                  m_nTheNameSize;
        int                         m_nRowHeight;
        ::rtl::Reference< PropertyControlContext_Impl >
                                    m_pControlContextImpl;

        void    UpdatePlayGround();
        void    ShowEntry(sal_uInt16 nPos);

    public:
        explicit OBrowserListBox(weld::Builder& rBuilder, weld::Container* pContainer);
        ~OBrowserListBox();

        void                        SetListener( IPropertyLineListener* _pListener );
        void                        SetObserver( IPropertyControlObserver* _pObserver );

        void                        EnableHelpSection( bool _bEnable );
        bool                        HasHelpSection() const;
        void                        SetHelpText( const OUString& _rHelpText );

        void                        Clear();

        void                        InsertEntry( const OLineDescriptor&, sal_uInt16 nPos );
        bool                        RemoveEntry( const OUString& _rName );
        void                        ChangeEntry( const OLineDescriptor&, ListBoxLines::size_type nPos );

        void                        SetPropertyValue( const OUString& rEntryName, const css::uno::Any& rValue, bool _bUnknownValue );
        sal_uInt16                  GetPropertyPos( std::u16string_view rEntryName ) const;
        css::uno::Reference< css::inspection::XPropertyControl >
                                    GetPropertyControl( const OUString& rEntryName );
        void                        EnablePropertyControls( const OUString& _rEntryName, sal_Int16 _nControls, bool _bEnable );
        void                        EnablePropertyLine( const OUString& _rEntryName, bool _bEnable );

        bool                        IsModified( ) const;
        void                        CommitModified( );

        /// @throws css::uno::RuntimeException
        void               focusGained( const css::uno::Reference< css::inspection::XPropertyControl >& Control );
        /// @throws css::uno::RuntimeException
        void               valueChanged( const css::uno::Reference< css::inspection::XPropertyControl >& Control );
        /// @throws css::uno::RuntimeException
        void               activateNextControl( const css::uno::Reference< css::inspection::XPropertyControl >& CurrentControl );

    private:
        // IButtonClickListener
        void    buttonClicked( OBrowserLine* _pLine, bool _bPrimary ) override;

        /** retrieves the index of a given control in our line list
            @param _rxControl
                The control to lookup. Must denote a control of one of the lines in ->m_aLines
        */
        sal_uInt16  impl_getControlPos( const css::uno::Reference< css::inspection::XPropertyControl >& _rxControl ) const;

        /** sets the given property value at the given control, after converting it as necessary
            @param _rLine
                The line whose at which the value is to be set.
            @param _rPropertyValue
                the property value to set. If it's not compatible with the control value,
                it will be converted, using <member>XPropertyHandler::convertToControlValue</member>
        */
        static void impl_setControlAsPropertyValue( const ListBoxLine& _rLine, const css::uno::Any& _rPropertyValue );

        /** retrieves the value for the given control, as a property value, after converting it as necessary
            @param _rLine
                The line whose at which the value is to be set.
        */
        static css::uno::Any
                    impl_getControlAsPropertyValue( const ListBoxLine& _rLine );

        /** retrieves the ->BrowserLinePointer for a given entry name
            @param  _rEntryName
                the name whose line is to be looked up
            @param  _out_rpLine
                contains, upon return, the found browser line, if any
            @return
                <TRUE/> if and only if a non-<NULL/> line for the given entry name could be
                found.
        */
        bool        impl_getBrowserLineForName( const OUString& _rEntryName, BrowserLinePointer& _out_rpLine ) const;
    };


} // namespace pcr



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
