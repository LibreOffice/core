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

#ifndef INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_BROWSERLISTBOX_HXX
#define INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_BROWSERLISTBOX_HXX

#include "browserline.hxx"
#include "modulepcr.hxx"
#include "pcrcommon.hxx"

#include <com/sun/star/inspection/XPropertyControl.hpp>
#include <com/sun/star/inspection/XPropertyHandler.hpp>
#include <vcl/scrbar.hxx>
#include <vcl/button.hxx>
#include <tools/link.hxx>
#include <rtl/ref.hxx>

#include <limits>
#include <memory>
#include <set>
#include <unordered_map>
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


    class OBrowserListBox final : public Control
                            ,public IButtonClickListener
    {
        VclPtr<Window>              m_aLinesPlayground;
        VclPtr<ScrollBar>           m_aVScroll;
        VclPtr<InspectorHelpWindow> m_pHelpWindow;
        ListBoxLines                m_aLines;
        IPropertyLineListener*      m_pLineListener;
        IPropertyControlObserver*   m_pControlObserver;
        long                        m_nYOffset;
        long                        m_nCurrentPreferredHelpHeight;
        css::uno::Reference< css::inspection::XPropertyControl >
                                    m_xActiveControl;
        sal_uInt16                  m_nTheNameSize;
        long                        m_nRowHeight;
        std::set<ListBoxLines::size_type> m_aOutOfDateLines;
        bool                    m_bIsActive : 1;
        bool                    m_bUpdate : 1;
        ::rtl::Reference< PropertyControlContext_Impl >
                                    m_pControlContextImpl;

        void    PositionLine( ListBoxLines::size_type _nIndex );
        void    UpdatePosNSize();
        void    UpdatePlayGround();
        void    UpdateVScroll();
        void    ShowEntry(sal_uInt16 nPos);
        void    MoveThumbTo(sal_Int32 nNewThumbPos);
        void    Resize() override;

    public:
        explicit                    OBrowserListBox( vcl::Window* pParent );

                                    virtual ~OBrowserListBox() override;
        virtual void                dispose() override;

        void                        ActivateListBox( bool _bActive );

        sal_uInt16                  CalcVisibleLines();
        void                        EnableUpdate();
        void                        DisableUpdate();
        bool                        EventNotify( NotifyEvent& _rNEvt ) override;
        virtual bool                PreNotify( NotifyEvent& _rNEvt ) override;

        void                        SetListener( IPropertyLineListener* _pListener );
        void                        SetObserver( IPropertyControlObserver* _pObserver );

        void                        EnableHelpSection( bool _bEnable );
        bool                        HasHelpSection() const;
        void                        SetHelpText( const OUString& _rHelpText );
        void                        SetHelpLineLimites( sal_Int32 _nMinLines, sal_Int32 _nMaxLines );

        void                        Clear();

        void                        InsertEntry( const OLineDescriptor&, sal_uInt16 nPos );
        bool                        RemoveEntry( const OUString& _rName );
        void                        ChangeEntry( const OLineDescriptor&, ListBoxLines::size_type nPos );

        void                        SetPropertyValue( const OUString& rEntryName, const css::uno::Any& rValue, bool _bUnknownValue );
        sal_uInt16                  GetPropertyPos( const OUString& rEntryName ) const;
        css::uno::Reference< css::inspection::XPropertyControl >
                                    GetPropertyControl( const OUString& rEntryName );
        void                        EnablePropertyControls( const OUString& _rEntryName, sal_Int16 _nControls, bool _bEnable );
        void                        EnablePropertyLine( const OUString& _rEntryName, bool _bEnable );

        sal_Int32                   GetMinimumWidth() const;
        sal_Int32                   GetMinimumHeight();


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

        using Window::SetHelpText;

        DECL_LINK( ScrollHdl, ScrollBar*, void );

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

        /** returns the preferred height (in pixels) of the help section, or 0 if we
            currently don't have a help section
        */
        long        impl_getPrefererredHelpHeight();

        using Window::Activate;
    };


} // namespace pcr


#endif // INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_BROWSERLISTBOX_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
