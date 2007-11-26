/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: docinf.cxx,v $
 *
 *  $Revision: 1.52 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-26 15:34:35 $
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
#include "precompiled_sfx2.hxx"

#include <sfx2/docinf.hxx>

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyContainer.hpp>
#include <com/sun/star/beans/XFastPropertySet.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <com/sun/star/document/XDocumentInfoSupplier.hpp>
#include <com/sun/star/uno/Exception.hpp>
#include <comphelper/processfactory.hxx>

#include <tools/urlobj.hxx>
#include <svtools/saveopt.hxx>
#include <svtools/useroptions.hxx>
#include <sot/storage.hxx>
#include <unotools/localfilehelper.hxx>
#include <svtools/cntwids.hrc>
#include <cppuhelper/implbase1.hxx>

#include <sfx2/sfx.hrc>
#include <sfx2/sfxsids.hrc>
#include "oleprops.hxx"
#include <sfx2/docfilt.hxx>
#include <sfx2/fcontnr.hxx>
#include "openflag.hxx"
#include <sfx2/app.hxx>
#include <sfx2/docinf.hxx>
#include <sfx2/docfile.hxx>
#include "graphhelp.hxx"
#include "sfxtypes.hxx"
#include "appdata.hxx"
#include <sfx2/doctempl.hxx>
#include <sfx2/objsh.hxx>
#include "sfxresid.hxx"
#include "doc.hrc"
#include <sfx2/objuno.hxx>

// ============================================================================

#define SFXDOCINFO_TITLELENMAX      63
#define SFXDOCINFO_THEMELENMAX      63
#define SFXDOCINFO_COMMENTLENMAX    255
#define SFXDOCINFO_KEYWORDLENMAX    127

#define NR_BUILTIN_PROPS 23

// stream names
#define STREAM_SUMMARYINFO      "\005SummaryInformation"
#define STREAM_DOCSUMMARYINFO   "\005DocumentSummaryInformation"

// usings
using namespace ::com::sun::star;

::com::sun::star::util::DateTime impl_DateTime_Object2Struct( const ::DateTime& aDateTimeObject )
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

::DateTime impl_DateTime_Struct2Object ( const ::com::sun::star::util::DateTime& aDateTimeStruct )
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
// ============================================================================

IMPL_PTRHINT(SfxDocumentInfoHint, SfxDocumentInfo)

// ============================================================================
// This Listener is used to get notified when the DocumentInfoObject is changed via its API
// If several changes are done from the SfxDocumentInfo it can use the "bQuiet" member to
// suppress notifications for some time.
class SfxDocInfoListener_Impl : public ::cppu::WeakImplHelper1< ::com::sun::star::util::XModifyListener >
{
public:
    SfxObjectShell* mpDoc;
    BOOL bQuiet;
    BOOL bGotModified;
    SfxDocInfoListener_Impl( SfxObjectShell* pDoc )
        : mpDoc(pDoc)
        , bQuiet(FALSE)
    {
    };
    virtual void SAL_CALL disposing( const lang::EventObject& aEvent ) throw ( uno::RuntimeException ) ;
    virtual void SAL_CALL modified( const lang::EventObject& ) throw (uno::RuntimeException );
};

void SAL_CALL SfxDocInfoListener_Impl::modified( const lang::EventObject& )
        throw ( uno::RuntimeException )
{
    bGotModified = TRUE;

    // notifies changes to the SfxObjectShell owning the SfxDocumentInfo
    if ( mpDoc && !bQuiet )
        mpDoc->FlushDocInfo();
}

void SAL_CALL SfxDocInfoListener_Impl::disposing( const lang::EventObject& ) throw ( uno::RuntimeException )
{
}

// ============================================================================

class SfxDocumentInfo_Impl
{
public:
    uno::Reference < document::XDocumentInfo >  xDocInfo;   // the DocumentInfo
    uno::Reference < beans::XPropertySet >      xSet;       // for convenience: other interfaces of it
    uno::Reference < beans::XFastPropertySet >  xFastSet;   // can be removed when a multiple inheritance interface is available
    uno::Reference < util::XModifyListener >    xListener;  // controls the lifetime of the ModifyListener
    SfxDocInfoListener_Impl*                    pListener;  // MofifyListener registered at DocumentInfo

    SfxDocumentInfo_Impl()
        : pListener(0)
    {
    }

    ~SfxDocumentInfo_Impl()
    {
        if ( pListener )
            // Listener must be notified that the document might get destroyed
            pListener->mpDoc=0;
    }
};

// ============================================================================

SfxDocumentInfo::SfxDocumentInfo( SfxObjectShell* pDoc )
{
    pImp = new SfxDocumentInfo_Impl;

    if ( pDoc )
    {
        uno::Reference < document::XDocumentInfoSupplier > xModel( pDoc->GetModel(), uno::UNO_QUERY );
        if ( xModel.is() )
        {
            pImp->xDocInfo = xModel->getDocumentInfo();
            pImp->xSet = uno::Reference < beans::XPropertySet> ( pImp->xDocInfo, uno::UNO_QUERY );
            pImp->xFastSet = uno::Reference < beans::XFastPropertySet> ( pImp->xDocInfo, uno::UNO_QUERY );
            uno::Reference< beans::XPropertySet > xStorProps( pDoc->GetStorage(), uno::UNO_QUERY_THROW );
            ::rtl::OUString aMediaType;
            xStorProps->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "MediaType" ) ) ) >>= aMediaType;
            SetMimeType( aMediaType );
            pImp->pListener = new SfxDocInfoListener_Impl( pDoc );
            pImp->xListener = pImp->pListener;
            uno::Reference < util::XModifyBroadcaster > xMod( pImp->xDocInfo, uno::UNO_QUERY );
            xMod->addModifyListener( pImp->xListener );
        }
    }

    if ( !pImp->xDocInfo.is() )
    {
        pImp->xDocInfo = new SfxDocumentInfoObject;
            /*uno::Reference < document::XDocumentInfo > ( ::comphelper::getProcessServiceFactory()->createInstance(
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.document.DocumentInfo" ))), uno::UNO_QUERY );*/
        pImp->xSet = uno::Reference < beans::XPropertySet> ( pImp->xDocInfo, uno::UNO_QUERY );
        pImp->xFastSet = uno::Reference < beans::XFastPropertySet> ( pImp->xDocInfo, uno::UNO_QUERY );
    }
}

SfxDocumentInfo::SfxDocumentInfo( const uno::Reference < document::XDocumentInfo >& rInfo )
{
    pImp = new SfxDocumentInfo_Impl;
    pImp->xDocInfo = rInfo;
    pImp->xSet = uno::Reference < beans::XPropertySet> ( pImp->xDocInfo, uno::UNO_QUERY );
    pImp->xFastSet = uno::Reference < beans::XFastPropertySet> ( pImp->xDocInfo, uno::UNO_QUERY );
}

SfxDocumentInfo::SfxDocumentInfo( const SfxDocumentInfo& rInfo )
{
    pImp = new SfxDocumentInfo_Impl;
    pImp->xDocInfo = new SfxDocumentInfoObject;
        /*uno::Reference < document::XDocumentInfo > ( ::comphelper::getProcessServiceFactory()->createInstance(
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.document.DocumentInfo" ))), uno::UNO_QUERY );*/
    pImp->xSet = uno::Reference < beans::XPropertySet> ( pImp->xDocInfo, uno::UNO_QUERY );
    pImp->xFastSet = uno::Reference < beans::XFastPropertySet> ( pImp->xDocInfo, uno::UNO_QUERY );
    (*this) = rInfo;
}

SfxDocumentInfo::~SfxDocumentInfo()
{
    delete pImp;
}

uno::Reference < document::XDocumentInfo > SfxDocumentInfo::GetInfo() const
{
    return pImp->xDocInfo;
}

//-------------------------------------------------------------------------

sal_uInt32 SfxDocumentInfo::LoadPropertySet( SotStorage* pStorage )
{
    // *** global properties from stream "005SummaryInformation" ***

    // load the property set
    SfxOlePropertySet aGlobSet;
    ErrCode nGlobError = aGlobSet.LoadPropertySet(
        pStorage, String( RTL_CONSTASCII_USTRINGPARAM( STREAM_SUMMARYINFO ) ) );

    // global section
    SfxOleSectionRef xGlobSect = aGlobSet.GetSection( SECTION_GLOBAL );
    if( xGlobSect.get() )
    {
        // set supported properties
        String aStrValue;
        DateTime aDateTime;

        if( xGlobSect->GetStringValue( aStrValue, PROPID_TITLE ) )
            SetTitle( aStrValue );
        if( xGlobSect->GetStringValue( aStrValue, PROPID_SUBJECT ) )
            SetTheme( aStrValue );
        if( xGlobSect->GetStringValue( aStrValue, PROPID_KEYWORDS ) )
            SetKeywords( aStrValue );
        if( xGlobSect->GetStringValue( aStrValue, PROPID_TEMPLATE ) )
            SetTemplateName( aStrValue );
        if( xGlobSect->GetStringValue( aStrValue, PROPID_COMMENTS ) )
            SetComment( aStrValue );

        DateTime aInvalid(0,0);
        if( xGlobSect->GetStringValue( aStrValue, PROPID_AUTHOR) )
            SetAuthor( aStrValue );
        else
            SetAuthor( String() );
        if( xGlobSect->GetFileTimeValue( aDateTime, PROPID_CREATED ) )
            SetCreationDate( aDateTime );
        else
            SetCreationDate( aInvalid );

        if( xGlobSect->GetStringValue( aStrValue, PROPID_LASTAUTHOR) )
            SetModificationAuthor( aStrValue );
        else
            SetModificationAuthor( String() );
        if( xGlobSect->GetFileTimeValue( aDateTime, PROPID_LASTSAVED ) )
            SetModificationDate( aDateTime );
        else
            SetModificationDate( aInvalid );

        SetPrintedBy( String() );
        if( xGlobSect->GetFileTimeValue( aDateTime, PROPID_LASTPRINTED ) )
            SetPrintDate( aDateTime );
        else
            SetPrintDate( aInvalid );

        if( xGlobSect->GetStringValue( aStrValue, PROPID_REVNUMBER ) )
            SetDocumentNumber( static_cast< USHORT >( aStrValue.ToInt32() ) );

        // TODO/REFACTOR: unclear how to code this
        if( xGlobSect->GetFileTimeValue( aDateTime, PROPID_EDITTIME ) )
        {
            aDateTime.ConvertToUTC();
            SetTime( aDateTime.GetTime() );
        }
    }

    // *** custom properties from stream "005DocumentSummaryInformation" ***

    // load the property set
    SfxOlePropertySet aDocSet;
    ErrCode nDocError = aDocSet.LoadPropertySet(
        pStorage, String( RTL_CONSTASCII_USTRINGPARAM( STREAM_DOCSUMMARYINFO ) ) );

    // custom properties
    SfxOleSectionRef xCustomSect = aDocSet.GetSection( SECTION_CUSTOM );
    if( xCustomSect.get() )
    {
        ::std::vector< sal_Int32 > aPropIds;
        xCustomSect->GetPropertyIds( aPropIds );
        for( ::std::vector< sal_Int32 >::const_iterator aIt = aPropIds.begin(), aEnd = aPropIds.end(); aIt != aEnd; ++aIt )
        {
            ::rtl::OUString aPropName = xCustomSect->GetPropertyName( *aIt );
            uno::Any aPropValue = xCustomSect->GetAnyValue( *aIt );
            if( (aPropName.getLength() > 0) && aPropValue.hasValue() )
                InsertCustomProperty( aPropName, aPropValue );
        }
    }

    // return code
    return (nGlobError != ERRCODE_NONE) ? nGlobError : nDocError;
}

BOOL SfxDocumentInfo::SavePropertySet( SotStorage* pStorage, GDIMetaFile* pThumb ) const
{
    // *** global properties into stream "005SummaryInformation" ***

    SfxOlePropertySet aGlobSet;

    // set supported properties
    SfxOleSection& rGlobSect = aGlobSet.AddSection( SECTION_GLOBAL );
    rGlobSect.SetStringValue( PROPID_TITLE, GetTitle() );
    rGlobSect.SetStringValue( PROPID_SUBJECT, GetTheme() );
    rGlobSect.SetStringValue( PROPID_KEYWORDS, GetKeywords() );
    rGlobSect.SetStringValue( PROPID_TEMPLATE, GetTemplateName() );
    rGlobSect.SetStringValue( PROPID_COMMENTS, GetComment() );
    rGlobSect.SetStringValue( PROPID_AUTHOR, GetAuthor() );
    rGlobSect.SetFileTimeValue( PROPID_CREATED, GetCreationDate() );
    rGlobSect.SetStringValue( PROPID_LASTAUTHOR, GetModificationAuthor() );
    rGlobSect.SetFileTimeValue( PROPID_LASTSAVED, GetModificationDate() );
    rGlobSect.SetFileTimeValue( PROPID_LASTPRINTED, GetPrintDate() );

    // TODO/REFACTOR: unclear how to code this
    DateTime aEditTime( Date( 0 ), GetTime() );
    aEditTime.ConvertToLocalTime();
    rGlobSect.SetFileTimeValue( PROPID_EDITTIME, aEditTime );

    rGlobSect.SetStringValue( PROPID_REVNUMBER, String::CreateFromInt32( GetDocumentNumber() ) );
    if ( pThumb )
        rGlobSect.SetThumbnailValue( PROPID_THUMBNAIL, *pThumb );

    // save the property set
    ErrCode nGlobError = aGlobSet.SavePropertySet(
        pStorage, String( RTL_CONSTASCII_USTRINGPARAM( STREAM_SUMMARYINFO ) ) );

    // *** custom properties into stream "005DocumentSummaryInformation" ***

    SfxOlePropertySet aDocSet;

    // set builtin properties
    aDocSet.AddSection( SECTION_BUILTIN );

    // set custom properties
    SfxOleSection& rCustomSect = aDocSet.AddSection( SECTION_CUSTOM );
    uno::Reference< beans::XPropertySetInfo > xSetInfo = pImp->xSet->getPropertySetInfo();
    const uno::Sequence< beans::Property >          lProps   = xSetInfo->getProperties();
    const beans::Property*                          pProps   = lProps.getConstArray();
          sal_Int32                                 c        = lProps.getLength();
          sal_Int32                                 i        = 0;
    for (i=0; i<c; ++i)
    {
        // "fix" property ? => ignore it !
        if ( !(pProps[i].Attributes & ::com::sun::star::beans::PropertyAttribute::REMOVABLE) )
            continue;

        // "dynamic" prop => export it
        sal_Int32 nPropId = rCustomSect.GetFreePropertyId();
        uno::Any aValue = pImp->xSet->getPropertyValue(pProps[i].Name);
        if( rCustomSect.SetAnyValue( nPropId, aValue ) )
            rCustomSect.SetPropertyName( nPropId, pProps[i].Name );
    }

    // save the property set
    ErrCode nDocError = aDocSet.SavePropertySet(
        pStorage, String( RTL_CONSTASCII_USTRINGPARAM( STREAM_DOCSUMMARYINFO ) ) );

    // return code
    return (nGlobError == ERRCODE_NONE) && (nDocError == ERRCODE_NONE);
}

//-------------------------------------------------------------------------
const SfxDocumentInfo& SfxDocumentInfo::operator=( const SfxDocumentInfo& rInf)
{
    if( pImp->xSet == rInf.pImp->xSet )
        return *this;

    if ( pImp->pListener )
    {
        pImp->pListener->bQuiet = TRUE;
        pImp->pListener->bGotModified = FALSE;
    }

    try
    {
        uno::Reference< beans::XPropertyContainer > xContainer( pImp->xSet, uno::UNO_QUERY );
        uno::Reference< beans::XPropertySetInfo > xSetInfo = pImp->xSet->getPropertySetInfo();
        uno::Sequence< beans::Property > lProps = xSetInfo->getProperties();
        const beans::Property* pProps = lProps.getConstArray();
        sal_Int32 c = lProps.getLength();
        sal_Int32 i = 0;
        for (i=0; i<c; ++i)
        {
            if ( pProps[i].Attributes & ::com::sun::star::beans::PropertyAttribute::REMOVABLE )
                xContainer->removeProperty( pProps[i].Name );
        }

        xSetInfo = rInf.pImp->xSet->getPropertySetInfo();
        lProps = xSetInfo->getProperties();
        pProps = lProps.getConstArray();
        c = lProps.getLength();
        for (i=0; i<c; ++i)
        {
            uno::Any aValue = rInf.pImp->xSet->getPropertyValue( pProps[i].Name );
            if ( pProps[i].Attributes & ::com::sun::star::beans::PropertyAttribute::REMOVABLE )
                // QUESTION: DefaultValue?!
                xContainer->addProperty( pProps[i].Name, pProps[i].Attributes, aValue );
            pImp->xSet->setPropertyValue( pProps[i].Name, aValue );
        }
    }
    catch ( uno::Exception& ) {}

    for(USHORT n=0; n<MAXDOCUSERKEYS; ++n)
        SetUserKey( rInf.GetUserKeyTitle(n), rInf.GetUserKeyWord(n), n );

    if ( pImp->pListener )
    {
        pImp->pListener->bQuiet = FALSE;
        if ( pImp->pListener->bGotModified )
            pImp->pListener->mpDoc->FlushDocInfo();
        pImp->pListener->bGotModified = FALSE;
    }

    return *this;
}

//-------------------------------------------------------------------------
// TODO/REFACTOR: get rid of that operator due to performance reasons
int SfxDocumentInfo::operator==( const SfxDocumentInfo& rCmp) const
{
    if( GetAuthor() != rCmp.GetAuthor() ||
        GetModificationAuthor() != rCmp.GetModificationAuthor() ||
        GetPrintedBy() != rCmp.GetPrintedBy() ||
        GetCreationDate() != rCmp.GetCreationDate() ||
        GetModificationDate() != rCmp.GetModificationDate() ||
        GetPrintDate() != rCmp.GetPrintDate() ||
        GetTitle() != rCmp.GetTitle() ||
        GetTheme() != rCmp.GetTheme() ||
        GetComment() != rCmp.GetComment() ||
        GetKeywords() != rCmp.GetKeywords() ||
        GetTemplateName() != rCmp.GetTemplateName() ||
        GetTemplateDate() != rCmp.GetTemplateDate() ||
       IsReloadEnabled() != rCmp.IsReloadEnabled() ||
       GetReloadURL() != rCmp.GetReloadURL() ||
       GetReloadDelay() != rCmp.GetReloadDelay() ||
       GetDefaultTarget() != rCmp.GetDefaultTarget())
        return FALSE;

    for(USHORT i = 0; i < MAXDOCUSERKEYS; ++i) {
        if(GetUserKeyTitle(i) != rCmp.GetUserKeyTitle(i) || GetUserKeyWord(i) != rCmp.GetUserKeyWord(i))
            return FALSE;
    }

    return TRUE;
}

//-------------------------------------------------------------------------
void SfxDocumentInfo::SetUserKeyWord( const String& rValue, USHORT n )
{
    DBG_ASSERT( n < GetUserKeyCount(), "user key index overflow" );
    if ( n<GetUserKeyCount())
    {
        ::rtl::OUString aValue( rValue );
        pImp->xDocInfo->setUserFieldValue(n, aValue);
    }
}

void SfxDocumentInfo::SetUserKeyTitle( const String& rName, USHORT n )
{
    DBG_ASSERT( n < GetUserKeyCount(), "user key index overflow" );
    if ( n<GetUserKeyCount())
    {
        ::rtl::OUString aName( rName );
        pImp->xDocInfo->setUserFieldName(n, aName);
    }
}

void SfxDocumentInfo::SetUserKey( const String& rName, const String& rValue, USHORT n )
{
    SetUserKeyTitle( rName, n );
    SetUserKeyWord( rValue, n );
}

USHORT SfxDocumentInfo::GetUserKeyCount() const
{
    return pImp->xDocInfo->getUserFieldCount();
}

String SfxDocumentInfo::GetUserKeyTitle(USHORT n) const
{
    DBG_ASSERT( n < GetUserKeyCount(), "user key index overflow" );
    return n<GetUserKeyCount() ? String(pImp->xDocInfo->getUserFieldName(n)) : String();
}

String SfxDocumentInfo::GetUserKeyWord(USHORT n) const
{
    return n<GetUserKeyCount() ? String(pImp->xDocInfo->getUserFieldValue(n)) : String();
}

BOOL SfxDocumentInfo::InsertCustomProperty(const ::rtl::OUString& aPropertyName, const uno::Any& aValue)
{
    /*SfxExtendedItemPropertyMap aProp;
    aProp.pName    = 0; // superflous -> held as hash key.
    aProp.nNameLen = 0;
    aProp.nFlags   = ::com::sun::star::beans::PropertyAttribute::REMOVEABLE;
    aProp.aValue   = aValue;
    aProp.nWID     = 0xFFFF;
    pImp->_lDynamicProps[aPropertyName] = aProp;*/
    try
    {
        uno::Reference < beans::XPropertyContainer > xCont( pImp->xDocInfo, uno::UNO_QUERY );
        xCont->addProperty( aPropertyName, ::com::sun::star::beans::PropertyAttribute::REMOVEABLE, aValue );
    }
    catch ( uno::Exception& )
    {
        return FALSE;
    }

    return TRUE;
}

//-------------------------------------------------------------------------
BOOL SfxDocumentInfo::BOOL_PROP(sal_Int32 nHandle) const
{
    sal_Bool bRet=sal_False;
    try
    {
        uno::Any aAny = pImp->xFastSet->getFastPropertyValue( nHandle );
        aAny >>= bRet;
    }
    catch ( uno::Exception& ) {}
    return bRet;
}

sal_Int32 SfxDocumentInfo::INT_PROP(sal_Int32 nHandle) const
{
    sal_Int32 nRet=sal_False;
    try
    {
        uno::Any aAny = pImp->xFastSet->getFastPropertyValue( nHandle );
        aAny >>= nRet;
    }
    catch ( uno::Exception& ) {}
    return nRet;
}

String SfxDocumentInfo::STRING_PROP(sal_Int32 nHandle) const
{
    ::rtl::OUString aRet;
    try
    {
        uno::Any aAny = pImp->xFastSet->getFastPropertyValue( nHandle );
        aAny >>= aRet;
    }
    catch ( uno::Exception& ) {}
    return aRet;
}

DateTime SfxDocumentInfo::DATETIME_PROP(sal_Int32 nHandle) const
{
    util::DateTime aRet;
    try
    {
        uno::Any aAny = pImp->xFastSet->getFastPropertyValue( nHandle );
        aAny >>= aRet;
    }
    catch ( uno::Exception& ) {}
    return impl_DateTime_Struct2Object( aRet );
}

void SfxDocumentInfo::SET_PROP(sal_Int32 nHandle, const uno::Any& aValue )
{
    try
    {
        pImp->xFastSet->setFastPropertyValue( nHandle, aValue );
    }
    catch ( uno::Exception& ) {}
}

//-------------------------------------------------------------------------
BOOL SfxDocumentInfo::IsReloadEnabled() const
{
    return BOOL_PROP(MID_DOCINFO_AUTOLOADENABLED);
}

sal_Int16 SfxDocumentInfo::GetDocumentNumber() const
{
    return sal_Int16( INT_PROP(MID_DOCINFO_REVISION) );
    //return IsUseUserData() ? sal_Int16( INT_PROP(MID_DOCINFO_REVISION) ) : sal_Int16(0);
}

sal_Int32 SfxDocumentInfo::GetTime() const
{
    return INT_PROP(MID_DOCINFO_EDITTIME);
    //return IsUseUserData() ? INT_PROP(MID_DOCINFO_EDITTIME) : 0;
}

sal_Int32 SfxDocumentInfo::GetReloadDelay() const
{
    return INT_PROP(MID_DOCINFO_AUTOLOADSECS);
    //return IsUseUserData() ? INT_PROP(MID_DOCINFO_AUTOLOADSECS) : 0;
}

String SfxDocumentInfo::GetDefaultTarget() const
{
    return STRING_PROP(MID_DOCINFO_DEFAULTTARGET);
}

String SfxDocumentInfo::GetReloadURL() const
{
    return STRING_PROP(MID_DOCINFO_AUTOLOADURL);
}

String SfxDocumentInfo::GetComment() const
{
    return STRING_PROP(MID_DOCINFO_DESCRIPTION);
}

String SfxDocumentInfo::GetKeywords() const
{
    return STRING_PROP(WID_KEYWORDS);
}

String SfxDocumentInfo::GetMimeType() const
{
    return STRING_PROP(WID_CONTENT_TYPE);
}

String SfxDocumentInfo::GetTheme() const
{
    return STRING_PROP(MID_DOCINFO_SUBJECT);
}

String SfxDocumentInfo::GetTemplateName() const
{
    return STRING_PROP(MID_DOCINFO_TEMPLATE);
}

String SfxDocumentInfo::GetTemplateFileName() const
{
    return STRING_PROP(SID_TEMPLATE_NAME);
}

String SfxDocumentInfo::GetTitle() const
{
    return STRING_PROP(WID_TITLE);
}

String SfxDocumentInfo::GetAuthor() const
{
    return STRING_PROP(WID_FROM);
}

String SfxDocumentInfo::GetModificationAuthor() const
{
    return STRING_PROP(MID_DOCINFO_MODIFICATIONAUTHOR);
}

String SfxDocumentInfo::GetPrintedBy() const
{
    return STRING_PROP(MID_DOCINFO_PRINTEDBY);
}

DateTime SfxDocumentInfo::GetTemplateDate() const
{
    return DATETIME_PROP(MID_DOCINFO_TEMPLATEDATE);
}

DateTime SfxDocumentInfo::GetCreationDate() const
{
    return DATETIME_PROP(WID_DATE_CREATED);
}

DateTime SfxDocumentInfo::GetModificationDate() const
{
    return DATETIME_PROP(WID_DATE_MODIFIED);
}

DateTime SfxDocumentInfo::GetPrintDate() const
{
    return DATETIME_PROP(MID_DOCINFO_PRINTDATE);
}

void SfxDocumentInfo::EnableReload( BOOL bEnable )
{
    uno::Any aAny;
    aAny <<= (sal_Bool) bEnable;
    SET_PROP( MID_DOCINFO_AUTOLOADENABLED, aAny );
}

void SfxDocumentInfo::SetDocumentNumber(sal_Int16 nNo)
{
    uno::Any aAny;
    aAny <<= nNo;
    SET_PROP( MID_DOCINFO_REVISION, aAny );
}

void SfxDocumentInfo::IncDocumentNumber()
{
    uno::Any aAny;
    sal_Int16 nNo=GetDocumentNumber()+1;
    aAny <<= nNo;
    SET_PROP( MID_DOCINFO_REVISION, aAny );
}

void SfxDocumentInfo::SetTime(sal_Int32 nTime)
{
    uno::Any aAny;
    aAny <<= nTime;
    SET_PROP( MID_DOCINFO_EDITTIME, aAny );
}

void SfxDocumentInfo::SetReloadDelay( sal_Int32 nSec )
{
    uno::Any aAny;
    aAny <<= nSec;
    SET_PROP( MID_DOCINFO_AUTOLOADSECS, aAny );
}

void SfxDocumentInfo::SetTemplateName( const String& rName )
{
    uno::Any aAny;
    aAny <<= ::rtl::OUString( rName );
    SET_PROP( MID_DOCINFO_TEMPLATE, aAny );
    //SetQueryLoadTemplate( sal_True );
}

void SfxDocumentInfo::SetTemplateFileName( const String& rName )
{
    uno::Any aAny;
    aAny <<= ::rtl::OUString( rName );
    SET_PROP( SID_TEMPLATE_NAME, aAny );
    //SetQueryLoadTemplate( sal_True );
}

void SfxDocumentInfo::SetMimeType( const String& rName )
{
    uno::Any aAny;
    aAny <<= ::rtl::OUString( rName );
    SET_PROP( WID_CONTENT_TYPE, aAny );
}

void SfxDocumentInfo::SetAuthor( const String& rAuthor )
{
    uno::Any aAny;
    aAny <<= ::rtl::OUString( rAuthor );
    SET_PROP( WID_FROM, aAny );
}

void SfxDocumentInfo::SetCreationDate( const DateTime& rTime )
{
    uno::Any aAny;
    aAny <<= impl_DateTime_Object2Struct( rTime );
    SET_PROP( WID_DATE_CREATED, aAny );
}

void SfxDocumentInfo::SetCreated( const String& rCreator )
{
    uno::Any aAny;
    aAny <<= rtl::OUString( rCreator );
    SET_PROP( WID_FROM, aAny );
    aAny <<= impl_DateTime_Object2Struct( DateTime() );
    SET_PROP( WID_DATE_CREATED, aAny );
}

void SfxDocumentInfo::SetModificationAuthor( const String& rName )
{
    uno::Any aAny;
    aAny <<= rtl::OUString( rName );
    SET_PROP( MID_DOCINFO_MODIFICATIONAUTHOR, aAny );
}

void SfxDocumentInfo::SetModificationDate( const DateTime& rTime )
{
    uno::Any aAny;
    aAny <<= impl_DateTime_Object2Struct( rTime );
    SET_PROP( WID_DATE_MODIFIED, aAny );
}

void SfxDocumentInfo::SetChanged( const String& rChanger )
{
    uno::Any aAny;
    aAny <<= ::rtl::OUString( rChanger );
    SET_PROP( MID_DOCINFO_MODIFICATIONAUTHOR, aAny );
    aAny <<= impl_DateTime_Object2Struct( DateTime() );
    SET_PROP( WID_DATE_MODIFIED, aAny );
}

void SfxDocumentInfo::SetPrintDate( const DateTime& rTime )
{
    uno::Any aAny;
    aAny <<= impl_DateTime_Object2Struct( rTime );
    SET_PROP( MID_DOCINFO_PRINTDATE, aAny );
}

void SfxDocumentInfo::SetPrintedBy( const String& rName )
{
    uno::Any aAny;
    aAny <<= ::rtl::OUString( rName );
    SET_PROP( MID_DOCINFO_PRINTEDBY, aAny );
}

void SfxDocumentInfo::SetPrinted( const String& rName )
{
    uno::Any aAny;
    aAny <<= ::rtl::OUString( rName );
    SET_PROP( MID_DOCINFO_PRINTEDBY, aAny );
    aAny <<= impl_DateTime_Object2Struct( DateTime() );
    SET_PROP( MID_DOCINFO_PRINTDATE, aAny );
}

void SfxDocumentInfo::SetTemplateDate(const DateTime& rDate)
{
    uno::Any aAny;
    aAny <<= impl_DateTime_Object2Struct( rDate );
    SET_PROP( MID_DOCINFO_TEMPLATEDATE, aAny );
    //SetQueryLoadTemplate( sal_True );
}

void SfxDocumentInfo::SetTitle( const String& rVal )
{
    uno::Any aAny;
    aAny <<= ::rtl::OUString( rVal );
    SET_PROP( WID_TITLE, aAny );
}

void SfxDocumentInfo::SetTheme( const String& rVal )
{
    uno::Any aAny;
    aAny <<= ::rtl::OUString( rVal );
    SET_PROP( MID_DOCINFO_SUBJECT, aAny );
}

void SfxDocumentInfo::SetComment( const String& rVal )
{
    uno::Any aAny;
    aAny <<= ::rtl::OUString( rVal );
    SET_PROP( MID_DOCINFO_DESCRIPTION, aAny );
}

void SfxDocumentInfo::SetKeywords( const String& rVal )
{
    uno::Any aAny;
    aAny <<= ::rtl::OUString( rVal );
    SET_PROP( WID_KEYWORDS, aAny );
}

void SfxDocumentInfo::SetDefaultTarget( const String& rString )
{
    uno::Any aAny;
    aAny <<= ::rtl::OUString( rString );
    SET_PROP( MID_DOCINFO_DEFAULTTARGET, aAny );
}

void SfxDocumentInfo::SetReloadURL( const String& rString )
{
    uno::Any aAny;
    aAny <<= ::rtl::OUString( rString );
    SET_PROP( MID_DOCINFO_AUTOLOADURL, aAny );
}

//-------------------------------------------------------------------------
void SfxDocumentInfo::Clear()
{
    (*this) = SfxDocumentInfo();
}

void SfxDocumentInfo::ClearTemplateInformation()
{
    if ( pImp->pListener )
    {
        pImp->pListener->bQuiet = TRUE;
        pImp->pListener->bGotModified = FALSE;
    }

    SetTemplateName( String() );
    SetTemplateFileName( String() );
    SetTemplateDate( DateTime() );

    if ( pImp->pListener )
    {
        pImp->pListener->bQuiet = FALSE;
        if ( pImp->pListener->bGotModified )
            pImp->pListener->mpDoc->FlushDocInfo();
        pImp->pListener->bGotModified = FALSE;
    }
}

void SfxDocumentInfo::DeleteUserData( const String* pAuthor )
{
    if ( pImp->pListener )
    {
        pImp->pListener->bQuiet = TRUE;
        pImp->pListener->bGotModified = FALSE;
    }

    if ( pAuthor )
    {
        // remove only data pointing to this user
        if ( *pAuthor == STRING_PROP( WID_FROM ) )
            SET_PROP( WID_FROM, uno::makeAny( ::rtl::OUString() ) );

        SetChanged( String() );

        if ( *pAuthor == STRING_PROP( MID_DOCINFO_PRINTEDBY ) )
            SET_PROP( MID_DOCINFO_PRINTEDBY, uno::makeAny( ::rtl::OUString() ) );
    }
    else
    {
        // remove every user data
        DateTime aInvalid(0,0);
        SetAuthor( String() );
        SetModificationAuthor( String() );
        SetPrintedBy( String() );
        SetCreationDate( aInvalid );
        SetModificationDate( aInvalid );
        SetPrintDate( aInvalid );
    }

    SetTime( 0L );
    SetDocumentNumber( 1 );

    if ( pImp->pListener )
    {
        pImp->pListener->bQuiet = FALSE;
        if ( pImp->pListener->bGotModified )
            pImp->pListener->mpDoc->FlushDocInfo();
        pImp->pListener->bGotModified = FALSE;
    }
}

void SfxDocumentInfo::ResetUserData( const String& rAuthor )
{
    if ( pImp->pListener )
    {
        pImp->pListener->bQuiet = TRUE;
        pImp->pListener->bGotModified = FALSE;
    }

    // set creation date to current date
    SetCreated( rAuthor );

    // remove all other user data
    DateTime aInvalid(0,0);
    SetModificationAuthor( String() );
    SetPrintedBy( String() );
    SetModificationDate( aInvalid );
    SetPrintDate( aInvalid );
    SetTime( 0L );
    SetDocumentNumber( 1 );

    if ( pImp->pListener )
    {
        pImp->pListener->bQuiet = FALSE;
        if ( pImp->pListener->bGotModified )
            pImp->pListener->mpDoc->FlushDocInfo();
        pImp->pListener->bGotModified = FALSE;
    }
}

uno::Sequence < ::rtl::OUString > SfxDocumentInfo::GetCustomPropertyNames() const
{
    uno::Reference< beans::XPropertySetInfo > xSetInfo = pImp->xSet->getPropertySetInfo();
    const uno::Sequence< beans::Property > lProps = xSetInfo->getProperties();
    sal_Int32 nProps = lProps.getLength();
    nProps -= NR_BUILTIN_PROPS;
    if ( nProps > 0 )
    {
        uno::Sequence < ::rtl::OUString > aRet(nProps);
        sal_Int32 c = lProps.getLength();
        sal_Int32 n = 0;
        for (sal_Int32 i=0; i<c; ++i)
        {
            if ( lProps[i].Attributes & ::com::sun::star::beans::PropertyAttribute::REMOVABLE )
            {
                if ( n == nProps )
                    aRet.realloc(++nProps);
                aRet[n++] = lProps[i].Name;
            }
        }

        if ( n < nProps )
            aRet.realloc(n);
        return aRet;
    }

    return uno::Sequence < ::rtl::OUString >();
}
