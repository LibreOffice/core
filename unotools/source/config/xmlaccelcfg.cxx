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


#include <unotools/xmlaccelcfg.hxx>

#include <vector>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <cppuhelper/implbase1.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::xml::sax;

using ::rtl::OUString;

#define ELEMENT_ACCELERATORLIST     "acceleratorlist"
#define ELEMENT_ACCELERATORITEM     "item"

#define ATTRIBUTE_KEYCODE           "code"
#define ATTRIBUTE_MODIFIER          "modifier"
#define ATTRIBUTE_URL               "url"

#define ATTRIBUTE_TYPE_CDATA        "CDATA"

namespace {

struct AttributeListImpl_impl;
class AttributeListImpl : public ::cppu::WeakImplHelper1< ::com::sun::star::xml::sax::XAttributeList >
{
protected:
    ~AttributeListImpl();

public:
    AttributeListImpl();
    AttributeListImpl( const AttributeListImpl & );

public:
    virtual sal_Int16 SAL_CALL getLength(void) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString  SAL_CALL getNameByIndex(sal_Int16 i) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString  SAL_CALL getTypeByIndex(sal_Int16 i) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString  SAL_CALL getTypeByName(const ::rtl::OUString& aName) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString  SAL_CALL getValueByIndex(sal_Int16 i) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString  SAL_CALL getValueByName(const ::rtl::OUString& aName) throw (::com::sun::star::uno::RuntimeException);

public:
    void addAttribute( const ::rtl::OUString &sName , const ::rtl::OUString &sType , const ::rtl::OUString &sValue );

private:
    struct AttributeListImpl_impl *m_pImpl;
};

struct TagAttribute
{
    TagAttribute(){}
    TagAttribute( const OUString &aName, const OUString &aType , const OUString &aValue )
    {
        sName   = aName;
        sType   = aType;
        sValue  = aValue;
    }

    OUString sName;
    OUString sType;
    OUString sValue;
};

struct AttributeListImpl_impl
{
    AttributeListImpl_impl()
    {
        // performance improvement during adding
        vecAttribute.reserve(20);
    }
    ::std::vector<struct TagAttribute> vecAttribute;
};



sal_Int16 SAL_CALL AttributeListImpl::getLength(void) throw (RuntimeException)
{
    return sal::static_int_cast< sal_Int16 >(m_pImpl->vecAttribute.size());
}


AttributeListImpl::AttributeListImpl( const AttributeListImpl &r ) :
    cppu::WeakImplHelper1<com::sun::star::xml::sax::XAttributeList>(r)
{
    m_pImpl = new AttributeListImpl_impl;
    *m_pImpl = *(r.m_pImpl);
}

OUString AttributeListImpl::getNameByIndex(sal_Int16 i) throw (RuntimeException)
{
    if( i < sal::static_int_cast<sal_Int16>(m_pImpl->vecAttribute.size()) ) {
        return m_pImpl->vecAttribute[i].sName;
    }
    return OUString();
}


OUString AttributeListImpl::getTypeByIndex(sal_Int16 i) throw (RuntimeException)
{
    if( i < sal::static_int_cast<sal_Int16>(m_pImpl->vecAttribute.size()) ) {
        return m_pImpl->vecAttribute[i].sType;
    }
    return OUString();
}

OUString AttributeListImpl::getValueByIndex(sal_Int16 i) throw (RuntimeException)
{
    if( i < sal::static_int_cast<sal_Int16>(m_pImpl->vecAttribute.size()) ) {
        return m_pImpl->vecAttribute[i].sValue;
    }
    return OUString();

}

OUString AttributeListImpl::getTypeByName( const OUString& sName ) throw (RuntimeException)
{
    ::std::vector<struct TagAttribute>::iterator ii = m_pImpl->vecAttribute.begin();

    for( ; ii != m_pImpl->vecAttribute.end() ; ++ii  ) {
        if( (*ii).sName == sName ) {
            return (*ii).sType;
        }
    }
    return OUString();
}

OUString AttributeListImpl::getValueByName(const OUString& sName) throw (RuntimeException)
{
    ::std::vector<struct TagAttribute>::iterator ii = m_pImpl->vecAttribute.begin();

    for( ; ii != m_pImpl->vecAttribute.end() ; ++ii ) {
        if( (*ii).sName == sName ) {
            return (*ii).sValue;
        }
    }
    return OUString();
}


AttributeListImpl::AttributeListImpl()
{
    m_pImpl = new AttributeListImpl_impl;
}



AttributeListImpl::~AttributeListImpl()
{
    delete m_pImpl;
}


void AttributeListImpl::addAttribute(   const OUString &sName ,
                                        const OUString &sType ,
                                        const OUString &sValue )
{
    m_pImpl->vecAttribute.push_back( TagAttribute( sName , sType , sValue ) );
}

} // anonymous namespace

Any SAL_CALL OReadAccelatorDocumentHandler::queryInterface( const Type & rType ) throw( RuntimeException )
{
    Any a = ::cppu::queryInterface( rType ,(static_cast< XDocumentHandler* >(this)));
    if ( a.hasValue() )
        return a;
    else
        return OWeakObject::queryInterface( rType );
}

void SAL_CALL OReadAccelatorDocumentHandler::ignorableWhitespace(
    const OUString& )
throw( SAXException, RuntimeException )
{
}

void SAL_CALL OReadAccelatorDocumentHandler::processingInstruction(
    const OUString&, const OUString& )
throw( SAXException, RuntimeException )
{
}

void SAL_CALL OReadAccelatorDocumentHandler::setDocumentLocator(
    const Reference< XLocator > &xLocator)
throw(  SAXException, RuntimeException )
{
    m_xLocator = xLocator;
}

::rtl::OUString OReadAccelatorDocumentHandler::getErrorLineString()
{
    char buffer[32];

    if ( m_xLocator.is() )
    {
        return OUString::createFromAscii( buffer );
    }
    else
        return OUString();
}

void SAL_CALL OReadAccelatorDocumentHandler::startDocument(void)
    throw ( SAXException, RuntimeException )
{
}

void SAL_CALL OReadAccelatorDocumentHandler::endDocument(void)
    throw( SAXException, RuntimeException )
{
    if ( m_nElementDepth > 0 )
    {
        OUString aErrorMessage = getErrorLineString();
        aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "A closing element is missing!" ));
        throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
    }
}


void SAL_CALL OReadAccelatorDocumentHandler::startElement(
    const OUString& aElementName, const Reference< XAttributeList > &xAttrList )
throw( SAXException, RuntimeException )
{
    m_nElementDepth++;

    if ( aElementName == ELEMENT_ACCELERATORLIST )
    {
        // acceleratorlist
        if ( m_bAcceleratorMode )
        {
            OUString aErrorMessage = getErrorLineString();
            aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "Accelerator list used twice!" ));
            throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
        }
        else
            m_bAcceleratorMode = sal_True;
    }
    else if ( aElementName == ELEMENT_ACCELERATORITEM )
    {
        // accelerator item
        if ( !m_bAcceleratorMode )
        {
            OUString aErrorMessage = getErrorLineString();
            aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "Accelerator list element has to be used before!" ));
            throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
        }
        else
        {
            // read accelerator item
            m_bItemCloseExpected = sal_True;

            SvtAcceleratorConfigItem aItem;

            // read attributes for accelerator
            for ( sal_Int16 i=0; i< xAttrList->getLength(); i++ )
            {
                OUString aName = xAttrList->getNameByIndex( i );
                OUString aValue = xAttrList->getValueByIndex( i );

                if ( aName == ATTRIBUTE_URL )
                    aItem.aCommand = aValue;
                else if ( aName == ATTRIBUTE_MODIFIER )
                    aItem.nModifier = (sal_uInt16)aValue.toInt32();
                else if ( aName == ATTRIBUTE_KEYCODE )
                    aItem.nCode = (sal_uInt16)aValue.toInt32();
            }

            m_aReadAcceleratorList.push_back( aItem );
        }
    }
    else
    {
        OUString aErrorMessage = getErrorLineString();
        aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "Unknown element found!" ));
        throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
    }
}


void SAL_CALL OReadAccelatorDocumentHandler::characters(const rtl::OUString&)
throw(  SAXException, RuntimeException )
{
}


void SAL_CALL OReadAccelatorDocumentHandler::endElement( const OUString& aName )
    throw( SAXException, RuntimeException )
{
    m_nElementDepth--;

    if ( aName == ELEMENT_ACCELERATORLIST )
    {
        // acceleratorlist
        if ( !m_bAcceleratorMode )
        {
            OUString aErrorMessage = getErrorLineString();
            aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "Accelerator list used twice!" ));
            throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
        }
    }
    else if ( aName == ELEMENT_ACCELERATORITEM )
    {
        if ( !m_bItemCloseExpected )
        {
            OUString aErrorMessage = getErrorLineString();
            aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "Closing accelerator item element expected!" ));
            throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
        }
    }
    else
    {
        OUString aErrorMessage = getErrorLineString();
        aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "Unknown closing element found!" ));
        throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
    }
}

// ------------------------------------------------------------------

OWriteAccelatorDocumentHandler::OWriteAccelatorDocumentHandler(
    const SvtAcceleratorItemList& aWriteAcceleratorList, Reference< XDocumentHandler > xDocumentHandler ) :
    m_xWriteDocumentHandler( xDocumentHandler ),
    m_aWriteAcceleratorList( aWriteAcceleratorList )
{
    m_aAttributeType = OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_TYPE_CDATA ));
}

OWriteAccelatorDocumentHandler::~OWriteAccelatorDocumentHandler()
{
}

void OWriteAccelatorDocumentHandler::WriteAcceleratorDocument()
    throw ( SAXException, RuntimeException )
{
    AttributeListImpl* pList = new AttributeListImpl;
    Reference< XAttributeList > rList( (XAttributeList *)pList , UNO_QUERY );

    m_xWriteDocumentHandler->startDocument();
    m_xWriteDocumentHandler->startElement( OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_ACCELERATORLIST )), rList );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );

    std::list< SvtAcceleratorConfigItem>::const_iterator p;
    for ( p = m_aWriteAcceleratorList.begin(); p != m_aWriteAcceleratorList.end(); ++p )
        WriteAcceleratorItem( *p );

    m_xWriteDocumentHandler->endElement( OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_ACCELERATORLIST )) );
    m_xWriteDocumentHandler->endDocument();
}

void OWriteAccelatorDocumentHandler::WriteAcceleratorItem(
    const SvtAcceleratorConfigItem& aAcceleratorItem )
    throw( SAXException, RuntimeException )
{
    AttributeListImpl* pAcceleratorAttributes = new AttributeListImpl;
    Reference< XAttributeList > xAcceleratorAttrList( (XAttributeList *)pAcceleratorAttributes , UNO_QUERY );

    // set attributes
    pAcceleratorAttributes->addAttribute(
        OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_KEYCODE )),
        m_aAttributeType,
        OUString::valueOf( aAcceleratorItem.nCode ));

    pAcceleratorAttributes->addAttribute(
        OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_MODIFIER )),
        m_aAttributeType,
        OUString::valueOf( aAcceleratorItem.nModifier ));

    pAcceleratorAttributes->addAttribute(
        OUString( RTL_CONSTASCII_USTRINGPARAM( ATTRIBUTE_URL )),
        m_aAttributeType,
        aAcceleratorItem.aCommand );

    // write start element
    m_xWriteDocumentHandler->startElement(
        OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_ACCELERATORITEM )),
        xAcceleratorAttrList );
    m_xWriteDocumentHandler->ignorableWhitespace( OUString() );
    m_xWriteDocumentHandler->endElement( OUString( RTL_CONSTASCII_USTRINGPARAM( ELEMENT_ACCELERATORITEM )) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
