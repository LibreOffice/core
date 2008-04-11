/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: acc_factory.cxx,v $
 * $Revision: 1.5 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_accessibility.hxx"
#include <accessibility/helper/acc_factory.hxx>

#ifndef _TOOLKIT_AWT_VCLXWINDOWS_HXX
#include <toolkit/awt/vclxwindows.hxx>
#endif
#include <accessibility/standard/vclxaccessiblebutton.hxx>
#include <accessibility/standard/vclxaccessiblecheckbox.hxx>
#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLEDROPDOWCOMBOBOX_HXX
#include <accessibility/standard/vclxaccessibledropdowncombobox.hxx>
#endif
#include <accessibility/standard/vclxaccessiblecombobox.hxx>
#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLEDROPDOWLISTBOX_HXX
#include <accessibility/standard/vclxaccessibledropdownlistbox.hxx>
#endif
#include <accessibility/standard/vclxaccessibleedit.hxx>
#include <accessibility/standard/vclxaccessiblefixedhyperlink.hxx>
#include <accessibility/standard/vclxaccessiblefixedtext.hxx>
#include <accessibility/standard/vclxaccessiblelistbox.hxx>
#include <accessibility/standard/vclxaccessiblemenu.hxx>
#include <accessibility/standard/vclxaccessibleradiobutton.hxx>
#include <accessibility/standard/vclxaccessiblescrollbar.hxx>
#include <accessibility/standard/vclxaccessibletextcomponent.hxx>
#include <accessibility/standard/vclxaccessibletoolbox.hxx>
#include <toolkit/awt/vclxaccessiblecomponent.hxx>
#include <accessibility/standard/vclxaccessiblestatusbar.hxx>
#include <accessibility/standard/vclxaccessibletabcontrol.hxx>
#include <accessibility/standard/vclxaccessibletabpagewindow.hxx>
#include <accessibility/standard/vclxaccessiblemenubar.hxx>
#include <accessibility/standard/vclxaccessiblepopupmenu.hxx>
#include <accessibility/extended/accessibletablistbox.hxx>
#include <accessibility/extended/AccessibleBrowseBox.hxx>
#include <accessibility/extended/accessibleiconchoicectrl.hxx>
#include <accessibility/extended/accessibletabbar.hxx>
#include <accessibility/extended/accessiblelistbox.hxx>
#include <accessibility/extended/AccessibleBrowseBoxHeaderBar.hxx>
#include <accessibility/extended/textwindowaccessibility.hxx>
#include <accessibility/extended/AccessibleBrowseBoxTableCell.hxx>
#include <accessibility/extended/AccessibleBrowseBoxHeaderCell.hxx>
#include <accessibility/extended/AccessibleBrowseBoxCheckBoxCell.hxx>
#include <accessibility/extended/accessibleeditbrowseboxcell.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/combobox.hxx>

//........................................................................
namespace accessibility
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::accessibility;
    using namespace ::svt;

    //================================================================
    //= IAccessibleFactory
    //================================================================
    class AccessibleFactory :public ::toolkit::IAccessibleFactory
                            ,public ::svt::IAccessibleFactory
    {
    private:
        oslInterlockedCount m_refCount;

    public:
        AccessibleFactory();

        // IReference
        virtual oslInterlockedCount SAL_CALL acquire();
        virtual oslInterlockedCount SAL_CALL release();

        // ::toolkit::IAccessibleFactory
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >
            createAccessibleContext( VCLXButton* _pXWindow );
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >
            createAccessibleContext( VCLXCheckBox* _pXWindow );
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >
            createAccessibleContext( VCLXRadioButton* _pXWindow );
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >
            createAccessibleContext( VCLXListBox* _pXWindow );
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >
            createAccessibleContext( VCLXFixedText* _pXWindow );
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >
            createAccessibleContext( VCLXFixedHyperlink* _pXWindow );
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >
            createAccessibleContext( VCLXScrollBar* _pXWindow );
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >
            createAccessibleContext( VCLXEdit* _pXWindow );
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >
            createAccessibleContext( VCLXComboBox* _pXWindow );
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >
            createAccessibleContext( VCLXToolBox* _pXWindow );
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >
            createAccessibleContext( VCLXWindow* _pXWindow );
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
            createAccessible( Menu* _pMenu, sal_Bool _bIsMenuBar );

        // ::svt::IAccessibleFactory
        virtual IAccessibleTabListBox*
            createAccessibleTabListBox(
                const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& rxParent,
                SvHeaderTabListBox& rBox
            ) const;

        virtual IAccessibleBrowseBox*
            createAccessibleBrowseBox(
                const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& _rxParent,
                IAccessibleTableProvider& _rBrowseBox
            ) const;

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
            createAccessibleIconChoiceCtrl(
                SvtIconChoiceCtrl& _rIconCtrl,
                const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& _xParent
            ) const;

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
            createAccessibleTabBar(
                TabBar& _rTabBar
            ) const;

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >
            createAccessibleTextWindowContext(
                VCLXWindow* pVclXWindow, TextEngine& rEngine, TextView& rView, bool bCompoundControlChild
            ) const;

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
            createAccessibleTreeListBox(
                SvTreeListBox& _rListBox,
                const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& _xParent
            ) const;

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
            createAccessibleBrowseBoxHeaderBar(
                const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& rxParent,
                IAccessibleTableProvider& _rOwningTable,
                AccessibleBrowseBoxObjType _eObjType
            ) const;

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
            createAccessibleBrowseBoxTableCell(
                const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& _rxParent,
                IAccessibleTableProvider& _rBrowseBox,
                const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >& _xFocusWindow,
                sal_Int32 _nRowId,
                sal_uInt16 _nColId,
                sal_Int32 _nOffset
            ) const;

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
            createAccessibleBrowseBoxHeaderCell(
                sal_Int32 _nColumnRowId,
                const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& rxParent,
                IAccessibleTableProvider& _rBrowseBox,
                const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >& _xFocusWindow,
                AccessibleBrowseBoxObjType  _eObjType
            ) const;

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
            createAccessibleCheckBoxCell(
                const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& _rxParent,
                IAccessibleTableProvider& _rBrowseBox,
                const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >& _xFocusWindow,
                sal_Int32 _nRowPos,
                sal_uInt16 _nColPos,
                const TriState& _eState,
                sal_Bool _bEnabled,
                sal_Bool _bIsTriState
            ) const;

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
            createEditBrowseBoxTableCellAccess(
                const ::com::sun::star::uno::Reference< com::sun::star::accessibility::XAccessible >& _rxParent,
                const ::com::sun::star::uno::Reference< com::sun::star::accessibility::XAccessible >& _rxControlAccessible,
                const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >& _rxFocusWindow,
                IAccessibleTableProvider& _rBrowseBox,
                sal_Int32 _nRowPos,
                sal_uInt16 _nColPos
            ) const;

    protected:
        virtual ~AccessibleFactory();
    };


    //--------------------------------------------------------------------
    AccessibleFactory::AccessibleFactory()
        :m_refCount( 0 )
    {
    }

    //--------------------------------------------------------------------
    AccessibleFactory::~AccessibleFactory()
    {
    }

    //--------------------------------------------------------------------
    oslInterlockedCount SAL_CALL AccessibleFactory::acquire()
    {
        return osl_incrementInterlockedCount( &m_refCount );
    }

    //--------------------------------------------------------------------
    oslInterlockedCount SAL_CALL AccessibleFactory::release()
    {
        if ( 0 == osl_decrementInterlockedCount( &m_refCount ) )
        {
            delete this;
            return 0;
        }
        return m_refCount;
    }

    //--------------------------------------------------------------------
    Reference< XAccessible > AccessibleFactory::createAccessible( Menu* _pMenu, sal_Bool _bIsMenuBar )
    {
        OAccessibleMenuBaseComponent* pAccessible;
        if ( _bIsMenuBar )
            pAccessible = new VCLXAccessibleMenuBar( _pMenu );
        else
            pAccessible = new VCLXAccessiblePopupMenu( _pMenu );
        pAccessible->SetStates();
        return pAccessible;
    }

    //--------------------------------------------------------------------
    Reference< XAccessibleContext > AccessibleFactory::createAccessibleContext( VCLXButton* _pXWindow )
    {
        return new VCLXAccessibleButton( _pXWindow );
    }

    //--------------------------------------------------------------------
    Reference< XAccessibleContext > AccessibleFactory::createAccessibleContext( VCLXCheckBox* _pXWindow )
    {
        return new VCLXAccessibleCheckBox( _pXWindow );
    }

    //--------------------------------------------------------------------
    Reference< XAccessibleContext > AccessibleFactory::createAccessibleContext( VCLXRadioButton* _pXWindow )
    {
        return new VCLXAccessibleRadioButton( _pXWindow );
    }

    //--------------------------------------------------------------------
    Reference< XAccessibleContext > AccessibleFactory::createAccessibleContext( VCLXListBox* _pXWindow )
    {
        sal_Bool bIsDropDownBox = sal_False;
        ListBox* pBox = static_cast< ListBox* >( _pXWindow->GetWindow() );
        if ( pBox )
            bIsDropDownBox = ( ( pBox->GetStyle() & WB_DROPDOWN ) == WB_DROPDOWN );

        if ( bIsDropDownBox )
            return new VCLXAccessibleDropDownListBox( _pXWindow );
        else
            return new VCLXAccessibleListBox( _pXWindow );
    }

    //--------------------------------------------------------------------
    Reference< XAccessibleContext > AccessibleFactory::createAccessibleContext( VCLXFixedText* _pXWindow )
    {
        return new VCLXAccessibleFixedText( _pXWindow );
    }

    //--------------------------------------------------------------------
    Reference< XAccessibleContext > AccessibleFactory::createAccessibleContext( VCLXFixedHyperlink* _pXWindow )
    {
        return new VCLXAccessibleFixedHyperlink( _pXWindow );
    }

    //--------------------------------------------------------------------
    Reference< XAccessibleContext > AccessibleFactory::createAccessibleContext( VCLXScrollBar* _pXWindow )
    {
        return new VCLXAccessibleScrollBar( _pXWindow );
    }

    //--------------------------------------------------------------------
    Reference< XAccessibleContext > AccessibleFactory::createAccessibleContext( VCLXEdit* _pXWindow )
    {
        return new VCLXAccessibleEdit( _pXWindow );
    }

    //--------------------------------------------------------------------
    Reference< XAccessibleContext > AccessibleFactory::createAccessibleContext( VCLXComboBox* _pXWindow )
    {
        sal_Bool bIsDropDownBox = sal_False;
        ComboBox* pBox = static_cast< ComboBox* >( _pXWindow->GetWindow() );
        if ( pBox )
            bIsDropDownBox = ( ( pBox->GetStyle() & WB_DROPDOWN ) == WB_DROPDOWN );

        if ( bIsDropDownBox )
            return new VCLXAccessibleDropDownComboBox( _pXWindow );
        else
            return new VCLXAccessibleComboBox( _pXWindow );
    }

    //--------------------------------------------------------------------
    Reference< XAccessibleContext > AccessibleFactory::createAccessibleContext( VCLXWindow* _pXWindow )
    {
        Reference< XAccessibleContext > xContext;

        Window* pWindow = _pXWindow->GetWindow();
        if ( pWindow )
        {
            WindowType nType = pWindow->GetType();

            if ( nType == WINDOW_MENUBARWINDOW || pWindow->IsMenuFloatingWindow() || pWindow->IsToolbarFloatingWindow() )
            {
                Reference< XAccessible > xAcc( pWindow->GetAccessible() );
                if ( xAcc.is() )
                {
                    Reference< XAccessibleContext > xCont( xAcc->getAccessibleContext() );
                    if ( pWindow->GetType() == WINDOW_MENUBARWINDOW ||
                        ( xCont.is() && xCont->getAccessibleRole() == AccessibleRole::POPUP_MENU ) )
                    {
                        xContext = xCont;
                    }
                }
            }
            else if ( nType == WINDOW_STATUSBAR )
            {
                xContext = (XAccessibleContext*) new VCLXAccessibleStatusBar( _pXWindow );
            }
            else if ( nType == WINDOW_TABCONTROL )
            {
                xContext = (XAccessibleContext*) new VCLXAccessibleTabControl( _pXWindow );
            }
            else if ( nType == WINDOW_TABPAGE && pWindow->GetAccessibleParentWindow() && pWindow->GetAccessibleParentWindow()->GetType() == WINDOW_TABCONTROL )
            {
                xContext = new VCLXAccessibleTabPageWindow( _pXWindow );
            }
            else if ( nType == WINDOW_HELPTEXTWINDOW )
            {
               xContext = (accessibility::XAccessibleContext*) new VCLXAccessibleFixedText( _pXWindow );
            }
            else
            {
               xContext = (accessibility::XAccessibleContext*) new VCLXAccessibleComponent( _pXWindow );
            }
        }

        return xContext;
    }

    //--------------------------------------------------------------------
    Reference< XAccessibleContext > AccessibleFactory::createAccessibleContext( VCLXToolBox* _pXWindow )
    {
        return new VCLXAccessibleToolBox( _pXWindow );
    }

    //--------------------------------------------------------------------
    IAccessibleTabListBox* AccessibleFactory::createAccessibleTabListBox(
        const Reference< XAccessible >& rxParent, SvHeaderTabListBox& rBox ) const
    {
        return new AccessibleTabListBox( rxParent, rBox );
    }

    //--------------------------------------------------------------------
    IAccessibleBrowseBox* AccessibleFactory::createAccessibleBrowseBox(
        const Reference< XAccessible >& _rxParent, IAccessibleTableProvider& _rBrowseBox ) const
    {
        return new AccessibleBrowseBoxAccess( _rxParent, _rBrowseBox );
    }

    //--------------------------------------------------------------------
    Reference< XAccessible > AccessibleFactory::createAccessibleIconChoiceCtrl(
        SvtIconChoiceCtrl& _rIconCtrl, const Reference< XAccessible >& _xParent ) const
    {
        return new AccessibleIconChoiceCtrl( _rIconCtrl, _xParent );
    }

    //--------------------------------------------------------------------
    Reference< XAccessible > AccessibleFactory::createAccessibleTabBar( TabBar& _rTabBar ) const
    {
        return new AccessibleTabBar( &_rTabBar );
    }

    //--------------------------------------------------------------------
    Reference< XAccessibleContext > AccessibleFactory::createAccessibleTextWindowContext(
        VCLXWindow* pVclXWindow, TextEngine& rEngine, TextView& rView, bool bCompoundControlChild ) const
    {
        return new Document( pVclXWindow, rEngine, rView, bCompoundControlChild );
    }

    //--------------------------------------------------------------------
    Reference< XAccessible > AccessibleFactory::createAccessibleTreeListBox(
        SvTreeListBox& _rListBox, const Reference< XAccessible >& _xParent ) const
    {
        return new AccessibleListBox( _rListBox, _xParent );
    }

    //--------------------------------------------------------------------
    Reference< XAccessible > AccessibleFactory::createAccessibleBrowseBoxHeaderBar(
        const Reference< XAccessible >& rxParent, IAccessibleTableProvider& _rOwningTable,
        AccessibleBrowseBoxObjType _eObjType ) const
    {
        return new AccessibleBrowseBoxHeaderBar( rxParent, _rOwningTable, _eObjType );
    }

    //--------------------------------------------------------------------
    Reference< XAccessible > AccessibleFactory::createAccessibleBrowseBoxTableCell(
        const Reference< XAccessible >& _rxParent, IAccessibleTableProvider& _rBrowseBox,
        const Reference< XWindow >& _xFocusWindow, sal_Int32 _nRowId, sal_uInt16 _nColId, sal_Int32 _nOffset ) const
    {
        return new AccessibleBrowseBoxTableCell( _rxParent, _rBrowseBox, _xFocusWindow,
            _nRowId, _nColId, _nOffset );
    }

    //--------------------------------------------------------------------
    Reference< XAccessible > AccessibleFactory::createAccessibleBrowseBoxHeaderCell(
        sal_Int32 _nColumnRowId, const Reference< XAccessible >& rxParent, IAccessibleTableProvider& _rBrowseBox,
        const Reference< XWindow >& _xFocusWindow, AccessibleBrowseBoxObjType  _eObjType ) const
    {
        return new AccessibleBrowseBoxHeaderCell( _nColumnRowId, rxParent, _rBrowseBox,
            _xFocusWindow, _eObjType );
    }

    //--------------------------------------------------------------------
    Reference< XAccessible > AccessibleFactory::createAccessibleCheckBoxCell(
        const Reference< XAccessible >& _rxParent, IAccessibleTableProvider& _rBrowseBox,
        const Reference< XWindow >& _xFocusWindow, sal_Int32 _nRowPos, sal_uInt16 _nColPos,
        const TriState& _eState, sal_Bool _bEnabled, sal_Bool _bIsTriState ) const
    {
        return new AccessibleCheckBoxCell( _rxParent, _rBrowseBox, _xFocusWindow,
            _nRowPos, _nColPos, _eState, _bEnabled, _bIsTriState );
    }

    //--------------------------------------------------------------------
    Reference< XAccessible > AccessibleFactory::createEditBrowseBoxTableCellAccess(
        const Reference< XAccessible >& _rxParent, const Reference< XAccessible >& _rxControlAccessible,
        const Reference< XWindow >& _rxFocusWindow, IAccessibleTableProvider& _rBrowseBox,
        sal_Int32 _nRowPos, sal_uInt16 _nColPos ) const
    {
        return new EditBrowseBoxTableCellAccess( _rxParent, _rxControlAccessible,
            _rxFocusWindow, _rBrowseBox, _nRowPos, _nColPos );
    }

//........................................................................
}   // namespace accessibility
//........................................................................

//========================================================================
extern "C" void* SAL_CALL getStandardAccessibleFactory()
{
    ::toolkit::IAccessibleFactory* pFactory = new ::accessibility::AccessibleFactory;
    pFactory->acquire();
    return pFactory;
}

extern "C" void* SAL_CALL getSvtAccessibilityComponentFactory()
{
    ::svt::IAccessibleFactory* pFactory = new ::accessibility::AccessibleFactory;
    pFactory->acquire();
    return pFactory;
}
