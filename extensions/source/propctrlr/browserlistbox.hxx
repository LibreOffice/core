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
#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>
#include <tools/link.hxx>
#include <rtl/ref.hxx>

#include <memory>
#include <set>
#include <unordered_map>
#include <vector>


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

        ListBoxLine( const OUString& rName, BrowserLinePointer _pLine, const css::uno::Reference< css::inspection::XPropertyHandler >& _rxHandler )
            : aName( rName ),
              pLine( _pLine ),
              xHandler( _rxHandler )
        {
        }
    };
    typedef ::std::vector< ListBoxLine > ListBoxLines;


    /** non-UNO version of XPropertyControlContext
    */
    class SAL_NO_VTABLE IControlContext
    {
    public:
        virtual void SAL_CALL focusGained( const css::uno::Reference< css::inspection::XPropertyControl >& Control ) throw (css::uno::RuntimeException) = 0;
        virtual void SAL_CALL valueChanged( const css::uno::Reference< css::inspection::XPropertyControl >& Control ) throw (css::uno::RuntimeException) = 0;
        virtual void SAL_CALL activateNextControl( const css::uno::Reference< css::inspection::XPropertyControl >& CurrentControl ) throw (css::uno::RuntimeException) = 0;

    protected:
        ~IControlContext() {}
    };

    class OBrowserListBox   :public Control
                            ,public IButtonClickListener
                            ,public IControlContext
                            ,public PcrClient
    {
    protected:
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
        ::std::set< sal_uInt16 >    m_aOutOfDateLines;
        bool                    m_bIsActive : 1;
        bool                    m_bUpdate : 1;
        ::rtl::Reference< PropertyControlContext_Impl >
                                    m_pControlContextImpl;

    protected:
        void    PositionLine( sal_uInt16 _nIndex );
        void    UpdatePosNSize();
        void    UpdatePlayGround();
        void    UpdateVScroll();
        void    ShowEntry(sal_uInt16 nPos);
        void    MoveThumbTo(sal_Int32 nNewThumbPos);
        void    Resize() SAL_OVERRIDE;

    public:
                                    OBrowserListBox( vcl::Window* pParent, WinBits nWinStyle = WB_DIALOGCONTROL );

                                    virtual ~OBrowserListBox();
        virtual void                dispose() SAL_OVERRIDE;

        void                        UpdateAll();

        void                        ActivateListBox( bool _bActive );

        sal_uInt16                  CalcVisibleLines();
        void                        EnableUpdate();
        void                        DisableUpdate();
        bool                        Notify( NotifyEvent& _rNEvt ) SAL_OVERRIDE;
        virtual bool                PreNotify( NotifyEvent& _rNEvt ) SAL_OVERRIDE;

        void                        SetListener( IPropertyLineListener* _pListener );
        void                        SetObserver( IPropertyControlObserver* _pObserver );

        void                        EnableHelpSection( bool _bEnable );
        bool                        HasHelpSection() const;
        void                        SetHelpText( const OUString& _rHelpText );
        void                        SetHelpLineLimites( sal_Int32 _nMinLines, sal_Int32 _nMaxLines );

        void                        Clear();

        sal_uInt16                  InsertEntry( const OLineDescriptor&, sal_uInt16 nPos = EDITOR_LIST_APPEND );
        bool                        RemoveEntry( const OUString& _rName );
        void                        ChangeEntry( const OLineDescriptor&, sal_uInt16 nPos );

        void                        SetPropertyValue( const OUString& rEntryName, const css::uno::Any& rValue, bool _bUnknownValue );
        sal_uInt16                  GetPropertyPos( const OUString& rEntryName ) const;
        css::uno::Reference< css::inspection::XPropertyControl >
                                    GetPropertyControl( const OUString& rEntryName );
        void                        EnablePropertyControls( const OUString& _rEntryName, sal_Int16 _nControls, bool _bEnable );
        void                        EnablePropertyLine( const OUString& _rEntryName, bool _bEnable );

        sal_Int32                   GetMinimumWidth();
        sal_Int32                   GetMinimumHeight();


        bool    IsModified( ) const;
        void        CommitModified( );

    protected:
        // IControlContext
        virtual void SAL_CALL focusGained( const css::uno::Reference< css::inspection::XPropertyControl >& Control ) throw (css::uno::RuntimeException) SAL_OVERRIDE;
        virtual void SAL_CALL valueChanged( const css::uno::Reference< css::inspection::XPropertyControl >& Control ) throw (css::uno::RuntimeException) SAL_OVERRIDE;
        virtual void SAL_CALL activateNextControl( const css::uno::Reference< css::inspection::XPropertyControl >& CurrentControl ) throw (css::uno::RuntimeException) SAL_OVERRIDE;

        // IButtonClickListener
        void    buttonClicked( OBrowserLine* _pLine, bool _bPrimary ) SAL_OVERRIDE;

        using Window::SetHelpText;
    private:
        DECL_LINK_TYPED( ScrollHdl, ScrollBar*, void );

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

    private:
        using Window::Activate;
    };


} // namespace pcr


#endif // INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_BROWSERLISTBOX_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
