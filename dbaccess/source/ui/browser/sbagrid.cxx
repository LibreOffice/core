/*************************************************************************
 *
 *  $RCSfile: sbagrid.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: fs $ $Date: 2000-11-09 12:50:48 $
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
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
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

#ifndef _SBA_GRID_HXX
#include "sbagrid.hxx"
#endif
//#ifndef _SBA_SBATTRDLG_HXX
//#include "dlgattr.hxx"
//#endif
//#ifndef _DLGSIZE_HXX
//#include "dlgsize.hxx"
//#endif
//#ifndef _SBA_API_HRC
//#include "api.hrc"
//#endif
#ifndef _COM_SUN_STAR_FORM_XLOADABLE_HPP_
#include <com/sun/star/form/XLoadable.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_COMMANDTYPE_HPP_
#include <com/sun/star/sdb/CommandType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XSQLQUERYCOMPOSERFACTORY_HPP_
#include <com/sun/star/sdb/XSQLQueryComposerFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORM_HPP_
#include <com/sun/star/form/XForm.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXCONTAINER_HPP_
#include <com/sun/star/container/XIndexContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_NUMBERFORMAT_HPP_
#include <com/sun/star/util/NumberFormat.hpp>
#endif

#ifndef _COM_SUN_STAR_VIEW_XSELECTIONSUPPLIER_HPP_
#include <com/sun/star/view/XSelectionSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_DATASELECTIONTYPE_HPP_
#include <com/sun/star/form/DataSelectionType.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_TEXTALIGN_HPP_
#include <com/sun/star/awt/TextAlign.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTEXTCOMPONENT_HPP_
#include <com/sun/star/awt/XTextComponent.hpp>
#endif

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif

//#ifndef _SVX_COLRITEM_HXX //autogen
//#include <svx/colritem.hxx>
//#endif
//
//#ifndef _SVX_ALGITEM_HXX //autogen
//#include <svx/algitem.hxx>
//#endif

#ifndef _SV_MULTISEL_HXX //autogen
#include <tools/multisel.hxx>
#endif

#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif

#ifndef _SV_DRAG_HXX //autogen
#include <vcl/drag.hxx>
#endif

#ifndef _NUMUNO_HXX
#include <svtools/numuno.hxx>
#endif

#ifndef _SFXITEMPOOL_HXX //autogen wg. SfxItemInfo
#include <svtools/itempool.hxx>
#endif

#ifndef _SFXITEMSET_HXX //autogen wg. SfxItemSet
#include <svtools/itemset.hxx>
#endif

#ifndef _SV_SYSTEM_HXX //autogen wg. ::com::sun::star::chaos::System
#include <vcl/system.hxx>
#endif

//#ifndef _SVX_FONTITEM_HXX //autogen wg. SvxFontItem
//#include <svx/fontitem.hxx>
//#endif
//
//#ifndef _SVX_FHGTITEM_HXX //autogen wg. SvxFontHeightItem
//#include <svx/fhgtitem.hxx>
//#endif
//
//#ifndef _SVX_WGHTITEM_HXX //autogen wg. SvxWeightItem
//#include <svx/wghtitem.hxx>
//#endif

#ifndef SBA_VCARDEXCHANGE_FORMAT
#define SBA_VCARDEXCHANGE_FORMAT    "+//ISBN 1-887687-00-9::versit::PDI//vCard"
#endif

//#ifndef _SVX_POSTITEM_HXX //autogen wg. SvxPostureItem
//#include <svx/postitem.hxx>
//#endif
//
//#ifndef _SVX_SHDDITEM_HXX //autogen wg. SvxShadowedItem
//#include <svx/shdditem.hxx>
//#endif
//
//#ifndef _SVX_ITEM_HXX //autogen wg. SvxContourItem
//#include <svx/cntritem.hxx>
//#endif
//
//#ifndef _SVX_CRSDITEM_HXX //autogen wg. SvxCrossedOutItem
//#include <svx/crsditem.hxx>
//#endif
//
//#ifndef _SVX_UDLNITEM_HXX //autogen wg. SvxUnderlineItem
//#include <svx/udlnitem.hxx>
//#endif

#ifndef _SFXRNGITEM_HXX
#include <svtools/rngitem.hxx>
#endif

#ifndef _EXCHOBJ_HXX
#include <sfx2/exchobj.hxx>
#endif

#ifndef _SV_WAITOBJ_HXX
#include <vcl/waitobj.hxx>
#endif
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/helper/vclunohelper.hxx>
#endif

#ifndef _ZFORLIST_HXX
#include <svtools/zforlist.hxx>
#endif
#ifndef _CPPUHELPER_QUERYINTERFACE_HXX_
#include <cppuhelper/queryinterface.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _CONNECTIVITY_DATECONVERSION_HXX_
#include <connectivity/DateConversion.hxx>
#endif
#ifndef _RTL_UUID_H_
#include <rtl/uuid.h>
#endif
#ifndef _RTL_MEMORY_H_
#include <rtl/memory.h>
#endif
#ifndef _CPPUHELPER_EXTRACT_HXX_
#include <cppuhelper/extract.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _SV_CLIP_HXX
#include <vcl/clip.hxx>
#endif
#ifndef _SVX_DBEXCH_HRC
#include <svx/dbexch.hrc>
#endif
#ifndef _DBU_RESOURCE_HRC_
#include "dbu_resource.hrc"
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef DBACCESS_UI_BROWSER_ID_HXX
#include "browserids.hxx"
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace dbaui;

/// collect the SBA_DEF_...-items and use them to create a new font based on the given one
::com::sun::star::awt::FontDescriptor   BuildFontFromItems(const SfxItemSet* pAttr, const Font& rBase);

/// translate the given font into SBA_DEF_...-items, put them into the set
void            BuildItemsFromFont(SfxItemSet* pAttr, const ::com::sun::star::awt::FontDescriptor& rFont);

//------------------------------------------------------------------------------
sal_uInt32 RegisterRowExchangeFormatName()
{
    static sal_uInt32 nFormat = 0;
    if (!nFormat)
        nFormat = Clipboard::RegisterFormatName(String::CreateFromAscii(SBA_DATAEXCHANGE_FORMAT));

    return nFormat;
}

//------------------------------------------------------------------------------
sal_uInt32 RegisterColumnExchangeFormatName()
{
    static sal_uInt32 nFormat = 0;
    if (!nFormat)
        nFormat = Clipboard::RegisterFormatName(String::CreateFromAscii(SBA_FIELDEXCHANGE_FORMAT));

    return nFormat;
}

//------------------------------------------------------------------------------
sal_uInt32 RegisterFieldExchangeFormatName()
{
    static sal_uInt32 nFormat = 0;
    if (!nFormat)
        nFormat = Clipboard::RegisterFormatName(String::CreateFromAscii(SBA_FIELDDATAEXCHANGE_FORMAT));

    return nFormat;
}

//------------------------------------------------------------------
String Any2String(const ::com::sun::star::uno::Any& rValue)
{
    String  aString;
    if (rValue.hasValue())
    {
        switch (rValue.getValueTypeClass())
        {
            case ::com::sun::star::uno::TypeClass_BOOLEAN:
                aString  = (char)::comphelper::getBOOL(rValue);
                break;
            case ::com::sun::star::uno::TypeClass_CHAR:
                aString  = ::comphelper::getINT16(rValue);
                break;
            case ::com::sun::star::uno::TypeClass_STRING:
                aString  = (const sal_Unicode*)::comphelper::getString(rValue);
                break;
            case ::com::sun::star::uno::TypeClass_BYTE:
                aString  = (char)::comphelper::getBOOL(rValue);
                break;
            case ::com::sun::star::uno::TypeClass_LONG:
                aString  = ::comphelper::getINT32(rValue);
                break;
            case ::com::sun::star::uno::TypeClass_SHORT:
                aString = (short)::comphelper::getINT16(rValue);
                break;
            case ::com::sun::star::uno::TypeClass_UNSIGNED_SHORT:
                aString = (short)::comphelper::getINT16(rValue);
                break;
            case ::com::sun::star::uno::TypeClass_UNSIGNED_LONG:
                aString = (int)::comphelper::getINT32(rValue);
                break;
            case ::com::sun::star::uno::TypeClass_VOID:
            default:
                ; // leerstring
        }
    }
    return aString;
}

//------------------------------------------------------------------
double Any2Double(const ::com::sun::star::uno::Any& rValue)
{
    double aRet = 0;
    if (rValue.hasValue())
    {
        switch (rValue.getValueTypeClass())
        {
            case ::com::sun::star::uno::TypeClass_BOOLEAN:
                aRet = ::comphelper::getBOOL(rValue);
                break;
            case ::com::sun::star::uno::TypeClass_CHAR:
                aRet = ::comphelper::getBOOL(rValue);
                break;
            case ::com::sun::star::uno::TypeClass_FLOAT:
                aRet  = ::comphelper::getFloat(rValue);
                break;
            case ::com::sun::star::uno::TypeClass_DOUBLE:
                aRet  = ::comphelper::getDouble(rValue);
                break;
            case ::com::sun::star::uno::TypeClass_LONG:
                aRet = ::comphelper::getINT32(rValue);
                break;
            case ::com::sun::star::uno::TypeClass_SHORT:
                aRet = ::comphelper::getINT16(rValue);
                break;
            case ::com::sun::star::uno::TypeClass_UNSIGNED_SHORT:
                aRet = ::comphelper::getINT16(rValue);
                break;
            case ::com::sun::star::uno::TypeClass_BYTE:
                aRet  = ::comphelper::getBOOL(rValue);
                break;
            case ::com::sun::star::uno::TypeClass_UNSIGNED_LONG:
                aRet = ::comphelper::getINT32(rValue);
                break;

            case ::com::sun::star::uno::TypeClass_VOID:
            default:
                // ripped these three routines somewhere ....
                if (::comphelper::isA(rValue,(const ::com::sun::star::util::Date*)0))
                {
                    aRet =  connectivity::DateConversion::toDouble(*static_cast< const ::com::sun::star::util::Date*>(rValue.getValue()));
                }
                else if (::comphelper::isA(rValue,(const ::com::sun::star::util::Time*)0))
                {
                    aRet =  connectivity::DateConversion::toDouble(*static_cast< const ::com::sun::star::util::Time*>(rValue.getValue()));
                }
                else if (::comphelper::isA(rValue,(const ::com::sun::star::util::DateTime*)0))
                {
                    aRet =  connectivity::DateConversion::toDouble(*static_cast< const ::com::sun::star::util::DateTime*>(rValue.getValue()));
                }
                break;
        }
    }
    return aRet;
}

//=======================================================================================
// SbaXGridControl
//=======================================================================================

//------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL SbaXGridControl_CreateInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory) throw( ::com::sun::star::uno::Exception )
{
    return *(new SbaXGridControl(_rxFactory));
}

//------------------------------------------------------------------------------
::rtl::OUString SAL_CALL SbaXGridControl::getImplementationName() throw( ::com::sun::star::uno::RuntimeException )
{
    return getImplementationName_Static();
}

//------------------------------------------------------------------------------
::rtl::OUString SbaXGridControl::getImplementationName_Static() throw( ::com::sun::star::uno::RuntimeException )
{
    return ::rtl::OUString::createFromAscii("com.sun.star.form.SbaXGridControl");
}

//------------------------------------------------------------------------------
::com::sun::star::uno::Sequence< ::rtl::OUString> SbaXGridControl::getSupportedServiceNames_Static(void) throw( ::com::sun::star::uno::RuntimeException )
{
    ::com::sun::star::uno::Sequence< ::rtl::OUString> aSupported(1);
    aSupported.getArray()[0] = ::rtl::OUString::createFromAscii("com.sun.star.form.control.ExtendedGridControl");
    return aSupported;
}

//---------------------------------------------------------------------------------------
SbaXGridControl::SbaXGridControl(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rM)
    : FmXGridControl(_rM)
{
}

//---------------------------------------------------------------------------------------
SbaXGridControl::~SbaXGridControl()
{
}

//---------------------------------------------------------------------------------------
FmXGridPeer* SbaXGridControl::imp_CreatePeer(Window* pParent)
{
    FmXGridPeer* pReturn = new SbaXGridPeer(m_xServiceFactory);

    // translate properties into WinBits
    WinBits nStyle = WB_TABSTOP;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xModelSet(getModel(), ::com::sun::star::uno::UNO_QUERY);
    if (xModelSet.is())
    {
        try
        {
            if (::comphelper::getINT16(xModelSet->getPropertyValue(::rtl::OUString::createFromAscii("Border"))))
                nStyle |= WB_BORDER;
        }
        catch(Exception&)
        {
        }

    }

    pReturn->Create(pParent, nStyle);
    return pReturn;
}

//------------------------------------------------------------------------------
::com::sun::star::uno::Any  SAL_CALL SbaXGridControl::queryInterface(const ::com::sun::star::uno::Type& _rType) throw (::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface(_rType,(::com::sun::star::frame::XDispatch*)this);
    if(aRet.hasValue())
        return aRet;
    return UnoControl::queryInterface(_rType);
}

//---------------------------------------------------------------------------------------
void SAL_CALL SbaXGridControl::createPeer(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit > & rToolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > & rParentPeer) throw( ::com::sun::star::uno::RuntimeException )
{
    FmXGridControl::createPeer(rToolkit, rParentPeer);
    if (0 == m_nPeerCreationLevel)
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >  xDisp(mxPeer, ::com::sun::star::uno::UNO_QUERY);
        for (   StatusMultiplexerArray::iterator aIter = m_aStatusMultiplexer.begin();
                aIter != m_aStatusMultiplexer.end();
                ++aIter)
        {
            if ((*aIter).second && (*aIter).second->getLength())
                xDisp->addStatusListener((*aIter).second, (*aIter).first);
        }
    }
}

//---------------------------------------------------------------------------------------
void SAL_CALL SbaXGridControl::dispatch(const ::com::sun::star::util::URL& aURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArgs) throw( ::com::sun::star::uno::RuntimeException )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >  xDisp(mxPeer, ::com::sun::star::uno::UNO_QUERY);
    if (xDisp.is())
        xDisp->dispatch(aURL, aArgs);
}
//---------------------------------------------------------------------------------------
void SAL_CALL SbaXGridControl::addStatusListener(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener > & xControl, const ::com::sun::star::util::URL& aURL) throw( ::com::sun::star::uno::RuntimeException )
{
    SbaXStatusMultiplexer*& pMultiplexer = m_aStatusMultiplexer[aURL];
    if (!pMultiplexer)
    {
        pMultiplexer = new SbaXStatusMultiplexer(*this,GetMutex());
        pMultiplexer->acquire();
    }

    pMultiplexer->addInterface(xControl);
    if (mxPeer.is() && pMultiplexer->getLength() == 1)
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >  xDisp(mxPeer, ::com::sun::star::uno::UNO_QUERY);
        xDisp->addStatusListener(pMultiplexer, aURL);
    }
}

//---------------------------------------------------------------------------------------
void SAL_CALL SbaXGridControl::removeStatusListener(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener > & xControl, const ::com::sun::star::util::URL& aURL) throw( ::com::sun::star::uno::RuntimeException )
{
    SbaXStatusMultiplexer*& pMultiplexer = m_aStatusMultiplexer[aURL];
    if (!pMultiplexer)
    {
        pMultiplexer = new SbaXStatusMultiplexer(*this,GetMutex());
        pMultiplexer->acquire();
    }

    if (mxPeer.is() && pMultiplexer->getLength() == 1)
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >  xDisp(mxPeer, ::com::sun::star::uno::UNO_QUERY);
        xDisp->removeStatusListener(pMultiplexer, aURL);
    }
    pMultiplexer->removeInterface(xControl);
}

//---------------------------------------------------------------------------------------
void SAL_CALL SbaXGridControl::dispose(void) throw( ::com::sun::star::uno::RuntimeException )
{
    ::com::sun::star::lang::EventObject aEvt;
    aEvt.Source = *this;

    for (   StatusMultiplexerArray::iterator aIter = m_aStatusMultiplexer.begin();
            aIter != m_aStatusMultiplexer.end();
            ++aIter)
    {
        if ((*aIter).second)
        {
            (*aIter).second->disposeAndClear(aEvt);
            (*aIter).second->release();
            (*aIter).second = NULL;
        }
    }

    FmXGridControl::dispose();
}

//=======================================================================================
// SbaXGridPeer
//=======================================================================================

//---------------------------------------------------------------------------------------
SbaXGridPeer::SbaXGridPeer(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rM)
: FmXGridPeer(_rM)
,m_aStatusListeners(m_aMutex)
{
}

//---------------------------------------------------------------------------------------
SbaXGridPeer::~SbaXGridPeer()
{
}

//---------------------------------------------------------------------------------------
void SAL_CALL SbaXGridPeer::dispose(void) throw( ::com::sun::star::uno::RuntimeException )
{
    ::com::sun::star::lang::EventObject aEvt(*this);

    m_aStatusListeners.disposeAndClear(aEvt);

    FmXGridPeer::dispose();
}

//---------------------------------------------------------------------------------------
void SbaXGridPeer::NotifyStatusChanged(const ::com::sun::star::util::URL& aUrl, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener > & xControl)
{
    SbaGridControl* pGrid = (SbaGridControl*) GetWindow();
    if (!pGrid)
        return;

    ::com::sun::star::frame::FeatureStateEvent aEvt;
    aEvt.Source = *this;
    aEvt.IsEnabled = !pGrid->IsReadOnlyDB();

    if (xControl.is())
        xControl->statusChanged(aEvt);
    else
    {
        ::cppu::OInterfaceContainerHelper * pIter = m_aStatusListeners.getContainer(aUrl);

        if (pIter)
        {
            ::cppu::OInterfaceIteratorHelper aListIter(*pIter);
            while (aListIter.hasMoreElements())
                ((::com::sun::star::frame::XStatusListener*)aListIter.next())->statusChanged(aEvt);
        }
    }
}

//------------------------------------------------------------------------------
::com::sun::star::uno::Any  SAL_CALL SbaXGridPeer::queryInterface(const ::com::sun::star::uno::Type& _rType) throw (::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface(_rType,(::com::sun::star::frame::XDispatch*)this);
    if(aRet.hasValue())
        return aRet;
    return FmXGridPeer::queryInterface(_rType);
}

//---------------------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >  SAL_CALL SbaXGridPeer::queryDispatch(const ::com::sun::star::util::URL& aURL, const ::rtl::OUString& aTargetFrameName, sal_Int32 nSearchFlags) throw( ::com::sun::star::uno::RuntimeException )
{
    if  (   (aURL.Complete.equals(::rtl::OUString::createFromAscii(".uno:GridSlots/BrowserAttribs")))
        ||  (aURL.Complete.equals(::rtl::OUString::createFromAscii(".uno:GridSlots/RowHeight")))
        ||  (aURL.Complete.equals(::rtl::OUString::createFromAscii(".uno:GridSlots/ColumnAttribs")))
        ||  (aURL.Complete.equals(::rtl::OUString::createFromAscii(".uno:GridSlots/ColumnWidth")))
        )
    {
        return (::com::sun::star::frame::XDispatch*)this;
    }

    return FmXGridPeer::queryDispatch(aURL, aTargetFrameName, nSearchFlags);
}

//---------------------------------------------------------------------------------------
void SAL_CALL SbaXGridPeer::dispatch(const ::com::sun::star::util::URL& aURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArgs) throw( ::com::sun::star::uno::RuntimeException )
{
    SbaGridControl* pGrid = (SbaGridControl*)GetWindow();
    if (!pGrid)
        return;

    sal_Int16 nColId = -1;
    const ::com::sun::star::beans::PropertyValue* pArgs = aArgs.getConstArray();
    for (sal_uInt16 i=0; i<aArgs.getLength(); ++i, ++pArgs)
    {
        if (pArgs->Name == ::rtl::OUString::createFromAscii("ColumnViewPos"))
        {
            nColId = pGrid->GetColumnIdFromViewPos(::comphelper::getINT16(pArgs->Value));
            break;
        }
        if (pArgs->Name == ::rtl::OUString::createFromAscii("ColumnModelPos"))
        {
            nColId = pGrid->GetColumnIdFromModelPos(::comphelper::getINT16(pArgs->Value));
            break;
        }
        if (pArgs->Name == ::rtl::OUString::createFromAscii("ColumnId"))
        {
            nColId = ::comphelper::getINT16(pArgs->Value);
            break;
        }
    }

    if (aURL.Complete == ::rtl::OUString::createFromAscii(".uno:GridSlots/BrowserAttribs"))
        pGrid->SetBrowserAttrs();
    else if (aURL.Complete == ::rtl::OUString::createFromAscii(".uno:GridSlots/RowHeight"))
        pGrid->SetRowHeight();
    else if (aURL.Complete == ::rtl::OUString::createFromAscii(".uno:GridSlots/ColumnAttribs"))
    {
        DBG_ASSERT(nColId != -1, "SbaXGridPeer::dispatch : invalid parameter !");
        if (nColId != -1)
            return;
        pGrid->SetColAttrs(nColId);
    }
    else if (aURL.Complete == ::rtl::OUString::createFromAscii(".uno:GridSlots/ColumnWidth"))
    {
        DBG_ASSERT(nColId != -1, "SbaXGridPeer::dispatch : invalid parameter !");
        if (nColId != -1)
            return;
        pGrid->SetColWidth(nColId);
    }
}

//---------------------------------------------------------------------------------------
void SAL_CALL SbaXGridPeer::addStatusListener(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener > & xControl, const ::com::sun::star::util::URL& aURL) throw( ::com::sun::star::uno::RuntimeException )
{
    ::cppu::OInterfaceContainerHelper* pCont = m_aStatusListeners.getContainer(aURL);
    if (!pCont)
        m_aStatusListeners.addInterface(aURL,xControl);
    else
        pCont->addInterface(xControl);
    NotifyStatusChanged(aURL, xControl);
}

//---------------------------------------------------------------------------------------
void SAL_CALL SbaXGridPeer::removeStatusListener(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener > & xControl, const ::com::sun::star::util::URL& aURL) throw( ::com::sun::star::uno::RuntimeException )
{
    ::cppu::OInterfaceContainerHelper* pCont = m_aStatusListeners.getContainer(aURL);
    if (!pCont)

    pCont->removeInterface(xControl);
}

//---------------------------------------------------------------------------------------
void SAL_CALL SbaXGridPeer::selectionChanged(const ::com::sun::star::lang::EventObject& aEvent)
{
    FmXGridPeer::selectionChanged(aEvent);

    SbaGridControl* pGrid = (SbaGridControl*) GetWindow();
    if (pGrid)
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >  xColumns = getColumns();
        ::com::sun::star::uno::Reference< ::com::sun::star::view::XSelectionSupplier >  xSelSupplier(aEvent.Source, ::com::sun::star::uno::UNO_QUERY);
        ::com::sun::star::uno::Any aSelection = xSelSupplier->getSelection();
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xSelection;
        if (aSelection.getValueTypeClass() == ::com::sun::star::uno::TypeClass_INTERFACE)
            xSelection = ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > (*(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > *)aSelection.getValue(), ::com::sun::star::uno::UNO_QUERY);
        sal_uInt16 nSelectedCol = (sal_uInt16)-1;
        if (xSelection.is())
        {
            for (sal_uInt16 i = 0; i < xColumns->getCount(); i++)
            {
                ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xCol(*(::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > *)xColumns->getByIndex(i).getValue());
                if (xCol == xSelection)
                {
                    nSelectedCol = i;
                    break;
                }
            }
        }

        // fuer das VCL-Control muessen die Columns 1-basiert sein
        // die Selektion an das VCL-Control weiterreichen, wenn noetig
        if (nSelectedCol != pGrid->GetSelectedColumn())
        {   // (wenn das nicht greift, wurde das selectionChanged implizit von dem Control selber ausgeloest
            if (nSelectedCol != (sal_uInt16)-1)
            {
                pGrid->SelectColumnPos(pGrid->GetViewColumnPos(pGrid->GetColumnIdFromModelPos(nSelectedCol)) + 1, sal_True);
                // SelectColumnPos hat wieder zu einem impliziten ActivateCell gefuehrt
                if (pGrid->IsEditing())
                    pGrid->DeactivateCell();
            }
            else
                pGrid->SetNoSelection();
        }
    }
}
//---------------------------------------------------------------------------------------
const ::com::sun::star::uno::Sequence< sal_Int8 > & SbaXGridPeer::getUnoTunnelId()
{
    static ::com::sun::star::uno::Sequence< sal_Int8 > * pSeq = 0;
    if( !pSeq )
    {
        ::osl::Guard< ::osl::Mutex > aGuard( ::osl::Mutex::getGlobalMutex() );
        if( !pSeq )
        {
            static ::com::sun::star::uno::Sequence< sal_Int8 > aSeq( 16 );
                rtl_createUuid( (sal_uInt8*)aSeq.getArray(), 0,sal_True );
                pSeq = &aSeq;
        }
    }
    return *pSeq;
}

//---------------------------------------------------------------------------------------
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL SbaXGridPeer::getTypes() throw (::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > aTypes = FmXGridPeer::getTypes();
    sal_Int32 nOldLen = aTypes.getLength();
    aTypes.realloc(nOldLen + 1);
    aTypes.getArray()[nOldLen] = ::getCppuType( reinterpret_cast< ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >* >(NULL) );

    return aTypes;
}

// return implementation specific data
//------------------------------------------------------------------
sal_Int64 SAL_CALL SbaXGridPeer::getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 > & rId )
{
    if( rId.getLength() == 16 && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),  rId.getConstArray(), 16 ) )
        return (sal_Int64)this;

    return FmXGridPeer::getSomething(rId);
}

//---------------------------------------------------------------------------------------
SbaXGridPeer* SbaXGridPeer::getImplementation(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxIFace)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XUnoTunnel > xTunnel(
        _rxIFace, ::com::sun::star::uno::UNO_QUERY);
    if (xTunnel.is())
        return reinterpret_cast<SbaXGridPeer*>(xTunnel->getSomething(getUnoTunnelId()));
    return NULL;
}

//---------------------------------------------------------------------------------------
void SAL_CALL SbaXGridPeer::propertyChange(const ::com::sun::star::beans::PropertyChangeEvent& evt)
{
    FmXGridPeer::propertyChange(evt);
}

//---------------------------------------------------------------------------------------
void SbaXGridPeer::addColumnListeners(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & xCol)
{
    FmXGridPeer::addColumnListeners(xCol);
}

//---------------------------------------------------------------------------------------
void SbaXGridPeer::removeColumnListeners(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & xCol)
{
    FmXGridPeer::removeColumnListeners(xCol);
}

//---------------------------------------------------------------------------------------
FmGridControl* SbaXGridPeer::imp_CreateControl(Window* pParent, WinBits nStyle)
{
    return new SbaGridControl(m_xServiceFactory, pParent, this, nStyle);
}

//==================================================================
// SbaGridHeader
//==================================================================

//---------------------------------------------------------------------------------------
SbaGridHeader::SbaGridHeader(BrowseBox* pParent, WinBits nWinBits)
    :FmGridHeader(pParent, nWinBits)
{
}

//---------------------------------------------------------------------------------------
void SbaGridHeader::ImplSelect(sal_uInt16 nId)
{
    sal_uInt16 nPos = GetModelColumnPos(nId);
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >  xColumns(((SbaGridControl*)GetParent())->GetPeer()->getColumns(), ::com::sun::star::uno::UNO_QUERY);
    if (nPos < xColumns->getCount())
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xColumn(*(::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > *)xColumns->getByIndex(nPos).getValue());
        ::com::sun::star::uno::Reference< ::com::sun::star::view::XSelectionSupplier >  xSelSupplier(xColumns, ::com::sun::star::uno::UNO_QUERY);
        if (xSelSupplier.is())
            xSelSupplier->select(::com::sun::star::uno::makeAny(xColumn));
    }
}

//---------------------------------------------------------------------------------------
void SbaGridHeader::Command( const CommandEvent& rEvt )
{
    sal_Bool bHandled = sal_False;
    if (COMMAND_STARTDRAG == rEvt.GetCommand())
    {
        Point aPos( rEvt.GetMousePosPixel() );

        sal_uInt16 nId = GetItemId(aPos);
        sal_Bool bResizingCol = sal_False;
        if (HEADERBAR_ITEM_NOTFOUND != nId)
        {
            Rectangle aColRect = GetItemRect(nId);
            aColRect.Left() += nId ? 3 : 0; // the handle col (nId == 0) does not have a left margin for resizing
            aColRect.Right() -= 3;
            bResizingCol = !aColRect.IsInside(aPos);
        }
        if (!bResizingCol)
        {
            // because we have 3d-buttons the select handler is called from MouseButtonUp, but StartDrag
            // occures earlier (while the mouse button is down)
            // so for optical reasons we select the column before really starting the drag operation.
            ImplSelect(nId);

            ((SbaGridControl*)GetParent())->GetDataWindow().Command(
                CommandEvent(
                    Point(
                        aPos.X() + GetPosPixel().X(),       // we aren't left-justified with our parent, in contrast to the data window
                        aPos.Y() - GetSizePixel().Height()
                    ),
                    COMMAND_STARTDRAG,
                    rEvt.IsMouseEvent()
                )
            );
            bHandled = sal_True;
        }
    }

    if (!bHandled)
        FmGridHeader::Command(rEvt);
}

//---------------------------------------------------------------------------------------
void SbaGridHeader::Select()
{
    DbBrowseHeader::Select();
    if (!((FmGridControl*)GetParent())->IsDesignMode())
    {   // in design mode the base class does the same ...
        ImplSelect(GetCurItemId());
    }
}

//---------------------------------------------------------------------------------------
void SbaGridHeader::PreExecuteColumnContextMenu(sal_uInt16 nColId, PopupMenu& rMenu)
{
    FmGridHeader::PreExecuteColumnContextMenu(nColId, rMenu);

    // some items are valid only if the db isn't readonly
    sal_Bool bDBIsReadOnly = ((SbaGridControl*)GetParent())->IsReadOnlyDB();

    if (bDBIsReadOnly)
    {
        rMenu.EnableItem(SID_FM_HIDECOL, sal_False);
        PopupMenu* pShowColsMenu = rMenu.GetPopupMenu(SID_FM_SHOWCOLS);
        if (pShowColsMenu)
        {
            // at most 16 items which mean "show column <name>"
            for (sal_uInt16 i=1; i<16; ++i)
                pShowColsMenu->EnableItem(i, sal_False);
            // "show cols/more..." and "show cols/all"
            pShowColsMenu->EnableItem(SID_FM_SHOWCOLS_MORE, sal_False);
            pShowColsMenu->EnableItem(SID_FM_SHOWALLCOLS, sal_False);
        }
    }

    // prepend some new items
    sal_Bool bColAttrs = (nColId != (sal_uInt16)-1);
    if (bColAttrs)
    {
        sal_uInt16 nPos = 0;
        PopupMenu aNewItems(ModuleRes(RID_SBA_GRID_COLCTXMENU));
        if (!bDBIsReadOnly)
        {
            rMenu.InsertItem(ID_BROWSER_COLATTRSET, aNewItems.GetItemText(ID_BROWSER_COLATTRSET), 0, nPos++);
            rMenu.SetHelpId(ID_BROWSER_COLATTRSET, aNewItems.GetHelpId(ID_BROWSER_COLATTRSET));
            rMenu.InsertSeparator(nPos++);

            rMenu.InsertItem(ID_BROWSER_COLWIDTH, aNewItems.GetItemText(ID_BROWSER_COLWIDTH), 0, nPos++);
            rMenu.SetHelpId(ID_BROWSER_COLWIDTH, aNewItems.GetHelpId(ID_BROWSER_COLWIDTH));
            rMenu.InsertSeparator(nPos++);
        }
    }
}

//---------------------------------------------------------------------------------------
void SbaGridHeader::PostExecuteColumnContextMenu(sal_uInt16 nColId, const PopupMenu& rMenu, sal_uInt16 nExecutionResult)
{
    switch (nExecutionResult)
    {
        case ID_BROWSER_COLWIDTH:
            ((SbaGridControl*)GetParent())->SetColWidth(nColId);
            break;

        case ID_BROWSER_COLATTRSET:
            ((SbaGridControl*)GetParent())->SetColAttrs(nColId);
            break;

        default: FmGridHeader::PostExecuteColumnContextMenu(nColId, rMenu, nExecutionResult);
    }
}

//==================================================================
// SbaGridControl
//==================================================================

//---------------------------------------------------------------------------------------
SbaGridControl::SbaGridControl(::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > _rM,
                               Window* pParent, FmXGridPeer* _pPeer, WinBits nBits)
    :FmGridControl(_rM,pParent, _pPeer, nBits)
    ,m_nLastColId(-1)
    ,m_nLastRowId(-1)
    ,m_nCurrentSelectedColumn(-1)
    ,m_nCurrentActionColId(-1)
    ,m_bSelecting(sal_False)
    ,m_pMasterListener(NULL)
    ,m_bActivatingForDrop(sal_False)
{
}

//---------------------------------------------------------------------------------------
SbaGridControl::~SbaGridControl()
{
}

//---------------------------------------------------------------------------------------
BrowserHeader* SbaGridControl::imp_CreateHeaderBar(BrowseBox* pParent)
{
    return new SbaGridHeader(pParent);
}

//---------------------------------------------------------------------------------------
DbCellController* SbaGridControl::GetController(long nRow, sal_uInt16 nCol)
{
    DbCellControllerRef aController;
    if (m_bActivatingForDrop)
        return &aController;

    return FmGridControl::GetController(nRow, nCol);
}

//---------------------------------------------------------------------------------------
void SbaGridControl::PreExecuteRowContextMenu(sal_uInt16 nRow, PopupMenu& rMenu)
{
    FmGridControl::PreExecuteRowContextMenu(nRow, rMenu);

    PopupMenu aNewItems(ModuleRes(RID_SBA_GRID_ROWCTXMENU));
    sal_uInt16 nPos = 0;

    if (!IsReadOnlyDB())
    {
        rMenu.InsertItem(ID_BROWSER_TABLEATTR, aNewItems.GetItemText(ID_BROWSER_TABLEATTR), 0, nPos++);
        rMenu.SetHelpId(ID_BROWSER_TABLEATTR, aNewItems.GetHelpId(ID_BROWSER_TABLEATTR));

        rMenu.InsertItem(ID_BROWSER_ROWHEIGHT, aNewItems.GetItemText(ID_BROWSER_ROWHEIGHT), 0, nPos++);
        rMenu.SetHelpId(ID_BROWSER_ROWHEIGHT, aNewItems.GetHelpId(ID_BROWSER_ROWHEIGHT));
    }
    if (nPos)
        rMenu.InsertSeparator(nPos);
}

//------------------------------------------------------------------------------
SvNumberFormatter* SbaGridControl::GetDatasourceFormatter()
{
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >  xSupplier = ::dbtools::getNumberFormats(::dbtools::getConnection(::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet > (getDataSource(),::com::sun::star::uno::UNO_QUERY)), sal_True,getServiceManager());

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XUnoTunnel > xTunnel(xSupplier,::com::sun::star::uno::UNO_QUERY);
    SvNumberFormatsSupplierObj* pSupplierImpl = (SvNumberFormatsSupplierObj*)xTunnel->getSomething(SvNumberFormatsSupplierObj::getUnoTunnelId());

    if (!pSupplierImpl)
        return NULL;

    SvNumberFormatter* pFormatter = pSupplierImpl->GetNumberFormatter();
    return pFormatter;
}

//------------------------------------------------------------------------------
void SbaGridControl::SetColWidth(sal_uInt16 nColId)
{
    // get the (UNO) column model
    sal_uInt16 nModelPos = GetModelColumnPos(nColId);
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >  xCols(GetPeer()->getColumns(), ::com::sun::star::uno::UNO_QUERY);
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xAffectedCol;
    if (xCols.is() && (nModelPos != (sal_uInt16)-1))
        xAffectedCol = ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > (*(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > *)xCols->getByIndex(nModelPos).getValue(), ::com::sun::star::uno::UNO_QUERY);

    if (xAffectedCol.is())
    {
        ::com::sun::star::uno::Any aWidth = xAffectedCol->getPropertyValue(PROPERTY_WIDTH);
        sal_Int32 nCurWidth = aWidth.hasValue() ? ::comphelper::getINT32(aWidth) : -1;

//      DlgSize aDlgColWidth(NULL, nCurWidth, sal_False);
//      if (aDlgColWidth.Execute())
//      {
//          sal_uInt16 nValue = aDlgColWidth.GetValue();
//          ::com::sun::star::uno::Any aNewWidth;
//          if ((sal_uInt16)-1 == nValue)
//          {   // set to default
//              ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyState >  xPropState(xAffectedCol, ::com::sun::star::uno::UNO_QUERY);
//              if (xPropState.is())
//              {
//                  try { aNewWidth = xPropState->getPropertyDefault(PROPERTY_WIDTH); } catch(Exception&) { } ;
//              }
//          }
//          else
//              aNewWidth <<= nValue;
//          try {  xAffectedCol->setPropertyValue(PROPERTY_WIDTH, aNewWidth); } catch(Exception&) { } ;
//      }
    }
}

//------------------------------------------------------------------------------
void SbaGridControl::SetRowHeight()
{
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xCols(GetPeer()->getColumns(), ::com::sun::star::uno::UNO_QUERY);
    if (!xCols.is())
        return;

    ::com::sun::star::uno::Any aHeight = xCols->getPropertyValue(PROPERTY_ROW_HEIGHT);
    sal_Int32 nCurHeight = aHeight.hasValue() ? ::comphelper::getINT32(aHeight) : -1;

    // TODO
//  DlgSize aDlgRowHeight(NULL, nCurHeight, sal_True);
//  if (aDlgRowHeight.Execute())
//  {
//      sal_uInt16 nValue = aDlgRowHeight.GetValue();
//      ::com::sun::star::uno::Any aNewHeight;
//      if ((sal_uInt16)-1 == nValue)
//      {   // set to default
//          ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyState >  xPropState(xCols, ::com::sun::star::uno::UNO_QUERY);
//          if (xPropState.is())
//          {
//              try { aNewHeight = xPropState->getPropertyDefault(PROPERTY_ROW_HEIGHT); } catch(Exception&) { } ;
//          }
//      }
//      else
//          aNewHeight <<= nValue;
//      try {  xCols->setPropertyValue(PROPERTY_ROW_HEIGHT, aNewHeight); } catch(Exception&) { } ;
//  }
}

//------------------------------------------------------------------------------
void SbaGridControl::SetColAttrs(sal_uInt16 nColId)
{
//  SvNumberFormatter* pFormatter = GetDatasourceFormatter();
//  if (!pFormatter)
//      return;
//
//  sal_uInt16 nModelPos = GetModelColumnPos(nColId);
//
//  // get the (UNO) column model
//  ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >  xCols(GetPeer()->getColumns(), ::com::sun::star::uno::UNO_QUERY);
//  ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xAffectedCol;
//  if (xCols.is() && (nModelPos != (sal_uInt16)-1))
//      xAffectedCol = ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > (*(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > *)xCols->getByIndex(nModelPos).getValue(), ::com::sun::star::uno::UNO_QUERY);
//
//  // get the field the column is bound to
//  ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xField = getField(nModelPos);
//
//  if (xAffectedCol.is() && xField.is())
//  {
//      // the allowed format changes depend of the type of the field ...
//      XubString sColName = ::comphelper::getString(xField->getPropertyValue(PROPERTY_NAME));
//
//      sal_uInt16  nFlags = TP_ATTR_ALIGN;
//      ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >  xInfo = xAffectedCol->getPropertySetInfo();
//      sal_Bool bHasFormat = xInfo->hasPropertyByName(PROPERTY_FORMATKEY);
//      if (bHasFormat)
//          nFlags |= TP_ATTR_NUMBER;
//
//      // ------------
//      // UNO->ItemSet
//      static SfxItemInfo aItemInfos[] =
//      {
//          { 0, 0 },
//          { SID_ATTR_NUMBERFORMAT_VALUE,      SFX_ITEM_POOLABLE },
//          { SID_ATTR_ALIGN_HOR_JUSTIFY,       SFX_ITEM_POOLABLE },
//          { SID_ATTR_NUMBERFORMAT_ONE_AREA,   SFX_ITEM_POOLABLE }
//      };
//      static sal_uInt16 aAttrMap[] =
//      {
//          SBA_DEF_RANGEFORMAT, SBA_ATTR_ALIGN_HOR_JUSTIFY,
//          SID_ATTR_NUMBERFORMAT_ONE_AREA, SID_ATTR_NUMBERFORMAT_ONE_AREA,
//          0
//      };
//
//      SfxPoolItem* pDefaults[] =
//      {
//          new SfxRangeItem(SBA_DEF_RANGEFORMAT, SBA_DEF_FMTVALUE, SBA_ATTR_ALIGN_HOR_JUSTIFY),
//          new SfxUInt32Item(SBA_DEF_FMTVALUE),
//          new SvxHorJustifyItem(SVX_HOR_JUSTIFY_STANDARD, SBA_ATTR_ALIGN_HOR_JUSTIFY),
//          new SfxBoolItem(SID_ATTR_NUMBERFORMAT_ONE_AREA, sal_False)
//      };
//
//      SfxItemPool* pPool = new SfxItemPool(String::CreateFromAscii("GridBrowserProperties"), SBA_DEF_RANGEFORMAT, SBA_ATTR_ALIGN_HOR_JUSTIFY, aItemInfos, pDefaults);
//      pPool->SetDefaultMetric( SFX_MAPUNIT_TWIP );    // ripped, don't understand why
//      pPool->FreezeIdRanges();                        // the same
//
//      SfxItemSet* pFormatDescriptor = new SfxItemSet(*pPool, aAttrMap);
//      // fill it
//      SvxCellHorJustify eJustify(SVX_HOR_JUSTIFY_STANDARD);
//      ::com::sun::star::uno::Any aAlignment = xAffectedCol->getPropertyValue(PROPERTY_ALIGN);
//      if (aAlignment.hasValue())
//          switch (::comphelper::getINT16(aAlignment))
//          {
//              case ::com::sun::star::awt::TextAlign::LEFT     : eJustify = SVX_HOR_JUSTIFY_LEFT; break;
//              case ::com::sun::star::awt::TextAlign::CENTER   : eJustify = SVX_HOR_JUSTIFY_CENTER; break;
//              case ::com::sun::star::awt::TextAlign::RIGHT    : eJustify = SVX_HOR_JUSTIFY_RIGHT; break;
//          }
//      pFormatDescriptor->Put(SvxHorJustifyItem(eJustify, SBA_ATTR_ALIGN_HOR_JUSTIFY));
//      if (bHasFormat)
//      {
//          sal_Int32 nFormatKey = ::comphelper::getINT32(xAffectedCol->getPropertyValue(PROPERTY_FORMATKEY));
//          // if the col is bound to a text field we have to disallow all non-text formats
//          sal_Int16 nFieldType = ::comphelper::getINT16(xField->getPropertyValue(PROPERTY_TYPE));
//          if ((::com::sun::star::data::DatabaseFieldType::CHAR == nFieldType) || (::com::sun::star::data::DatabaseFieldType::VARCHAR == nFieldType) || (::com::sun::star::data::DatabaseFieldType::LONGVARCHAR == nFieldType))
//          {
//              pFormatDescriptor->Put(SfxBoolItem(SID_ATTR_NUMBERFORMAT_ONE_AREA, sal_True));
//              if (!pFormatter->IsTextFormat(nFormatKey))
//                  // text fields can only have text formats
//                  nFormatKey = pFormatter->GetStandardFormat(NUMBERFORMAT_TEXT, Application::GetAppInternational().GetLanguage());
//          }
//
//          pFormatDescriptor->Put(SfxUInt32Item(SBA_DEF_FMTVALUE, nFormatKey));
//      }
//
//      {   // want the dialog to be destroyed before our set
//          SbaSbAttrDlg aDlg(NULL, pFormatDescriptor, pFormatter, nFlags);
//          if (aDlg.Execute())
//          {
//              // ------------
//              // ItemSet->UNO
//              // UNO-properties
//              const SfxItemSet* pSet = aDlg.GetExampleSet();
//              // (of course we could put the modified items directly into the column, but then the UNO-model
//              // won't reflect these changes, and why do we have a model, then ?)
//
//              // horizontal justify
//              SFX_ITEMSET_GET(*pSet, pHorJustify, SvxHorJustifyItem, SBA_ATTR_ALIGN_HOR_JUSTIFY, sal_True);
//
//              SvxCellHorJustify   eHorJustify = (SvxCellHorJustify)pHorJustify->GetValue();
//              ::com::sun::star::uno::Any aTextAlign;
//              switch (eHorJustify)
//              {
//                  case SVX_HOR_JUSTIFY_LEFT:
//                      aTextAlign <<= ::com::sun::star::awt::TextAlign::LEFT;
//                      break;
//                  case SVX_HOR_JUSTIFY_CENTER:
//                      aTextAlign <<= ::com::sun::star::awt::TextAlign::CENTER;
//                      break;
//                  case SVX_HOR_JUSTIFY_RIGHT:
//                      aTextAlign <<= ::com::sun::star::awt::TextAlign::RIGHT;
//                      break;
//              }
//              xAffectedCol->setPropertyValue(PROPERTY_ALIGN, aTextAlign);
//
//              // format key
//              if (nFlags & TP_ATTR_NUMBER)
//              {
//                  SFX_ITEMSET_GET(*pSet, pFormat, SfxUInt32Item, SBA_DEF_FMTVALUE, sal_True);
//                  xAffectedCol->setPropertyValue(PROPERTY_FORMATKEY, ::com::sun::star::uno::makeAny((sal_Int32)pFormat->GetValue()));
//              }
//          }
//      }
//
//      delete pFormatDescriptor;
//      delete pPool;
//      for (sal_uInt16 i=0; i<sizeof(pDefaults)/sizeof(pDefaults[0]); ++i)
//          delete pDefaults[i];
//  }
}


//------------------------------------------------------------------------------
void SbaGridControl::SetBrowserAttrs()
{
//  ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xGridModel(GetPeer()->getColumns(), ::com::sun::star::uno::UNO_QUERY);
//  if (!xGridModel.is())
//      return;
//
//  SvNumberFormatter* pFormatter = GetDatasourceFormatter();
//  if (!pFormatter)
//      return;
//
//  // build an itemset describing the current font
//  static SfxItemInfo aItemInfos[] =
//  {
//      { 0, 0 },
//      { SID_ATTR_CHAR_FONT,       SFX_ITEM_POOLABLE },
//      { SID_ATTR_CHAR_FONTHEIGHT, SFX_ITEM_POOLABLE },
//      { SID_ATTR_CHAR_COLOR,      SFX_ITEM_POOLABLE },
//      { SID_ATTR_CHAR_WEIGHT,     SFX_ITEM_POOLABLE },
//      { SID_ATTR_CHAR_POSTURE,    SFX_ITEM_POOLABLE },
//      { 0, 0 },
//      { 0, 0 },
//      { SID_ATTR_CHAR_STRIKEOUT,  SFX_ITEM_POOLABLE },
//      { SID_ATTR_CHAR_UNDERLINE,  SFX_ITEM_POOLABLE }
//  };
//  static sal_uInt16 aAttrMap[] =
//  {
//      SBA_DEF_FONT, SBA_DEF_UNDERLINE,
//      0
//  };
//
//  Font    aFont = System::GetStandardFont(STDFONT_APP);
//  aFont.SetSize(Size(aFont.GetSize().Width(), 10));
//  sal_uInt32  nFontHeight = (sal_uInt32)PixelToLogic(aFont.GetSize(), MAP_TWIP).Height();
//
//  SfxPoolItem* pDefaults[] =
//  {
//      new SfxRangeItem( SBA_DEF_RANGEFONT, SBA_DEF_FONT, SBA_DEF_UNDERLINE ),
//      new SvxFontItem(aFont.GetFamily(), aFont.GetName(), String(), PITCH_DONTKNOW, RTL_TEXTENCODING_DONTKNOW, SBA_DEF_FONT),
//      new SvxFontHeightItem(nFontHeight, 100, SBA_DEF_FONTHEIGHT ),
//      new SvxColorItem(aFont.GetColor(), SBA_DEF_FONTCOLOR ),
//      new SvxWeightItem(WEIGHT_LIGHT, SBA_DEF_FONTWEIGHT),
//      new SvxPostureItem( ITALIC_NONE, SBA_DEF_POSTURE ),
//      new SvxShadowedItem( sal_False, SBA_DEF_SHADOWED ),
//      new SvxContourItem( sal_False, SBA_DEF_CONTOUR ),
//      new SvxCrossedOutItem( STRIKEOUT_NONE, SBA_DEF_CROSSEDOUT ),
//      new SvxUnderlineItem( UNDERLINE_NONE, SBA_DEF_UNDERLINE )
//  };
//
//  SfxItemPool* pPool = new SfxItemPool(String::CreateFromAscii("GridBrowserProperties"), SBA_DEF_RANGEFONT, SBA_DEF_UNDERLINE, aItemInfos, pDefaults);
//  pPool->SetDefaultMetric( SFX_MAPUNIT_TWIP );
//  pPool->FreezeIdRanges();
//
//  SfxItemSet* pFontDescriptor = new SfxItemSet(*pPool, aAttrMap);
//
//  // initialize it with the current settings (font/color)
//  ::com::sun::star::awt::FontDescriptor aCurrentFont;
//  xGridModel->getPropertyValue(PROPERTY_FONT) >>= aCurrentFont;
//  BuildItemsFromFont(pFontDescriptor, aCurrentFont);
//  ::com::sun::star::uno::Any aColor = xGridModel->getPropertyValue(PROPERTY_TEXTCOLOR);
//  if (aColor.hasValue())
//      pFontDescriptor->Put(SvxColorItem((Color)::comphelper::getINT32(aColor), SBA_DEF_FONTCOLOR));
//
//  {   // want the dialog to be destroyed before our set
//      SbaSbAttrDlg aDlg(GetParent(), pFontDescriptor, pFormatter, TP_ATTR_CHAR, sal_True);
//      if (aDlg.Execute())
//      {
//          ::com::sun::star::awt::FontDescriptor aNewFont = BuildFontFromItems(aDlg.GetExampleSet(), GetFont());
//          xGridModel->setPropertyValue(PROPERTY_FONT, ::com::sun::star::uno::Any(&aNewFont, ::getCppuType((const ::com::sun::star::awt::FontDescriptor*)0)));
//
//          SFX_ITEMSET_GET(*aDlg.GetOutputItemSet(), pColor, SvxColorItem, SBA_DEF_FONTCOLOR, sal_True);
//          xGridModel->setPropertyValue(PROPERTY_TEXTCOLOR, ::com::sun::star::uno::makeAny((sal_Int32)pColor->GetValue().GetColor()));
//      }
//  }
//
//  delete pFontDescriptor;
//  delete pPool;
//  for (sal_uInt16 i=0; i<sizeof(pDefaults)/sizeof(pDefaults[0]); ++i)
//      delete pDefaults[i];
}

//---------------------------------------------------------------------------------------
void SbaGridControl::PostExecuteRowContextMenu(sal_uInt16 nRow, const PopupMenu& rMenu, sal_uInt16 nExecutionResult)
{
    switch (nExecutionResult)
    {
        case ID_BROWSER_TABLEATTR:
            SetBrowserAttrs();
            break;
        case ID_BROWSER_ROWHEIGHT:
            SetRowHeight();
            break;

        default:
            FmGridControl::PostExecuteRowContextMenu(nRow, rMenu, nExecutionResult);
            break;
    }
}

//---------------------------------------------------------------------------------------
void SbaGridControl::Select()
{
    // irgendeine Selektion hat sich geaendert ....
    FmGridControl::Select();

    // ... betrifft das unsere Spalten ?
    const MultiSelection* pColumnSelection = GetColumnSelection();

    long nSelectedColumn =
        pColumnSelection && pColumnSelection->GetSelectCount()
            ? ((MultiSelection*)pColumnSelection)->FirstSelected()
            : -1L;
    // die HandleColumn wird nicht selektiert
    switch (nSelectedColumn)
    {
        case -1 : break;    // no selection
        case  0 : nSelectedColumn = -1; break;  // handle col can't be seledted
        default :
            // get the model col pos instead of the view col pos
            nSelectedColumn = GetModelColumnPos(GetColumnIdFromViewPos(nSelectedColumn - 1));
            break;
    }

    if (nSelectedColumn != m_nCurrentSelectedColumn)
    {
        // VOR dem Aufruf des select am SelectionSupplier !
        m_nCurrentSelectedColumn = nSelectedColumn;

        if (!m_bSelecting)
        {
            m_bSelecting = sal_True;

            try
            {
                ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >  xColumns(GetPeer()->getColumns(), ::com::sun::star::uno::UNO_QUERY);
                ::com::sun::star::uno::Reference< ::com::sun::star::view::XSelectionSupplier >  xSelSupplier(xColumns, ::com::sun::star::uno::UNO_QUERY);
                if (xSelSupplier.is())
                    if (nSelectedColumn != -1)
                    {
                        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xColumn(*(::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > *)xColumns->getByIndex(nSelectedColumn).getValue());
                        xSelSupplier->select(::com::sun::star::uno::makeAny(xColumn));
                    }
                    else
                    {
                        xSelSupplier->select(::com::sun::star::uno::Any());
                    }
            }
            catch(Exception&)
            {
            }


            m_bSelecting = sal_False;
        }
    }

    if (m_pMasterListener)
        m_pMasterListener->SelectionChanged();
}

//---------------------------------------------------------------------------------------
void SbaGridControl::CursorMoved()
{
    FmGridControl::CursorMoved();
    if (m_nLastRowId != GetCurRow())
        RowChanged();
    if (m_nLastColId != GetCurColumnId())
        ColChanged();

    m_nLastColId = GetCurColumnId();
    m_nLastRowId = GetCurRow();
}

//---------------------------------------------------------------------------------------
void SbaGridControl::ActivateCell(long nRow, sal_uInt16 nCol, sal_Bool bSetCellFocus /*= sal_True*/ )
{
    FmGridControl::ActivateCell(nRow, nCol, bSetCellFocus);
    if (m_pMasterListener)
        m_pMasterListener->CellActivated();
}

//---------------------------------------------------------------------------------------
void SbaGridControl::DeactivateCell(sal_Bool bUpdate /*= sal_True*/)
{
    FmGridControl::DeactivateCell(bUpdate);
    if (m_pMasterListener)
        m_pMasterListener->CellDeactivated();
}

//---------------------------------------------------------------------------------------
void SbaGridControl::RowChanged()
{
    if (m_pMasterListener)
        m_pMasterListener->RowChanged();
}

//---------------------------------------------------------------------------------------
void SbaGridControl::ColChanged()
{
    if (m_pMasterListener)
        m_pMasterListener->ColumnChanged();
}


//------------------------------------------------------------------------------
void SbaGridControl::setDataSource(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet > & rCursor, sal_uInt16 nOpts)
{
    FmGridControl::setDataSource(rCursor, nOpts);

    // for DnD we need a query composer
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >   xFormSet = getDataSource();
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >  xForm(xFormSet, ::com::sun::star::uno::UNO_QUERY);
    if (xForm.is() && xFormSet.is() && ::comphelper::getBOOL(xFormSet->getPropertyValue(PROPERTY_USE_ESCAPE_PROCESSING)))
    {   //  (only if the statement isn't native)
        ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSQLQueryComposerFactory >  xFactory(::dbtools::getConnection(::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet > (xFormSet,::com::sun::star::uno::UNO_QUERY)), ::com::sun::star::uno::UNO_QUERY);
        if (xFactory.is())
            m_xComposer = xFactory->createQueryComposer();
    }
    else
        m_xComposer = NULL;
}

//------------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  SbaGridControl::getField(sal_uInt16 nModelPos)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xEmptyReturn;
    try
    {
        // first get the name of the column
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >  xCols(GetPeer()->getColumns(), ::com::sun::star::uno::UNO_QUERY);
        if (!xCols.is())
            return xEmptyReturn;

        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xCol(*(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > *)xCols->getByIndex(nModelPos).getValue(), ::com::sun::star::uno::UNO_QUERY);
        if (!xCol.is())
            return xEmptyReturn;

        ::com::sun::star::uno::Any aBoundField = xCol->getPropertyValue(PROPERTY_BOUNDFIELD);
        if (aBoundField.hasValue())
            return ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > (*(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > *)aBoundField.getValue(), ::com::sun::star::uno::UNO_QUERY);
    }
    catch(Exception&)
    {
    }

    return xEmptyReturn;
}

//---------------------------------------------------------------------------------------
sal_Bool SbaGridControl::IsReadOnlyDB() const
{
    // assume yes if anything fails
    sal_Bool bDBIsReadOnly = sal_True;

    // the db is the implemented by the parent of the grid control's model ...
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XChild >  xColumns(GetPeer()->getColumns(), ::com::sun::star::uno::UNO_QUERY);
    if (xColumns.is())
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >  xDataSource(xColumns->getParent(), ::com::sun::star::uno::UNO_QUERY);
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XChild >  xConn(::dbtools::getConnection(xDataSource),::com::sun::star::uno::UNO_QUERY);
        if (xConn.is())
        {
            // ... and the RO-flag simply is implemented by a property
            ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xDbProps(xConn->getParent(), ::com::sun::star::uno::UNO_QUERY);
            if (xDbProps.is())
            {
                ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >  xInfo = xDbProps->getPropertySetInfo();
                if (xInfo->hasPropertyByName(PROPERTY_ISREADONLY))
                    bDBIsReadOnly = ::comphelper::getBOOL(xDbProps->getPropertyValue(PROPERTY_ISREADONLY));
            }
        }
    }
    return bDBIsReadOnly;
}

//---------------------------------------------------------------------------------------
void SbaGridControl::MouseButtonDown( const BrowserMouseEvent& rMEvt)
{
    long nRow = GetRowAtYPosPixel(rMEvt.GetPosPixel().Y());
    sal_uInt16 nColPos = GetColumnAtXPosPixel(rMEvt.GetPosPixel().X());
    sal_uInt16 nViewPos = (nColPos == BROWSER_INVALIDID) ? (sal_uInt16)-1 : nColPos-1;
        // 'the handle column' and 'no valid column' will both result in a view position of -1 !

    sal_Bool bHitEmptySpace = (nRow > GetRowCount()) || (nViewPos == (sal_uInt16)-1);

    if (bHitEmptySpace && (rMEvt.GetClicks() == 2) && rMEvt.IsMod1())
        Control::MouseButtonDown(rMEvt);
    else
        FmGridControl::MouseButtonDown(rMEvt);
}

//------------------------------------------------------------------------------
void SbaGridControl::Command(const CommandEvent& rEvt)
{
    sal_Bool bHandled = sal_False;
    switch (rEvt.GetCommand())
    {
        case COMMAND_STARTDRAG:
            // determine if dragging is allowed
            // (Yes, this is controller (not view) functionality. But collecting and evaluating all the
            // informations necessary via UNO would be quite difficult (if not impossible) so
            // my laziness says 'do it here' ...)
            long nRow = GetRowAtYPosPixel(rEvt.GetMousePosPixel().Y());
            sal_uInt16 nColPos = GetColumnAtXPosPixel(rEvt.GetMousePosPixel().X());
            sal_uInt16 nViewPos = (nColPos == BROWSER_INVALIDID) ? (sal_uInt16)-1 : nColPos-1;
                // 'the handle column' and 'no valid column' will both result in a view position of -1 !

            sal_Bool bCurrentRowVirtual = IsCurrentAppending() && IsModified();
            // the current row doesn't really exist : the user's appendign a new one and already has entered some data,
            // so the row contains data which has no counter part within the data source

            long nCorrectRowCount = GetRowCount();
            if (GetOptions() & OPT_INSERT)
                --nCorrectRowCount; // there is a empty row for inserting records
            if (bCurrentRowVirtual)
                --nCorrectRowCount;

            if ((nColPos == BROWSER_INVALIDID) || (nRow >= nCorrectRowCount))
                break;

            sal_Bool bHitHandle = (nColPos == 0);

            // check which kind of dragging has to be initiated
            if  (   bHitHandle                          //  the handle column
                &&  (   GetSelectRowCount()             //  at least one row is selected
                    ||  (   (nRow >= 0)                 //  a row below the header
                        &&  !bCurrentRowVirtual         //  we aren't appending a new record
                        &&  (nRow != GetCurrentPos())   //  a row which is not the current one
                        )
                    )
                )
            {   // => start dragging the row
                if (GetDataWindow().IsMouseCaptured())
                    GetDataWindow().ReleaseMouse();

                getMouseEvent().Clear();
                DoRowDrag(nRow);

                bHandled = sal_True;
            }
            else if (   (nRow < 0)                      // the header
                    &&  (!bHitHandle)                   // non-handle column
                    &&  (nViewPos < GetViewColCount())  // valid (existing) column
                    )
            {   // => start dragging the column
                if (GetDataWindow().IsMouseCaptured())
                    GetDataWindow().ReleaseMouse();

                getMouseEvent().Clear();
                DoColumnDrag(nViewPos);

                bHandled = sal_True;
            }
            else if (   !bHitHandle     // non-handle column
                    &&  (nRow >= 0)     // non-header row
                    )
            {   // => start dragging the field content
                if (GetDataWindow().IsMouseCaptured())
                    GetDataWindow().ReleaseMouse();

                getMouseEvent().Clear();
                DoFieldDrag(nViewPos, nRow);

                bHandled = sal_True;
            }
            break;
    }

    if (!bHandled)
        FmGridControl::Command(rEvt);
}

// -----------------------------------------------------------------------
void SbaGridControl::DoColumnDrag(sal_uInt16 nColumnPos)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xDataSource(getDataSource(), ::com::sun::star::uno::UNO_QUERY);
    DBG_ASSERT(xDataSource.is(), "SbaGridControl::DoColumnDrag : invalid data source !");
    // collect some properties from our data source
    sal_Int32               nDataType;
    XubString               sDataSource;
    XubString               sDBAlias;
    try
    {
        nDataType = ::comphelper::getINT32(xDataSource->getPropertyValue(PROPERTY_COMMANDTYPE));
        sDataSource = (const sal_Unicode*)::comphelper::getString(xDataSource->getPropertyValue(PROPERTY_COMMAND));
        sDBAlias = (const sal_Unicode*)::comphelper::getString(xDataSource->getPropertyValue(PROPERTY_DATASOURCENAME));
    }
    catch(Exception&)
    {
        DBG_ERROR("SbaGridControl::DoColumnDrag : could not collect essential data source attributes !");
        return;
    }


//  XubString sDBAlias = EnsureDBLink(m_aDbEnv, GetFormDatabase(xDataSource), sal_True /* allow ui */, sal_True /* really need an alias */ );
//  if (!sDBAlias.Len())
//      // the user did not allow us to create an alias
//      return;

    XubString sObjectKind = (nDataType == ::com::sun::star::sdb::CommandType::TABLE) ? XubString('0') : XubString('1');

    // If the data source is an SQL-statement and simple enough (means "select <field list> from <table> where ....")
    // we are able to fake the drag information we are about to create.
    sal_Bool bTryToParse = sal_False;
    if (nDataType == ::com::sun::star::sdb::CommandType::COMMAND)
    {
        try
        {
            bTryToParse = ::comphelper::getBOOL(xDataSource->getPropertyValue(PROPERTY_USE_ESCAPE_PROCESSING));
        }
        catch(Exception&)
        {
            DBG_ERROR("SbaGridControl::DoColumnDrag : could not ask for the escape processing property ! ignoring this ...");
        }
        ;
    }
    if (bTryToParse)
    {
        try
        {
            ::rtl::OUString sFilter;
            if (::comphelper::getBOOL(xDataSource->getPropertyValue(PROPERTY_APPLYFILTER)))
                sFilter = ::comphelper::getString(xDataSource->getPropertyValue(PROPERTY_FILTER));
            ::rtl::OUString sSort = ::comphelper::getString(xDataSource->getPropertyValue(PROPERTY_ORDER));

            if (m_xComposer.is())
            {
                m_xComposer->setQuery(::comphelper::getString(xDataSource->getPropertyValue(PROPERTY_ACTIVECOMMAND)));
                m_xComposer->setFilter(sFilter);
                m_xComposer->setOrder(sSort);
                Reference<XTablesSupplier> xSupTab(m_xComposer,UNO_QUERY);
                if(xSupTab.is())
                {
                    Reference<XNameAccess> xNames = xSupTab->getTables();
                    sDataSource = xNames->getElementNames()[0];
                    sObjectKind = '0';                  // means type 'table'
                }
            }
        }
        catch(Exception&)
        {
            DBG_ERROR("SbaGridControl::DoColumnDrag : could not collect essential data source attributes (part two) !");
            return;
        }
    }

    XubString aCopyData = sDBAlias;
    aCopyData   += char(11);
    aCopyData   += sDataSource;
    aCopyData   += char(11);
    aCopyData   += sObjectKind;
    aCopyData   += char(11);

    XubString sField;
    try
    {
        sal_uInt16 nModelPos = GetModelColumnPos(GetColumnIdFromViewPos(nColumnPos));
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >  xCols(GetPeer()->getColumns(), ::com::sun::star::uno::UNO_QUERY);
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xAffectedCol = *(::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > *)xCols->getByIndex(nModelPos).getValue();
        sField = (const sal_Unicode*)::comphelper::getString(xAffectedCol->getPropertyValue(PROPERTY_CONTROLSOURCE));
    }
    catch(Exception&)
    {
        DBG_ERROR("SbaGridControl::DoColumnDrag : something went wrong while getting the column");
        return;
    }

    aCopyData   += sField;

    DragServer::Clear();
    if (!DragServer::CopyData(aCopyData.GetBuffer(), aCopyData.Len() + 1, RegisterColumnExchangeFormatName()))
        return;

    Pointer aMovePtr(POINTER_MOVEDATA),
            aCopyPtr(POINTER_COPYDATA),
            aLinkPtr(POINTER_LINKDATA);
    ExecuteDrag(aMovePtr, aCopyPtr, aLinkPtr, DRAG_COPYABLE | DRAG_LINKABLE);
}

// -----------------------------------------------------------------------
void SbaGridControl::DoRowDrag(sal_uInt16 nRowPos)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xDataSource(getDataSource(), ::com::sun::star::uno::UNO_QUERY);
    DBG_ASSERT(xDataSource.is(), "SbaGridControl::DoRowDrag : invalid data source !");
    // collect some data source properties
    sal_Int32               nDataType;
    XubString               sDataSource;
    XubString               sDBAlias;

    try
    {
        nDataType = ::comphelper::getINT32(xDataSource->getPropertyValue(PROPERTY_COMMANDTYPE));
        sDataSource = (const sal_Unicode*)::comphelper::getString(xDataSource->getPropertyValue(PROPERTY_COMMAND));
        sDBAlias = (const sal_Unicode*)::comphelper::getString(xDataSource->getPropertyValue(PROPERTY_DATASOURCENAME));
    }
    catch(Exception&)
    {
        DBG_ERROR("SbaGridControl::DoRowDrag : could not collect essential data source attributes !");
        return;
    }

    sal_Bool bIsStatement = ::com::sun::star::sdb::CommandType::COMMAND == nDataType;
    XubString sObjectKind = (::com::sun::star::sdb::CommandType::TABLE == nDataType) ? XubString('1') : XubString('0');

//  XubString sDBAlias = EnsureDBLink(m_aDbEnv, GetFormDatabase(xDataSource), sal_True /* allow ui */, sal_True /* really need an alias */ );
//  if (!sDBAlias.Len())
//      // the user did not allow us to create an alias
//      return;

    // check if the SQL-statement is modified
    sal_Bool bHasFilterOrSort(sal_False);
    XubString sCompleteStatement;
    try
    {
        ::rtl::OUString sFilter;
        if (::comphelper::getBOOL(xDataSource->getPropertyValue(PROPERTY_APPLYFILTER)))
            sFilter = ::comphelper::getString(xDataSource->getPropertyValue(PROPERTY_FILTER));
        ::rtl::OUString sSort = ::comphelper::getString(xDataSource->getPropertyValue(PROPERTY_ORDER));
        bHasFilterOrSort = (sFilter.len()>0) || (sSort.len()>0);

        if (m_xComposer.is())
        {
            m_xComposer->setQuery(::comphelper::getString(xDataSource->getPropertyValue(PROPERTY_ACTIVECOMMAND)));
            m_xComposer->setFilter(sFilter);
            m_xComposer->setOrder(sSort);
            sCompleteStatement = (const sal_Unicode*)m_xComposer->getComposedQuery();
        }
        else
            sCompleteStatement = (const sal_Unicode*)::comphelper::getString(xDataSource->getPropertyValue(PROPERTY_ACTIVECOMMAND));
    }
    catch(Exception&)
    {
        DBG_ERROR("SbaGridControl::DoRowDrag : could not collect essential data source attributes (part two) !");
        return;
    }


    XubString aCopyData = sDBAlias;
    aCopyData   += char(11);
    aCopyData   += bIsStatement ? XubString() : sDataSource;
    aCopyData   += char(11);
    aCopyData   += sObjectKind;
    aCopyData   += char(11);
    aCopyData   += (::com::sun::star::sdb::CommandType::QUERY == nDataType) && !bHasFilterOrSort
        ? XubString()
        : sCompleteStatement;
        // compatibility says : always add the statement, but don't if it is a "pure" query
    aCopyData   += char(11);

    // collect the affected rows
    if ((GetSelectRowCount() == 0) && (nRowPos >= 0))
    {
        aCopyData += String::CreateFromInt32(nRowPos + 1);
        aCopyData += char(11);
    }
    else if (!IsAllSelected())
    {
        for (long nIdx = FirstSelectedRow();
             nIdx >= 0;
             nIdx = NextSelectedRow())
        {
                 aCopyData += String::CreateFromInt32(nIdx + 1);
                 aCopyData += char(11);
        }
    }

    aCopyData.EraseTrailingChars(char(11));

    Pointer aMovePtr(POINTER_COPYDATA),
            aCopyPtr(POINTER_COPYDATA),
            aLinkPtr(POINTER_LINKDATA);

    // for the SbaExplorerExchObj we need a SBA-descrition of the data source
//  DBObject dbType = (::com::sun::star::sdb::CommandType::TABLE == nDataType) ? dbTable : dbQuery;
//  SbaDatabase* pDB = SBA_MOD()->GetDatabase(sDBAlias);
//  if (!pDB)
//  {
//      DBG_ERROR("SbaGridControl::DoRowDrag : could not get an SBA-database !");
//      return;
//  }
//
//  SbaExplorerExchObjRef xExchange = new SbaExplorerExchObj( pDB, dbType, sDataSource, aCopyData );
//  xExchange->ExecuteDrag( this, aMovePtr, aCopyPtr, aLinkPtr, DRAG_COPYABLE | DRAG_LINKABLE );

    return;
}

// -----------------------------------------------------------------------
void SbaGridControl::DoFieldDrag(sal_uInt16 nColumnPos, sal_uInt16 nRowPos)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xDataSource(getDataSource(), ::com::sun::star::uno::UNO_QUERY);
    DBG_ASSERT(xDataSource.is(), "SbaGridControl::DoFieldDrag : invalid data source !");
    // collect some properties from our data source
    sal_Int32               nDataType;
    XubString               sDataSource;
    XubString               sDBAlias;
    try
    {
        nDataType = ::comphelper::getINT32(xDataSource->getPropertyValue(PROPERTY_COMMANDTYPE));
        sDataSource = (const sal_Unicode*)::comphelper::getString(xDataSource->getPropertyValue(PROPERTY_COMMAND));
        sDBAlias = (const sal_Unicode*)::comphelper::getString(xDataSource->getPropertyValue(PROPERTY_DATASOURCENAME));
    }
    catch(Exception&)
    {
        DBG_ERROR("SbaGridControl::DoFieldDrag : could not collect essential data source attributes !");
        return;
    }


    sal_Bool bIsStatement = ::com::sun::star::sdb::CommandType::COMMAND == nDataType;
    XubString sObjectKind = (::com::sun::star::sdb::CommandType::TABLE == nDataType) ? XubString('1') : XubString('0');

//  XubString sDBAlias = EnsureDBLink(m_aDbEnv, GetFormDatabase(xDataSource), sal_True /* allow ui */, sal_True /* really need an alias */ );
//  if (!sDBAlias.Len())
//      // the user did not allow us to create an alias
//      return;

    // build the SBA_FIELDDATAEXCHANGE_FORMAT
    XubString aCopyData = sDBAlias;
    aCopyData   += char(11);
    aCopyData   += bIsStatement ? XubString() : sDataSource;
    aCopyData   += char(11);
    aCopyData   += sObjectKind;
    aCopyData   += char(11);

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xAffectedCol;
    // append the control source of the column
    XubString sField;
    try
    {
        sal_uInt16 nModelPos = GetModelColumnPos(GetColumnIdFromViewPos(nColumnPos));
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >  xCols(GetPeer()->getColumns(), ::com::sun::star::uno::UNO_QUERY);
        xAffectedCol = *(::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > *)xCols->getByIndex(nModelPos).getValue();
        sField = (const sal_Unicode*)::comphelper::getString(xAffectedCol->getPropertyValue(PROPERTY_CONTROLSOURCE));
    }
    catch(Exception&)
    {
        DBG_ERROR("SbaGridControl::DoFieldDrag : something went wrong while retrieving the column's control source !");
        return;
    }

    aCopyData   += sField;
    aCopyData   += char(11);

    // append the field contents formatted as input string
    XubString sInputFormatted;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >  xInfo = xAffectedCol->getPropertySetInfo();
    if (xInfo->hasPropertyByName(PROPERTY_FORMATKEY))       // no chance if the column model doesn't know a format key
    {
        try
        {
            // the number formatter of the connection
            ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >  xFormatter = getNumberFormatter();
            DBG_ASSERT(xFormatter.is(), "SbaGridControl::DoFieldDrag : have no number formatter !");

            // the field the affected col is bound to
            ::com::sun::star::uno::Reference< ::com::sun::star::form::XGridFieldDataSupplier >  xFieldData((::com::sun::star::form::XGridPeer*)GetPeer(), ::com::sun::star::uno::UNO_QUERY);
#ifdef DBG_UTIL
            ::com::sun::star::uno::Sequence<sal_Bool> aSupportingAny = xFieldData->queryFieldDataType(::getCppuType((const ::com::sun::star::uno::Any*)0));
            DBG_ASSERT(aSupportingAny.getConstArray()[nColumnPos],
                "SbaGridControl::DoFieldDrag : the GridFieldDataSupplier should supply UsrAnys !");
#endif
            ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any> aCellContents = xFieldData->queryFieldData(nRowPos, ::getCppuType((const ::com::sun::star::uno::Any*)0));
            ::com::sun::star::uno::Any aFieldValue = aCellContents.getConstArray()[nColumnPos];

            // the format key of the column (we don't use the one supplied by the bound field because it may be out-of-date :
            // modifying a column format is propagated only to the column's model, not to the database field)
            sal_Int32 nKey = ::comphelper::getINT32(xAffectedCol->getPropertyValue(PROPERTY_FORMATKEY));

            // check if the key indicates a text format
            sal_Int16 nType = ::com::sun::star::util::NumberFormat::UNDEFINED;
            ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormats >  xFormats = xFormatter->getNumberFormatsSupplier()->getNumberFormats();
            ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xFormat = xFormats->getByKey(nKey);
            if (xFormat.is())
            {
                try { nType = ::comphelper::getINT16(xFormat->getPropertyValue(PROPERTY_TYPE)); } catch(Exception&) { } ;
            }

            // if it is a text format we can ask for the string
            if ((nType & ~::com::sun::star::util::NumberFormat::DEFINED) == ::com::sun::star::util::NumberFormat::TEXT) // mask the 'user defined' flag
                sInputFormatted = Any2String(aFieldValue);
            else
            {   // else we have to make one distinction more : a currency field always needs an output string
                ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xFieldSet;
                ::cppu::extractInterface(xFieldSet, xAffectedCol->getPropertyValue(PROPERTY_BOUNDFIELD));
                if (xFieldSet.is())
                {
                    sal_Int32 nFieldType = ::comphelper::getINT32(xFieldSet->getPropertyValue(PROPERTY_TYPE));
                    if (::comphelper::getBOOL(xFieldSet->getPropertyValue(::rtl::OUString::createFromAscii("IsCurrency"))))
                        sInputFormatted = (const sal_Unicode*)xFormatter->convertNumberToString(nKey, Any2Double(aFieldValue));
                    else
                        sInputFormatted = (const sal_Unicode*)xFormatter->getInputString(nKey, Any2Double(aFieldValue));
                }
                else
                    sInputFormatted.Erase();
            }
        }
        catch(Exception&)
        {
            DBG_ERROR("SbaGridControl::DoFieldDrag : could not retrieve the cell's input string !");
            return;
        }

    }
    aCopyData += sInputFormatted;

    // feed the drag server with the format just built
    DragServer::Clear();
    if (!DragServer::CopyData(aCopyData.GetBuffer(), aCopyData.Len() + 1, RegisterFieldExchangeFormatName()))
        return;

    // and supply - as an additional pure-text format - the cell contents
    try
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::form::XGridFieldDataSupplier >  xFieldData((::com::sun::star::form::XGridPeer*)GetPeer(), ::com::sun::star::uno::UNO_QUERY);
        ::com::sun::star::uno::Sequence<sal_Bool> aSupportingText = xFieldData->queryFieldDataType(::getCppuType((const ::rtl::OUString*)0));
        if (aSupportingText.getConstArray()[nColumnPos])
        {
            ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any> aCellContents = xFieldData->queryFieldData(nRowPos, ::getCppuType((const ::rtl::OUString*)0));
            ::rtl::OUString aText = ::comphelper::getString(aCellContents.getConstArray()[nColumnPos]);
            DragServer::CopyString(aText);
        }
    }
    catch(Exception&)
    {
        DBG_ERROR("SbaGridControl::DoFieldDrag : could not retrieve the cell's contents !");
        return;
    }



    Pointer aMovePtr(POINTER_MOVEDATA),
            aCopyPtr(POINTER_COPYDATA),
            aLinkPtr(POINTER_LINKDATA);
    ExecuteDrag(aMovePtr, aCopyPtr, aLinkPtr, DRAG_COPYABLE);
}

//------------------------------------------------------------------------------
sal_Bool SbaGridControl::QueryDrop(const BrowserDropEvent& rEvt)
{
    // we need a valid connection
    if (!::dbtools::getConnection(::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet > (getDataSource(),::com::sun::star::uno::UNO_QUERY)).is())
        return sal_False;

    sal_Bool bAllow = sal_False;

    // check formats
    SvDataObjectRef xDataObj = SvDataObject::PasteDragServer( rEvt );
    if (!xDataObj.Is())
        return sal_False;

    const SvDataTypeList& rTypeList = xDataObj->GetTypeList();
    if ((rTypeList.Get(Exchange::RegisterFormatName(String::CreateFromAscii(SBA_VCARDEXCHANGE_FORMAT)))) ||
        (rTypeList.Get(Exchange::RegisterFormatName(String::CreateFromAscii(SBA_DATAEXCHANGE_FORMAT)))) )
    {
        bAllow = (GetOptions() & OPT_INSERT) && rEvt.GetColumnId() > 0 && rEvt.GetRow() >= 0;
        ((BrowserDropEvent&)rEvt).SetAction(DROP_COPY);
    }

    if (rTypeList.Get(FORMAT_STRING)) do
    {   // odd construction, but spares us a lot of (explicit ;) goto's

        if (!GetEmptyRow().Is())
            // without an empty row we're not in update mode
            break;

        long    nRow = GetRowAtYPosPixel(rEvt.GetPosPixel().Y(), sal_False);
        sal_uInt16  nCol = GetColumnAtXPosPixel(rEvt.GetPosPixel().X(), sal_False);

        long nCorrectRowCount = GetRowCount();
        if (GetOptions() & OPT_INSERT)
            --nCorrectRowCount; // there is a empty row for inserting records
        if (IsCurrentAppending())
            --nCorrectRowCount; // the current data record doesn't really exist, we are appending a new one

        if ((nCol == BROWSER_INVALIDID) || (nRow >= nCorrectRowCount))
            // no valid cell under the mouse cursor
            break;

        // from now we work with ids instead of positions
        nCol = GetColumnId(nCol);

        Rectangle aRect = GetCellRect(nRow, nCol, sal_False);
        if (!aRect.IsInside(rEvt.GetPosPixel()))
            // not dropped within a cell (a cell isn't as wide as the column - the are small spaces)
            break;

        if ((IsModified() || (GetCurrentRow().Is() && GetCurrentRow()->IsModified())) && (GetCurrentPos() != nRow))
            // there is a current and modified row or cell and he text is to be dropped into another one
            break;

        DbCellControllerRef xCurrentController = Controller();
        if (xCurrentController.Is() && xCurrentController->IsModified() && ((nRow != GetCurRow()) || (nCol != GetCurColumnId())))
            // the current controller is modified and the user wants to drop in another cell -> no chance
            // (when leaving the modified cell a error may occur - this is deadly while dragging)
            break;

        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xField = getField(GetModelColumnPos(nCol));
        if (!xField.is())
            // the column is not valid bound (for instance a binary field)
            break;

        try
        {
            if (::comphelper::getBOOL(xField->getPropertyValue(PROPERTY_ISREADONLY)))
                break;
        }
        catch(Exception&)
        {
            // assume RO
            break;
        }


        // assume that text can be dropped into a field if the column has a ::com::sun::star::awt::XTextComponent interface
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >  xColumnControls((::com::sun::star::form::XGridPeer*)GetPeer(), ::com::sun::star::uno::UNO_QUERY);
        if (!xColumnControls.is())
            break;
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextComponent >  xColControl(*(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > *)xColumnControls->getByIndex(GetViewColumnPos(nCol)).getValue(), ::com::sun::star::uno::UNO_QUERY);
        if (!xColControl.is())
            break;

        m_bActivatingForDrop = sal_True;
        GoToRowColumnId(nRow, nCol);
        m_bActivatingForDrop = sal_False;

        bAllow = sal_True;

        ((BrowserDropEvent&)rEvt).SetAction(DROP_COPY);
        // see below. as we don't have a m_bDraggingOwnText we have to be more restrictive. text can't be moved into a grid control.

    } while (sal_False);

    return (bAllow) ? bAllow : FmGridControl::QueryDrop(rEvt);
}

//------------------------------------------------------------------------------
sal_Bool SbaGridControl::Drop(const BrowserDropEvent& rEvt)
{
    // we need some properties of our data source
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xDataSource = getDataSource();
    if (!xDataSource.is())
        return sal_False;

    // we need a valid connection
    if (!::dbtools::getConnection(::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet > (xDataSource,::com::sun::star::uno::UNO_QUERY)).is())
        return sal_False;

    //////////////////////////////////////////////////////////////////////
    // DataExch-XubString holen
    SotDataObjectRef xDataObj = ((DropEvent&)rEvt).GetData();
    const SvDataTypeList& rTypeList = xDataObj->GetTypeList();

    if (rTypeList.Get(FORMAT_STRING))
    {
        long    nRow = GetRowAtYPosPixel(rEvt.GetPosPixel().Y(), sal_False);
        sal_uInt16  nCol = GetColumnAtXPosPixel(rEvt.GetPosPixel().X(), sal_False);

        long nCorrectRowCount = GetRowCount();
        if (GetOptions() & OPT_INSERT)
            --nCorrectRowCount; // there is a empty row for inserting records
        if (IsCurrentAppending())
            --nCorrectRowCount; // the current data record doesn't really exist, we are appending a new one

        DBG_ASSERT((nCol != BROWSER_INVALIDID) && (nRow < nCorrectRowCount), "SbaGridControl::Drop : dropped on an invalid position !");
            // QueryDrop should have catched this

        // from now we work with ids instead of positions
        nCol = GetColumnId(nCol);

        GoToRowColumnId(nRow, nCol);
        if (!IsEditing())
            ActivateCell();

        DbCellControllerRef xCurrentController = Controller();
        if (!xCurrentController.Is() || !xCurrentController->ISA(DbEditCellController))
            return sal_False;
        Edit& rEdit = (Edit&)xCurrentController->GetWindow();

        // Daten aus
        SvData aData(FORMAT_STRING);
        if (!xDataObj->GetData(&aData))
            return sal_False;

        XubString sDropped;
        aData.GetData(sDropped);
        rEdit.SetText(sDropped);
        xCurrentController->SetModified();
        rEdit.Modify();
            // SetText itself doesn't call a Modify as it isn't a user interaction

        return sal_True;
    }

    // the last known format
    sal_uInt32 nSbaDataExchangeFormat = Exchange::RegisterFormatName(String::CreateFromAscii(SBA_DATAEXCHANGE_FORMAT));
    if( !rTypeList.Get(nSbaDataExchangeFormat) )
        return sal_False;

    SvData aData(nSbaDataExchangeFormat);
    xDataObj->GetData(&aData);
    XubString sDataExchStr;
    if (!aData.GetData(sDataExchStr))
        return sal_False;

    if (!sDataExchStr.Len())
        return sal_False;

    // Formerly we just casted the xDataObj-ptr to SbaExplorerExchObj*. Unfortunally this isn't
    // valid anymore (it also seems to be very bad style to me)
    // Now xDataObj is an SfxExchangeObject, so we have to try other approches ....
    // FS - 69292 - 20.10.99

//  SbaExplorerExchObj* pDataExchObj = NULL;
//  // so first we try our new internal format which we created for this situation
//  SvData aExplorerExchangeTransfer(Exchange::RegisterFormatName(String::CreateFromAscii(SBA_DATATYPE_INTERNAL_EXPLOREREXCHANGE)));
//  if (xDataObj->GetData(&aExplorerExchangeTransfer))
//  {
//      DBG_ASSERT(aExplorerExchangeTransfer.GetMemorySize() == sizeof(pDataExchObj), "SbaGridControl::Drop : somebody gave me invalida data !");
//      void* pDestination = &pDataExchObj;
//      aExplorerExchangeTransfer.GetData(&pDestination);
//  }
//
//  // next try for a SfxExchangeObject
//  // (this is already somewhat dirty : we need a hard cast below ...)
//  if (!pDataExchObj)
//  {
//      SfxExchangeObjectRef aSfxExchange = SfxExchangeObject::PasteDragServer((DropEvent&)rEvt);
//      if (&aSfxExchange == &xDataObj)
//      {
//          // loop through all objects
//          for (sal_uInt32 i=0; i<aSfxExchange->Count(); ++i)
//          {
//              if (aSfxExchange->GetObject(i)->HasFormat(SvDataType(nSbaDataExchangeFormat)))
//              {
//                  pDataExchObj = (SbaExplorerExchObj*)aSfxExchange->GetObject(i);
//                  break;
//              }
//          }
//          DBG_ASSERT(pDataExchObj, "SbaGridControl::Drop : invalid SfxExchangeObject !?");
//              // if the SfxExchangeObject itself has the format, at least one of it children should, too
//      }
//  }
//
//  // last : the (real dirty :) old behaviuor : a hard cast
//  if (!pDataExchObj)
//      pDataExchObj = (SbaExplorerExchObj*)&xDataObj;
//
//  // first we need to translate the ::com::sun::star::form::DataSelectionType into a DBObject
//  sal_Int32 nDataType;
//  try
//  {
//      nDataType = ::comphelper::getINT32(xDataSource->getPropertyValue(PROPERTY_COMMANDTYPE));
//  }
//  catch(Exception&)
//  {
//      DBG_ERROR("SbaGridControl::Drop : could not collect essential data source attributes !");
//      return sal_False;
//  }


//  if (pDataExchObj->QueryInsertObject(eObj))
//  {
//      if (pDataExchObj->GetData(getDataSource()))
//          refresh();
//          // if the GetData would work with our own cursor, not with a newly created one, UpdateDataSource(sal_True) would be
//          // suffient (me thinks)
//      return sal_True;
//  }
    return sal_False;
}

//------------------------------------------------------------------------------
void SbaGridControl::refresh()
{
    // aktualisieren
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XLoadable >  xLoadable(getDataSource(), ::com::sun::star::uno::UNO_QUERY);
    DBG_ASSERT(xLoadable.is(), "SbaGridControl::Drop : invalid data source !");
    if (xLoadable.is())
    {
        WaitObject aWO(this);
        xLoadable->reload();
    }
}

//------------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  SbaGridControl::getDataSource() const
{
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xReturn;

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XChild >  xColumns(GetPeer()->getColumns(), ::com::sun::star::uno::UNO_QUERY);
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xDataSource;
    if (xColumns.is())
        xReturn = ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > (xColumns->getParent(), ::com::sun::star::uno::UNO_QUERY);

    return xReturn;
}



//------------------------------------------------------------------------------
::com::sun::star::awt::FontDescriptor BuildFontFromItems(const SfxItemSet* pAttr, const Font& rBase)
{
    Font aReturn(rBase);

//  const SfxPoolItem* pAnyItem;
//
//  pAnyItem = &pAttr->Get( SBA_DEF_FONT );
//  const SvxFontItem* pFontItem = (SvxFontItem*)pAnyItem;
//
//  pAnyItem = &pAttr->Get( SBA_DEF_FONTWEIGHT );
//  const SvxWeightItem* pWeightItem = (SvxWeightItem*)pAnyItem;
//
//  pAnyItem = &pAttr->Get( SBA_DEF_FONTHEIGHT );
//  const SvxFontHeightItem* pFontHeightItem = (SvxFontHeightItem*)pAnyItem;
//
//  pAnyItem = &pAttr->Get( SBA_DEF_POSTURE );
//  const SvxPostureItem* pFontItalicItem = (SvxPostureItem*)pAnyItem;
//
//  pAnyItem = &pAttr->Get( SBA_DEF_SHADOWED );
//  const SvxShadowedItem* pFontShadowItem = (SvxShadowedItem*)pAnyItem;
//
//  pAnyItem = &pAttr->Get( SBA_DEF_CONTOUR );
//  const SvxContourItem* pFontContourItem = (SvxContourItem*)pAnyItem;
//
//  pAnyItem = &pAttr->Get( SBA_DEF_UNDERLINE );
//  const SvxUnderlineItem* pFontUnderlineItem = (SvxUnderlineItem*)pAnyItem;
//
//  pAnyItem = &pAttr->Get( SBA_DEF_CROSSEDOUT );
//  const SvxCrossedOutItem* pFontCrossedOutItem = (SvxCrossedOutItem*)pAnyItem;
//
//  // build the returned font
//  aReturn.SetFamily( pFontItem->GetFamily() );
//  aReturn.SetName( pFontItem->GetFamilyName() );
//  aReturn.SetStyleName( pFontItem->GetStyleName() );
//  aReturn.SetCharSet( pFontItem->GetCharSet() );
//
//  aReturn.SetWeight( pWeightItem->GetWeight() );
//  aReturn.SetItalic( pFontItalicItem->GetPosture() );
//  aReturn.SetUnderline( pFontUnderlineItem->GetUnderline() );
//
//  aReturn.SetShadow( pFontShadowItem->GetValue() );
//  aReturn.SetOutline( pFontContourItem->GetValue() );
//  aReturn.SetStrikeout( pFontCrossedOutItem->GetStrikeout() );
//
//  sal_uInt32 nHeight = pFontHeightItem->GetHeight();
//
//  Size aSize(Size(0, nHeight));
//  aSize = OutputDevice::LogicToLogic(aSize, MAP_TWIP, MAP_POINT);
//  aReturn.SetSize(aSize);
//
//  // now create a font descriptor
    ::com::sun::star::awt::FontDescriptor aRealReturn;
//  aRealReturn.Name = aReturn.GetName();
//  aRealReturn.StyleName = aReturn.GetStyleName();
//  aRealReturn.Height = aReturn.GetSize().Height();
//  aRealReturn.Width = aReturn.GetSize().Width();
//  aRealReturn.Family = aReturn.GetFamily();
//  aRealReturn.CharSet = aReturn.GetCharSet();
//  aRealReturn.Pitch = aReturn.GetPitch();
//  aRealReturn.CharacterWidth = VCLUnoHelper::ConvertFontWidth(aReturn.GetWidthType());
//  aRealReturn.Weight= VCLUnoHelper::ConvertFontWeight(aReturn.GetWeight());
//  aRealReturn.Slant = (::com::sun::star::awt::FontSlant)aReturn.GetItalic();
//  aRealReturn.Underline = aReturn.GetUnderline();
//  aRealReturn.Strikeout = aReturn.GetStrikeout();
//  aRealReturn.Orientation = aReturn.GetOrientation();
//  aRealReturn.Kerning = aReturn.IsKerning();
//  aRealReturn.WordLineMode = aReturn.IsWordLineMode();
//  aRealReturn.Type = 0;
    return aRealReturn;
}

//------------------------------------------------------------------------------
void BuildItemsFromFont(SfxItemSet* pAttr, const ::com::sun::star::awt::FontDescriptor& rFont)
{
    // the following items are not aupported by the FontDescriptor structure
//  pAttr->Put(SvxContourItem(rFont.IsOutline(), SBA_DEF_CONTOUR));
//  pAttr->Put(SvxShadowedItem(rFont.IsShadow(), SBA_DEF_SHADOWED));

//  pAttr->Put(SvxWeightItem(VCLUnoHelper::ConvertFontWeight(rFont.Weight), SBA_DEF_FONTWEIGHT));
//  pAttr->Put(SvxPostureItem((FontItalic)rFont.Slant, SBA_DEF_POSTURE));
//  pAttr->Put(SvxUnderlineItem((FontUnderline)rFont.Underline, SBA_DEF_UNDERLINE));
//  pAttr->Put(SvxCrossedOutItem((FontStrikeout)rFont.Strikeout, SBA_DEF_CROSSEDOUT));
//
//  Size aSize(0, rFont.Height);
//  aSize = OutputDevice::LogicToLogic(aSize, MAP_POINT, MAP_TWIP);
//  pAttr->Put(SvxFontHeightItem(aSize.Height(), 100, SBA_DEF_FONTHEIGHT));
//
//  pAttr->Put(SvxFontItem((FontFamily)rFont.Family, rFont.Name, rFont.StyleName,
//      PITCH_DONTKNOW, (CharSet)rFont.CharSet, SBA_DEF_FONT));
}
// -------------------------------------------------------------------------




