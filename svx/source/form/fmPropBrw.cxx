/*************************************************************************
 *
 *  $RCSfile: fmPropBrw.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-11 17:05:00 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
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
#ifndef SVX_FMPROPBRW_HXX
#include "fmPropBrw.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
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
#include "svxids.hrc"
#endif
#ifndef _SVX_FMSHELL_HXX
#include "fmshell.hxx"
#endif
#ifndef _SVX_FMSHIMP_HXX
#include "fmshimp.hxx"
#endif
#ifndef _SVX_FMPROP_HRC
#include "fmprop.hrc"
#endif
#ifndef _SVX_FMHELP_HRC
#include "fmhelp.hrc"
#endif
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/unohlp.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_POSSIZE_HPP_
#include <com/sun/star/awt/PosSize.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORM_HPP_
#include <com/sun/star/form/XForm.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_FORMCOMPONENTTYPE_HPP_
#include <com/sun/star/form/FormComponentType.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XLAYOUTCONSTRAINS_HPP_
#include <com/sun/star/awt/XLayoutConstrains.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCONTROLCONTAINER_HPP_
#include <com/sun/star/awt/XControlContainer.hpp>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif
#ifndef _SVX_DIALMGR_HXX //autogen
#include "dialmgr.hxx"
#endif
#ifndef _SVX_FMRESIDS_HRC
#include "fmresids.hrc"
#endif
#ifndef _SVX_FMSERVS_HXX
#include "fmservs.hxx"
#endif
#ifndef _SVDPAGV_HXX
#include "svdpagv.hxx"
#endif
#ifndef _VCL_STDTEXT_HXX
#include <vcl/stdtext.hxx>
#endif

#ifndef _SFXDISPATCH_HXX
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

/*************************************************************************/
//========================================================================
//= FmPropBrwMgr
//========================================================================
//-----------------------------------------------------------------------
SFX_IMPL_FLOATINGWINDOW(FmPropBrwMgr, SID_FM_SHOW_PROPERTIES)

//-----------------------------------------------------------------------
FmPropBrwMgr::FmPropBrwMgr( Window *pParent, sal_uInt16 nId,
                            SfxBindings *pBindings, SfxChildWinInfo* pInfo)
              :SfxChildWindow(pParent, nId)
{
    // my UNO representation
    m_xUnoRepresentation = VCLUnoHelper::CreateControlContainer(pParent);

    pWindow = new FmPropBrw( ::comphelper::getProcessServiceFactory(), pBindings, this, pParent, pInfo );
    eChildAlignment = SFX_ALIGN_NOALIGNMENT;
    ((SfxFloatingWindow*)pWindow)->Initialize( pInfo );
}

//========================================================================
//========================================================================
const long STD_WIN_SIZE_X = 300;
const long STD_WIN_SIZE_Y = 350;

const long STD_MIN_SIZE_X = 250;
const long STD_MIN_SIZE_Y = 250;

const long STD_WIN_POS_X = 50;
const long STD_WIN_POS_Y = 50;
const long WIN_BORDER = 2;
const long MIN_WIN_SIZE_X = 50;
const long MIN_WIN_SIZE_Y = 50;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::svxform;

::rtl::OUString GetUIHeadlineName(sal_Int16 nClassId, const Any& aUnoObj)
{
    sal_uInt16 nClassNameResourceId = 0;

    switch ( nClassId )
    {
        case FormComponentType::TEXTFIELD:
        {
            Reference< XInterface >  xIFace;
            aUnoObj >>= xIFace;
            nClassNameResourceId = RID_STR_PROPTITLE_EDIT;
            if (xIFace.is())
            {   // we have a chance to check if it's a formatted field model
                Reference< XServiceInfo >  xInfo(xIFace, UNO_QUERY);
                if (xInfo.is() && (xInfo->supportsService(FM_SUN_COMPONENT_FORMATTEDFIELD)))
                    nClassNameResourceId = RID_STR_PROPTITLE_FORMATTED;
                else if (!xInfo.is())
                {
                    // couldn't distinguish between formatted and edit with the service name, so try with the properties
                    Reference< XPropertySet >  xProps(xIFace, UNO_QUERY);
                    if (xProps.is())
                    {
                        Reference< XPropertySetInfo >  xPropsInfo = xProps->getPropertySetInfo();
                        if (xPropsInfo.is() && xPropsInfo->hasPropertyByName(FM_PROP_FORMATSSUPPLIER))
                            nClassNameResourceId = RID_STR_PROPTITLE_FORMATTED;
                    }
                }
            }
        }
        break;

        case FormComponentType::COMMANDBUTTON:
            nClassNameResourceId = RID_STR_PROPTITLE_PUSHBUTTON; break;
        case FormComponentType::RADIOBUTTON:
            nClassNameResourceId = RID_STR_PROPTITLE_RADIOBUTTON; break;
        case FormComponentType::CHECKBOX:
            nClassNameResourceId = RID_STR_PROPTITLE_CHECKBOX; break;
        case FormComponentType::LISTBOX:
            nClassNameResourceId = RID_STR_PROPTITLE_LISTBOX; break;
        case FormComponentType::COMBOBOX:
            nClassNameResourceId = RID_STR_PROPTITLE_COMBOBOX; break;
        case FormComponentType::GROUPBOX:
            nClassNameResourceId = RID_STR_PROPTITLE_GROUPBOX; break;
        case FormComponentType::IMAGEBUTTON:
            nClassNameResourceId = RID_STR_PROPTITLE_IMAGEBUTTON; break;
        case FormComponentType::FIXEDTEXT:
            nClassNameResourceId = RID_STR_PROPTITLE_FIXEDTEXT; break;
        case FormComponentType::GRIDCONTROL:
            nClassNameResourceId = RID_STR_PROPTITLE_DBGRID; break;
        case FormComponentType::FILECONTROL:
            nClassNameResourceId = RID_STR_PROPTITLE_FILECONTROL; break;
        case FormComponentType::DATEFIELD:
            nClassNameResourceId = RID_STR_PROPTITLE_DATEFIELD; break;
        case FormComponentType::TIMEFIELD:
            nClassNameResourceId = RID_STR_PROPTITLE_TIMEFIELD; break;
        case FormComponentType::NUMERICFIELD:
            nClassNameResourceId = RID_STR_PROPTITLE_NUMERICFIELD; break;
        case FormComponentType::CURRENCYFIELD:
            nClassNameResourceId = RID_STR_PROPTITLE_CURRENCYFIELD; break;
        case FormComponentType::PATTERNFIELD:
            nClassNameResourceId = RID_STR_PROPTITLE_PATTERNFIELD; break;
        case FormComponentType::IMAGECONTROL:
            nClassNameResourceId = RID_STR_PROPTITLE_IMAGECONTROL; break;
        case FormComponentType::HIDDENCONTROL:
            nClassNameResourceId = RID_STR_HIDDEN_CLASSNAME; break;
        case FormComponentType::SCROLLBAR:
            nClassNameResourceId = RID_STR_PROPTITLE_SCROLLBAR; break;
        case FormComponentType::SPINBUTTON:
            nClassNameResourceId = RID_STR_PROPTITLE_SPINBUTTON; break;
        case FormComponentType::NAVIGATIONBAR:
            nClassNameResourceId = RID_STR_PROPTITLE_NAVBAR; break;
        case FormComponentType::CONTROL:
        default:
            nClassNameResourceId = RID_STR_CONTROL_CLASSNAME; break;
    }

    if ( !nClassNameResourceId )
        return ::rtl::OUString();

    return ::rtl::OUString( SVX_RES( nClassNameResourceId ) );
}

//========================================================================
// class FmPropBrw
//========================================================================
DBG_NAME(FmPropBrw);
//------------------------------------------------------------------------
FmPropBrw::FmPropBrw( const Reference< XMultiServiceFactory >& _xORB, SfxBindings* pBindings,
            SfxChildWindow* pMgr, Window* pParent, const SfxChildWinInfo* _pInfo )
    :SfxFloatingWindow(pBindings, pMgr, pParent,WinBits(WB_STDMODELESS|WB_SIZEABLE|WB_3DLOOK|WB_ROLLABLE))
    ,SfxControllerItem(SID_FM_PROPERTY_CONTROL, *pBindings)
    ,m_bInitialStateChange(sal_True)
    ,m_xORB(_xORB)
{
    DBG_CTOR(FmPropBrw,NULL);

    Size aPropWinSize(STD_WIN_SIZE_X,STD_WIN_SIZE_Y);
    SetMinOutputSizePixel(Size(STD_MIN_SIZE_X,STD_MIN_SIZE_Y));
    SetOutputSizePixel(aPropWinSize);
    SetUniqueId(UID_FORMPROPBROWSER_FRAME);

    try
    {
        // create a frame wrapper for myself
        m_xMeAsFrame = Reference< XFrame >(m_xORB->createInstance(::rtl::OUString::createFromAscii("com.sun.star.frame.Frame")), UNO_QUERY);
        if (m_xMeAsFrame.is())
        {
            m_xMeAsFrame->initialize( VCLUnoHelper::GetInterface ( this ) );
            m_xMeAsFrame->setName(::rtl::OUString::createFromAscii("form property browser"));
            if ( pBindings->GetDispatcher() )
            {
                ::com::sun::star::uno::Reference < ::com::sun::star::frame::XFramesSupplier >
                        xSupp ( pBindings->GetDispatcher()->GetFrame()->GetFrame()->GetFrameInterface(), ::com::sun::star::uno::UNO_QUERY );
//                if ( xSupp.is() )
//                    xSupp->getFrames()->append( m_xMeAsFrame );
            }
        }
    }
    catch (Exception&)
    {
        DBG_ERROR("FmPropBrw::FmPropBrw: could not create/initialize my frame!");
        m_xMeAsFrame.clear();
    }

    if (m_xMeAsFrame.is())
    {
        pMgr->SetFrame( m_xMeAsFrame );
        try
        {
            Sequence< Any > aArgs(1);
            aArgs[0] <<= PropertyValue(
                ::rtl::OUString::createFromAscii("ParentWindow"),
                0,
                makeAny(VCLUnoHelper::GetInterface ( this )),
                PropertyState_DIRECT_VALUE
            );
            // create a property browser controller
            static const ::rtl::OUString s_sControllerServiceName = ::rtl::OUString::createFromAscii("com.sun.star.form.PropertyBrowserController");
            m_xBrowserController = Reference< XPropertySet >(
                m_xORB->createInstance(s_sControllerServiceName), UNO_QUERY
            );
            if (!m_xBrowserController.is())
            {
                ShowServiceNotAvailableError(pParent, s_sControllerServiceName, sal_True);
            }
            else
            {
                Reference< XController > xAsXController(m_xBrowserController, UNO_QUERY);
                DBG_ASSERT(xAsXController.is(), "FmPropBrw::FmPropBrw: invalid controller object!");
                if (!xAsXController.is())
                {
                    ::comphelper::disposeComponent(m_xBrowserController);
                    m_xBrowserController.clear();
                }
                else
                {
                    xAsXController->attachFrame(m_xMeAsFrame);
                    m_xBrowserComponentWindow = m_xMeAsFrame->getComponentWindow();
                    DBG_ASSERT(m_xBrowserComponentWindow.is(), "FmPropBrw::FmPropBrw: attached the controller, but have no component window!");
                }
            }
        }
        catch (Exception&)
        {
            DBG_ERROR("FmPropBrw::FmPropBrw: could not create/initialize the browser controller!");
            try
            {
                ::comphelper::disposeComponent(m_xBrowserController);
                ::comphelper::disposeComponent(m_xBrowserComponentWindow);
            }
            catch(Exception&) { }
            m_xBrowserController.clear();
            m_xBrowserComponentWindow.clear();
        }
    }


    if ( m_xBrowserComponentWindow.is() )
        m_xBrowserComponentWindow->setVisible( sal_True );

    if ( _pInfo )
        m_sLastActivePage = _pInfo->aExtraString;
}

//------------------------------------------------------------------------
FmPropBrw::~FmPropBrw()
{
    if (m_xBrowserController.is())
        implDetachController();

    DBG_DTOR(FmPropBrw,NULL);
}

//-----------------------------------------------------------------------
::rtl::OUString FmPropBrw::getCurrentPage() const
{
    ::rtl::OUString sCurrentPage;
    try
    {
        if ( m_xBrowserController.is() )
            m_xBrowserController->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CurrentPage" ) ) ) >>= sCurrentPage;

        if ( !sCurrentPage.getLength() )
            sCurrentPage = m_sLastActivePage;
    }
    catch( const Exception& )
    {
        OSL_ENSURE( sal_False, "FmPropBrw::getCurrentPage: caught an exception while retrieving the current page!" );
    }
    return sCurrentPage;
}

//-----------------------------------------------------------------------
void FmPropBrw::implDetachController()
{
    m_sLastActivePage = getCurrentPage();

    implSetNewObject(Reference< XPropertySet >());
    if (m_xMeAsFrame.is())
        m_xMeAsFrame->setComponent(NULL, NULL);

    // we attached a frame to the controller manually, so we need to manually tell it that it's detached, too
    // 96068 - 09.01.2002 - fs@openoffice.org
    Reference< XController > xAsXController( m_xBrowserController, UNO_QUERY );
    if ( xAsXController.is() )
        xAsXController->attachFrame( NULL );

    m_xBrowserController.clear();
    m_xMeAsFrame.clear();
}

//-----------------------------------------------------------------------
sal_Bool FmPropBrw::Close()
{
    // suspend the controller (it is allowed to veto)
    if ( m_xMeAsFrame.is() )
    {
        try
        {
            Reference< XController > xController( m_xMeAsFrame->getController() );
            if ( xController.is() && !xController->suspend( sal_True ) )
                return sal_False;
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "FmPropBrw::Close: caught an exception while asking the controller!" );
        }
    }

    implDetachController();

    if( IsRollUp() )
        RollDown();

    // remember our bindings: while we're closed, we're deleted, too, so accessing the bindings after this
    // would be deadly
    // 10/19/00 - 79321 - FS
    SfxBindings& rBindings = SfxControllerItem::GetBindings();

    sal_Bool bClose = SfxFloatingWindow::Close();

    if (bClose)
    {
        rBindings.Invalidate(SID_FM_CTL_PROPERTIES);
        rBindings.Invalidate(SID_FM_PROPERTIES);
    }

    return bClose;
}

//-----------------------------------------------------------------------
void FmPropBrw::implSetNewObject(const Reference< XPropertySet >& _rxObject)
{
    if (m_xBrowserController.is())
    {
        try
        {
            m_xBrowserController->setPropertyValue(
                ::rtl::OUString::createFromAscii("IntrospectedObject"),
                makeAny(_rxObject)
            );
        }
        catch( const PropertyVetoException& )
        {
            return;
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "FmPropBrw::implSetNewObject: caught an unexpected exception!" );
            return;
        }

        // set the new title according to the selected object
        String sTitle;
        if  (::comphelper::hasProperty(FM_PROP_CLASSID, _rxObject))
        {
            Any aClassIdValue(_rxObject->getPropertyValue(FM_PROP_CLASSID));
            if (aClassIdValue.hasValue())
            {
                sal_Int16 nClassID = ::comphelper::getINT16(_rxObject->getPropertyValue(FM_PROP_CLASSID));
                sTitle = String(SVX_RES(RID_STR_PROPERTIES_CONTROL));
                sTitle += String(GetUIHeadlineName(nClassID, makeAny(_rxObject)));
            }
        }
        else if (Reference< XForm >(_rxObject, UNO_QUERY).is())
            sTitle = String(SVX_RES(RID_STR_PROPERTIES_FORM));
        else if (!_rxObject.is())
            sTitle = String(SVX_RES(RID_STR_NO_PROPERTIES));
        else if (!::comphelper::hasProperty(FM_PROP_DATASOURCE, _rxObject) || !::comphelper::hasProperty(FM_PROP_NAME, _rxObject))
        {
            // no form component and (no form or no name) -> Multiselection
            sTitle = String(SVX_RES(RID_STR_PROPERTIES_CONTROL));
            sTitle += String(SVX_RES(RID_STR_PROPTITLE_MULTISELECT));
        }

        SetText(sTitle);

        // #95343# ---------------------------------
        Reference< ::com::sun::star::awt::XLayoutConstrains > xLayoutConstrains( m_xBrowserController, UNO_QUERY );
        if( xLayoutConstrains.is() )
        {
            Size aSize;
            ::com::sun::star::awt::Size aMinSize = xLayoutConstrains->getMinimumSize();
            aMinSize.Height += 4;
            aMinSize.Width += 4;
            aSize.setHeight( aMinSize.Height );
            aSize.setWidth( aMinSize.Width );
            SetMinOutputSizePixel( aSize );
            aSize = GetOutputSizePixel();
            sal_Bool bResize = sal_False;
            if( aSize.Width() < aMinSize.Width )
            {
                aSize.setWidth( aMinSize.Width );
                bResize = sal_True;
            }
            if( aSize.Height() < aMinSize.Height )
            {
                aSize.setHeight( aMinSize.Height );
                bResize = sal_True;
            }
            if( bResize )
                SetOutputSizePixel( aSize );
        }
    }
}

//-----------------------------------------------------------------------
void FmPropBrw::FillInfo( SfxChildWinInfo& rInfo ) const
{
    rInfo.bVisible = sal_False;
    rInfo.aExtraString = getCurrentPage();
}

//-----------------------------------------------------------------------
IMPL_LINK( FmPropBrw, OnAsyncGetFocus, void*, NOTINTERESTEDIN )
{
    if (m_xBrowserComponentWindow.is())
        m_xBrowserComponentWindow->setFocus();
    return 0L;
}

//-----------------------------------------------------------------------
void FmPropBrw::StateChanged(sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState)
{
    try
    {
        if (!pState  || SID_FM_PROPERTY_CONTROL != nSID)
            return;

        if (eState >= SFX_ITEM_AVAILABLE)
        {
            FmFormShell* pShell = PTR_CAST(FmFormShell,((SfxObjectItem*)pState)->GetShell());
            Reference< XPropertySet > xObject;
            if ( pShell )
            {
                // is there a selected object?
                xObject = xObject.query( pShell->GetImpl()->getSelObject() );
            }

            // for some functionality, the property browser needs to know the context of the controls
            Reference< awt::XControlContainer > xControlContext;
            if ( pShell && pShell->GetFormView() )
            {
                SdrPageView* pPageView = pShell->GetFormView()->GetPageViewPvNum(0);
                xControlContext = pPageView->GetWindow(0)->GetControlContainerRef();
            }
            try
            {
                if ( m_xBrowserController.is() )
                    m_xBrowserController->setPropertyValue(
                        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ControlContext" ) ),
                        makeAny( xControlContext )
                    );
            }
            catch( const Exception& )
            {
                OSL_ENSURE( sal_False, "FmPropBrw::StateChanged: caught an exception while setting the control context!" );
            }

            // set the new object to inspect
            implSetNewObject( xObject );

            // if this is the first time we're here, some additional things need to be done ...
            if ( m_bInitialStateChange )
            {
                // if we're just newly created, we want to have the focus
                PostUserEvent( LINK( this, FmPropBrw, OnAsyncGetFocus ) );

                // and additionally, we want to show the page which was active during
                // our previous incarnation
                if ( m_sLastActivePage.getLength() )
                {
                    try
                    {
                        if ( m_xBrowserController.is() )
                            m_xBrowserController->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CurrentPage" ) ), makeAny( m_sLastActivePage ) );
                    }
                    catch( const Exception& )
                    {
                        OSL_ENSURE( sal_False, "FmPropBrw::StateChanged: caught an exception while setting the initial page!" );
                    }
                }

                m_bInitialStateChange = sal_False;
            }

        }
        else
        {
            implSetNewObject(Reference< XPropertySet >());
        }
    }
    catch (Exception&)
    {
        DBG_ERROR("FmPropBrw::StateChanged: Exception occured!");
    }

}
