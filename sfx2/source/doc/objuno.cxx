/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: objuno.cxx,v $
 *
 *  $Revision: 1.39 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 16:28:11 $
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

#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/StringPair.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/util/XModifiable.hpp>

#include <unotools/configmgr.hxx>
#include <tools/inetdef.hxx>
#include <unotools/bootstrap.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <osl/mutex.hxx>
#include <rtl/ustrbuf.hxx>
#include <hash_map>                     // dynamic props
#include <svtools/itemprop.hxx>         // dynamic props
#include <vcl/svapp.hxx>
#include <vos/mutex.hxx>

#include <tools/errcode.hxx>
#include <svtools/cntwids.hrc>
#include <comphelper/sequenceasvector.hxx>
#include <comphelper/storagehelper.hxx>
#include <sot/storage.hxx>

#include <sfx2/objuno.hxx>
#include <sfx2/sfx.hrc>
#include <sfx2/sfxsids.hrc>
#include <sfx2/viewsh.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/docinf.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/dispatch.hxx>
#include "openflag.hxx"
#include <sfx2/app.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/request.hxx>
#include <sfx2/sfxuno.hxx>
#include <objshimp.hxx>

#include "sfxresid.hxx"
#include "doc.hrc"

using namespace ::com::sun::star;

// TODO/REFACTOR: provide service for MS formats
// TODO/REFACTOR: IsEncrypted is never set nor read
// Generator is not saved ATM; which value?!
// Generator handling must be implemented
// Deprecate "Theme", rework IDL
// AutoLoadEnabled is deprecated?!
// Reasonable defaults for DateTime
// MIMEType readonly?!
// Announce changes about Theme, Language, Generator, removed entries etc.
// IsEncrypted is necessary for binary formats!
// Open: When to call PrepareDocInfoForSave? Currently only called for own formats and HTML/Writer
// Open: How to load and save EditingTime to MS formats
// PPT-Export should use SavePropertySet

//=============================================================================

#define PROPERTY_UNBOUND 0
#define PROPERTY_MAYBEVOID ::com::sun::star::beans::PropertyAttribute::MAYBEVOID

SfxItemPropertyMap aDocInfoPropertyMap_Impl[] =
{
    { "Author"          , 6 , WID_FROM,           &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
    { "AutoloadEnabled" , 15, MID_DOCINFO_AUTOLOADENABLED, &::getBooleanCppuType(),   PROPERTY_UNBOUND, 0 },
    { "AutoloadSecs"    , 12, MID_DOCINFO_AUTOLOADSECS, &::getCppuType((const sal_Int32*)0),     PROPERTY_UNBOUND, 0 },
    { "AutoloadURL"     , 11, MID_DOCINFO_AUTOLOADURL, &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
    { "CreationDate"    , 12, WID_DATE_CREATED,   &::getCppuType((const ::com::sun::star::util::DateTime*)0),PROPERTY_MAYBEVOID, 0 },
    { "DefaultTarget"   , 13, MID_DOCINFO_DEFAULTTARGET, &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
    { "Description"     , 11, MID_DOCINFO_DESCRIPTION, &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
    { "DocumentStatistic", 17 , MID_DOCINFO_STATISTIC, &::getCppuType((const uno::Sequence< beans::NamedValue >*)0), PROPERTY_UNBOUND, 0 },
    { "EditingCycles"   , 13, MID_DOCINFO_REVISION, &::getCppuType((const sal_Int16*)0),   PROPERTY_UNBOUND, 0 },
    { "EditingDuration" , 15, MID_DOCINFO_EDITTIME, &::getCppuType((const sal_Int32*)0),   PROPERTY_UNBOUND, 0 },
    { "Generator"       , 9,  SID_APPLICATION, &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
    { "Keywords"        , 8 , WID_KEYWORDS,       &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
    { "Language"        , 8,  MID_DOCINFO_CHARLOCALE, &::getCppuType((const lang::Locale*)0), PROPERTY_UNBOUND, 0 },
    { "MIMEType"        , 8 , WID_CONTENT_TYPE,   &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND | ::com::sun::star::beans::PropertyAttribute::READONLY, 0 },
    { "ModifiedBy"      , 10, MID_DOCINFO_MODIFICATIONAUTHOR, &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
    { "ModifyDate"      , 10, WID_DATE_MODIFIED,  &::getCppuType((const ::com::sun::star::util::DateTime*)0),PROPERTY_MAYBEVOID, 0 },
    { "ODFVersion"      , 10, SID_VERSION,   &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
    { "PrintDate"       , 9 , MID_DOCINFO_PRINTDATE, &::getCppuType((const ::com::sun::star::util::DateTime*)0),PROPERTY_MAYBEVOID, 0 },
    { "PrintedBy"       , 9 , MID_DOCINFO_PRINTEDBY, &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
    { "Subject"         , 7 , MID_DOCINFO_SUBJECT, &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
    { "Template"        , 8 , MID_DOCINFO_TEMPLATE, &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
    { "TemplateFileName", 16, SID_TEMPLATE_NAME, &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
    { "TemplateDate"    , 12, MID_DOCINFO_TEMPLATEDATE, &::getCppuType((const ::com::sun::star::util::DateTime*)0),PROPERTY_MAYBEVOID, 0 },
    { "Title"           , 5 , WID_TITLE,          &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
    {0,0,0,0,0,0}
};

static USHORT aDaysInMonth[12] = { 31, 28, 31, 30, 31, 30,
                                   31, 31, 30, 31, 30, 31 };

inline USHORT DaysInMonth( USHORT nMonth, USHORT nYear )
{
    if ( nMonth != 2 )
        return aDaysInMonth[nMonth-1];
    else
    {
        if ( ((nYear % 4) == 0) && ((nYear % 100) != 0) ||
             ((nYear % 400) == 0) )
            return aDaysInMonth[nMonth-1] + 1;
        else
            return aDaysInMonth[nMonth-1];
    }
}

bool IsValidDateTime( const util::DateTime& rDT )
{
    if ( !rDT.Month || (rDT.Month > 12) )
        return false;
    if ( !rDT.Day || (rDT.Day > DaysInMonth( rDT.Month, rDT.Year )) )
        return false;
    else if ( rDT.Year <= 1582 )
    {
        if ( rDT.Year < 1582 )
            return false;
        else if ( rDT.Month < 10 )
            return false;
        else if ( (rDT.Month == 10) && (rDT.Day < 15) )
            return false;
    }

    return true;
}

struct OUStringHashCode
{
    size_t operator()( const ::rtl::OUString& sString ) const
    {
        return sString.hashCode();
    }
};

struct SfxExtendedItemPropertyMap : public SfxItemPropertyMap
{
    ::com::sun::star::uno::Any aValue;
};

typedef ::std::hash_map< ::rtl::OUString                    ,
                         SfxExtendedItemPropertyMap         ,
                         OUStringHashCode                   ,
                         ::std::equal_to< ::rtl::OUString > > TDynamicProps;

void Copy( const uno::Reference < document::XStandaloneDocumentInfo >& rSource, const uno::Reference < document::XStandaloneDocumentInfo >& rTarget )
{
    try
    {
        uno::Reference< beans::XPropertySet > xSet( rSource, uno::UNO_QUERY );
        uno::Reference< beans::XPropertySet > xTarget( rTarget, uno::UNO_QUERY );
        uno::Reference< beans::XPropertySetInfo > xSetInfo = xSet->getPropertySetInfo();
        uno::Reference< beans::XPropertyContainer > xContainer( rTarget, uno::UNO_QUERY );
        uno::Sequence< beans::Property > lProps = xSetInfo->getProperties();
        const beans::Property* pProps = lProps.getConstArray();
        sal_Int32 c = lProps.getLength();
        sal_Int32 i = 0;
        for (i=0; i<c; ++i)
        {
            uno::Any aValue = xSet->getPropertyValue( pProps[i].Name );
            if ( pProps[i].Attributes & ::com::sun::star::beans::PropertyAttribute::REMOVABLE )
                // QUESTION: DefaultValue?!
                xContainer->addProperty( pProps[i].Name, pProps[i].Attributes, aValue );
            try
            {
                // it is possible that the propertysets from XML and binary files differ; we shouldn't break then
                xTarget->setPropertyValue( pProps[i].Name, aValue );
            }
            catch ( uno::Exception& ) {}
        }

        sal_Int16 nCount = rSource->getUserFieldCount();
        sal_Int16 nSupportedCount = rTarget->getUserFieldCount();
        for ( sal_Int16 nInd = 0; nInd < nCount && nInd < nSupportedCount; nInd++ )
        {
            ::rtl::OUString aPropName = rSource->getUserFieldName( nInd );
            rTarget->setUserFieldName( nInd, aPropName );
            ::rtl::OUString aPropVal = rSource->getUserFieldValue( nInd );
            rTarget->setUserFieldValue( nInd, aPropVal );
        }
    }
    catch ( uno::Exception& ) {}
}

class MixedPropertySetInfo : public ::cppu::WeakImplHelper1< ::com::sun::star::beans::XPropertySetInfo >
{
    private:

        SfxItemPropertyMap* _pFixProps;
        TDynamicProps*      _pDynamicProps;

    public:

        MixedPropertySetInfo(SfxItemPropertyMap* pFixProps    ,
                             TDynamicProps*      pDynamicProps);

        virtual ~MixedPropertySetInfo();

        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property > SAL_CALL getProperties(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::beans::Property SAL_CALL getPropertyByName( const ::rtl::OUString& aName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL hasPropertyByName( const ::rtl::OUString& Name ) throw (::com::sun::star::uno::RuntimeException);
};

//-----------------------------------------------------------------------------

MixedPropertySetInfo::MixedPropertySetInfo(SfxItemPropertyMap* pFixProps    ,
                                           TDynamicProps*      pDynamicProps)
    : _pFixProps    (pFixProps    )
    , _pDynamicProps(pDynamicProps)
{
}

//-----------------------------------------------------------------------------

MixedPropertySetInfo::~MixedPropertySetInfo()
{
}

//-----------------------------------------------------------------------------

::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property > SAL_CALL MixedPropertySetInfo::getProperties()
    throw(::com::sun::star::uno::RuntimeException)
{
    ::comphelper::SequenceAsVector< ::com::sun::star::beans::Property > lProps;

    // copy "fix" props
    SfxItemPropertyMap* pFixProp = _pFixProps;
    while(pFixProp && pFixProp->pName)
    {
        ::com::sun::star::beans::Property aProp;

        aProp.Name       = ::rtl::OUString::createFromAscii(pFixProp->pName);
        aProp.Handle     = pFixProp->nWID;
        aProp.Type       = *(pFixProp->pType);
        aProp.Attributes = (sal_Int16)(pFixProp->nFlags);

        lProps.push_back(aProp);
        ++pFixProp;
    }

    // copy "dynamic" props
    TDynamicProps::const_iterator pDynamicProp;
    for (pDynamicProp  = _pDynamicProps->begin();
         pDynamicProp != _pDynamicProps->end()  ;
         ++pDynamicProp                         )
    {
        const SfxExtendedItemPropertyMap& rDynamicProp = pDynamicProp->second;
        ::com::sun::star::beans::Property aProp;

        aProp.Name       = pDynamicProp->first;
        aProp.Handle     = -1; // dont change it. Needed as difference between fix and dynamic props!
        aProp.Type       = rDynamicProp.aValue.getValueType();
        aProp.Attributes = (sal_Int16)(rDynamicProp.nFlags);

        lProps.push_back(aProp);
    }

    return lProps.getAsConstList();
}

//-----------------------------------------------------------------------------

::com::sun::star::beans::Property SAL_CALL MixedPropertySetInfo::getPropertyByName( const ::rtl::OUString& sName )
    throw(::com::sun::star::beans::UnknownPropertyException,
          ::com::sun::star::uno::RuntimeException          )
{
    ::com::sun::star::beans::Property aProp;

    // search it as "fix" prop
    SfxItemPropertyMap* pFixProp = _pFixProps;
    while(pFixProp && pFixProp->pName)
    {
        if (sName.equalsAscii(pFixProp->pName))
        {
            aProp.Name       = sName;
            aProp.Handle     = pFixProp->nWID;
            aProp.Type       = *(pFixProp->pType);
            aProp.Attributes = (sal_Int16)(pFixProp->nFlags);
            return aProp;
        }
        ++pFixProp;
    }

    // search it as "dynamic" prop
    TDynamicProps::const_iterator pDynamicProp = _pDynamicProps->find(sName);
    if (pDynamicProp != _pDynamicProps->end())
    {
        const SfxExtendedItemPropertyMap& rDynamicProp = pDynamicProp->second;
        aProp.Name       = sName;
        aProp.Handle     = rDynamicProp.nWID;
        aProp.Type       = *(rDynamicProp.pType);
        aProp.Attributes = (sal_Int16)(rDynamicProp.nFlags);
        return aProp;
    }

    throw ::com::sun::star::beans::UnknownPropertyException(
            ::rtl::OUString(),
            static_cast< ::cppu::OWeakObject*  >(this));
}

//-----------------------------------------------------------------------------

::sal_Bool SAL_CALL MixedPropertySetInfo::hasPropertyByName(const ::rtl::OUString& sName)
    throw(::com::sun::star::uno::RuntimeException)
{
    // "fix" prop?
    SfxItemPropertyMap* pFixProp = _pFixProps;
    while(pFixProp && pFixProp->pName)
    {
        if (sName.equalsAscii(pFixProp->pName))
            return sal_True;
        ++pFixProp;
    }

    // "dynamic" prop?
    TDynamicProps::const_iterator pDynamicProp = _pDynamicProps->find(sName);
    return (pDynamicProp != _pDynamicProps->end());
}

//-----------------------------------------------------------------------------
struct SfxDocumentInfoObject_Impl
{
    ::osl::Mutex                        _aMutex;
    ::cppu::OInterfaceContainerHelper   _aDisposeContainer;
    ::cppu::OInterfaceContainerHelper   aModifyListenerContainer;
    SfxItemPropertySet _aPropSet;

    TDynamicProps lDynamicProps;
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue > aStatistic;
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::StringPair > aUserKeys;
    ::com::sun::star::lang::Locale aCharLocale;
    com::sun::star::util::DateTime aCreated;    // WID_DATE_CREATED
    com::sun::star::util::DateTime aModified;   // WID_DATE_MODIFIED
    com::sun::star::util::DateTime aPrinted;    // MID_DOCINFO_PRINTDATE
    com::sun::star::util::DateTime aTemplateDate; // MID_DOCINFO_TEMPLATEDATE
    ::rtl::OUString     aMediaType;             // WID_CONTENT_TYPE
    ::rtl::OUString     aAuthor;                // WID_FROM
    ::rtl::OUString     aTitle;                 // WID_TITLE
    ::rtl::OUString     aSubject;               // MID_DOCINFO_SUBJECT
    ::rtl::OUString     aModifiedBy;            // MID_DOCINFO_MODIFICATIONAUTHOR
    ::rtl::OUString     aPrintedBy;             // MID_DOCINFO_PRINTEDBY
    ::rtl::OUString     aKeywords;              // WID_KEYWORDS
    ::rtl::OUString     aDescription;           // MID_DOCINFO_DESCRIPTION
    ::rtl::OUString     aTemplateName;          // MID_DOCINFO_TEMPLATE
    ::rtl::OUString     aTemplateFileName;      // SID_TEMPLATE_NAME
    ::rtl::OUString     aReloadURL;             // MID_DOCINFO_AUTOLOADURL
    ::rtl::OUString     aDefaultTarget;         // MID_DOCINFO_DEFAULTTARGET
    ::rtl::OUString     aGenerator;             // SID_APPLICATION
    ::rtl::OUString     aODFVersion;
    sal_Int32           nEditTime;              // MID_DOCINFO_EDITTIME
    sal_Int32           nReloadSecs;            // MID_DOCINFO_AUTOLOADSECS
    sal_Int16           nRevision;              // MID_DOCINFO_REVISION
    sal_Bool            bReloadEnabled;         // MID_DOCINFO_AUTOLOADENABLED
    sal_Bool            bModified;
    sal_Bool            bDisposed;

    SfxDocumentInfoObject_Impl()
        : _aDisposeContainer( _aMutex )
    , aModifyListenerContainer( _aMutex )
    , _aPropSet( aDocInfoPropertyMap_Impl )
        , nEditTime(0)
        , nReloadSecs(0)
        , nRevision(0)
        , bReloadEnabled(sal_False)
        , bModified(sal_False)
        , bDisposed(sal_False)
    {
        // the number of user fields is not changeable from the outside
        // we can't set it too high because every name/value pair will be written to the file (even if empty)
        // currently our dialog has only 4 user keys so 4 is still a reasonable number
        aUserKeys.realloc(4);
        const String sInfo( SfxResId( STR_DOCINFO_INFOFIELD ) );
        const String sVar( RTL_CONSTASCII_USTRINGPARAM( "%1" ) );
        for( sal_Int32 i = 0; i < MAXDOCUSERKEYS; ++i )
        {
            String sTitle( sInfo );
            sTitle.SearchAndReplace( sVar, String::CreateFromInt32(i+1) );
            aUserKeys[i].First = sTitle;
        }

        aODFVersion = ::rtl::OUString::createFromAscii("1.1");
    }

    void Reset();
};

void SfxDocumentInfoObject_Impl::Reset()
{
    aStatistic.realloc(0);
    // QUESTION: do we have a reasonable default for Locales?
    aCharLocale = com::sun::star::lang::Locale();

    aCreated=com::sun::star::util::DateTime();
    aModified=com::sun::star::util::DateTime();
    aPrinted=com::sun::star::util::DateTime();
    aTemplateDate=com::sun::star::util::DateTime();

    aMediaType = ::rtl::OUString();
    aAuthor = ::rtl::OUString();
    aTitle = ::rtl::OUString();
    aSubject = ::rtl::OUString();
    aModifiedBy = ::rtl::OUString();
    aPrintedBy = ::rtl::OUString();
    aKeywords = ::rtl::OUString();
    aDescription = ::rtl::OUString();
    aTemplateName = ::rtl::OUString();
    aTemplateFileName = ::rtl::OUString();
    aReloadURL = ::rtl::OUString();
    aDefaultTarget = ::rtl::OUString();
    nEditTime=0;
    nReloadSecs=0;
    nRevision=0;
    bModified=sal_False;
    lDynamicProps.clear();
    const ::rtl::OUString aInf( DEFINE_CONST_UNICODE( "Info " ) );
    for( sal_Int32 i = 0; i<4; ++i )
    {
        aUserKeys[i].First = aInf;
        aUserKeys[i].First += String::CreateFromInt32(i+1);
        aUserKeys[i].Second = ::rtl::OUString();
    }
}

//-----------------------------------------------------------------------------

SfxDocumentInfoObject::SfxDocumentInfoObject()
    : _pImp( new SfxDocumentInfoObject_Impl() )
{
}

//-----------------------------------------------------------------------------

SfxDocumentInfoObject::~SfxDocumentInfoObject()
{
    delete _pImp;
}

//-----------------------------------------------------------------------------

void SAL_CALL SfxDocumentInfoObject::dispose() throw( ::com::sun::star::uno::RuntimeException )
{
    ::com::sun::star::lang::EventObject aEvent( (::cppu::OWeakObject *)this );
    _pImp->_aDisposeContainer.disposeAndClear( aEvent );
    ::osl::MutexGuard aGuard( _pImp->_aMutex );
    _pImp->bDisposed = sal_True;
}

void SfxDocumentInfoObject::NotifyModified()
{
    if ( _pImp->bDisposed )
        return;
    lang::EventObject aEvent( (document::XDocumentInfo*)this );
    ::cppu::OInterfaceIteratorHelper aIt( _pImp->aModifyListenerContainer );
    while( aIt.hasMoreElements() )
    {
        try
        {
            ((util::XModifyListener*)aIt.next())->modified( aEvent );
        }
        catch( uno::RuntimeException& )
        {
            aIt.remove();
        }
    }
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
    ::osl::MutexGuard aGuard( _pImp->_aMutex );

    MixedPropertySetInfo* pInfo = new MixedPropertySetInfo( aDocInfoPropertyMap_Impl, &_pImp->lDynamicProps );
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > xInfo(
        static_cast< ::com::sun::star::beans::XPropertySetInfo* >(pInfo),
        ::com::sun::star::uno::UNO_QUERY_THROW);
    return xInfo;
}

//-----------------------------------------------------------------------------

void SAL_CALL  SfxDocumentInfoObject::setPropertyValue(const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue) throw( ::com::sun::star::uno::RuntimeException )
{
    const SfxItemPropertyMap* pMap = SfxItemPropertyMap::GetByName(
            aDocInfoPropertyMap_Impl,
            aPropertyName );
    // fix prop!
    if ( pMap )
        setFastPropertyValue( pMap->nWID, aValue );
    else
    // dynamic prop!
    {
        ::osl::ClearableMutexGuard aGuard( _pImp->_aMutex );
        TDynamicProps::iterator pProp = _pImp->lDynamicProps.find(aPropertyName);
        if ( pProp != _pImp->lDynamicProps.end() )
        {
            SfxExtendedItemPropertyMap& rExtMap = pProp->second;
            if (( rExtMap.nFlags & ::com::sun::star::beans::PropertyAttribute::READONLY ) != ::com::sun::star::beans::PropertyAttribute::READONLY )
            {
                rExtMap.aValue = aValue;
                // TODO/REFACTOR check if value was realy changed!
                // no objsh if we are used from a StandaloneDocInfo!
                _pImp->bModified = sal_True;
                aGuard.clear();
                NotifyModified();
            }
        }
    }
}

//-----------------------------------------------------------------------------

::com::sun::star::uno::Any  SAL_CALL  SfxDocumentInfoObject::getPropertyValue(const ::rtl::OUString& aPropertyName)  throw( ::com::sun::star::uno::RuntimeException )
{
    const SfxItemPropertyMap* pMap = SfxItemPropertyMap::GetByName( aDocInfoPropertyMap_Impl,
        aPropertyName );
    // fix prop!
    if ( pMap )
        return getFastPropertyValue( pMap->nWID );
    else
    // dynamic prop!
    {
        ::osl::MutexGuard aGuard( _pImp->_aMutex );
        TDynamicProps::iterator pProp = _pImp->lDynamicProps.find(aPropertyName);
        if ( pProp != _pImp->lDynamicProps.end() )
        {
            SfxExtendedItemPropertyMap& rExtMap = pProp->second;
            return rExtMap.aValue;
        }
    }

    return ::com::sun::star::uno::Any();
}

sal_Bool SAL_CALL SfxDocumentInfoObject::isModified() throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( _pImp->_aMutex );
    return _pImp->bModified;
}

void SAL_CALL SfxDocumentInfoObject::setModified( sal_Bool bModified )
        throw (::com::sun::star::beans::PropertyVetoException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::ClearableMutexGuard aGuard( _pImp->_aMutex );
    _pImp->bModified = bModified;
    if ( bModified )
    {
        aGuard.clear();
        NotifyModified();
    }
}

void SAL_CALL SfxDocumentInfoObject::addModifyListener( const uno::Reference< util::XModifyListener >& xListener) throw( uno::RuntimeException )
{
    _pImp->aModifyListenerContainer.addInterface( xListener );
}

void SAL_CALL SfxDocumentInfoObject::removeModifyListener( const uno::Reference< util::XModifyListener >& xListener) throw( uno::RuntimeException )
{
    _pImp->aModifyListenerContainer.removeInterface( xListener );
}

//-----------------------------------------------------------------------------

void SAL_CALL  SfxDocumentInfoObject::addPropertyChangeListener(const ::rtl::OUString&, const uno::Reference< beans::XPropertyChangeListener > & ) throw( uno::RuntimeException )
{}

//-----------------------------------------------------------------------------

void SAL_CALL  SfxDocumentInfoObject::removePropertyChangeListener(const ::rtl::OUString&, const uno::Reference< ::beans::XPropertyChangeListener > & ) throw( uno::RuntimeException )
{}

//-----------------------------------------------------------------------------

void SAL_CALL  SfxDocumentInfoObject::addVetoableChangeListener(const ::rtl::OUString&, const uno::Reference< beans::XVetoableChangeListener > & ) throw( uno::RuntimeException )
{}

//-----------------------------------------------------------------------------

void SAL_CALL  SfxDocumentInfoObject::removeVetoableChangeListener(const ::rtl::OUString&, const uno::Reference< beans::XVetoableChangeListener > & ) throw( uno::RuntimeException )
{}

uno::Sequence< beans::PropertyValue > SAL_CALL  SfxDocumentInfoObject::getPropertyValues( void ) throw( uno::RuntimeException )
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

void SAL_CALL SfxDocumentInfoObject::addProperty(const ::rtl::OUString&            sName        ,
                                                       sal_Int16                   nAttributes  ,
                                                 const ::com::sun::star::uno::Any& aDefaultValue)
    throw(::com::sun::star::beans::PropertyExistException ,
          ::com::sun::star::beans::IllegalTypeException   ,
          ::com::sun::star::lang::IllegalArgumentException,
          ::com::sun::star::uno::RuntimeException         )
{
    ::osl::ClearableMutexGuard aGuard( _pImp->_aMutex );

    // clash with "fix" properties ?
    sal_Bool bFixProp = (SfxItemPropertyMap::GetByName( aDocInfoPropertyMap_Impl, sName ) != 0);

    // clash with "dynamic" properties ?
    sal_Bool bDynamicProp = (_pImp->lDynamicProps.find(sName) != _pImp->lDynamicProps.end());

    if ( bFixProp || bDynamicProp )
    {
        ::rtl::OUStringBuffer sMsg(256);
        sMsg.appendAscii("The property \""   );
        sMsg.append     (sName               );
        sMsg.appendAscii("\" "               );
        if ( bFixProp )
            sMsg.appendAscii(" already exists as a fix property. Please have a look into the IDL documentation of the DocumentInfo service.");
        else
        if ( bDynamicProp )
            sMsg.appendAscii(" already exists as a user defined property.");

        throw ::com::sun::star::beans::PropertyExistException(
                sMsg.makeStringAndClear(),
                static_cast< ::cppu::OWeakObject* >(this));
    }

    // TODO filter Any-Type and reject unsupported ones!
    sal_Bool bTypeOK = sal_False;
    switch(aDefaultValue.getValueTypeClass())
    {
        case com::sun::star::uno::TypeClass_BYTE :
        case com::sun::star::uno::TypeClass_SHORT :
        case com::sun::star::uno::TypeClass_UNSIGNED_SHORT :
        case com::sun::star::uno::TypeClass_LONG :
        case com::sun::star::uno::TypeClass_UNSIGNED_LONG :
        case com::sun::star::uno::TypeClass_BOOLEAN :
        case com::sun::star::uno::TypeClass_FLOAT :
        case com::sun::star::uno::TypeClass_DOUBLE :
        case com::sun::star::uno::TypeClass_STRING :
            {
                bTypeOK = sal_True;
            }
            break;

        case com::sun::star::uno::TypeClass_STRUCT :
            {
                ::com::sun::star::util::Date     aDate    ;
                ::com::sun::star::util::Time     aTime    ;
                ::com::sun::star::util::DateTime aDateTime;
                bTypeOK = (
                            (aDefaultValue >>= aDate    ) ||
                            (aDefaultValue >>= aTime    ) ||
                            (aDefaultValue >>= aDateTime)
                          );
            }
            break;
        default:
            break;
    }

    if (!bTypeOK)
        throw ::com::sun::star::beans::IllegalTypeException(
                ::rtl::OUString::createFromAscii("Only the following value types are supported:\nBYTE, SHORT, INTEGER, LONG, BOOLEAN, FLOAT, DOUBLE, STRING, DATE, TIME, DATETIME."),
                static_cast< ::cppu::OWeakObject* >(this));

    SfxExtendedItemPropertyMap aProp;
    aProp.pName    = 0; // superflous -> holded as hash key.
    aProp.nNameLen = 0;
    aProp.nFlags   = nAttributes;
    aProp.aValue   = aDefaultValue;
    aProp.nWID     = 0xffff;

    if (aProp.nFlags == 0)
    {
        aProp.nFlags = ::com::sun::star::beans::PropertyAttribute::TRANSIENT |
                        ::com::sun::star::beans::PropertyAttribute::REMOVABLE ;
    }

    _pImp->lDynamicProps[sName] = aProp;
    _pImp->bModified = sal_True;
    aGuard.clear();
    NotifyModified();
}

void SAL_CALL SfxDocumentInfoObject::removeProperty(const ::rtl::OUString& sName)
    throw(::com::sun::star::beans::UnknownPropertyException,
          ::com::sun::star::beans::NotRemoveableException  ,
          ::com::sun::star::uno::RuntimeException          )
{
    ::osl::ClearableMutexGuard aGuard( _pImp->_aMutex );

    // clash with "fix" properties ?
    sal_Bool bFixProp = (SfxItemPropertyMap::GetByName( aDocInfoPropertyMap_Impl, sName ) != 0);
    if ( bFixProp )
    {
        ::rtl::OUStringBuffer sMsg(256);
        sMsg.appendAscii("The property \""                                                    );
        sMsg.append     (sName                                                                );
        sMsg.appendAscii("\" cant be removed. Its a fix property of the DocumentInfo service.");

        throw ::com::sun::star::beans::NotRemoveableException(
                sMsg.makeStringAndClear(),
                static_cast< ::cppu::OWeakObject* >(this));
    }

    // clash with "dynamic" properties ?
    TDynamicProps::iterator pDynamicProp = _pImp->lDynamicProps.find(sName);
    sal_Bool                bDynamicProp = ( pDynamicProp != _pImp->lDynamicProps.end() );
    if ( bDynamicProp )
    {
        SfxExtendedItemPropertyMap& rProp = pDynamicProp->second;
        if (( rProp.nFlags & ::com::sun::star::beans::PropertyAttribute::REMOVEABLE ) != ::com::sun::star::beans::PropertyAttribute::REMOVEABLE )
        {
            ::rtl::OUStringBuffer sMsg(256);
            sMsg.appendAscii("The property \""                );
            sMsg.append     (sName                            );
            sMsg.appendAscii("\" is marked as non removeable.");

            throw ::com::sun::star::beans::NotRemoveableException(
                    sMsg.makeStringAndClear(),
                    static_cast< ::cppu::OWeakObject* >(this));
        }

        // found and removeable -> do it
        _pImp->lDynamicProps.erase(pDynamicProp);
        _pImp->bModified = sal_True;
        aGuard.clear();
        NotifyModified();
        return;
    }

    // non existing
    ::rtl::OUStringBuffer sMsg(256);
    sMsg.appendAscii("The property \""   );
    sMsg.append     (sName               );
    sMsg.appendAscii("\" does not exist.");

    throw ::com::sun::star::beans::UnknownPropertyException(
            sMsg.makeStringAndClear(),
            static_cast< ::cppu::OWeakObject* >(this));
}

BOOL equalsDateTime( const util::DateTime& D1, const util::DateTime& D2 )
{
    return D1.HundredthSeconds == D2.HundredthSeconds &&
           D1.Seconds == D2.Seconds &&
           D1.Minutes == D2.Minutes &&
           D1.Hours == D2.Hours &&
           D1.Day == D2.Day &&
           D1.Month == D2.Month &&
           D1.Year == D2.Year;
}

void SAL_CALL  SfxDocumentInfoObject::setFastPropertyValue(sal_Int32 nHandle, const ::com::sun::star::uno::Any& aValue) throw( ::com::sun::star::uno::RuntimeException )
{
    // Attention: Only fix properties should be provided by this method.
    // Dynamic properties has no handle in real ... because it cant be used inside multithreaded environments :-)

    ::osl::ClearableMutexGuard aGuard( _pImp->_aMutex );
    sal_Bool bModified = sal_False;

    if ( aValue.getValueType() == ::getCppuType((const ::rtl::OUString*)0) )
    {
        ::rtl::OUString sTemp ;
        aValue >>= sTemp ;
        switch ( nHandle )
        {
            case SID_APPLICATION :
                if ( _pImp->aGenerator != sTemp )
                    bModified = sal_True;
                _pImp->aGenerator = sTemp;
                break;
            case WID_FROM :
            {
                // QUESTION: do we still need this?
                /*
                // String aStrVal( sTemp );
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
                } */

                if ( _pImp->aAuthor != sTemp )
                    bModified = sal_True;
                _pImp->aAuthor = sTemp;
                break;
            }
            case MID_DOCINFO_PRINTEDBY:
                if ( _pImp->aPrintedBy != sTemp )
                    bModified = sal_True;
                _pImp->aPrintedBy = sTemp;
                break;
            case MID_DOCINFO_MODIFICATIONAUTHOR:
                if ( _pImp->aModifiedBy != sTemp )
                    bModified = sal_True;
                _pImp->aModifiedBy = sTemp;
                break;
            case WID_TITLE :
            {
                if ( _pImp->aTitle != sTemp )
                    bModified = sal_True;
                _pImp->aTitle = sTemp;
                break;
            }
            case MID_DOCINFO_SUBJECT :
                if ( _pImp->aSubject != sTemp )
                    bModified = sal_True;
                _pImp->aSubject = sTemp;
                break;
            case WID_KEYWORDS :
                if ( _pImp->aKeywords != sTemp )
                    bModified = sal_True;
                _pImp->aKeywords = sTemp;
                break;
            case MID_DOCINFO_TEMPLATE:
                if ( _pImp->aTemplateName != sTemp )
                    bModified = sal_True;
                _pImp->aTemplateName = sTemp;
                break;
            case SID_TEMPLATE_NAME:
                if ( _pImp->aTemplateFileName != sTemp )
                    bModified = sal_True;
                _pImp->aTemplateFileName = sTemp;
                break;
            case MID_DOCINFO_DESCRIPTION:
                if ( _pImp->aDescription != sTemp )
                    bModified = sal_True;
                _pImp->aDescription = sTemp;
                break;
            case MID_DOCINFO_AUTOLOADURL:
                if ( _pImp->aReloadURL != sTemp )
                    bModified = sal_True;
                _pImp->aReloadURL = sTemp;
                break;
            case MID_DOCINFO_DEFAULTTARGET:
                if ( _pImp->aDefaultTarget != sTemp )
                    bModified = sal_True;
                _pImp->aDefaultTarget = sTemp;
                break;
            case WID_CONTENT_TYPE :
                if ( _pImp->aMediaType != sTemp )
                    bModified = sal_True;
                _pImp->aMediaType = sTemp;
                break;
            case SID_VERSION:
                _pImp->aODFVersion = sTemp;
                break;
            default:
                bModified = sal_False;
                break;
        }
    }
    else if ( aValue.getValueType() == ::getCppuType((const ::com::sun::star::util::DateTime*)0) )
    {
        com::sun::star::util::DateTime aTemp;
        aValue >>= aTemp ;
        switch ( nHandle )
        {
            case WID_DATE_CREATED :
            {
                if ( !equalsDateTime(_pImp->aCreated, aTemp ) )
                {
                    bModified = sal_True;
                    _pImp->aCreated = aTemp;
                }
                break;
            }
            case WID_DATE_MODIFIED :
            {
                if ( !equalsDateTime(_pImp->aModified, aTemp ) )
                {
                    bModified = sal_True;
                    _pImp->aModified = aTemp;
                }
                break;
            }
            case MID_DOCINFO_PRINTDATE :
            {
                if ( !equalsDateTime(_pImp->aPrinted, aTemp ) )
                {
                    bModified = sal_True;
                    _pImp->aPrinted = aTemp;
                }
                break;
            }
            case MID_DOCINFO_TEMPLATEDATE :
            {
                if ( !equalsDateTime(_pImp->aTemplateDate, aTemp ) )
                {
                    bModified = sal_True;
                    _pImp->aTemplateDate = aTemp;
                }
                break;
            }
            default:
                bModified = sal_False;
                break;
        }
    }
    else if ( aValue.getValueType() == ::getVoidCppuType() )
    {
        // DateTime properties may be void to describe an invalid DateTime value
        switch ( nHandle )
        {
            case WID_DATE_CREATED :
            {
                if ( IsValidDateTime(_pImp->aCreated) )
                {
                    bModified = sal_True;
                    _pImp->aCreated = util::DateTime();
                }
                break;
            }
            case WID_DATE_MODIFIED :
            {
                if ( IsValidDateTime(_pImp->aModified) )
                {
                    bModified = sal_True;
                    _pImp->aModified = util::DateTime();
                }
                break;
            }
            case MID_DOCINFO_PRINTDATE :
            {
                if ( IsValidDateTime(_pImp->aPrinted) )
                {
                    bModified = sal_True;
                    _pImp->aPrinted = util::DateTime();
                }
                break;
            }
            case MID_DOCINFO_TEMPLATEDATE :
            {
                if ( IsValidDateTime(_pImp->aTemplateDate) )
                {
                    bModified = sal_True;
                    _pImp->aTemplateDate = util::DateTime();
                }
                break;
            }
            default:
                bModified = sal_False;
                break;
        }
    }
    else if ( aValue.getValueType() == ::getBooleanCppuType() )
    {
        sal_Bool bBoolVal = false;
        aValue >>= bBoolVal ;
        switch ( nHandle )
        {
            case MID_DOCINFO_AUTOLOADENABLED:
                if ( bBoolVal != _pImp->bReloadEnabled )
                    bModified = sal_True;
                _pImp->bReloadEnabled = bBoolVal;
                break;
            default:
                bModified = sal_False;
        }
    }
    else if ( aValue.getValueType() == ::getCppuType((const sal_Int32*)0) )
    {
        sal_Int32 nIntVal = 0;
        aValue >>= nIntVal ;
        switch ( nHandle )
        {
            case MID_DOCINFO_AUTOLOADSECS:
                if ( nIntVal != _pImp->nReloadSecs )
                    bModified = sal_True;
                _pImp->nReloadSecs = nIntVal;
                break;
            case MID_DOCINFO_EDITTIME:
                if ( nIntVal != _pImp->nEditTime )
                    bModified = sal_True;
                _pImp->nEditTime = nIntVal;
            default:
                bModified = sal_False;
        }
    }
    else if ( aValue.getValueType() == ::getCppuType((const sal_Int16*)0) )
    {
        short nIntVal = 0;
        aValue >>= nIntVal ;
        switch ( nHandle )
        {
            case MID_DOCINFO_REVISION:
                if ( nIntVal != _pImp->nRevision )
                    bModified = sal_True;
                _pImp->nRevision = nIntVal;
            default:
                bModified = sal_False;
                break;
        }
    }
    else if ( aValue.getValueType() == ::getCppuType((const uno::Sequence< beans::NamedValue >*)0) )
    {
        if ( nHandle == MID_DOCINFO_STATISTIC )
        {
            uno::Sequence < beans::NamedValue > aData;
            aValue >>= aData;
            if ( aData != _pImp->aStatistic )
            {
                bModified = sal_True;
                aValue >>= _pImp->aStatistic;
/*
                for ( sal_Int32 n=0; n<_pImp->aStatistic.getLength(); n++ )
                {
                    ::rtl::OUString aName = _pImp->aStatistic[n].Name;
                    if ( aName.equalsAscii("bla") )
                        bModified = sal_True;
                }
*/
            }
        }
        else
            bModified = sal_False;
    }
    else if ( aValue.getValueType() == ::getCppuType((const lang::Locale*)0) )
    {
        if ( nHandle == MID_DOCINFO_CHARLOCALE )
        {
            lang::Locale aLocale;
            aValue >>= aLocale;
            if ( aLocale.Language != _pImp->aCharLocale.Language ||
                 aLocale.Country !=  _pImp->aCharLocale.Country ||
                 aLocale.Variant !=  _pImp->aCharLocale.Variant   )
            {
                bModified = sal_True;
                aValue >>= _pImp->aCharLocale;
            }
        }
        else
            bModified = sal_False;
    }

    if ( bModified )
    {
        _pImp->bModified = sal_True;
        aGuard.clear();
        NotifyModified();
    }
}

//-----------------------------------------------------------------------------

::com::sun::star::uno::Any SAL_CALL  SfxDocumentInfoObject::getFastPropertyValue(sal_Int32 nHandle) throw( ::com::sun::star::uno::RuntimeException )
{
    // Attention: Only fix properties should be provided by this method.
    // Dynamic properties has no handle in real ... because it cant be used inside multithreaded environments :-)

    ::osl::MutexGuard aGuard( _pImp->_aMutex );
    ::com::sun::star::uno::Any aValue;
    switch ( nHandle )
    {
        case SID_APPLICATION :
            aValue <<= _pImp->aGenerator;
            break;
        case WID_CONTENT_TYPE :
            aValue <<= _pImp->aMediaType;
            break;
        case MID_DOCINFO_REVISION :
            aValue <<= _pImp->nRevision;
            break;
        case MID_DOCINFO_EDITTIME :
            aValue <<= _pImp->nEditTime;
            break;
        case WID_FROM :
            aValue <<= _pImp->aAuthor;
            break;
        case WID_DATE_CREATED :
            if ( IsValidDateTime( _pImp->aCreated ) )
                aValue <<= _pImp->aCreated;
            break;
        case WID_TITLE :
            aValue <<= _pImp->aTitle;
            break;
        case MID_DOCINFO_SUBJECT:
            aValue <<= _pImp->aSubject;
            break;
        case MID_DOCINFO_MODIFICATIONAUTHOR:
            aValue <<= _pImp->aModifiedBy;
            break;
        case WID_DATE_MODIFIED :
            if ( IsValidDateTime( _pImp->aModified ) )
                aValue <<= _pImp->aModified;
            break;
        case MID_DOCINFO_PRINTEDBY:
            aValue <<= _pImp->aPrintedBy;
            break;
        case MID_DOCINFO_PRINTDATE:
            if ( IsValidDateTime( _pImp->aPrinted ) )
                aValue <<= _pImp->aPrinted;
            break;
        case WID_KEYWORDS :
            aValue <<= _pImp->aKeywords;
            break;
        case MID_DOCINFO_DESCRIPTION:
            aValue <<= _pImp->aDescription;
            break;
        case MID_DOCINFO_TEMPLATE:
            aValue <<= _pImp->aTemplateName;
            break;
        case SID_TEMPLATE_NAME:
            aValue <<= _pImp->aTemplateFileName;
            break;
        case MID_DOCINFO_TEMPLATEDATE:
            if ( IsValidDateTime( _pImp->aTemplateDate ) )
                aValue <<= _pImp->aTemplateDate;
            break;
        case MID_DOCINFO_AUTOLOADENABLED:
            aValue <<= _pImp->bReloadEnabled;
            break;
        case MID_DOCINFO_AUTOLOADURL:
            aValue <<= _pImp->aReloadURL;
            break;
        case SID_VERSION:
            aValue <<= _pImp->aODFVersion;
            break;
        case MID_DOCINFO_AUTOLOADSECS:
            aValue <<= _pImp->nReloadSecs;
            break;
        case MID_DOCINFO_DEFAULTTARGET:
            aValue <<= _pImp->aDefaultTarget;
            break;
        case MID_DOCINFO_STATISTIC:
            aValue <<= _pImp->aStatistic;
            break;
        case MID_DOCINFO_CHARLOCALE:
            aValue <<= _pImp->aCharLocale;
            break;
        default:
            aValue <<= ::rtl::OUString();
            break;
    }

    return aValue;
}

//-----------------------------------------------------------------------------

sal_Int16 SAL_CALL  SfxDocumentInfoObject::getUserFieldCount() throw( ::com::sun::star::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( _pImp->_aMutex );
    return (sal_Int16) _pImp->aUserKeys.getLength();
}

//-----------------------------------------------------------------------------

::rtl::OUString SAL_CALL  SfxDocumentInfoObject::getUserFieldName(sal_Int16 nIndex) throw( ::com::sun::star::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( _pImp->_aMutex );
    if ( nIndex < getUserFieldCount() )
        return _pImp->aUserKeys[nIndex].First;
    else
        return ::rtl::OUString();
}

//-----------------------------------------------------------------------------

::rtl::OUString SAL_CALL  SfxDocumentInfoObject::getUserFieldValue(sal_Int16 nIndex) throw( ::com::sun::star::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( _pImp->_aMutex );
    if ( nIndex < getUserFieldCount() )
        return _pImp->aUserKeys[nIndex].Second;
    else
        return ::rtl::OUString();
}

//-----------------------------------------------------------------------------

void  SAL_CALL SfxDocumentInfoObject::setUserFieldName(sal_Int16 nIndex, const ::rtl::OUString& aName ) throw( ::com::sun::star::uno::RuntimeException )
{
    ::osl::ClearableMutexGuard aGuard( _pImp->_aMutex );
    if ( nIndex < getUserFieldCount() )
    {
        if (_pImp->aUserKeys[nIndex].First != aName )
        {
            _pImp->aUserKeys[nIndex].First = aName;
            _pImp->bModified = sal_True;
            aGuard.clear();
            NotifyModified();
        }
    }
}

//-----------------------------------------------------------------------------

void SAL_CALL  SfxDocumentInfoObject::setUserFieldValue( sal_Int16 nIndex, const ::rtl::OUString& aValue ) throw( ::com::sun::star::uno::RuntimeException )
{
    ::osl::ClearableMutexGuard aGuard( _pImp->_aMutex );
    if ( nIndex < getUserFieldCount() )
    {
        if (_pImp->aUserKeys[nIndex].Second != aValue )
        {
            _pImp->aUserKeys[nIndex].Second = aValue;
            _pImp->bModified = sal_True;
            aGuard.clear();
            NotifyModified();
        }
    }
}

//-----------------------------------------------------------------------------
SFX_IMPL_XINTERFACE_2( SfxStandaloneDocumentInfoObject, SfxDocumentInfoObject, ::com::sun::star::lang::XServiceInfo, ::com::sun::star::document::XStandaloneDocumentInfo  )
SFX_IMPL_XTYPEPROVIDER_10( SfxStandaloneDocumentInfoObject, ::com::sun::star::document::XDocumentInfo, ::com::sun::star::lang::XComponent,
    ::com::sun::star::beans::XPropertySet, ::com::sun::star::beans::XFastPropertySet, ::com::sun::star::beans::XPropertyAccess,
    ::com::sun::star::beans::XPropertyContainer, ::com::sun::star::util::XModifiable, ::com::sun::star::util::XModifyBroadcaster,
    ::com::sun::star::document::XStandaloneDocumentInfo, ::com::sun::star::lang::XServiceInfo )

SFX_IMPL_XSERVICEINFO( SfxStandaloneDocumentInfoObject, "com.sun.star.document.StandaloneDocumentInfo", "com.sun.star.comp.sfx2.StandaloneDocumentInfo" )
SFX_IMPL_SINGLEFACTORY( SfxStandaloneDocumentInfoObject )

SfxStandaloneDocumentInfoObject::SfxStandaloneDocumentInfoObject( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xFactory )
    : SfxDocumentInfoObject()
    , _xFactory( xFactory )
{
}

//-----------------------------------------------------------------------------

SfxStandaloneDocumentInfoObject::~SfxStandaloneDocumentInfoObject()
{
}

//-----------------------------------------------------------------------------

uno::Reference< embed::XStorage > GetStorage_Impl( const ::rtl::OUString& rName, sal_Bool bWrite, uno::Reference < lang::XMultiServiceFactory >& xFactory )
{
    // catch unexpected exceptions under solaris
    // Client code checks the returned reference but is not interested on error details.
    try
    {
        ::vos::OGuard aSolarGuard( Application::GetSolarMutex() );
        return ::comphelper::OStorageHelper::GetStorageFromURL(
                        rName,
                        bWrite ? embed::ElementModes::READWRITE : embed::ElementModes::READ,
                        xFactory );
    }
    catch(const uno::Exception&)
    {}

    return uno::Reference< embed::XStorage >();
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
void SAL_CALL  SfxStandaloneDocumentInfoObject::loadFromURL(const ::rtl::OUString& aURL)
    throw( ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException )
{
    sal_Bool bOK = sal_False;

    ::osl::ClearableMutexGuard aGuard( _pImp->_aMutex );
    _pImp->Reset();
    aGuard.clear();

    uno::Reference< embed::XStorage > xStorage = GetStorage_Impl( aURL, sal_False, _xFactory );
    if ( xStorage.is() )
    {
        try
        {
            // set the mediatype from the storage
            ::rtl::OUString aMediaType;
            uno::Reference< beans::XPropertySet > xStorProps( xStorage, uno::UNO_QUERY_THROW );
            xStorProps->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "MediaType" ) ) ) >>= _pImp->aMediaType;

            // import from XML meta data using SAX parser
            uno::Reference< XInterface > xXMLParser = _xFactory->createInstance( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.xml.sax.Parser" )) );
            if( xXMLParser.is() )
            {
                // create input source for SAX parser
                xml::sax::InputSource aParserInput;
                aParserInput.sSystemId = aURL;

                ::rtl::OUString aDocName = ::rtl::OUString::createFromAscii( "meta.xml" );
                if ( xStorage->hasByName( aDocName ) && xStorage->isStreamElement( aDocName ) )
                {
                    uno::Reference< io::XStream > xStorageStream = xStorage->openStreamElement( aDocName, embed::ElementModes::READ );
                    aParserInput.aInputStream = xStorageStream->getInputStream();
                    if ( aParserInput.aInputStream.is() )
                    {
                        sal_Bool bOasis = ( SotStorage::GetVersion( xStorage ) > SOFFICE_FILEFORMAT_60 );
                        const sal_Char *pServiceName = bOasis
                            ? "com.sun.star.document.XMLOasisMetaImporter"
                            : "com.sun.star.document.XMLMetaImporter";

                        // create importer service
                        uno::Reference < xml::sax::XDocumentHandler > xDocHandler( _xFactory->createInstanceWithArguments(
                                rtl::OUString::createFromAscii(pServiceName),
                                uno::Sequence < uno::Any >() ), uno::UNO_QUERY );

                        // connect importer with this object
                        uno::Reference < document::XImporter > xImporter( xDocHandler, uno::UNO_QUERY );
                        if ( xImporter.is() )
                            xImporter->setTargetDocument( this );

                        // connect parser and filter
                        uno::Reference < xml::sax::XParser > xParser( xXMLParser, uno::UNO_QUERY );
                        xParser->setDocumentHandler( xDocHandler );

                        // parse
                        xParser->parseStream( aParserInput );
                        bOK = sal_True;
                    }
                }
            }
        }
        catch( uno::Exception& )
        {
        }
    }
    else
    {
        uno::Reference < document::XStandaloneDocumentInfo > xBinary( _xFactory->createInstance(
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.document.BinaryStandaloneDocumentInfo" ) ) ), uno::UNO_QUERY );
        if ( xBinary.is() )
        {
            xBinary->loadFromURL( aURL );
            bOK = sal_True;
            uno::Reference < document::XStandaloneDocumentInfo > xTarget( static_cast < document::XStandaloneDocumentInfo*> (this), uno::UNO_QUERY );
            Copy( xBinary, xTarget );
        }
    }

    if ( !bOK )
        throw task::ErrorCodeIOException( ::rtl::OUString(), uno::Reference< uno::XInterface >(), ERRCODE_IO_CANTREAD );
}

//-----------------------------------------------------------------------------

void SAL_CALL  SfxStandaloneDocumentInfoObject::storeIntoURL(const ::rtl::OUString& aURL) throw( ::com::sun::star::io::IOException )
{
    sal_Bool bOK = sal_False;
    uno::Reference< embed::XStorage > xStorage = GetStorage_Impl( aURL, sal_True, _xFactory );
    if ( xStorage.is() )
    {
        try
        {
            // set the mediatype to the storage
            ::rtl::OUString aMTPropName( RTL_CONSTASCII_USTRINGPARAM( "MediaType" ) );
            //::rtl::OUString aMediaType;
            uno::Reference< beans::XPropertySet > xStorProps( xStorage, uno::UNO_QUERY_THROW );
            xStorProps->setPropertyValue( aMTPropName, uno::makeAny( _pImp->aMediaType ) );

            uno::Reference< io::XStream > xStorageStream = xStorage->openStreamElement(
                                                        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "meta.xml" ) ),
                                                        embed::ElementModes::READWRITE | embed::ElementModes::TRUNCATE );

            uno::Reference< beans::XPropertySet > xStreamProps( xStorageStream, uno::UNO_QUERY_THROW );
            xStreamProps->setPropertyValue( aMTPropName,
                                            uno::makeAny( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "text/xml" ) ) ) );
            xStreamProps->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Compressed" ) ),
                                            uno::makeAny( (sal_Bool) sal_False ) );
            xStreamProps->setPropertyValue(
                                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "UseCommonStoragePasswordEncryption" ) ),
                                    uno::makeAny( (sal_Bool) sal_False ) );


            uno::Reference< io::XOutputStream > xInfoOutput = xStorageStream->getOutputStream();
            if ( !xInfoOutput.is() )
                throw uno::RuntimeException();

            // Export to XML meta data using SAX writer
            uno::Reference< io::XActiveDataSource > xSaxWriter(
                            _xFactory->createInstance(
                                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.xml.sax.Writer" ) ) ),
                            uno::UNO_QUERY_THROW );
            xSaxWriter->setOutputStream( xInfoOutput );

            sal_Bool bOasis = ( SotStorage::GetVersion( xStorage ) > SOFFICE_FILEFORMAT_60 );
            const sal_Char *pServiceName = bOasis
                ? "com.sun.star.document.XMLOasisMetaExporter"
                : "com.sun.star.document.XMLMetaExporter";

            // create exporter service
            uno::Reference< xml::sax::XDocumentHandler > xDocHandler( xSaxWriter, uno::UNO_QUERY_THROW );
            uno::Sequence< uno::Any > aSeq( 1 );
            aSeq[0] <<= xDocHandler;
               uno::Reference< document::XExporter > xExporter(
                            _xFactory->createInstanceWithArguments(
                                rtl::OUString::createFromAscii( pServiceName ),
                                aSeq ),
                            uno::UNO_QUERY_THROW );
            xExporter->setSourceDocument( this );

            uno::Reference< document::XFilter > xFilter ( xExporter, uno::UNO_QUERY_THROW );
            if ( xFilter->filter( uno::Sequence< beans::PropertyValue >() ) )
            {
                uno::Reference< embed::XTransactedObject > xTransaction( xStorage, uno::UNO_QUERY );
                if ( xTransaction.is() )
                    xTransaction->commit();
                bOK = sal_True;
            }
        }
        catch( uno::RuntimeException& )
        {
            throw;
        }
        catch( uno::Exception& )
        {
        }
    }
    else
    {
        uno::Reference < document::XStandaloneDocumentInfo > xBinary( _xFactory->createInstance(
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.document.BinaryStandaloneDocumentInfo" ) ) ), uno::UNO_QUERY );
        if ( xBinary.is() )
        {
            Copy( this, xBinary );
            xBinary->storeIntoURL( aURL );
            bOK = sal_True;
        }
    }

    if ( !bOK )
        throw task::ErrorCodeIOException( ::rtl::OUString(), uno::Reference< uno::XInterface >(), ERRCODE_IO_CANTWRITE );
}
