/*************************************************************************
 *
 *  $RCSfile: fmtools.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: fs $ $Date: 2000-11-09 10:03:55 $
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
#ifndef _ISOLANG_HXX
#include <tools/isolang.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif
#ifndef _SVX_FMTOOLS_HXX
#include "fmtools.hxx"
#endif
#ifndef _SVX_FMSERVS_HXX
#include "fmservs.hxx"
#endif
#ifndef _SVX_FMGLOB_HXX
#include "fmglob.hxx"
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
#ifndef _COM_SUN_STAR_SDB_XDATABASEENVIRONMENT_HPP_
#include <com/sun/star/sdb/XDatabaseEnvironment.hpp>
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
#include <com/sun/star/beans/PropertyAttribute.hpp>
#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
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

#ifndef _TOOLS_SOLMATH_HXX //autogen wg. SolarMath
#include <tools/solmath.hxx>
#endif

#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif

#ifndef _INTN_HXX //autogen
#include <tools/intn.hxx>
#endif

#ifndef _SVX_DBERRBOX_HXX
#include "dbmsgbox.hxx"
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
#ifndef _CPPUHELPER_EXTRACT_HXX_
#include <cppuhelper/extract.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif
#ifndef _CPPUHELPER_EXTRACT_HXX_
#include <cppuhelper/extract.hxx>
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;

IMPLEMENT_CONSTASCII_USTRING(DATA_MODE,"DataMode");
IMPLEMENT_CONSTASCII_USTRING(FILTER_MODE,"FilterMode");

//==============================================================================
//------------------------------------------------------------------------------
void displayException(const ::dbtools::SQLExceptionInfo& _rError)
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    if (_rError.isKindOf(::dbtools::SQLExceptionInfo::SQL_EXCEPTION))
    {
        SvxDBMsgBox aSvxDBMsgBox(GetpApp()->GetAppWindow(), *(const ::com::sun::star::sdbc::SQLException*)_rError, WB_OK);
        aSvxDBMsgBox.Execute();
    }
    else
    {
        DBG_ERROR("::displayException : invalid event (does not contain an ::com::sun::star::sdbc::SQLException) !");
        ::com::sun::star::sdbc::SQLException aDummy;
        SvxDBMsgBox aSvxDBMsgBox(GetpApp()->GetAppWindow(), aDummy, WB_OK);
        aSvxDBMsgBox.Execute();
            // the SvxDBMsgBox will create a default context info ("read error") for this
    }
}

//  ------------------------------------------------------------------------------
void displayException(const ::com::sun::star::sdbc::SQLException& _rExcept, WinBits nStyle)
{
    ::dbtools::SQLExceptionInfo aInfo(_rExcept);
    displayException(aInfo);
}

//  ------------------------------------------------------------------------------
void displayException(const ::com::sun::star::sdb::SQLErrorEvent& _rEvent, WinBits nStyle)
{
    ::dbtools::SQLExceptionInfo aInfo(_rEvent);
    displayException(aInfo);
}

//------------------------------------------------------------------------------
// Vergleichen von Properties
extern "C" int
#if defined( WNT )
 __cdecl
#endif
#if defined( ICC ) && defined( OS2 )
_Optlink
#endif
    PropertyCompare( const void* pFirst, const void* pSecond)
{
    return ((::com::sun::star::beans::Property*)pFirst)->Name.compareTo(((::com::sun::star::beans::Property*)pSecond)->Name);
}






//------------------------------------------------------------------------------
Reference< XInterface> clone(const Reference< ::com::sun::star::io::XPersistObject>& _xObj)
{
    Reference< XInterface> xClone;
    if (!_xObj.is())
        return Reference< XInterface>();

    // ::std::copy it by streaming

    // creating a pipe
    Reference< ::com::sun::star::io::XOutputStream> xOutPipe(::comphelper::getProcessServiceFactory()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.io.Pipe")), UNO_QUERY);
    Reference< ::com::sun::star::io::XInputStream> xInPipe(xOutPipe, UNO_QUERY);

    // creating the mark streams
    Reference< ::com::sun::star::io::XInputStream> xMarkIn(::comphelper::getProcessServiceFactory()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.io.MarkableInputStream")), UNO_QUERY);
    Reference< ::com::sun::star::io::XActiveDataSink> xMarkSink(xMarkIn, UNO_QUERY);
    xMarkSink->setInputStream(xInPipe);

    Reference< ::com::sun::star::io::XOutputStream> xMarkOut(::comphelper::getProcessServiceFactory()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.io.MarkableOutputStream")), UNO_QUERY);
    Reference< ::com::sun::star::io::XActiveDataSource> xMarkSource(xMarkOut, UNO_QUERY);
    xMarkSource->setOutputStream(xOutPipe);

    // connect mark and sink
    Reference< ::com::sun::star::io::XActiveDataSink> xSink(::comphelper::getProcessServiceFactory()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.io.ObjectInputStream")), UNO_QUERY);
    xSink->setInputStream(xMarkIn);

    // connect mark and source
    Reference< ::com::sun::star::io::XActiveDataSource> xSource(::comphelper::getProcessServiceFactory()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.io.ObjectOutputStream")), UNO_QUERY);
    xSource->setOutputStream(xMarkOut);

    // write the string to source
    Reference< ::com::sun::star::io::XObjectOutputStream> xOutStrm(xSource, UNO_QUERY);
    xOutStrm->writeObject(_xObj);
    xOutStrm->closeOutput();

    Reference< ::com::sun::star::io::XObjectInputStream> xInStrm(xSink, UNO_QUERY);
    xClone = xInStrm->readObject();
    xInStrm->closeInput();

    return xClone;
}

//------------------------------------------------------------------------------
Reference< XInterface> cloneUsingProperties(const Reference< ::com::sun::star::io::XPersistObject>& _xObj)
{
    if (!_xObj.is())
        return Reference< XInterface>();

    // create a new object
    ::rtl::OUString aObjectService = _xObj->getServiceName();
    Reference< ::com::sun::star::beans::XPropertySet> xDestSet(::comphelper::getProcessServiceFactory()->createInstance(aObjectService), UNO_QUERY);
    if (!xDestSet.is())
    {
        DBG_ERROR("cloneUsingProperties : could not instantiate an object of the given type !");
        return Reference< XInterface>();
    }
    // transfer properties
    Reference< ::com::sun::star::beans::XPropertySet> xSourceSet(_xObj, UNO_QUERY);
    Reference< ::com::sun::star::beans::XPropertySetInfo> xSourceInfo( xSourceSet->getPropertySetInfo());
    Sequence< ::com::sun::star::beans::Property> aSourceProperties = xSourceInfo->getProperties();
    Reference< ::com::sun::star::beans::XPropertySetInfo> xDestInfo( xDestSet->getPropertySetInfo());
    Sequence< ::com::sun::star::beans::Property> aDestProperties = xDestInfo->getProperties();
    int nDestLen = aDestProperties.getLength();

    ::com::sun::star::beans::Property* pSourceProps = aSourceProperties.getArray();
    ::com::sun::star::beans::Property* pDestProps = aDestProperties.getArray();

    for (sal_Int16 i=0; i<aSourceProperties.getLength(); ++i)
    {
        ::com::sun::star::beans::Property* pResult = (::com::sun::star::beans::Property*) bsearch(pSourceProps + i, (void*)pDestProps, nDestLen, sizeof(::com::sun::star::beans::Property),
            &PropertyCompare);
        if  (   pResult
            &&  (pResult->Attributes == pSourceProps[i].Attributes)
            &&  ((pResult->Attributes &  ::com::sun::star::beans::PropertyAttribute::READONLY) == 0)
            &&  (pResult->Type.equals(pSourceProps[i].Type))
            )
        {   // Attribute/type are the same and the prop isn't readonly
            try
            {
                xDestSet->setPropertyValue(pResult->Name, xSourceSet->getPropertyValue(pResult->Name));
            }
            catch(::com::sun::star::lang::IllegalArgumentException e)
            {
                e;
#ifdef DBG_UTIL
                ::rtl::OString sMessage("cloneUsingProperties : could not transfer the value for property \"");
                sMessage = sMessage + S(pResult->Name);
                sMessage = sMessage + '\"';
                DBG_ERROR(sMessage);
#endif
            }

        }
    }

    return xDestSet;
}

//------------------------------------------------------------------------------
void CloneForms(const Reference< ::com::sun::star::container::XIndexContainer>& _xSource, const Reference< ::com::sun::star::container::XIndexContainer>& _xDest)
{
    DBG_ASSERT(_xSource.is() && _xDest.is(), "CloneForms : invalid argument !");

    sal_Int32 nSourceCount = _xSource->getCount();
    Reference< ::com::sun::star::sdbc::XRowSet> xCurrent;
    for (sal_Int32 i=nSourceCount-1; i>=0; --i)
    {
        _xSource->getByIndex(i) >>= xCurrent;
        if (!xCurrent.is())
            continue;

        Reference< ::com::sun::star::io::XPersistObject> xCurrentPersist(xCurrent, UNO_QUERY);
        DBG_ASSERT(xCurrentPersist.is(), "CloneForms : a form should always be a PersistObject !");

        // don't use a simple clone on xCurrentPersist as this would clone all childs, too
        Reference< XInterface> xNewObject( cloneUsingProperties(xCurrentPersist));
        Reference< ::com::sun::star::sdbc::XRowSet> xNew(xNewObject, UNO_QUERY);
        if (!xNew.is())
        {
            DBG_ERROR("CloneForms : could not clone a form object !");
            ::comphelper::disposeComponent(xNewObject);
            continue;
        }
        _xDest->insertByIndex(0, makeAny(xNew));

        Reference< ::com::sun::star::container::XIndexContainer> xStepIntoSource(xCurrent, UNO_QUERY);
        Reference< ::com::sun::star::container::XIndexContainer> xStepIntoDest(xNew, UNO_QUERY);
        if (xStepIntoSource.is() && xStepIntoDest.is())
            CloneForms(xStepIntoSource, xStepIntoDest);
    }
}

//------------------------------------------------------------------------------
sal_Bool searchElement(const Reference< ::com::sun::star::container::XIndexAccess>& xCont, const Reference< XInterface>& xElement)
{
    if (!xCont.is() || !xElement.is())
        return sal_False;

    sal_Int32 nCount = xCont->getCount();
    Reference< XInterface> xComp;
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
sal_Int32 getElementPos(const Reference< ::com::sun::star::container::XIndexAccess>& xCont, const Reference< XInterface>& xElement)
{
    sal_Int32 nIndex = -1;
    if (!xCont.is())
        return nIndex;

    Reference< XInterface> xToFind;
    Type xRequestedElementClass( xCont->getElementType());

    if (::comphelper::isA(xRequestedElementClass,(Reference< ::com::sun::star::form::XFormComponent>*)0))
        xToFind = Reference< ::com::sun::star::form::XFormComponent>(xElement, UNO_QUERY);
    else if (::comphelper::isA(xRequestedElementClass,(Reference< ::com::sun::star::form::XForm>*)0))
        xToFind = Reference< ::com::sun::star::form::XForm>(xElement, UNO_QUERY);
    else if (::comphelper::isA(xRequestedElementClass,(Reference< ::com::sun::star::beans::XPropertySet>*)0))
        xToFind = Reference< ::com::sun::star::beans::XPropertySet>(xElement, UNO_QUERY);

    DBG_ASSERT(xToFind.is(), "Unknown Element");
    if (xToFind.is())
    {
        // Feststellen an welcher Position sich das Kind befindet
        nIndex = xCont->getCount();
        while (nIndex--)
        {
            try
            {
                Reference< XInterface> xCurrent;
                ::cppu::extractInterface(xCurrent, xCont->getByIndex(nIndex));
                if (xToFind == xCurrent)
                    break;
            }
            catch(Exception&)
            {
            }

        }
    }
    return nIndex;
}

//------------------------------------------------------------------
String getFormComponentAccessPath(const Reference< XInterface>& _xElement, Reference< XInterface>& _rTopLevelElement)
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
String getFormComponentAccessPath(const Reference< XInterface>& _xElement)
{
    Reference< XInterface> xDummy;
    return getFormComponentAccessPath(_xElement, xDummy);
}

//------------------------------------------------------------------------------
Reference< XInterface> getElementFromAccessPath(const Reference< ::com::sun::star::container::XIndexAccess>& _xParent, const String& _rRelativePath)
{
    if (!_xParent.is())
        return Reference< XInterface>();
    Reference< ::com::sun::star::container::XIndexAccess> xContainer(_xParent);
    Reference< XInterface> xElement( _xParent);

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
sal_Bool hasString(const ::rtl::OUString& aStr, const Sequence< ::rtl::OUString>& rList)
{
    const ::rtl::OUString* pStrList = rList.getConstArray();
    ::rtl::OUString* pResult = (::rtl::OUString*) bsearch(&aStr, (void*)pStrList, rList.getLength(), sizeof(::rtl::OUString),
        &NameCompare);

    return pResult != NULL;
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
void ModifyPropertyAttributes(Sequence< ::com::sun::star::beans::Property>& seqProps, const ::rtl::OUString& ustrPropName, sal_Int16 nAddAttrib, sal_Int16 nRemoveAttrib)
{
    sal_Int32 nLen = seqProps.getLength();

    // binaere Suche
    Type type;
    ::com::sun::star::beans::Property propSearchDummy(ustrPropName, 0, type, 0);
    ::com::sun::star::beans::Property* pResult = (::com::sun::star::beans::Property*) bsearch(&propSearchDummy, (void*)seqProps.getArray(), nLen, sizeof(::com::sun::star::beans::Property),
        &PropertyCompare);

    // gefunden ?
    if (pResult)
    {
        pResult->Attributes |= nAddAttrib;
        pResult->Attributes &= ~nRemoveAttrib;
    }
}

//------------------------------------------------------------------
void RemoveProperty(Sequence< ::com::sun::star::beans::Property>& seqProps, const ::rtl::OUString& ustrPropName)
{
    sal_Int32 nLen = seqProps.getLength();

    // binaere Suche
    Type type;
    ::com::sun::star::beans::Property propSearchDummy(ustrPropName, 0, type, 0);
    const ::com::sun::star::beans::Property* pProperties = seqProps.getConstArray();
    ::com::sun::star::beans::Property* pResult = (::com::sun::star::beans::Property*) bsearch(&propSearchDummy, (void*)pProperties, nLen, sizeof(::com::sun::star::beans::Property),
        &PropertyCompare);

    // gefunden ?
    if (pResult)
    {
        DBG_ASSERT(pResult->Name == ustrPropName, "::RemoveProperty Properties nicht sortiert");
        ::comphelper::removeElementAt(seqProps, pResult - pProperties);
    }
}

//------------------------------------------------------------------
Reference< ::com::sun::star::frame::XModel> getXModel(const Reference< XInterface>& xIface)
{
    Reference< ::com::sun::star::frame::XModel> xModel(xIface, UNO_QUERY);
    if (xModel.is())
        return xModel;
    else
    {
        Reference< ::com::sun::star::container::XChild> xChild(xIface, UNO_QUERY);
        if (xChild.is())
        {
            Reference< XInterface> xParent( xChild->getParent());
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



//------------------------------------------------------------------
//sal_Bool set_impl(Reflection* pRefl, void* pData, const Any& rValue)
//{
//  sal_Bool bRes = sal_True;
//  void* pConv = TypeConversion::to(pRefl, rValue);
//
//  if (!pConv && pRefl->getTypeClass() != TypeClass_ANY)
//      bRes = pRefl->getTypeClass() == TypeClass_VOID;
//  else
//  {
//      switch (pRefl->getTypeClass())
//      {
//          case TypeClass_BOOLEAN:
//              *(sal_Bool*)pData = *(sal_Bool *)pConv; break;
//          case TypeClass_CHAR:
//              *(char*)pData = *(char *)pConv; break;
//          case TypeClass_STRING:
//              *(::rtl::OUString*)pData = *(::rtl::OUString *)pConv; break;
//          case TypeClass_FLOAT:
//              *(float*)pData = *(float *)pConv; break;
//          case TypeClass_DOUBLE:
//              *(double*)pData = *(double *)pConv; break;
//          case TypeClass_BYTE:
//              *(BYTE*)pData = *(BYTE *)pConv; break;
//          case TypeClass_SHORT:
//              *(sal_Int16*)pData = *(sal_Int16 *)pConv; break;
//          case TypeClass_LONG:
//              *(sal_Int32*)pData = *(sal_Int32 *)pConv; break;
//          case TypeClass_UNSIGNED_SHORT:
//              *(sal_uInt16*)pData = *(sal_uInt16 *)pConv; break;
//          case TypeClass_UNSIGNED_LONG:
//              *(sal_uInt32*)pData = *(sal_uInt32 *)pConv; break;
//          case TypeClass_ANY:
//              *(Any*)pData = rValue; break;
//          default:
//              bRes = sal_False;
//      }
//  }
//  return bRes;
//}


//------------------------------------------------------------------------------
sal_uInt32 findValue(const Sequence< Any>& rList, const Any& rValue)
{
    sal_uInt32 nLen = rList.getLength();
    const Any* pArray = (const Any*)rList.getConstArray();
    sal_uInt32 i;
    for (i = 0; i < nLen; i++)
    {
        if (::comphelper::compare(rValue, pArray[i]))
            break;
    }
    return (i < nLen) ? i : LIST_ENTRY_NOTFOUND;
}

//------------------------------------------------------------------------------
Sequence<sal_Int16> findValueINT16(const Sequence< ::rtl::OUString>& rList, const ::rtl::OUString& rValue, sal_Bool bOnlyFirst )
{
    if( bOnlyFirst )
    {
        //////////////////////////////////////////////////////////////////////
        // An welcher Position finde ich den Wert?
        sal_Int32 nPos = -1;
        const ::rtl::OUString* pTArray = (const ::rtl::OUString*)rList.getConstArray();
        for (sal_uInt32 i = 0; i < rList.getLength(); i++)
        {
            if( rValue==pTArray[i] )
            {
                nPos = i;
                break;
            }
        }

        //////////////////////////////////////////////////////////////////////
        // Sequence fuellen
        if( nPos>-1 )
        {
            Sequence<sal_Int16> aRetSeq( 1 );
            aRetSeq.getArray()[0] = (sal_Int16)nPos;

            return aRetSeq;
        }

        return Sequence<sal_Int16>();

    }
    else
    {
        //////////////////////////////////////////////////////////////////////
        // Wie oft kommt der Wert vor?
        sal_uInt32 nCount = 0;
        const ::rtl::OUString* pTArray = (const ::rtl::OUString*)rList.getConstArray();
        sal_uInt32 i;
        for (i = 0; i < rList.getLength(); i++)
        {
            if( rValue==pTArray[i] )
                nCount++;
        }

        //////////////////////////////////////////////////////////////////////
        // Jetzt Sequence fuellen
        Sequence<sal_Int16> aRetSeq( nCount );
        sal_uInt32 j = 0;
        for (i = 0; i < rList.getLength(); i++)
        {
            if( rValue==pTArray[i] )
            {
                aRetSeq.getArray()[j] = (sal_Int16)i;
                j++;
            }
        }

        return aRetSeq;
    }
}

//------------------------------------------------------------------------------
Sequence<sal_Int16> findValue(const Sequence< ::rtl::OUString>& rList, const ::rtl::OUString& rValue, sal_Bool bOnlyFirst )
{
    if( bOnlyFirst )
    {
        //////////////////////////////////////////////////////////////////////
        // An welcher Position finde ich den Wert?
        sal_Int32 nPos = -1;
        const ::rtl::OUString* pTArray = (const ::rtl::OUString*)rList.getConstArray();
        for (sal_uInt32 i = 0; i < rList.getLength(); ++i, ++pTArray)
        {
            if( rValue == *pTArray )
            {
                nPos = i;
                break;
            }
        }

        //////////////////////////////////////////////////////////////////////
        // Sequence fuellen
        if( nPos>-1 )
        {
            Sequence<sal_Int16> aRetSeq( 1 );
            aRetSeq.getArray()[0] = (sal_Int16)nPos;

            return aRetSeq;
        }

        return Sequence<sal_Int16>();

    }

    else
    {
        //////////////////////////////////////////////////////////////////////
        // Wie oft kommt der Wert vor?
        sal_uInt32 nCount = 0;
        const ::rtl::OUString* pTArray = (const ::rtl::OUString*)rList.getConstArray();
        sal_uInt32 i;
        for (i = 0; i < rList.getLength(); i++)
        {
            if( rValue==pTArray[i] )
                ++nCount;
        }

        //////////////////////////////////////////////////////////////////////
        // Jetzt Sequence fuellen
        Sequence<sal_Int16> aRetSeq( nCount );
        sal_uInt32 j = 0;
        for (i = 0; i < rList.getLength(); i++)
        {
            if( rValue==pTArray[i] )
            {
                aRetSeq.getArray()[j] = (sal_Int16)i;
                ++j;
            }
        }

        return aRetSeq;
    }
}

//------------------------------------------------------------------------------
sal_uInt32 findValue1(const Sequence< ::rtl::OUString>& rList, const ::rtl::OUString& rValue)
{
    const ::rtl::OUString* pTArray = (const ::rtl::OUString*)rList.getConstArray();
    ::rtl::OString aStr1 = S(rValue);
    sal_uInt32 i;
    for (i = 0; i < rList.getLength(); i++)
    {
        ::rtl::OString aStr2 = S(pTArray[i]);
        if( rValue==pTArray[i] )
            break;
    }
    return (i < rList.getLength()) ? i : LIST_ENTRY_NOTFOUND;
}


//==================================================================
// StringConversion
//==================================================================
::rtl::OUString AnyToStr( const Any& aValue)
{
    UniString aRetStr;

    switch( aValue.getValueTypeClass() )
    {
        case TypeClass_INTERFACE:       aRetStr.AssignAscii("TYPE INTERFACE");          break;
        case TypeClass_SERVICE:         aRetStr.AssignAscii("TYPE SERVICE");            break;
        case TypeClass_MODULE:          aRetStr.AssignAscii("TYPE MODULE");             break;
        case TypeClass_STRUCT:          aRetStr.AssignAscii("TYPE STRUCT");             break;
        case TypeClass_TYPEDEF:         aRetStr.AssignAscii("TYPE TYPEDEF");            break;
        case TypeClass_UNION:           aRetStr.AssignAscii("TYPE UNION");              break;
        case TypeClass_ENUM:                aRetStr.AssignAscii("TYPE ENUM");               break;
        case TypeClass_EXCEPTION:       aRetStr.AssignAscii("TYPE EXCEPTION");          break;
        case TypeClass_ARRAY:           aRetStr.AssignAscii("TYPE ARRAY");              break;
        case TypeClass_SEQUENCE:            aRetStr.AssignAscii("TYPE SEQUENCE");           break;
        case TypeClass_VOID:                aRetStr.AssignAscii("");                        break;
        case TypeClass_ANY:             aRetStr.AssignAscii("TYPE any");                break;
        case TypeClass_UNKNOWN:         aRetStr.AssignAscii("TYPE unknown");            break;
        case TypeClass_BOOLEAN:         aRetStr = ::comphelper::getBOOL(aValue) ? '1' : '0';    break;
        case TypeClass_CHAR:                aRetStr = String::CreateFromInt32(::comphelper::getINT16(aValue));          break;
        case TypeClass_STRING:          aRetStr = (const sal_Unicode*)::comphelper::getString(aValue);  break;
        //  case TypeClass_FLOAT:           SolarMath::DoubleToString( aRetStr, ::comphelper::getFloat(aValue), 'F', 40, '.', sal_True); break;
        //  case TypeClass_DOUBLE:          SolarMath::DoubleToString( aRetStr, ::comphelper::getDouble(aValue), 'F', 400, '.', sal_True); break;
        case TypeClass_FLOAT:           aRetStr = String::CreateFromFloat( ::comphelper::getFloat(aValue));break;
        case TypeClass_DOUBLE:          aRetStr = String::CreateFromDouble( ::comphelper::getDouble(aValue));break;
                // use SolarMath::DoubleToString instead of sprintf as it is more flexible
                // with respect to the decimal digits (sprintf uses a default value for the number
                // of dec digits and isn't able to cut trailing zeros)
                // 67901 - 27.07.99 - FS
        case TypeClass_BYTE:                aRetStr = String::CreateFromInt32(::comphelper::getINT16(aValue));      break;
        case TypeClass_SHORT:           aRetStr = String::CreateFromInt32(::comphelper::getINT16(aValue));      break;
        case TypeClass_LONG:                aRetStr = String::CreateFromInt32(::comphelper::getINT32(aValue));      break;
        case TypeClass_HYPER:           aRetStr.AssignAscii("TYPE HYPER");          break;
        case TypeClass_UNSIGNED_SHORT:  aRetStr = String::CreateFromInt32(::comphelper::getINT16(aValue));      break;
        case TypeClass_UNSIGNED_LONG:   aRetStr = String::CreateFromInt32(::comphelper::getINT32(aValue));      break;
        case TypeClass_UNSIGNED_HYPER:  aRetStr.AssignAscii("TYPE UNSIGNED_HYPER"); break;
    }

    return aRetStr;
}

// Hilfs-Funktion, um ein ::rtl::OUString in einen Any zu konvertieren
Any StringToAny( ::rtl::OUString _Str, TypeClass eTargetType )
{
    String aStr(_Str);
    Any aRetAny;
    switch( eTargetType )
    {
        case TypeClass_INTERFACE:       break;
        case TypeClass_SERVICE:         break;
        case TypeClass_MODULE:          break;
        case TypeClass_STRUCT:          break;
        case TypeClass_TYPEDEF:         break;
        case TypeClass_UNION:           break;
        case TypeClass_ENUM:                break;
        case TypeClass_EXCEPTION:       break;
        case TypeClass_ARRAY:           break;
        case TypeClass_SEQUENCE:            break;
        case TypeClass_VOID:                break;
        case TypeClass_ANY:             break;
        case TypeClass_UNKNOWN:         break;
        case TypeClass_BOOLEAN:
            {
                sal_Bool bB = (aStr.ToInt32() != 0);
                aRetAny.setValue(&bB,getBooleanCppuType() );
                break;
            }
        case TypeClass_CHAR:
            {
                sal_Char cC = (aStr.GetChar(0));
                aRetAny.setValue(&cC,getCharCppuType() );       break;
            }
        case TypeClass_STRING:          aRetAny <<= _Str;           break;
        case TypeClass_FLOAT:           aRetAny <<= aStr.ToFloat(); break;
        case TypeClass_DOUBLE:          aRetAny <<= aStr.ToDouble(); break;
        case TypeClass_BYTE:                aRetAny <<=  (sal_uInt8)aStr.ToInt32(); break;
        case TypeClass_SHORT:           aRetAny <<=  (sal_Int16)aStr.ToInt32(); break;
        case TypeClass_LONG:                aRetAny <<=  (sal_Int32)aStr.ToInt32(); break;
        case TypeClass_HYPER:           break;
        case TypeClass_UNSIGNED_SHORT:  aRetAny <<=  (sal_uInt16)aStr.ToInt32();    break;
        case TypeClass_UNSIGNED_LONG:   aRetAny <<=  (sal_uInt32)aStr.ToInt32();    break;
        case TypeClass_UNSIGNED_HYPER:  break;
    }
    return aRetAny;
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

    m_xBookmarkOperations   = Reference< ::com::sun::star::sdbcx::XRowLocate>(m_xMoveOperations, UNO_QUERY);
    m_xColumnsSupplier      = Reference< ::com::sun::star::sdbcx::XColumnsSupplier>(m_xMoveOperations, UNO_QUERY);

    if (!m_xMoveOperations.is() || !m_xBookmarkOperations.is() || !m_xColumnsSupplier.is())
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

//==============================================================================
//==============================================================================
//IndexAccessIterator::IndexAccessIterator(Reference< XInterface> xStartingPoint)
//  :m_xStartingPoint(xStartingPoint)
//  ,m_xCurrentObject(NULL)
//{
//  DBG_ASSERT(m_xStartingPoint.is(), "IndexAccessIterator::IndexAccessIterator : no starting point !");
//}
//
//  ------------------------------------------------------------------------------
//Reference< XInterface> IndexAccessIterator::Next()
//{
//  sal_Bool bCheckingStartingPoint = !m_xCurrentObject.is();
//      // ist die aktuelle Node der Anfangspunkt ?
//  sal_Bool bAlreadyCheckedCurrent = m_xCurrentObject.is();
//      // habe ich die aktuelle Node schon mal mittels ShouldHandleElement testen ?
//  if (!m_xCurrentObject.is())
//      m_xCurrentObject = m_xStartingPoint;
//
//  Reference< XInterface> xSearchLoop( m_xCurrentObject);
//  sal_Bool bHasMoreToSearch = sal_True;
//  sal_Bool bFoundSomething = sal_False;
//  while (!bFoundSomething && bHasMoreToSearch)
//  {
//      // pre-order-traversierung
//      if (!bAlreadyCheckedCurrent && ShouldHandleElement(xSearchLoop))
//      {
//          m_xCurrentObject = xSearchLoop;
//          bFoundSomething = sal_True;
//      }
//      else
//      {
//          // zuerst absteigen, wenn moeglich
//          Reference< ::com::sun::star::container::XIndexAccess> xContainerAccess(xSearchLoop, UNO_QUERY);
//          if (xContainerAccess.is() && xContainerAccess->getCount() && ShouldStepInto(xContainerAccess))
//          {   // zum ersten Child
//              Any aElement(xContainerAccess->getByIndex(0));
//              xSearchLoop = *(Reference< XInterface>*)aElement.getValue();
//              bCheckingStartingPoint = sal_False;
//
//              m_arrChildIndizies.Insert(ULONG(0), m_arrChildIndizies.Count());
//          }
//          else
//          {
//              // dann nach oben und nach rechts, wenn moeglich
//              while (m_arrChildIndizies.Count() > 0)
//              {   // (mein Stack ist nich leer, also kann ich noch nach oben gehen)
//                  Reference< ::com::sun::star::container::XChild> xChild(xSearchLoop, UNO_QUERY);
//                  DBG_ASSERT(xChild.is(), "IndexAccessIterator::Next : a content has no approriate interface !");
//
//                  Reference< XInterface> xParent( xChild->getParent());
//                  xContainerAccess = Reference< ::com::sun::star::container::XIndexAccess>(xParent, UNO_QUERY);
//                  DBG_ASSERT(xContainerAccess.is(), "IndexAccessIterator::Next : a content has an invalid parent !");
//
//                  // den Index, den SearchLoop in diesem Parent hatte, von meinem 'Stack'
//                  ULONG nOldSearchChildIndex = m_arrChildIndizies.GetObject(m_arrChildIndizies.Count() - 1);
//                  m_arrChildIndizies.Remove(m_arrChildIndizies.Count() - 1);
//
//                  if (nOldSearchChildIndex < xContainerAccess->getCount() - 1)
//                  {   // auf dieser Ebene geht es noch nach rechts
//                      ++nOldSearchChildIndex;
//                      // also das naechste Child
//                      Any aElement(xContainerAccess->getByIndex(nOldSearchChildIndex));
//                      xSearchLoop = *(Reference< XInterface>*) aElement.getValue();
//                      bCheckingStartingPoint = sal_False;
//                      // und dessen Position auf den 'Stack'
//                      m_arrChildIndizies.Insert(ULONG(nOldSearchChildIndex), m_arrChildIndizies.Count());
//
//                      break;
//                  }
//                  // hierher komme ich, wenn es auf der aktuellen Ebene nicht nach rechts geht, dann mache ich eine darueber weiter
//                  xSearchLoop = xParent;
//                  bCheckingStartingPoint = sal_False;
//              }
//
//              if ((m_arrChildIndizies.Count() == 0) && !bCheckingStartingPoint)
//              {   // das ist genau dann der Fall, wenn ich keinen rechten Nachbarn fuer irgendeinen der direkten Vorfahren des
//                  // urspruenglichen xSearchLoop gefunden habe
//                  bHasMoreToSearch = sal_False;
//              }
//          }
//
//          if (bHasMoreToSearch)
//          {   // ich habe in xSearchLoop jetzt ein Interface eines 'Knotens' meines 'Baumes', den ich noch abtesten kann
//              if (ShouldHandleElement(xSearchLoop))
//              {
//                  m_xCurrentObject = xSearchLoop;
//                  bFoundSomething = sal_True;
//              }
//              else
//                  if (bCheckingStartingPoint)
//                      // ich bin noch am Anfang, konnte nicht absteigen, und habe an diesem Anfang nix gefunden -> nix mehr zu tun
//                      bHasMoreToSearch = sal_False;
//              bAlreadyCheckedCurrent = sal_True;
//          }
//      }
//  }
//
//  if (!bFoundSomething)
//  {
//      DBG_ASSERT(m_arrChildIndizies.Count() == 0, "IndexAccessIterator::Next : items left on stack ! how this ?");
//      Invalidate();
//  }
//
//  return m_xCurrentObject;
//}


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

/*
sal_Int16 getControlTypeByModelName(const ::rtl::OUString& rModel)
{
    if (rModel.equals(FM_COMPONENT_EDIT))   // 5.0-Name
        return OBJ_FM_EDIT;
    if (rModel.equals(FM_COMPONENT_TEXTFIELD))
        return OBJ_FM_EDIT;
    if (rModel.equals(FM_COMPONENT_COMMANDBUTTON))
        return OBJ_FM_BUTTON;
    if (rModel.equals(FM_COMPONENT_FIXEDTEXT))
        return OBJ_FM_FIXEDTEXT;
    if (rModel.equals(FM_COMPONENT_LISTBOX))
        return OBJ_FM_LISTBOX;
    if (rModel.equals(FM_COMPONENT_CHECKBOX))
        return OBJ_FM_CHECKBOX;
    if (rModel.equals(FM_COMPONENT_RADIOBUTTON))
        return OBJ_FM_RADIOBUTTON;
    if (rModel.equals(FM_COMPONENT_GROUPBOX))
        return OBJ_FM_GROUPBOX;
    if (rModel.equals(FM_COMPONENT_COMBOBOX))
        return OBJ_FM_COMBOBOX;
    if (rModel.equals(FM_COMPONENT_GRID))   // 5.0-Name
        return OBJ_FM_GRID;
    if (rModel.equals(FM_COMPONENT_GRIDCONTROL))
        return OBJ_FM_GRID;
    if (rModel.equals(FM_COMPONENT_IMAGEBUTTON))
        return OBJ_FM_IMAGEBUTTON;
    if (rModel.equals(FM_COMPONENT_FILECONTROL))
        return OBJ_FM_FILECONTROL;
    if (rModel.equals(FM_COMPONENT_DATEFIELD))
        return OBJ_FM_DATEFIELD;
    if (rModel.equals(FM_COMPONENT_TIMEFIELD))
        return OBJ_FM_TIMEFIELD;
    if (rModel.equals(FM_COMPONENT_NUMERICFIELD))
        return OBJ_FM_NUMERICFIELD;
    if (rModel.equals(FM_COMPONENT_CURRENCYFIELD))
        return OBJ_FM_CURRENCYFIELD;
    if (rModel.equals(FM_COMPONENT_PATTERNFIELD))
        return OBJ_FM_PATTERNFIELD;
    if (rModel.equals(FM_COMPONENT_HIDDEN)) // 5.0-Name
        return OBJ_FM_HIDDEN;
    if (rModel.equals(FM_COMPONENT_HIDDENCONTROL))
        return OBJ_FM_HIDDEN;
    if (rModel.equals(FM_COMPONENT_IMAGECONTROL))
        return OBJ_FM_IMAGECONTROL;
    if (rModel.equals(FM_COMPONENT_FORMATTEDFIELD))
        return OBJ_FM_FORMATTEDFIELD;
    return OBJ_FM_CONTROL;
}
*/

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
        *pNames = pType->ppMembers[i]->pTypeName;
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
sal_Int16   GridModel2ViewPos(const Reference< ::com::sun::star::container::XIndexAccess>& rColumns, sal_Int16 nModelPos)
{
    try
    {
        if (rColumns.is())
        {
            // invalid pos ?
            if (nModelPos >= rColumns->getCount())
                return (sal_Int16)-1;

            // the column itself shouldn't be hidden
            Reference< ::com::sun::star::beans::XPropertySet> xAskedFor;
            rColumns->getByIndex(nModelPos) >>= xAskedFor;
            if (::comphelper::getBOOL(xAskedFor->getPropertyValue(FM_PROP_HIDDEN)))
            {
                DBG_ERROR("GridModel2ViewPos : invalid argument !");
                return (sal_Int16)-1;
            }

            sal_Int16 nViewPos = nModelPos;
            Reference< ::com::sun::star::beans::XPropertySet> xCur;
            for (sal_Int16 i=0; i<nModelPos; ++i)
            {
                rColumns->getByIndex(i) >>= xCur;
                if (::comphelper::getBOOL(xCur->getPropertyValue(FM_PROP_HIDDEN)))
                    --nViewPos;
            }
            return nViewPos;
        }
    }
    catch(...)
    {
        DBG_ERROR("GridModel2ViewPos Exception occured!");
    }
    return (sal_Int16)-1;
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
    catch(...)
    {
        DBG_ERROR("GridView2ModelPos Exception occured!");
    }
    return (sal_Int16)-1;
}

//------------------------------------------------------------------------------
sal_Int16   GridViewColumnCount(const Reference< ::com::sun::star::container::XIndexAccess>& rColumns)
{
    try
    {
        if (rColumns.is())
        {
            sal_Int16 nCount = rColumns->getCount();
            // loop through all columns
            Reference< ::com::sun::star::beans::XPropertySet> xCur;
            for (sal_Int16 i=0; i<rColumns->getCount(); ++i)
            {
                rColumns->getByIndex(i) >>= xCur;
                if (::comphelper::getBOOL(xCur->getPropertyValue(FM_PROP_HIDDEN)))
                    --nCount;
            }
            return nCount;
        }
    }
    catch(...)
    {
        DBG_ERROR("GridView2ModelPos Exception occured!");
    }
    return 0;
}

// dbtools
//------------------------------------------------------------------------------
UniString quoteName(const UniString& rQuote, const UniString& rName)
{
    UniString sReturn(rQuote);
    sReturn += rName;
    sReturn += rQuote;
    return sReturn;
}

//------------------------------------------------------------------------------
UniString quoteTableName(const Reference< ::com::sun::star::sdbc::XDatabaseMetaData>& _rxMeta, const UniString& rName)
{
    UniString sQuote = _rxMeta->getIdentifierQuoteString();
    UniString sQuotedName;
    UniString aTableName(rName);

    if (_rxMeta->supportsCatalogsInDataManipulation())
    {
        sal_Unicode cSeparator = '.';
        UniString sSeparator = _rxMeta->getCatalogSeparator();
        if (sSeparator.Len() != 0)
            cSeparator = sSeparator.GetChar(0);

        if (aTableName.GetTokenCount(cSeparator) >= 2)
        {
            UniString aDatabaseName(aTableName.GetToken(0,cSeparator));
            sQuotedName += quoteName(sQuote, aDatabaseName);
            sQuotedName += cSeparator;
            aTableName.Erase(0, aDatabaseName.Len() + 1);
        }
    }
    if (_rxMeta->supportsSchemasInDataManipulation())
    {
        if (aTableName.GetTokenCount('.') == 2)
        {
            sQuotedName += quoteName(sQuote, aTableName.GetToken(0,'.'));
            sQuotedName += '.';
            sQuotedName += quoteName(sQuote, aTableName.GetToken(1,'.'));
        }
        else
            sQuotedName += quoteName(sQuote, aTableName);
    }
    else
        sQuotedName += quoteName(sQuote, aTableName);

    return sQuotedName;
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
        "FmSlotDispatch::dispatch : invalid argument !");
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
        "FmSlotDispatch::dispatch : invalid argument !");
    m_aStatusListeners.removeInterface( xControl );
}

//------------------------------------------------------------------------------
void SAL_CALL FmSlotDispatch::dispose(  ) throw(RuntimeException)
{
    ::com::sun::star::lang::EventObject aEvt(Reference< XInterface >(*this));
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
void FmSlotDispatch::StateChanged(sal_Int16 nSID, SfxItemState eState, const SfxPoolItem* pState)
{
    DBG_ASSERT(nSID == m_nSlot, "FmSlotDispatch::StateChanged : where did this come from ?");

    ::com::sun::star::frame::FeatureStateEvent eEvent = BuildEvent(eState, pState);
    NOTIFY_LISTENERS(m_aStatusListeners, ::com::sun::star::frame::XStatusListener, statusChanged, eEvent);
}

//------------------------------------------------------------------------------
::com::sun::star::frame::FeatureStateEvent FmSlotDispatch::BuildEvent(SfxItemState eState, const SfxPoolItem* pState)
{
    ::com::sun::star::frame::FeatureStateEvent aReturn;
    aReturn.Source = (Reference< XInterface >)(*this);
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

// search in the hierarchy for a connection
//------------------------------------------------------------------------------
Reference< ::com::sun::star::sdbc::XConnection> findConnection(const Reference< XInterface>& xParent)
{
    Reference< ::com::sun::star::sdbc::XConnection> xConnection(xParent, UNO_QUERY);
    if (!xConnection.is())
    {
        Reference< ::com::sun::star::container::XChild> xChild(xParent, UNO_QUERY);
        if (xChild.is())
            return findConnection(xChild->getParent());
    }
    return xConnection;
}

//========================================================================
//= FmXDispatchInterceptorImpl
//========================================================================

DBG_NAME(FmXDispatchInterceptorImpl);
//------------------------------------------------------------------------
FmXDispatchInterceptorImpl::FmXDispatchInterceptorImpl(
            const Reference< XDispatchProviderInterception>& _rToIntercept, FmDispatchInterceptor* _pMaster,
            sal_Int16 _nId, Sequence< ::rtl::OUString > _rInterceptedSchemes)
    :FmXDispatchInterceptorImpl_BASE(_pMaster && _pMaster->getInterceptorMutex() ? *_pMaster->getInterceptorMutex() : m_aFallback)
    ,m_xIntercepted(_rToIntercept)
    ,m_pMaster(_pMaster)
    ,m_nId(_nId)
    ,m_aInterceptedURLSchemes(_rInterceptedSchemes)
{
    DBG_CTOR(FmXDispatchInterceptorImpl,NULL);

    ::osl::MutexGuard aGuard(getAccessSafety());
    ::comphelper::increment(m_refCount);
    if (m_xIntercepted.is())
    {
        m_xIntercepted->registerDispatchProviderInterceptor((::com::sun::star::frame::XDispatchProviderInterceptor*)this);
        // this should make us the top-level dispatch-provider for the component, via a call to our
        // setDispatchProvider we should have got an fallback for requests we (i.e. our master) cannot fullfill
        Reference< ::com::sun::star::lang::XComponent> xInterceptedComponent(m_xIntercepted, UNO_QUERY);
        if (xInterceptedComponent.is())
            xInterceptedComponent->addEventListener(this);
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
    if (Source.Source == m_xIntercepted)
        ImplDetach();
}

//------------------------------------------------------------------------------
void FmXDispatchInterceptorImpl::ImplDetach()
{
    ::osl::MutexGuard aGuard(getAccessSafety());

    // deregister ourself from the interception component
    if (m_xIntercepted.is())
        m_xIntercepted->releaseDispatchProviderInterceptor((::com::sun::star::frame::XDispatchProviderInterceptor*)this);

    m_xIntercepted = NULL;
    m_pMaster = NULL;
}

//------------------------------------------------------------------------------
void FmXDispatchInterceptorImpl::disposing()
{
    // remove ourself as event listener from the interception component
    Reference< ::com::sun::star::lang::XComponent> xInterceptedComponent(m_xIntercepted, UNO_QUERY);
    if (xInterceptedComponent.is())
        xInterceptedComponent->removeEventListener((::com::sun::star::lang::XEventListener*)this);

    // detach from the interception component
    ImplDetach();

}

//==============================================================================
//==============================================================================

//------------------------------------------------------------------------------
sal_Bool isLoadable(const Reference< XInterface>& xLoad)
{
    // determines whether a form should be loaded or not
    // if there is no datasource or connection there is no reason to load a form
    Reference< ::com::sun::star::beans::XPropertySet> xSet(xLoad, UNO_QUERY);
    if (xSet.is())
    {
        try
        {
            // is there already a active connection
            Reference< XInterface> xConn;
            xSet->getPropertyValue(FM_PROP_ACTIVE_CONNECTION) >>= xConn;
            return (xConn.is() ||
                    ::comphelper::getString(xSet->getPropertyValue(FM_PROP_DATASOURCE)).len() ||
                    ::comphelper::getString(xSet->getPropertyValue(FM_PROP_URL)).len() ||
                    ::findConnection(xLoad).is());
        }
        catch(Exception&)
        {
            DBG_ERROR("isLoadable Exception occured!");
        }

    }
    return sal_False;
}

//------------------------------------------------------------------------------
Reference< ::com::sun::star::container::XNameAccess> getTableFields(const Reference< ::com::sun::star::sdbc::XConnection>& _rxConn, const ::rtl::OUString& _rsName)
{
    Reference< ::com::sun::star::sdbcx::XTablesSupplier> xSupplyTables(_rxConn, UNO_QUERY);
    DBG_ASSERT(xSupplyTables.is(), "::getTableFields : invalid connection !");
        // the conn already said it would support the service sdb::Connection
    Reference< ::com::sun::star::container::XNameAccess> xTables( xSupplyTables->getTables());
    if (xTables.is() && xTables->hasByName(_rsName))
    {
        Reference< ::com::sun::star::sdbcx::XColumnsSupplier> xTableCols;
        xTables->getByName(_rsName) >>= xTableCols;
        DBG_ASSERT(xTableCols.is(), "::getTableFields : invalid table !");
            // the table is expected to support the service sddb::Table, which requires an ::com::sun::star::sdbcx::XColumnsSupplier interface

        Reference< ::com::sun::star::container::XNameAccess> xFieldNames(xTableCols->getColumns(), UNO_QUERY);
        DBG_ASSERT(xFieldNames.is(), "::getTableFields : TableCols->getColumns doesn't export a NameAccess !");
        return xFieldNames;
    }

    return Reference< ::com::sun::star::container::XNameAccess>();
}


//------------------------------------------------------------------------------
Reference< ::com::sun::star::sdbc::XDataSource> getDataSource(const ::rtl::OUString& _rsTitleOrPath)
{
    DBG_ASSERT(_rsTitleOrPath.len(), "::getDataSource : invalid arg !");

    Reference< ::com::sun::star::sdbc::XDataSource>  xReturn;

    // is it a file url ?
    Reference< ::com::sun::star::container::XNameAccess> xNamingContext(::comphelper::getProcessServiceFactory()->createInstance(SRV_SDB_DATABASE_CONTEXT), UNO_QUERY);
    if (xNamingContext.is() && xNamingContext->hasByName(_rsTitleOrPath))
    {
        DBG_ASSERT(Reference< XNamingService>(xNamingContext, UNO_QUERY).is(), "::getDataSource : no NamingService interface on the DatabaseAccessContext !");
        xReturn = Reference< ::com::sun::star::sdbc::XDataSource>(Reference< XNamingService>(xNamingContext, UNO_QUERY)->getRegisteredObject(_rsTitleOrPath), UNO_QUERY);
    }
    return xReturn;
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

// retrieve the current command of a rowset
//------------------------------------------------------------------------------
::rtl::OUString getCommand(const Reference< ::com::sun::star::sdbc::XRowSet>& _rxRowSet, sal_Bool& bEscapeProcessing, Reference< ::com::sun::star::sdbc::XConnection>& xConn)
{
    ::rtl::OUString aReturn;
    Reference< ::com::sun::star::beans::XPropertySet> xRowSetProps(_rxRowSet, UNO_QUERY);
    if (xRowSetProps.is())
    {
        try
        {
            Any aConn( xRowSetProps->getPropertyValue(FM_PROP_ACTIVE_CONNECTION) );
            if (aConn.getValueTypeClass() != TypeClass_INTERFACE)
                return ::rtl::OUString();

            ::cppu::extractInterface(xConn, aConn);

            // which escapeProcessing we use?
            bEscapeProcessing = ::comphelper::getBOOL(xRowSetProps->getPropertyValue(FM_PROP_ESCAPE_PROCESSING));

            // get the command
            ::rtl::OUString aCommand(::comphelper::getString( xRowSetProps->getPropertyValue(FM_PROP_COMMAND) ));

            // and the command type
            sal_Int32 nType = ::comphelper::getINT32(xRowSetProps->getPropertyValue(FM_PROP_COMMANDTYPE));
            switch (nType)
            {
                case ::com::sun::star::sdb::CommandType::TABLE:
                {
                    //  Reference< ::com::sun::star::sdbc::XDatabaseMetaData> xMeta( xConn->getMetaData());
                    UniString aStmt;
                    aStmt.AssignAscii("SELECT * FROM ");
                    aStmt += quoteTableName(xConn->getMetaData(), aCommand);
                    aReturn = aStmt;
                }   break;
                case ::com::sun::star::sdb::CommandType::QUERY:
                {
                    Reference< ::com::sun::star::sdb::XQueriesSupplier> xQueriesAccess(xConn, UNO_QUERY);
                    if (xQueriesAccess.is())
                    {
                        Reference< ::com::sun::star::container::XNameAccess> xQueries(xQueriesAccess->getQueries());
                        if (xQueries->hasByName(aCommand))
                        {
                            Reference< ::com::sun::star::beans::XPropertySet> xQuery;
                            xQueries->getByName(aCommand) >>= xQuery;
                            OSL_ENSHURE(xQuery.is(),"xQuery is null!");
                            aReturn= ::comphelper::getString(xQuery->getPropertyValue(FM_PROP_COMMAND));
                            bEscapeProcessing = ::comphelper::getBOOL(xQuery->getPropertyValue(FM_PROP_ESCAPE_PROCESSING));
                        }
                    }
                }   break;
                default:
                    aReturn = aCommand;
            }


        }
        catch(Exception&)
        {
        }

    }
    return aReturn;
}

//------------------------------------------------------------------------------
Reference< ::com::sun::star::sdb::XSQLQueryComposer> getCurrentSettingsComposer(const Reference< ::com::sun::star::beans::XPropertySet>& _rxRowSetProps)
{
    Reference< ::com::sun::star::sdb::XSQLQueryComposer> xReturn;
    Reference< ::com::sun::star::sdbc::XRowSet> xRowSet(_rxRowSetProps, UNO_QUERY);
    Reference< ::com::sun::star::sdbc::XConnection> xConn( ::dbtools::calcConnection(xRowSet,::comphelper::getProcessServiceFactory()));
    try
    {
        if (xConn.is())     // implies xRowSet.is() implies _rxRowSetProps.is()
        {
            // build the statement the row set is based on (can't use the ActiveCommand property of the set
            // as this reflects the status after the last execute, not the currently set properties

            ::rtl::OUString sStatement;
            // first ensure we have all properties needed
            if (::comphelper::hasProperty(FM_PROP_COMMAND, _rxRowSetProps) && ::comphelper::hasProperty(FM_PROP_COMMANDTYPE, _rxRowSetProps)
                && ::comphelper::hasProperty(FM_PROP_FILTER_CRITERIA, _rxRowSetProps) && ::comphelper::hasProperty(FM_PROP_SORT, _rxRowSetProps)
                && ::comphelper::hasProperty(FM_PROP_ESCAPE_PROCESSING, _rxRowSetProps) && ::comphelper::hasProperty(FM_PROP_APPLYFILTER, _rxRowSetProps))
            {
                sal_Int32 nCommandType = ::comphelper::getINT32(_rxRowSetProps->getPropertyValue(FM_PROP_COMMANDTYPE));
                ::rtl::OUString sCommand = ::comphelper::getString(_rxRowSetProps->getPropertyValue(FM_PROP_COMMAND));
                sal_Bool bEscapeProcessing = ::comphelper::getBOOL(_rxRowSetProps->getPropertyValue(FM_PROP_ESCAPE_PROCESSING));
                switch (nCommandType)
                {
                    case ::com::sun::star::sdb::CommandType::COMMAND:
                        if (!bEscapeProcessing)
                        {   // native sql -> no parsable statement
                            sStatement = ::rtl::OUString::createFromAscii("");
                        }
                        else
                        {
                            sStatement = sCommand;
                        }
                        break;
                    case ::com::sun::star::sdb::CommandType::TABLE:
                    {
                        if (!sCommand.getLength())
                            break;

                        UniString sTableName = quoteTableName(xConn->getMetaData(), sCommand);
                        sStatement = ::rtl::OUString::createFromAscii("SELECT * FROM ");
                        sStatement += sTableName;
                    }
                    break;
                    case ::com::sun::star::sdb::CommandType::QUERY:
                    {
                        // ask the connection for the query
                        Reference< ::com::sun::star::sdb::XQueriesSupplier> xSupplyQueries(xConn, UNO_QUERY);
                        if (!xSupplyQueries.is())
                            break;

                        Reference< ::com::sun::star::container::XNameAccess> xQueries(xSupplyQueries->getQueries(), UNO_QUERY);
                        if (!xQueries.is() || !xQueries->hasByName(sCommand))
                            break;

                        Reference< ::com::sun::star::beans::XPropertySet> xQueryProps;
                        xQueries->getByName(sCommand) >>= xQueryProps;
                        if (!xQueryProps.is())
                            break;

                        //  a native query ?
                        if (!::comphelper::hasProperty(FM_PROP_ESCAPE_PROCESSING, xQueryProps))
                            break;
                        if (!::comphelper::getBOOL(xQueryProps->getPropertyValue(FM_PROP_ESCAPE_PROCESSING)))
                            break;

                        if (!::comphelper::hasProperty(FM_PROP_COMMAND, xQueryProps))
                            break;

                        // the command used by the query
                        sStatement = ::comphelper::getString(xQueryProps->getPropertyValue(FM_PROP_COMMAND));

                        // use an additional composer to build a statement from the query filter/order props
                        Reference< ::com::sun::star::sdb::XSQLQueryComposerFactory> xFactory(xConn, UNO_QUERY);
                        Reference< ::com::sun::star::sdb::XSQLQueryComposer> xLocalComposer;
                        if (xFactory.is())
                            xLocalComposer = xFactory->createQueryComposer();
                        if (!xLocalComposer.is())
                            break;

                        xLocalComposer->setQuery(sStatement);
                        // the sort order
                        if (::comphelper::hasProperty(FM_PROP_SORT, xQueryProps))
                            xLocalComposer->setOrder(::comphelper::getString(xQueryProps->getPropertyValue(FM_PROP_SORT)));

                        sal_Bool bApplyFilter = sal_False;
                        if (::comphelper::hasProperty(FM_PROP_APPLYFILTER, xQueryProps))
                            bApplyFilter = ::comphelper::getBOOL(xQueryProps->getPropertyValue(FM_PROP_APPLYFILTER));

                        if (bApplyFilter)
                        {
                            if (::comphelper::hasProperty(FM_PROP_FILTER_CRITERIA, xQueryProps))
                                xLocalComposer->setFilter(::comphelper::getString(xQueryProps->getPropertyValue(FM_PROP_FILTER_CRITERIA)));
                        }
                        sStatement = xLocalComposer->getComposedQuery();
                    }
                    break;
                    default:
                        DBG_ERROR("::getCurrentSettingsComposer : no table, no query, no statement - what else ?!");
                        break;
                }
            }

            if (sStatement.getLength())
            {
                // create an composer
                Reference< ::com::sun::star::sdb::XSQLQueryComposerFactory> xFactory(xConn, UNO_QUERY);
                if (xFactory.is())
                    xReturn = xFactory->createQueryComposer();
                if (xReturn.is())
                {
                    xReturn->setQuery(sStatement);
                    // append filter/sort
                    xReturn->setOrder(::comphelper::getString(_rxRowSetProps->getPropertyValue(FM_PROP_SORT)));
                    sal_Bool bApplyFilter = ::comphelper::getBOOL(_rxRowSetProps->getPropertyValue(FM_PROP_APPLYFILTER));
                    if (bApplyFilter)
                        xReturn->setFilter(::comphelper::getString(_rxRowSetProps->getPropertyValue(FM_PROP_FILTER_CRITERIA)));
                }
            }
        }
    }
    catch(Exception&)
    {
        DBG_ERROR("::getCurrentSettingsComposer : catched an exception !");
        xReturn = NULL;
    }


    return xReturn;
}

//------------------------------------------------------------------------------
sal_Bool isRowSetAlive(const Reference< XInterface>& _rxRowSet)
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


//==============================================================================
DataColumn::DataColumn(const Reference< ::com::sun::star::beans::XPropertySet>& _rxIFace)
{
    m_xPropertySet = _rxIFace;
    m_xColumn = Reference< ::com::sun::star::sdb::XColumn>(_rxIFace, UNO_QUERY);
    m_xColumnUpdate = Reference< ::com::sun::star::sdb::XColumnUpdate>(_rxIFace, UNO_QUERY);

    if (!m_xPropertySet.is() || !m_xColumn.is())
    {
        m_xPropertySet = NULL;
        m_xColumn = NULL;
        m_xColumnUpdate = NULL;
    }
}



