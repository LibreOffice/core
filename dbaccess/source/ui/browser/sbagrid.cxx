/*************************************************************************
 *
 *  $RCSfile: sbagrid.cxx,v $
 *
 *  $Revision: 1.70 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 17:17:31 $
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

#ifndef DBACCESS_SBA_GRID_HRC
#include "sbagrid.hrc"
#endif

#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif

#define ITEMID_HORJUSTIFY       SID_ATTR_ALIGN_HOR_JUSTIFY
#define ITEMID_VERJUSTIFY       SID_ATTR_ALIGN_VER_JUSTIFY
//#define ITEMID_ORIENTATION     SID_ATTR_ALIGN_ORIENTATION
#define ITEMID_LINEBREAK        SID_ATTR_ALIGN_LINEBREAK
#define ITEMID_MARGIN           SID_ATTR_ALIGN_MARGIN
#define ITEMID_NUMBERINFO       SID_ATTR_NUMBERFORMAT_INFO


#define _ZFORLIST_DECLARE_TABLE
#ifndef _SVX_NUMINF_HXX
#include <svx/numinf.hxx>
#endif
#ifndef _EEITEMID_HXX
#include <svx/eeitemid.hxx>
#endif
#ifndef _SVX_DBAEXCHANGE_HXX_
#include <svx/dbaexchange.hxx>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_XEXECUTABLEDIALOG_HPP_
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#endif

#ifndef _SBA_GRID_HXX
#include "sbagrid.hxx"
#endif
#ifndef DBAUI_SBATTRDLG_HXX
#include "dlgattr.hxx"
#endif
#ifndef _DBAUI_DLGSIZE_HXX
#include "dlgsize.hxx"
#endif
#ifndef _COM_SUN_STAR_FORM_XLOADABLE_HPP_
#include <com/sun/star/form/XLoadable.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_COMMANDTYPE_HPP_
#include <com/sun/star/sdb/CommandType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XSQLQUERYCOMPOSERFACTORY_HPP_
#include <com/sun/star/sdb/XSQLQueryComposerFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XRESULTSETACCESS_HPP_
#include <com/sun/star/sdb/XResultSetAccess.hpp>
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
#ifndef _COM_SUN_STAR_UTIL_DATE_HPP_
#include <com/sun/star/util/Date.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_TIME_HPP_
#include <com/sun/star/util/Time.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSETUPDATE_HPP_
#include <com/sun/star/sdbc/XResultSetUpdate.hpp>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

#ifndef _SFXINTITEM_HXX
#include <svtools/intitem.hxx>
#endif

#ifndef _SVX_ALGITEM_HXX //autogen
#include <svx/algitem.hxx>
#endif

#ifndef _SV_MULTISEL_HXX //autogen
#include <tools/multisel.hxx>
#endif

#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
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

#ifndef _SFXRNGITEM_HXX
#include <svtools/rngitem.hxx>
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
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _DBHELPER_DBCONVERSION_HXX_
#include <connectivity/dbconversion.hxx>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _RTL_UUID_H_
#include <rtl/uuid.h>
#endif
#ifndef _RTL_MEMORY_H_
#include <rtl/memory.h>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _SVX_DBEXCH_HRC
#include <svx/dbexch.hrc>
#endif
#ifndef _DBU_BRW_HRC_
#include "dbu_brw.hrc"
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
#ifndef _DBU_REGHELPER_HXX_
#include "dbu_reghelper.hxx"
#endif
#ifndef DBAUI_DBEXCHANGE_HXX
#include "dbexchange.hxx"
#endif
#ifndef DBAUI_TABLEROW_EXCHANGE_HXX
#include "TableRowExchange.hxx"
#endif
#ifndef DBAUI_TABLEROW_HXX
#include "TableRow.hxx"
#endif
#ifndef DBAUI_FIELDDESCRIPTIONS_HXX
#include "FieldDescriptions.hxx"
#endif
#ifndef _SVTOOLS_STRINGTRANSFER_HXX_
#include <svtools/stringtransfer.hxx>
#endif
#ifndef _VCL_STDTEXT_HXX
#include <vcl/stdtext.hxx>
#endif
#ifndef DBAUI_TOOLS_HXX
#include "UITools.hxx"
#endif
#ifndef DBAUI_TOKENWRITER_HXX
#include "TokenWriter.hxx"
#endif

using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::datatransfer;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::view;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::util;
using namespace ::dbaui;
using namespace ::dbtools;
using namespace ::svx;
using namespace ::svt;

extern "C" void SAL_CALL createRegistryInfo_SbaXGridControl()
{
    static OMultiInstanceAutoRegistration< SbaXGridControl > aAutoRegistration;
}
//-------------------------------------------------------------------------
::comphelper::StringSequence SAL_CALL SbaXGridControl::getSupportedServiceNames() throw()
{
    return getSupportedServiceNames_Static();
}
// -------------------------------------------------------------------------
Reference< XInterface > SAL_CALL SbaXGridControl::Create(const Reference<XMultiServiceFactory >& _rxFactory)
{
    return *(new SbaXGridControl(_rxFactory));
}

//------------------------------------------------------------------
String Any2String(const Any& rValue)
{
    String  aString;
    if (rValue.hasValue())
    {
        switch (rValue.getValueTypeClass())
        {
            case TypeClass_BOOLEAN:
                aString.CreateFromInt32((sal_Int32)(char)::comphelper::getBOOL(rValue));
                break;
            case TypeClass_CHAR:
                aString.CreateFromInt32((sal_Int32)::comphelper::getINT16(rValue));
                break;
            case TypeClass_STRING:
                aString  = (const sal_Unicode*)::comphelper::getString(rValue);
                break;
            case TypeClass_BYTE:
                aString.CreateFromInt32((char)::comphelper::getBOOL(rValue));
                break;
            case TypeClass_LONG:
                aString.CreateFromInt32(::comphelper::getINT32(rValue));
                break;
            case TypeClass_SHORT:
                aString.CreateFromInt32((sal_Int32)::comphelper::getINT16(rValue));
                break;
            case TypeClass_UNSIGNED_SHORT:
                aString.CreateFromInt32((sal_Int32)::comphelper::getINT16(rValue));
                break;
            case TypeClass_UNSIGNED_LONG:
                aString.CreateFromInt32((sal_Int32)::comphelper::getINT32(rValue));
                break;
            case TypeClass_VOID:
            default:
                ; // leerstring
        }
    }
    return aString;
}

//------------------------------------------------------------------
double Any2Double(const Any& rValue)
{
    double aRet = 0;
    if (rValue.hasValue())
    {
        switch (rValue.getValueTypeClass())
        {
            case TypeClass_BOOLEAN:
                aRet = ::comphelper::getBOOL(rValue);
                break;
            case TypeClass_CHAR:
                aRet = ::comphelper::getBOOL(rValue);
                break;
            case TypeClass_FLOAT:
                aRet  = ::comphelper::getFloat(rValue);
                break;
            case TypeClass_DOUBLE:
                aRet  = ::comphelper::getDouble(rValue);
                break;
            case TypeClass_LONG:
                aRet = ::comphelper::getINT32(rValue);
                break;
            case TypeClass_SHORT:
                aRet = ::comphelper::getINT16(rValue);
                break;
            case TypeClass_UNSIGNED_SHORT:
                aRet = ::comphelper::getINT16(rValue);
                break;
            case TypeClass_BYTE:
                aRet  = ::comphelper::getBOOL(rValue);
                break;
            case TypeClass_UNSIGNED_LONG:
                aRet = ::comphelper::getINT32(rValue);
                break;

            case TypeClass_VOID:
            default:
                {
                    // ripped these three routines somewhere ....
                    ::com::sun::star::util::Date        aDate;
                    ::com::sun::star::util::DateTime    aDateTime;
                    ::com::sun::star::util::Time        aTime;
                    if (rValue >>= aDate)
                        aRet =  ::dbtools::DBTypeConversion::toDouble(aDate);
                    else if (rValue >>= aTime)
                        aRet =  ::dbtools::DBTypeConversion::toDouble(aTime);
                    else if (rValue >>= aDateTime)
                        aRet =  dbtools::DBTypeConversion::toDouble(aDateTime);
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
Reference< XInterface > SAL_CALL SbaXGridControl_CreateInstance(const Reference< XMultiServiceFactory >& _rxFactory) throw( Exception )
{
    return *(new SbaXGridControl(_rxFactory));
}

//------------------------------------------------------------------------------
::rtl::OUString SAL_CALL SbaXGridControl::getImplementationName() throw()
{
    return getImplementationName_Static();
}

//------------------------------------------------------------------------------
::rtl::OUString SbaXGridControl::getImplementationName_Static() throw( RuntimeException )
{
    return ::rtl::OUString::createFromAscii("com.sun.star.comp.dbu.SbaXGridControl");
}

//------------------------------------------------------------------------------
Sequence< ::rtl::OUString> SbaXGridControl::getSupportedServiceNames_Static(void) throw( RuntimeException )
{
    Sequence< ::rtl::OUString> aSupported(3);
    aSupported[0] = ::rtl::OUString::createFromAscii("com.sun.star.form.control.InteractionGridControl");
    aSupported[1] = ::rtl::OUString::createFromAscii("com.sun.star.form.control.GridControl");
    aSupported[2] = ::rtl::OUString::createFromAscii("com.sun.star.awt.UnoControl");
    return aSupported;
}
DBG_NAME(SbaXGridControl );
//---------------------------------------------------------------------------------------
SbaXGridControl::SbaXGridControl(const Reference< XMultiServiceFactory >& _rM)
    : FmXGridControl(_rM)
{
    DBG_CTOR(SbaXGridControl ,NULL);
}

//---------------------------------------------------------------------------------------
SbaXGridControl::~SbaXGridControl()
{
    DBG_DTOR(SbaXGridControl ,NULL);
}

//---------------------------------------------------------------------------------------
FmXGridPeer* SbaXGridControl::imp_CreatePeer(Window* pParent)
{
    FmXGridPeer* pReturn = new SbaXGridPeer(m_xServiceFactory);

    // translate properties into WinBits
    WinBits nStyle = WB_TABSTOP;
    Reference< XPropertySet >  xModelSet(getModel(), UNO_QUERY);
    if (xModelSet.is())
    {
        try
        {
            if (::comphelper::getINT16(xModelSet->getPropertyValue(PROPERTY_BORDER)))
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
Any SAL_CALL SbaXGridControl::queryInterface(const Type& _rType) throw (RuntimeException)
{
    Any aRet = FmXGridControl::queryInterface(_rType);
    return aRet.hasValue() ? aRet : ::cppu::queryInterface(_rType,(::com::sun::star::frame::XDispatch*)this);
}

//------------------------------------------------------------------------------
Sequence< Type > SAL_CALL SbaXGridControl::getTypes(  ) throw (RuntimeException)
{
    Sequence< Type > aTypes = FmXGridControl::getTypes();

    sal_Int32 nTypes = aTypes.getLength();
    aTypes.realloc(nTypes + 1);
    aTypes[nTypes] = ::getCppuType(static_cast< Reference< ::com::sun::star::frame::XDispatch >* >(NULL));

    return aTypes;
}

//------------------------------------------------------------------------------
Sequence< sal_Int8 > SAL_CALL SbaXGridControl::getImplementationId(  ) throw (RuntimeException)
{
    static ::cppu::OImplementationId * pId = 0;
    if (! pId)
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if (! pId)
        {
            static ::cppu::OImplementationId aId;
            pId = &aId;
        }
    }
    return pId->getImplementationId();
}

//---------------------------------------------------------------------------------------
void SAL_CALL SbaXGridControl::createPeer(const Reference< ::com::sun::star::awt::XToolkit > & rToolkit, const Reference< ::com::sun::star::awt::XWindowPeer > & rParentPeer) throw( RuntimeException )
{
    FmXGridControl::createPeer(rToolkit, rParentPeer);

    DBG_ASSERT(/*(0 == m_nPeerCreationLevel) && */!mbCreatingPeer, "FmXGridControl::createPeer : recursion!");
        // see the base class' createPeer for a comment on this
        // 14.05.2001 - 86836 - frank.schoenheit@germany.sun.com

    // TODO: why the hell this whole class does not use any mutex?

//  if (0 == m_nPeerCreationLevel)
    {
        Reference< ::com::sun::star::frame::XDispatch >  xDisp(getPeer(), UNO_QUERY);
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
void SAL_CALL SbaXGridControl::dispatch(const ::com::sun::star::util::URL& aURL, const Sequence< PropertyValue >& aArgs) throw( RuntimeException )
{
    Reference< ::com::sun::star::frame::XDispatch >  xDisp(getPeer(), UNO_QUERY);
    if (xDisp.is())
        xDisp->dispatch(aURL, aArgs);
}
//---------------------------------------------------------------------------------------
void SAL_CALL SbaXGridControl::addStatusListener( const Reference< XStatusListener > & _rxListener, const URL& _rURL ) throw( RuntimeException )
{
    ::osl::MutexGuard aGuard( GetMutex() );
    if ( _rxListener.is() )
    {
        SbaXStatusMultiplexer*& pMultiplexer = m_aStatusMultiplexer[ _rURL ];
        if ( !pMultiplexer )
        {
            pMultiplexer = new SbaXStatusMultiplexer( *this, GetMutex() );
            pMultiplexer->acquire();
        }

        pMultiplexer->addInterface( _rxListener );
        if ( getPeer().is() )
        {
            if ( 1 == pMultiplexer->getLength() )
            {   // the first external listener for this URL
                Reference< XDispatch >  xDisp( getPeer(), UNO_QUERY );
                xDisp->addStatusListener( pMultiplexer, _rURL );
            }
            else
            {   // already have other listeners for this URL
                _rxListener->statusChanged( pMultiplexer->getLastEvent() );
            }
        }
    }
}

//---------------------------------------------------------------------------------------
void SAL_CALL SbaXGridControl::removeStatusListener(const Reference< ::com::sun::star::frame::XStatusListener > & _rxListener, const ::com::sun::star::util::URL& _rURL) throw( RuntimeException )
{
    ::osl::MutexGuard aGuard( GetMutex() );

    SbaXStatusMultiplexer*& pMultiplexer = m_aStatusMultiplexer[_rURL];
    if (!pMultiplexer)
    {
        pMultiplexer = new SbaXStatusMultiplexer(*this,GetMutex());
        pMultiplexer->acquire();
    }

    if (getPeer().is() && pMultiplexer->getLength() == 1)
    {
        Reference< ::com::sun::star::frame::XDispatch >  xDisp(getPeer(), UNO_QUERY);
        xDisp->removeStatusListener(pMultiplexer, _rURL);
    }
    pMultiplexer->removeInterface( _rxListener );
}

//---------------------------------------------------------------------------------------
void SAL_CALL SbaXGridControl::dispose(void) throw( RuntimeException )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    EventObject aEvt;
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
    StatusMultiplexerArray().swap(m_aStatusMultiplexer);

    FmXGridControl::dispose();
}

//=======================================================================================
// SbaXGridPeer
//=======================================================================================
DBG_NAME(SbaXGridPeer )
//---------------------------------------------------------------------------------------
SbaXGridPeer::SbaXGridPeer(const Reference< XMultiServiceFactory >& _rM)
: FmXGridPeer(_rM)
,m_aStatusListeners(m_aMutex)
{
    DBG_CTOR(SbaXGridPeer ,NULL);
}

//---------------------------------------------------------------------------------------
SbaXGridPeer::~SbaXGridPeer()
{
    DBG_DTOR(SbaXGridPeer ,NULL);
}

//---------------------------------------------------------------------------------------
void SAL_CALL SbaXGridPeer::dispose(void) throw( RuntimeException )
{
    EventObject aEvt(*this);

    m_aStatusListeners.disposeAndClear(aEvt);

    FmXGridPeer::dispose();
}

//---------------------------------------------------------------------------------------
void SbaXGridPeer::NotifyStatusChanged(const ::com::sun::star::util::URL& _rUrl, const Reference< ::com::sun::star::frame::XStatusListener > & xControl)
{
    SbaGridControl* pGrid = (SbaGridControl*) GetWindow();
    if (!pGrid)
        return;

    ::com::sun::star::frame::FeatureStateEvent aEvt;
    aEvt.Source = *this;
    aEvt.IsEnabled = !pGrid->IsReadOnlyDB();
    aEvt.FeatureURL = _rUrl;

    ConstMapDispatchToBoolIterator aURLStatePos = m_aDispatchStates.find( classifyDispatchURL( _rUrl ) );
    if ( m_aDispatchStates.end() != aURLStatePos )
        aEvt.State <<= aURLStatePos->second;
    else
        aEvt.State <<= sal_False;

    if (xControl.is())
        xControl->statusChanged(aEvt);
    else
    {
        ::cppu::OInterfaceContainerHelper * pIter = m_aStatusListeners.getContainer(_rUrl);

        if (pIter)
        {
            ::cppu::OInterfaceIteratorHelper aListIter(*pIter);
            while (aListIter.hasMoreElements())
                ((::com::sun::star::frame::XStatusListener*)aListIter.next())->statusChanged(aEvt);
        }
    }
}

//------------------------------------------------------------------------------
Any SAL_CALL SbaXGridPeer::queryInterface(const Type& _rType) throw (RuntimeException)
{
    Any aRet = ::cppu::queryInterface(_rType,(::com::sun::star::frame::XDispatch*)this);
    if(aRet.hasValue())
        return aRet;
    return FmXGridPeer::queryInterface(_rType);
}

//---------------------------------------------------------------------------------------
Reference< ::com::sun::star::frame::XDispatch >  SAL_CALL SbaXGridPeer::queryDispatch(const ::com::sun::star::util::URL& aURL, const ::rtl::OUString& aTargetFrameName, sal_Int32 nSearchFlags) throw( RuntimeException )
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
IMPL_LINK( SbaXGridPeer, OnDispatchEvent, void*, NOTINTERESTEDIN )
{
    SbaGridControl* pGrid = static_cast< SbaGridControl* >( GetWindow() );
    if ( pGrid )    // if this fails, we were disposing before arriving here
    {
        if ( Application::GetMainThreadIdentifier() != ::vos::OThread::getCurrentIdentifier() )
        {
            // still not in the main thread (see SbaXGridPeer::dispatch). post an event, again
            // without moving the special even to the back of the queue
            pGrid->PostUserEvent( LINK( this, SbaXGridPeer, OnDispatchEvent ) );
        }
        else
        {
            DispatchArgs aArgs = m_aDispatchArgs.front();
            m_aDispatchArgs.pop();

            SbaXGridPeer::dispatch( aArgs.aURL, aArgs.aArgs );
        }
    }

    return 0;
}

//---------------------------------------------------------------------------------------
SbaXGridPeer::DispatchType SbaXGridPeer::classifyDispatchURL( const URL& _rURL )
{
    DispatchType eURLType = dtUnknown;
    if ( _rURL.Complete.equalsAscii( ".uno:GridSlots/BrowserAttribs" ) )
        eURLType = dtBrowserAttribs;
    else if ( _rURL.Complete.equalsAscii( ".uno:GridSlots/RowHeight" ) )
        eURLType = dtRowHeight;
    else if ( _rURL.Complete.equalsAscii( ".uno:GridSlots/ColumnAttribs" ) )
        eURLType = dtColumnAttribs;
    else if ( _rURL.Complete.equalsAscii( ".uno:GridSlots/ColumnWidth" ) )
        eURLType = dtColumnWidth;
    return eURLType;
}

//---------------------------------------------------------------------------------------
void SAL_CALL SbaXGridPeer::dispatch(const URL& aURL, const Sequence< PropertyValue >& aArgs) throw( RuntimeException )
{
    SbaGridControl* pGrid = (SbaGridControl*)GetWindow();
    if (!pGrid)
        return;

    if ( Application::GetMainThreadIdentifier() != ::vos::OThread::getCurrentIdentifier() )
    {
        // we're not in the main thread. This is bad, as we want to raise windows here,
        // and VCL does not like windows to be opened in non-main threads (at least on Win32).
        // Okay, do this async. No problem with this, as XDispatch::dispatch is defined to be
        // a one-way method.

        // save the args
        DispatchArgs aDispatchArgs;
        aDispatchArgs.aURL = aURL;
        aDispatchArgs.aArgs = aArgs;
        m_aDispatchArgs.push( aDispatchArgs );

        // post an event
        // we use the Window::PostUserEvent here, instead of the application::PostUserEvent
        // this saves us from keeping track of these events - as soon as the window dies,
        // the events are deleted automatically. For the application way, we would need to
        // do this ourself.
        // As we use our grid as window, and the grid dies before we dy, this should be no problem.
        pGrid->PostUserEvent( LINK( this, SbaXGridPeer, OnDispatchEvent ) );
        return;
    }

    ::vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Int16 nColId = -1;
    const PropertyValue* pArgs = aArgs.getConstArray();
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

    DispatchType eURLType = classifyDispatchURL( aURL );

    if ( dtUnknown != eURLType )
    {
        // notify any status listeners that the dialog is now active (well, about to be active)
        MapDispatchToBool::iterator aThisURLState = m_aDispatchStates.insert( MapDispatchToBool::value_type( eURLType, sal_True ) ).first;
        NotifyStatusChanged( aURL, NULL );

        // execute the dialog
        switch ( eURLType )
        {
            case dtBrowserAttribs:
                pGrid->SetBrowserAttrs();
                break;

            case dtRowHeight:
                pGrid->SetRowHeight();
                break;

            case dtColumnAttribs:
            {
                DBG_ASSERT(nColId != -1, "SbaXGridPeer::dispatch : invalid parameter !");
                if (nColId != -1)
                    break;
                pGrid->SetColAttrs(nColId);
            }
            break;

            case dtColumnWidth:
            {
                DBG_ASSERT(nColId != -1, "SbaXGridPeer::dispatch : invalid parameter !");
                if (nColId != -1)
                    break;
                pGrid->SetColWidth(nColId);
            }
            break;
        }

        // notify any status listeners that the dialog vanished
        m_aDispatchStates.erase( aThisURLState );
        NotifyStatusChanged( aURL, NULL );
    }
}

//---------------------------------------------------------------------------------------
void SAL_CALL SbaXGridPeer::addStatusListener(const Reference< ::com::sun::star::frame::XStatusListener > & xControl, const ::com::sun::star::util::URL& aURL) throw( RuntimeException )
{
    ::cppu::OInterfaceContainerHelper* pCont = m_aStatusListeners.getContainer(aURL);
    if (!pCont)
        m_aStatusListeners.addInterface(aURL,xControl);
    else
        pCont->addInterface(xControl);
    NotifyStatusChanged(aURL, xControl);
}

//---------------------------------------------------------------------------------------
void SAL_CALL SbaXGridPeer::removeStatusListener(const Reference< ::com::sun::star::frame::XStatusListener > & xControl, const ::com::sun::star::util::URL& aURL) throw( RuntimeException )
{
    ::cppu::OInterfaceContainerHelper* pCont = m_aStatusListeners.getContainer(aURL);
    if ( pCont )
        pCont->removeInterface(xControl);
}

//---------------------------------------------------------------------------------------
const Sequence< sal_Int8 > & SbaXGridPeer::getUnoTunnelId()
{
    static Sequence< sal_Int8 > * pSeq = 0;
    if( !pSeq )
    {
        ::osl::Guard< ::osl::Mutex > aGuard( ::osl::Mutex::getGlobalMutex() );
        if( !pSeq )
        {
            static Sequence< sal_Int8 > aSeq( 16 );
                rtl_createUuid( (sal_uInt8*)aSeq.getArray(), 0,sal_True );
                pSeq = &aSeq;
        }
    }
    return *pSeq;
}

//---------------------------------------------------------------------------------------
Sequence< Type > SAL_CALL SbaXGridPeer::getTypes() throw (RuntimeException)
{
    Sequence< Type > aTypes = FmXGridPeer::getTypes();
    sal_Int32 nOldLen = aTypes.getLength();
    aTypes.realloc(nOldLen + 1);
    aTypes.getArray()[nOldLen] = ::getCppuType( reinterpret_cast< Reference< ::com::sun::star::frame::XDispatch >* >(NULL) );

    return aTypes;
}

// return implementation specific data
//------------------------------------------------------------------
sal_Int64 SAL_CALL SbaXGridPeer::getSomething( const Sequence< sal_Int8 > & rId ) throw(::com::sun::star::uno::RuntimeException)
{
    if( rId.getLength() == 16 && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),  rId.getConstArray(), 16 ) )
        return (sal_Int64)this;

    return FmXGridPeer::getSomething(rId);
}

//---------------------------------------------------------------------------------------
SbaXGridPeer* SbaXGridPeer::getImplementation(const Reference< XInterface >& _rxIFace)
{
    Reference< XUnoTunnel > xTunnel(
        _rxIFace, UNO_QUERY);
    if (xTunnel.is())
        return reinterpret_cast<SbaXGridPeer*>(xTunnel->getSomething(getUnoTunnelId()));
    return NULL;
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
    ,DragSourceHelper(this)
{
}

//---------------------------------------------------------------------------------------
void SbaGridHeader::StartDrag( sal_Int8 _nAction, const Point& _rPosPixel )
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
        // in the new DnD API, the solar mutex is not locked when StartDrag get's called

    ImplStartColumnDrag( _nAction, _rPosPixel );
}

//---------------------------------------------------------------------------------------
void SbaGridHeader::MouseButtonDown( const MouseEvent& _rMEvt )
{
    if (_rMEvt.IsLeft())
        if (_rMEvt.GetClicks() != 2)
        {
            // the base class will start a column move here, which we don't want to allow
            // (at the moment. If we store relative positions with the columns, we can allow column moves ....)

//          sal_uInt16  nPos(0);
//          sal_uInt16  nHitTest = ImplHitTest( _rMEvt.GetPosPixel(), mnMouseOff, nPos );
//          if (!nHitTest & HEAD_HITTEST_DIVIDER)
//              return;
        }

    FmGridHeader::MouseButtonDown(_rMEvt);
}

//---------------------------------------------------------------------------------------
sal_Bool SbaGridHeader::ImplStartColumnDrag(sal_Int8 _nAction, const Point& _rMousePos)
{
    sal_uInt16 nId = GetItemId(_rMousePos);
    sal_Bool bResizingCol = sal_False;
    if (HEADERBAR_ITEM_NOTFOUND != nId)
    {
        Rectangle aColRect = GetItemRect(nId);
        aColRect.Left() += nId ? 3 : 0; // the handle col (nId == 0) does not have a left margin for resizing
        aColRect.Right() -= 3;
        bResizingCol = !aColRect.IsInside(_rMousePos);
    }
    if (!bResizingCol)
    {
        // force the the base class to end it's drag mode
        EndTracking(ENDTRACK_CANCEL | ENDTRACK_END);

        // because we have 3d-buttons the select handler is called from MouseButtonUp, but StartDrag
        // occures earlier (while the mouse button is down)
        // so for optical reasons we select the column before really starting the drag operation.
        notifyColumnSelect(nId);

        static_cast<SbaGridControl*>(GetParent())->StartDrag(_nAction,
                Point(
                    _rMousePos.X() + GetPosPixel().X(),     // we aren't left-justified with our parent, in contrast to the data window
                    _rMousePos.Y() - GetSizePixel().Height()
                )
            );
        return sal_True;
    }

    return sal_False;
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
    sal_Bool bColAttrs = (nColId != (sal_uInt16)-1) && (nColId != 0);
    if ( bColAttrs && !bDBIsReadOnly)
    {
        PopupMenu aNewItems(ModuleRes(RID_SBA_GRID_COLCTXMENU));
        sal_uInt16 nPos = 0;
        sal_uInt16 nModelPos = ((SbaGridControl*)GetParent())->GetModelColumnPos(nColId);
        Reference< XPropertySet >  xField = ((SbaGridControl*)GetParent())->getField(nModelPos);

        if ( xField.is() )
        {
            switch( ::comphelper::getINT32(xField->getPropertyValue(PROPERTY_TYPE)) )
            {
            case DataType::BINARY:
            case DataType::VARBINARY:
            case DataType::LONGVARBINARY:
            case DataType::SQLNULL:
            case DataType::OBJECT:
            case DataType::BLOB:
            case DataType::CLOB:
            case DataType::REF:
                break;
            default:
                rMenu.InsertItem(ID_BROWSER_COLATTRSET, aNewItems.GetItemText(ID_BROWSER_COLATTRSET), 0, nPos++);
                rMenu.SetHelpId(ID_BROWSER_COLATTRSET, aNewItems.GetHelpId(ID_BROWSER_COLATTRSET));
                rMenu.InsertSeparator(nPos++);
            }
        }

        rMenu.InsertItem(ID_BROWSER_COLWIDTH, aNewItems.GetItemText(ID_BROWSER_COLWIDTH), 0, nPos++);
        rMenu.SetHelpId(ID_BROWSER_COLWIDTH, aNewItems.GetHelpId(ID_BROWSER_COLWIDTH));
        rMenu.InsertSeparator(nPos++);
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
        case ID_BROWSER_COLUMNINFO:
            {
                sal_uInt16 nModelPos = ((SbaGridControl*)GetParent())->GetModelColumnPos(nColId);
                Reference< XPropertySet >  xField = ((SbaGridControl*)GetParent())->getField(nModelPos);

                if(!xField.is())
                    break;
                ::std::vector<OTableRow*> vClipboardList;
                OTableRow* pTableRow = new OTableRow(xField);
                // send it to the clipboard
                vClipboardList.push_back(pTableRow);
                OTableRowExchange* pData = new OTableRowExchange(vClipboardList);
                Reference< ::com::sun::star::datatransfer::XTransferable> xRef = pData;
#if SUPD<631
                pData->CopyToClipboard();
#else
                pData->CopyToClipboard(GetParent());
#endif
            }
            break;

        default: FmGridHeader::PostExecuteColumnContextMenu(nColId, rMenu, nExecutionResult);
    }
}

//==================================================================
// SbaGridControl
//==================================================================
DBG_NAME(SbaGridControl );
//---------------------------------------------------------------------------------------
SbaGridControl::SbaGridControl(Reference< XMultiServiceFactory > _rM,
                               Window* pParent, FmXGridPeer* _pPeer, WinBits nBits)
    :FmGridControl(_rM,pParent, _pPeer, nBits)
    ,m_nLastColId(-1)
    ,m_nLastRowId(-1)
    ,m_nCurrentActionColId(-1)
    ,m_pMasterListener(NULL)
    ,m_bActivatingForDrop(sal_False)
    ,m_nAsyncDropEvent(0)
{
    DBG_CTOR(SbaGridControl ,NULL);
}

//---------------------------------------------------------------------------------------
SbaGridControl::~SbaGridControl()
{
    DBG_DTOR(SbaGridControl ,NULL);
    if (m_nAsyncDropEvent)
        Application::RemoveUserEvent(m_nAsyncDropEvent);
}

//---------------------------------------------------------------------------------------
BrowserHeader* SbaGridControl::imp_CreateHeaderBar(BrowseBox* pParent)
{
    return new SbaGridHeader(pParent);
}

//---------------------------------------------------------------------------------------
CellController* SbaGridControl::GetController(long nRow, sal_uInt16 nCol)
{
    CellControllerRef aController;
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
    Reference< ::com::sun::star::util::XNumberFormatsSupplier >  xSupplier = ::dbtools::getNumberFormats(::dbtools::getConnection(Reference< XRowSet > (getDataSource(),UNO_QUERY)), sal_True,getServiceManager());

    Reference< XUnoTunnel > xTunnel(xSupplier,UNO_QUERY);
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
    Reference< XIndexAccess >  xCols(GetPeer()->getColumns(), UNO_QUERY);
    Reference< XPropertySet >  xAffectedCol;
    if (xCols.is() && (nModelPos != (sal_uInt16)-1))
        ::cppu::extractInterface(xAffectedCol,xCols->getByIndex(nModelPos));

    if (xAffectedCol.is())
    {
        Any aWidth = xAffectedCol->getPropertyValue(PROPERTY_WIDTH);
        sal_Int32 nCurWidth = aWidth.hasValue() ? ::comphelper::getINT32(aWidth) : -1;

        DlgSize aDlgColWidth(this, nCurWidth, sal_False);
        if (aDlgColWidth.Execute())
        {
            sal_Int32 nValue = aDlgColWidth.GetValue();
            Any aNewWidth;
            if (-1 == nValue)
            {   // set to default
                Reference< XPropertyState >  xPropState(xAffectedCol, UNO_QUERY);
                if (xPropState.is())
                {
                    try { aNewWidth = xPropState->getPropertyDefault(PROPERTY_WIDTH); } catch(Exception&) { } ;
                }
            }
            else
                aNewWidth <<= nValue;
            try {  xAffectedCol->setPropertyValue(PROPERTY_WIDTH, aNewWidth); } catch(Exception&) { } ;
        }
    }
}

//------------------------------------------------------------------------------
void SbaGridControl::SetRowHeight()
{
    Reference< XPropertySet >  xCols(GetPeer()->getColumns(), UNO_QUERY);
    if (!xCols.is())
        return;

    Any aHeight = xCols->getPropertyValue(PROPERTY_ROW_HEIGHT);
    sal_Int32 nCurHeight = aHeight.hasValue() ? ::comphelper::getINT32(aHeight) : -1;

    DlgSize aDlgRowHeight(this, nCurHeight, sal_True);
    if (aDlgRowHeight.Execute())
    {
        sal_Int32 nValue = aDlgRowHeight.GetValue();
        Any aNewHeight;
        if ((sal_Int16)-1 == nValue)
        {   // set to default
            Reference< XPropertyState >  xPropState(xCols, UNO_QUERY);
            if (xPropState.is())
            {
                try
                {
                    aNewHeight = xPropState->getPropertyDefault(PROPERTY_ROW_HEIGHT);
                }
                catch(Exception&)
                { }
            }
        }
        else
            aNewHeight <<= nValue;
        try
        {
            xCols->setPropertyValue(PROPERTY_ROW_HEIGHT, aNewHeight);
        }
        catch(Exception&)
        {
            OSL_ENSURE(0,"setPropertyValue: PROPERTY_ROW_HEIGHT throws a exception");
        }
    }
}

//------------------------------------------------------------------------------
void SbaGridControl::SetColAttrs(sal_uInt16 nColId)
{
    SvNumberFormatter* pFormatter = GetDatasourceFormatter();
    if (!pFormatter)
        return;

    sal_uInt16 nModelPos = GetModelColumnPos(nColId);

    // get the (UNO) column model
    Reference< XIndexAccess >  xCols(GetPeer()->getColumns(), UNO_QUERY);
    Reference< XPropertySet >  xAffectedCol;
    if (xCols.is() && (nModelPos != (sal_uInt16)-1))
        ::cppu::extractInterface(xAffectedCol,xCols->getByIndex(nModelPos));

    // get the field the column is bound to
    Reference< XPropertySet >  xField = getField(nModelPos);
    ::dbaui::callColumnFormatDialog(xAffectedCol,xField,pFormatter,this);//(Window::GetSettings().GetLanguage());
}


//------------------------------------------------------------------------------
void SbaGridControl::SetBrowserAttrs()
{
    Reference< XPropertySet >  xGridModel(GetPeer()->getColumns(), UNO_QUERY);
    if (!xGridModel.is())
        return;

    try
    {
        PropertyValue aArg;
        aArg.Name = ::rtl::OUString::createFromAscii("IntrospectedObject");
        aArg.Value <<= xGridModel;
        Sequence< Any > aDialogArgs(1);
        aDialogArgs[0] <<= aArg;

        Reference< XInterface > xDialog = getServiceManager()->createInstanceWithArguments(
            ::rtl::OUString::createFromAscii("com.sun.star.form.ControlFontDialog"),
            aDialogArgs
            );
        if (!xDialog.is())
        {
            ShowServiceNotAvailableError(this, String::CreateFromAscii("com.sun.star.form.ControlFontDialog"), sal_True);
            return;
        }

        Reference< XExecutableDialog > xExecute(xDialog, UNO_QUERY);
        OSL_ENSURE(xExecute.is(), "SbaGridControl::SetBrowserAttrs: missing an interface on the dialog!");
        if (xExecute.is())
            xExecute->execute();
    }
    catch(const Exception&)
    {
        OSL_ENSURE(sal_False, "SbaGridControl::SetBrowserAttrs: caught an exception while creating/executing the dialog!");
    }
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
    m_nLastRowId = (sal_uInt16)GetCurRow();
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
//---------------------------------------------------------------------------------------
void SbaGridControl::BeforeDrop()
{
    if (m_pMasterListener)
        m_pMasterListener->BeforeDrop();
}
//---------------------------------------------------------------------------------------
void SbaGridControl::AfterDrop()
{
    if (m_pMasterListener)
        m_pMasterListener->AfterDrop();
}


//------------------------------------------------------------------------------
Reference< XPropertySet >  SbaGridControl::getField(sal_uInt16 nModelPos)
{
    Reference< XPropertySet >  xEmptyReturn;
    try
    {
        // first get the name of the column
        Reference< XIndexAccess >  xCols(GetPeer()->getColumns(), UNO_QUERY);
        if ( xCols.is() && xCols->getCount() > nModelPos )
        {
            Reference< XPropertySet >  xCol(xCols->getByIndex(nModelPos),UNO_QUERY);
            if ( xCol.is() )
                xEmptyReturn.set(xCol->getPropertyValue(PROPERTY_BOUNDFIELD),UNO_QUERY);
        }
        else
            OSL_ENSURE(0,"SbaGridControl::getField getColumns returns NULL or ModelPos is > than count!");
    }
    catch(Exception&)
    {
        OSL_ENSURE(0,"SbaGridControl::getField Exception occured!");
    }

    return xEmptyReturn;
}

//---------------------------------------------------------------------------------------
sal_Bool SbaGridControl::IsReadOnlyDB() const
{
    // assume yes if anything fails
    sal_Bool bDBIsReadOnly = sal_True;

    // the db is the implemented by the parent of the grid control's model ...
    Reference< XChild >  xColumns(GetPeer()->getColumns(), UNO_QUERY);
    if (xColumns.is())
    {
        Reference< XRowSet >  xDataSource(xColumns->getParent(), UNO_QUERY);
        Reference< XChild >  xConn(::dbtools::getConnection(xDataSource),UNO_QUERY);
        if (xConn.is())
        {
            // ... and the RO-flag simply is implemented by a property
            Reference< XPropertySet >  xDbProps(xConn->getParent(), UNO_QUERY);
            if (xDbProps.is())
            {
                Reference< XPropertySetInfo >  xInfo = xDbProps->getPropertySetInfo();
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

//---------------------------------------------------------------------------------------
void SbaGridControl::StartDrag( sal_Int8 _nAction, const Point& _rPosPixel )
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
        // in the new DnD API, the solar mutex is not locked when StartDrag get's called

    sal_Bool bHandled = sal_False;

    do
    {
        // determine if dragging is allowed
        // (Yes, this is controller (not view) functionality. But collecting and evaluating all the
        // informations necessary via UNO would be quite difficult (if not impossible) so
        // my laziness says 'do it here' ...)
        long nRow = GetRowAtYPosPixel(_rPosPixel.Y());
        sal_uInt16 nColPos = GetColumnAtXPosPixel(_rPosPixel.X());
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
                                                    // AND
            &&  (   GetSelectRowCount()                     //  at least one row is selected
                                                        // OR
                ||  (   (nRow >= 0)                         //  a row below the header
                    &&  !bCurrentRowVirtual                 //  we aren't appending a new record
                    &&  (nRow != GetCurrentPos())           //  a row which is not the current one
                    )                                   // OR
                ||  (   (0 == GetSelectRowCount())          // no rows selected
                    &&  (-1 == nRow)                        // hit the header
                    )
                )
            )
        {   // => start dragging the row
            if (GetDataWindow().IsMouseCaptured())
                GetDataWindow().ReleaseMouse();

            if (0 == GetSelectRowCount())
                // no rows selected, but here in this branch
                // -> the user started dragging the upper left corner, which symbolizes the whole table
                SelectAll();

            getMouseEvent().Clear();
            DoRowDrag((sal_Int16)nRow);

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
            DoFieldDrag(nViewPos, (sal_Int16)nRow);

            bHandled = sal_True;
        }
    }
    while (sal_False);

    if (!bHandled)
        FmGridControl::StartDrag(_nAction, _rPosPixel);
}

//------------------------------------------------------------------------------
void SbaGridControl::Command(const CommandEvent& rEvt)
{
    FmGridControl::Command(rEvt);
}

// -----------------------------------------------------------------------
void SbaGridControl::DoColumnDrag(sal_uInt16 nColumnPos)
{
    Reference< XPropertySet >  xDataSource(getDataSource(), UNO_QUERY);
    DBG_ASSERT(xDataSource.is(), "SbaGridControl::DoColumnDrag : invalid data source !");

    Reference< XPropertySet > xAffectedCol;
    Reference< XPropertySet > xAffectedField;
    Reference< XConnection > xActiveConnection;

    // determine the field to drag
    ::rtl::OUString sField;
    try
    {
        xActiveConnection = ::dbtools::getConnection(Reference< XRowSet >(getDataSource(),UNO_QUERY));

        sal_uInt16 nModelPos = GetModelColumnPos(GetColumnIdFromViewPos(nColumnPos));
        Reference< XIndexContainer >  xCols(GetPeer()->getColumns(), UNO_QUERY);
        xAffectedCol.set(xCols->getByIndex(nModelPos),UNO_QUERY);
        if (xAffectedCol.is())
        {
            xAffectedCol->getPropertyValue(PROPERTY_CONTROLSOURCE) >>= sField;
            xAffectedField.set(xAffectedCol->getPropertyValue(PROPERTY_BOUNDFIELD),UNO_QUERY);
        }
    }
    catch(Exception&)
    {
        DBG_ERROR("SbaGridControl::DoColumnDrag : something went wrong while getting the column");
    }
    if (0 == sField.getLength())
        return;

    OColumnTransferable* pDataTransfer = new OColumnTransferable(xDataSource, sField, xAffectedField, xActiveConnection, CTF_FIELD_DESCRIPTOR | CTF_COLUMN_DESCRIPTOR);
    Reference< XTransferable > xEnsureDelete = pDataTransfer;
    pDataTransfer->StartDrag(this, DND_ACTION_COPY | DND_ACTION_LINK);
}

// -----------------------------------------------------------------------
void SbaGridControl::DoRowDrag(sal_Int16 nRowPos)
{
    Reference< XPropertySet >  xDataSource(getDataSource(), UNO_QUERY);
    DBG_ASSERT(xDataSource.is(), "SbaGridControl::DoRowDrag : invalid data source !");

    // build the sequence of numbers of selected rows
    Sequence< Any > aSelectedRows;

    // collect the affected rows
    if ((GetSelectRowCount() == 0) && (nRowPos >= 0))
    {
        aSelectedRows.realloc(1);
        aSelectedRows[0] <<= (sal_Int32)(nRowPos + 1);
    }
    else if ( !IsAllSelected() && GetSelectRowCount() )
    {
        aSelectedRows.realloc(GetSelectRowCount());
        Any* pSelectedRows = aSelectedRows.getArray();

        for (long nIdx = FirstSelectedRow();
             nIdx >= 0;
             nIdx = NextSelectedRow(), ++pSelectedRows)
        {
            (*pSelectedRows) <<= (sal_Int32)(nIdx + 1);
        }
    }

    Reference< XResultSet> xRowSetClone;
    try
    {
        Reference< XResultSetAccess > xResultSetAccess(xDataSource,UNO_QUERY);
        if ( xResultSetAccess.is() )
            xRowSetClone = xResultSetAccess->createResultSet();

        ODataClipboard* pTransfer = new ODataClipboard(xDataSource, aSelectedRows,xRowSetClone);

        Reference< XTransferable > xEnsureDelete = pTransfer;
        pTransfer->StartDrag(this, DND_ACTION_COPY | DND_ACTION_LINK);
    }
    catch(Exception&)
    {
    }
}

// -----------------------------------------------------------------------
void SbaGridControl::DoFieldDrag(sal_uInt16 nColumnPos, sal_Int16 nRowPos)
{
    // the only thing to do here is dragging the pure cell text
    // the old implementation copied a SBA_FIELDDATAEXCHANGE_FORMAT, too, (which was rather expensive to obtain),
    // but we have no client for this DnD format anymore (the mail part of SO 5.2 was the only client)

    ::rtl::OUString sCellText;
    try
    {
        Reference< XGridFieldDataSupplier >  xFieldData(static_cast< XGridPeer* >(GetPeer()), UNO_QUERY);
        Sequence<sal_Bool> aSupportingText = xFieldData->queryFieldDataType(::getCppuType(&sCellText));
        if (aSupportingText.getConstArray()[nColumnPos])
        {
            Sequence< Any> aCellContents = xFieldData->queryFieldData(nRowPos, ::getCppuType(&sCellText));
            sCellText = ::comphelper::getString(aCellContents.getConstArray()[nColumnPos]);
            ::svt::OStringTransfer::StartStringDrag(sCellText, this, DND_ACTION_COPY);
        }
    }
    catch(Exception&)
    {
        DBG_ERROR("SbaGridControl::DoFieldDrag : could not retrieve the cell's contents !");
        return;
    }

}
/// unary_function Functor object for class ZZ returntype is void
    struct SbaGridControlPrec : ::std::unary_function<DataFlavorExVector::value_type,bool>
    {
        sal_Bool    bQueryDrop;
        SbaGridControlPrec(sal_Bool _bQueryDrop)
            : bQueryDrop(_bQueryDrop)
        {
        }

        inline bool operator()(const DataFlavorExVector::value_type& _aType)
        {
            switch (_aType.mnSotId)
            {
//              case SOT_FORMAT_RTF:                    // RTF data descriptions
//              case SOT_FORMATSTR_ID_HTML:             // HTML data descriptions
//              case SOT_FORMATSTR_ID_HTML_SIMPLE:      // HTML data descriptions
                case SOT_FORMATSTR_ID_DBACCESS_TABLE:   // table descriptor
                case SOT_FORMATSTR_ID_DBACCESS_QUERY:   // query descriptor
                case SOT_FORMATSTR_ID_DBACCESS_COMMAND: // SQL command
                    return true;
                    break;
            }
            return false;
        }
    };
//------------------------------------------------------------------------------
sal_Int8 SbaGridControl::AcceptDrop( const BrowserAcceptDropEvent& rEvt )
{
    sal_Int8 nAction = DND_ACTION_NONE;

    // we need a valid connection
    if (!::dbtools::getConnection(Reference< XRowSet > (getDataSource(),UNO_QUERY)).is())
        return nAction;

    if ( IsDropFormatSupported( FORMAT_STRING ) ) do
    {   // odd construction, but spares us a lot of (explicit ;) goto's

        if (!GetEmptyRow().Is())
            // without an empty row we're not in update mode
            break;

        long    nRow = GetRowAtYPosPixel(rEvt.maPosPixel.Y(), sal_False);
        sal_uInt16  nCol = GetColumnAtXPosPixel(rEvt.maPosPixel.X(), sal_False);

        long nCorrectRowCount = GetRowCount();
        if (GetOptions() & OPT_INSERT)
            --nCorrectRowCount; // there is a empty row for inserting records
        if (IsCurrentAppending())
            --nCorrectRowCount; // the current data record doesn't really exist, we are appending a new one

        if ((nCol == BROWSER_INVALIDID) || (nRow >= nCorrectRowCount))
            // no valid cell under the mouse cursor
            break;
        Rectangle aRect = GetCellRect(nRow, nCol, sal_False);
        if (!aRect.IsInside(rEvt.maPosPixel))
            // not dropped within a cell (a cell isn't as wide as the column - the are small spaces)
            break;

        if ((IsModified() || (GetCurrentRow().Is() && GetCurrentRow()->IsModified())) && (GetCurrentPos() != nRow))
            // there is a current and modified row or cell and he text is to be dropped into another one
            break;

        CellControllerRef xCurrentController = Controller();
        if (xCurrentController.Is() && xCurrentController->IsModified() && ((nRow != GetCurRow()) || (nCol != GetCurColumnId())))
            // the current controller is modified and the user wants to drop in another cell -> no chance
            // (when leaving the modified cell a error may occur - this is deadly while dragging)
            break;

        Reference< XPropertySet >  xField = getField(GetModelColumnPos(nCol));
        if (!xField.is())
            // the column is not valid bound (for instance a binary field)
            break;

        try
        {
            if (::comphelper::getBOOL(xField->getPropertyValue(PROPERTY_ISREADONLY)))
                break;
        }
        catch (const Exception& e )
        {
            e; // make compiler happy
            // assume RO
            break;
        }

        try
        {
            // assume that text can be dropped into a field if the column has a ::com::sun::star::awt::XTextComponent interface
            Reference< XIndexAccess >  xColumnControls((::com::sun::star::form::XGridPeer*)GetPeer(), UNO_QUERY);
            if (xColumnControls.is())
            {
                Reference< ::com::sun::star::awt::XTextComponent >  xColControl;
                ::cppu::extractInterface(xColControl,xColumnControls->getByIndex(GetViewColumnPos(nCol)));
                if (xColControl.is())
                {
                    m_bActivatingForDrop = sal_True;
                    GoToRowColumnId(nRow, nCol);
                    m_bActivatingForDrop = sal_False;

                    nAction = DND_ACTION_COPY;
                }
            }
        }
        catch( const Exception& e )
        {
            e; // make compiler happy
            DBG_ERROR( "SbaGridControl::AcceptDrop: caught an exception!" );
        }

    } while (sal_False);

    if(nAction != DND_ACTION_COPY && GetEmptyRow().Is())
    {
        const DataFlavorExVector& _rFlavors = GetDataFlavors();
        if(::std::find_if(_rFlavors.begin(),_rFlavors.end(),SbaGridControlPrec(sal_True)) != _rFlavors.end())
            nAction = DND_ACTION_COPY;
    }

/*
    // check formats
    SvDataObjectRef xDataObj = SvDataObject::PasteDragServer( rEvt );
    if (!xDataObj.Is())
        return sal_False;

    const SvDataTypeList& rTypeList = xDataObj->GetTypeList();
    if ((rTypeList.Get(Exchange::RegisterFormatName(String::CreateFromAscii(SBA_DATAEXCHANGE_FORMAT)))) )
    {
        bAllow = (GetOptions() & OPT_INSERT) && rEvt.GetColumnId() > 0 && rEvt.GetRow() >= 0;
        ((BrowserDropEvent&)rEvt).SetAction(DROP_COPY);
    }

*/
    return (DND_ACTION_NONE != nAction) ? nAction : FmGridControl::AcceptDrop(rEvt);
}

//------------------------------------------------------------------------------
sal_Int8 SbaGridControl::ExecuteDrop( const BrowserExecuteDropEvent& rEvt )
{
    // we need some properties of our data source
    Reference< XPropertySet >  xDataSource = getDataSource();
    if (!xDataSource.is())
        return DND_ACTION_NONE;

    // we need a valid connection
    if (!::dbtools::getConnection(Reference< XRowSet > (xDataSource,UNO_QUERY)).is())
        return DND_ACTION_NONE;

    if ( IsDropFormatSupported( FORMAT_STRING ) )
    {
        long    nRow = GetRowAtYPosPixel(rEvt.maPosPixel.Y(), sal_False);
        sal_uInt16  nCol = GetColumnAtXPosPixel(rEvt.maPosPixel.X(), sal_False);

        long nCorrectRowCount = GetRowCount();
        if (GetOptions() & OPT_INSERT)
            --nCorrectRowCount; // there is a empty row for inserting records
        if (IsCurrentAppending())
            --nCorrectRowCount; // the current data record doesn't really exist, we are appending a new one

        DBG_ASSERT((nCol != BROWSER_INVALIDID) && (nRow < nCorrectRowCount), "SbaGridControl::Drop : dropped on an invalid position !");
            // AcceptDrop should have caught this

        // from now we work with ids instead of positions
        nCol = GetColumnId(nCol);

        GoToRowColumnId(nRow, nCol);
        if (!IsEditing())
            ActivateCell();

        CellControllerRef xCurrentController = Controller();
        if (!xCurrentController.Is() || !xCurrentController->ISA(EditCellController))
            return DND_ACTION_NONE;
        Edit& rEdit = (Edit&)xCurrentController->GetWindow();

        // get the dropped string
        TransferableDataHelper aDropped( rEvt.maDropEvent.Transferable );
        String sDropped;
        if ( !aDropped.GetString( FORMAT_STRING, sDropped ) )
            return DND_ACTION_NONE;

        rEdit.SetText( sDropped );
        xCurrentController->SetModified();
        rEdit.Modify();
            // SetText itself doesn't call a Modify as it isn't a user interaction

        return DND_ACTION_COPY;
    }

    if(GetEmptyRow().Is())
    {
        const DataFlavorExVector& _rFlavors = GetDataFlavors();
        DataFlavorExVector::const_iterator aFind = ::std::find_if(_rFlavors.begin(),_rFlavors.end(),SbaGridControlPrec(sal_True));
        if( aFind != _rFlavors.end())
        {
            TransferableDataHelper aDropped( rEvt.maDropEvent.Transferable );
            m_aDataDescriptor = ODataAccessObjectTransferable::extractObjectDescriptor(aDropped);
            if (m_nAsyncDropEvent)
                Application::RemoveUserEvent(m_nAsyncDropEvent);
            m_nAsyncDropEvent = Application::PostUserEvent(LINK(this, SbaGridControl, AsynchDropEvent));
            return DND_ACTION_COPY;
        }
    }

    return DND_ACTION_NONE;
}

//------------------------------------------------------------------------------
void SbaGridControl::refresh()
{
    // aktualisieren
    Reference< ::com::sun::star::form::XLoadable >  xLoadable(getDataSource(), UNO_QUERY);
    DBG_ASSERT(xLoadable.is(), "SbaGridControl::Drop : invalid data source !");
    if (xLoadable.is())
    {
        WaitObject aWO(this);
        xLoadable->reload();
    }
}

//------------------------------------------------------------------------------
Reference< XPropertySet >  SbaGridControl::getDataSource() const
{
    Reference< XPropertySet >  xReturn;

    Reference< XChild >  xColumns(GetPeer()->getColumns(), UNO_QUERY);
    Reference< XPropertySet >  xDataSource;
    if (xColumns.is())
        xReturn = Reference< XPropertySet > (xColumns->getParent(), UNO_QUERY);

    return xReturn;
}
// -----------------------------------------------------------------------------
IMPL_LINK(SbaGridControl, AsynchDropEvent, void*, EMPTY_ARG)
{
    m_nAsyncDropEvent = 0;

    Reference< XPropertySet >  xDataSource = getDataSource();
    if ( xDataSource.is() )
    {
        sal_Bool bCountFinal = sal_False;
        xDataSource->getPropertyValue(PROPERTY_ISROWCOUNTFINAL) >>= bCountFinal;
        if ( !bCountFinal )
            setDataSource(NULL); // deattach from grid control
        Reference< XResultSetUpdate > xResultSetUpdate(xDataSource,UNO_QUERY);
        ODatabaseImportExport* pImExport = new ORowSetImportExport(this,xResultSetUpdate,m_aDataDescriptor,getServiceManager());
        Reference<XEventListener> xHolder = pImExport;
        pImExport->initialize(m_aDataDescriptor);
        Hide();
        try
        {
            BeforeDrop();
            if(!pImExport->Read())
            {
                String sError = String(ModuleRes(STR_NO_COLUMNNAME_MATCHING));
                throwGenericSQLException(sError,NULL);
            }
            AfterDrop();
            Show();
        }
        catch(const SQLException& e)
        {
            AfterDrop();
            Show();
            ::dbaui::showError(::dbtools::SQLExceptionInfo(e),this,getServiceManager());
        }
        catch(const Exception& )
        {
            AfterDrop();
            Show();
            OSL_ENSURE(0,"Exception catched!");
        }
        if ( !bCountFinal )
            setDataSource(Reference< XRowSet >(xDataSource,UNO_QUERY));
    }
    m_aDataDescriptor.clear();

    return 0L;
}
// -------------------------------------------------------------------------
::rtl::OUString SbaGridControl::GetAccessibleDescription( ::svt::AccessibleBrowseBoxObjType eObjType,sal_Int32 _nPosition) const
{
    ::rtl::OUString sRet;
    if ( ::svt::BBTYPE_BROWSEBOX == eObjType )
    {
        ::vos::OGuard aGuard(Application::GetSolarMutex());
        sRet = String(ModuleRes(STR_DATASOURCE_GRIDCONTROL_DESC));
    }
    else
        sRet = FmGridControl::GetAccessibleDescription( eObjType,_nPosition);
    return sRet;
}
// -----------------------------------------------------------------------------



