/*************************************************************************
 *
 *  $RCSfile: xml_impctx.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2003-09-04 09:19:22 $
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

#include "osl/diagnose.h"
#include "osl/mutex.hxx"
#include "rtl/ustrbuf.hxx"
#include "cppuhelper/factory.hxx"
#include "cppuhelper/implementationentry.hxx"
#include "cppuhelper/implbase1.hxx"
#include "cppuhelper/implbase3.hxx"
#include "xmlscript/xml_import.hxx"

#include "com/sun/star/xml/input/XAttributes.hpp"
#include "com/sun/star/lang/XInitialization.hpp"
#include "com/sun/star/uno/XComponentContext.hpp"

#include <vector>
#include <hash_map>
#include <memory>


using namespace ::rtl;
using namespace ::osl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace xmlscript
{

const sal_Int32 UID_UNKNOWN = -1;

static Sequence< OUString > service_getSupportedServiceNames()
{
    OUString name( RTL_CONSTASCII_USTRINGPARAM(
                       "com.sun.star.xml.input.SaxDocumentHandler") );
    return Sequence< OUString >( &name, 1 );
}

static OUString service_getImplementationName()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM(
                         "com.sun.star.comp.xml.input.SaxDocumentHandler") );
}

typedef ::std::hash_map< OUString, sal_Int32, OUStringHash > t_OUString2LongMap;
typedef ::std::hash_map< sal_Int32, OUString > t_Long2OUStringMap;

struct PrefixEntry
{
    ::std::vector< sal_Int32 > m_Uids;

    inline PrefixEntry() SAL_THROW( () )
        { m_Uids.reserve( 4 ); }
};

typedef ::std::hash_map<
    OUString, PrefixEntry *, OUStringHash > t_OUString2PrefixMap;

struct ElementEntry
{
    Reference< xml::input::XElement > m_xElement;
    ::std::vector< OUString > m_prefixes;

    inline ElementEntry()
        { m_prefixes.reserve( 2 ); }
};

typedef ::std::vector< ElementEntry * > t_ElementVector;

class ExtendedAttributes;

//==============================================================================
struct MGuard
{
    Mutex * m_pMutex;
    explicit MGuard( Mutex * pMutex )
        : m_pMutex( pMutex )
        { if (m_pMutex) m_pMutex->acquire(); }
    ~MGuard() throw ()
        { if (m_pMutex) m_pMutex->release(); }
};

//==============================================================================
class DocumentHandlerImpl :
    public ::cppu::WeakImplHelper3< container::XNameAccess,
                                    xml::sax::XDocumentHandler,
                                    lang::XInitialization >
{
    friend class ExtendedAttributes;

    Reference< xml::input::XRoot > m_xRoot;

    t_OUString2LongMap m_URI2Uid;
    sal_Int32 m_uid_count;

    OUString m_sXMLNS_PREFIX_UNKNOWN;
    OUString m_sXMLNS;

    OUString m_aLastURI_lookup;
    sal_Int32 m_nLastURI_lookup;

    t_OUString2PrefixMap m_prefixes;
    OUString m_aLastPrefix_lookup;
    sal_Int32 m_nLastPrefix_lookup;

    t_ElementVector m_elements;
    sal_Int32 m_nSkipElements;

    Mutex * m_pMutex;

    inline Reference< xml::input::XElement > getCurrentElement() const;

    inline sal_Int32 getUidByURI( OUString const & rURI );
    inline sal_Int32 getUidByPrefix( OUString const & rPrefix );

    inline void pushPrefix(
        OUString const & rPrefix, OUString const & rURI );
    inline void popPrefix( OUString const & rPrefix );

    inline void getElementName(
        OUString const & rQName, sal_Int32 * pUid, OUString * pLocalName );

public:
    DocumentHandlerImpl(
        Reference< xml::input::XRoot > const & xRoot,
        bool bSingleThreadedUse );
    virtual ~DocumentHandlerImpl() throw ();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw (RuntimeException);
    virtual sal_Bool SAL_CALL supportsService(
        OUString const & servicename )
        throw (RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw (RuntimeException);

    // XInitialization
    virtual void SAL_CALL initialize(
        Sequence< Any > const & arguments )
        throw (Exception);

    // XDocumentHandler
    virtual void SAL_CALL startDocument()
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL endDocument()
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL startElement(
        OUString const & rQElementName,
        Reference< xml::sax::XAttributeList > const & xAttribs )
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL endElement(
        OUString const & rQElementName )
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL characters(
        OUString const & rChars )
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL ignorableWhitespace(
        OUString const & rWhitespaces )
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL processingInstruction(
        OUString const & rTarget, OUString const & rData )
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL setDocumentLocator(
        Reference< xml::sax::XLocator > const & xLocator )
        throw (xml::sax::SAXException, RuntimeException);

    // XNameAccess
    virtual Any SAL_CALL getByName(
        OUString const & name )
        throw (container::NoSuchElementException, lang::WrappedTargetException,
               RuntimeException);
    virtual Sequence< OUString > SAL_CALL getElementNames()
        throw (RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( OUString const & name )
        throw (RuntimeException);
    // XElementAccess
    virtual Type SAL_CALL getElementType()
        throw (RuntimeException);
    virtual sal_Bool SAL_CALL hasElements()
        throw (RuntimeException);
};

//______________________________________________________________________________
DocumentHandlerImpl::DocumentHandlerImpl(
    Reference< xml::input::XRoot > const & xRoot,
    bool bSingleThreadedUse )
    : m_xRoot( xRoot ),
      m_uid_count( 0 ),
      m_sXMLNS_PREFIX_UNKNOWN(
          RTL_CONSTASCII_USTRINGPARAM("<<< unknown prefix >>>") ),
      m_sXMLNS( RTL_CONSTASCII_USTRINGPARAM("xmlns") ),
      m_nLastURI_lookup( UID_UNKNOWN ),
      m_aLastURI_lookup( RTL_CONSTASCII_USTRINGPARAM("<<< unknown URI >>>") ),
      m_nLastPrefix_lookup( UID_UNKNOWN ),
      m_aLastPrefix_lookup(
          RTL_CONSTASCII_USTRINGPARAM("<<< unknown URI >>>") ),
      m_nSkipElements( 0 ),
      m_pMutex( 0 )
{
    m_elements.reserve( 10 );

    if (! bSingleThreadedUse)
        m_pMutex = new Mutex();
}

//______________________________________________________________________________
DocumentHandlerImpl::~DocumentHandlerImpl() throw ()
{
    if (m_pMutex != 0)
    {
        delete m_pMutex;
#if OSL_DEBUG_LEVEL == 0
        m_pMutex = 0;
#endif
    }
}

//______________________________________________________________________________
inline Reference< xml::input::XElement >
DocumentHandlerImpl::getCurrentElement() const
{
    MGuard aGuard( m_pMutex );
    if (m_elements.empty())
        return Reference< xml::input::XElement >();
    else
        return m_elements.back()->m_xElement;
}

//______________________________________________________________________________
inline sal_Int32 DocumentHandlerImpl::getUidByURI( OUString const & rURI )
{
    MGuard guard( m_pMutex );
    if (m_nLastURI_lookup == UID_UNKNOWN || m_aLastURI_lookup != rURI)
    {
        t_OUString2LongMap::const_iterator iFind( m_URI2Uid.find( rURI ) );
        if (iFind != m_URI2Uid.end()) // id found
        {
            m_nLastURI_lookup = iFind->second;
            m_aLastURI_lookup = rURI;
        }
        else
        {
            m_nLastURI_lookup = m_uid_count;
            ++m_uid_count;
            m_URI2Uid[ rURI ] = m_nLastURI_lookup;
            m_aLastURI_lookup = rURI;
        }
    }
    return m_nLastURI_lookup;
}

//______________________________________________________________________________
inline sal_Int32 DocumentHandlerImpl::getUidByPrefix(
    OUString const & rPrefix )
{
    // commonly the last added prefix is used often for several tags...
    // good guess
    if (m_nLastPrefix_lookup == UID_UNKNOWN || m_aLastPrefix_lookup != rPrefix)
    {
        t_OUString2PrefixMap::const_iterator iFind(
            m_prefixes.find( rPrefix ) );
        if (iFind != m_prefixes.end())
        {
            const PrefixEntry & rPrefixEntry = *iFind->second;
            OSL_ASSERT( ! rPrefixEntry.m_Uids.empty() );
            m_nLastPrefix_lookup = rPrefixEntry.m_Uids.back();
            m_aLastPrefix_lookup = rPrefix;
        }
        else
        {
            m_nLastPrefix_lookup = UID_UNKNOWN;
            m_aLastPrefix_lookup = m_sXMLNS_PREFIX_UNKNOWN;
        }
    }
    return m_nLastPrefix_lookup;
}

//______________________________________________________________________________
inline void DocumentHandlerImpl::pushPrefix(
    OUString const & rPrefix, OUString const & rURI )
{
    // lookup id for URI
    sal_Int32 nUid = getUidByURI( rURI );

    // mark prefix with id
    t_OUString2PrefixMap::const_iterator iFind( m_prefixes.find( rPrefix ) );
    if (iFind == m_prefixes.end()) // unused prefix
    {
        PrefixEntry * pEntry = new PrefixEntry();
        pEntry->m_Uids.push_back( nUid ); // latest id for prefix
        m_prefixes[ rPrefix ] = pEntry;
    }
    else
    {
        PrefixEntry * pEntry = iFind->second;
        OSL_ASSERT( ! pEntry->m_Uids.empty() );
        pEntry->m_Uids.push_back( nUid );
    }

    m_aLastPrefix_lookup = rPrefix;
    m_nLastPrefix_lookup = nUid;
}

//______________________________________________________________________________
inline void DocumentHandlerImpl::popPrefix(
    OUString const & rPrefix )
{
    t_OUString2PrefixMap::iterator iFind( m_prefixes.find( rPrefix ) );
    if (iFind != m_prefixes.end()) // unused prefix
    {
        PrefixEntry * pEntry = iFind->second;
        pEntry->m_Uids.pop_back(); // pop last id for prefix
        if (pEntry->m_Uids.empty()) // erase prefix key
        {
            m_prefixes.erase( iFind );
            delete pEntry;
        }
    }

    m_nLastPrefix_lookup = UID_UNKNOWN;
    m_aLastPrefix_lookup = m_sXMLNS_PREFIX_UNKNOWN;
}

//______________________________________________________________________________
inline void DocumentHandlerImpl::getElementName(
    OUString const & rQName, sal_Int32 * pUid, OUString * pLocalName )
{
    sal_Int32 nColonPos = rQName.indexOf( (sal_Unicode)':' );
    *pLocalName = (nColonPos >= 0 ? rQName.copy( nColonPos +1 ) : rQName);
    *pUid = getUidByPrefix(
        nColonPos >= 0 ? rQName.copy( 0, nColonPos ) : OUString() );
}


//==============================================================================
class ExtendedAttributes :
    public ::cppu::WeakImplHelper1< xml::input::XAttributes >
{
    sal_Int32 m_nAttributes;
    sal_Int32 * m_pUids;
    OUString * m_pPrefixes;
    OUString * m_pLocalNames;
    OUString * m_pQNames;
    OUString * m_pValues;

    DocumentHandlerImpl * m_pHandler;

public:
    inline ExtendedAttributes(
        sal_Int32 nAttributes,
        sal_Int32 * pUids, OUString * pPrefixes,
        OUString * pLocalNames, OUString * pQNames,
        Reference< xml::sax::XAttributeList > const & xAttributeList,
        DocumentHandlerImpl * pHandler );
    virtual ~ExtendedAttributes() throw ();

    // XAttributes
    virtual sal_Int32 SAL_CALL getLength()
        throw (RuntimeException);
    virtual sal_Int32 SAL_CALL getIndexByQName(
        OUString const & rQName )
        throw (RuntimeException);
    virtual sal_Int32 SAL_CALL getIndexByUidName(
        sal_Int32 nUid, OUString const & rLocalName )
        throw (RuntimeException);
    virtual OUString SAL_CALL getQNameByIndex(
        sal_Int32 nIndex )
        throw (RuntimeException);
    virtual sal_Int32 SAL_CALL getUidByIndex(
        sal_Int32 nIndex )
        throw (RuntimeException);
    virtual OUString SAL_CALL getLocalNameByIndex(
        sal_Int32 nIndex )
        throw (RuntimeException);
    virtual OUString SAL_CALL getValueByIndex(
        sal_Int32 nIndex )
        throw (RuntimeException);
    virtual OUString SAL_CALL getValueByUidName(
        sal_Int32 nUid, OUString const & rLocalName )
        throw (RuntimeException);
    virtual OUString SAL_CALL getTypeByIndex(
        sal_Int32 nIndex )
        throw (RuntimeException);
};

//______________________________________________________________________________
inline ExtendedAttributes::ExtendedAttributes(
    sal_Int32 nAttributes,
    sal_Int32 * pUids, OUString * pPrefixes,
    OUString * pLocalNames, OUString * pQNames,
    Reference< xml::sax::XAttributeList > const & xAttributeList,
    DocumentHandlerImpl * pHandler )
    : m_nAttributes( nAttributes )
    , m_pUids( pUids )
    , m_pPrefixes( pPrefixes )
    , m_pLocalNames( pLocalNames )
    , m_pQNames( pQNames )
    , m_pValues( new OUString[ nAttributes ] )
    , m_pHandler( pHandler )
{
    m_pHandler->acquire();

    for ( sal_Int16 nPos = 0; nPos < nAttributes; ++nPos )
    {
        m_pValues[ nPos ] = xAttributeList->getValueByIndex( nPos );
    }
}

//______________________________________________________________________________
ExtendedAttributes::~ExtendedAttributes() throw ()
{
    m_pHandler->release();

    delete [] m_pUids;
    delete [] m_pPrefixes;
    delete [] m_pLocalNames;
    delete [] m_pQNames;
    delete [] m_pValues;
}


//##############################################################################

// XServiceInfo

//______________________________________________________________________________
OUString DocumentHandlerImpl::getImplementationName()
    throw (RuntimeException)
{
    return service_getImplementationName();
}

//______________________________________________________________________________
sal_Bool DocumentHandlerImpl::supportsService(
    OUString const & servicename )
    throw (RuntimeException)
{
    Sequence< OUString > names( service_getSupportedServiceNames() );
    for ( sal_Int32 nPos = names.getLength(); nPos--; )
    {
        if (names[ nPos ].equals( servicename ))
            return sal_True;
    }
    return sal_False;
}

//______________________________________________________________________________
Sequence< OUString > DocumentHandlerImpl::getSupportedServiceNames()
    throw (RuntimeException)
{
    return service_getSupportedServiceNames();
}

// XInitialization

//______________________________________________________________________________
void DocumentHandlerImpl::initialize(
    Sequence< Any > const & arguments )
    throw (Exception)
{
    MGuard guard( m_pMutex );
    Reference< xml::input::XRoot > xRoot;
    if (arguments.getLength() == 1 &&
        (arguments[ 0 ] >>= xRoot) &&
        xRoot.is())
    {
        m_xRoot = xRoot;
    }
    else
    {
        throw RuntimeException(
            OUString( RTL_CONSTASCII_USTRINGPARAM(
                          "missing root instance!") ),
            Reference< XInterface >() );
    }
}

// XNameAccess

//______________________________________________________________________________
Any DocumentHandlerImpl::getByName( OUString const & name )
    throw (container::NoSuchElementException, lang::WrappedTargetException,
           RuntimeException)
{
    return makeAny( getUidByURI( name ) );
}

//______________________________________________________________________________
Sequence< OUString > DocumentHandlerImpl::getElementNames()
    throw (RuntimeException)
{
    MGuard guard( m_pMutex );
    Sequence< OUString > names( m_URI2Uid.size() );
    t_OUString2LongMap::const_iterator iPos( m_URI2Uid.begin() );
    t_OUString2LongMap::const_iterator const iEnd( m_URI2Uid.end() );
    OUString * pNames = names.getArray();
    sal_Int32 nPos = 0;
    for ( ; iPos != iEnd; ++iPos )
    {
        pNames[ nPos ] = iPos->first;
        ++nPos;
    }
    return names;
}

//______________________________________________________________________________
sal_Bool DocumentHandlerImpl::hasByName( OUString const & name )
    throw (RuntimeException)
{
    MGuard guard( m_pMutex );
    return m_URI2Uid.find( name ) != m_URI2Uid.end();
}

// XElementAccess

//______________________________________________________________________________
Type DocumentHandlerImpl::getElementType()
    throw (RuntimeException)
{
    return ::getCppuType( reinterpret_cast< sal_Int32 const * >( 0 ) );
}

//______________________________________________________________________________
sal_Bool DocumentHandlerImpl::hasElements()
    throw (RuntimeException)
{
    MGuard guard( m_pMutex );
    return ! m_URI2Uid.empty();
}


// XDocumentHandler

//______________________________________________________________________________
void DocumentHandlerImpl::startDocument()
    throw (xml::sax::SAXException, RuntimeException)
{
    m_xRoot->startDocument( static_cast< container::XNameAccess * >( this ) );
}

//______________________________________________________________________________
void DocumentHandlerImpl::endDocument()
    throw (xml::sax::SAXException, RuntimeException)
{
    m_xRoot->endDocument();
}

//______________________________________________________________________________
void DocumentHandlerImpl::startElement(
    OUString const & rQElementName,
    Reference< xml::sax::XAttributeList > const & xAttribs )
    throw (xml::sax::SAXException, RuntimeException)
{
    Reference< xml::input::XElement > xCurrentElement;
    Reference< xml::input::XAttributes > xAttributes;
    sal_Int32 nUid;
    OUString aLocalName;
    ::std::auto_ptr< ElementEntry > elementEntry( new ElementEntry );

    { // guard start:
    MGuard aGuard( m_pMutex );
    // currently skipping elements and waiting for end tags?
    if (m_nSkipElements > 0)
    {
        ++m_nSkipElements; // wait for another end tag
#if OSL_DEBUG_LEVEL > 1
        OString aQName(
            OUStringToOString( rQElementName, RTL_TEXTENCODING_ASCII_US ) );
        OSL_TRACE( "### no context given on createChildElement() "
                   "=> ignoring element \"%s\" ...", aQName.getStr() );
#endif
        return;
    }

    sal_Int16 nAttribs = xAttribs->getLength();

    // save all namespace ids
    sal_Int32 * pUids = new sal_Int32[ nAttribs ];
    OUString * pPrefixes = new OUString[ nAttribs ];
    OUString * pLocalNames = new OUString[ nAttribs ];
    OUString * pQNames = new OUString[ nAttribs ];

    // first recognize all xmlns attributes
    sal_Int16 nPos;
    for ( nPos = 0; nPos < nAttribs; ++nPos )
    {
        // mark attribute to be collected further
        // on with attribute's uid and current prefix
        pUids[ nPos ] = 0; // modified

        pQNames[ nPos ] = xAttribs->getNameByIndex( nPos );
        OUString const & rQAttributeName = pQNames[ nPos ];

        if (rQAttributeName.compareTo( m_sXMLNS, 5 ) == 0)
        {
            if (rQAttributeName.getLength() == 5) // set default namespace
            {
                OUString aDefNamespacePrefix;
                pushPrefix(
                    aDefNamespacePrefix,
                    xAttribs->getValueByIndex( nPos ) );
                elementEntry->m_prefixes.push_back( aDefNamespacePrefix );
                pUids[ nPos ]          = UID_UNKNOWN;
                pPrefixes[ nPos ]      = m_sXMLNS;
                pLocalNames[ nPos ]    = aDefNamespacePrefix;
            }
            else if ((sal_Unicode)':' == rQAttributeName[ 5 ]) // set prefix
            {
                OUString aPrefix( rQAttributeName.copy( 6 ) );
                pushPrefix( aPrefix, xAttribs->getValueByIndex( nPos ) );
                elementEntry->m_prefixes.push_back( aPrefix );
                pUids[ nPos ]          = UID_UNKNOWN;
                pPrefixes[ nPos ]      = m_sXMLNS;
                pLocalNames[ nPos ]    = aPrefix;
            }
            // else just a name starting with xmlns, but no prefix
        }
    }

    // now read out attribute prefixes (all namespace prefixes have been set)
    for ( nPos = 0; nPos < nAttribs; ++nPos )
    {
        if (pUids[ nPos ] >= 0) // no xmlns: attribute
        {
            OUString const & rQAttributeName = pQNames[ nPos ];
            OSL_ENSURE(
                rQAttributeName.compareToAscii(
                    RTL_CONSTASCII_STRINGPARAM("xmlns:") ) != 0,
                "### unexpected xmlns!" );

            // collect attribute's uid and current prefix
            sal_Int32 nColonPos = rQAttributeName.indexOf( (sal_Unicode) ':' );
            if (nColonPos >= 0)
            {
                pPrefixes[ nPos ] = rQAttributeName.copy( 0, nColonPos );
                pLocalNames[ nPos ] = rQAttributeName.copy( nColonPos +1 );
            }
            else
            {
                pPrefixes[ nPos ] = OUString();
                pLocalNames[ nPos ] = rQAttributeName;
                // leave local names unmodified
            }
            pUids[ nPos ] = getUidByPrefix( pPrefixes[ nPos ] );
        }
    }
    // ownership of arrays belongs to attribute list
    xAttributes = static_cast< xml::input::XAttributes * >(
        new ExtendedAttributes(
            nAttribs, pUids, pPrefixes, pLocalNames, pQNames,
            xAttribs, this ) );

    getElementName( rQElementName, &nUid, &aLocalName );

    // create new child context and append to list
    if (! m_elements.empty())
        xCurrentElement = m_elements.back()->m_xElement;
    } // :guard end

    if (xCurrentElement.is())
    {
        elementEntry->m_xElement =
            xCurrentElement->startChildElement( nUid, aLocalName, xAttributes );
    }
    else
    {
        elementEntry->m_xElement =
            m_xRoot->startRootElement( nUid, aLocalName, xAttributes );
    }

    {
    MGuard aGuard( m_pMutex );
    if (elementEntry->m_xElement.is())
    {
        m_elements.push_back( elementEntry.release() );
    }
    else
    {
        ++m_nSkipElements;
#if OSL_DEBUG_LEVEL > 1
        OString aQName(
            OUStringToOString( rQElementName, RTL_TEXTENCODING_ASCII_US ) );
        OSL_TRACE(
            "### no context given on createChildElement() => "
            "ignoring element \"%s\" ...", aQName.getStr() );
#endif
    }
    }
}

//______________________________________________________________________________
void DocumentHandlerImpl::endElement(
    OUString const & rQElementName )
    throw (xml::sax::SAXException, RuntimeException)
{
    Reference< xml::input::XElement > xCurrentElement;
    {
    MGuard aGuard( m_pMutex );
    if (m_nSkipElements)
    {
        --m_nSkipElements;
#if OSL_DEBUG_LEVEL > 1
        OString aQName(
            OUStringToOString( rQElementName, RTL_TEXTENCODING_ASCII_US ) );
        OSL_TRACE( "### received endElement() for \"%s\".", aQName.getStr() );
#endif
        return;
    }

    // popping context
    OSL_ASSERT( ! m_elements.empty() );
    ElementEntry * pEntry = m_elements.back();
    xCurrentElement = pEntry->m_xElement;

#if OSL_DEBUG_LEVEL > 0
    sal_Int32 nUid;
    OUString aLocalName;
    getElementName( rQElementName, &nUid, &aLocalName );
    OSL_ASSERT( xCurrentElement->getLocalName() == aLocalName );
    OSL_ASSERT( xCurrentElement->getUid() == nUid );
#endif

    // pop prefixes
    for ( sal_Int32 nPos = pEntry->m_prefixes.size(); nPos--; )
    {
        popPrefix( pEntry->m_prefixes[ nPos ] );
    }
    m_elements.pop_back();
    delete pEntry;
    }
    xCurrentElement->endElement();
}

//______________________________________________________________________________
void DocumentHandlerImpl::characters( OUString const & rChars )
    throw (xml::sax::SAXException, RuntimeException)
{
    Reference< xml::input::XElement > xCurrentElement( getCurrentElement() );
    if (xCurrentElement.is())
        xCurrentElement->characters( rChars );
}

//______________________________________________________________________________
void DocumentHandlerImpl::ignorableWhitespace(
    OUString const & rWhitespaces )
    throw (xml::sax::SAXException, RuntimeException)
{
    Reference< xml::input::XElement > xCurrentElement( getCurrentElement() );
    if (xCurrentElement.is())
        xCurrentElement->ignorableWhitespace( rWhitespaces );
}

//______________________________________________________________________________
void DocumentHandlerImpl::processingInstruction(
    OUString const & rTarget, OUString const & rData )
    throw (xml::sax::SAXException, RuntimeException)
{
    Reference< xml::input::XElement > xCurrentElement( getCurrentElement() );
    if (xCurrentElement.is())
        xCurrentElement->processingInstruction( rTarget, rData );
    else
        m_xRoot->processingInstruction( rTarget, rData );
}

//______________________________________________________________________________
void DocumentHandlerImpl::setDocumentLocator(
    Reference< xml::sax::XLocator > const & xLocator )
    throw (xml::sax::SAXException, RuntimeException)
{
    m_xRoot->setDocumentLocator( xLocator );
}

//##############################################################################

// XAttributes

//______________________________________________________________________________
sal_Int32 ExtendedAttributes::getIndexByQName( OUString const & rQName )
    throw (RuntimeException)
{
    for ( sal_Int32 nPos = m_nAttributes; nPos--; )
    {
        if (m_pQNames[ nPos ].equals( rQName ))
        {
            return nPos;
        }
    }
    return -1;
}

//______________________________________________________________________________
sal_Int32 ExtendedAttributes::getLength()
    throw (RuntimeException)
{
    return m_nAttributes;
}

//______________________________________________________________________________
OUString ExtendedAttributes::getLocalNameByIndex( sal_Int32 nIndex )
    throw (RuntimeException)
{
    return m_pLocalNames[ nIndex ];
}

//______________________________________________________________________________
OUString ExtendedAttributes::getQNameByIndex( sal_Int32 nIndex )
    throw (RuntimeException)
{
    OSL_ASSERT( nIndex < m_nAttributes );
    return m_pQNames[ nIndex ];
}

//______________________________________________________________________________
OUString ExtendedAttributes::getTypeByIndex( sal_Int32 nIndex )
    throw (RuntimeException)
{
    OSL_ASSERT( nIndex < m_nAttributes );
    return OUString(); // unsupported
}

//______________________________________________________________________________
OUString ExtendedAttributes::getValueByIndex( sal_Int32 nIndex )
    throw (RuntimeException)
{
    return m_pValues[ nIndex ];
}

//______________________________________________________________________________
sal_Int32 ExtendedAttributes::getIndexByUidName(
    sal_Int32 nUid, OUString const & rLocalName )
    throw (RuntimeException)
{
    for ( sal_Int32 nPos = m_nAttributes; nPos--; )
    {
        if (m_pUids[ nPos ] == nUid && m_pLocalNames[ nPos ] == rLocalName)
        {
            return nPos;
        }
    }
    return -1;
}

//______________________________________________________________________________
sal_Int32 ExtendedAttributes::getUidByIndex( sal_Int32 nIndex )
    throw (RuntimeException)
{
    return m_pUids[ nIndex ];
}

//______________________________________________________________________________
OUString ExtendedAttributes::getValueByUidName(
    sal_Int32 nUid, OUString const & rLocalName )
    throw (RuntimeException)
{
    for ( sal_Int32 nPos = m_nAttributes; nPos--; )
    {
        if (m_pUids[ nPos ] == nUid && m_pLocalNames[ nPos ] == rLocalName)
        {
            return m_pValues[ nPos ];
        }
    }
    return OUString();
}


//##############################################################################


//==============================================================================
Reference< xml::sax::XDocumentHandler > SAL_CALL createDocumentHandler(
    Reference< xml::input::XRoot > const & xRoot,
    bool bSingleThreadedUse )
    SAL_THROW( () )
{
    OSL_ASSERT( xRoot.is() );
    if (xRoot.is())
    {
        return static_cast< xml::sax::XDocumentHandler * >(
            new DocumentHandlerImpl( xRoot, bSingleThreadedUse ) );
    }
    return Reference< xml::sax::XDocumentHandler >();
}

//------------------------------------------------------------------------------
static Reference< XInterface > SAL_CALL service_create(
    Reference< XComponentContext > const & )
    SAL_THROW( (Exception) )
{
    return static_cast< ::cppu::OWeakObject * >(
        new DocumentHandlerImpl(
            Reference< xml::input::XRoot >(), false /* mt use */ ) );
}

static struct ::cppu::ImplementationEntry s_entries [] =
{
    {
        service_create, service_getImplementationName,
        service_getSupportedServiceNames, ::cppu::createSingleComponentFactory,
        0, 0
    },
    { 0, 0, 0, 0, 0, 0 }
};

}

extern "C"
{

//==============================================================================
void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

//==============================================================================
sal_Bool SAL_CALL component_writeInfo(
    void * pServiceManager, void * pRegistryKey )
{
    return ::cppu::component_writeInfoHelper(
        pServiceManager, pRegistryKey, ::xmlscript::s_entries );
}

//==============================================================================
void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    return ::cppu::component_getFactoryHelper(
        pImplName, pServiceManager, pRegistryKey, ::xmlscript::s_entries );
}

}
