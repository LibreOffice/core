/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: browserlistbox.hxx,v $
 * $Revision: 1.13 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _EXTENSIONS_PROPCTRLR_BROWSERLISTBOX_HXX_
#define _EXTENSIONS_PROPCTRLR_BROWSERLISTBOX_HXX_

#include "browserline.hxx"
#ifndef _EXTENSIONS_PROPCTRLR_MODULEPRC_HXX_
#include "modulepcr.hxx"
#endif
#include "pcrcommon.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/inspection/XPropertyControl.hpp>
#include <com/sun/star/inspection/XPropertyHandler.hpp>
/** === end UNO includes === **/
#include <vcl/scrbar.hxx>
#include <vcl/lstbox.hxx>
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#include <tools/link.hxx>
#include <rtl/ref.hxx>

#include <set>
#include <vector>
#include <hash_map>
#include <boost/shared_ptr.hpp>

//............................................................................
namespace pcr
{
//............................................................................

    class IPropertyLineListener;
    class IPropertyControlObserver;
    struct OLineDescriptor;
    class InspectorHelpWindow;
    class PropertyControlContext_Impl;

    //========================================================================
    //= administrative structures for OBrowserListBox
    //========================================================================
    typedef ::boost::shared_ptr< OBrowserLine > BrowserLinePointer;
    struct ListBoxLine
    {
        BrowserLinePointer                      pLine;
        ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyHandler >
                                                xHandler;

        ListBoxLine() { }
        ListBoxLine( BrowserLinePointer _pLine, const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyHandler >& _rxHandler )
            :pLine( _pLine )
            ,xHandler( _rxHandler )
        {
        }
    };
    typedef ::std::hash_map< ::rtl::OUString, ListBoxLine, ::rtl::OUStringHash >    ListBoxLines;
    typedef ::std::vector< ListBoxLines::iterator >                                 OrderedListBoxLines;

    //========================================================================
    //= IControlContext
    //========================================================================
    /** non-UNO version of XPropertyControlContext
    */
    class SAL_NO_VTABLE IControlContext
    {
    public:
        virtual void SAL_CALL focusGained( const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControl >& Control ) throw (::com::sun::star::uno::RuntimeException) = 0;
        virtual void SAL_CALL valueChanged( const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControl >& Control ) throw (::com::sun::star::uno::RuntimeException) = 0;
        virtual void SAL_CALL activateNextControl( const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControl >& CurrentControl ) throw (::com::sun::star::uno::RuntimeException) = 0;
    };

    //========================================================================
    //= OBrowserListBox
    //========================================================================
    class OBrowserListBox   :public Control
                            ,public IButtonClickListener
                            ,public IControlContext
                            ,public PcrClient
    {
    protected:
        Window                      m_aLinesPlayground;
        ScrollBar                   m_aVScroll;
        ::std::auto_ptr< InspectorHelpWindow >
                                    m_pHelpWindow;
        ListBoxLines                m_aLines;
        OrderedListBoxLines         m_aOrderedLines;
        IPropertyLineListener*      m_pLineListener;
        IPropertyControlObserver*   m_pControlObserver;
        long                        m_nYOffset;
        long                        m_nCurrentPreferredHelpHeight;
        ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControl >
                                    m_xActiveControl;
        sal_uInt16                  m_nTheNameSize;
        sal_uInt16                  m_nRowHeight;
        ::std::set< sal_uInt16 >    m_aOutOfDateLines;
        sal_Bool                    m_bIsActive : 1;
        sal_Bool                    m_bUpdate : 1;
        ::rtl::Reference< PropertyControlContext_Impl >
                                    m_pControlContextImpl;

    protected:
        void    PositionLine( sal_uInt16 _nIndex );
        void    UpdatePosNSize();
        void    UpdatePlayGround();
        void    UpdateVScroll();
        void    ShowEntry(sal_uInt16 nPos);
        void    MoveThumbTo(sal_Int32 nNewThumbPos);
        void    Resize();

    public:
                                    OBrowserListBox( Window* pParent, WinBits nWinStyle = WB_DIALOGCONTROL );

                                    ~OBrowserListBox();

        void                        UpdateAll();

        void                        ActivateListBox( sal_Bool _bActive );

        sal_uInt16                  CalcVisibleLines();
        void                        EnableUpdate();
        void                        DisableUpdate();
        long                        Notify( NotifyEvent& _rNEvt );

        void                        SetListener( IPropertyLineListener* _pListener );
        void                        SetObserver( IPropertyControlObserver* _pObserver );

        void                        EnableHelpSection( bool _bEnable );
        bool                        HasHelpSection() const;
        void                        SetHelpText( const ::rtl::OUString& _rHelpText );
        void                        SetHelpLineLimites( sal_Int32 _nMinLines, sal_Int32 _nMaxLines );

        void                        Clear();

        sal_uInt16                  InsertEntry( const OLineDescriptor&, sal_uInt16 nPos = EDITOR_LIST_APPEND );
        sal_Bool                    RemoveEntry( const ::rtl::OUString& _rName );
        void                        ChangeEntry( const OLineDescriptor&, sal_uInt16 nPos );

        void                        SetPropertyValue( const ::rtl::OUString& rEntryName, const ::com::sun::star::uno::Any& rValue );
        ::com::sun::star::uno::Any  GetPropertyValue( const ::rtl::OUString& rEntryName ) const;
        sal_uInt16                  GetPropertyPos( const ::rtl::OUString& rEntryName ) const;
        ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControl >
                                    GetPropertyControl( const ::rtl::OUString& rEntryName );
        void                        EnablePropertyControls( const ::rtl::OUString& _rEntryName, sal_Int16 _nControls, bool _bEnable );
        void                        EnablePropertyLine( const ::rtl::OUString& _rEntryName, bool _bEnable );
        sal_Bool                    IsPropertyInputEnabled( const ::rtl::OUString& _rEntryName ) const;

        sal_Int32                   GetMinimumWidth();
        sal_Int32                   GetMinimumHeight();


        sal_Bool    IsModified( ) const;
        void        CommitModified( );

    protected:
        // IControlContext
        virtual void SAL_CALL focusGained( const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControl >& Control ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL valueChanged( const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControl >& Control ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL activateNextControl( const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControl >& CurrentControl ) throw (::com::sun::star::uno::RuntimeException);

        // IButtonClickListener
        void    buttonClicked( OBrowserLine* _pLine, sal_Bool _bPrimary );

        using Window::SetHelpText;
    private:
        DECL_LINK( ScrollHdl, ScrollBar* );

        /** retrieves the index of a given control in our line list
            @param _rxControl
                The control to lookup. Must denote a control of one of the lines in ->m_aLines
        */
        sal_uInt16  impl_getControlPos( const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControl >& _rxControl ) const;

        /** retrieves (a reference to) the ->ListBoxLine for a given control
            @param _rxControl
                The control to lookup. Must denote a control of one of the lines in ->m_aLines
        */
        inline const ListBoxLine&
                    impl_getControlLine( const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControl >& _rxControl ) const
        {
            return m_aOrderedLines[ impl_getControlPos( _rxControl ) ]->second;
        }

        /** sets the given property value at the given control, after converting it as necessary
            @param _rLine
                The line whose at which the value is to be set.
            @param _rPropertyValue
                the property value to set. If it's not compatible with the control value,
                it will be converted, using <member>XPropertyHandler::convertToControlValue</member>
        */
        void        impl_setControlAsPropertyValue( const ListBoxLine& _rLine, const ::com::sun::star::uno::Any& _rPropertyValue );

        /** retrieves the value for the given control, as a property value, after converting it as necessary
            @param _rLine
                The line whose at which the value is to be set.
        */
        ::com::sun::star::uno::Any
                    impl_getControlAsPropertyValue( const ListBoxLine& _rLine ) const;

        /** retrieves the ->BrowserLinePointer for a given entry name
            @param  _rEntryName
                the name whose line is to be looked up
            @param  _out_rpLine
                contains, upon return, the found browser line, if any
            @return
                <TRUE/> if and only if a non-<NULL/> line for the given entry name could be
                found.
        */
        bool        impl_getBrowserLineForName( const ::rtl::OUString& _rEntryName, BrowserLinePointer& _out_rpLine ) const;

        /** returns the preferred height (in pixels) of the help section, or 0 if we
            currently don't have a help section
        */
        long        impl_getPrefererredHelpHeight();

    private:
        using Window::Activate;
    };

//............................................................................
} // namespace pcr
//............................................................................

#endif // _EXTENSIONS_PROPCTRLR_BROWSERLISTBOX_HXX_


