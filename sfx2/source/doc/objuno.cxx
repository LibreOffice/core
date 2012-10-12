/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


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
#include <com/sun/star/document/DocumentProperties.hpp>
#include <com/sun/star/document/XCompatWriterDocProperties.hpp>

#include <unotools/configmgr.hxx>
#include <unotools/bootstrap.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <osl/mutex.hxx>
#include <rtl/ustrbuf.hxx>
#include <vcl/svapp.hxx>

#include <tools/errcode.hxx>
#include <svl/cntwids.hrc>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <comphelper/sequenceasvector.hxx>
#include <comphelper/storagehelper.hxx>
#include <sot/storage.hxx>

#include <sfx2/objuno.hxx>
#include <sfx2/sfx.hrc>

#include <vector>
#include <algorithm>

#include "sfx2/sfxresid.hxx"
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

// The number of user defined fields handled by the evil XDocumentInfo
// interface. There are exactly 4. No more, no less.
#define FOUR 4

#define PROPERTY_UNBOUND 0
#define PROPERTY_MAYBEVOID ::com::sun::star::beans::PropertyAttribute::MAYBEVOID

static const SfxItemPropertyMapEntry* lcl_GetDocInfoPropertyMap()
{
    static SfxItemPropertyMapEntry aDocInfoPropertyMap_Impl[] =
    {
        { "Author"          , 6 , WID_FROM,           &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
        { "AutoloadEnabled" , 15, MID_DOCINFO_AUTOLOADENABLED, &::getBooleanCppuType(),   PROPERTY_UNBOUND, 0 },
        { "AutoloadSecs"    , 12, MID_DOCINFO_AUTOLOADSECS, &::getCppuType((const sal_Int32*)0),     PROPERTY_UNBOUND, 0 },
        { "AutoloadURL"     , 11, MID_DOCINFO_AUTOLOADURL, &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
    { "Category"            , 8 , MID_CATEGORY,           &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
    { "Company"         , 7 , MID_COMPANY,           &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
    { "Manager"         , 7 , MID_MANAGER,           &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
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
        { "PrintDate"       , 9 , MID_DOCINFO_PRINTDATE, &::getCppuType((const ::com::sun::star::util::DateTime*)0),PROPERTY_MAYBEVOID, 0 },
        { "PrintedBy"       , 9 , MID_DOCINFO_PRINTEDBY, &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
        { "Subject"         , 7 , MID_DOCINFO_SUBJECT, &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
        { "Template"        , 8 , MID_DOCINFO_TEMPLATE, &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
        { "TemplateFileName", 16, SID_TEMPLATE_NAME, &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
        { "TemplateDate"    , 12, MID_DOCINFO_TEMPLATEDATE, &::getCppuType((const ::com::sun::star::util::DateTime*)0),PROPERTY_MAYBEVOID, 0 },
        { "Title"           , 5 , WID_TITLE,          &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
        {0,0,0,0,0,0}
    };
    return aDocInfoPropertyMap_Impl;
}

static sal_uInt16 aDaysInMonth[12] = { 31, 28, 31, 30, 31, 30,
                                   31, 31, 30, 31, 30, 31 };

inline sal_uInt16 DaysInMonth( sal_uInt16 nMonth, sal_uInt16 nYear )
{
    if ( nMonth != 2 )
        return aDaysInMonth[nMonth-1];
    else
    {
        if ( (((nYear % 4) == 0) && ((nYear % 100) != 0)) ||
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

struct SfxExtendedItemPropertyMap : public SfxItemPropertyMapEntry
{
    ::com::sun::star::uno::Any aValue;
};

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
            catch ( const uno::Exception& ) {}
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
    catch ( const uno::Exception& ) {}
}

class MixedPropertySetInfo : public ::cppu::WeakImplHelper1< ::com::sun::star::beans::XPropertySetInfo >
{
    private:

        SfxItemPropertyMap  _aPropertyMap;
        ::rtl::OUString* _pUserKeys;
        uno::Reference<beans::XPropertySet> _xUDProps;

    public:

        MixedPropertySetInfo( const SfxItemPropertyMapEntry* pFixProps,
                             ::rtl::OUString* pUserKeys,
                             uno::Reference<beans::XPropertySet> xUDProps);

        virtual ~MixedPropertySetInfo();

        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property > SAL_CALL getProperties(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::beans::Property SAL_CALL getPropertyByName( const ::rtl::OUString& aName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL hasPropertyByName( const ::rtl::OUString& Name ) throw (::com::sun::star::uno::RuntimeException);
};

//-----------------------------------------------------------------------------

MixedPropertySetInfo::MixedPropertySetInfo(const SfxItemPropertyMapEntry* pFixProps,
                     ::rtl::OUString* pUserKeys,
                     uno::Reference<beans::XPropertySet> xUDProps)
    : _aPropertyMap( pFixProps )
    , _pUserKeys(pUserKeys)
    , _xUDProps(xUDProps)
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
    //todo: os: this ugly thing should be replaced
    const SfxItemPropertyMapEntry* pFixProp = lcl_GetDocInfoPropertyMap();

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

    // NB: this is really ugly:
    // The returned properties must _not_ include the 4 user-defined fields!
    // These are _not_ properties of the XDocumentInfo interface.
    // Some things rely on this, e.g. Copy would break otherwise.
    // This will have interesting consequences if someone expects to insert
    // a property with the same name as an user-defined key, but nobody
    // sane does that.
    uno::Sequence<beans::Property> udProps =
        _xUDProps->getPropertySetInfo()->getProperties();
    for (sal_Int32 i = 0; i < udProps.getLength(); ++i) {
        if (std::find(_pUserKeys, _pUserKeys+FOUR, udProps[i].Name)
            == _pUserKeys+FOUR) {
                // #i100027#: handles from udProps are not valid here
                udProps[i].Handle = -1;
                lProps.push_back(udProps[i]);
        }
    }

    return lProps.getAsConstList();
}

//-----------------------------------------------------------------------------

::com::sun::star::beans::Property SAL_CALL MixedPropertySetInfo::getPropertyByName(
    const ::rtl::OUString& sName )
    throw(::com::sun::star::beans::UnknownPropertyException,
          ::com::sun::star::uno::RuntimeException          )
{
    ::com::sun::star::beans::Property aProp;

    // search it as "fix" prop
    if( _aPropertyMap.hasPropertyByName( sName ) )
        return _aPropertyMap.getPropertyByName( sName );
    else
    // search it as "dynamic" prop
    return _xUDProps->getPropertySetInfo()->getPropertyByName(sName);
}

//-----------------------------------------------------------------------------

::sal_Bool SAL_CALL MixedPropertySetInfo::hasPropertyByName(const ::rtl::OUString& sName)
    throw(::com::sun::star::uno::RuntimeException)
{
    return _aPropertyMap.hasPropertyByName( sName ) ? // "fix" prop?
        sal_True :
        _xUDProps->getPropertySetInfo()->hasPropertyByName(sName); // "dynamic" prop?
}

//-----------------------------------------------------------------------------

struct SfxDocumentInfoObject_Impl
{
    ::osl::Mutex                        _aMutex;
    ::cppu::OInterfaceContainerHelper   _aDisposeContainer;

    sal_Bool            bDisposed;

    // this contains the names of the 4 user defined properties
    // which are accessible via the evil XDocumentInfo interface
    ::rtl::OUString m_UserDefined[FOUR];

    // the actual contents
    uno::Reference<document::XDocumentProperties> m_xDocProps;
    SfxItemPropertyMap      m_aPropertyMap;

    SfxDocumentInfoObject_Impl()
        : _aDisposeContainer( _aMutex )
        , bDisposed(sal_False)
        , m_xDocProps()
        , m_aPropertyMap( lcl_GetDocInfoPropertyMap() )
    {
        // the number of user fields is not changeable from the outside
        // we can't set it too high because every name/value pair will be written to the file (even if empty)
        // currently our dialog has only 4 user keys so 4 is still a reasonable number
    }

    /// the initialization function
    void Reset(uno::Reference<document::XDocumentProperties> xDocProps, ::rtl::OUString* pUserDefined = 0);
};

void SfxDocumentInfoObject_Impl::Reset(uno::Reference<document::XDocumentProperties> xDocProps, ::rtl::OUString* pUserDefined)
{
    if (pUserDefined == 0) {
        // NB: this is an ugly hack; the "Properties" ui dialog displays
        //     exactly 4 user-defined fields and expects these to be available
        //     (should be redesigned), but I do not want to do this in
        //     DocumentProperties; do it here instead
        uno::Reference<beans::XPropertyAccess> xPropAccess(
            xDocProps->getUserDefinedProperties(), uno::UNO_QUERY_THROW);
        uno::Reference<beans::XPropertyContainer> xPropContainer(
            xPropAccess, uno::UNO_QUERY_THROW);
        uno::Sequence< beans::PropertyValue >
            props = xPropAccess->getPropertyValues();
        sal_Int32 oldLength = props.getLength();
        if (oldLength < FOUR) {
            std::vector< ::rtl::OUString > names;
            for (sal_Int32 i = 0; i < oldLength; ++i) {
                names.push_back(props[i].Name);
            }
            const ::rtl::OUString sInfo(
                        SfxResId(STR_DOCINFO_INFOFIELD).toString());
            for (sal_Int32 i = oldLength; i < FOUR; ++i) {
                ::rtl::OUString sName(sInfo);
                sal_Int32 idx = sName.indexOfAsciiL("%1", 2);
                ::rtl::OUString name = (idx > 0)
                    ? sName.replaceAt(idx, 2, ::rtl::OUString::valueOf(i+1))
                    : sName + ::rtl::OUString::valueOf(i+1);
                while (std::find(names.begin(), names.end(), name)
                       != names.end()) {
                    name += ::rtl::OUString("'");
                }
                // FIXME there is a race condition here
                try {
                    xPropContainer->addProperty(name,
                        beans::PropertyAttribute::REMOVEABLE,
                        uno::makeAny(::rtl::OUString("")));
                } catch (const uno::RuntimeException&) {
                    throw;
                } catch (const uno::Exception&) {
                    // ignore
                }
            }
        }
        props = xPropAccess->getPropertyValues();
        for (sal_Int32 i = 0; i < FOUR; ++i) {
            m_UserDefined[i] = props[i].Name;
        }
    } else {
        std::copy(pUserDefined, pUserDefined+FOUR, m_UserDefined);
    }
    m_xDocProps = xDocProps;
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

// ::com::sun::star::lang::XInitialization:
void SAL_CALL
SfxDocumentInfoObject::initialize(const uno::Sequence< uno::Any > & aArguments)
    throw (uno::RuntimeException, uno::Exception)
{
    if (aArguments.getLength() >= 1) {
        uno::Any any = aArguments[0];
        uno::Reference<document::XDocumentProperties> xDoc;
        if (!(any >>= xDoc) || !xDoc.is()) throw lang::IllegalArgumentException(
            ::rtl::OUString(
                "SfxDocumentInfoObject::initialize: no XDocumentProperties given"),
                *this, 0);
        _pImp->Reset(xDoc);
    } else {
        throw lang::IllegalArgumentException(
            ::rtl::OUString(
                "SfxDocumentInfoObject::initialize: no argument given"),
                *this, 0);
    }
}

// ::com::sun::star::util::XCloneable:
uno::Reference<util::XCloneable> SAL_CALL
SfxDocumentInfoObject::createClone() throw (uno::RuntimeException)
{
    SfxDocumentInfoObject *pNew = new SfxDocumentInfoObject;
    uno::Reference< util::XCloneable >
        xCloneable(_pImp->m_xDocProps, uno::UNO_QUERY_THROW);
    uno::Reference<document::XDocumentProperties> xDocProps(
        xCloneable->createClone(), uno::UNO_QUERY_THROW);
    pNew->_pImp->Reset(xDocProps, _pImp->m_UserDefined);
    return pNew;
}

// ::com::sun::star::document::XDocumentProperties:
uno::Reference< document::XDocumentProperties > SAL_CALL
SfxDocumentInfoObject::getDocumentProperties()
    throw(::com::sun::star::uno::RuntimeException)
{
    return _pImp->m_xDocProps;
}

//-----------------------------------------------------------------------------

const SfxDocumentInfoObject& SfxDocumentInfoObject::operator=( const SfxDocumentInfoObject & rOther)
{
    uno::Reference< util::XCloneable >
        xCloneable(rOther._pImp->m_xDocProps, uno::UNO_QUERY_THROW);
    uno::Reference<document::XDocumentProperties> xDocProps(
        xCloneable->createClone(), uno::UNO_QUERY_THROW);
    _pImp->Reset(xDocProps, rOther._pImp->m_UserDefined);
    return *this;
}

//-----------------------------------------------------------------------------

void SAL_CALL SfxDocumentInfoObject::dispose() throw( ::com::sun::star::uno::RuntimeException )
{
    ::com::sun::star::lang::EventObject aEvent( (::cppu::OWeakObject *)this );
    _pImp->_aDisposeContainer.disposeAndClear( aEvent );
    ::osl::MutexGuard aGuard( _pImp->_aMutex );
    _pImp->m_xDocProps = 0;
    // NB: do not call m_xDocProps->dispose(), there could be other refs
    _pImp->bDisposed = sal_True;
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

    uno::Reference<beans::XPropertySet> xPropSet(
        _pImp->m_xDocProps->getUserDefinedProperties(), uno::UNO_QUERY_THROW);
    MixedPropertySetInfo* pInfo = new MixedPropertySetInfo( lcl_GetDocInfoPropertyMap(), _pImp->m_UserDefined, xPropSet);
    uno::Reference< beans::XPropertySetInfo > xInfo(
        static_cast< beans::XPropertySetInfo* >(pInfo), uno::UNO_QUERY_THROW);
    return xInfo;
}

//-----------------------------------------------------------------------------

void SAL_CALL  SfxDocumentInfoObject::setPropertyValue(const ::rtl::OUString& aPropertyName, const uno::Any& aValue) throw (
        uno::RuntimeException, beans::UnknownPropertyException,
        beans::PropertyVetoException, lang::IllegalArgumentException,
        lang::WrappedTargetException)
{
    const SfxItemPropertySimpleEntry* pEntry = _pImp->m_aPropertyMap.getByName( aPropertyName );
    // fix prop!
    if ( pEntry )
        setFastPropertyValue( pEntry->nWID, aValue );
    else
    // dynamic prop!
    {
        uno::Reference<beans::XPropertySet> xPropSet(
            _pImp->m_xDocProps->getUserDefinedProperties(), uno::UNO_QUERY_THROW);
        return xPropSet->setPropertyValue(aPropertyName, aValue);
    }
}

//-----------------------------------------------------------------------------

uno::Any  SAL_CALL  SfxDocumentInfoObject::getPropertyValue(const ::rtl::OUString& aPropertyName)  throw(
        uno::RuntimeException, beans::UnknownPropertyException,
        lang::WrappedTargetException)
{
    const SfxItemPropertySimpleEntry* pEntry = _pImp->m_aPropertyMap.getByName( aPropertyName );
    // fix prop!
    if ( pEntry )
        return getFastPropertyValue( pEntry->nWID );
    else
    // dynamic prop!
    {
        uno::Reference<beans::XPropertySet> xPropSet(
            _pImp->m_xDocProps->getUserDefinedProperties(), uno::UNO_QUERY_THROW);
        return xPropSet->getPropertyValue(aPropertyName);
    }
}

sal_Bool SAL_CALL SfxDocumentInfoObject::isModified() throw(::com::sun::star::uno::RuntimeException)
{
    uno::Reference<util::XModifiable> xModif(
            _pImp->m_xDocProps, uno::UNO_QUERY_THROW);
    return xModif->isModified();
}

void SAL_CALL SfxDocumentInfoObject::setModified( sal_Bool bModified )
        throw (::com::sun::star::beans::PropertyVetoException, ::com::sun::star::uno::RuntimeException)
{
    uno::Reference<util::XModifiable> xModif(
            _pImp->m_xDocProps, uno::UNO_QUERY_THROW);
    return xModif->setModified(bModified);
}

void SAL_CALL SfxDocumentInfoObject::addModifyListener( const uno::Reference< util::XModifyListener >& xListener) throw( uno::RuntimeException )
{
    uno::Reference<util::XModifiable> xModif(
            _pImp->m_xDocProps, uno::UNO_QUERY_THROW);
    return xModif->addModifyListener(xListener);
}

void SAL_CALL SfxDocumentInfoObject::removeModifyListener( const uno::Reference< util::XModifyListener >& xListener) throw( uno::RuntimeException )
{
    uno::Reference<util::XModifiable> xModif(
            _pImp->m_xDocProps, uno::UNO_QUERY_THROW);
    return xModif->removeModifyListener(xListener);
}

//-----------------------------------------------------------------------------

void SAL_CALL  SfxDocumentInfoObject::addPropertyChangeListener(const ::rtl::OUString&, const uno::Reference< beans::XPropertyChangeListener > & ) throw(
        uno::RuntimeException, beans::UnknownPropertyException,
        lang::WrappedTargetException)
{}

//-----------------------------------------------------------------------------

void SAL_CALL  SfxDocumentInfoObject::removePropertyChangeListener(const ::rtl::OUString&, const uno::Reference< beans::XPropertyChangeListener > & ) throw(
        uno::RuntimeException, beans::UnknownPropertyException,
        lang::WrappedTargetException)
{}

//-----------------------------------------------------------------------------

void SAL_CALL  SfxDocumentInfoObject::addVetoableChangeListener(const ::rtl::OUString&, const uno::Reference< beans::XVetoableChangeListener > & ) throw(
        uno::RuntimeException, beans::UnknownPropertyException,
        lang::WrappedTargetException)
{}

//-----------------------------------------------------------------------------

void SAL_CALL  SfxDocumentInfoObject::removeVetoableChangeListener(const ::rtl::OUString&, const uno::Reference< beans::XVetoableChangeListener > & ) throw(
        uno::RuntimeException, beans::UnknownPropertyException,
        lang::WrappedTargetException)
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
    // clash with "fix" properties ?
    sal_Bool bFixProp = _pImp->m_aPropertyMap.getByName( sName ) != 0;
    if ( bFixProp )
    {
        ::rtl::OUStringBuffer sMsg(256);
        sMsg.appendAscii(RTL_CONSTASCII_STRINGPARAM("The property \""));
        sMsg.append(sName);
        sMsg.appendAscii(RTL_CONSTASCII_STRINGPARAM("\" "));
        if ( bFixProp )
            sMsg.appendAscii(RTL_CONSTASCII_STRINGPARAM(" already exists as a fix property. Please have a look into the IDL documentation of the DocumentInfo service."));

        throw ::com::sun::star::beans::PropertyExistException(
                sMsg.makeStringAndClear(),
                static_cast< ::cppu::OWeakObject* >(this));
    }

    uno::Reference<beans::XPropertyContainer> xPropSet(
        _pImp->m_xDocProps->getUserDefinedProperties(), uno::UNO_QUERY_THROW);
    return xPropSet->addProperty(sName, nAttributes, aDefaultValue);
}

void SAL_CALL SfxDocumentInfoObject::removeProperty(const ::rtl::OUString& sName)
    throw(::com::sun::star::beans::UnknownPropertyException,
          ::com::sun::star::beans::NotRemoveableException  ,
          ::com::sun::star::uno::RuntimeException          )
{
    // clash with "fix" properties ?
    sal_Bool bFixProp = _pImp->m_aPropertyMap.getByName( sName ) != 0;
    if ( bFixProp )
    {
        ::rtl::OUStringBuffer sMsg(256);
        sMsg.appendAscii(RTL_CONSTASCII_STRINGPARAM("The property \""));
        sMsg.append(sName);
        sMsg.appendAscii(RTL_CONSTASCII_STRINGPARAM("\" cant be removed. Its a fix property of the DocumentInfo service."));

        throw ::com::sun::star::beans::NotRemoveableException(
                sMsg.makeStringAndClear(),
                static_cast< ::cppu::OWeakObject* >(this));
    }

    uno::Reference<beans::XPropertyContainer> xPropSet(
        _pImp->m_xDocProps->getUserDefinedProperties(), uno::UNO_QUERY_THROW);
    return xPropSet->removeProperty(sName);
}

sal_Bool equalsDateTime( const util::DateTime& D1, const util::DateTime& D2 )
{
    return D1.HundredthSeconds == D2.HundredthSeconds &&
           D1.Seconds == D2.Seconds &&
           D1.Minutes == D2.Minutes &&
           D1.Hours == D2.Hours &&
           D1.Day == D2.Day &&
           D1.Month == D2.Month &&
           D1.Year == D2.Year;
}

void SAL_CALL  SfxDocumentInfoObject::setFastPropertyValue(sal_Int32 nHandle, const ::com::sun::star::uno::Any& aValue) throw(
        uno::RuntimeException, beans::UnknownPropertyException,
        beans::PropertyVetoException, lang::IllegalArgumentException,
        lang::WrappedTargetException)
{
    // Attention: Only fix properties should be provided by this method.
    // Dynamic properties has no handle in real ... because it cant be used inside multithreaded environments :-)

    ::osl::ClearableMutexGuard aGuard( _pImp->_aMutex );

    if ( aValue.getValueType() == ::getCppuType((const ::rtl::OUString*)0) )
    {
        ::rtl::OUString sTemp ;
        aValue >>= sTemp ;
        switch ( nHandle )
        {
            case SID_APPLICATION :
                _pImp->m_xDocProps->setGenerator(sTemp);
                break;
            case WID_FROM :
            {
                if ( _pImp->m_xDocProps->getAuthor() != sTemp )
                    _pImp->m_xDocProps->setAuthor(sTemp);
                break;
            }
            case MID_DOCINFO_PRINTEDBY:
                if ( _pImp->m_xDocProps->getPrintedBy() != sTemp )
                    _pImp->m_xDocProps->setPrintedBy(sTemp);
                break;
            case MID_DOCINFO_MODIFICATIONAUTHOR:
                if ( _pImp->m_xDocProps->getModifiedBy() != sTemp )
                    _pImp->m_xDocProps->setModifiedBy(sTemp);
                break;
            case WID_TITLE :
            {
                if ( _pImp->m_xDocProps->getTitle() != sTemp )
                    _pImp->m_xDocProps->setTitle(sTemp);
                break;
            }
            case MID_DOCINFO_SUBJECT :
                if ( _pImp->m_xDocProps->getSubject() != sTemp )
                    _pImp->m_xDocProps->setSubject(sTemp);
                break;
            case WID_KEYWORDS :
                {
                    _pImp->m_xDocProps->setKeywords(
                        ::comphelper::string::convertCommaSeparated(sTemp));
                }
                break;
            case MID_DOCINFO_TEMPLATE:
                if ( _pImp->m_xDocProps->getTemplateName() != sTemp )
                    _pImp->m_xDocProps->setTemplateName(sTemp);
                break;
            case SID_TEMPLATE_NAME:
                if ( _pImp->m_xDocProps->getTemplateURL() != sTemp )
                    _pImp->m_xDocProps->setTemplateURL(sTemp);
                break;
            case MID_DOCINFO_DESCRIPTION:
                if ( _pImp->m_xDocProps->getDescription() != sTemp )
                    _pImp->m_xDocProps->setDescription(sTemp);
                break;
            case MID_DOCINFO_AUTOLOADURL:
                if ( _pImp->m_xDocProps->getAutoloadURL() != sTemp )
                    _pImp->m_xDocProps->setAutoloadURL(sTemp);
                break;
            case MID_DOCINFO_DEFAULTTARGET:
                if ( _pImp->m_xDocProps->getDefaultTarget() != sTemp )
                    _pImp->m_xDocProps->setDefaultTarget(sTemp);
                break;
               case MID_CATEGORY:
               case MID_MANAGER:
               case MID_COMPANY:
                   {
                       uno::Reference< document::XCompatWriterDocProperties > xWriterProps( _pImp->m_xDocProps, uno::UNO_QUERY  );
                       if ( xWriterProps.is() )
                       {
                           if ( nHandle ==  MID_CATEGORY )
                               xWriterProps->setCategory( sTemp );
                           else if ( nHandle ==  MID_MANAGER )
                               xWriterProps->setManager( sTemp );
                           else
                               xWriterProps->setCompany( sTemp );
                           break;
                       }
                   }
            default:
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
                if ( !equalsDateTime(_pImp->m_xDocProps->getCreationDate(), aTemp ) )
                {
                    _pImp->m_xDocProps->setCreationDate(aTemp);
                }
                break;
            }
            case WID_DATE_MODIFIED :
            {
                if ( !equalsDateTime(_pImp->m_xDocProps->getModificationDate(), aTemp ) )
                {
                    _pImp->m_xDocProps->setModificationDate(aTemp);
                }
                break;
            }
            case MID_DOCINFO_PRINTDATE :
            {
                if ( !equalsDateTime(_pImp->m_xDocProps->getPrintDate(), aTemp ) )
                {
                    _pImp->m_xDocProps->setPrintDate(aTemp);
                }
                break;
            }
            case MID_DOCINFO_TEMPLATEDATE :
            {
                if ( !equalsDateTime(_pImp->m_xDocProps->getTemplateDate(), aTemp ) )
                {
                    _pImp->m_xDocProps->setTemplateDate(aTemp);
                }
                break;
            }
            default:
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
                // NB: this property does not exist any more
                //     it is emulated as enabled iff delay > 0
                if ( bBoolVal && (0 == _pImp->m_xDocProps->getAutoloadSecs()) ) {
                    _pImp->m_xDocProps->setAutoloadSecs(60); // default
                } else if ( !bBoolVal && (0 != _pImp->m_xDocProps->getAutoloadSecs()) ) {
                    _pImp->m_xDocProps->setAutoloadSecs(0);
                    _pImp->m_xDocProps->setAutoloadURL(::rtl::OUString(""));
                }
                break;
            default:
                break;
        }
    }
    else if ( aValue.getValueType() == ::getCppuType((const sal_Int32*)0) )
    {
        sal_Int32 nIntVal = 0;
        aValue >>= nIntVal ;
        switch ( nHandle )
        {
            case MID_DOCINFO_AUTOLOADSECS:
                if ( nIntVal != _pImp->m_xDocProps->getAutoloadSecs())
                    _pImp->m_xDocProps->setAutoloadSecs(nIntVal);
                break;
            case MID_DOCINFO_EDITTIME:
                if ( nIntVal != _pImp->m_xDocProps->getEditingDuration())
                    _pImp->m_xDocProps->setEditingDuration(nIntVal);
                break;
            default:
                break;
        }
    }
    else if ( aValue.getValueType() == ::getCppuType((const sal_Int16*)0) )
    {
        short nIntVal = 0;
        aValue >>= nIntVal ;
        switch ( nHandle )
        {
            case MID_DOCINFO_REVISION:
                if ( nIntVal != _pImp->m_xDocProps->getEditingCycles())
                    _pImp->m_xDocProps->setEditingCycles(nIntVal);
                break;
            default:
                break;
        }
    }
    else if ( aValue.getValueType() == ::getCppuType((const uno::Sequence< beans::NamedValue >*)0) )
    {
        if ( nHandle == MID_DOCINFO_STATISTIC )
        {
            uno::Sequence < beans::NamedValue > aData;
            aValue >>= aData;
            {
                _pImp->m_xDocProps->setDocumentStatistics(aData);
            }
        }
    }
    else if ( aValue.getValueType() == ::getCppuType((const lang::Locale*)0) )
    {
        if ( nHandle == MID_DOCINFO_CHARLOCALE )
        {
            lang::Locale aLocale;
            aValue >>= aLocale;
            lang::Locale oldLocale = _pImp->m_xDocProps->getLanguage();
            if ( aLocale.Language != oldLocale.Language ||
                 aLocale.Country  != oldLocale.Country  ||
                 aLocale.Variant  != oldLocale.Variant   )
            {
                _pImp->m_xDocProps->setLanguage(aLocale);
            }
        }
    }
}

//-----------------------------------------------------------------------------

::com::sun::star::uno::Any SAL_CALL  SfxDocumentInfoObject::getFastPropertyValue(sal_Int32 nHandle) throw(
        uno::RuntimeException, beans::UnknownPropertyException,
        lang::WrappedTargetException)
{
    // Attention: Only fix properties should be provided by this method.
    // Dynamic properties has no handle in real ... because it cant be used inside multithreaded environments :-)

    ::osl::MutexGuard aGuard( _pImp->_aMutex );
    ::com::sun::star::uno::Any aValue;
    switch ( nHandle )
    {
        case SID_APPLICATION :
            aValue <<= _pImp->m_xDocProps->getGenerator();
            break;
        case WID_CONTENT_TYPE :
// FIXME this is not available anymore
            aValue <<= ::rtl::OUString();
            break;
        case MID_DOCINFO_REVISION :
            aValue <<= _pImp->m_xDocProps->getEditingCycles();
            break;
        case MID_DOCINFO_EDITTIME :
            aValue <<= _pImp->m_xDocProps->getEditingDuration();
            break;
        case WID_FROM :
            aValue <<= _pImp->m_xDocProps->getAuthor();
            break;
        case WID_DATE_CREATED :
            if ( IsValidDateTime( _pImp->m_xDocProps->getCreationDate() ) )
                aValue <<= _pImp->m_xDocProps->getCreationDate();
            break;
        case WID_TITLE :
            aValue <<= _pImp->m_xDocProps->getTitle();
            break;
        case MID_DOCINFO_SUBJECT:
            aValue <<= _pImp->m_xDocProps->getSubject();
            break;
        case MID_DOCINFO_MODIFICATIONAUTHOR:
            aValue <<= _pImp->m_xDocProps->getModifiedBy();
            break;
        case WID_DATE_MODIFIED :
            if ( IsValidDateTime( _pImp->m_xDocProps->getModificationDate() ) )
                aValue <<= _pImp->m_xDocProps->getModificationDate();
            break;
        case MID_DOCINFO_PRINTEDBY:
            aValue <<= _pImp->m_xDocProps->getPrintedBy();
            break;
        case MID_DOCINFO_PRINTDATE:
            if ( IsValidDateTime( _pImp->m_xDocProps->getPrintDate() ) )
                aValue <<= _pImp->m_xDocProps->getPrintDate();
            break;
        case WID_KEYWORDS :
            aValue <<= ::comphelper::string::convertCommaSeparated(
                _pImp->m_xDocProps->getKeywords());
            break;
        case MID_DOCINFO_DESCRIPTION:
            aValue <<= _pImp->m_xDocProps->getDescription();
            break;
        case MID_DOCINFO_TEMPLATE:
            aValue <<= _pImp->m_xDocProps->getTemplateName();
            break;
        case SID_TEMPLATE_NAME:
            aValue <<= _pImp->m_xDocProps->getTemplateURL();
            break;
        case MID_DOCINFO_TEMPLATEDATE:
            if ( IsValidDateTime( _pImp->m_xDocProps->getTemplateDate() ) )
                aValue <<= _pImp->m_xDocProps->getTemplateDate();
            break;
        case MID_DOCINFO_AUTOLOADENABLED:
            aValue <<= static_cast<sal_Bool>
                        (   (_pImp->m_xDocProps->getAutoloadSecs() != 0)
                        || !_pImp->m_xDocProps->getAutoloadURL().isEmpty());
            break;
        case MID_DOCINFO_AUTOLOADURL:
            aValue <<= _pImp->m_xDocProps->getAutoloadURL();
            break;
        case MID_DOCINFO_AUTOLOADSECS:
            aValue <<= _pImp->m_xDocProps->getAutoloadSecs();
            break;
        case MID_DOCINFO_DEFAULTTARGET:
            aValue <<= _pImp->m_xDocProps->getDefaultTarget();
            break;
        case MID_DOCINFO_STATISTIC:
            aValue <<= _pImp->m_xDocProps->getDocumentStatistics();
            break;
        case MID_DOCINFO_CHARLOCALE:
            aValue <<= _pImp->m_xDocProps->getLanguage();
            break;
        case MID_CATEGORY:
        case MID_MANAGER:
        case MID_COMPANY:
            {
                uno::Reference< document::XCompatWriterDocProperties > xWriterProps( _pImp->m_xDocProps, uno::UNO_QUERY  );
                if ( xWriterProps.is() )
                {
                    if ( nHandle ==  MID_CATEGORY )
                        aValue <<= xWriterProps->getCategory();
                    else if ( nHandle ==  MID_MANAGER )
                        aValue <<= xWriterProps->getManager();
                    else
                        aValue <<= xWriterProps->getCompany();
                        break;
               }
           }

        default:
            aValue <<= ::rtl::OUString();
            break;
    }

    return aValue;
}

//-----------------------------------------------------------------------------

sal_Int16 SAL_CALL  SfxDocumentInfoObject::getUserFieldCount() throw( ::com::sun::star::uno::RuntimeException )
{
    return FOUR;
}

//-----------------------------------------------------------------------------

::rtl::OUString SAL_CALL  SfxDocumentInfoObject::getUserFieldName(sal_Int16 nIndex) throw( ::com::sun::star::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( _pImp->_aMutex );
    if (nIndex < FOUR)
        return _pImp->m_UserDefined[nIndex];
    else
        return ::rtl::OUString();
}

//-----------------------------------------------------------------------------

::rtl::OUString SAL_CALL  SfxDocumentInfoObject::getUserFieldValue(sal_Int16 nIndex) throw( ::com::sun::star::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( _pImp->_aMutex );
    if (nIndex < FOUR) {
        ::rtl::OUString name = _pImp->m_UserDefined[nIndex];
        uno::Reference<beans::XPropertySet> xPropSet(
            _pImp->m_xDocProps->getUserDefinedProperties(), uno::UNO_QUERY_THROW);
        ::rtl::OUString val;
        try {
            xPropSet->getPropertyValue(name) >>= val;
            return val;
        } catch (const uno::RuntimeException &) {
            throw;
        } catch (const uno::Exception &) {
            return ::rtl::OUString(); // ignore
        }
    } else
        return ::rtl::OUString();
}

//-----------------------------------------------------------------------------

void  SAL_CALL SfxDocumentInfoObject::setUserFieldName(sal_Int16 nIndex, const ::rtl::OUString& aName ) throw( ::com::sun::star::uno::RuntimeException )
{
    ::osl::ClearableMutexGuard aGuard( _pImp->_aMutex );
    if (nIndex < FOUR) // yes, four!
    {
        // FIXME this is full of race conditions because the PropertyBag
        // can be accessed from clients of the DocumentProperties!
        ::rtl::OUString name = _pImp->m_UserDefined[nIndex];
        if (name != aName) {
            uno::Reference<beans::XPropertySet> xPropSet(
                _pImp->m_xDocProps->getUserDefinedProperties(),
                uno::UNO_QUERY_THROW);
            uno::Reference<beans::XPropertyContainer> xPropContainer(
                _pImp->m_xDocProps->getUserDefinedProperties(),
                uno::UNO_QUERY_THROW);
            uno::Any value;
            try {
                value = xPropSet->getPropertyValue(name);
                xPropContainer->removeProperty(name);
                xPropContainer->addProperty(aName,
                    beans::PropertyAttribute::REMOVEABLE, value);
                _pImp->m_UserDefined[nIndex] = aName;
            } catch (const beans::UnknownPropertyException&) {
                try {
                    xPropContainer->addProperty(aName,
                        beans::PropertyAttribute::REMOVEABLE,
                        uno::makeAny(::rtl::OUString("")));
                    _pImp->m_UserDefined[nIndex] = aName;
                } catch (const beans::PropertyExistException&) {
                    _pImp->m_UserDefined[nIndex] = aName;
                    // ignore
                }
            } catch (const beans::PropertyExistException&) {
                try {
                    xPropContainer->addProperty(name,
                        beans::PropertyAttribute::REMOVEABLE, value);
                } catch (const beans::PropertyExistException&) {
                    // bugger...
                }
            } catch (const uno::RuntimeException &) {
                throw;
            } catch (const uno::Exception &) {
                // ignore everything else; xPropSet _may_ be corrupted
            }
        }
    }
}

//-----------------------------------------------------------------------------

void SAL_CALL  SfxDocumentInfoObject::setUserFieldValue( sal_Int16 nIndex, const ::rtl::OUString& aValue ) throw( ::com::sun::star::uno::RuntimeException )
{
    ::osl::ClearableMutexGuard aGuard( _pImp->_aMutex );
    if (nIndex < FOUR) // yes, four!
    {
        ::rtl::OUString name = _pImp->m_UserDefined[nIndex];
        uno::Reference<beans::XPropertySet> xPropSet(
            _pImp->m_xDocProps->getUserDefinedProperties(),
            uno::UNO_QUERY_THROW);
        uno::Reference<beans::XPropertyContainer> xPropContainer(
            _pImp->m_xDocProps->getUserDefinedProperties(),
            uno::UNO_QUERY_THROW);
        uno::Any aAny;
        aAny <<= aValue;
        try {
            uno::Any value = xPropSet->getPropertyValue(name);
            if (value != aAny) {
                xPropSet->setPropertyValue(name, aAny);
            }
        } catch (const beans::UnknownPropertyException&) {
            try {
                // someone removed it, add it back again
                xPropContainer->addProperty(name,
                    beans::PropertyAttribute::REMOVEABLE, aAny);
            } catch (const uno::RuntimeException &) {
                throw;
            } catch (const uno::Exception &) {
                // ignore everything else
            }
        } catch (const uno::RuntimeException &) {
            throw;
        } catch (const uno::Exception &) {
            // ignore everything else
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
    uno::Reference< lang::XInitialization > xDocProps(
        document::DocumentProperties::create(comphelper::getComponentContext(_xFactory)),
        uno::UNO_QUERY_THROW);
    uno::Any a;
    a <<= xDocProps;
    uno::Sequence<uno::Any> args(1);
    args[0] = a;
    initialize(args);
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
        SolarMutexGuard aSolarGuard;
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
    uno::Reference< document::XDocumentProperties > xDocProps(
        document::DocumentProperties::create(comphelper::getComponentContext(_xFactory)) );
    _pImp->Reset(xDocProps);
    aGuard.clear();

    uno::Reference< embed::XStorage > xStorage = GetStorage_Impl( aURL, sal_False, _xFactory );
    if ( xStorage.is() )
    {
        try
        {
            uno::Sequence<beans::PropertyValue> medium(2);
            medium[0].Name = ::rtl::OUString("DocumentBaseURL");
            medium[0].Value <<= aURL;
            medium[1].Name = ::rtl::OUString("URL");
            medium[1].Value <<= aURL;
            _pImp->m_xDocProps->loadFromStorage(xStorage, medium);
            _pImp->Reset(_pImp->m_xDocProps);
            bOK = sal_True;
        }
        catch( const uno::Exception& )
        {
        }
    }
    else
    {
        uno::Reference < document::XStandaloneDocumentInfo > xBinary( _xFactory->createInstance(
            ::rtl::OUString("com.sun.star.document.BinaryStandaloneDocumentInfo"  ) ), uno::UNO_QUERY );
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
            uno::Sequence<beans::PropertyValue> medium(2);
            medium[0].Name = ::rtl::OUString("DocumentBaseURL");
            medium[0].Value <<= aURL;
            medium[1].Name = ::rtl::OUString("URL");
            medium[1].Value <<= aURL;

            _pImp->m_xDocProps->storeToStorage(xStorage, medium);
            bOK = sal_True;
        }
        catch( const io::IOException & )
        {
            throw;
        }
        catch( const uno::RuntimeException& )
        {
            throw;
        }
        catch( const uno::Exception& )
        {
        }
    }
    else
    {
        uno::Reference < document::XStandaloneDocumentInfo > xBinary( _xFactory->createInstance(
            ::rtl::OUString("com.sun.star.document.BinaryStandaloneDocumentInfo"  ) ), uno::UNO_QUERY );
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
