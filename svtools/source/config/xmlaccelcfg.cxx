/*************************************************************************
 *
 *  $RCSfile: xmlaccelcfg.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-15 17:52:51 $
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

#include "xmlaccelcfg.hxx"

#include <vector>
#if OSL_DEBUG_LEVEL == 0
#define NDEBUG
#endif
#include <assert.h>

#ifndef _COM_SUN_STAR_XML_SAX_XATTRIBUTELIST_HPP_
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::xml::sax;

#define ELEMENT_ACCELERATORLIST     "acceleratorlist"
#define ELEMENT_ACCELERATORITEM     "item"

#define ATTRIBUTE_KEYCODE           "code"
#define ATTRIBUTE_MODIFIER          "modifier"
#define ATTRIBUTE_URL               "url"

#define ATTRIBUTE_TYPE_CDATA        "CDATA"

// ------------------------------------------------------------------

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
    void clear();

private:
    struct AttributeListImpl_impl *m_pImpl;
};

struct TagAttribute
{
    TagAttribute(){}
    TagAttribute( const OUString &sName, const OUString &sType , const OUString &sValue )
    {
        this->sName     = sName;
        this->sType     = sType;
        this->sValue    = sValue;
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
    return m_pImpl->vecAttribute.size();
}


AttributeListImpl::AttributeListImpl( const AttributeListImpl &r )
{
    m_pImpl = new AttributeListImpl_impl;
    *m_pImpl = *(r.m_pImpl);
}

OUString AttributeListImpl::getNameByIndex(sal_Int16 i) throw (RuntimeException)
{
    if( i < m_pImpl->vecAttribute.size() ) {
        return m_pImpl->vecAttribute[i].sName;
    }
    return OUString();
}


OUString AttributeListImpl::getTypeByIndex(sal_Int16 i) throw (RuntimeException)
{
    if( i < m_pImpl->vecAttribute.size() ) {
        return m_pImpl->vecAttribute[i].sType;
    }
    return OUString();
}

OUString AttributeListImpl::getValueByIndex(sal_Int16 i) throw (RuntimeException)
{
    if( i < m_pImpl->vecAttribute.size() ) {
        return m_pImpl->vecAttribute[i].sValue;
    }
    return OUString();

}

OUString AttributeListImpl::getTypeByName( const OUString& sName ) throw (RuntimeException)
{
    ::std::vector<struct TagAttribute>::iterator ii = m_pImpl->vecAttribute.begin();

    for( ; ii != m_pImpl->vecAttribute.end() ; ii ++ ) {
        if( (*ii).sName == sName ) {
            return (*ii).sType;
        }
    }
    return OUString();
}

OUString AttributeListImpl::getValueByName(const OUString& sName) throw (RuntimeException)
{
    ::std::vector<struct TagAttribute>::iterator ii = m_pImpl->vecAttribute.begin();

    for( ; ii != m_pImpl->vecAttribute.end() ; ii ++ ) {
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

void AttributeListImpl::clear()
{
    ::std::vector<struct TagAttribute> dummy;
    m_pImpl->vecAttribute.swap( dummy );

    assert( ! getLength() );
}

// ------------------------------------------------------------------

Any SAL_CALL OReadAccelatorDocumentHandler::queryInterface( const Type & rType ) throw( RuntimeException )
{
    Any a = ::cppu::queryInterface( rType ,SAL_STATIC_CAST( XDocumentHandler*, this ));
    if ( a.hasValue() )
        return a;
    else
        return OWeakObject::queryInterface( rType );
}

void SAL_CALL OReadAccelatorDocumentHandler::ignorableWhitespace(
    const OUString& aWhitespaces )
throw( SAXException, RuntimeException )
{
}

void SAL_CALL OReadAccelatorDocumentHandler::processingInstruction(
    const OUString& aTarget, const OUString& aData )
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
    const OUString& aName, const Reference< XAttributeList > &xAttrList )
throw( SAXException, RuntimeException )
{
    m_nElementDepth++;

    if ( aName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( ELEMENT_ACCELERATORLIST )))
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
    else if ( aName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( ELEMENT_ACCELERATORITEM )))
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
            for ( int i=0; i< xAttrList->getLength(); i++ )
            {
                OUString aName = xAttrList->getNameByIndex( i );
                OUString aValue = xAttrList->getValueByIndex( i );

                if ( aName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( ATTRIBUTE_URL )))
                    aItem.aCommand = aValue;
                else if ( aName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( ATTRIBUTE_MODIFIER )))
                    aItem.nModifier = (sal_uInt16)aValue.toInt32();
                else if ( aName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( ATTRIBUTE_KEYCODE )))
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


void SAL_CALL OReadAccelatorDocumentHandler::characters(const rtl::OUString& aChars)
throw(  SAXException, RuntimeException )
{
}


void SAL_CALL OReadAccelatorDocumentHandler::endElement( const OUString& aName )
    throw( SAXException, RuntimeException )
{
    m_nElementDepth--;

    if ( aName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( ELEMENT_ACCELERATORLIST )))
    {
        // acceleratorlist
        if ( !m_bAcceleratorMode )
        {
            OUString aErrorMessage = getErrorLineString();
            aErrorMessage += OUString( RTL_CONSTASCII_USTRINGPARAM( "Accelerator list used twice!" ));
            throw SAXException( aErrorMessage, Reference< XInterface >(), Any() );
        }
    }
    else if ( aName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( ELEMENT_ACCELERATORITEM )))
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
    for ( p = m_aWriteAcceleratorList.begin(); p != m_aWriteAcceleratorList.end(); p++ )
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
