/*************************************************************************
 *
 *  $RCSfile: xml_impctx.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dbo $ $Date: 2001-02-16 14:14:47 $
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

#include <vector>
#include <hash_map>

#include <osl/diagnose.h>
#include <osl/mutex.hxx>

#include <rtl/ustrbuf.hxx>

#include <cppuhelper/implbase1.hxx>
#include <xmlscript/xml_helper.hxx>

#include <com/sun/star/xml/sax2/XExtendedAttributes.hpp>

using namespace rtl;
using namespace osl;
using namespace com::sun::star;
using namespace com::sun::star::uno;


namespace xmlscript
{

typedef ::std::hash_map< OUString, sal_Int32, OUStringHash > t_OUString2LongMap;
typedef ::std::hash_map< sal_Int32, OUString > t_Long2OUStringMap;

struct PrefixEntry
{
    ::std::vector< sal_Int32 > _Uids;

    inline PrefixEntry() throw ()
        { _Uids.reserve( 4 ); }
};

typedef ::std::hash_map< OUString, PrefixEntry *, OUStringHash > t_OUString2PrefixMap;

struct ContextEntry
{
    Reference< xml::XImportContext > _xContext;
    ::std::vector< OUString > _prefixes;

    inline ContextEntry()
        { _prefixes.reserve( 2 ); }
};
typedef ::std::vector< ContextEntry * > t_ContextVector;

class ExtendedAttributes;

//==============================================================================================
struct MGuard
{
    Mutex * _pMutex;
    MGuard( Mutex * pMutex = 0 )
        : _pMutex( pMutex )
        { if (_pMutex) _pMutex->acquire(); }
    ~MGuard()
        { if (_pMutex) _pMutex->release(); }
};

//==============================================================================================
class DocumentHandlerImpl
    : public ::cppu::WeakImplHelper1< xml::sax::XDocumentHandler >
{
    friend class ExtendedAttributes;

    Reference< xml::XImporter >          _xImporter;

    t_OUString2LongMap                   _URI2Uid;
    t_Long2OUStringMap                   _Uid2URI;

    sal_Int32                            _nUnknownNamespaceUid;
    OUString                             _sXMLNS_URI_UNKNOWN;
    OUString                             _sXMLNS_PREFIX_UNKNOWN;
    OUString                             _sXMLNS;

    OUString                             _aLastURI_lookup;
    sal_Int32                            _nLastURI_lookup;

    t_OUString2PrefixMap                 _prefixes;
    OUString                             _aLastPrefix_lookup;
    sal_Int32                            _nLastPrefix_lookup;

    t_ContextVector                      _contexts;
    sal_Int32                            _nSkipElements;

    Mutex *                              _pMutex;

    inline sal_Int32 getUidByURI(
        OUString const & rURI )
        throw ();
    inline OUString getURIByUid(
        sal_Int32 nUid )
        throw ();

    inline sal_Int32 getUidByPrefix(
        OUString const & rPrefix )
        throw ();
    inline void pushPrefix(
        OUString const & rPrefix, OUString const & rURI )
        throw ();
    inline void popPrefix(
        OUString const & rPrefix )
        throw ();
    inline void getElementName(
        OUString const & rQName, sal_Int32 * pUid, OUString * pLocalName )
        throw ();
public:
    DocumentHandlerImpl(
        const NameSpaceUid * pNamespaceUids, sal_Int32 nNameSpaceUids,
        sal_Int32 nUnknownNamespaceUid,
        Reference< xml::XImporter > const & xImporter,
        bool bSingleThreadedUse )
        throw ();
    virtual ~DocumentHandlerImpl()
        throw ();

    // XDocumentHandler
    virtual void SAL_CALL startDocument()
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL endDocument()
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL startElement(
        OUString const & rQElementName, Reference< xml::sax::XAttributeList > const & xAttribs )
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
};

//__________________________________________________________________________________________________
DocumentHandlerImpl::DocumentHandlerImpl(
    const NameSpaceUid * pNamespaceUids, sal_Int32 nNameSpaceUids,
    sal_Int32 nUnknownNamespaceUid,
    Reference< xml::XImporter > const & xImporter,
    bool bSingleThreadedUse )
    throw ()
    : _xImporter( xImporter )
    , _nUnknownNamespaceUid( nUnknownNamespaceUid )
    , _sXMLNS_URI_UNKNOWN( RTL_CONSTASCII_USTRINGPARAM("<<< unknown URI >>>") )
    , _sXMLNS_PREFIX_UNKNOWN( RTL_CONSTASCII_USTRINGPARAM("<<< unknown prefix >>>") )
    , _sXMLNS( RTL_CONSTASCII_USTRINGPARAM("xmlns") )
    , _nLastURI_lookup( nUnknownNamespaceUid )
    , _aLastURI_lookup( RTL_CONSTASCII_USTRINGPARAM("<<< unknown URI >>>") )
    , _nLastPrefix_lookup( nUnknownNamespaceUid )
    , _aLastPrefix_lookup( RTL_CONSTASCII_USTRINGPARAM("<<< unknown URI >>>") )
    , _nSkipElements( 0 )
    , _pMutex( 0 )
{
    _contexts.reserve( 10 );

    if (! bSingleThreadedUse)
    {
        _pMutex = new Mutex();
    }

    for ( sal_Int32 nPos = nNameSpaceUids; nPos--; )
    {
        const NameSpaceUid & rEntry = pNamespaceUids[ nPos ];
        // default namespace has empty string as URI
        _URI2Uid[ rEntry.sURI ] = rEntry.nUid;
        _Uid2URI[ rEntry.nUid ] = rEntry.sURI;
    }
}
//__________________________________________________________________________________________________
DocumentHandlerImpl::~DocumentHandlerImpl()
    throw ()
{
    if (_pMutex)
    {
        delete _pMutex;
#ifndef _DEBUG
        _pMutex = 0;
#endif
    }
}
//__________________________________________________________________________________________________
inline sal_Int32 DocumentHandlerImpl::getUidByURI(
    OUString const & rURI )
    throw ()
{
    if (_nLastURI_lookup == _nUnknownNamespaceUid || _aLastURI_lookup != rURI)
    {
        t_OUString2LongMap::const_iterator iFind( _URI2Uid.find( rURI ) );
        if (iFind != _URI2Uid.end()) // id found
        {
            _nLastURI_lookup = iFind->second;
            _aLastURI_lookup = rURI;
        }
        else
        {
            _nLastURI_lookup = _nUnknownNamespaceUid;
            _aLastURI_lookup = _sXMLNS_URI_UNKNOWN;
        }
    }
    return _nLastURI_lookup;
}
//__________________________________________________________________________________________________
inline OUString DocumentHandlerImpl::getURIByUid( sal_Int32 nUid )
    throw ()
{
    if (nUid != _nLastURI_lookup)
    {
        t_Long2OUStringMap::const_iterator iFind( _Uid2URI.find( nUid ) );
        if (iFind != _Uid2URI.end())
        {
            _aLastURI_lookup = iFind->second;
            _nLastURI_lookup = nUid;
        }
        else
        {
            _nLastURI_lookup = _nUnknownNamespaceUid;
            _aLastURI_lookup = _sXMLNS_URI_UNKNOWN;
        }
    }
    return _aLastURI_lookup;
}
//__________________________________________________________________________________________________
inline sal_Int32 DocumentHandlerImpl::getUidByPrefix(
    OUString const & rPrefix )
    throw ()
{
    // commonly the last added prefix is used often for several tags... good guess
    if (_nLastPrefix_lookup == _nUnknownNamespaceUid || _aLastPrefix_lookup != rPrefix)
    {
        t_OUString2PrefixMap::const_iterator iFind( _prefixes.find( rPrefix ) );
        if (iFind != _prefixes.end())
        {
            const PrefixEntry & rPrefixEntry = *iFind->second;
            OSL_ASSERT( ! rPrefixEntry._Uids.empty() );
            _nLastPrefix_lookup = rPrefixEntry._Uids.back();
            _aLastPrefix_lookup = rPrefix;
        }
        else
        {
            _nLastPrefix_lookup = _nUnknownNamespaceUid;
            _aLastPrefix_lookup = _sXMLNS_PREFIX_UNKNOWN;
        }
    }
    return _nLastPrefix_lookup;
}
//__________________________________________________________________________________________________
inline void DocumentHandlerImpl::pushPrefix(
    OUString const & rPrefix, OUString const & rURI )
    throw ()
{
    // lookup id for URI
    sal_Int32 nUid = getUidByURI( rURI );

    // mark prefix with id
    t_OUString2PrefixMap::const_iterator iFind( _prefixes.find( rPrefix ) );
    if (iFind == _prefixes.end()) // unused prefix
    {
        PrefixEntry * pEntry = new PrefixEntry();
        pEntry->_Uids.push_back( nUid ); // latest id for prefix
        _prefixes[ rPrefix ] = pEntry;
    }
    else
    {
        PrefixEntry * pEntry = iFind->second;
        OSL_ASSERT( ! pEntry->_Uids.empty() );
        pEntry->_Uids.push_back( nUid );
    }

    _aLastPrefix_lookup = rPrefix;
    _nLastPrefix_lookup = nUid;
}
//__________________________________________________________________________________________________
inline void DocumentHandlerImpl::popPrefix(
    OUString const & rPrefix )
    throw ()
{
    t_OUString2PrefixMap::iterator iFind( _prefixes.find( rPrefix ) );
    if (iFind != _prefixes.end()) // unused prefix
    {
        PrefixEntry * pEntry = iFind->second;
        pEntry->_Uids.pop_back(); // pop last id for prefix
        if (pEntry->_Uids.empty()) // erase prefix key
        {
            _prefixes.erase( iFind );
            delete pEntry;
        }
    }

    _nLastPrefix_lookup = _nUnknownNamespaceUid;
    _aLastPrefix_lookup = _sXMLNS_PREFIX_UNKNOWN;
}
//__________________________________________________________________________________________________
inline void DocumentHandlerImpl::getElementName(
    OUString const & rQName, sal_Int32 * pUid, OUString * pLocalName )
    throw ()
{
    sal_Int32 nColonPos = rQName.indexOf( (sal_Unicode)':' );
    *pLocalName = (nColonPos >= 0 ? rQName.copy( nColonPos +1 ) : rQName);
    *pUid = getUidByPrefix( nColonPos >= 0 ? rQName.copy( 0, nColonPos ) : OUString() );
}

//==================================================================================================
class ExtendedAttributes
    : public ::cppu::WeakImplHelper1< xml::sax2::XExtendedAttributes >
{
    sal_Int32                             _nAttributes;
    sal_Int32 *                           _pUids;
    OUString *                            _pPrefixes;
    OUString *                            _pLocalNames;
    OUString *                            _pQNames;
    OUString *                            _pValues;

    DocumentHandlerImpl *                 _pHandler;

public:
    inline ExtendedAttributes(
        sal_Int32 nAttributes,
        sal_Int32 * pUids, OUString * pPrefixes, OUString * pLocalNames, OUString * pQNames,
        Reference< xml::sax::XAttributeList > const & xAttributeList,
        DocumentHandlerImpl * pHandler )
        throw ();
    virtual ~ExtendedAttributes()
        throw ();

    // XAttributes
    virtual sal_Int32 SAL_CALL getIndexByQName(
        OUString const & rQName )
        throw (RuntimeException);
    virtual sal_Int32 SAL_CALL getIndexByName(
        OUString const & rURI, OUString const & rLocalName )
        throw (RuntimeException);
    virtual sal_Int32 SAL_CALL getLength()
        throw (RuntimeException);
    virtual OUString SAL_CALL getLocalNameByIndex(
        sal_Int32 nIndex )
        throw (RuntimeException);
    virtual OUString SAL_CALL getQNameByIndex(
        sal_Int32 nIndex )
        throw (RuntimeException);
    virtual OUString SAL_CALL getTypeByIndex(
        sal_Int32 nIndex )
        throw (RuntimeException);
    virtual OUString SAL_CALL getTypeByName(
        OUString const & rURI, OUString const & rLocalName )
        throw (RuntimeException);
    virtual OUString SAL_CALL getURIByIndex(
        sal_Int32 nIndex )
        throw (RuntimeException);
    virtual OUString SAL_CALL getValueByIndex(
        sal_Int32 nIndex )
        throw (RuntimeException);
    virtual OUString SAL_CALL getValueByQName(
        OUString const & rQName )
        throw (RuntimeException);
    virtual OUString SAL_CALL getValueByName(
        OUString const & rURI, OUString const & rLocalName )
        throw (RuntimeException);

    // XExtendedAttributes
    virtual sal_Int32 SAL_CALL getIndexByUidName(
        sal_Int32 nUid, OUString const & rLocalName )
        throw (RuntimeException);
    virtual OUString SAL_CALL getTypeByUidName(
        sal_Int32 nUid, OUString const & rLocalName )
        throw (RuntimeException);
    virtual sal_Int32 SAL_CALL getUidByIndex(
        sal_Int32 nIndex )
        throw (RuntimeException);
    virtual OUString SAL_CALL getValueByUidName(
        sal_Int32 nUid, OUString const & rLocalName )
        throw (RuntimeException);
};
//__________________________________________________________________________________________________
inline ExtendedAttributes::ExtendedAttributes(
    sal_Int32 nAttributes,
    sal_Int32 * pUids, OUString * pPrefixes, OUString * pLocalNames, OUString * pQNames,
    Reference< xml::sax::XAttributeList > const & xAttributeList,
    DocumentHandlerImpl * pHandler )
    throw ()
    : _nAttributes( nAttributes )
    , _pUids( pUids )
    , _pPrefixes( pPrefixes )
    , _pLocalNames( pLocalNames )
    , _pQNames( pQNames )
    , _pValues( new OUString[ nAttributes ] )
    , _pHandler( pHandler )
{
    _pHandler->acquire();

    for ( sal_Int16 nPos = 0; nPos < nAttributes; ++nPos )
    {
        _pValues[ nPos ] = xAttributeList->getValueByIndex( nPos );
    }
}
//__________________________________________________________________________________________________
ExtendedAttributes::~ExtendedAttributes()
    throw ()
{
    _pHandler->release();

    delete [] _pUids;
    delete [] _pPrefixes;
    delete [] _pLocalNames;
    delete [] _pQNames;
    delete [] _pValues;
}


//##################################################################################################


// XDocumentHandler
//__________________________________________________________________________________________________
void DocumentHandlerImpl::startDocument()
    throw (xml::sax::SAXException, RuntimeException)
{
    _xImporter->startDocument();
}
//__________________________________________________________________________________________________
void DocumentHandlerImpl::endDocument()
    throw (xml::sax::SAXException, RuntimeException)
{
    _xImporter->endDocument();
}
//__________________________________________________________________________________________________
void DocumentHandlerImpl::startElement(
    OUString const & rQElementName, Reference< xml::sax::XAttributeList > const & xAttribs )
    throw (xml::sax::SAXException, RuntimeException)
{
    Reference< xml::XImportContext > xCurrentContext;

    sal_Int32 nUid;
    OUString aLocalName;

    sal_Int16 nAttribs = xAttribs->getLength();

    ContextEntry * pContextEntry = new ContextEntry();

    // save all namespace ids
    sal_Int32 * pUids = new sal_Int32[ nAttribs ];
    OUString * pPrefixes = new OUString[ nAttribs ];
    OUString * pLocalNames = new OUString[ nAttribs ];
    OUString * pQNames = new OUString[ nAttribs ];

    { // guard start:
    MGuard aGuard( _pMutex );
    if (_nSkipElements) // currently skipping elements and waiting for end tags?
    {
        ++_nSkipElements; // wait for another end tag
#ifdef _DEBUG
        OString aQName( OUStringToOString( rQElementName, RTL_TEXTENCODING_ASCII_US ) );
        OSL_TRACE( "### no context given on createChildContext() => ignoring element \"%s\" ...", aQName.getStr() );
#endif
        return;
    }

    // first recognize all xmlns attributes
    sal_Int16 nPos;
    for ( nPos = 0; nPos < nAttribs; ++nPos )
    {
        // mark attribute to be collected further on with attribute's uid and current prefix
        pUids[ nPos ] = _nUnknownNamespaceUid +1; // modified

        pQNames[ nPos ] = xAttribs->getNameByIndex( nPos );
        OUString const & rQAttributeName = pQNames[ nPos ];

        if (rQAttributeName.compareTo( _sXMLNS, 5 ) == 0)
        {
            if (rQAttributeName.getLength() == 5) // set default namespace
            {
                OUString aDefNamespacePrefix;
                pushPrefix( aDefNamespacePrefix, xAttribs->getValueByIndex( nPos ) );
                pContextEntry->_prefixes.push_back( aDefNamespacePrefix );
                pUids[ nPos ]          = _nUnknownNamespaceUid;
                pPrefixes[ nPos ]      = _sXMLNS;
                pLocalNames[ nPos ]    = aDefNamespacePrefix;
            }
            else if ((sal_Unicode)':' == rQAttributeName[ 5 ]) // set prefix
            {
                OUString aPrefix( rQAttributeName.copy( 6 ) );
                pushPrefix( aPrefix, xAttribs->getValueByIndex( nPos ) );
                pContextEntry->_prefixes.push_back( aPrefix );
                pUids[ nPos ]          = _nUnknownNamespaceUid;
                pPrefixes[ nPos ]      = _sXMLNS;
                pLocalNames[ nPos ]    = aPrefix;
            }
            // else just a name starting with xmlns: no prefix
        }
    }

    // now read out attribute prefixes (all namespace prefixes have been set)
    for ( nPos = 0; nPos < nAttribs; ++nPos )
    {
        if (pUids[ nPos ] != _nUnknownNamespaceUid) // no namespace attribute
        {
            OUString const & rQAttributeName = pQNames[ nPos ];
            OSL_ENSURE( rQAttributeName.compareToAscii( "xmlns:", 6 ) != 0, "### unexpected xmlns!" );

            // collect attribute's uid and current prefix
            sal_Int32 nColonPos = rQAttributeName.indexOf( (sal_Unicode)':' );
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

    getElementName( rQElementName, &nUid, &aLocalName );

    // create new child context and append to list
    if (! _contexts.empty())
    {
        xCurrentContext = _contexts.back()->_xContext;
    }
    } // :guard end

    // wrap for XAttributeList to act like sax2 XExtendedAttributes
    // ownership of arrays belongs to attribute list
    Reference< xml::sax2::XExtendedAttributes > xAttributes( static_cast< xml::sax2::XExtendedAttributes * >(
        new ExtendedAttributes( nAttribs, pUids, pPrefixes, pLocalNames, pQNames, xAttribs, this ) ) );

    pContextEntry->_xContext = (xCurrentContext.is()
                                ? xCurrentContext->createChildContext( nUid, aLocalName, xAttributes )
                                : _xImporter->createRootContext( nUid, aLocalName, xAttributes ) );

    {
    MGuard aGuard( _pMutex );
    if (pContextEntry->_xContext.is())
    {
        _contexts.push_back( pContextEntry );
    }
    else
    {
        ++_nSkipElements;
#ifdef _DEBUG
        OString aQName( OUStringToOString( rQElementName, RTL_TEXTENCODING_ASCII_US ) );
        OSL_TRACE( "### no context given on createChildContext() => ignoring element \"%s\" ...", aQName.getStr() );
#endif
    }
    }
}
//__________________________________________________________________________________________________
void DocumentHandlerImpl::endElement(
    OUString const & rQElementName )
    throw (xml::sax::SAXException, RuntimeException)
{
    Reference< xml::XImportContext > xCurrentContext;
    {
    MGuard aGuard( _pMutex );
    if (_nSkipElements)
    {
        --_nSkipElements;
#ifdef _DEBUG
        OString aQName( OUStringToOString( rQElementName, RTL_TEXTENCODING_ASCII_US ) );
        OSL_TRACE( "### received endElement() for \"%s\".", aQName.getStr() );
#endif
        return;
    }

    // popping context
    OSL_ASSERT( ! _contexts.empty() );
    ContextEntry * pEntry = _contexts.back();
    xCurrentContext = pEntry->_xContext;

#ifdef _DEBUG
    sal_Int32 nUid;
    OUString aLocalName;
    getElementName( rQElementName, &nUid, &aLocalName );
    OSL_ASSERT( xCurrentContext->getLocalName() == aLocalName );
    OSL_ASSERT( xCurrentContext->getUid() == nUid );
#endif

    // pop prefixes
    for ( sal_Int32 nPos = pEntry->_prefixes.size(); nPos--; )
    {
        popPrefix( pEntry->_prefixes[ nPos ] );
    }
    _contexts.pop_back();
    delete pEntry;
    }
    xCurrentContext->endElement();
}
//__________________________________________________________________________________________________
void DocumentHandlerImpl::characters(
    OUString const & rChars )
    throw (xml::sax::SAXException, RuntimeException)
{
    Reference< xml::XImportContext > xCurrentContext;
    {
    MGuard aGuard( _pMutex );
    if (! _contexts.empty())
        xCurrentContext = _contexts.back()->_xContext;
    }
    if (xCurrentContext.is())
        xCurrentContext->characters( rChars );
}
//__________________________________________________________________________________________________
void DocumentHandlerImpl::ignorableWhitespace(
    OUString const & rWhitespaces )
    throw (xml::sax::SAXException, RuntimeException)
{
    Reference< xml::XImportContext > xCurrentContext;
    {
    MGuard aGuard( _pMutex );
    if (! _contexts.empty())
        xCurrentContext = _contexts.back()->_xContext;
    }
    if (xCurrentContext.is())
        xCurrentContext->ignorableWhitespace( rWhitespaces );
}
//__________________________________________________________________________________________________
void DocumentHandlerImpl::processingInstruction(
    OUString const & rTarget, OUString const & rData )
    throw (xml::sax::SAXException, RuntimeException)
{
    _xImporter->processingInstruction( rTarget, rData );
}
//__________________________________________________________________________________________________
void DocumentHandlerImpl::setDocumentLocator(
    Reference< xml::sax::XLocator > const & xLocator )
    throw (xml::sax::SAXException, RuntimeException)
{
    _xImporter->setDocumentLocator( xLocator );
}


//##################################################################################################


// XAttributes
//__________________________________________________________________________________________________
sal_Int32 ExtendedAttributes::getIndexByQName(
    OUString const & rQName )
    throw (RuntimeException)
{
    for ( sal_Int32 nPos = _nAttributes; nPos--; )
    {
        if (_pQNames[ nPos ] == rQName)
        {
            return nPos;
        }
    }
    return -1;
}
//__________________________________________________________________________________________________
sal_Int32 ExtendedAttributes::getIndexByName(
    OUString const & rURI, OUString const & rLocalName )
    throw (RuntimeException)
{
    sal_Int32 nUid;
    {
    MGuard aGuard( _pHandler->_pMutex );
    nUid = _pHandler->getUidByURI( rURI );
    }
    for ( sal_Int32 nPos = _nAttributes; nPos--; )
    {
        if (_pUids[ nPos ] == nUid && _pLocalNames[ nPos ] == rLocalName)
        {
            return nPos;
        }
    }
    return -1;
}
//__________________________________________________________________________________________________
sal_Int32 ExtendedAttributes::getLength()
    throw (RuntimeException)
{
    return _nAttributes;
}
//__________________________________________________________________________________________________
OUString ExtendedAttributes::getLocalNameByIndex(
    sal_Int32 nIndex )
    throw (RuntimeException)
{
    return _pLocalNames[ nIndex ];
}
//__________________________________________________________________________________________________
OUString ExtendedAttributes::getQNameByIndex(
    sal_Int32 nIndex )
    throw (RuntimeException)
{
    OSL_ASSERT( nIndex < _nAttributes );
    return _pQNames[ nIndex ];
}
//__________________________________________________________________________________________________
OUString ExtendedAttributes::getTypeByIndex(
    sal_Int32 nIndex )
    throw (RuntimeException)
{
    OSL_ASSERT( nIndex < _nAttributes );
    // xxx todo what about prefixed types?
    return OUString();
}
//__________________________________________________________________________________________________
OUString ExtendedAttributes::getTypeByName(
    OUString const & rURI, OUString const & rLocalName )
    throw (RuntimeException)
{
    // xxx todo
//      sal_Int32 nUid;
//      {
//      MGuard aGuard( _pHandler->_pMutex );
//      nUid = _pHandler->getUidByURI( rURI );
//      }
//      for ( sal_Int32 nPos = _nAttributes; nPos--; )
//      {
//          if (_pUids[ nPos ] == nUid && _pLocalNames[ nPos ] == rLocalName)
//          {
//              return _xAttributeList->getTypeByIndex( (sal_Int16)nPos );
//          }
//      }
    return OUString();
}
//__________________________________________________________________________________________________
OUString ExtendedAttributes::getURIByIndex(
    sal_Int32 nIndex )
    throw (RuntimeException)
{
    OSL_ASSERT( nIndex < _nAttributes );
    MGuard aGuard( _pHandler->_pMutex );
    return _pHandler->getURIByUid( _pUids[ nIndex ] );
}
//__________________________________________________________________________________________________
OUString ExtendedAttributes::getValueByIndex(
    sal_Int32 nIndex )
    throw (RuntimeException)
{
    return _pValues[ nIndex ];
}
//__________________________________________________________________________________________________
OUString ExtendedAttributes::getValueByQName(
    OUString const & rQName )
    throw (RuntimeException)
{
    for ( sal_Int32 nPos = _nAttributes; nPos--; )
    {
        if (_pQNames[ nPos ] == rQName)
        {
            return _pValues[ nPos ];
        }
    }
    return OUString();
}
//__________________________________________________________________________________________________
OUString ExtendedAttributes::getValueByName(
    OUString const & rURI, OUString const & rLocalName )
    throw (RuntimeException)
{
    sal_Int32 nUid;
    {
    MGuard aGuard( _pHandler->_pMutex );
    nUid = _pHandler->getUidByURI( rURI );
    }
    for ( sal_Int32 nPos = _nAttributes; nPos--; )
    {
        if (_pUids[ nPos ] == nUid && _pLocalNames[ nPos ] == rLocalName)
        {
            return _pValues[ nPos ];
        }
    }
    return OUString();
}

// XExtendedAttributes
//__________________________________________________________________________________________________
sal_Int32 ExtendedAttributes::getIndexByUidName(
    sal_Int32 nUid, OUString const & rLocalName )
    throw (RuntimeException)
{
    for ( sal_Int32 nPos = _nAttributes; nPos--; )
    {
        if (_pUids[ nPos ] == nUid && _pLocalNames[ nPos ] == rLocalName)
        {
            return nPos;
        }
    }
    return -1;
}
//__________________________________________________________________________________________________
OUString ExtendedAttributes::getTypeByUidName(
    sal_Int32 nUid, OUString const & rLocalName )
    throw (RuntimeException)
{
    // xxx todo
//      for ( sal_Int32 nPos = _nAttributes; nPos--; )
//      {
//          if (_pUids[ nPos ] == nUid && _pLocalNames[ nPos ] == rLocalName)
//          {
//          }
//      }
    return OUString();
}
//__________________________________________________________________________________________________
sal_Int32 ExtendedAttributes::getUidByIndex(
    sal_Int32 nIndex )
    throw (RuntimeException)
{
    return _pUids[ nIndex ];
}
//__________________________________________________________________________________________________
OUString ExtendedAttributes::getValueByUidName(
    sal_Int32 nUid, OUString const & rLocalName )
    throw (RuntimeException)
{
    for ( sal_Int32 nPos = _nAttributes; nPos--; )
    {
        if (_pUids[ nPos ] == nUid && _pLocalNames[ nPos ] == rLocalName)
        {
            return _pValues[ nPos ];
        }
    }
    return OUString();
}


//##################################################################################################


//==================================================================================================
Reference< xml::sax::XDocumentHandler > SAL_CALL createDocumentHandler(
    NameSpaceUid const * pNamespaceUids, sal_Int32 nNameSpaceUids,
    sal_Int32 nUnknownNamespaceUid,
    Reference< xml::XImporter > const & xImporter,
    bool bSingleThreadedUse )
    throw ()
{
    Reference< xml::sax::XDocumentHandler > xRet;

    OSL_ASSERT( xImporter.is() );
    if (xImporter.is())
    {
        DocumentHandlerImpl * pImpl = new DocumentHandlerImpl(
            pNamespaceUids, nNameSpaceUids, nUnknownNamespaceUid,
            xImporter, bSingleThreadedUse );

        xRet = static_cast< xml::sax::XDocumentHandler * >( pImpl );
    }

    return xRet;
}

};
