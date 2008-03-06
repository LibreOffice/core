/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: propbrw.cxx,v $
 *
 *  $Revision: 1.28 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 19:14:29 $
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
#include "precompiled_basctl.hxx"


#ifndef _BASCTL_PROPBRW_HXX
#include "propbrw.hxx"
#endif

#ifndef _BASCTL_DLGEDOBJ_HXX
#include "dlgedobj.hxx"
#endif

#include "basidesh.hxx"

#ifndef _IDERID_HXX
#include <iderid.hxx>
#endif

#ifndef _BASCTL_DLGRESID_HRC
#include <dlgresid.hrc>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef TOOLS_DIAGNOSE_EX_H
#include <tools/diagnose_ex.h>
#endif

#ifndef _SFX_BINDINGS_HXX
#include <sfx2/bindings.hxx>
#endif

#ifndef _SFX_CHILDWIN_HXX
#include <sfx2/childwin.hxx>
#endif

#ifndef _SFX_OBJITEM_HXX
#include <sfx2/objitem.hxx>
#endif

#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif

#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif

#ifndef _VCL_STDTEXT_HXX
#include <vcl/stdtext.hxx>
#endif

#ifndef _SVDVIEW_HXX //autogen
#include <svx/svdview.hxx>
#endif

#ifndef _SVDOGRP_HXX
#include <svx/svdogrp.hxx>
#endif

#ifndef _SVDPAGE_HXX
#include <svx/svdpage.hxx>
#endif

#ifndef _SVDITER_HXX
#include <svx/svditer.hxx>
#endif

#ifndef _SFXVIEWSH_HXX //autogen
#include <sfx2/viewsh.hxx>
#endif

#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/unohlp.hxx>
#endif

#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif

#ifndef _COMPHELPER_COMPOSEDPROPS_HXX_
#include <comphelper/composedprops.hxx>
#endif

#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif

#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_POSSIZE_HPP_
#include <com/sun/star/awt/PosSize.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_INSPECTION_XOBJECTINSPECTOR_HPP_
#include <com/sun/star/inspection/XObjectInspector.hpp>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _CPPUHELPER_COMPONENT_CONTEXT_HXX_
#include <cppuhelper/component_context.hxx>
#endif

#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::comphelper;

//============================================================================
// PropBrwMgr
//============================================================================

SFX_IMPL_FLOATINGWINDOW(PropBrwMgr, SID_SHOW_PROPERTYBROWSER)

//----------------------------------------------------------------------------

PropBrwMgr::PropBrwMgr( Window* _pParent, sal_uInt16 nId,
                        SfxBindings *pBindings, SfxChildWinInfo* pInfo)
              :SfxChildWindow( _pParent, nId )
{
    // set current selection
    SfxViewShell* pShell = SfxViewShell::Current();
    pWindow = new PropBrw(
        ::comphelper::getProcessServiceFactory(),
        pBindings,
        this,
        _pParent,
        pShell ? pShell->GetCurrentDocument() : Reference< XModel >()
        );

    eChildAlignment = SFX_ALIGN_NOALIGNMENT;
    ((SfxFloatingWindow*)pWindow)->Initialize( pInfo );

    ((PropBrw*)pWindow)->Update( pShell );
}

//----------------------------------------------------------------------------
void PropBrw::Update( const SfxViewShell* _pShell )
{
    const BasicIDEShell* pBasicIDEShell = dynamic_cast< const BasicIDEShell* >( _pShell );
    OSL_ENSURE( pBasicIDEShell || !_pShell, "PropBrw::Update: invalid shell!" );
    if ( pBasicIDEShell )
    {
        ImplUpdate( pBasicIDEShell->GetCurrentDocument(), pBasicIDEShell->GetCurDlgView() );
    }
    else if ( _pShell )
    {
        ImplUpdate( NULL, _pShell->GetDrawView() );
    }
    else
    {
        ImplUpdate( NULL, NULL );
    }
}

//----------------------------------------------------------------------------

const long STD_WIN_SIZE_X = 300;
const long STD_WIN_SIZE_Y = 350;

const long STD_MIN_SIZE_X = 250;
const long STD_MIN_SIZE_Y = 250;

const long STD_WIN_POS_X = 50;
const long STD_WIN_POS_Y = 50;
const long WIN_BORDER = 2;
const long MIN_WIN_SIZE_X = 50;
const long MIN_WIN_SIZE_Y = 50;

//----------------------------------------------------------------------------

//============================================================================
// PropBrw
//============================================================================

DBG_NAME(PropBrw)

//----------------------------------------------------------------------------

PropBrw::PropBrw( const Reference< XMultiServiceFactory >& _xORB, SfxBindings* _pBindings, PropBrwMgr* _pMgr, Window* _pParent,
            const Reference< XModel >& _rxContextDocument )
    :SfxFloatingWindow( _pBindings, _pMgr, _pParent, WinBits( WB_STDMODELESS | WB_SIZEABLE | WB_3DLOOK | WB_ROLLABLE ) )
    ,m_bInitialStateChange(sal_True)
    ,m_xORB(_xORB)
    ,m_xContextDocument( _rxContextDocument )
    ,pView( NULL )
{
    DBG_CTOR(PropBrw,NULL);

    Size aPropWinSize(STD_WIN_SIZE_X,STD_WIN_SIZE_Y);
    SetMinOutputSizePixel(Size(STD_MIN_SIZE_X,STD_MIN_SIZE_Y));
    SetOutputSizePixel(aPropWinSize);

    try
    {
        // create a frame wrapper for myself
        m_xMeAsFrame = Reference< XFrame >(m_xORB->createInstance(::rtl::OUString::createFromAscii("com.sun.star.frame.Frame")), UNO_QUERY);
        if (m_xMeAsFrame.is())
        {
            m_xMeAsFrame->initialize( VCLUnoHelper::GetInterface ( this ) );
            m_xMeAsFrame->setName(::rtl::OUString::createFromAscii("form property browser"));  // change name!
        }
    }
    catch (Exception&)
    {
        DBG_ERROR("PropBrw::PropBrw: could not create/initialize my frame!");
        m_xMeAsFrame.clear();
    }

    ImplReCreateController();
}

//----------------------------------------------------------------------------

void PropBrw::ImplReCreateController()
{
    OSL_PRECOND( m_xMeAsFrame.is(), "PropBrw::ImplCreateController: no frame for myself!" );
    if ( !m_xMeAsFrame.is() )
        return;

    if ( m_xBrowserController.is() )
        ImplDestroyController();

    try
    {
        Reference< XPropertySet > xFactoryProperties( m_xORB, UNO_QUERY_THROW );
        Reference< XComponentContext > xOwnContext(
            xFactoryProperties->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DefaultContext" ) ) ),
            UNO_QUERY_THROW );

        // a ComponentContext for the
        ::cppu::ContextEntry_Init aHandlerContextInfo[] =
        {
            ::cppu::ContextEntry_Init( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DialogParentWindow" ) ), makeAny( VCLUnoHelper::GetInterface ( this ) ) ),
            ::cppu::ContextEntry_Init( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ContextDocument" ) ), makeAny( m_xContextDocument ) )
        };
        Reference< XComponentContext > xInspectorContext(
            ::cppu::createComponentContext( aHandlerContextInfo, sizeof( aHandlerContextInfo ) / sizeof( aHandlerContextInfo[0] ),
            xOwnContext ) );

        // create a property browser controller
        Reference< XMultiComponentFactory > xFactory( xInspectorContext->getServiceManager(), UNO_QUERY_THROW );
        static const ::rtl::OUString s_sControllerServiceName = ::rtl::OUString::createFromAscii("com.sun.star.awt.PropertyBrowserController");
        m_xBrowserController = Reference< XPropertySet >(
            xFactory->createInstanceWithContext( s_sControllerServiceName, xInspectorContext ), UNO_QUERY
        );
        if ( !m_xBrowserController.is() )
        {
            ShowServiceNotAvailableError( GetParent(), s_sControllerServiceName, sal_True );
        }
        else
        {
            Reference< XController > xAsXController( m_xBrowserController, UNO_QUERY );
            DBG_ASSERT(xAsXController.is(), "PropBrw::PropBrw: invalid controller object!");
            if (!xAsXController.is())
            {
                ::comphelper::disposeComponent(m_xBrowserController);
                m_xBrowserController.clear();
            }
            else
            {
                xAsXController->attachFrame(m_xMeAsFrame);
                m_xBrowserComponentWindow = m_xMeAsFrame->getComponentWindow();
                DBG_ASSERT(m_xBrowserComponentWindow.is(), "PropBrw::PropBrw: attached the controller, but have no component window!");
            }
        }

        Point aPropWinPos = Point( WIN_BORDER, WIN_BORDER );
        Size  aPropWinSize(STD_WIN_SIZE_X,STD_WIN_SIZE_Y);
        aPropWinSize.Width() -= (2*WIN_BORDER);
        aPropWinSize.Height() -= (2*WIN_BORDER);

        if ( m_xBrowserComponentWindow.is() )
        {
            m_xBrowserComponentWindow->setPosSize(aPropWinPos.X(), aPropWinPos.Y(), aPropWinSize.Width(), aPropWinSize.Height(),
                ::com::sun::star::awt::PosSize::WIDTH | ::com::sun::star::awt::PosSize::HEIGHT |
                ::com::sun::star::awt::PosSize::X | ::com::sun::star::awt::PosSize::Y);
            m_xBrowserComponentWindow->setVisible(sal_True);
        }
    }
    catch (Exception&)
    {
        DBG_ERROR("PropBrw::PropBrw: could not create/initialize the browser controller!");
        try
        {
            ::comphelper::disposeComponent(m_xBrowserController);
            ::comphelper::disposeComponent(m_xBrowserComponentWindow);
        }
        catch(Exception&)
        {
        }

        m_xBrowserController.clear();
        m_xBrowserComponentWindow.clear();
    }

    Resize();
}

//----------------------------------------------------------------------------

PropBrw::~PropBrw()
{
    if ( m_xBrowserController.is() )
        ImplDestroyController();

    DBG_DTOR(PropBrw,NULL);
}

//----------------------------------------------------------------------------

void PropBrw::ImplDestroyController()
{
    implSetNewObject( Reference< XPropertySet >() );

    if ( m_xMeAsFrame.is() )
        m_xMeAsFrame->setComponent( NULL, NULL );

    Reference< XController > xAsXController( m_xBrowserController, UNO_QUERY );
    if ( xAsXController.is() )
        xAsXController->attachFrame( NULL );

    try
    {
        ::comphelper::disposeComponent( m_xBrowserController );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    m_xBrowserController.clear();
}

//----------------------------------------------------------------------------

sal_Bool PropBrw::Close()
{
    ImplDestroyController();

    if( IsRollUp() )
        RollDown();

    sal_Bool bClose = SfxFloatingWindow::Close();

    return bClose;
}

//----------------------------------------------------------------------------
Sequence< Reference< XInterface > >
    PropBrw::CreateMultiSelectionSequence( const SdrMarkList& _rMarkList )
{
    Sequence< Reference< XInterface > > aSeq;
    InterfaceArray aInterfaces;

    sal_uInt32 nMarkCount = _rMarkList.GetMarkCount();
    for( sal_uInt32 i = 0 ; i < nMarkCount ; i++ )
    {
        SdrObject* pCurrent = _rMarkList.GetMark(i)->GetMarkedSdrObj();

        SdrObjListIter* pGroupIterator = NULL;
        if (pCurrent->IsGroupObject())
        {
            pGroupIterator = new SdrObjListIter(*pCurrent->GetSubList());
            pCurrent = pGroupIterator->IsMore() ? pGroupIterator->Next() : NULL;
        }

        while (pCurrent)
        {
            DlgEdObj* pDlgEdObj = PTR_CAST(DlgEdObj, pCurrent);
            if (pDlgEdObj)
            {
                Reference< XInterface > xControlInterface(pDlgEdObj->GetUnoControlModel(), UNO_QUERY);
                if (xControlInterface.is())
                    aInterfaces.push_back(xControlInterface);
            }

            // next element
            pCurrent = pGroupIterator && pGroupIterator->IsMore() ? pGroupIterator->Next() : NULL;
        }
        if (pGroupIterator)
            delete pGroupIterator;
    }

    sal_Int32 nCount = aInterfaces.size();
    aSeq.realloc( nCount );
    Reference< XInterface >* pInterfaces = aSeq.getArray();
    for( sal_Int32 i = 0 ; i < nCount ; i++ )
        pInterfaces[i] = aInterfaces[i];

    return aSeq;
}

//----------------------------------------------------------------------------
void PropBrw::implSetNewObjectSequence
    ( const Sequence< Reference< XInterface > >& _rObjectSeq )
{
    Reference< inspection::XObjectInspector > xObjectInspector(m_xBrowserController, UNO_QUERY);
    if ( xObjectInspector.is() )
    {
        xObjectInspector->inspect( _rObjectSeq );

        ::rtl::OUString aText = ::rtl::OUString(String(IDEResId(RID_STR_BRWTITLE_PROPERTIES)));
        aText += ::rtl::OUString(String(IDEResId(RID_STR_BRWTITLE_MULTISELECT)));
        SetText( aText );
    }
}

//----------------------------------------------------------------------------

void PropBrw::implSetNewObject( const Reference< XPropertySet >& _rxObject )
{
    if ( m_xBrowserController.is() )
    {
        m_xBrowserController->setPropertyValue(
            ::rtl::OUString::createFromAscii( "IntrospectedObject" ),
            makeAny( _rxObject )
        );

        // set the new title according to the selected object
        SetText( GetHeadlineName( _rxObject ) );
    }
}

//----------------------------------------------------------------------------

::rtl::OUString PropBrw::GetHeadlineName( const Reference< XPropertySet >& _rxObject )
{
    ::rtl::OUString aName;
    Reference< lang::XServiceInfo > xServiceInfo( _rxObject, UNO_QUERY );

    if (xServiceInfo.is())    // single selection
    {
        sal_uInt16 nResId = 0;
        aName = ::rtl::OUString(String(IDEResId(RID_STR_BRWTITLE_PROPERTIES)));

        if ( xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlDialogModel" ) ) ) )
        {
            nResId = RID_STR_CLASS_DIALOG;
        }
        else if ( xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlButtonModel" ) ) ) )
        {
            nResId = RID_STR_CLASS_BUTTON;
        }
        else if ( xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlRadioButtonModel" ) ) ) )
        {
            nResId = RID_STR_CLASS_RADIOBUTTON;
        }
        else if ( xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlCheckBoxModel" ) ) ) )
        {
            nResId = RID_STR_CLASS_CHECKBOX;
        }
        else if ( xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlListBoxModel" ) ) ) )
        {
            nResId = RID_STR_CLASS_LISTBOX;
        }
        else if ( xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlComboBoxModel" ) ) ) )
        {
            nResId = RID_STR_CLASS_COMBOBOX;
        }
        else if ( xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlGroupBoxModel" ) ) ) )
        {
            nResId = RID_STR_CLASS_GROUPBOX;
        }
        else if ( xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlEditModel" ) ) ) )
        {
            nResId = RID_STR_CLASS_EDIT;
        }
        else if ( xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlFixedTextModel" ) ) ) )
        {
            nResId = RID_STR_CLASS_FIXEDTEXT;
        }
        else if ( xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlImageControlModel" ) ) ) )
        {
            nResId = RID_STR_CLASS_IMAGECONTROL;
        }
        else if ( xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlProgressBarModel" ) ) ) )
        {
            nResId = RID_STR_CLASS_PROGRESSBAR;
        }
        else if ( xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlScrollBarModel" ) ) ) )
        {
            nResId = RID_STR_CLASS_SCROLLBAR;
        }
        else if ( xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlFixedLineModel" ) ) ) )
        {
            nResId = RID_STR_CLASS_FIXEDLINE;
        }
        else if ( xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlDateFieldModel" ) ) ) )
        {
            nResId = RID_STR_CLASS_DATEFIELD;
        }
        else if ( xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlTimeFieldModel" ) ) ) )
        {
            nResId = RID_STR_CLASS_TIMEFIELD;
        }
        else if ( xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlNumericFieldModel" ) ) ) )
        {
            nResId = RID_STR_CLASS_NUMERICFIELD;
        }
        else if ( xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlCurrencyFieldModel" ) ) ) )
        {
            nResId = RID_STR_CLASS_CURRENCYFIELD;
        }
        else if ( xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlFormattedFieldModel" ) ) ) )
        {
            nResId = RID_STR_CLASS_FORMATTEDFIELD;
        }
        else if ( xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlPatternFieldModel" ) ) ) )
        {
            nResId = RID_STR_CLASS_PATTERNFIELD;
        }
        else if ( xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlFileControlModel" ) ) ) )
        {
            nResId = RID_STR_CLASS_FILECONTROL;
        }
        else if ( xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.tree.TreeControlModel" ) ) ) )
        {
            nResId = RID_STR_CLASS_TREECONTROL;
        }
        else
        {
            nResId = RID_STR_CLASS_CONTROL;
        }

        if (nResId)
        {
            aName += ::rtl::OUString( String(IDEResId(nResId)) );
        }
    }
    else if (!_rxObject.is())    // no properties
    {
        aName = ::rtl::OUString(String(IDEResId(RID_STR_BRWTITLE_NO_PROPERTIES)));
    }
    // #i73075 Handled in implSetNewObjectSequence
    //else    // multiselection
    //{
    //  aName = ::rtl::OUString(String(IDEResId(RID_STR_BRWTITLE_PROPERTIES)));
    //  aName += ::rtl::OUString(String(IDEResId(RID_STR_BRWTITLE_MULTISELECT)));
    //}

    return aName;
}

//----------------------------------------------------------------------------

void PropBrw::FillInfo( SfxChildWinInfo& rInfo ) const
{
    rInfo.bVisible = sal_False;
}

//----------------------------------------------------------------------------

void PropBrw::Resize()
{
    SfxFloatingWindow::Resize();

    // adjust size
    Size aSize_ = GetOutputSizePixel();
    Size aPropWinSize( aSize_ );
    aPropWinSize.Width() -= (2*WIN_BORDER);
    aPropWinSize.Height() -= (2*WIN_BORDER);

    if (m_xBrowserComponentWindow.is())
    {
        m_xBrowserComponentWindow->setPosSize(0, 0, aPropWinSize.Width(), aPropWinSize.Height(),
            ::com::sun::star::awt::PosSize::WIDTH | ::com::sun::star::awt::PosSize::HEIGHT);
    }
}

//----------------------------------------------------------------------------

void PropBrw::ImplUpdate( const Reference< XModel >& _rxContextDocument, SdrView* pNewView )
{
    Reference< XModel > xContextDocument( _rxContextDocument );

    // if we should simply "empty" ourself, assume the context document didn't change
    if ( !pNewView )
    {
        OSL_ENSURE( !_rxContextDocument.is(), "PropBrw::ImplUpdate: no view, but a document?!" );
        xContextDocument = m_xContextDocument;
    }

    if ( xContextDocument != m_xContextDocument )
    {
        m_xContextDocument = xContextDocument;
        ImplReCreateController();
    }

    try
    {
        if ( pView )
        {
            EndListening( *(pView->GetModel()) );
            pView = NULL;
        }

        if ( !pNewView )
            return;

        pView = pNewView;

        // set focus on initialization
        if ( m_bInitialStateChange )
        {
            if ( m_xBrowserComponentWindow.is() )
                m_xBrowserComponentWindow->setFocus();
            m_bInitialStateChange = sal_False;
        }

        const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
        sal_uInt32 nMarkCount = rMarkList.GetMarkCount();

        if ( nMarkCount == 0 )
        {
            EndListening( *(pView->GetModel()) );
            pView = NULL;
            implSetNewObject( NULL );
            return;
        }

        Reference< XPropertySet > xNewObject;
        Sequence< Reference< XInterface > > aNewObjects;
        if ( nMarkCount == 1 )
        {
            DlgEdObj* pDlgEdObj = PTR_CAST( DlgEdObj, rMarkList.GetMark(0)->GetMarkedSdrObj() );
            if ( pDlgEdObj )
            {
                if ( pDlgEdObj->IsGroupObject() ) // group object
                    aNewObjects = CreateMultiSelectionSequence( rMarkList );
                else // single selection
                    xNewObject = xNewObject.query( pDlgEdObj->GetUnoControlModel() );
            }
        }
        else if ( nMarkCount > 1 ) // multiple selection
        {
            aNewObjects = CreateMultiSelectionSequence( rMarkList );
        }

        if ( aNewObjects.getLength() )
            implSetNewObjectSequence( aNewObjects );
        else
            implSetNewObject( xNewObject );

        StartListening( *(pView->GetModel()) );
    }
    catch ( const PropertyVetoException& ) { /* silence */ }
    catch ( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

//----------------------------------------------------------------------------
