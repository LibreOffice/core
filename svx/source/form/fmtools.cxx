/*************************************************************************
 *
 *  $RCSfile: fmtools.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:17 $
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

#ifndef _SVT_SDBPARSE_HXX
#include <svtools/sdbparse.hxx>
#endif

#ifndef _CPPUHELPER_SERVICEFACTORY_HXX_
#include <cppuhelper/servicefactory.hxx>
#endif
#ifndef _UTL_TYPES_HXX_
#include <unotools/types.hxx>
#endif
#ifndef _UTL_PROPERTY_HXX_
#include <unotools/property.hxx>
#endif
#ifndef _UTL_CONTAINER_HXX_
#include <unotools/container.hxx>
#endif
#ifndef _UTL_UNO3_DB_TOOLS_HXX_
#include <unotools/dbtools.hxx>
#endif
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX_
#include <unotools/processfactory.hxx>
#endif
#ifndef _UTL_SEQUENCE_HXX_
#include <unotools/sequence.hxx>
#endif
#ifndef _FM_STATIC_HXX_
#include "fmstatic.hxx"
#endif
#ifndef _CPPUHELPER_EXTRACT_HXX_
#include <cppuhelper/extract.hxx>
#endif
#ifndef _UTL_UNO3_HXX_
#include <unotools/uno3.hxx>
#endif
#ifndef _UTL_DB_EXCEPTION_HXX_
#include <unotools/dbexception.hxx>
#endif
#ifndef _CPPUHELPER_EXTRACT_HXX_
#include <cppuhelper/extract.hxx>
#endif


IMPLEMENT_CONSTASCII_USTRING(DATA_MODE,"DataMode");
IMPLEMENT_CONSTASCII_USTRING(FILTER_MODE,"FilterMode");

Date STANDARD_DATE(1,1,1900);
//------------------------------------------------------------------------------
double ToStandardDate(const Date& rNullDate, double rVal)
{
    if (rNullDate != STANDARD_DATE)
        rVal -= (STANDARD_DATE - rNullDate);
    return rVal;
}

//------------------------------------------------------------------
double ToNullDate(const Date& rNullDate, double rVal)
{
    if (rNullDate != STANDARD_DATE)
        rVal += (STANDARD_DATE - rNullDate);
    return rVal;
}

//------------------------------------------------------------------------------
SdbSqlParser& getSQLParser()
{
    static SdbSqlParser aParser(Application::GetAppInternational().GetLanguage());
    return aParser;
}

//==============================================================================
//------------------------------------------------------------------------------
void displayException(const ::utl::SQLExceptionInfo& _rError)
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    if (_rError.isKindOf(::utl::SQLExceptionInfo::SQL_EXCEPTION))
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
    ::utl::SQLExceptionInfo aInfo(_rExcept);
    displayException(aInfo);
}

//  ------------------------------------------------------------------------------
void displayException(const ::com::sun::star::sdb::SQLErrorEvent& _rEvent, WinBits nStyle)
{
    ::utl::SQLExceptionInfo aInfo(_rEvent);
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
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> clone(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XPersistObject>& _xObj)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> xClone;
    if (!_xObj.is())
        return ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>();

    // ::std::copy it by streaming

    // creating a pipe
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream> xOutPipe(::utl::getProcessServiceFactory()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.io.Pipe")), ::com::sun::star::uno::UNO_QUERY);
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream> xInPipe(xOutPipe, ::com::sun::star::uno::UNO_QUERY);

    // creating the mark streams
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream> xMarkIn(::utl::getProcessServiceFactory()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.io.MarkableInputStream")), ::com::sun::star::uno::UNO_QUERY);
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XActiveDataSink> xMarkSink(xMarkIn, ::com::sun::star::uno::UNO_QUERY);
    xMarkSink->setInputStream(xInPipe);

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream> xMarkOut(::utl::getProcessServiceFactory()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.io.MarkableOutputStream")), ::com::sun::star::uno::UNO_QUERY);
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XActiveDataSource> xMarkSource(xMarkOut, ::com::sun::star::uno::UNO_QUERY);
    xMarkSource->setOutputStream(xOutPipe);

    // connect mark and sink
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XActiveDataSink> xSink(::utl::getProcessServiceFactory()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.io.ObjectInputStream")), ::com::sun::star::uno::UNO_QUERY);
    xSink->setInputStream(xMarkIn);

    // connect mark and source
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XActiveDataSource> xSource(::utl::getProcessServiceFactory()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.io.ObjectOutputStream")), ::com::sun::star::uno::UNO_QUERY);
    xSource->setOutputStream(xMarkOut);

    // write the string to source
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream> xOutStrm(xSource, ::com::sun::star::uno::UNO_QUERY);
    xOutStrm->writeObject(_xObj);
    xOutStrm->closeOutput();

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream> xInStrm(xSink, ::com::sun::star::uno::UNO_QUERY);
    xClone = xInStrm->readObject();
    xInStrm->closeInput();

    return xClone;
}

//------------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> cloneUsingProperties(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XPersistObject>& _xObj)
{
    if (!_xObj.is())
        return ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>();

    // create a new object
    ::rtl::OUString aObjectService = _xObj->getServiceName();
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xDestSet(::utl::getProcessServiceFactory()->createInstance(aObjectService), ::com::sun::star::uno::UNO_QUERY);
    if (!xDestSet.is())
    {
        DBG_ERROR("cloneUsingProperties : could not instantiate an object of the given type !");
        return ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>();
    }
    // transfer properties
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xSourceSet(_xObj, ::com::sun::star::uno::UNO_QUERY);
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo> xSourceInfo( xSourceSet->getPropertySetInfo());
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property> aSourceProperties = xSourceInfo->getProperties();
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo> xDestInfo( xDestSet->getPropertySetInfo());
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property> aDestProperties = xDestInfo->getProperties();
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
void CloneForms(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer>& _xSource, const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer>& _xDest)
{
    DBG_ASSERT(_xSource.is() && _xDest.is(), "CloneForms : invalid argument !");

    sal_Int32 nSourceCount = _xSource->getCount();
    for (sal_Int32 i=nSourceCount-1; i>=0; --i)
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet> xCurrent(*(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>*)_xSource->getByIndex(i).getValue(), ::com::sun::star::uno::UNO_QUERY);
        if (!xCurrent.is())
            continue;

        ::com::sun::star::uno::Reference< ::com::sun::star::io::XPersistObject> xCurrentPersist(xCurrent, ::com::sun::star::uno::UNO_QUERY);
        DBG_ASSERT(xCurrentPersist.is(), "CloneForms : a form should always be a PersistObject !");

        // don't use a simple clone on xCurrentPersist as this would clone all childs, too
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> xNewObject( cloneUsingProperties(xCurrentPersist));
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet> xNew(xNewObject, ::com::sun::star::uno::UNO_QUERY);
        if (!xNew.is())
        {
            DBG_ERROR("CloneForms : could not clone a form object !");
            ::utl::disposeComponent(xNewObject);
            continue;
        }
        _xDest->insertByIndex(0, ::com::sun::star::uno::makeAny(xNew));

        ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer> xStepIntoSource(xCurrent, ::com::sun::star::uno::UNO_QUERY);
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer> xStepIntoDest(xNew, ::com::sun::star::uno::UNO_QUERY);
        if (xStepIntoSource.is() && xStepIntoDest.is())
            CloneForms(xStepIntoSource, xStepIntoDest);
    }
}

//------------------------------------------------------------------------------
sal_Bool searchElement(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess>& xCont, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& xElement)
{
    if (!xCont.is() || !xElement.is())
        return sal_False;

    sal_Int32 nCount = xCont->getCount();
    ::com::sun::star::uno::Any aRet;
    for (sal_Int32 i = 0; i < nCount; i++)
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> xComp;
        try
        {
            aRet = xCont->getByIndex(i);
            if (aRet.hasValue())
            {
                ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> xIface(*(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>*) aRet.getValue());
                ::utl::query_interface(xIface, xComp);
                if (((::com::sun::star::uno::XInterface *)xElement.get()) == (::com::sun::star::uno::XInterface*)xComp.get())
                    return sal_True;
                else
                {
                    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess> xCont2(xComp, ::com::sun::star::uno::UNO_QUERY);
                    if (xCont2.is() && searchElement(xCont2, xElement))
                        return sal_True;
                }
            }
        }
        catch(...)
        {
        }
    }
    return sal_False;
}

//------------------------------------------------------------------------------
sal_Int32 getElementPos(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess>& xCont, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& xElement)
{
    sal_Int32 nIndex = -1;
    if (!xCont.is())
        return nIndex;

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> xToFind;
    ::com::sun::star::uno::Type xRequestedElementClass( xCont->getElementType());

    if (::utl::isA(xRequestedElementClass,(::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent>*)0))
        xToFind = ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent>(xElement, ::com::sun::star::uno::UNO_QUERY);
    else if (::utl::isA(xRequestedElementClass,(::com::sun::star::uno::Reference< ::com::sun::star::form::XForm>*)0))
        xToFind = ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm>(xElement, ::com::sun::star::uno::UNO_QUERY);
    else if (::utl::isA(xRequestedElementClass,(::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>*)0))
        xToFind = ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>(xElement, ::com::sun::star::uno::UNO_QUERY);

    DBG_ASSERT(xToFind.is(), "Unknown Element");
    if (xToFind.is())
    {
        // Feststellen an welcher Position sich das Kind befindet
        nIndex = xCont->getCount();
        while (nIndex--)
        {
            try
            {
                ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> xCurrent;
                ::cppu::extractInterface(xCurrent, xCont->getByIndex(nIndex));
                if (xToFind == xCurrent)
                    break;
            }
            catch(...)
            {
            }

        }
    }
    return nIndex;
}

//------------------------------------------------------------------
String getFormComponentAccessPath(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& _xElement, ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& _rTopLevelElement)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent> xChild(_xElement, ::com::sun::star::uno::UNO_QUERY);
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess> xParent;
    if (xChild.is())
        xParent = ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess>(xChild->getParent(), ::com::sun::star::uno::UNO_QUERY);

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
        if (::utl::query_interface((::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >)xParent,xChild))
            xParent = ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess>(xChild->getParent(), ::com::sun::star::uno::UNO_QUERY);
    }

    _rTopLevelElement = xParent;
    return sReturn;
}

//------------------------------------------------------------------
String getFormComponentAccessPath(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& _xElement)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> xDummy;
    return getFormComponentAccessPath(_xElement, xDummy);
}

//------------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> getElementFromAccessPath(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess>& _xParent, const String& _rRelativePath)
{
    if (!_xParent.is())
        return ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>();
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess> xContainer(_xParent);
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> xElement( _xParent);

    String sPath(_rRelativePath);
    while (sPath.Len() && xContainer.is())
    {
        sal_Int32 nSepPos = sPath.Search((sal_Unicode)'\\');

        String sIndex(sPath.Copy(0, (nSepPos == -1) ? sPath.Len() : nSepPos));
        //  DBG_ASSERT(sIndex.IsNumeric(), "getElementFromAccessPath : invalid path !");

        sPath = sPath.Copy((nSepPos == -1) ? sPath.Len() : nSepPos+1);

        ::cppu::extractInterface(xElement, xContainer->getByIndex(sIndex.ToInt32()));
        xContainer = ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess>::query(xElement);
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
sal_Bool hasString(const ::rtl::OUString& aStr, const ::com::sun::star::uno::Sequence< ::rtl::OUString>& rList)
{
    const ::rtl::OUString* pStrList = rList.getConstArray();
    ::rtl::OUString* pResult = (::rtl::OUString*) bsearch(&aStr, (void*)pStrList, rList.getLength(), sizeof(::rtl::OUString),
        &NameCompare);

    return pResult != NULL;
}

//------------------------------------------------------------------------------
sal_Int32 findPos(const ::rtl::OUString& aStr, const ::com::sun::star::uno::Sequence< ::rtl::OUString>& rList)
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
void ModifyPropertyAttributes(::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property>& seqProps, const ::rtl::OUString& ustrPropName, sal_Int16 nAddAttrib, sal_Int16 nRemoveAttrib)
{
    sal_Int32 nLen = seqProps.getLength();

    // binaere Suche
    ::com::sun::star::uno::Type type;
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
void RemoveProperty(::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property>& seqProps, const ::rtl::OUString& ustrPropName)
{
    sal_Int32 nLen = seqProps.getLength();

    // binaere Suche
    ::com::sun::star::uno::Type type;
    ::com::sun::star::beans::Property propSearchDummy(ustrPropName, 0, type, 0);
    const ::com::sun::star::beans::Property* pProperties = seqProps.getConstArray();
    ::com::sun::star::beans::Property* pResult = (::com::sun::star::beans::Property*) bsearch(&propSearchDummy, (void*)pProperties, nLen, sizeof(::com::sun::star::beans::Property),
        &PropertyCompare);

    // gefunden ?
    if (pResult)
    {
        DBG_ASSERT(pResult->Name == ustrPropName, "::RemoveProperty Properties nicht sortiert");
        ::utl::removeElementAt(seqProps, pResult - pProperties);
    }
}

//------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel> getXModel(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& xIface)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel> xModel(xIface, ::com::sun::star::uno::UNO_QUERY);
    if (xModel.is())
        return xModel;
    else
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XChild> xChild(xIface, ::com::sun::star::uno::UNO_QUERY);
        if (xChild.is())
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> xParent( xChild->getParent());
            return getXModel(xParent);
        }
        else
            return NULL;
    }
}

//------------------------------------------------------------------
::rtl::OUString getLabelName(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& xControlModel)
{
    if (!xControlModel.is())
        return ::rtl::OUString();

    if (::utl::hasProperty(FM_PROP_CONTROLLABEL, xControlModel))
    {
        ::com::sun::star::uno::Any aLabelModel( xControlModel->getPropertyValue(FM_PROP_CONTROLLABEL) );
        if (aLabelModel.getValueTypeClass() == ::com::sun::star::uno::TypeClass_INTERFACE)
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xLabelSet(*(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>*)aLabelModel.getValue(), ::com::sun::star::uno::UNO_QUERY);
            if (xLabelSet.is() && ::utl::hasProperty(FM_PROP_LABEL, xLabelSet))
            {
                ::com::sun::star::uno::Any aLabel( xLabelSet->getPropertyValue(FM_PROP_LABEL) );
                if ((aLabel.getValueTypeClass() == ::com::sun::star::uno::TypeClass_STRING) && ::utl::getString(aLabel).getLength())
                    return ::utl::getString(aLabel);
            }
        }
    }

    return ::utl::getString(xControlModel->getPropertyValue(FM_PROP_CONTROLSOURCE));
}



//------------------------------------------------------------------
//sal_Bool set_impl(Reflection* pRefl, void* pData, const ::com::sun::star::uno::Any& rValue)
//{
//  sal_Bool bRes = sal_True;
//  void* pConv = TypeConversion::to(pRefl, rValue);
//
//  if (!pConv && pRefl->getTypeClass() != ::com::sun::star::uno::TypeClass_ANY)
//      bRes = pRefl->getTypeClass() == ::com::sun::star::uno::TypeClass_VOID;
//  else
//  {
//      switch (pRefl->getTypeClass())
//      {
//          case ::com::sun::star::uno::TypeClass_BOOLEAN:
//              *(sal_Bool*)pData = *(sal_Bool *)pConv; break;
//          case ::com::sun::star::uno::TypeClass_CHAR:
//              *(char*)pData = *(char *)pConv; break;
//          case ::com::sun::star::uno::TypeClass_STRING:
//              *(::rtl::OUString*)pData = *(::rtl::OUString *)pConv; break;
//          case ::com::sun::star::uno::TypeClass_FLOAT:
//              *(float*)pData = *(float *)pConv; break;
//          case ::com::sun::star::uno::TypeClass_DOUBLE:
//              *(double*)pData = *(double *)pConv; break;
//          case ::com::sun::star::uno::TypeClass_BYTE:
//              *(BYTE*)pData = *(BYTE *)pConv; break;
//          case ::com::sun::star::uno::TypeClass_SHORT:
//              *(sal_Int16*)pData = *(sal_Int16 *)pConv; break;
//          case ::com::sun::star::uno::TypeClass_LONG:
//              *(sal_Int32*)pData = *(sal_Int32 *)pConv; break;
//          case ::com::sun::star::uno::TypeClass_UNSIGNED_SHORT:
//              *(sal_uInt16*)pData = *(sal_uInt16 *)pConv; break;
//          case ::com::sun::star::uno::TypeClass_UNSIGNED_LONG:
//              *(sal_uInt32*)pData = *(sal_uInt32 *)pConv; break;
//          case ::com::sun::star::uno::TypeClass_ANY:
//              *(::com::sun::star::uno::Any*)pData = rValue; break;
//          default:
//              bRes = sal_False;
//      }
//  }
//  return bRes;
//}


//------------------------------------------------------------------------------
sal_uInt32 findValue(const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any>& rList, const ::com::sun::star::uno::Any& rValue)
{
    sal_uInt32 nLen = rList.getLength();
    const ::com::sun::star::uno::Any* pArray = (const ::com::sun::star::uno::Any*)rList.getConstArray();
    sal_uInt32 i;
    for (i = 0; i < nLen; i++)
    {
        if (::utl::compare(rValue, pArray[i]))
            break;
    }
    return (i < nLen) ? i : LIST_ENTRY_NOTFOUND;
}

//------------------------------------------------------------------------------
::com::sun::star::uno::Sequence<sal_Int16> findValueINT16(const ::com::sun::star::uno::Sequence< ::rtl::OUString>& rList, const ::rtl::OUString& rValue, sal_Bool bOnlyFirst )
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
        // ::com::sun::star::uno::Sequence fuellen
        if( nPos>-1 )
        {
            ::com::sun::star::uno::Sequence<sal_Int16> aRetSeq( 1 );
            aRetSeq.getArray()[0] = (sal_Int16)nPos;

            return aRetSeq;
        }

        return ::com::sun::star::uno::Sequence<sal_Int16>();

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
        // Jetzt ::com::sun::star::uno::Sequence fuellen
        ::com::sun::star::uno::Sequence<sal_Int16> aRetSeq( nCount );
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
::com::sun::star::uno::Sequence<sal_Int16> findValue(const ::com::sun::star::uno::Sequence< ::rtl::OUString>& rList, const ::rtl::OUString& rValue, sal_Bool bOnlyFirst )
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
        // ::com::sun::star::uno::Sequence fuellen
        if( nPos>-1 )
        {
            ::com::sun::star::uno::Sequence<sal_Int16> aRetSeq( 1 );
            aRetSeq.getArray()[0] = (sal_Int16)nPos;

            return aRetSeq;
        }

        return ::com::sun::star::uno::Sequence<sal_Int16>();

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
        // Jetzt ::com::sun::star::uno::Sequence fuellen
        ::com::sun::star::uno::Sequence<sal_Int16> aRetSeq( nCount );
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
sal_uInt32 findValue1(const ::com::sun::star::uno::Sequence< ::rtl::OUString>& rList, const ::rtl::OUString& rValue)
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
::rtl::OUString AnyToStr( const ::com::sun::star::uno::Any& aValue)
{
    UniString aRetStr;

    switch( aValue.getValueTypeClass() )
    {
        case ::com::sun::star::uno::TypeClass_INTERFACE:        aRetStr.AssignAscii("TYPE INTERFACE");          break;
        case ::com::sun::star::uno::TypeClass_SERVICE:          aRetStr.AssignAscii("TYPE SERVICE");            break;
        case ::com::sun::star::uno::TypeClass_MODULE:           aRetStr.AssignAscii("TYPE MODULE");             break;
        case ::com::sun::star::uno::TypeClass_STRUCT:           aRetStr.AssignAscii("TYPE STRUCT");             break;
        case ::com::sun::star::uno::TypeClass_TYPEDEF:          aRetStr.AssignAscii("TYPE TYPEDEF");            break;
        case ::com::sun::star::uno::TypeClass_UNION:            aRetStr.AssignAscii("TYPE UNION");              break;
        case ::com::sun::star::uno::TypeClass_ENUM:             aRetStr.AssignAscii("TYPE ENUM");               break;
        case ::com::sun::star::uno::TypeClass_EXCEPTION:        aRetStr.AssignAscii("TYPE EXCEPTION");          break;
        case ::com::sun::star::uno::TypeClass_ARRAY:            aRetStr.AssignAscii("TYPE ARRAY");              break;
        case ::com::sun::star::uno::TypeClass_SEQUENCE:         aRetStr.AssignAscii("TYPE SEQUENCE");           break;
        case ::com::sun::star::uno::TypeClass_VOID:             aRetStr.AssignAscii("");                        break;
        case ::com::sun::star::uno::TypeClass_ANY:              aRetStr.AssignAscii("TYPE any");                break;
        case ::com::sun::star::uno::TypeClass_UNKNOWN:          aRetStr.AssignAscii("TYPE unknown");            break;
        case ::com::sun::star::uno::TypeClass_BOOLEAN:          aRetStr = ::utl::getBOOL(aValue) ? '1' : '0';   break;
        case ::com::sun::star::uno::TypeClass_CHAR:             aRetStr = String::CreateFromInt32(::utl::getINT16(aValue));         break;
        case ::com::sun::star::uno::TypeClass_STRING:           aRetStr = (const sal_Unicode*)::utl::getString(aValue); break;
        //  case ::com::sun::star::uno::TypeClass_FLOAT:            SolarMath::DoubleToString( aRetStr, ::utl::getFloat(aValue), 'F', 40, '.', sal_True); break;
        //  case ::com::sun::star::uno::TypeClass_DOUBLE:           SolarMath::DoubleToString( aRetStr, ::utl::getDouble(aValue), 'F', 400, '.', sal_True); break;
        case ::com::sun::star::uno::TypeClass_FLOAT:            aRetStr = String::CreateFromFloat( ::utl::getFloat(aValue));break;
        case ::com::sun::star::uno::TypeClass_DOUBLE:           aRetStr = String::CreateFromDouble( ::utl::getDouble(aValue));break;
                // use SolarMath::DoubleToString instead of sprintf as it is more flexible
                // with respect to the decimal digits (sprintf uses a default value for the number
                // of dec digits and isn't able to cut trailing zeros)
                // 67901 - 27.07.99 - FS
        case ::com::sun::star::uno::TypeClass_BYTE:             aRetStr = String::CreateFromInt32(::utl::getINT16(aValue));     break;
        case ::com::sun::star::uno::TypeClass_SHORT:            aRetStr = String::CreateFromInt32(::utl::getINT16(aValue));     break;
        case ::com::sun::star::uno::TypeClass_LONG:             aRetStr = String::CreateFromInt32(::utl::getINT32(aValue));     break;
        case ::com::sun::star::uno::TypeClass_HYPER:            aRetStr.AssignAscii("TYPE HYPER");          break;
        case ::com::sun::star::uno::TypeClass_UNSIGNED_SHORT:   aRetStr = String::CreateFromInt32(::utl::getINT16(aValue));     break;
        case ::com::sun::star::uno::TypeClass_UNSIGNED_LONG:    aRetStr = String::CreateFromInt32(::utl::getINT32(aValue));     break;
        case ::com::sun::star::uno::TypeClass_UNSIGNED_HYPER:   aRetStr.AssignAscii("TYPE UNSIGNED_HYPER"); break;
    }

    return aRetStr;
}

// Hilfs-Funktion, um ein ::rtl::OUString in einen Any zu konvertieren
::com::sun::star::uno::Any StringToAny( ::rtl::OUString _Str, ::com::sun::star::uno::TypeClass eTargetType )
{
    String aStr(_Str);
    ::com::sun::star::uno::Any aRetAny;
    switch( eTargetType )
    {
        case ::com::sun::star::uno::TypeClass_INTERFACE:        break;
        case ::com::sun::star::uno::TypeClass_SERVICE:          break;
        case ::com::sun::star::uno::TypeClass_MODULE:           break;
        case ::com::sun::star::uno::TypeClass_STRUCT:           break;
        case ::com::sun::star::uno::TypeClass_TYPEDEF:          break;
        case ::com::sun::star::uno::TypeClass_UNION:            break;
        case ::com::sun::star::uno::TypeClass_ENUM:             break;
        case ::com::sun::star::uno::TypeClass_EXCEPTION:        break;
        case ::com::sun::star::uno::TypeClass_ARRAY:            break;
        case ::com::sun::star::uno::TypeClass_SEQUENCE:         break;
        case ::com::sun::star::uno::TypeClass_VOID:             break;
        case ::com::sun::star::uno::TypeClass_ANY:              break;
        case ::com::sun::star::uno::TypeClass_UNKNOWN:          break;
        case ::com::sun::star::uno::TypeClass_BOOLEAN:
            {
                sal_Bool bB = (aStr.ToInt32() != 0);
                aRetAny.setValue(&bB,getBooleanCppuType() );
                break;
            }
        case ::com::sun::star::uno::TypeClass_CHAR:
            {
                sal_Char cC = (aStr.GetChar(0));
                aRetAny.setValue(&cC,getCharCppuType() );       break;
            }
        case ::com::sun::star::uno::TypeClass_STRING:           aRetAny <<= _Str;           break;
        case ::com::sun::star::uno::TypeClass_FLOAT:            aRetAny <<= aStr.ToFloat(); break;
        case ::com::sun::star::uno::TypeClass_DOUBLE:           aRetAny <<= aStr.ToDouble(); break;
        case ::com::sun::star::uno::TypeClass_BYTE:             aRetAny <<=  (sal_uInt8)aStr.ToInt32(); break;
        case ::com::sun::star::uno::TypeClass_SHORT:            aRetAny <<=  (sal_Int16)aStr.ToInt32(); break;
        case ::com::sun::star::uno::TypeClass_LONG:             aRetAny <<=  (sal_Int32)aStr.ToInt32(); break;
        case ::com::sun::star::uno::TypeClass_HYPER:            break;
        case ::com::sun::star::uno::TypeClass_UNSIGNED_SHORT:   aRetAny <<=  (sal_uInt16)aStr.ToInt32();    break;
        case ::com::sun::star::uno::TypeClass_UNSIGNED_LONG:    aRetAny <<=  (sal_uInt32)aStr.ToInt32();    break;
        case ::com::sun::star::uno::TypeClass_UNSIGNED_HYPER:   break;
    }
    return aRetAny;
}


//========================================================================
// = CursorWrapper
//------------------------------------------------------------------------
CursorWrapper::CursorWrapper(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet>& _rxCursor, sal_Bool bUseCloned)
{
    ImplConstruct(::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>(_rxCursor, ::com::sun::star::uno::UNO_QUERY), bUseCloned);
}

//------------------------------------------------------------------------
CursorWrapper::CursorWrapper(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>& _rxCursor, sal_Bool bUseCloned)
{
    ImplConstruct(_rxCursor, bUseCloned);
}

//------------------------------------------------------------------------
void CursorWrapper::ImplConstruct(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>& _rxCursor, sal_Bool bUseCloned)
{
    if (bUseCloned)
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XResultSetAccess> xAccess(_rxCursor, ::com::sun::star::uno::UNO_QUERY);
        try
        {
            m_xMoveOperations = xAccess.is() ? xAccess->createResultSet() : ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>();
        }
        catch(...)
        {
        }
    }
    else
        m_xMoveOperations   = _rxCursor;

    m_xBookmarkOperations   = ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XRowLocate>(m_xMoveOperations, ::com::sun::star::uno::UNO_QUERY);
    m_xColumnsSupplier      = ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XColumnsSupplier>(m_xMoveOperations, ::com::sun::star::uno::UNO_QUERY);

    if (!m_xMoveOperations.is() || !m_xBookmarkOperations.is() || !m_xColumnsSupplier.is())
    {   // all or nothing !!
        m_xMoveOperations = NULL;
        m_xBookmarkOperations = NULL;
        m_xColumnsSupplier = NULL;
    }
}

//------------------------------------------------------------------------
const CursorWrapper& CursorWrapper::operator=(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet>& _rxCursor)
{
    m_xMoveOperations = ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>(_rxCursor, ::com::sun::star::uno::UNO_QUERY);
    m_xBookmarkOperations = ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XRowLocate>(_rxCursor, ::com::sun::star::uno::UNO_QUERY);
    m_xColumnsSupplier = ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XColumnsSupplier>(_rxCursor, ::com::sun::star::uno::UNO_QUERY);
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
//IndexAccessIterator::IndexAccessIterator(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> xStartingPoint)
//  :m_xStartingPoint(xStartingPoint)
//  ,m_xCurrentObject(NULL)
//{
//  DBG_ASSERT(m_xStartingPoint.is(), "IndexAccessIterator::IndexAccessIterator : no starting point !");
//}
//
//  ------------------------------------------------------------------------------
//::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> IndexAccessIterator::Next()
//{
//  sal_Bool bCheckingStartingPoint = !m_xCurrentObject.is();
//      // ist die aktuelle Node der Anfangspunkt ?
//  sal_Bool bAlreadyCheckedCurrent = m_xCurrentObject.is();
//      // habe ich die aktuelle Node schon mal mittels ShouldHandleElement testen ?
//  if (!m_xCurrentObject.is())
//      m_xCurrentObject = m_xStartingPoint;
//
//  ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> xSearchLoop( m_xCurrentObject);
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
//          ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess> xContainerAccess(xSearchLoop, ::com::sun::star::uno::UNO_QUERY);
//          if (xContainerAccess.is() && xContainerAccess->getCount() && ShouldStepInto(xContainerAccess))
//          {   // zum ersten Child
//              ::com::sun::star::uno::Any aElement(xContainerAccess->getByIndex(0));
//              xSearchLoop = *(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>*)aElement.getValue();
//              bCheckingStartingPoint = sal_False;
//
//              m_arrChildIndizies.Insert(ULONG(0), m_arrChildIndizies.Count());
//          }
//          else
//          {
//              // dann nach oben und nach rechts, wenn moeglich
//              while (m_arrChildIndizies.Count() > 0)
//              {   // (mein Stack ist nich leer, also kann ich noch nach oben gehen)
//                  ::com::sun::star::uno::Reference< ::com::sun::star::container::XChild> xChild(xSearchLoop, ::com::sun::star::uno::UNO_QUERY);
//                  DBG_ASSERT(xChild.is(), "IndexAccessIterator::Next : a content has no approriate interface !");
//
//                  ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> xParent( xChild->getParent());
//                  xContainerAccess = ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess>(xParent, ::com::sun::star::uno::UNO_QUERY);
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
//                      ::com::sun::star::uno::Any aElement(xContainerAccess->getByIndex(nOldSearchChildIndex));
//                      xSearchLoop = *(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>*) aElement.getValue();
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
FmXDisposeMultiplexer::FmXDisposeMultiplexer(FmXDisposeListener* _pListener, const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent>& _rxObject, sal_Int16 _nId)
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
void FmXDisposeMultiplexer::disposing(const ::com::sun::star::lang::EventObject& _Source) throw( ::com::sun::star::uno::RuntimeException )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener> xPreventDelete(this);

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
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener> xPreventDelete(this);

        m_xObject->removeEventListener(this);
        m_xObject = NULL;

        m_pListener->setAdapter(NULL);
        m_pListener = NULL;
    }
}

//==============================================================================
//------------------------------------------------------------------------------
sal_Int16 getControlTypeByObject(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XServiceInfo>& _rxObject)
{
    // ask for the persistent service name
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XPersistObject> xPersistence(_rxObject, ::com::sun::star::uno::UNO_QUERY);
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
::com::sun::star::uno::Sequence< ::rtl::OUString> getEventMethods(const ::com::sun::star::uno::Type& type)
{
    typelib_InterfaceTypeDescription *pType=0;
    type.getDescription( (typelib_TypeDescription**)&pType);

    if(!pType)
        return ::com::sun::star::uno::Sequence< ::rtl::OUString>();

    ::com::sun::star::uno::Sequence< ::rtl::OUString> aNames(pType->nMembers);
    ::rtl::OUString* pNames = aNames.getArray();
    for(sal_Int32 i=0;i<pType->nMembers;i++,++pNames)
    {
        *pNames = pType->ppMembers[i]->pTypeName;
    }

    typelib_typedescription_release( (typelib_TypeDescription *)pType );
    return aNames;
}


//------------------------------------------------------------------------------
void TransferEventScripts(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel>& xModel, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl>& xControl,
    const ::com::sun::star::uno::Sequence< ::com::sun::star::script::ScriptEventDescriptor>& rTransferIfAvailable)
{
    // first check if we have a XEventAttacherManager for the model
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XChild> xModelChild(xModel, ::com::sun::star::uno::UNO_QUERY);
    if (!xModelChild.is())
        return; // nothing to do

    ::com::sun::star::uno::Reference< ::com::sun::star::script::XEventAttacherManager> xEventManager(xModelChild->getParent(), ::com::sun::star::uno::UNO_QUERY);
    if (!xEventManager.is())
        return; // nothing to do

    if (!rTransferIfAvailable.getLength())
        return; // nothing to do

    // check for the index of the model within it's parent
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess> xParentIndex(xModelChild->getParent(), ::com::sun::star::uno::UNO_QUERY);
    if (!xParentIndex.is())
        return; // nothing to do
    sal_Int32 nIndex = getElementPos(xParentIndex, xModel);
    if (nIndex<0 || nIndex>=xParentIndex->getCount())
        return; // nothing to do

    // then we need informations about the listeners supported by the control and the model
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type>   aModelListeners;
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type>   aControlListeners;

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XIntrospection> xModelIntrospection(::utl::getProcessServiceFactory()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.beans.Introspection")), ::com::sun::star::uno::UNO_QUERY);
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XIntrospection> xControlIntrospection(::utl::getProcessServiceFactory()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.beans.Introspection")), ::com::sun::star::uno::UNO_QUERY);

    if (xModelIntrospection.is() && xModel.is())
    {
        ::com::sun::star::uno::Any aModel(::com::sun::star::uno::makeAny(xModel));
        aModelListeners = xModelIntrospection->inspect(aModel)->getSupportedListeners();
    }

    if (xControlIntrospection.is() && xControl.is())
    {
        ::com::sun::star::uno::Any aControl(::com::sun::star::uno::makeAny(xControl));
        aControlListeners = xControlIntrospection->inspect(aControl)->getSupportedListeners();
    }

    sal_Int32 nMaxNewLen = aModelListeners.getLength() + aControlListeners.getLength();
    if (!nMaxNewLen)
        return; // the model and the listener don't support any listeners (or we were unable to retrieve these infos)

    ::com::sun::star::uno::Sequence< ::com::sun::star::script::ScriptEventDescriptor>   aTransferable(nMaxNewLen);
    ::com::sun::star::script::ScriptEventDescriptor* pTransferable = aTransferable.getArray();

    const ::com::sun::star::script::ScriptEventDescriptor* pCurrent = rTransferIfAvailable.getConstArray();
    sal_Int32 i,j,k;
    for (i=0; i<rTransferIfAvailable.getLength(); ++i, ++pCurrent)
    {
        // search the model/control idl classes for the event described by pCurrent
        for (   ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type>* pCurrentArray = &aModelListeners;
                pCurrentArray;
                pCurrentArray = (pCurrentArray == &aModelListeners) ? &aControlListeners : NULL
            )
        {
            const ::com::sun::star::uno::Type* pCurrentListeners = pCurrentArray->getConstArray();
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
                ::com::sun::star::uno::Sequence< ::rtl::OUString> aMethodsNames = getEventMethods(*pCurrentListeners);
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
sal_Int16   GridModel2ViewPos(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess>& rColumns, sal_Int16 nModelPos)
{
    if (rColumns.is())
    {
        // invalid pos ?
        if (nModelPos >= rColumns->getCount())
            return (sal_Int16)-1;

        // the column itself shouldn't be hidden
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xAskedFor( *(::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>*)rColumns->getByIndex(nModelPos).getValue());
        if (::utl::getBOOL(xAskedFor->getPropertyValue(FM_PROP_HIDDEN)))
        {
            DBG_ERROR("GridModel2ViewPos : invalid argument !");
            return (sal_Int16)-1;
        }

        sal_Int16 nViewPos = nModelPos;
        for (sal_Int16 i=0; i<nModelPos; ++i)
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xCur( *(::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>*)rColumns->getByIndex(i).getValue());
            if (::utl::getBOOL(xCur->getPropertyValue(FM_PROP_HIDDEN)))
                --nViewPos;
        }
        return nViewPos;
    }
    return (sal_Int16)-1;
}

//------------------------------------------------------------------------------
sal_Int16   GridView2ModelPos(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess>& rColumns, sal_Int16 nViewPos)
{
    if (rColumns.is())
    {
        // loop through all columns
        sal_Int16 i;
        for (i=0; i<rColumns->getCount(); ++i)
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xCur( *(::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>*)rColumns->getByIndex(i).getValue());
            if (!::utl::getBOOL(xCur->getPropertyValue(FM_PROP_HIDDEN)))
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
    return (sal_Int16)-1;
}

//------------------------------------------------------------------------------
sal_Int16   GridViewColumnCount(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess>& rColumns)
{
    if (rColumns.is())
    {
        sal_Int16 nCount = rColumns->getCount();
        // loop through all columns
        for (sal_Int16 i=0; i<rColumns->getCount(); ++i)
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xCur( *(::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>*)rColumns->getByIndex(i).getValue());
            if (::utl::getBOOL(xCur->getPropertyValue(FM_PROP_HIDDEN)))
                --nCount;
        }
        return nCount;
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
UniString quoteTableName(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData>& _rxMeta, const UniString& rName)
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
FmSlotDispatch::FmSlotDispatch(const  ::com::sun::star::util::URL& rUrl, sal_Int16 nSlotId, SfxBindings& rBindings)
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
void FmSlotDispatch::dispatch(const  ::com::sun::star::util::URL& aURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& aArgs) throw( ::com::sun::star::uno::RuntimeException )
{
    DBG_ASSERT(aURL.Main.compareTo(m_aUrl.Main) == COMPARE_EQUAL, "FmSlotDispatch::dispatch : invalid argument !");
    DBG_ASSERT(m_aExecutor.IsSet(), "FmSlotDispatch::dispatch : no executor !");
    // if we have no executor we would have disabled this feature in statusChanged-calls

    m_aExecutor.Call(this);
}

//------------------------------------------------------------------------------
void FmSlotDispatch::NotifyState(SfxItemState eState, const SfxPoolItem* pState, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener>& rListener)
{
    ::com::sun::star::frame::FeatureStateEvent aEvent = BuildEvent(eState, pState);

    if (rListener.is())
        rListener->statusChanged(aEvent);
    else
        NOTIFY_LISTENERS(m_aStatusListeners, ::com::sun::star::frame::XStatusListener, statusChanged, aEvent);
}

//------------------------------------------------------------------------------
void SAL_CALL FmSlotDispatch::addStatusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >& xControl, const ::com::sun::star::util::URL& aURL ) throw(::com::sun::star::uno::RuntimeException)
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
void SAL_CALL FmSlotDispatch::removeStatusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >& xControl, const ::com::sun::star::util::URL& aURL ) throw(::com::sun::star::uno::RuntimeException)
{
    DBG_ASSERT((aURL.Main.getLength() == 0) || (aURL.Main.compareTo(m_aUrl.Main) == COMPARE_EQUAL),
        "FmSlotDispatch::dispatch : invalid argument !");
    m_aStatusListeners.removeInterface( xControl );
}

//------------------------------------------------------------------------------
void SAL_CALL FmSlotDispatch::dispose(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::lang::EventObject aEvt(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >(*this));
    m_aDisposeListeners.disposeAndClear(aEvt);
    m_aStatusListeners.disposeAndClear(aEvt);
}

//------------------------------------------------------------------------------
void SAL_CALL FmSlotDispatch::addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw(::com::sun::star::uno::RuntimeException)
{
    m_aDisposeListeners.addInterface( xListener );
}

//------------------------------------------------------------------------------
void SAL_CALL FmSlotDispatch::removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw(::com::sun::star::uno::RuntimeException)
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
    aReturn.Source = (::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >)(*this);
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
::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> findConnection(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& xParent)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> xConnection(xParent, ::com::sun::star::uno::UNO_QUERY);
    if (!xConnection.is())
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XChild> xChild(xParent, ::com::sun::star::uno::UNO_QUERY);
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
FmXDispatchInterceptorImpl::FmXDispatchInterceptorImpl(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterception>& _rToIntercept, FmDispatchInterceptor* _pMaster, sal_Int16 _nId)
    : ::cppu::OComponentHelper(_pMaster && _pMaster->getInterceptorMutex() ? *_pMaster->getInterceptorMutex() : m_aFallback)
    ,m_xIntercepted(_rToIntercept)
    ,m_pMaster(_pMaster)
    ,m_nId(_nId)
{
    DBG_CTOR(FmXDispatchInterceptorImpl,NULL);

    ::osl::MutexGuard aGuard(getAccessSafety());
    if (m_xIntercepted.is())
    ::utl::increment(m_refCount);
    {
        m_xIntercepted->registerDispatchProviderInterceptor((::com::sun::star::frame::XDispatchProviderInterceptor*)this);
        // this should make us the top-level dispatch-provider for the component, via a call to our
        // setDispatchProvider we should have got an fallback for requests we (i.e. our master) cannot fullfill
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent> xInterceptedComponent(m_xIntercepted, ::com::sun::star::uno::UNO_QUERY);
        if (xInterceptedComponent.is())
            xInterceptedComponent->addEventListener((::com::sun::star::lang::XEventListener*)this);
    }
    ::utl::decrement(m_refCount);
}

//------------------------------------------------------------------------
FmXDispatchInterceptorImpl::~FmXDispatchInterceptorImpl()
{
    if (!rBHelper.bDisposed)
        dispose();

    DBG_DTOR(FmXDispatchInterceptorImpl,NULL);
}

//------------------------------------------------------------------------
::com::sun::star::uno::Any SAL_CALL FmXDispatchInterceptorImpl::queryInterface( const ::com::sun::star::uno::Type& type) throw ( ::com::sun::star::uno::RuntimeException )
{
    ::com::sun::star::uno::Any aOut = ::cppu::queryInterface(type,static_cast< ::com::sun::star::frame::XDispatchProviderInterceptor*>(this),
        static_cast< ::com::sun::star::frame::XDispatchProvider*>(this),
        static_cast< ::com::sun::star::lang::XEventListener*>(this));
    if(aOut.hasValue())
        return aOut;
    return OComponentHelper::queryInterface(type);
}
//------------------------------------------------------------------------------
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL FmXDispatchInterceptorImpl::getTypes(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > aTypes(OComponentHelper::getTypes());
    aTypes.realloc(2);
    ::com::sun::star::uno::Type* pTypes = aTypes.getArray();

    pTypes[aTypes.getLength()-2] = ::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterceptor>*)0);
    pTypes[aTypes.getLength()-1] = ::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener>*)0);
    return aTypes;
}
//------------------------------------------------------------------------------
::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL FmXDispatchInterceptorImpl::getImplementationId() throw(::com::sun::star::uno::RuntimeException)
{
    return ::form::OImplementationIds::getImplementationId(getTypes());
}
//------------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > SAL_CALL FmXDispatchInterceptorImpl::queryDispatch( const ::com::sun::star::util::URL& aURL, const ::rtl::OUString& aTargetFrameName, sal_Int32 nSearchFlags ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(getAccessSafety());
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch> xResult;
    // ask our 'real' interceptor
    if (m_pMaster)
        xResult = m_pMaster->interceptedQueryDispatch(m_nId, aURL, aTargetFrameName, nSearchFlags);

    // ask our slave provider
    if (!xResult.is() && m_xSlaveDispatcher.is())
        xResult = m_xSlaveDispatcher->queryDispatch(aURL, aTargetFrameName, nSearchFlags);

    return xResult;
}

//------------------------------------------------------------------------------
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > > SAL_CALL
FmXDispatchInterceptorImpl::queryDispatches( const ::com::sun::star::uno::Sequence< ::com::sun::star::frame::DispatchDescriptor >& aDescripts ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(getAccessSafety());
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch> > aReturn(aDescripts.getLength());
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch>* pReturn = aReturn.getArray();
    const ::com::sun::star::frame::DispatchDescriptor* pDescripts = aDescripts.getConstArray();
    for (sal_Int16 i=0; i<aDescripts.getLength(); ++i, ++pReturn, ++pDescripts)
    {
        *pReturn = queryDispatch(pDescripts->FeatureURL, pDescripts->FrameName, pDescripts->SearchFlags);
    }
    return aReturn;
}

//------------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > SAL_CALL FmXDispatchInterceptorImpl::getSlaveDispatchProvider(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(getAccessSafety());
    return m_xSlaveDispatcher;
}

//------------------------------------------------------------------------------
void SAL_CALL FmXDispatchInterceptorImpl::setSlaveDispatchProvider(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider>& xNewDispatchProvider) throw( ::com::sun::star::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard(getAccessSafety());
    m_xSlaveDispatcher = xNewDispatchProvider;
}

//------------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider> SAL_CALL FmXDispatchInterceptorImpl::getMasterDispatchProvider(void) throw( ::com::sun::star::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard(getAccessSafety());
    return m_xMasterDispatcher;
}

//------------------------------------------------------------------------------
void SAL_CALL FmXDispatchInterceptorImpl::setMasterDispatchProvider(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider>& xNewSupplier) throw( ::com::sun::star::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard(getAccessSafety());
    m_xMasterDispatcher = xNewSupplier;
}

//------------------------------------------------------------------------------
void SAL_CALL FmXDispatchInterceptorImpl::disposing(const ::com::sun::star::lang::EventObject& Source) throw( ::com::sun::star::uno::RuntimeException )
{
    if (Source.Source == m_xIntercepted)
    {
        ImplDetach();
    }
}

//------------------------------------------------------------------------------
void FmXDispatchInterceptorImpl::ImplDetach()
{
    ::osl::MutexGuard aGuard(getAccessSafety());

/*! PB: das macht der Frame lieber selber
    // remove ourself from the interceptor chain
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterceptor> xSlave(m_xSlaveDispatcher, ::com::sun::star::uno::UNO_QUERY);
    if (xSlave.is())
        xSlave->setMasterDispatchProvider(m_xMasterDispatcher);

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterceptor> xMaster(m_xMasterDispatcher, ::com::sun::star::uno::UNO_QUERY);
    if (xMaster.is())
        xMaster->setSlaveDispatchProvider(m_xSlaveDispatcher);

    m_xSlaveDispatcher = m_xMasterDispatcher = NULL;
*/

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
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent> xInterceptedComponent(m_xIntercepted, ::com::sun::star::uno::UNO_QUERY);
    if (xInterceptedComponent.is())
        xInterceptedComponent->removeEventListener((::com::sun::star::lang::XEventListener*)this);

    // detach from the interception component
    ImplDetach();

}

//==============================================================================
//==============================================================================

//------------------------------------------------------------------------------
sal_Bool isLoadable(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& xLoad)
{
    // determines whether a form should be loaded or not
    // if there is no datasource or connection there is no reason to load a form
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xSet(xLoad, ::com::sun::star::uno::UNO_QUERY);
    if (xSet.is())
    {
        try
        {
            // is there already a active connection
            ::com::sun::star::uno::Any aConn( xSet->getPropertyValue(FM_PROP_ACTIVE_CONNECTION) );
            if (aConn.getValueTypeClass() == ::com::sun::star::uno::TypeClass_INTERFACE &&
                ((::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>*)aConn.getValue())->is())
                return sal_True;
            else if (::utl::getString(xSet->getPropertyValue(FM_PROP_DATASOURCE)).len() ||
                     ::utl::getString(xSet->getPropertyValue(FM_PROP_URL)).len() ||
                     ::findConnection(xLoad).is())
                return sal_True;
        }
        catch(...)
        {
        }

    }
    return sal_False;
}

//------------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess> getTableFields(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _rxConn, const ::rtl::OUString& _rsName)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XTablesSupplier> xSupplyTables(_rxConn, ::com::sun::star::uno::UNO_QUERY);
    DBG_ASSERT(xSupplyTables.is(), "::getTableFields : invalid connection !");
        // the conn already said it would support the service sdb::Connection
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess> xTables( xSupplyTables->getTables());
    if (xTables.is() && xTables->hasByName(_rsName))
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XColumnsSupplier> xTableCols(*(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>*)xTables->getByName(_rsName).getValue(), ::com::sun::star::uno::UNO_QUERY);
        DBG_ASSERT(xTableCols.is(), "::getTableFields : invalid table !");
            // the table is expected to support the service sddb::Table, which requires an ::com::sun::star::sdbcx::XColumnsSupplier interface

        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess> xFieldNames(xTableCols->getColumns(), ::com::sun::star::uno::UNO_QUERY);
        DBG_ASSERT(xFieldNames.is(), "::getTableFields : TableCols->getColumns doesn't export a NameAccess !");
        return xFieldNames;
    }

    return ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>();
}


//------------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDataSource> getDataSource(const ::rtl::OUString& _rsTitleOrPath)
{
    DBG_ASSERT(_rsTitleOrPath.len(), "::getDataSource : invalid arg !");

    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDataSource>  xReturn;

    // is it a file url ?
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess> xNamingContext(::utl::getProcessServiceFactory()->createInstance(SRV_SDB_DATABASE_ACCESS_CONTEXT), ::com::sun::star::uno::UNO_QUERY);
    if (xNamingContext.is() && xNamingContext->hasByName(_rsTitleOrPath))
    {
        DBG_ASSERT(::com::sun::star::uno::Reference< ::com::sun::star::uno::XNamingService>(xNamingContext, ::com::sun::star::uno::UNO_QUERY).is(), "::getDataSource : no NamingService interface on the DatabaseAccessContext !");
        xReturn = ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDataSource>(::com::sun::star::uno::Reference< ::com::sun::star::uno::XNamingService>(xNamingContext, ::com::sun::star::uno::UNO_QUERY)->getRegisteredObject(_rsTitleOrPath), ::com::sun::star::uno::UNO_QUERY);
    }
    else
    {   // is it a favorite title ?
        ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XDatabaseEnvironment> xEnvironment(::utl::getProcessServiceFactory()->createInstance(SRV_SDB_DATABASE_ENVIRONMENT), ::com::sun::star::uno::UNO_QUERY);
        if (xEnvironment.is())
        {
            try
            {
                xReturn = ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDataSource>(xEnvironment->getDatabaseAccess(_rsTitleOrPath), ::com::sun::star::uno::UNO_QUERY);
            }
            catch(::com::sun::star::sdbc::SQLException e)
            {   // allowed, the env may throw an exception in case of an invalid name
                e; // make compiler happy
            }

        }

    }

    return xReturn;
}


//------------------------------------------------------------------------------
void setConnection(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet>& _rxRowSet, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _rxConn)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xRowSetProps(_rxRowSet, ::com::sun::star::uno::UNO_QUERY);
    if (xRowSetProps.is())
    {
        try
        {
            ::com::sun::star::uno::Any aConn(::com::sun::star::uno::makeAny(_rxConn));
            xRowSetProps->setPropertyValue(FM_PROP_ACTIVE_CONNECTION, aConn);
        }
        catch(...)
        {
            DBG_ERROR("::setConnection : could not set the connection !");
        }

    }
}

// retrieve the current command of a rowset
//------------------------------------------------------------------------------
::rtl::OUString getCommand(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet>& _rxRowSet, sal_Bool& bEscapeProcessing, ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& xConn)
{
    ::rtl::OUString aReturn;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xRowSetProps(_rxRowSet, ::com::sun::star::uno::UNO_QUERY);
    if (xRowSetProps.is())
    {
        try
        {
            ::com::sun::star::uno::Any aConn( xRowSetProps->getPropertyValue(FM_PROP_ACTIVE_CONNECTION) );
            if (aConn.getValueTypeClass() != ::com::sun::star::uno::TypeClass_INTERFACE)
                return ::rtl::OUString();

            ::cppu::extractInterface(xConn, aConn);

            // which escapeProcessing we use?
            bEscapeProcessing = ::utl::getBOOL(xRowSetProps->getPropertyValue(FM_PROP_ESCAPE_PROCESSING));

            // get the command
            ::rtl::OUString aCommand(::utl::getString( xRowSetProps->getPropertyValue(FM_PROP_COMMAND) ));

            // and the command type
            sal_Int32 nType = ::utl::getINT32(xRowSetProps->getPropertyValue(FM_PROP_COMMANDTYPE));
            switch (nType)
            {
                case ::com::sun::star::sdb::CommandType::TABLE:
                {
                    //  ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData> xMeta( xConn->getMetaData());
                    UniString aStmt;
                    aStmt.AssignAscii("SELECT * FROM ");
                    aStmt += quoteTableName(xConn->getMetaData(), aCommand);
                    aReturn = aStmt;
                }   break;
                case ::com::sun::star::sdb::CommandType::QUERY:
                {
                    ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XQueriesSupplier> xQueriesAccess(xConn, ::com::sun::star::uno::UNO_QUERY);
                    if (xQueriesAccess.is())
                    {
                        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess> xQueries(xQueriesAccess->getQueries());
                        if (xQueries->hasByName(aCommand))
                        {
                            ::com::sun::star::uno::Any aElement(xQueries->getByName(aCommand));
                            ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xQuery(*(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>*)aElement.getValue(), ::com::sun::star::uno::UNO_QUERY);
                            aReturn= ::utl::getString(xQuery->getPropertyValue(FM_PROP_COMMAND));
                            bEscapeProcessing = ::utl::getBOOL(xQuery->getPropertyValue(FM_PROP_ESCAPE_PROCESSING));
                        }
                    }
                }   break;
                default:
                    aReturn = aCommand;
            }


        }
        catch(...)
        {
        }

    }
    return aReturn;
}

//------------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSQLQueryComposer> getCurrentSettingsComposer(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _rxRowSetProps)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSQLQueryComposer> xReturn;
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet> xRowSet(_rxRowSetProps, ::com::sun::star::uno::UNO_QUERY);
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> xConn( ::utl::calcConnection(xRowSet,::utl::getProcessServiceFactory()));
    try
    {
        if (xConn.is())     // implies xRowSet.is() implies _rxRowSetProps.is()
        {
            // build the statement the row set is based on (can't use the ActiveCommand property of the set
            // as this reflects the status after the last execute, not the currently set properties

            ::rtl::OUString sStatement;
            // first ensure we have all properties needed
            if (::utl::hasProperty(FM_PROP_COMMAND, _rxRowSetProps) && ::utl::hasProperty(FM_PROP_COMMANDTYPE, _rxRowSetProps)
                && ::utl::hasProperty(FM_PROP_FILTER_CRITERIA, _rxRowSetProps) && ::utl::hasProperty(FM_PROP_SORT, _rxRowSetProps)
                && ::utl::hasProperty(FM_PROP_ESCAPE_PROCESSING, _rxRowSetProps) && ::utl::hasProperty(FM_PROP_APPLYFILTER, _rxRowSetProps))
            {
                sal_Int32 nCommandType = ::utl::getINT32(_rxRowSetProps->getPropertyValue(FM_PROP_COMMANDTYPE));
                ::rtl::OUString sCommand = ::utl::getString(_rxRowSetProps->getPropertyValue(FM_PROP_COMMAND));
                sal_Bool bEscapeProcessing = ::utl::getBOOL(_rxRowSetProps->getPropertyValue(FM_PROP_ESCAPE_PROCESSING));
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
                        ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XQueriesSupplier> xSupplyQueries(xConn, ::com::sun::star::uno::UNO_QUERY);
                        if (!xSupplyQueries.is())
                            break;

                        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess> xQueries(xSupplyQueries->getQueries(), ::com::sun::star::uno::UNO_QUERY);
                        if (!xQueries.is() || !xQueries->hasByName(sCommand))
                            break;

                        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xQueryProps(*(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>*)xQueries->getByName(sCommand).getValue(), ::com::sun::star::uno::UNO_QUERY);
                        if (!xQueryProps.is())
                            break;

                        //  a native query ?
                        if (!::utl::hasProperty(FM_PROP_ESCAPE_PROCESSING, xQueryProps))
                            break;
                        if (!::utl::getBOOL(xQueryProps->getPropertyValue(FM_PROP_ESCAPE_PROCESSING)))
                            break;

                        if (!::utl::hasProperty(FM_PROP_COMMAND, xQueryProps))
                            break;

                        // the command used by the query
                        sStatement = ::utl::getString(xQueryProps->getPropertyValue(FM_PROP_COMMAND));

                        // use an additional composer to build a statement from the query filter/order props
                        ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSQLQueryComposerFactory> xFactory(xConn, ::com::sun::star::uno::UNO_QUERY);
                        ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSQLQueryComposer> xLocalComposer;
                        if (xFactory.is())
                            xLocalComposer = xFactory->createQueryComposer();
                        if (!xLocalComposer.is())
                            break;

                        xLocalComposer->setQuery(sStatement);
                        // the sort order
                        if (::utl::hasProperty(FM_PROP_SORT, xQueryProps))
                            xLocalComposer->setOrder(::utl::getString(xQueryProps->getPropertyValue(FM_PROP_SORT)));

                        sal_Bool bApplyFilter = sal_False;
                        if (::utl::hasProperty(FM_PROP_APPLYFILTER, xQueryProps))
                            bApplyFilter = ::utl::getBOOL(xQueryProps->getPropertyValue(FM_PROP_APPLYFILTER));

                        if (bApplyFilter)
                        {
                            if (::utl::hasProperty(FM_PROP_FILTER_CRITERIA, xQueryProps))
                                xLocalComposer->setFilter(::utl::getString(xQueryProps->getPropertyValue(FM_PROP_FILTER_CRITERIA)));
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
                ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSQLQueryComposerFactory> xFactory(xConn, ::com::sun::star::uno::UNO_QUERY);
                if (xFactory.is())
                    xReturn = xFactory->createQueryComposer();
                if (xReturn.is())
                {
                    xReturn->setQuery(sStatement);
                    // append filter/sort
                    xReturn->setOrder(::utl::getString(_rxRowSetProps->getPropertyValue(FM_PROP_SORT)));
                    sal_Bool bApplyFilter = ::utl::getBOOL(_rxRowSetProps->getPropertyValue(FM_PROP_APPLYFILTER));
                    if (bApplyFilter)
                        xReturn->setFilter(::utl::getString(_rxRowSetProps->getPropertyValue(FM_PROP_FILTER_CRITERIA)));
                }
            }
        }
    }
    catch(...)
    {
        DBG_ERROR("::getCurrentSettingsComposer : catched an exception !");
        xReturn = NULL;
    }


    return xReturn;
}

//------------------------------------------------------------------------------
sal_Bool isRowSetAlive(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& _rxRowSet)
{
    sal_Bool bIsAlive = sal_False;
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XColumnsSupplier> xSupplyCols(_rxRowSet, ::com::sun::star::uno::UNO_QUERY);
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess> xCols;
    if (xSupplyCols.is())
        xCols = ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess>(xSupplyCols->getColumns(), ::com::sun::star::uno::UNO_QUERY);
    if (xCols.is() && (xCols->getCount() > 0))
        bIsAlive = sal_True;

    return bIsAlive;
}


//==============================================================================
DataColumn::DataColumn(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _rxIFace)
{
    m_xPropertySet = _rxIFace;
    m_xColumn = ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn>(_rxIFace, ::com::sun::star::uno::UNO_QUERY);
    m_xColumnUpdate = ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumnUpdate>(_rxIFace, ::com::sun::star::uno::UNO_QUERY);

    if (!m_xPropertySet.is() || !m_xColumn.is() || !m_xColumnUpdate.is())
    {
        m_xPropertySet = NULL;
        m_xColumn = NULL;
        m_xColumnUpdate = NULL;
    }
}



