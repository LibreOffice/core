/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <cppuhelper/interfacecontainer.hxx>
#include <com/sun/star/io/IOException.hpp>

#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/document/XImporter.hpp>

#include <legacysmgr/legacy_binfilters_smgr.hxx>

#include <tools/errcode.hxx>
#include <bf_so3/svstor.hxx>
#include <bf_svtools/cntwids.hrc>
#include <bf_svtools/itemset.hxx>
#include <bf_svtools/stritem.hxx>
#include <bf_svtools/intitem.hxx>
#include <bf_svtools/eitem.hxx>
#include <bf_svtools/adrparse.hxx>
#include <unotools/streamwrap.hxx>

#include "objuno.hxx"
#include "sfx.hrc"
#include "sfxsids.hrc"
#include "printer.hxx"
#include "objsh.hxx"
#include "docinf.hxx"
#include "docfile.hxx"
#include "openflag.hxx"
#include "app.hxx"
#include "fcontnr.hxx"
#include "request.hxx"
#include "sfxuno.hxx"
#include "objshimp.hxx"

#include <osl/mutex.hxx>
#include <osl/mutex.hxx>
namespace binfilter {

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;

#define SfxIOException_Impl( nErr ) ::com::sun::star::io::IOException()

//=============================================================================

#define PROPERTY_UNBOUND 0
#define PROPERTY_MAYBEVOID ::com::sun::star::beans::PropertyAttribute::MAYBEVOID

/*N*/ SfxItemPropertyMap aDocInfoPropertyMap_Impl[] =
/*N*/ {
/*N*/ 	{ "Author"			, 6	, WID_FROM,           &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
/*N*/ 	{ "AutoloadEnabled"	, 15, MID_DOCINFO_AUTOLOADENABLED, &::getBooleanCppuType(),   PROPERTY_UNBOUND, 0 },
/*N*/ 	{ "AutoloadSecs"	, 12, MID_DOCINFO_AUTOLOADSECS, &::getCppuType((const sal_Int16*)0),     PROPERTY_UNBOUND, 0 },
/*N*/ 	{ "AutoloadURL"		, 11, MID_DOCINFO_AUTOLOADURL, &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
/*N*/ 	{ "BlindCopiesTo"	, 13, WID_BCC,            &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
/*N*/ 	{ "CopyTo"			, 6 , WID_CC,             &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
/*N*/     { "CreationDate"    , 12, WID_DATE_CREATED,   &::getCppuType((const ::com::sun::star::util::DateTime*)0),PROPERTY_MAYBEVOID, 0 },
/*N*/ 	{ "DefaultTarget"	, 13, MID_DOCINFO_DEFAULTTARGET, &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
/*N*/ 	{ "Description"		, 11, MID_DOCINFO_DESCRIPTION, &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
/*N*/ 	{ "EditingCycles"	, 13, MID_DOCINFO_REVISION, &::getCppuType((const sal_Int16*)0),   PROPERTY_UNBOUND, 0 },
/*N*/ 	{ "EditingDuration"	, 15, MID_DOCINFO_EDITTIME, &::getCppuType((const sal_Int32*)0),   PROPERTY_UNBOUND, 0 },
/*N*/ 	{ "InReplyTo"		, 9 , WID_IN_REPLY_TO,    &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
/*N*/     { "IsEncrypted"		, 11, MID_DOCINFO_ENCRYPTED, &::getBooleanCppuType(),     PROPERTY_UNBOUND | ::com::sun::star::beans::PropertyAttribute::READONLY, 0 },
/*N*/ 	{ "Keywords"		, 8 , WID_KEYWORDS,       &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
/*N*/     { "MIMEType"		, 8 , WID_CONTENT_TYPE,   &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND | ::com::sun::star::beans::PropertyAttribute::READONLY, 0 },
/*N*/ 	{ "ModifiedBy"		, 10, MID_DOCINFO_MODIFICATIONAUTHOR, &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
/*N*/     { "ModifyDate"      , 10, WID_DATE_MODIFIED,  &::getCppuType((const ::com::sun::star::util::DateTime*)0),PROPERTY_MAYBEVOID, 0 },
/*N*/ 	{ "Newsgroups"		, 10, WID_NEWSGROUPS,     &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
/*N*/ 	{ "Original"		, 8 , WID_MESSAGE_ID,     &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
/*N*/     { "PrintDate"       , 9 , MID_DOCINFO_PRINTDATE, &::getCppuType((const ::com::sun::star::util::DateTime*)0),PROPERTY_MAYBEVOID, 0 },
/*N*/ 	{ "PrintedBy"		, 9 , MID_DOCINFO_PRINTEDBY, &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
/*N*/ 	{ "Priority"		, 8 , WID_PRIORITY,       &::getCppuType((const sal_Int16*)0),   PROPERTY_UNBOUND, 0 },
/*N*/ 	{ "Recipient"		, 9 , WID_TO,             &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
/*N*/ 	{ "References"		, 10, WID_REFERENCES,     &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
/*N*/ 	{ "ReplyTo"			, 7 , WID_REPLY_TO,       &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
/*N*/     { "SaveVersionOnClose", 18 , SID_VERSION, &::getBooleanCppuType(), PROPERTY_UNBOUND, 0 },
/*N*/ 	{ "Template"		, 8 , MID_DOCINFO_TEMPLATE, &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
/*N*/ 	{ "TemplateFileName", 16, SID_TEMPLATE_NAME, &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
/*N*/     { "TemplateDate"    , 12, MID_DOCINFO_TEMPLATEDATE, &::getCppuType((const ::com::sun::star::util::DateTime*)0),PROPERTY_MAYBEVOID, 0 },
/*N*/ 	{ "Theme"			, 5 , MID_DOCINFO_SUBJECT, &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
/*N*/ 	{ "Title"			, 5 , WID_TITLE,          &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
/*N*/ 	{0,0,0,0,0}
/*N*/ };

//-----------------------------------------------------------------------------
/*N*/ SFX_IMPL_XINTERFACE_5( SfxDocumentInfoObject, OWeakObject, ::com::sun::star::document::XDocumentInfo, ::com::sun::star::lang::XComponent, ::com::sun::star::beans::XPropertySet, ::com::sun::star::beans::XFastPropertySet, ::com::sun::star::beans::XPropertyAccess )
/*N*/ SFX_IMPL_XTYPEPROVIDER_5( SfxDocumentInfoObject, ::com::sun::star::document::XDocumentInfo, ::com::sun::star::lang::XComponent, ::com::sun::star::beans::XPropertySet, ::com::sun::star::beans::XFastPropertySet, ::com::sun::star::beans::XPropertyAccess )
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
/*N*/ struct SfxDocumentInfoObject_Impl
/*N*/ {
/*N*/ 	SfxObjectShell*						_pObjSh;
/*N*/ 	::osl::Mutex						_aMutex;
/*N*/ 	::cppu::OInterfaceContainerHelper	_aDisposeContainer;
/*N*/
/*N*/ 	SfxDocumentInfoObject_Impl( SfxObjectShell* pObjSh )
/*N*/ 		: _pObjSh( pObjSh )
/*N*/ 		, _aDisposeContainer( _aMutex )
/*N*/ 	{
/*N*/ 	}
/*N*/ };

//-----------------------------------------------------------------------------

/*N*/ SfxDocumentInfoObject::SfxDocumentInfoObject( sal_Bool bStandalone )
/*N*/ 	: _aPropSet( aDocInfoPropertyMap_Impl )
/*N*/ 	, _pInfo( NULL )
/*N*/ 	, _pFilter( NULL )
/*N*/ 	, _bStandalone( bStandalone )
/*N*/ 	, _pImp( new SfxDocumentInfoObject_Impl( NULL ) )
/*N*/ {
        _pInfo = new SfxDocumentInfo;
/*N*/ }

//-----------------------------------------------------------------------------

/*N*/ SfxDocumentInfoObject::SfxDocumentInfoObject( SfxObjectShell *pObjSh )
/*N*/ 	: _aPropSet( aDocInfoPropertyMap_Impl )
/*N*/ 	, _pInfo( 0 )
/*N*/ 	, _pFilter( NULL )
/*N*/ 	, _bStandalone( sal_False )
/*N*/ 	, _pImp( new SfxDocumentInfoObject_Impl( pObjSh ) )
/*N*/ {
/*N*/ 	if ( pObjSh )
/*N*/ 	{
/*N*/ 		SfxDocumentInfo &rDocInfo = pObjSh->GetDocInfo();
/*N*/ 		_pInfo = &rDocInfo;
/*N*/ 		_pFilter = pObjSh->GetMedium()->GetFilter();
/*N*/         _wModel = pObjSh->GetModel();
/*N*/ 	}
/*N*/ }

//-----------------------------------------------------------------------------

/*N*/ SfxDocumentInfoObject::~SfxDocumentInfoObject()
/*N*/ {
/*N*/ 	if ( _bStandalone )
/*N*/ 		delete _pInfo;
/*N*/ 	delete _pImp;
/*N*/ }

//-----------------------------------------------------------------------------

/*N*/ void SAL_CALL SfxDocumentInfoObject::dispose() throw( ::com::sun::star::uno::RuntimeException )
/*N*/ {
/*N*/ 	::com::sun::star::lang::EventObject aEvent( (::cppu::OWeakObject *)this );
/*N*/ 	_pImp->_aDisposeContainer.disposeAndClear( aEvent );

/*
    _xCurrent = ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController > ();
    for ( sal_uInt32 n = _aControllers.ge.getLength(); n; --n )
        SequenceRemoveElementAt( _aControllers, n-1 );
*/
/*N*/ }

//-----------------------------------------------------------------------------

/*N*/ void SAL_CALL  SfxDocumentInfoObject::addEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw( ::com::sun::star::uno::RuntimeException )
/*N*/ {
/*N*/ 	_pImp->_aDisposeContainer.addInterface( aListener );
/*N*/ }

//-----------------------------------------------------------------------------

/*N*/ void SAL_CALL  SfxDocumentInfoObject::removeEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw( ::com::sun::star::uno::RuntimeException )
/*N*/ {
/*N*/ 	_pImp->_aDisposeContainer.removeInterface( aListener );
/*N*/ }
//-----------------------------------------------------------------------------

/*N*/ ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >  SAL_CALL  SfxDocumentInfoObject::getPropertySetInfo()  throw( ::com::sun::star::uno::RuntimeException )
/*N*/ {
/*N*/ 	return _aPropSet.getPropertySetInfo();
/*N*/ }

//-----------------------------------------------------------------------------

/*N*/ void SAL_CALL  SfxDocumentInfoObject::setPropertyValue(const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue) throw( ::com::sun::star::uno::RuntimeException )
/*N*/ {
/*N*/ 	const SfxItemPropertyMap* pMap = SfxItemPropertyMap::GetByName(
/*N*/ 			aDocInfoPropertyMap_Impl,
/*N*/ 			aPropertyName );
/*N*/ 	if ( pMap )
/*N*/ 		setFastPropertyValue( pMap->nWID, aValue );
/*N*/ }

//-----------------------------------------------------------------------------

/*N*/ ::com::sun::star::uno::Any  SAL_CALL  SfxDocumentInfoObject::getPropertyValue(const ::rtl::OUString& aPropertyName)  throw( ::com::sun::star::uno::RuntimeException )
/*N*/ {
/*N*/ 	const SfxItemPropertyMap* pMap = SfxItemPropertyMap::GetByName( aDocInfoPropertyMap_Impl,
/*N*/ 		aPropertyName );
/*N*/ 	if ( pMap )
/*N*/ 		return getFastPropertyValue( pMap->nWID );
/*N*/ 	else
/*N*/ 		return ::com::sun::star::uno::Any();
/*N*/ }

//-----------------------------------------------------------------------------

/*N*/ void SAL_CALL  SfxDocumentInfoObject::addPropertyChangeListener(const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener > & aListener) throw( ::com::sun::star::uno::RuntimeException )
/*N*/ {}

//-----------------------------------------------------------------------------

/*N*/ void SAL_CALL  SfxDocumentInfoObject::removePropertyChangeListener(const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener > & aListener) throw( ::com::sun::star::uno::RuntimeException )
/*N*/ {}

//-----------------------------------------------------------------------------

/*N*/ void SAL_CALL  SfxDocumentInfoObject::addVetoableChangeListener(const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener > & aListener) throw( ::com::sun::star::uno::RuntimeException )
/*N*/ {}

//-----------------------------------------------------------------------------

/*N*/ void SAL_CALL  SfxDocumentInfoObject::removeVetoableChangeListener(const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener > & aListener) throw( ::com::sun::star::uno::RuntimeException )
/*N*/ {}
/*N*/
/*N*/ ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL  SfxDocumentInfoObject::getPropertyValues( void ) throw( ::com::sun::star::uno::RuntimeException )
/*N*/ {
/*N*/ 	::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >  xInfo = getPropertySetInfo();
/*N*/ 	::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property > aProps = xInfo->getProperties();
/*N*/
/*N*/ 	const ::com::sun::star::beans::Property* pProps  = aProps.getConstArray();
/*N*/ 	sal_uInt32 nCount = aProps.getLength();
/*N*/
/*N*/ 	::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >aSeq( nCount );
/*N*/ 	::com::sun::star::beans::PropertyValue*  pValues = aSeq.getArray();
/*N*/
/*N*/ 	for ( sal_uInt32 n = 0; n < nCount; ++n )
/*N*/ 	{
/*N*/ 		::com::sun::star::beans::PropertyValue& rCurrValue = pValues[n];
/*N*/ 		const ::com::sun::star::beans::Property& rCurrProp = pProps[n];
/*N*/
/*N*/ 		rCurrValue.Name = rCurrProp.Name;
/*N*/ 		rCurrValue.Handle = rCurrProp.Handle;
/*N*/ 		rCurrValue.Value = getPropertyValue( rCurrProp.Name );
/*N*/ 	}
/*N*/
/*N*/ 	return aSeq;
/*N*/ }
/*N*/
/*N*/ void SAL_CALL  SfxDocumentInfoObject::setPropertyValues( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aProps )
/*N*/         throw( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException )
/*N*/ {
/*N*/ 	const ::com::sun::star::beans::PropertyValue* pProps  = aProps.getConstArray();
/*N*/ 	sal_uInt32 nCount = aProps.getLength();
/*N*/
/*N*/ 	for ( sal_uInt32 n = 0; n < nCount; ++n )
/*N*/ 	{
/*N*/ 		const ::com::sun::star::beans::PropertyValue& rProp = pProps[n];
/*N*/ 		setPropertyValue( rProp.Name, rProp.Value );
/*N*/ 	}
/*N*/ }
/*N*/
/*N*/ ::com::sun::star::util::DateTime SfxDocumentInfoObject::impl_DateTime_Object2Struct( const ::DateTime& aDateTimeObject )
/*N*/ {
/*N*/ 	//	Attention!
/*N*/ 	//		We can't use a DateTime-object as type of any property with new UNO!
/*N*/ 	//		We must use a DateTime-struct and convert the values.
/*N*/
/*N*/ 	::com::sun::star::util::DateTime	aDateTimeStruct	;	// com/sun/star/util/DateTime	!!!
/*N*/
/*N*/ 	// ... and convert DateTime-object to struct ! (copy values)
/*N*/     aDateTimeStruct.HundredthSeconds	= aDateTimeObject.Get100Sec	() ;
/*N*/     aDateTimeStruct.Seconds				= aDateTimeObject.GetSec	() ;
/*N*/     aDateTimeStruct.Minutes				= aDateTimeObject.GetMin	() ;
/*N*/     aDateTimeStruct.Hours				= aDateTimeObject.GetHour	() ;
/*N*/     aDateTimeStruct.Day					= aDateTimeObject.GetDay	() ;
/*N*/     aDateTimeStruct.Month				= aDateTimeObject.GetMonth	() ;
/*N*/     aDateTimeStruct.Year				= aDateTimeObject.GetYear	() ;
/*N*/
/*N*/ 	// Return ricght type with right values.
/*N*/ 	return aDateTimeStruct ;
/*N*/ }

/*N*/ ::DateTime SfxDocumentInfoObject::impl_DateTime_Struct2Object ( const ::com::sun::star::util::DateTime& aDateTimeStruct )
/*N*/ {
/*N*/ 	//	Attention!
/*N*/ 	//		We can't use a DateTime-object as type of any property with new UNO!
/*N*/ 	//		We must use a DateTime-struct and convert the values.
/*N*/
/*N*/ 	// Get values from DateTime-struct and create a date and time instance ...
/*N*/ 	::Date aDate ( aDateTimeStruct.Day	, aDateTimeStruct.Month		, aDateTimeStruct.Year										) ;
/*N*/ 	::Time aTime ( aDateTimeStruct.Hours, aDateTimeStruct.Minutes	, aDateTimeStruct.Seconds, aDateTimeStruct.HundredthSeconds ) ;
/*N*/
/*N*/ 	// ... to use it for creation of a DateTime-object ...
/*N*/ 	::DateTime	aDateTimeObject ( aDate, aTime		) ;
/*N*/
/*N*/ 	// Return converted values.
/*N*/ 	return aDateTimeObject ;
/*N*/ }


/*N*/ void SAL_CALL  SfxDocumentInfoObject::setFastPropertyValue(sal_Int32 nHandle, const ::com::sun::star::uno::Any& aValue) throw( ::com::sun::star::uno::RuntimeException )
/*N*/ {
/*N*/ 	SolarMutexGuard aGuard;
/*N*/ 	sal_Bool bModified = sal_True;
/*N*/
/*N*/ 	if ( aValue.getValueType() == ::getCppuType((const ::rtl::OUString*)0) )
/*N*/ 	{
/*N*/ 		::rtl::OUString sTemp ;
/*N*/ 		aValue >>= sTemp ;
/*N*/ 		String aStrVal( sTemp );
/*N*/ 		switch ( nHandle )
/*N*/ 		{
/*N*/ 			case WID_FROM :
/*N*/ 			{
/*N*/ 				const SfxStamp& rStamp = _pInfo->GetCreated();
/*N*/ 				if ( aStrVal.Len() > TIMESTAMP_MAXLENGTH )
/*N*/ 				{
/*N*/ 					SvAddressParser aParser( aStrVal );
/*N*/ 					if ( aParser.Count() > 0 )
/*N*/ 					{
/*N*/ 						String aEmail = aParser.GetEmailAddress(0);
/*N*/ 						String aRealname = aParser.GetRealName(0);
/*N*/
/*N*/ 						if ( aRealname.Len() <= TIMESTAMP_MAXLENGTH )
/*N*/ 							aStrVal = aRealname;
/*N*/ 						else if ( aEmail.Len() <= TIMESTAMP_MAXLENGTH )
/*N*/ 							aStrVal = aEmail;
/*N*/ 					}
/*N*/ 				}
/*N*/ 				_pInfo->SetCreated( SfxStamp( aStrVal, rStamp.GetTime() ) );
/*N*/ 				break;
/*N*/ 			}
/*N*/             case MID_DOCINFO_PRINTEDBY:
/*N*/             {
/*N*/ 				const SfxStamp& rStamp = _pInfo->GetPrinted();
/*N*/ 				_pInfo->SetPrinted( SfxStamp( aStrVal, rStamp.GetTime() ) );
/*N*/                 break;
/*N*/             }
/*N*/ 			case MID_DOCINFO_MODIFICATIONAUTHOR:
/*N*/ 			{
/*N*/ 				const SfxStamp& rStamp = _pInfo->GetChanged();
/*N*/                 _pInfo->SetChanged( SfxStamp( aStrVal, rStamp.GetTime() ) );
/*N*/ 				break;
/*N*/ 			}
/*N*/ 			case WID_TITLE :
/*N*/             {
/*N*/ 				_pInfo->SetTitle( aStrVal );
/*N*/                 Reference < XModel > xModel( _wModel.get(), UNO_QUERY );
/*N*/                 if ( xModel.is() )
/*N*/                 {
/*N*/                     _pImp->_pObjSh->InvalidateName();
/*N*/                     _pImp->_pObjSh->Broadcast( SfxSimpleHint( SFX_HINT_TITLECHANGED ) );
/*N*/                 }
/*N*/ 				break;
/*N*/             }
/*N*/ 			case MID_DOCINFO_SUBJECT :
/*N*/ 				_pInfo->SetTheme( aStrVal );
/*N*/ 				break;
/*N*/ 			case WID_KEYWORDS :
/*N*/ 				_pInfo->SetKeywords( aStrVal );
/*N*/ 				break;
/*N*/ 			case WID_CC :
/*N*/ 				_pInfo->SetCopiesTo( aStrVal );
/*N*/ 				break;
/*N*/ 			case WID_MESSAGE_ID :
/*N*/ 				_pInfo->SetOriginal( aStrVal );
/*N*/ 				break;
/*N*/ 			case WID_REFERENCES :
/*N*/ 				_pInfo->SetReferences( aStrVal );
/*N*/ 				break;
/*N*/ 			case WID_TO :
/*N*/ 				_pInfo->SetRecipient( aStrVal );
/*N*/ 				break;
/*N*/ 			case WID_REPLY_TO :
/*N*/ 				_pInfo->SetReplyTo( aStrVal );
/*N*/ 				break;
/*N*/ 			case MID_DOCINFO_TEMPLATE:
/*N*/ 				_pInfo->SetTemplateName( aStrVal );
/*N*/ 				break;
/*N*/ 			case SID_TEMPLATE_NAME:
/*N*/ 				_pInfo->SetTemplateFileName( aStrVal );
/*N*/ 				break;
/*N*/ 			case WID_BCC :
/*N*/ 				_pInfo->SetBlindCopies( aStrVal );
/*N*/ 				break;
/*N*/ 			case WID_IN_REPLY_TO :
/*N*/ 				_pInfo->SetInReplyTo( aStrVal );
/*N*/ 				break;
/*N*/ 			case WID_NEWSGROUPS :
/*N*/ 				_pInfo->SetNewsgroups( aStrVal );
/*N*/ 				break;
/*N*/ 			case MID_DOCINFO_DESCRIPTION:
/*N*/ 				_pInfo->SetComment( aStrVal );
/*N*/ 				break;
/*N*/ 			case MID_DOCINFO_AUTOLOADURL:
/*N*/ 				_pInfo->SetReloadURL( aStrVal );
/*N*/ 				break;
/*N*/ 			case MID_DOCINFO_DEFAULTTARGET:
/*N*/ 				_pInfo->SetDefaultTarget( aStrVal );
/*N*/ 				break;
/*N*/ 			default:
/*N*/ 				bModified = sal_False;
/*N*/ 				break;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else if ( aValue.getValueType() == ::getCppuType((const ::com::sun::star::util::DateTime*)0) )
/*N*/ 	{
/*N*/ 		switch ( nHandle )
/*N*/ 		{
/*N*/ 			case WID_DATE_CREATED :
/*N*/ 			{
/*N*/ 				const SfxStamp& rStamp = _pInfo->GetCreated();
/*N*/ 				::com::sun::star::util::DateTime aDateTime ;
/*N*/ 				aValue >>= aDateTime ;
/*N*/ 				_pInfo->SetCreated( SfxStamp( rStamp.GetName(), impl_DateTime_Struct2Object(aDateTime) ) );
/*N*/ 				break;
/*N*/ 			}
/*N*/ 			case WID_DATE_MODIFIED :
/*N*/ 			{
/*N*/ 				const SfxStamp& rStamp = _pInfo->GetChanged();
/*N*/ 				::com::sun::star::util::DateTime aDateTime ;
/*N*/ 				aValue >>= aDateTime ;
/*N*/                 _pInfo->SetChanged( SfxStamp( rStamp.GetName(), impl_DateTime_Struct2Object(aDateTime) ) );
/*N*/ 				break;
/*N*/ 			}
/*N*/             case MID_DOCINFO_PRINTDATE :
/*N*/ 			{
/*N*/                 const SfxStamp& rStamp = _pInfo->GetPrinted();
/*N*/ 				::com::sun::star::util::DateTime aDateTime ;
/*N*/ 				aValue >>= aDateTime ;
/*N*/                 _pInfo->SetPrinted( SfxStamp( rStamp.GetName(), impl_DateTime_Struct2Object(aDateTime) ) );
/*N*/ 				break;
/*N*/ 			}
/*N*/             case MID_DOCINFO_TEMPLATEDATE :
/*N*/ 			{
/*N*/ 				::com::sun::star::util::DateTime aDateTime ;
/*N*/ 				aValue >>= aDateTime ;
/*N*/                 _pInfo->SetTemplateDate( impl_DateTime_Struct2Object(aDateTime) );
/*N*/ 				break;
/*N*/ 			}
/*N*/ 			default:
/*N*/ 				bModified = sal_False;
/*N*/ 				break;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else if ( aValue.getValueType() == ::getBooleanCppuType() )
/*N*/ 	{
/*N*/ 		sal_Bool bBoolVal ;
/*N*/ 		aValue >>= bBoolVal ;
/*N*/ 		switch ( nHandle )
/*N*/ 		{
/*N*/             case SID_VERSION :
/*N*/                 _pInfo->SetSaveVersionOnClose( bBoolVal );
/*N*/                 break;
/*N*/ 			case MID_DOCINFO_AUTOLOADENABLED:
/*N*/ 				_pInfo->EnableReload( bBoolVal );
/*N*/ 				break;
/*N*/ 			default:
/*N*/ 				bModified = sal_False;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else if ( aValue.getValueType() == ::getCppuType((const sal_Int32*)0) )
/*N*/ 	{
/*N*/ 		long nIntVal ;
/*N*/ 		aValue >>= nIntVal ;
/*N*/ 		switch ( nHandle )
/*N*/ 		{
/*N*/ 			case MID_DOCINFO_AUTOLOADSECS:
/*N*/ 				_pInfo->SetReloadDelay( nIntVal );
/*N*/ 				break;
/*N*/ 			case MID_DOCINFO_EDITTIME:
/*N*/ 				_pInfo->SetTime( nIntVal );
/*N*/ 			default:
/*N*/ 				bModified = sal_False;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else if ( aValue.getValueType() == ::getCppuType((const sal_Int16*)0) )
/*N*/ 	{
/*N*/ 		short nIntVal ;
/*N*/ 		aValue >>= nIntVal ;
/*N*/ 		switch ( nHandle )
/*N*/ 		{
/*N*/ 			case MID_DOCINFO_REVISION:
/*N*/ 				_pInfo->SetDocumentNumber( nIntVal );
/*N*/ 			case WID_PRIORITY :
/*N*/ 				_pInfo->SetPriority( nIntVal );
/*N*/ 				break;
/*N*/ 			default:
/*N*/ 				bModified = sal_False;
/*N*/ 				break;
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/     Reference < XModel > xModel( _wModel.get(), UNO_QUERY );
/*N*/     if ( bModified && xModel.is() )
/*N*/             _pImp->_pObjSh->FlushDocInfo();
/*N*/ }

//-----------------------------------------------------------------------------

/*N*/ ::com::sun::star::uno::Any SAL_CALL  SfxDocumentInfoObject::getFastPropertyValue(sal_Int32 nHandle) throw( ::com::sun::star::uno::RuntimeException )
/*N*/ {
/*N*/ 	SolarMutexGuard aGuard;
/*N*/ 	::com::sun::star::uno::Any aValue;
/*N*/ 	if ( nHandle == WID_CONTENT_TYPE )
/*N*/ 	{
/*N*/         if ( _pInfo->GetSpecialMimeType().Len() )
/*N*/ 		{
/*N*/ 			::rtl::OUString sTemp ( _pInfo->GetSpecialMimeType() );
/*N*/ 			aValue <<= sTemp ;
/*N*/ 		}
/*N*/ 		else if ( _pFilter )
/*N*/ 		{
/*N*/ 			::rtl::OUString sTemp ( _pFilter->GetMimeType() );
/*N*/ 			aValue <<= sTemp ;
/*N*/ 		}
/*N*/ 		else
/*N*/ 			aValue <<= ::rtl::OUString() ;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		switch ( nHandle )
/*N*/ 		{
/*N*/             case SID_VERSION :
/*N*/                 aValue <<= _pInfo->IsSaveVersionOnClose();
/*N*/                 break;
/*N*/ 			case MID_DOCINFO_REVISION :
/*N*/ 				aValue <<= _pInfo->GetDocumentNumber() ;
/*N*/ 				break;
/*N*/ 			case MID_DOCINFO_EDITTIME :
/*N*/ 				aValue <<= _pInfo->GetTime();
/*N*/ 				break;
/*N*/ 			case WID_FROM :
/*N*/ 			{
/*N*/ 				const SfxStamp& rStamp = _pInfo->GetCreated();
/*N*/ 				if ( rStamp.IsValid() )
/*N*/ 					aValue <<= ::rtl::OUString( rStamp.GetName() );
/*N*/                 else
/*N*/ 					aValue <<= ::rtl::OUString() ;
/*N*/ 				break;
/*N*/ 			}
/*N*/ 			case WID_DATE_CREATED :
/*N*/ 			{
/*N*/ 				const SfxStamp& rStamp = _pInfo->GetCreated();
/*N*/ 				if ( rStamp.IsValid() )
/*N*/ 					aValue <<= impl_DateTime_Object2Struct(rStamp.GetTime());
/*N*/ 				break;
/*N*/ 			}
/*N*/ 			case WID_TITLE :
/*N*/ 				aValue <<= ::rtl::OUString( _pInfo->GetTitle() );
/*N*/ 				break;
/*N*/ 			case MID_DOCINFO_SUBJECT:
/*N*/ 				aValue <<= ::rtl::OUString( _pInfo->GetTheme() );
/*N*/ 				break;
/*N*/ 			case MID_DOCINFO_MODIFICATIONAUTHOR:
/*N*/ 			{
/*N*/ 				const SfxStamp& rStamp = _pInfo->GetChanged();
/*N*/ 				if ( rStamp.IsValid() )
/*N*/ 					aValue <<= ::rtl::OUString( rStamp.GetName() );
/*N*/                 else
/*N*/ 					aValue <<= ::rtl::OUString() ;
/*N*/ 				break;
/*N*/ 			}
/*N*/ 			case WID_DATE_MODIFIED :
/*N*/ 			{
/*N*/ 				const SfxStamp& rStamp = _pInfo->GetChanged();
/*N*/ 				if ( rStamp.IsValid() )
/*N*/ 					aValue <<= impl_DateTime_Object2Struct(rStamp.GetTime());
/*N*/ 				break;
/*N*/ 			}
/*N*/ 			case MID_DOCINFO_PRINTEDBY:
/*N*/ 			{
/*N*/ 				const SfxStamp& rStamp = _pInfo->GetPrinted();
/*N*/ 				if ( rStamp.IsValid() )
/*N*/ 					aValue <<= ::rtl::OUString( rStamp.GetName() );
/*N*/                 else
/*N*/ 					aValue <<= ::rtl::OUString() ;
/*N*/ 				break;
/*N*/ 			}
/*N*/ 			case MID_DOCINFO_PRINTDATE:
/*N*/ 			{
/*N*/ 				const SfxStamp& rStamp = _pInfo->GetPrinted();
/*N*/ 				if ( rStamp.IsValid() )
/*N*/ 					aValue <<= impl_DateTime_Object2Struct(rStamp.GetTime());
/*N*/ 				break;
/*N*/ 			}
/*N*/ 			case WID_KEYWORDS :
/*N*/ 				aValue <<= ::rtl::OUString( _pInfo->GetKeywords() );
/*N*/ 				break;
/*N*/ 			case WID_PRIORITY :
/*N*/ 				aValue <<= _pInfo->GetPriority() ;
/*N*/ 				break;
/*N*/ 			case WID_CC :
/*N*/ 				aValue <<= ::rtl::OUString( _pInfo->GetCopiesTo() );
/*N*/ 				break;
/*N*/ 			case WID_MESSAGE_ID :
/*N*/ 				aValue <<=  ::rtl::OUString( _pInfo->GetOriginal() );
/*N*/ 				break;
/*N*/ 			case WID_REFERENCES :
/*N*/ 				aValue <<=  ::rtl::OUString( _pInfo->GetReferences() );
/*N*/ 				break;
/*N*/ 			case WID_TO :
/*N*/ 				aValue <<=  ::rtl::OUString( _pInfo->GetRecipient() );
/*N*/ 				break;
/*N*/ 			case WID_REPLY_TO :
/*N*/ 				aValue <<=  ::rtl::OUString( _pInfo->GetReplyTo() );
/*N*/ 				break;
/*N*/ 			case WID_BCC :
/*N*/ 				aValue <<=  ::rtl::OUString( _pInfo->GetBlindCopies() );
/*N*/ 				break;
/*N*/ 			case WID_IN_REPLY_TO :
/*N*/ 				aValue <<=  ::rtl::OUString( _pInfo->GetInReplyTo() );
/*N*/ 				break;
/*N*/ 			case WID_NEWSGROUPS :
/*N*/ 				aValue <<=  ::rtl::OUString( _pInfo->GetNewsgroups() );
/*N*/ 				break;
/*N*/ 			case MID_DOCINFO_DESCRIPTION:
/*N*/ 				aValue <<=  ::rtl::OUString( _pInfo->GetComment() );
/*N*/ 				break;
/*N*/ 			case MID_DOCINFO_TEMPLATE:
/*N*/ 				aValue <<=  ::rtl::OUString( _pInfo->GetTemplateName() );
/*N*/ 				break;
/*N*/ 			case SID_TEMPLATE_NAME:
/*N*/ 				aValue <<=  ::rtl::OUString( _pInfo->GetTemplateFileName() );
/*N*/ 				break;
/*N*/ 			case MID_DOCINFO_TEMPLATEDATE:
/*N*/ 				aValue <<= impl_DateTime_Object2Struct(_pInfo->GetTemplateDate());
/*N*/ 				break;
/*N*/ 			case MID_DOCINFO_AUTOLOADENABLED:
/*N*/ 				aValue <<=  _pInfo->IsReloadEnabled() ;
/*N*/ 				break;
/*N*/ 			case MID_DOCINFO_AUTOLOADURL:
/*N*/ 				aValue <<=  ::rtl::OUString( _pInfo->GetReloadURL() );
/*N*/ 				break;
/*N*/ 			case MID_DOCINFO_AUTOLOADSECS:
/*N*/ 				aValue <<=  _pInfo->GetReloadDelay() ;
/*N*/ 				break;
/*N*/ 			case MID_DOCINFO_DEFAULTTARGET:
/*N*/ 				aValue <<=  ::rtl::OUString( _pInfo->GetDefaultTarget() );
/*N*/ 				break;
/*N*/ 			default:
/*N*/ 				aValue <<= ::rtl::OUString();
/*N*/ 				break;
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	return aValue;
/*N*/ }

//-----------------------------------------------------------------------------

/*N*/ sal_Int16 SAL_CALL  SfxDocumentInfoObject::getUserFieldCount() throw( ::com::sun::star::uno::RuntimeException )
/*N*/ {
/*N*/ 	SolarMutexGuard aGuard;
/*N*/ 	return _pInfo->GetUserKeyCount();
/*N*/ }

//-----------------------------------------------------------------------------

/*N*/ ::rtl::OUString SAL_CALL  SfxDocumentInfoObject::getUserFieldName(sal_Int16 nIndex) throw( ::com::sun::star::uno::RuntimeException )
/*N*/ {
/*N*/ 	SolarMutexGuard aGuard;
/*N*/ 	if ( nIndex < _pInfo->GetUserKeyCount() )
/*N*/ 		return _pInfo->GetUserKey( nIndex ).GetTitle();
/*N*/ 	else
/*N*/ 		return ::rtl::OUString();
/*N*/ }

//-----------------------------------------------------------------------------

/*N*/ ::rtl::OUString SAL_CALL  SfxDocumentInfoObject::getUserFieldValue(sal_Int16 nIndex) throw( ::com::sun::star::uno::RuntimeException )
/*N*/ {
/*N*/ 	SolarMutexGuard aGuard;
/*N*/ 	if ( nIndex < _pInfo->GetUserKeyCount() )
/*N*/ 		return _pInfo->GetUserKey( nIndex ).GetWord();
/*N*/ 	else
/*N*/ 		return ::rtl::OUString();
/*N*/ }

//-----------------------------------------------------------------------------

/*N*/ void  SAL_CALL SfxDocumentInfoObject::setUserFieldName(sal_Int16 nIndex, const ::rtl::OUString& aName ) throw( ::com::sun::star::uno::RuntimeException )
/*N*/ {
/*N*/ 	SolarMutexGuard aGuard;
/*N*/ 	if ( nIndex < _pInfo->GetUserKeyCount() )
/*N*/ 	{
/*N*/ 		const SfxDocUserKey& rKey = _pInfo->GetUserKey( nIndex );
/*N*/ 		_pInfo->SetUserKey( SfxDocUserKey( aName, rKey.GetWord() ), nIndex );
/*N*/         Reference < XModel > xModel( _wModel.get(), UNO_QUERY );
/*N*/         if ( xModel.is() )
/*N*/             _pImp->_pObjSh->FlushDocInfo();
/*N*/ 	}
/*N*/ }

//-----------------------------------------------------------------------------

/*N*/ void SAL_CALL  SfxDocumentInfoObject::setUserFieldValue( sal_Int16 nIndex, const ::rtl::OUString& aValue ) throw( ::com::sun::star::uno::RuntimeException )
/*N*/ {
/*N*/ 	SolarMutexGuard aGuard;
/*N*/ 	if ( nIndex < _pInfo->GetUserKeyCount() )
/*N*/ 	{
/*N*/ 		const SfxDocUserKey& rKey = _pInfo->GetUserKey( nIndex );
/*N*/ 		_pInfo->SetUserKey( SfxDocUserKey( rKey.GetTitle(), aValue ), nIndex );
/*N*/         Reference < XModel > xModel( _wModel.get(), UNO_QUERY );
/*N*/         if ( xModel.is() )
/*N*/             _pImp->_pObjSh->FlushDocInfo();
/*N*/ 	}
/*N*/ }

//-----------------------------------------------------------------------------
/*N*/ SFX_IMPL_XINTERFACE_2( SfxStandaloneDocumentInfoObject, SfxDocumentInfoObject, ::com::sun::star::lang::XServiceInfo, ::com::sun::star::document::XStandaloneDocumentInfo  )
/*N*/ SFX_IMPL_XTYPEPROVIDER_7( SfxStandaloneDocumentInfoObject, ::com::sun::star::lang::XServiceInfo, ::com::sun::star::document::XDocumentInfo, ::com::sun::star::lang::XComponent, ::com::sun::star::beans::XPropertySet, ::com::sun::star::beans::XFastPropertySet, ::com::sun::star::beans::XPropertyAccess, ::com::sun::star::document::XStandaloneDocumentInfo )
/*N*/ SFX_IMPL_XSERVICEINFO( SfxStandaloneDocumentInfoObject, "com.sun.star.document.BinaryStandaloneDocumentInfo", "com.sun.star.comp.sfx2.BinaryStandaloneDocumentInfo" )
/*N*/ SFX_IMPL_SINGLEFACTORY( SfxStandaloneDocumentInfoObject )
/*N*/
/*N*/ SfxStandaloneDocumentInfoObject::SfxStandaloneDocumentInfoObject( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xFactory )
/*N*/ 	: SfxDocumentInfoObject( sal_True )
/*N*/ 	, _pMedium( NULL )
/*N*/     , _xFactory( xFactory )
/*N*/ {
/*N*/ }

//-----------------------------------------------------------------------------

/*N*/ SfxStandaloneDocumentInfoObject::~SfxStandaloneDocumentInfoObject()
/*N*/ {
/*N*/ 	delete _pMedium;
/*N*/ }

//-----------------------------------------------------------------------------

/*N*/ SvStorage* SfxStandaloneDocumentInfoObject::GetStorage_Impl( const String& rName, sal_Bool bWrite )
/*N*/ {
/*N*/ 	// Medium erstellen
/*N*/ 	if ( _pMedium )
/*N*/ 		delete _pMedium;
/*N*/
/*N*/     _pMedium = new SfxMedium( rName, bWrite ? SFX_STREAM_READWRITE : SFX_STREAM_READONLY, sal_True );
/*N*/     if ( !_pMedium->GetStorage() || SVSTREAM_OK != _pMedium->GetError() )
/*N*/ 		// Datei existiert nicht oder ist kein Storage
/*N*/ 		return NULL;
/*N*/
/*N*/ 	// Filter-Detection wegen FileFormat-Version
/*N*/ 	_pFilter = 0;
/*N*/ 	if ( 0 != SFX_APP()->GetFilterMatcher().GuessFilter( *_pMedium, &_pFilter )
/*N*/ 			|| !bWrite && !_pFilter )
/*N*/ 		// unbekanntes Dateiformat
/*N*/ 		return NULL;
/*N*/
/*N*/ 	// Storage "offnen
/*N*/ 	SvStorageRef xStor = _pMedium->GetStorage();
/*N*/ 	DBG_ASSERT( xStor.Is(), "no storage" );
/*N*/     xStor->SetVersion( _pFilter ? _pFilter->GetVersion() : SOFFICE_FILEFORMAT_CURRENT );
/*N*/ 	return xStor;
/*N*/ }

//-----------------------------------------------------------------------------

/*N*/ sal_Int16 SAL_CALL SfxStandaloneDocumentInfoObject::getUserFieldCount() throw( ::com::sun::star::uno::RuntimeException )
/*N*/ {
/*N*/ 	return SfxDocumentInfoObject::getUserFieldCount();
/*N*/ }

//-----------------------------------------------------------------------------

/*N*/ ::rtl::OUString SAL_CALL  SfxStandaloneDocumentInfoObject::getUserFieldName(sal_Int16 nIndex) throw( ::com::sun::star::uno::RuntimeException )
/*N*/ {
/*N*/ 	return SfxDocumentInfoObject::getUserFieldName(nIndex);
/*N*/ }

//-----------------------------------------------------------------------------

/*N*/ ::rtl::OUString SAL_CALL  SfxStandaloneDocumentInfoObject::getUserFieldValue(sal_Int16 nIndex) throw( ::com::sun::star::uno::RuntimeException )
/*N*/ {
/*N*/ 	return SfxDocumentInfoObject::getUserFieldValue(nIndex);
/*N*/ }

//-----------------------------------------------------------------------------

/*N*/ void  SAL_CALL SfxStandaloneDocumentInfoObject::setUserFieldName(sal_Int16 nIndex, const ::rtl::OUString& aName ) throw( ::com::sun::star::uno::RuntimeException )
/*N*/ {
/*N*/ 	SfxDocumentInfoObject::setUserFieldName( nIndex, aName );
/*N*/ }

//-----------------------------------------------------------------------------

/*N*/ void SAL_CALL  SfxStandaloneDocumentInfoObject::setUserFieldValue( sal_Int16 nIndex, const ::rtl::OUString& aValue ) throw( ::com::sun::star::uno::RuntimeException )
/*N*/ {
/*N*/ 	SfxDocumentInfoObject::setUserFieldValue( nIndex, aValue );
/*N*/ }
//-----------------------------------------------------------------------------

/*N*/ void SAL_CALL  SfxStandaloneDocumentInfoObject::loadFromURL(const ::rtl::OUString& aURL) throw( ::com::sun::star::io::IOException )
/*N*/ {
/*N*/ 	SolarMutexGuard aGuard;
/*N*/ 	sal_Bool bOK = sal_False;
/*N*/ 	String aName( aURL );
/*N*/     SvStorage* pStorage = GetStorage_Impl( aName, sal_False );
/*N*/     if ( pStorage )
/*N*/ 	{
/*N*/         if ( pStorage->GetVersion() >= SOFFICE_FILEFORMAT_60 )
/*N*/         {
/*N*/             // import from XML meta data using SAX parser
/*N*/             uno::Reference< XInterface > xXMLParser = _xFactory->createInstance(
/*N*/                             ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.xml.sax.Parser" )) );
/*N*/             if( xXMLParser.is() )
/*N*/             {
/*N*/                 // create input source for SAX parser
/*N*/                 xml::sax::InputSource aParserInput;
/*N*/                 aParserInput.sSystemId = aURL;
/*N*/
/*N*/                 SvStorageStreamRef xDocStream;
/*N*/                 String sDocName( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("meta.xml")) );
/*N*/                 if ( pStorage->IsStream(sDocName) )
/*N*/                     xDocStream = pStorage->OpenStream( sDocName, STREAM_READ | STREAM_NOCREATE );
/*N*/                 xDocStream->SetBufferSize( 16*1024 );
/*N*/                 aParserInput.aInputStream = new ::utl::OInputStreamWrapper( *xDocStream );
/*N*/
/*N*/                 // create importer service
/*N*/                 Reference < xml::sax::XDocumentHandler > xDocHandler( _xFactory->createInstanceWithArguments(
/*N*/                         ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.document.XMLMetaImporter")),
/*N*/                         Sequence < Any >() ), UNO_QUERY );
/*N*/
/*N*/                 // connect importer with this object
/*N*/                 Reference < document::XImporter > xImporter( xDocHandler, UNO_QUERY );
/*N*/                 if ( xImporter.is() )
/*N*/                     xImporter->setTargetDocument( this );
/*N*/
/*N*/                 // connect parser and filter
/*N*/                 Reference < xml::sax::XParser > xParser( xXMLParser, UNO_QUERY );
/*N*/                 xParser->setDocumentHandler( xDocHandler );
/*N*/
/*N*/                 // parse
/*N*/                 try
/*N*/                 {
/*N*/                     xParser->parseStream( aParserInput );
/*N*/                     bOK = sal_True;
/*N*/                 }
/*N*/                 catch( ::com::sun::star::uno::Exception& )
/*N*/                 {
/*N*/                 }
/*N*/             }
/*N*/         }
/*N*/         else
/*N*/             bOK = _pInfo->Load( pStorage );
/*N*/     }
/*N*/
/*N*/ 	DELETEZ( _pMedium );
/*N*/ 	if ( !bOK )
/*N*/ 		throw SfxIOException_Impl( ERRCODE_IO_CANTREAD );
/*N*/ }

//-----------------------------------------------------------------------------

/*N*/ void SAL_CALL  SfxStandaloneDocumentInfoObject::storeIntoURL(const ::rtl::OUString& aURL) throw( ::com::sun::star::io::IOException )
/*N*/ {
/*N*/ 	SolarMutexGuard aGuard;
/*N*/ 	sal_Bool bOK = sal_False;
/*N*/ 	String aName( aURL );
/*N*/ 	SvStorage* pStor = GetStorage_Impl( aName, sal_True );
/*N*/ 	if ( pStor )
/*N*/ 	{
/*N*/ 		// DocInfo speichern
/*N*/ 		bOK = _pInfo->Save( pStor ) && pStor->Commit();
/*N*/ 	}
/*N*/
/*N*/ 	DELETEZ( _pMedium );
/*N*/ 	if ( !bOK )
/*N*/ 		throw SfxIOException_Impl( ERRCODE_IO_CANTREAD );
/*N*/ }

Reference< XInterface > SAL_CALL bf_BinaryDocInfo_createInstance(const Reference< com::sun::star::lang::XMultiServiceFactory > & rSMgr)
    throw( Exception )
{
    static ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > mxLegServFact;
    if ( !mxLegServFact.is() )
    {
        mxLegServFact = ::legacy_binfilters::getLegacyProcessServiceFactory();
        ::com::sun::star::uno::Reference < com::sun::star::lang::XComponent > xWrapper( mxLegServFact->createInstance(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.office.OfficeWrapper" ))), UNO_QUERY );
    }

    return (cppu::OWeakObject*)new SfxStandaloneDocumentInfoObject(rSMgr);
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
//	::com::sun::star::uno::Reference< ::com::sun::star::document::XDocumentInfo >  xInfo = (::com::sun::star::document::XDocumentInfo*) xSSI->newInstance()->queryInterface( ::getCppuType((const Reference< ::com::sun::star::document::XDocumentInfo >*)0) );

// automatisch auskommentiert - [getIdlClass(es) or queryInterface] - Bitte XTypeProvider benutzen!
//	::com::sun::star::uno::Reference< ::com::sun::star::beans::XFastPropertySet >  xProps = (::com::sun::star::beans::XFastPropertySet*) xInfo->queryInterface( ::getCppuType((const Reference< ::com::sun::star::beans::XFastPropertySet >*)0) );


    // Laden und Werte auslesen
    xInfo->load( StringToOUString( aURL, CHARSET_SYSTEM ) );
    ::com::sun::star::uno::Any aValue = xProps->getFastPropertyValue( WID_FROM );
    String aAuthor = OUStringToString( aValue.getString(), CHARSET_SYSTEM );
#endif
*/


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
