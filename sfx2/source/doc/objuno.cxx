/*************************************************************************
 *
 *  $RCSfile: objuno.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:52:32 $
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

#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif
#ifndef _COM_SUN_STAR_IO_IOEXCEPTION_HPP_
#include <com/sun/star/io/IOException.hpp>
#endif

#include <tools/errcode.hxx>
#include <so3/svstor.hxx>
#include <svtools/cntwids.hrc>
#include <svtools/itemset.hxx>
#include <svtools/stritem.hxx>
#include <svtools/intitem.hxx>
#include <svtools/eitem.hxx>
#include <svtools/adrparse.hxx>

#include "objuno.hxx"
#include "sfx.hrc"
#include "sfxsids.hrc"
#include "viewsh.hxx"
#include "viewfrm.hxx"
#include "printer.hxx"
#include "objsh.hxx"
#include "docinf.hxx"
#include "docfile.hxx"
#include "dispatch.hxx"
#include "openflag.hxx"
#include "app.hxx"
#include "fcontnr.hxx"
#include "request.hxx"
#include "sfxuno.hxx"
#include <objshimp.hxx>

#include <osl/mutex.hxx>
#ifdef _USE_NAMESPACE
using namespace vos;
#endif

#define SfxIOException_Impl( nErr ) ::com::sun::star::io::IOException()

//=============================================================================

#define PROPERTY_UNBOUND 0
SfxItemPropertyMap aDocInfoPropertyMap_Impl[] =
{
    { "Author"          , 6 , WID_FROM,           &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
    { "AutoloadEnabled" , 15, MID_DOCINFO_AUTOLOADENABLED, &::getBooleanCppuType(),   PROPERTY_UNBOUND, 0 },
    { "AutoloadSecs"    , 12, MID_DOCINFO_AUTOLOADSECS, &::getCppuType((const sal_Int16*)0),     PROPERTY_UNBOUND, 0 },
    { "AutoloadURL"     , 11, MID_DOCINFO_AUTOLOADURL, &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
    { "BlindCopiesTo"   , 13, WID_BCC,            &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
    { "CopyTo"          , 6 , WID_CC,             &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
    { "CreationDate"    , 12, WID_DATE_CREATED,   &::getCppuType((const ::com::sun::star::util::DateTime*)0),PROPERTY_UNBOUND, 0 },
    { "DefaultTarget"   , 13, MID_DOCINFO_DEFAULTTARGET, &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
    { "Description"     , 11, MID_DOCINFO_DESCRIPTION, &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
    { "EditingCycles"   , 13, MID_DOCINFO_REVISION, &::getCppuType((const sal_Int16*)0),   PROPERTY_UNBOUND, 0 },
    { "EditingDuration" , 15, MID_DOCINFO_EDITTIME, &::getCppuType((const sal_Int32*)0),   PROPERTY_UNBOUND, 0 },
    { "InReplyTo"       , 9 , WID_IN_REPLY_TO,    &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
    { "IsEncrypted"     , 11, MID_DOCINFO_ENCRYPTED, &::getBooleanCppuType(),     PROPERTY_UNBOUND | ::com::sun::star::beans::PropertyAttribute::READONLY, 0 },
    { "Keywords"        , 8 , WID_KEYWORDS,       &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
    { "MIMEType"        , 8 , WID_CONTENT_TYPE,   &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND | ::com::sun::star::beans::PropertyAttribute::READONLY, 0 },
    { "ModifiedBy"      , 10, MID_DOCINFO_MODIFICATIONAUTHOR, &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
    { "ModifyDate"      , 10, WID_DATE_MODIFIED,  &::getCppuType((const ::com::sun::star::util::DateTime*)0),PROPERTY_UNBOUND, 0 },
    { "Newsgroups"      , 10, WID_NEWSGROUPS,     &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
    { "Original"        , 8 , WID_MESSAGE_ID,     &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
    { "PrintDate"       , 9 , MID_DOCINFO_PRINTDATE, &::getCppuType((const ::com::sun::star::util::DateTime*)0),PROPERTY_UNBOUND, 0 },
    { "PrintedBy"       , 9 , MID_DOCINFO_PRINTEDBY, &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
    { "Priority"        , 8 , WID_PRIORITY,       &::getCppuType((const sal_Int16*)0),   PROPERTY_UNBOUND, 0 },
    { "Recipient"       , 9 , WID_TO,             &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
    { "References"      , 10, WID_REFERENCES,     &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
    { "ReplyTo"         , 7 , WID_REPLY_TO,       &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
    { "Template"        , 8 , MID_DOCINFO_TEMPLATE, &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
    { "TemplateFileName", 16, SID_TEMPLATE_NAME, &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
    { "TemplateDate"    , 12, MID_DOCINFO_TEMPLATEDATE, &::getCppuType((const ::com::sun::star::util::DateTime*)0),PROPERTY_UNBOUND, 0 },
    { "Theme"           , 5 , MID_DOCINFO_SUBJECT, &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
    { "Title"           , 5 , WID_TITLE,          &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
    {0,0,0,0,0}
};

//-----------------------------------------------------------------------------
SFX_IMPL_XINTERFACE_5( SfxDocumentInfoObject, OWeakObject, ::com::sun::star::document::XDocumentInfo, ::com::sun::star::lang::XComponent, ::com::sun::star::beans::XPropertySet, ::com::sun::star::beans::XFastPropertySet, ::com::sun::star::beans::XPropertyAccess )
SFX_IMPL_XTYPEPROVIDER_5( SfxDocumentInfoObject, ::com::sun::star::document::XDocumentInfo, ::com::sun::star::lang::XComponent, ::com::sun::star::beans::XPropertySet, ::com::sun::star::beans::XFastPropertySet, ::com::sun::star::beans::XPropertyAccess )
/*
::com::sun::star::uno::Reference< ::com::sun::star::reflection::XIdlClass >  SfxDocumentInfoObject::getStaticIdlClass()
{
    // Global Method, must be guarded (multithreading)
    MutexGuard aGuard( Mutex::getGlobalMutex() );
    {
        // use the standard class implementation of the usr library
        static ::com::sun::star::uno::Reference< ::com::sun::star::reflection::XIdlClass >  xClass =
        createStandardClass(
            "com.sun.star.comp.sfx2.DocumentInfo",
            OWeakObject::getStaticIdlClass(),
            4,
            ::getCppuType((const ::com::sun::star::lang::XComponent*)0),
            ::getCppuType((const ::com::sun::star::document::XDocumentInfo*)0),
            ::getCppuType((const ::com::sun::star::beans::XPropertyAccess*)0),
            ::getCppuType((const ::com::sun::star::beans::XPropertySet*)0),
            ::getCppuType((const ::com::sun::star::beans::XFastPropertySet*)0) );

        return xClass;
    }
}
*/
//-----------------------------------------------------------------------------
struct SfxDocumentInfoObject_Impl
{
    SfxObjectShell*                     _pObjSh;
    ::osl::Mutex                        _aMutex;
    ::cppu::OInterfaceContainerHelper   _aDisposeContainer;

    SfxDocumentInfoObject_Impl( SfxObjectShell* pObjSh )
        : _pObjSh( pObjSh )
        , _aDisposeContainer( _aMutex )
    {
    }
};

//-----------------------------------------------------------------------------

SfxDocumentInfoObject::SfxDocumentInfoObject( sal_Bool bStandalone )
    : _aPropSet( aDocInfoPropertyMap_Impl )
    , _pInfo( NULL )
    , _pFilter( NULL )
    , _bStandalone( bStandalone )
    , _pImp( new SfxDocumentInfoObject_Impl( NULL ) )
{
}

//-----------------------------------------------------------------------------

SfxDocumentInfoObject::SfxDocumentInfoObject( SfxObjectShell *pObjSh )
    : _aPropSet( aDocInfoPropertyMap_Impl )
    , _pInfo( 0 )
    , _pFilter( NULL )
    , _bStandalone( sal_False )
    , _pImp( new SfxDocumentInfoObject_Impl( pObjSh ) )
{
    if ( pObjSh )
    {
        SfxDocumentInfo &rDocInfo = pObjSh->GetDocInfo();
        _pInfo = &rDocInfo;
        _pFilter = pObjSh->GetMedium()->GetFilter();
        _xObjSh = pObjSh;
    }
}

//-----------------------------------------------------------------------------

SfxDocumentInfoObject::~SfxDocumentInfoObject()
{
    if ( _bStandalone )
        delete _pInfo;
    delete _pImp;
}

//-----------------------------------------------------------------------------

void SAL_CALL SfxDocumentInfoObject::dispose() throw( ::com::sun::star::uno::RuntimeException )
{
    ::com::sun::star::lang::EventObject aEvent( (::cppu::OWeakObject *)this );
    _pImp->_aDisposeContainer.disposeAndClear( aEvent );

/*
    _xCurrent = ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController > ();
    for ( sal_uInt32 n = _aControllers.ge.getLength(); n; --n )
        SequenceRemoveElementAt( _aControllers, n-1 );
*/
}

//-----------------------------------------------------------------------------

void SAL_CALL  SfxDocumentInfoObject::addEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw( ::com::sun::star::uno::RuntimeException )
{
    _pImp->_aDisposeContainer.addInterface( aListener );
}

//-----------------------------------------------------------------------------

void SAL_CALL  SfxDocumentInfoObject::removeEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw( ::com::sun::star::uno::RuntimeException )
{
    _pImp->_aDisposeContainer.removeInterface( aListener );
}
//-----------------------------------------------------------------------------

::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >  SAL_CALL  SfxDocumentInfoObject::getPropertySetInfo()  throw( ::com::sun::star::uno::RuntimeException )
{
    return _aPropSet.getPropertySetInfo();
}

//-----------------------------------------------------------------------------

void SAL_CALL  SfxDocumentInfoObject::setPropertyValue(const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue) throw( ::com::sun::star::uno::RuntimeException )
{
    const SfxItemPropertyMap* pMap = SfxItemPropertyMap::GetByName(
            aDocInfoPropertyMap_Impl,
            aPropertyName );
    if ( pMap )
        setFastPropertyValue( pMap->nWID, aValue );
}

//-----------------------------------------------------------------------------

::com::sun::star::uno::Any  SAL_CALL  SfxDocumentInfoObject::getPropertyValue(const ::rtl::OUString& aPropertyName)  throw( ::com::sun::star::uno::RuntimeException )
{
    const SfxItemPropertyMap* pMap = SfxItemPropertyMap::GetByName( aDocInfoPropertyMap_Impl,
        aPropertyName );
    if ( pMap )
        return getFastPropertyValue( pMap->nWID );
    else
        return ::com::sun::star::uno::Any();
}

//-----------------------------------------------------------------------------

void SAL_CALL  SfxDocumentInfoObject::addPropertyChangeListener(const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener > & aListener) throw( ::com::sun::star::uno::RuntimeException )
{}

//-----------------------------------------------------------------------------

void SAL_CALL  SfxDocumentInfoObject::removePropertyChangeListener(const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener > & aListener) throw( ::com::sun::star::uno::RuntimeException )
{}

//-----------------------------------------------------------------------------

void SAL_CALL  SfxDocumentInfoObject::addVetoableChangeListener(const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener > & aListener) throw( ::com::sun::star::uno::RuntimeException )
{}

//-----------------------------------------------------------------------------

void SAL_CALL  SfxDocumentInfoObject::removeVetoableChangeListener(const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener > & aListener) throw( ::com::sun::star::uno::RuntimeException )
{}

::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL  SfxDocumentInfoObject::getPropertyValues( void ) throw( ::com::sun::star::uno::RuntimeException )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >  xInfo = getPropertySetInfo();
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property > aProps = xInfo->getProperties();

    const ::com::sun::star::beans::Property* pProps  = aProps.getConstArray();
    sal_uInt32 nCount = aProps.getLength();

    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >aSeq( nCount );
    ::com::sun::star::beans::PropertyValue*  pValues = aSeq.getArray();

    for ( sal_uInt32 n = 0; n < nCount; ++n )
    {
        ::com::sun::star::beans::PropertyValue& rCurrValue = pValues[n];
        const ::com::sun::star::beans::Property& rCurrProp = pProps[n];

        rCurrValue.Name = rCurrProp.Name;
        rCurrValue.Handle = rCurrProp.Handle;
        rCurrValue.Value = getPropertyValue( rCurrProp.Name );
    }

    return aSeq;
}

void SAL_CALL  SfxDocumentInfoObject::setPropertyValues( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aProps )
        throw( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException )
{
    const ::com::sun::star::beans::PropertyValue* pProps  = aProps.getConstArray();
    sal_uInt32 nCount = aProps.getLength();

    for ( sal_uInt32 n = 0; n < nCount; ++n )
    {
        const ::com::sun::star::beans::PropertyValue& rProp = pProps[n];
        setPropertyValue( rProp.Name, rProp.Value );
    }
}

::com::sun::star::util::DateTime SfxDocumentInfoObject::impl_DateTime_Object2Struct( const ::DateTime& aDateTimeObject )
{
    //  Attention!
    //      We can't use a DateTime-object as type of any property with new UNO!
    //      We must use a DateTime-struct and convert the values.

    ::com::sun::star::util::DateTime    aDateTimeStruct ;   // com/sun/star/util/DateTime   !!!

    // ... and convert DateTime-object to struct ! (copy values)
    aDateTimeStruct.HundredthSeconds    = aDateTimeObject.Get100Sec () ;
    aDateTimeStruct.Seconds             = aDateTimeObject.GetSec    () ;
    aDateTimeStruct.Minutes             = aDateTimeObject.GetMin    () ;
    aDateTimeStruct.Hours               = aDateTimeObject.GetHour   () ;
    aDateTimeStruct.Day                 = aDateTimeObject.GetDay    () ;
    aDateTimeStruct.Month               = aDateTimeObject.GetMonth  () ;
    aDateTimeStruct.Year                = aDateTimeObject.GetYear   () ;

    // Return ricght type with right values.
    return aDateTimeStruct ;
}

::DateTime SfxDocumentInfoObject::impl_DateTime_Struct2Object ( const ::com::sun::star::util::DateTime& aDateTimeStruct )
{
    //  Attention!
    //      We can't use a DateTime-object as type of any property with new UNO!
    //      We must use a DateTime-struct and convert the values.

    // Get values from DateTime-struct and create a date and time instance ...
    ::Date aDate ( aDateTimeStruct.Day  , aDateTimeStruct.Month     , aDateTimeStruct.Year                                      ) ;
    ::Time aTime ( aDateTimeStruct.Hours, aDateTimeStruct.Minutes   , aDateTimeStruct.Seconds, aDateTimeStruct.HundredthSeconds ) ;

    // ... to use it for creation of a DateTime-object ...
    ::DateTime  aDateTimeObject ( aDate, aTime      ) ;

    // Return converted values.
    return aDateTimeObject ;
}


void SAL_CALL  SfxDocumentInfoObject::setFastPropertyValue(sal_Int32 nHandle, const ::com::sun::star::uno::Any& aValue) throw( ::com::sun::star::uno::RuntimeException )
{
    sal_Bool bModified = sal_True;

    if ( _pInfo && aValue.getValueType() == ::getCppuType((const ::rtl::OUString*)0) )
    {
        ::rtl::OUString sTemp ;
        aValue >>= sTemp ;
        String aStrVal( sTemp );
        switch ( nHandle )
        {
            case WID_FROM :
            {
                const SfxStamp& rStamp = _pInfo->GetCreated();
                if ( aStrVal.Len() > TIMESTAMP_MAXLENGTH )
                {
                    SvAddressParser aParser( aStrVal );
                    if ( aParser.Count() > 0 )
                    {
                        String aEmail = aParser.GetEmailAddress(0);
                        String aRealname = aParser.GetRealName(0);

                        if ( aRealname.Len() <= TIMESTAMP_MAXLENGTH )
                            aStrVal = aRealname;
                        else if ( aEmail.Len() <= TIMESTAMP_MAXLENGTH )
                            aStrVal = aEmail;
                    }
                }
                _pInfo->SetCreated( SfxStamp( aStrVal, rStamp.GetTime() ) );
                break;
            }
            case MID_DOCINFO_PRINTEDBY:
            {
                const SfxStamp& rStamp = _pInfo->GetPrinted();
                _pInfo->SetPrinted( SfxStamp( aStrVal, rStamp.GetTime() ) );
            }
            case MID_DOCINFO_MODIFICATIONAUTHOR:
            {
                const SfxStamp& rStamp = _pInfo->GetChanged();
                _pInfo->SetChanged( SfxStamp( aStrVal, rStamp.GetTime() ) );
                break;
            }
            case WID_TITLE :
                _pInfo->SetTitle( aStrVal );
                if ( _xObjSh )
                {
                    _xObjSh->InvalidateName();
                    _xObjSh->Broadcast( SfxSimpleHint( SFX_HINT_TITLECHANGED ) );
                }
                break;
            case MID_DOCINFO_SUBJECT :
                _pInfo->SetTheme( aStrVal );
                break;
            case WID_KEYWORDS :
                _pInfo->SetKeywords( aStrVal );
                break;
            case WID_CC :
                _pInfo->SetCopiesTo( aStrVal );
                break;
            case WID_MESSAGE_ID :
                _pInfo->SetOriginal( aStrVal );
                break;
            case WID_REFERENCES :
                _pInfo->SetReferences( aStrVal );
                break;
            case WID_TO :
                _pInfo->SetRecipient( aStrVal );
                break;
            case WID_REPLY_TO :
                _pInfo->SetReplyTo( aStrVal );
                break;
            case MID_DOCINFO_TEMPLATE:
                _pInfo->SetTemplateName( aStrVal );
                break;
            case SID_TEMPLATE_NAME:
                _pInfo->SetTemplateFileName( aStrVal );
                break;
            case WID_BCC :
                _pInfo->SetBlindCopies( aStrVal );
                break;
            case WID_IN_REPLY_TO :
                _pInfo->SetInReplyTo( aStrVal );
                break;
            case WID_NEWSGROUPS :
                _pInfo->SetNewsgroups( aStrVal );
                break;
            case MID_DOCINFO_DESCRIPTION:
                _pInfo->SetComment( aStrVal );
                break;
            case MID_DOCINFO_AUTOLOADURL:
                _pInfo->SetReloadURL( aStrVal );
                break;
            case MID_DOCINFO_DEFAULTTARGET:
                _pInfo->SetDefaultTarget( aStrVal );
                break;
            default:
                bModified = sal_False;
                break;
        }
    }
    else if ( _pInfo && aValue.getValueType() == ::getCppuType((const ::com::sun::star::util::DateTime*)0) )
    {
        switch ( nHandle )
        {
            case WID_DATE_CREATED :
            {
                const SfxStamp& rStamp = _pInfo->GetCreated();
                ::com::sun::star::util::DateTime aDateTime ;
                aValue >>= aDateTime ;
                _pInfo->SetCreated( SfxStamp( rStamp.GetName(), impl_DateTime_Struct2Object(aDateTime) ) );
                break;
            }
            case WID_DATE_MODIFIED :
            {
                const SfxStamp& rStamp = _pInfo->GetChanged();
                ::com::sun::star::util::DateTime aDateTime ;
                aValue >>= aDateTime ;
                _pInfo->SetChanged( SfxStamp( rStamp.GetName(), impl_DateTime_Struct2Object(aDateTime) ) );
                break;
            }
            case MID_DOCINFO_PRINTDATE :
            {
                const SfxStamp& rStamp = _pInfo->GetPrinted();
                ::com::sun::star::util::DateTime aDateTime ;
                aValue >>= aDateTime ;
                _pInfo->SetPrinted( SfxStamp( rStamp.GetName(), impl_DateTime_Struct2Object(aDateTime) ) );
                break;
            }
            default:
                bModified = sal_False;
                break;
        }
    }
    else if ( _pInfo && aValue.getValueType() == ::getBooleanCppuType() )
    {
        sal_Bool bBoolVal ;
        aValue >>= bBoolVal ;
        switch ( nHandle )
        {
            case MID_DOCINFO_AUTOLOADENABLED:
                _pInfo->EnableReload( bBoolVal );
                break;
            default:
                bModified = sal_False;
        }
    }
    else if ( _pInfo && aValue.getValueType() == ::getCppuType((const sal_Int32*)0) )
    {
        long nIntVal ;
        aValue >>= nIntVal ;
        switch ( nHandle )
        {
            case MID_DOCINFO_AUTOLOADSECS:
                _pInfo->SetReloadDelay( nIntVal );
                break;
            case MID_DOCINFO_EDITTIME:
                _pInfo->SetTime( nIntVal );
            default:
                bModified = sal_False;
        }
    }
    else if ( _pInfo && aValue.getValueType() == ::getCppuType((const sal_Int16*)0) )
    {
        short nIntVal ;
        aValue >>= nIntVal ;
        switch ( nHandle )
        {
            case MID_DOCINFO_REVISION:
                _pInfo->SetDocumentNumber( nIntVal );
            case WID_PRIORITY :
                _pInfo->SetPriority( nIntVal );
                break;
            default:
                bModified = sal_False;
                break;
        }
    }

    if ( bModified && _pImp->_pObjSh )
        _pImp->_pObjSh->SetModified( sal_True );
}

//-----------------------------------------------------------------------------

::com::sun::star::uno::Any SAL_CALL  SfxDocumentInfoObject::getFastPropertyValue(sal_Int32 nHandle) throw( ::com::sun::star::uno::RuntimeException )
{
    ::com::sun::star::uno::Any aValue;
    if ( nHandle == WID_CONTENT_TYPE )
    {
        if ( _pInfo && _pInfo->GetSpecialMimeType().Len() )
        {
            ::rtl::OUString sTemp ( _pInfo->GetSpecialMimeType() );
            aValue <<= sTemp ;
        }
        else if ( _pFilter )
        {
            ::rtl::OUString sTemp ( _pFilter->GetMimeType() );
            aValue <<= sTemp ;
        }
        else
            aValue <<= ::rtl::OUString() ;
    }
    else if ( _pInfo )
    {
        switch ( nHandle )
        {
            case MID_DOCINFO_REVISION :
                aValue <<= _pInfo->GetDocumentNumber() ;
                break;
            case MID_DOCINFO_EDITTIME :
                aValue <<= _pInfo->GetTime();
                break;
            case WID_FROM :
            {
                const SfxStamp& rStamp = _pInfo->GetCreated();
                if ( rStamp.IsValid() )
                    aValue <<= ::rtl::OUString( rStamp.GetName() );
                else
                    aValue <<= ::rtl::OUString() ;
                break;
            }
            case WID_DATE_CREATED :
            {
                const SfxStamp& rStamp = _pInfo->GetCreated();
                if ( rStamp.IsValid() )
                    aValue <<= impl_DateTime_Object2Struct(rStamp.GetTime());
                break;
            }
            case WID_TITLE :
                aValue <<= ::rtl::OUString( _pInfo->GetTitle() );
                break;
            case MID_DOCINFO_SUBJECT:
                aValue <<= ::rtl::OUString( _pInfo->GetTheme() );
                break;
            case MID_DOCINFO_MODIFICATIONAUTHOR:
            {
                const SfxStamp& rStamp = _pInfo->GetChanged();
                if ( rStamp.IsValid() )
                    aValue <<= ::rtl::OUString( rStamp.GetName() );
                else
                    aValue <<= ::rtl::OUString() ;
                break;
            }
            case WID_DATE_MODIFIED :
            {
                const SfxStamp& rStamp = _pInfo->GetChanged();
                if ( rStamp.IsValid() )
                    aValue <<= impl_DateTime_Object2Struct(rStamp.GetTime());
                break;
            }
            case MID_DOCINFO_PRINTEDBY:
            {
                const SfxStamp& rStamp = _pInfo->GetPrinted();
                if ( rStamp.IsValid() )
                    aValue <<= ::rtl::OUString( rStamp.GetName() );
                else
                    aValue <<= ::rtl::OUString() ;
                break;
            }
            case MID_DOCINFO_PRINTDATE:
            {
                const SfxStamp& rStamp = _pInfo->GetPrinted();
                if ( rStamp.IsValid() )
                    aValue <<= impl_DateTime_Object2Struct(rStamp.GetTime());
                break;
            }
            case WID_KEYWORDS :
                aValue <<= rtl::OUString( _pInfo->GetKeywords() );
                break;
            case WID_PRIORITY :
                aValue <<= _pInfo->GetPriority() ;
                break;
            case WID_CC :
                aValue <<= rtl::OUString( _pInfo->GetCopiesTo() );
                break;
            case WID_MESSAGE_ID :
                aValue <<=  rtl::OUString( _pInfo->GetOriginal() );
                break;
            case WID_REFERENCES :
                aValue <<=  rtl::OUString( _pInfo->GetReferences() );
                break;
            case WID_TO :
                aValue <<=  rtl::OUString( _pInfo->GetRecipient() );
                break;
            case WID_REPLY_TO :
                aValue <<=  rtl::OUString( _pInfo->GetReplyTo() );
                break;
            case WID_BCC :
                aValue <<=  rtl::OUString( _pInfo->GetBlindCopies() );
                break;
            case WID_IN_REPLY_TO :
                aValue <<=  rtl::OUString( _pInfo->GetInReplyTo() );
                break;
            case WID_NEWSGROUPS :
                aValue <<=  rtl::OUString( _pInfo->GetNewsgroups() );
                break;
            case MID_DOCINFO_DESCRIPTION:
                aValue <<=  rtl::OUString( _pInfo->GetComment() );
                break;
            case MID_DOCINFO_TEMPLATE:
                aValue <<=  rtl::OUString( _pInfo->GetTemplateName() );
                break;
            case SID_TEMPLATE_NAME:
                aValue <<=  rtl::OUString( _pInfo->GetTemplateFileName() );
                break;
            case MID_DOCINFO_TEMPLATEDATE:
                aValue <<= impl_DateTime_Object2Struct(_pInfo->GetTemplateDate());
                break;
            case MID_DOCINFO_AUTOLOADENABLED:
                aValue <<=  _pInfo->IsReloadEnabled() ;
                break;
            case MID_DOCINFO_AUTOLOADURL:
                aValue <<=  rtl::OUString( _pInfo->GetReloadURL() );
                break;
            case MID_DOCINFO_AUTOLOADSECS:
                aValue <<=  _pInfo->GetReloadDelay() ;
                break;
            case MID_DOCINFO_DEFAULTTARGET:
                aValue <<=  rtl::OUString( _pInfo->GetDefaultTarget() );
                break;
            default:
                aValue <<= ::rtl::OUString();
                break;
        }
    }

    return aValue;
}

//-----------------------------------------------------------------------------

sal_Int16 SAL_CALL  SfxDocumentInfoObject::getUserFieldCount() throw( ::com::sun::star::uno::RuntimeException )
{
    return _pInfo->GetUserKeyCount();
}

//-----------------------------------------------------------------------------

::rtl::OUString SAL_CALL  SfxDocumentInfoObject::getUserFieldName(sal_Int16 nIndex) throw( ::com::sun::star::uno::RuntimeException )
{
    if ( _pInfo && nIndex < _pInfo->GetUserKeyCount() )
        return _pInfo->GetUserKey( nIndex ).GetTitle();
    else
        return ::rtl::OUString();
}

//-----------------------------------------------------------------------------

::rtl::OUString SAL_CALL  SfxDocumentInfoObject::getUserFieldValue(sal_Int16 nIndex) throw( ::com::sun::star::uno::RuntimeException )
{
    if ( _pInfo && nIndex < _pInfo->GetUserKeyCount() )
        return _pInfo->GetUserKey( nIndex ).GetWord();
    else
        return ::rtl::OUString();
}

//-----------------------------------------------------------------------------

void  SAL_CALL SfxDocumentInfoObject::setUserFieldName(sal_Int16 nIndex, const ::rtl::OUString& aName ) throw( ::com::sun::star::uno::RuntimeException )
{
    if ( _pInfo && nIndex < _pInfo->GetUserKeyCount() )
    {
        const SfxDocUserKey& rKey = _pInfo->GetUserKey( nIndex );
        _pInfo->SetUserKey( SfxDocUserKey( aName, rKey.GetWord() ), nIndex );
        if ( _pImp->_pObjSh )
            _pImp->_pObjSh->SetModified( sal_True );
    }
}

//-----------------------------------------------------------------------------

void SAL_CALL  SfxDocumentInfoObject::setUserFieldValue( sal_Int16 nIndex, const ::rtl::OUString& aValue ) throw( ::com::sun::star::uno::RuntimeException )
{
    if ( _pInfo && nIndex < _pInfo->GetUserKeyCount() )
    {
        const SfxDocUserKey& rKey = _pInfo->GetUserKey( nIndex );
        _pInfo->SetUserKey( SfxDocUserKey( rKey.GetTitle(), aValue ), nIndex );
        if ( _pImp->_pObjSh )
            _pImp->_pObjSh->SetModified( sal_True );
    }
}

//-----------------------------------------------------------------------------
SFX_IMPL_XINTERFACE_1( SfxStandaloneDocumentInfoObject, SfxDocumentInfoObject, ::com::sun::star::document::XStandaloneDocumentInfo  )
SFX_IMPL_XTYPEPROVIDER_6( SfxStandaloneDocumentInfoObject, ::com::sun::star::document::XDocumentInfo, ::com::sun::star::lang::XComponent, ::com::sun::star::beans::XPropertySet, ::com::sun::star::beans::XFastPropertySet, ::com::sun::star::beans::XPropertyAccess, ::com::sun::star::document::XStandaloneDocumentInfo )
SFX_IMPL_XSERVICEINFO( SfxStandaloneDocumentInfoObject, "com.sun.star.document.StandaloneDocumentInfo", "com.sun.star.comp.sfx2.StandaloneDocumentInfo" )
SFX_IMPL_SINGLEFACTORY( SfxStandaloneDocumentInfoObject )

SfxStandaloneDocumentInfoObject::SfxStandaloneDocumentInfoObject( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xFactory )
    : SfxDocumentInfoObject( sal_True )
    , _pMedium( NULL )
{
}

//-----------------------------------------------------------------------------

SfxStandaloneDocumentInfoObject::~SfxStandaloneDocumentInfoObject()
{
    delete _pMedium;
}

//-----------------------------------------------------------------------------

SvStorage* SfxStandaloneDocumentInfoObject::GetStorage_Impl( const String& rName, sal_Bool bWrite )
{
    // Medium erstellen
    if ( _pMedium )
        delete _pMedium;

    _pMedium = new SfxMedium( rName, bWrite ? SFX_STREAM_READWRITE : SFX_STREAM_READONLY, sal_True );
    if ( !_pMedium->IsStorage() || SVSTREAM_OK != _pMedium->GetError() )
        // Datei existiert nicht oder ist kein Storage
        return NULL;

    // Filter-Detection wegen FileFormat-Version
    _pFilter = 0;
    if ( 0 != SFX_APP()->GetFilterMatcher().GuessFilter( *_pMedium, &_pFilter )
            || !bWrite && !_pFilter )
        // unbekanntes Dateiformat
        return NULL;

    // Storage "offnen
    SvStorageRef xStor = _pMedium->GetStorage();
    DBG_ASSERT( xStor.Is(), "no storage" );
    xStor->SetVersion( _pFilter ? _pFilter->GetVersion() : SOFFICE_FILEFORMAT_NOW );
    return xStor;
}

//-----------------------------------------------------------------------------

sal_Int16 SAL_CALL SfxStandaloneDocumentInfoObject::getUserFieldCount() throw( ::com::sun::star::uno::RuntimeException )
{
    return SfxDocumentInfoObject::getUserFieldCount();
}

//-----------------------------------------------------------------------------

::rtl::OUString SAL_CALL  SfxStandaloneDocumentInfoObject::getUserFieldName(sal_Int16 nIndex) throw( ::com::sun::star::uno::RuntimeException )
{
    return SfxDocumentInfoObject::getUserFieldName(nIndex);
}

//-----------------------------------------------------------------------------

::rtl::OUString SAL_CALL  SfxStandaloneDocumentInfoObject::getUserFieldValue(sal_Int16 nIndex) throw( ::com::sun::star::uno::RuntimeException )
{
    return SfxDocumentInfoObject::getUserFieldValue(nIndex);
}

//-----------------------------------------------------------------------------

void  SAL_CALL SfxStandaloneDocumentInfoObject::setUserFieldName(sal_Int16 nIndex, const ::rtl::OUString& aName ) throw( ::com::sun::star::uno::RuntimeException )
{
    SfxDocumentInfoObject::setUserFieldName( nIndex, aName );
}

//-----------------------------------------------------------------------------

void SAL_CALL  SfxStandaloneDocumentInfoObject::setUserFieldValue( sal_Int16 nIndex, const ::rtl::OUString& aValue ) throw( ::com::sun::star::uno::RuntimeException )
{
    SfxDocumentInfoObject::setUserFieldValue( nIndex, aValue );
}
//-----------------------------------------------------------------------------

void SAL_CALL  SfxStandaloneDocumentInfoObject::loadFromURL(const ::rtl::OUString& aURL) throw( ::com::sun::star::uno::RuntimeException )
{
    sal_Bool bOK = sal_False;
    String aName( aURL );
    SvStorage* pStor = GetStorage_Impl( aName, sal_False );
    if ( pStor )
    {
        if ( !_pInfo )
            _pInfo = new SfxDocumentInfo;

        // DocInfo laden
        bOK = _pInfo->Load( pStor );
    }

    DELETEZ( _pMedium );
    if ( !bOK )
        throw SfxIOException_Impl( ERRCODE_IO_CANTREAD );
}

//-----------------------------------------------------------------------------

void SAL_CALL  SfxStandaloneDocumentInfoObject::storeIntoURL(const ::rtl::OUString& aURL) throw( ::com::sun::star::uno::RuntimeException )
{
    sal_Bool bOK = sal_False;
    String aName( aURL );
    SvStorage* pStor = GetStorage_Impl( aName, sal_True );
    if ( pStor )
    {
        if ( !_pInfo )
            _pInfo = new SfxDocumentInfo;

        // DocInfo speichern
        bOK = _pInfo->Save( pStor ) && pStor->Commit();
    }

    DELETEZ( _pMedium );
    if ( !bOK )
        throw SfxIOException_Impl( ERRCODE_IO_CANTREAD );
}

//=============================================================================
/* ASMUSS
#ifdef TEST_CODE
    // Testcode fuer DocumentInfoObject
    // Einmal zum Holen
    ::com::sun::star::uno::Reference< XServiceManager >  xMan = getGlobalServiceManager();
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Uik > aIfaces( 2 );
    aIfaces.getArray()[0] = ::getCppuType((const Reference< ::com::sun::star::document::XDocumentInfo >*)0);
    aIfaces.getArray()[1] = ::getCppuType((const Reference< ::com::sun::star::beans::XFastPropertySet >*)0);
    ::com::sun::star::uno::Reference< XServiceProvider >  xSSI = xMan->getServiceProvider(
            "com.sun.star.document.DocumentInfo", aIfaces, ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Uik >() );
// automatisch auskommentiert - [getIdlClass(es) or queryInterface] - Bitte XTypeProvider benutzen!
//  ::com::sun::star::uno::Reference< ::com::sun::star::document::XDocumentInfo >  xInfo = (::com::sun::star::document::XDocumentInfo*) xSSI->newInstance()->queryInterface( ::getCppuType((const Reference< ::com::sun::star::document::XDocumentInfo >*)0) );

// automatisch auskommentiert - [getIdlClass(es) or queryInterface] - Bitte XTypeProvider benutzen!
//  ::com::sun::star::uno::Reference< ::com::sun::star::beans::XFastPropertySet >  xProps = (::com::sun::star::beans::XFastPropertySet*) xInfo->queryInterface( ::getCppuType((const Reference< ::com::sun::star::beans::XFastPropertySet >*)0) );


    // Laden und Werte auslesen
    xInfo->load( StringToOUString( aURL, CHARSET_SYSTEM ) );
    ::com::sun::star::uno::Any aValue = xProps->getFastPropertyValue( WID_FROM );
    String aAuthor = OUStringToString( aValue.getString(), CHARSET_SYSTEM );
#endif
*/


