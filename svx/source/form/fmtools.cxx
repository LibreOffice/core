/*************************************************************************
 *
 *  $RCSfile: fmtools.cxx,v $
 *
 *  $Revision: 1.26 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 15:02:36 $
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
#pragma hdrstop

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XPERSISTOBJECT_HPP_
#include <com/sun/star/io/XPersistObject.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_XEXECUTABLEDIALOG_HPP_
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XCOMPLETEDCONNECTION_HPP_
#include <com/sun/star/sdb/XCompletedConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_PRIVILEGE_HPP_
#include <com/sun/star/sdbcx/Privilege.hpp>
#endif
#ifndef _ISOLANG_HXX
#include <tools/isolang.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif
#ifndef _SVX_FMTOOLS_HXX
#include "fmtools.hxx"
#endif
#ifndef SVX_DBTOOLSCLIENT_HXX
#include "dbtoolsclient.hxx"
#endif
#ifndef _SVX_FMSERVS_HXX
#include "fmservs.hxx"
#endif
#ifndef _SVX_FMGLOB_HXX
#include "fmglob.hxx"
#endif
#ifndef _VCL_STDTEXT_HXX
#include <vcl/stdtext.hxx>
#endif
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/unohlp.hxx>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <wchar.h>

#ifndef _COM_SUN_STAR_UNO_XNAMINGSERVICE_HPP_
#include <com/sun/star/uno/XNamingService.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATASOURCE_HPP_
#include <com/sun/star/sdbc/XDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_COMMANDTYPE_HPP_
#include <com/sun/star/sdb/CommandType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XQUERIESSUPPLIER_HPP_
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_SQLCONTEXT_HPP_
#include <com/sun/star/sdb/SQLContext.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XRESULTSETACCESS_HPP_
#include <com/sun/star/sdb/XResultSetAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_NUMBERFORMAT_HPP_
#include <com/sun/star/util/NumberFormat.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASINK_HPP_
#include <com/sun/star/io/XActiveDataSink.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASOURCE_HPP_
#include <com/sun/star/io/XActiveDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XEVENTATTACHERMANAGER_HPP_
#include <com/sun/star/script/XEventAttacherManager.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORM_HPP_
#include <com/sun/star/form/XForm.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORMCOMPONENT_HPP_
#include <com/sun/star/form/XFormComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATTER_HPP_
#include <com/sun/star/util/XNumberFormatter.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATSSUPPLIER_HPP_
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_LANGUAGE_HPP_
#include <com/sun/star/util/Language.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATS_HPP_
#include <com/sun/star/util/XNumberFormats.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATTYPES_HPP_
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCLONEABLE_HPP_
#include <com/sun/star/util/XCloneable.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOBJECTINPUTSTREAM_HPP_
#include <com/sun/star/io/XObjectInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOBJECTOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XObjectOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_REFLECTION_XIDLCLASS_HPP_
#include <com/sun/star/reflection/XIdlClass.hpp>
#endif
#ifndef _COM_SUN_STAR_REFLECTION_XIDLMETHOD_HPP_
#include <com/sun/star/reflection/XIdlMethod.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XINTROSPECTION_HPP_
#include <com/sun/star/beans/XIntrospection.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif

#ifndef _TOOLS_DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#ifndef _SBXVAR_HXX //autogen
#include <svtools/sbxvar.hxx>
#endif

#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif

#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif

#ifndef _INTN_HXX //autogen
#include <tools/intn.hxx>
#endif

#ifndef _SVX_FMPROP_HRC
#include "fmprop.hrc"
#endif

#ifndef _SFX_BINDINGS_HXX //autogen wg. SfxBindings
#include <sfx2/bindings.hxx>
#endif

#ifndef _SFXENUMITEM_HXX //autogen wg. SfxBoolItem
#include <svtools/eitem.hxx>
#endif

#ifndef _SFXSTRITEM_HXX //autogen wg. SfxStringItem
#include <svtools/stritem.hxx>
#endif

#ifndef _CPPUHELPER_SERVICEFACTORY_HXX_
#include <cppuhelper/servicefactory.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _COMPHELPER_CONTAINER_HXX_
#include <comphelper/container.hxx>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _FM_STATIC_HXX_
#include "fmstatic.hxx"
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#include <algorithm>

namespace svxform
{

    IMPLEMENT_CONSTASCII_USTRING(DATA_MODE,"DataMode");
    IMPLEMENT_CONSTASCII_USTRING(FILTER_MODE,"FilterMode");

}   // namespace svxform

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::task;
using namespace ::svxform;
using namespace ::connectivity::simple;

//  ------------------------------------------------------------------------------
void displayException(const Any& _rExcept, Window* _pParent = NULL)
{
    try
    {
        // the parent window
        Window* pParentWindow = _pParent ? _pParent : GetpApp()->GetDefDialogParent();
        Reference< XWindow > xParentWindow = VCLUnoHelper::GetInterface(pParentWindow);

        Sequence< Any > aArgs(2);
        aArgs[0] <<= PropertyValue(::rtl::OUString::createFromAscii("SQLException"), 0, makeAny(_rExcept), PropertyState_DIRECT_VALUE);
        aArgs[1] <<= PropertyValue(::rtl::OUString::createFromAscii("ParentWindow"), 0, makeAny(xParentWindow), PropertyState_DIRECT_VALUE);

        static ::rtl::OUString s_sDialogServiceName = ::rtl::OUString::createFromAscii("com.sun.star.sdb.ErrorMessageDialog");
        Reference< XExecutableDialog > xErrorDialog(
            ::comphelper::getProcessServiceFactory()->createInstanceWithArguments(s_sDialogServiceName, aArgs), UNO_QUERY);
        if (xErrorDialog.is())
            xErrorDialog->execute();
        else
            ShowServiceNotAvailableError(pParentWindow, s_sDialogServiceName, sal_True);
    }
    catch(Exception&)
    {
        OSL_ENSURE(sal_False, "displayException: could not display the error message!");
    }
}

//  ------------------------------------------------------------------------------
void displayException(const ::com::sun::star::sdbc::SQLException& _rExcept, Window* _pParent)
{
    displayException(makeAny(_rExcept), _pParent);
}

//  ------------------------------------------------------------------------------
void displayException(const ::com::sun::star::sdbc::SQLWarning& _rExcept, Window* _pParent)
{
    displayException(makeAny(_rExcept), _pParent);
}

//  ------------------------------------------------------------------------------
void displayException(const ::com::sun::star::sdb::SQLContext& _rExcept, Window* _pParent)
{
    displayException(makeAny(_rExcept), _pParent);
}

//  ------------------------------------------------------------------------------
void displayException(const ::com::sun::star::sdb::SQLErrorEvent& _rEvent, Window* _pParent)
{
    displayException(_rEvent.Reason, _pParent);
}

//------------------------------------------------------------------------------
Reference< XInterface > cloneUsingProperties(const Reference< ::com::sun::star::io::XPersistObject>& _xObj)
{
    if (!_xObj.is())
        return Reference< XInterface >();

    // create a new object
    ::rtl::OUString aObjectService = _xObj->getServiceName();
    Reference< ::com::sun::star::beans::XPropertySet> xDestSet(::comphelper::getProcessServiceFactory()->createInstance(aObjectService), UNO_QUERY);
    if (!xDestSet.is())
    {
        DBG_ERROR("cloneUsingProperties : could not instantiate an object of the given type !");
        return Reference< XInterface >();
    }
    // transfer properties
    Reference< XPropertySet > xSourceSet(_xObj, UNO_QUERY);
    Reference< XPropertySetInfo > xSourceInfo( xSourceSet->getPropertySetInfo());
    Sequence< Property> aSourceProperties = xSourceInfo->getProperties();
    Reference< XPropertySetInfo > xDestInfo( xDestSet->getPropertySetInfo());
    Sequence< Property> aDestProperties = xDestInfo->getProperties();
    int nDestLen = aDestProperties.getLength();

    Property* pSourceProps = aSourceProperties.getArray();
    Property* pSourceEnd = pSourceProps + aSourceProperties.getLength();
    Property* pDestProps = aDestProperties.getArray();

    for (; pSourceProps != pSourceEnd; ++pSourceProps)
    {
        ::com::sun::star::beans::Property* pResult = ::std::lower_bound(
                pDestProps,
                pDestProps + nDestLen,
                pSourceProps->Name,
                ::comphelper::PropertyStringLessFunctor()
            );

        if  (   ( pResult != pDestProps + nDestLen )
            &&  ( pResult->Name == pSourceProps->Name )
            &&  ( pResult->Attributes == pSourceProps->Attributes )
            &&  ( (pResult->Attributes &  PropertyAttribute::READONLY ) == 0 )
            &&  ( pResult->Type.equals( pSourceProps->Type ) )
            )
        {   // Attribute/type are the same and the prop isn't readonly
            try
            {
                xDestSet->setPropertyValue(pResult->Name, xSourceSet->getPropertyValue(pResult->Name));
            }
            catch(IllegalArgumentException e)
            {
                e;
#ifdef DBG_UTIL
                ::rtl::OString sMessage("cloneUsingProperties : could not transfer the value for property \"");
                sMessage = sMessage + ::rtl::OString(pResult->Name.getStr(), pResult->Name.getLength(), RTL_TEXTENCODING_ASCII_US);
                sMessage = sMessage + '\"';
                DBG_ERROR(sMessage);
#endif
            }

        }
    }

    return xDestSet;
}

//------------------------------------------------------------------------------
sal_Bool searchElement(const Reference< ::com::sun::star::container::XIndexAccess>& xCont, const Reference< XInterface >& xElement)
{
    if (!xCont.is() || !xElement.is())
        return sal_False;

    sal_Int32 nCount = xCont->getCount();
    Reference< XInterface > xComp;
    for (sal_Int32 i = 0; i < nCount; i++)
    {
        try
        {
            xCont->getByIndex(i) >>= xComp;
            if (xComp.is())
            {
                if (((XInterface *)xElement.get()) == (XInterface*)xComp.get())
                    return sal_True;
                else
                {
                    Reference< ::com::sun::star::container::XIndexAccess> xCont2(xComp, UNO_QUERY);
                    if (xCont2.is() && searchElement(xCont2, xElement))
                        return sal_True;
                }
            }
        }
        catch(Exception&)
        {
        }
    }
    return sal_False;
}

//------------------------------------------------------------------------------
sal_Int32 getElementPos(const Reference< ::com::sun::star::container::XIndexAccess>& xCont, const Reference< XInterface >& xElement)
{
    sal_Int32 nIndex = -1;
    if (!xCont.is())
        return nIndex;


    Reference< XInterface > xNormalized( xElement, UNO_QUERY );
    DBG_ASSERT( xNormalized.is(), "getElementPos: invalid element!" );
    if ( xNormalized.is() )
    {
        // Feststellen an welcher Position sich das Kind befindet
        nIndex = xCont->getCount();
        while (nIndex--)
        {
            try
            {
                Reference< XInterface > xCurrent;
                xCont->getByIndex( nIndex ) >>= xCurrent;
                DBG_ASSERT( xCurrent.get() == Reference< XInterface >( xCurrent, UNO_QUERY ).get(),
                    "getElementPos: container element not normalized!" );
                if ( xNormalized.get() == xCurrent.get() )
                    break;
            }
            catch(Exception&)
            {
                DBG_ERROR( "getElementPos: caught an exception!" );
            }

        }
    }
    return nIndex;
}

//------------------------------------------------------------------
String getFormComponentAccessPath(const Reference< XInterface >& _xElement, Reference< XInterface >& _rTopLevelElement)
{
    Reference< ::com::sun::star::form::XFormComponent> xChild(_xElement, UNO_QUERY);
    Reference< ::com::sun::star::container::XIndexAccess> xParent;
    if (xChild.is())
        xParent = Reference< ::com::sun::star::container::XIndexAccess>(xChild->getParent(), UNO_QUERY);

    // while the current content is a form
    String sReturn;
    String sCurrentIndex;
    while (xChild.is())
    {
        // get the content's relative pos within it's parent container
        sal_Int32 nPos = getElementPos(xParent, xChild);

        // prepend this current relaive pos
        sCurrentIndex = String::CreateFromInt32(nPos);
        if (sReturn.Len() != 0)
        {
            sCurrentIndex += '\\';
            sCurrentIndex += sReturn;
        }

        sReturn = sCurrentIndex;

        // travel up
        if (::comphelper::query_interface((Reference< XInterface >)xParent,xChild))
            xParent = Reference< ::com::sun::star::container::XIndexAccess>(xChild->getParent(), UNO_QUERY);
    }

    _rTopLevelElement = xParent;
    return sReturn;
}

//------------------------------------------------------------------
String getFormComponentAccessPath(const Reference< XInterface >& _xElement)
{
    Reference< XInterface > xDummy;
    return getFormComponentAccessPath(_xElement, xDummy);
}

//------------------------------------------------------------------------------
Reference< XInterface > getElementFromAccessPath(const Reference< ::com::sun::star::container::XIndexAccess>& _xParent, const String& _rRelativePath)
{
    if (!_xParent.is())
        return Reference< XInterface >();
    Reference< ::com::sun::star::container::XIndexAccess> xContainer(_xParent);
    Reference< XInterface > xElement( _xParent);

    String sPath(_rRelativePath);
    while (sPath.Len() && xContainer.is())
    {
        xub_StrLen nSepPos = sPath.Search((sal_Unicode)'\\');

        String sIndex(sPath.Copy(0, (nSepPos == STRING_NOTFOUND) ? sPath.Len() : nSepPos));
        //  DBG_ASSERT(sIndex.IsNumeric(), "getElementFromAccessPath : invalid path !");

        sPath = sPath.Copy((nSepPos == STRING_NOTFOUND) ? sPath.Len() : nSepPos+1);

        ::cppu::extractInterface(xElement, xContainer->getByIndex(sIndex.ToInt32()));
        xContainer = Reference< ::com::sun::star::container::XIndexAccess>::query(xElement);
    }

    if (sPath.Len() != 0)
        // the loop terminated because an element wasn't a container, but we stil have a path -> the path is invalid
        xElement = NULL;

    return xElement;
}

//------------------------------------------------------------------
// Vergleichen von PropertyInfo
extern "C" int
#if defined( WNT )
 __cdecl
#endif
#if defined( ICC ) && defined( OS2 )
_Optlink
#endif
    NameCompare(const void* pFirst, const void* pSecond)
{
    return ((::rtl::OUString*)pFirst)->compareTo(*(::rtl::OUString*)pSecond);
}

//------------------------------------------------------------------------------
sal_Int32 findPos(const ::rtl::OUString& aStr, const Sequence< ::rtl::OUString>& rList)
{
    const ::rtl::OUString* pStrList = rList.getConstArray();
    ::rtl::OUString* pResult = (::rtl::OUString*) bsearch(&aStr, (void*)pStrList, rList.getLength(), sizeof(::rtl::OUString),
        &NameCompare);

    if (pResult)
        return (pResult - pStrList);
    else
        return -1;
}

//------------------------------------------------------------------
Reference< ::com::sun::star::frame::XModel> getXModel(const Reference< XInterface >& xIface)
{
    Reference< ::com::sun::star::frame::XModel> xModel(xIface, UNO_QUERY);
    if (xModel.is())
        return xModel;
    else
    {
        Reference< ::com::sun::star::container::XChild> xChild(xIface, UNO_QUERY);
        if (xChild.is())
        {
            Reference< XInterface > xParent( xChild->getParent());
            return getXModel(xParent);
        }
        else
            return NULL;
    }
}

//------------------------------------------------------------------
::rtl::OUString getLabelName(const Reference< ::com::sun::star::beans::XPropertySet>& xControlModel)
{
    if (!xControlModel.is())
        return ::rtl::OUString();

    if (::comphelper::hasProperty(FM_PROP_CONTROLLABEL, xControlModel))
    {
        Reference< ::com::sun::star::beans::XPropertySet> xLabelSet;
        xControlModel->getPropertyValue(FM_PROP_CONTROLLABEL) >>= xLabelSet;
        if (xLabelSet.is() && ::comphelper::hasProperty(FM_PROP_LABEL, xLabelSet))
        {
            Any aLabel( xLabelSet->getPropertyValue(FM_PROP_LABEL) );
            if ((aLabel.getValueTypeClass() == TypeClass_STRING) && ::comphelper::getString(aLabel).getLength())
                return ::comphelper::getString(aLabel);
        }
    }

    return ::comphelper::getString(xControlModel->getPropertyValue(FM_PROP_CONTROLSOURCE));
}

//========================================================================
// = CursorWrapper
//------------------------------------------------------------------------
CursorWrapper::CursorWrapper(const Reference< ::com::sun::star::sdbc::XRowSet>& _rxCursor, sal_Bool bUseCloned)
{
    ImplConstruct(Reference< ::com::sun::star::sdbc::XResultSet>(_rxCursor, UNO_QUERY), bUseCloned);
}

//------------------------------------------------------------------------
CursorWrapper::CursorWrapper(const Reference< ::com::sun::star::sdbc::XResultSet>& _rxCursor, sal_Bool bUseCloned)
{
    ImplConstruct(_rxCursor, bUseCloned);
}

//------------------------------------------------------------------------
void CursorWrapper::ImplConstruct(const Reference< ::com::sun::star::sdbc::XResultSet>& _rxCursor, sal_Bool bUseCloned)
{
    if (bUseCloned)
    {
        Reference< ::com::sun::star::sdb::XResultSetAccess> xAccess(_rxCursor, UNO_QUERY);
        try
        {
            m_xMoveOperations = xAccess.is() ? xAccess->createResultSet() : Reference< ::com::sun::star::sdbc::XResultSet>();
        }
        catch(Exception&)
        {
        }
    }
    else
        m_xMoveOperations   = _rxCursor;

    m_xBookmarkOperations   = m_xBookmarkOperations.query( m_xMoveOperations );
    m_xColumnsSupplier      = m_xColumnsSupplier.query( m_xMoveOperations );
    m_xPropertyAccess       = m_xPropertyAccess.query( m_xMoveOperations );

    if ( !m_xMoveOperations.is() || !m_xBookmarkOperations.is() || !m_xColumnsSupplier.is() || !m_xPropertyAccess.is() )
    {   // all or nothing !!
        m_xMoveOperations = NULL;
        m_xBookmarkOperations = NULL;
        m_xColumnsSupplier = NULL;
    }
    else
        m_xGeneric = m_xMoveOperations.get();
}

//------------------------------------------------------------------------
const CursorWrapper& CursorWrapper::operator=(const Reference< ::com::sun::star::sdbc::XRowSet>& _rxCursor)
{
    m_xMoveOperations = Reference< ::com::sun::star::sdbc::XResultSet>(_rxCursor, UNO_QUERY);
    m_xBookmarkOperations = Reference< ::com::sun::star::sdbcx::XRowLocate>(_rxCursor, UNO_QUERY);
    m_xColumnsSupplier = Reference< ::com::sun::star::sdbcx::XColumnsSupplier>(_rxCursor, UNO_QUERY);
    if (!m_xMoveOperations.is() || !m_xBookmarkOperations.is() || !m_xColumnsSupplier.is())
    {   // all or nothing !!
        m_xMoveOperations = NULL;
        m_xBookmarkOperations = NULL;
        m_xColumnsSupplier = NULL;
    }
    return *this;
}

//------------------------------------------------------------------------------
FmXDisposeListener::~FmXDisposeListener()
{
    setAdapter(NULL);
}

//------------------------------------------------------------------------------
void FmXDisposeListener::setAdapter(FmXDisposeMultiplexer* pAdapter)
{
    if (m_pAdapter)
    {
        ::osl::MutexGuard aGuard(m_rMutex);
        m_pAdapter->release();
        m_pAdapter = NULL;
    }

    if (pAdapter)
    {
        ::osl::MutexGuard aGuard(m_rMutex);
        m_pAdapter = pAdapter;
        m_pAdapter->acquire();
    }
}

//==============================================================================
DBG_NAME(FmXDisposeMultiplexer);
//------------------------------------------------------------------------------
FmXDisposeMultiplexer::FmXDisposeMultiplexer(FmXDisposeListener* _pListener, const Reference< ::com::sun::star::lang::XComponent>& _rxObject, sal_Int16 _nId)
    :m_pListener(_pListener)
    ,m_xObject(_rxObject)
    ,m_nId(_nId)
{
    DBG_CTOR(FmXDisposeMultiplexer, NULL);
    m_pListener->setAdapter(this);

    if (m_xObject.is())
        m_xObject->addEventListener(this);
}

//------------------------------------------------------------------------------
FmXDisposeMultiplexer::~FmXDisposeMultiplexer()
{
    DBG_DTOR(FmXDisposeMultiplexer, NULL);
}

// ::com::sun::star::lang::XEventListener
//------------------------------------------------------------------
void FmXDisposeMultiplexer::disposing(const ::com::sun::star::lang::EventObject& _Source) throw( RuntimeException )
{
    Reference< ::com::sun::star::lang::XEventListener> xPreventDelete(this);

    if (m_pListener)
    {
        m_pListener->disposing(_Source, m_nId);
        m_pListener->setAdapter(NULL);
        m_pListener = NULL;
    }
    m_xObject = NULL;
}

//------------------------------------------------------------------
void FmXDisposeMultiplexer::dispose()
{
    if (m_xObject.is())
    {
        Reference< ::com::sun::star::lang::XEventListener> xPreventDelete(this);

        m_xObject->removeEventListener(this);
        m_xObject = NULL;

        m_pListener->setAdapter(NULL);
        m_pListener = NULL;
    }
}

//==============================================================================
//------------------------------------------------------------------------------
sal_Int16 getControlTypeByObject(const Reference< ::com::sun::star::lang::XServiceInfo>& _rxObject)
{
    // ask for the persistent service name
    Reference< ::com::sun::star::io::XPersistObject> xPersistence(_rxObject, UNO_QUERY);
    DBG_ASSERT(xPersistence.is(), "::getControlTypeByObject : argument shold be an ::com::sun::star::io::XPersistObject !");
    if (!xPersistence.is())
        return OBJ_FM_CONTROL;

    ::rtl::OUString sPersistentServiceName = xPersistence->getServiceName();
    if (sPersistentServiceName.equals(FM_COMPONENT_EDIT))   // 5.0-Name
    {
        // may be a simple edit field or a formatted field, dependent of the supported services
        if (_rxObject->supportsService(FM_SUN_COMPONENT_FORMATTEDFIELD))
            return OBJ_FM_FORMATTEDFIELD;
        return OBJ_FM_EDIT;
    }
    if (sPersistentServiceName.equals(FM_COMPONENT_TEXTFIELD))
        return OBJ_FM_EDIT;
    if (sPersistentServiceName.equals(FM_COMPONENT_COMMANDBUTTON))
        return OBJ_FM_BUTTON;
    if (sPersistentServiceName.equals(FM_COMPONENT_FIXEDTEXT))
        return OBJ_FM_FIXEDTEXT;
    if (sPersistentServiceName.equals(FM_COMPONENT_LISTBOX))
        return OBJ_FM_LISTBOX;
    if (sPersistentServiceName.equals(FM_COMPONENT_CHECKBOX))
        return OBJ_FM_CHECKBOX;
    if (sPersistentServiceName.equals(FM_COMPONENT_RADIOBUTTON))
        return OBJ_FM_RADIOBUTTON;
    if (sPersistentServiceName.equals(FM_COMPONENT_GROUPBOX))
        return OBJ_FM_GROUPBOX;
    if (sPersistentServiceName.equals(FM_COMPONENT_COMBOBOX))
        return OBJ_FM_COMBOBOX;
    if (sPersistentServiceName.equals(FM_COMPONENT_GRID))   // 5.0-Name
        return OBJ_FM_GRID;
    if (sPersistentServiceName.equals(FM_COMPONENT_GRIDCONTROL))
        return OBJ_FM_GRID;
    if (sPersistentServiceName.equals(FM_COMPONENT_IMAGEBUTTON))
        return OBJ_FM_IMAGEBUTTON;
    if (sPersistentServiceName.equals(FM_COMPONENT_FILECONTROL))
        return OBJ_FM_FILECONTROL;
    if (sPersistentServiceName.equals(FM_COMPONENT_DATEFIELD))
        return OBJ_FM_DATEFIELD;
    if (sPersistentServiceName.equals(FM_COMPONENT_TIMEFIELD))
        return OBJ_FM_TIMEFIELD;
    if (sPersistentServiceName.equals(FM_COMPONENT_NUMERICFIELD))
        return OBJ_FM_NUMERICFIELD;
    if (sPersistentServiceName.equals(FM_COMPONENT_CURRENCYFIELD))
        return OBJ_FM_CURRENCYFIELD;
    if (sPersistentServiceName.equals(FM_COMPONENT_PATTERNFIELD))
        return OBJ_FM_PATTERNFIELD;
    if (sPersistentServiceName.equals(FM_COMPONENT_HIDDEN)) // 5.0-Name
        return OBJ_FM_HIDDEN;
    if (sPersistentServiceName.equals(FM_COMPONENT_HIDDENCONTROL))
        return OBJ_FM_HIDDEN;
    if (sPersistentServiceName.equals(FM_COMPONENT_IMAGECONTROL))
        return OBJ_FM_IMAGECONTROL;
    if (sPersistentServiceName.equals(FM_COMPONENT_FORMATTEDFIELD))
    {
        DBG_ERROR("::getControlTypeByObject : suspicious persistent service name (formatted field) !");
            // objects with that service name should exist as they aren't compatible with older versions
        return OBJ_FM_FORMATTEDFIELD;
    }

    DBG_ERROR("::getControlTypeByObject : unknown object type !");
    return OBJ_FM_CONTROL;
}

//------------------------------------------------------------------------------
::rtl::OUString getServiceNameByControlType(sal_Int16 nType)
{
    switch (nType)
    {
        case OBJ_FM_EDIT            : return FM_COMPONENT_TEXTFIELD;
        case OBJ_FM_BUTTON          : return FM_COMPONENT_COMMANDBUTTON;
        case OBJ_FM_FIXEDTEXT       : return FM_COMPONENT_FIXEDTEXT;
        case OBJ_FM_LISTBOX         : return FM_COMPONENT_LISTBOX;
        case OBJ_FM_CHECKBOX        : return FM_COMPONENT_CHECKBOX;
        case OBJ_FM_RADIOBUTTON     : return FM_COMPONENT_RADIOBUTTON;
        case OBJ_FM_GROUPBOX        : return FM_COMPONENT_GROUPBOX;
        case OBJ_FM_COMBOBOX        : return FM_COMPONENT_COMBOBOX;
        case OBJ_FM_GRID            : return FM_COMPONENT_GRIDCONTROL;
        case OBJ_FM_IMAGEBUTTON     : return FM_COMPONENT_IMAGEBUTTON;
        case OBJ_FM_FILECONTROL     : return FM_COMPONENT_FILECONTROL;
        case OBJ_FM_DATEFIELD       : return FM_COMPONENT_DATEFIELD;
        case OBJ_FM_TIMEFIELD       : return FM_COMPONENT_TIMEFIELD;
        case OBJ_FM_NUMERICFIELD    : return FM_COMPONENT_NUMERICFIELD;
        case OBJ_FM_CURRENCYFIELD   : return FM_COMPONENT_CURRENCYFIELD;
        case OBJ_FM_PATTERNFIELD    : return FM_COMPONENT_PATTERNFIELD;
        case OBJ_FM_HIDDEN          : return FM_COMPONENT_HIDDENCONTROL;
        case OBJ_FM_IMAGECONTROL    : return FM_COMPONENT_IMAGECONTROL;
        case OBJ_FM_FORMATTEDFIELD  : return FM_COMPONENT_FORMATTEDFIELD;
    }
    return ::rtl::OUString();
}
//------------------------------------------------------------------------------
Sequence< ::rtl::OUString> getEventMethods(const Type& type)
{
    typelib_InterfaceTypeDescription *pType=0;
    type.getDescription( (typelib_TypeDescription**)&pType);

    if(!pType)
        return Sequence< ::rtl::OUString>();

    Sequence< ::rtl::OUString> aNames(pType->nMembers);
    ::rtl::OUString* pNames = aNames.getArray();
    for(sal_Int32 i=0;i<pType->nMembers;i++,++pNames)
    {
        // the decription reference
        typelib_TypeDescriptionReference* pMemberDescriptionReference = pType->ppMembers[i];
        // the description for the reference
        typelib_TypeDescription* pMemberDescription = NULL;
        typelib_typedescriptionreference_getDescription(&pMemberDescription, pMemberDescriptionReference);
        if (pMemberDescription)
        {
            typelib_InterfaceMemberTypeDescription* pRealMemberDescription =
                reinterpret_cast<typelib_InterfaceMemberTypeDescription*>(pMemberDescription);
            *pNames = pRealMemberDescription->pMemberName;
        }
    }

    typelib_typedescription_release( (typelib_TypeDescription *)pType );
    return aNames;
}


//------------------------------------------------------------------------------
void TransferEventScripts(const Reference< ::com::sun::star::awt::XControlModel>& xModel, const Reference< ::com::sun::star::awt::XControl>& xControl,
    const Sequence< ::com::sun::star::script::ScriptEventDescriptor>& rTransferIfAvailable)
{
    // first check if we have a XEventAttacherManager for the model
    Reference< ::com::sun::star::container::XChild> xModelChild(xModel, UNO_QUERY);
    if (!xModelChild.is())
        return; // nothing to do

    Reference< ::com::sun::star::script::XEventAttacherManager> xEventManager(xModelChild->getParent(), UNO_QUERY);
    if (!xEventManager.is())
        return; // nothing to do

    if (!rTransferIfAvailable.getLength())
        return; // nothing to do

    // check for the index of the model within it's parent
    Reference< ::com::sun::star::container::XIndexAccess> xParentIndex(xModelChild->getParent(), UNO_QUERY);
    if (!xParentIndex.is())
        return; // nothing to do
    sal_Int32 nIndex = getElementPos(xParentIndex, xModel);
    if (nIndex<0 || nIndex>=xParentIndex->getCount())
        return; // nothing to do

    // then we need informations about the listeners supported by the control and the model
    Sequence< Type> aModelListeners;
    Sequence< Type> aControlListeners;

    Reference< ::com::sun::star::beans::XIntrospection> xModelIntrospection(::comphelper::getProcessServiceFactory()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.beans.Introspection")), UNO_QUERY);
    Reference< ::com::sun::star::beans::XIntrospection> xControlIntrospection(::comphelper::getProcessServiceFactory()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.beans.Introspection")), UNO_QUERY);

    if (xModelIntrospection.is() && xModel.is())
    {
        Any aModel(makeAny(xModel));
        aModelListeners = xModelIntrospection->inspect(aModel)->getSupportedListeners();
    }

    if (xControlIntrospection.is() && xControl.is())
    {
        Any aControl(makeAny(xControl));
        aControlListeners = xControlIntrospection->inspect(aControl)->getSupportedListeners();
    }

    sal_Int32 nMaxNewLen = aModelListeners.getLength() + aControlListeners.getLength();
    if (!nMaxNewLen)
        return; // the model and the listener don't support any listeners (or we were unable to retrieve these infos)

    Sequence< ::com::sun::star::script::ScriptEventDescriptor>  aTransferable(nMaxNewLen);
    ::com::sun::star::script::ScriptEventDescriptor* pTransferable = aTransferable.getArray();

    const ::com::sun::star::script::ScriptEventDescriptor* pCurrent = rTransferIfAvailable.getConstArray();
    sal_Int32 i,j,k;
    for (i=0; i<rTransferIfAvailable.getLength(); ++i, ++pCurrent)
    {
        // search the model/control idl classes for the event described by pCurrent
        for (   Sequence< Type>* pCurrentArray = &aModelListeners;
                pCurrentArray;
                pCurrentArray = (pCurrentArray == &aModelListeners) ? &aControlListeners : NULL
            )
        {
            const Type* pCurrentListeners = pCurrentArray->getConstArray();
            for (j=0; j<pCurrentArray->getLength(); ++j, ++pCurrentListeners)
            {
                UniString aListener = (*pCurrentListeners).getTypeName();
                sal_Int32 nTokens = aListener.GetTokenCount('.');
                if (nTokens)
                    aListener = aListener.GetToken(nTokens - 1, '.');

                if (aListener == pCurrent->ListenerType.getStr())
                    // the current ::com::sun::star::script::ScriptEventDescriptor doesn't match the current listeners class
                    continue;

                // now check the methods
                Sequence< ::rtl::OUString> aMethodsNames = getEventMethods(*pCurrentListeners);
                const ::rtl::OUString* pMethodsNames = aMethodsNames.getConstArray();
                for (k=0; k<aMethodsNames.getLength(); ++k, ++pMethodsNames)
                {
                    if ((*pMethodsNames).compareTo(pCurrent->EventMethod) != COMPARE_EQUAL)
                        // the current ::com::sun::star::script::ScriptEventDescriptor doesn't match the current listeners current method
                        continue;

                    // we can transfer the script event : the model (control) supports it
                    *pTransferable = *pCurrent;
                    ++pTransferable;
                    break;
                }
                if (k<aMethodsNames.getLength())
                    break;
            }
        }
    }

    sal_Int32 nRealNewLen = pTransferable - aTransferable.getArray();
    aTransferable.realloc(nRealNewLen);

    xEventManager->registerScriptEvents(nIndex, aTransferable);
}

//------------------------------------------------------------------------------
sal_Int16   GridView2ModelPos(const Reference< ::com::sun::star::container::XIndexAccess>& rColumns, sal_Int16 nViewPos)
{
    try
    {
        if (rColumns.is())
        {
            // loop through all columns
            sal_Int16 i;
            Reference< ::com::sun::star::beans::XPropertySet> xCur;
            for (i=0; i<rColumns->getCount(); ++i)
            {
                rColumns->getByIndex(i) >>= xCur;
                if (!::comphelper::getBOOL(xCur->getPropertyValue(FM_PROP_HIDDEN)))
                    // for every visible col : if nViewPos is greater zero, decrement it, else we
                    // have found the model position
                    if (!nViewPos)
                        break;
                    else
                        --nViewPos;
            }
            if (i<rColumns->getCount())
                return i;
        }
    }
    catch(const Exception&)
    {
        DBG_ERROR("GridView2ModelPos Exception occured!");
    }
    return (sal_Int16)-1;
}

//==============================================================================
// FmSlotDispatch - some kind of translator between the Sfx-Slots and the UNO-dispatchers
//==============================================================================

//  SMART_UNO_IMPLEMENTATION(FmSlotDispatch, UsrObject);


DBG_NAME(FmSlotDispatch);
//------------------------------------------------------------------------------
FmSlotDispatch::FmSlotDispatch(const  URL& rUrl, sal_Int16 nSlotId, SfxBindings& rBindings)
    :SfxControllerItem(nSlotId, rBindings)
    ,m_aDisposeListeners(m_aAccessSafety)
    ,m_aStatusListeners(m_aAccessSafety)
    ,m_aUrl(rUrl)
    ,m_nSlot(nSlotId)
{
    DBG_CTOR(FmSlotDispatch,NULL);

}

//------------------------------------------------------------------------------
FmSlotDispatch::~FmSlotDispatch()
{

    DBG_DTOR(FmSlotDispatch,NULL);
}

//------------------------------------------------------------------------------
void FmSlotDispatch::BroadcastCurrentState( )
{
    SfxPoolItem* pCurrentState = NULL;
    SfxItemState eCurrentState = GetBindings().QueryState( m_nSlot, pCurrentState );
    NotifyState( eCurrentState, pCurrentState );
    delete pCurrentState;
}

//------------------------------------------------------------------------------
void FmSlotDispatch::dispatch(const  URL& aURL, const Sequence< ::com::sun::star::beans::PropertyValue>& aArgs) throw( RuntimeException )
{
    DBG_ASSERT(aURL.Main.compareTo(m_aUrl.Main) == COMPARE_EQUAL, "FmSlotDispatch::dispatch : invalid argument !");
    DBG_ASSERT(m_aExecutor.IsSet(), "FmSlotDispatch::dispatch : no executor !");
    // if we have no executor we would have disabled this feature in statusChanged-calls

    m_aExecutor.Call(this);
}

//------------------------------------------------------------------------------
void FmSlotDispatch::NotifyState(SfxItemState eState, const SfxPoolItem* pState, const Reference< ::com::sun::star::frame::XStatusListener>& rListener)
{
    ::com::sun::star::frame::FeatureStateEvent aEvent = BuildEvent(eState, pState);

    if (rListener.is())
        rListener->statusChanged(aEvent);
    else
        NOTIFY_LISTENERS(m_aStatusListeners, ::com::sun::star::frame::XStatusListener, statusChanged, aEvent);
}

//------------------------------------------------------------------------------
void SAL_CALL FmSlotDispatch::addStatusListener( const Reference< ::com::sun::star::frame::XStatusListener >& xControl, const URL& aURL ) throw(RuntimeException)
{
    DBG_ASSERT((aURL.Main.getLength() == 0) || (aURL.Main.compareTo(m_aUrl.Main) == COMPARE_EQUAL),
        "FmSlotDispatch::addStatusListener: invalid argument !");
    m_aStatusListeners.addInterface( xControl );

    // acknowledge the initial status
    SfxPoolItem* pState = NULL;
    SfxItemState eInitialState = GetBindings().QueryState(m_nSlot, pState);

    NotifyState(eInitialState, pState, xControl);
}

//------------------------------------------------------------------------------
void SAL_CALL FmSlotDispatch::removeStatusListener( const Reference< ::com::sun::star::frame::XStatusListener >& xControl, const URL& aURL ) throw(RuntimeException)
{
    DBG_ASSERT((aURL.Main.getLength() == 0) || (aURL.Main.compareTo(m_aUrl.Main) == COMPARE_EQUAL),
        "FmSlotDispatch::removeStatusListener: invalid argument !");
    m_aStatusListeners.removeInterface( xControl );
}

//------------------------------------------------------------------------------
void SAL_CALL FmSlotDispatch::dispose(  ) throw(RuntimeException)
{
    Reference< XInterface > xXInterface((*this));
    ::com::sun::star::lang::EventObject aEvt(xXInterface);
    m_aDisposeListeners.disposeAndClear(aEvt);
    m_aStatusListeners.disposeAndClear(aEvt);
}

//------------------------------------------------------------------------------
void SAL_CALL FmSlotDispatch::addEventListener( const Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw(RuntimeException)
{
    m_aDisposeListeners.addInterface( xListener );
}

//------------------------------------------------------------------------------
void SAL_CALL FmSlotDispatch::removeEventListener( const Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw(RuntimeException)
{
    m_aDisposeListeners.removeInterface( aListener );
}

//------------------------------------------------------------------------------
void FmSlotDispatch::StateChanged(USHORT _nSID, SfxItemState _eState, const SfxPoolItem* _pState)
{
    DBG_ASSERT(_nSID == m_nSlot, "FmSlotDispatch::StateChanged : where did this come from ?");

    ::com::sun::star::frame::FeatureStateEvent eEvent = BuildEvent(_eState, _pState);
    NOTIFY_LISTENERS(m_aStatusListeners, ::com::sun::star::frame::XStatusListener, statusChanged, eEvent);
}

//------------------------------------------------------------------------------
::com::sun::star::frame::FeatureStateEvent FmSlotDispatch::BuildEvent(SfxItemState eState, const SfxPoolItem* pState)
{
    ::com::sun::star::frame::FeatureStateEvent aReturn;
    aReturn.Source = static_cast< ::cppu::OWeakObject* >( this );
    aReturn.FeatureURL = m_aUrl;
    aReturn.IsEnabled = (SFX_ITEM_DISABLED != eState) && m_aExecutor.IsSet();
    aReturn.Requery = sal_False;

    if (pState)
    {
        if (pState->ISA(SfxBoolItem))
            aReturn.State <<= ((SfxBoolItem*)pState)->GetValue();
        else if (pState->ISA(SfxStringItem))
            aReturn.State <<= ::rtl::OUString(((SfxStringItem*)pState)->GetValue());
#if DBG_UTIL
        else if (!pState->ISA(SfxVoidItem))
            DBG_ERROR("FmSlotDispatch::BuildEvent : don't know what to do with the ItemState !");
#endif
    }

    return aReturn;
}

//========================================================================
//= FmXDispatchInterceptorImpl
//========================================================================

DBG_NAME(FmXDispatchInterceptorImpl);
//------------------------------------------------------------------------
FmXDispatchInterceptorImpl::FmXDispatchInterceptorImpl(
            const Reference< XDispatchProviderInterception >& _rxToIntercept, FmDispatchInterceptor* _pMaster,
            sal_Int16 _nId, Sequence< ::rtl::OUString > _rInterceptedSchemes)
    :FmXDispatchInterceptorImpl_BASE(_pMaster && _pMaster->getInterceptorMutex() ? *_pMaster->getInterceptorMutex() : m_aFallback)
    ,m_xIntercepted(_rxToIntercept)
    ,m_pMaster(_pMaster)
    ,m_nId(_nId)
    ,m_aInterceptedURLSchemes(_rInterceptedSchemes)
    ,m_bListening(sal_False)
{
    DBG_CTOR(FmXDispatchInterceptorImpl,NULL);

    ::osl::MutexGuard aGuard(getAccessSafety());
    ::comphelper::increment(m_refCount);
    if (_rxToIntercept.is())
    {
        _rxToIntercept->registerDispatchProviderInterceptor((::com::sun::star::frame::XDispatchProviderInterceptor*)this);
        // this should make us the top-level dispatch-provider for the component, via a call to our
        // setDispatchProvider we should have got an fallback for requests we (i.e. our master) cannot fullfill
        Reference< ::com::sun::star::lang::XComponent> xInterceptedComponent(_rxToIntercept, UNO_QUERY);
        if (xInterceptedComponent.is())
        {
            xInterceptedComponent->addEventListener(this);
            m_bListening = sal_True;
        }
    }
    ::comphelper::decrement(m_refCount);
}

//------------------------------------------------------------------------
FmXDispatchInterceptorImpl::~FmXDispatchInterceptorImpl()
{
    if (!rBHelper.bDisposed)
        dispose();

    DBG_DTOR(FmXDispatchInterceptorImpl,NULL);
}

//------------------------------------------------------------------------------
Sequence< sal_Int8 > SAL_CALL FmXDispatchInterceptorImpl::getImplementationId() throw(RuntimeException)
{
    return ::form::OImplementationIds::getImplementationId(getTypes());
}
//------------------------------------------------------------------------------
Reference< ::com::sun::star::frame::XDispatch > SAL_CALL FmXDispatchInterceptorImpl::queryDispatch( const URL& aURL, const ::rtl::OUString& aTargetFrameName, sal_Int32 nSearchFlags ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard(getAccessSafety());
    Reference< ::com::sun::star::frame::XDispatch> xResult;
    // ask our 'real' interceptor
    if (m_pMaster)
        xResult = m_pMaster->interceptedQueryDispatch(m_nId, aURL, aTargetFrameName, nSearchFlags);

    // ask our slave provider
    if (!xResult.is() && m_xSlaveDispatcher.is())
        xResult = m_xSlaveDispatcher->queryDispatch(aURL, aTargetFrameName, nSearchFlags);

    return xResult;
}

//------------------------------------------------------------------------------
Sequence< Reference< ::com::sun::star::frame::XDispatch > > SAL_CALL
FmXDispatchInterceptorImpl::queryDispatches( const Sequence< ::com::sun::star::frame::DispatchDescriptor >& aDescripts ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard(getAccessSafety());
    Sequence< Reference< ::com::sun::star::frame::XDispatch> > aReturn(aDescripts.getLength());
    Reference< ::com::sun::star::frame::XDispatch>* pReturn = aReturn.getArray();
    const ::com::sun::star::frame::DispatchDescriptor* pDescripts = aDescripts.getConstArray();
    for (sal_Int16 i=0; i<aDescripts.getLength(); ++i, ++pReturn, ++pDescripts)
    {
        *pReturn = queryDispatch(pDescripts->FeatureURL, pDescripts->FrameName, pDescripts->SearchFlags);
    }
    return aReturn;
}

//------------------------------------------------------------------------------
Reference< ::com::sun::star::frame::XDispatchProvider > SAL_CALL FmXDispatchInterceptorImpl::getSlaveDispatchProvider(  ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard(getAccessSafety());
    return m_xSlaveDispatcher;
}

//------------------------------------------------------------------------------
void SAL_CALL FmXDispatchInterceptorImpl::setSlaveDispatchProvider(const Reference< ::com::sun::star::frame::XDispatchProvider>& xNewDispatchProvider) throw( RuntimeException )
{
    ::osl::MutexGuard aGuard(getAccessSafety());
    m_xSlaveDispatcher = xNewDispatchProvider;
}

//------------------------------------------------------------------------------
Reference< ::com::sun::star::frame::XDispatchProvider> SAL_CALL FmXDispatchInterceptorImpl::getMasterDispatchProvider(void) throw( RuntimeException )
{
    ::osl::MutexGuard aGuard(getAccessSafety());
    return m_xMasterDispatcher;
}

//------------------------------------------------------------------------------
void SAL_CALL FmXDispatchInterceptorImpl::setMasterDispatchProvider(const Reference< ::com::sun::star::frame::XDispatchProvider>& xNewSupplier) throw( RuntimeException )
{
    ::osl::MutexGuard aGuard(getAccessSafety());
    m_xMasterDispatcher = xNewSupplier;
}

//------------------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL FmXDispatchInterceptorImpl::getInterceptedURLs(  ) throw(RuntimeException)
{
    return m_aInterceptedURLSchemes;
}

//------------------------------------------------------------------------------
void SAL_CALL FmXDispatchInterceptorImpl::disposing(const ::com::sun::star::lang::EventObject& Source) throw( ::com::sun::star::uno::RuntimeException )
{
    if (m_bListening)
    {
        Reference< XDispatchProviderInterception > xIntercepted(m_xIntercepted.get(), UNO_QUERY);
        if (Source.Source == xIntercepted)
            ImplDetach();
    }
}

//------------------------------------------------------------------------------
void FmXDispatchInterceptorImpl::ImplDetach()
{
    ::osl::MutexGuard aGuard(getAccessSafety());
    OSL_ENSURE(m_bListening, "FmXDispatchInterceptorImpl::ImplDetach: invalid call!");

    // deregister ourself from the interception component
    Reference< XDispatchProviderInterception > xIntercepted(m_xIntercepted.get(), UNO_QUERY);
    if (xIntercepted.is())
        xIntercepted->releaseDispatchProviderInterceptor(static_cast<XDispatchProviderInterceptor*>(this));

//  m_xIntercepted = Reference< XDispatchProviderInterception >();
        // Don't reset m_xIntercepted: It may be needed by our owner to check for which object we were
        // responsible. As we hold the object with a weak reference only, this should be no problem.
        // 88936 - 23.07.2001 - frank.schoenheit@sun.com
    m_pMaster = NULL;
    m_bListening = sal_False;
}

//------------------------------------------------------------------------------
void FmXDispatchInterceptorImpl::disposing()
{
    // remove ourself as event listener from the interception component
    if (m_bListening)
    {
        Reference< ::com::sun::star::lang::XComponent> xInterceptedComponent(m_xIntercepted.get(), UNO_QUERY);
        if (xInterceptedComponent.is())
            xInterceptedComponent->removeEventListener(static_cast<XEventListener*>(this));

        // detach from the interception component
        ImplDetach();
    }
}

//==============================================================================
//==============================================================================

//------------------------------------------------------------------------------
sal_Bool isLoadable(const Reference< XInterface >& xLoad)
{
    // determines whether a form should be loaded or not
    // if there is no datasource or connection there is no reason to load a form
    Reference< ::com::sun::star::beans::XPropertySet> xSet(xLoad, UNO_QUERY);
    if (xSet.is())
    {
        try
        {
            // is there already a active connection
            Reference< XInterface > xConn;
            xSet->getPropertyValue(FM_PROP_ACTIVE_CONNECTION) >>= xConn;
            return (xConn.is() ||
                    ::comphelper::getString(xSet->getPropertyValue(FM_PROP_DATASOURCE)).getLength() ||
                    ::comphelper::getString(xSet->getPropertyValue(FM_PROP_URL)).getLength());
        }
        catch(Exception&)
        {
            DBG_ERROR("isLoadable Exception occured!");
        }

    }
    return sal_False;
}

//------------------------------------------------------------------------------
void setConnection(const Reference< ::com::sun::star::sdbc::XRowSet>& _rxRowSet, const Reference< ::com::sun::star::sdbc::XConnection>& _rxConn)
{
    Reference< ::com::sun::star::beans::XPropertySet> xRowSetProps(_rxRowSet, UNO_QUERY);
    if (xRowSetProps.is())
    {
        try
        {
            Any aConn(makeAny(_rxConn));
            xRowSetProps->setPropertyValue(FM_PROP_ACTIVE_CONNECTION, aConn);
        }
        catch(Exception&)
        {
            DBG_ERROR("::setConnection : could not set the connection !");
        }

    }
}
//------------------------------------------------------------------------------
sal_Bool isRowSetAlive(const Reference< XInterface >& _rxRowSet)
{
    sal_Bool bIsAlive = sal_False;
    Reference< ::com::sun::star::sdbcx::XColumnsSupplier> xSupplyCols(_rxRowSet, UNO_QUERY);
    Reference< ::com::sun::star::container::XIndexAccess> xCols;
    if (xSupplyCols.is())
        xCols = Reference< ::com::sun::star::container::XIndexAccess>(xSupplyCols->getColumns(), UNO_QUERY);
    if (xCols.is() && (xCols->getCount() > 0))
        bIsAlive = sal_True;

    return bIsAlive;
}

