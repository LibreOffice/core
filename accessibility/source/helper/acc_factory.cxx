/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: acc_factory.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 15:26:59 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_accessibility.hxx"

#ifndef ACCESSIBILITY_HELPER_FACTORY_HXX
#include <accessibility/helper/acc_factory.hxx>
#endif

#ifndef _TOOLKIT_AWT_VCLXWINDOWS_HXX
#include <toolkit/awt/vclxwindows.hxx>
#endif

#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLEBUTTON_HXX
#include <accessibility/standard/vclxaccessiblebutton.hxx>
#endif
#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLECHECKBOX_HXX
#include <accessibility/standard/vclxaccessiblecheckbox.hxx>
#endif
#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLEDROPDOWCOMBOBOX_HXX
#include <accessibility/standard/vclxaccessibledropdowncombobox.hxx>
#endif
#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLECOMBOBOX_HXX
#include <accessibility/standard/vclxaccessiblecombobox.hxx>
#endif
#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLEDROPDOWLISTBOX_HXX
#include <accessibility/standard/vclxaccessibledropdownlistbox.hxx>
#endif
#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLEEDIT_HXX
#include <accessibility/standard/vclxaccessibleedit.hxx>
#endif
#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLEFIXEDHYPERLINK_HXX
#include <accessibility/standard/vclxaccessiblefixedhyperlink.hxx>
#endif
#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLEFIXEDTEXT_HXX
#include <accessibility/standard/vclxaccessiblefixedtext.hxx>
#endif
#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLELISTBOX_HXX
#include <accessibility/standard/vclxaccessiblelistbox.hxx>
#endif
#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLEMENU_HXX
#include <accessibility/standard/vclxaccessiblemenu.hxx>
#endif
#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLERADIOBUTTON_HXX
#include <accessibility/standard/vclxaccessibleradiobutton.hxx>
#endif
#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLESCROLLBAR_HXX
#include <accessibility/standard/vclxaccessiblescrollbar.hxx>
#endif
#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLETEXTCOMPONENT_HXX
#include <accessibility/standard/vclxaccessibletextcomponent.hxx>
#endif
#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLETOOLBOX_HXX
#include <accessibility/standard/vclxaccessibletoolbox.hxx>
#endif
#ifndef _TOOLKIT_AWT_VCLXACCESSIBLECOMPONENT_HXX_
#include <toolkit/awt/vclxaccessiblecomponent.hxx>
#endif
#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLESTATUSBAR_HXX
#include <accessibility/standard/vclxaccessiblestatusbar.hxx>
#endif
#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLETABCONTROL_HXX
#include <accessibility/standard/vclxaccessibletabcontrol.hxx>
#endif
#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLETABPAGEWINDOW_HXX
#include <accessibility/standard/vclxaccessibletabpagewindow.hxx>
#endif
#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLEMENUBAR_HXX
#include <accessibility/standard/vclxaccessiblemenubar.hxx>
#endif
#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLEPOPUPMENU_HXX
#include <accessibility/standard/vclxaccessiblepopupmenu.hxx>
#endif

#ifndef ACCESSIBILITY_EXT_ACCESSIBLETABLISTBOX_HXX
#include <accessibility/extended/accessibletablistbox.hxx>
#endif
#ifndef ACCESSIBILITY_EXT_ACCESSIBLEBROWSEBOX_HXX
#include <accessibility/extended/AccessibleBrowseBox.hxx>
#endif
#ifndef ACCESSIBILITY_EXT_ACCESSIBLEICONCHOICECTRL_HXX_
#include <accessibility/extended/accessibleiconchoicectrl.hxx>
#endif
#ifndef ACCESSIBILITY_EXT_ACCESSIBLETABBAR_HXX_
#include <accessibility/extended/accessibletabbar.hxx>
#endif
#ifndef ACCESSIBILITY_EXT_ACCESSIBLELISTBOX_HXX_
#include <accessibility/extended/accessiblelistbox.hxx>
#endif
#ifndef ACCESSIBILITY_EXT_ACCESSIBLEBROWSEBOXHEADERBAR_HXX
#include <accessibility/extended/AccessibleBrowseBoxHeaderBar.hxx>
#endif
#ifndef INCLUDED_ACCESSIBILITY_TEXTWINDOWACCESSIBILITY_HXX
#include <accessibility/extended/textwindowaccessibility.hxx>
#endif
#ifndef ACCESSIBILITY_EXT_ACCESSIBILEBROWSEBOXTABLECELL_HXX
#include <accessibility/extended/AccessibleBrowseBoxTableCell.hxx>
#endif
#ifndef ACCESSIBILITY_EXT_ACCESSIBLEBROWSEBOXHEADERCELL_HXX
#include <accessibility/extended/AccessibleBrowseBoxHeaderCell.hxx>
#endif
#ifndef ACCESSIBILITY_EXT_ACCESSIBLEBROWSEBOXCHECKBOXCELL_HXX
#include <accessibility/extended/AccessibleBrowseBoxCheckBoxCell.hxx>
#endif
#ifndef ACCESSIBILITY_EXT_ACCESSIBILEEDITBROWSEBOXTABLECELL_HXX
#include <accessibility/extended/accessibleeditbrowseboxcell.hxx>
#endif

#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _SV_COMBOBOX_HXX
#include <vcl/combobox.hxx>
#endif

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
