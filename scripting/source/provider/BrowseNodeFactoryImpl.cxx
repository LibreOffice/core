/*************************************************************************
 *
 *  $RCSfile: BrowseNodeFactoryImpl.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2004-07-23 14:09:35 $
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

#include <cppuhelper/weakref.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/implbase1.hxx>

#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/reflection/XProxyFactory.hpp>

#include <drafts/com/sun/star/script/provider/XScriptProviderFactory.hpp>
#include <drafts/com/sun/star/script/browse/BrowseNodeFactoryViewType.hpp>


#include "BrowseNodeFactoryImpl.hxx"
#include "ActiveMSPList.hxx"
#include <util/MiscUtils.hxx>
#include <util/util.hxx>

#include <vector>
#include <algorithm>
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::drafts::com::sun::star::script;
using namespace ::sf_misc;

namespace browsenodefactory
{
class BrowseNodeAggregator :
    public ::cppu::WeakImplHelper1< browse::XBrowseNode >
{
private:
    ::rtl::OUString m_Name;
    Sequence< Reference< browse::XBrowseNode > > m_Nodes;

public:

    BrowseNodeAggregator( const Reference< browse::XBrowseNode >& node )
    {
        OSL_TRACE ("GETCHILDNODES(): Creating aggregator: %s",
            ::rtl::OUStringToOString( node->getName(),
                RTL_TEXTENCODING_ASCII_US ).pData->buffer );

        m_Name = node->getName();
        m_Nodes.realloc( 1 );
        m_Nodes[ 0 ] = node;
    }

    ~BrowseNodeAggregator()
    {
        OSL_TRACE ("~BrowseNodeAggregator(): Destroying aggregator: %s ",
            ::rtl::OUStringToOString( m_Name,
                RTL_TEXTENCODING_ASCII_US ).pData->buffer );
    }

    void addBrowseNode( const Reference< browse::XBrowseNode>& node )
    {
        sal_Int32 index = m_Nodes.getLength();

        m_Nodes.realloc( index + 1 );
        m_Nodes[ index ] = node;
    }

    virtual ::rtl::OUString
    SAL_CALL getName()
            throw ( RuntimeException )
    {
        return m_Name;
    }

    virtual Sequence< Reference< browse::XBrowseNode > > SAL_CALL
    getChildNodes()
        throw ( RuntimeException )
    {
        std::vector<  Sequence< Reference < browse::XBrowseNode > > > seqs;
        seqs.reserve( m_Nodes.getLength() );

        sal_Int32 numChildren = 0;

        for ( sal_Int32 i = 0; i < m_Nodes.getLength(); i++ )
        {
            Sequence< Reference < browse::XBrowseNode > > childs = m_Nodes[ i ]->getChildNodes();
            seqs.push_back( childs );
            numChildren += childs.getLength();
        }

        std::vector<  Sequence< Reference < browse::XBrowseNode > > >::const_iterator it = seqs.begin();
        std::vector<  Sequence< Reference < browse::XBrowseNode > > >::const_iterator it_end = seqs.end();

        Sequence< Reference < browse::XBrowseNode > > result( numChildren );
        for ( sal_Int32 index = 0; it != it_end && index < numChildren ; ++it )
        {
            Sequence< Reference < browse::XBrowseNode > > childs = *it;
            for ( sal_Int32 j = 0; j < childs.getLength(); j++ )
            {
                result[ index++ ] = childs[ j ];
            }
        }
        return result;
    }

    virtual sal_Bool SAL_CALL
    hasChildNodes()
        throw ( RuntimeException )
    {
        if ( m_Nodes.getLength() != 0 )
        {
            for ( sal_Int32 i = 0 ; i < m_Nodes.getLength(); i++ )
            {
                if ( m_Nodes[ i ]->hasChildNodes() )
                {
                    return sal_True;
                }
            }
        }

        return sal_False;
    }

    virtual sal_Int16 SAL_CALL getType()
        throw ( RuntimeException )
    {
        return browse::BrowseNodeTypes::CONTAINER;
    }
};


//typedef ::std::map< ::rtl::OUString, Reference< browse::XBrowseNode > >
typedef ::std::hash_map< ::rtl::OUString, Reference< browse::XBrowseNode >,
    ::rtl::OUStringHash, ::std::equal_to< ::rtl::OUString > >
        BrowseNodeAggregatorHash;
typedef ::std::vector< ::rtl::OUString > vString;


struct alphaSort
{
    bool operator()( const ::rtl::OUString& a, const ::rtl::OUString& b )
    {
        return a.compareTo( b ) < 0;
    }
};
class LocationBrowseNode :
    public ::cppu::WeakImplHelper1< browse::XBrowseNode >
{
private:
    BrowseNodeAggregatorHash* m_hBNA;
    vString m_vStr;
    ::rtl::OUString m_sNodeName;
    Reference< browse::XBrowseNode > m_origNode;

public:

    LocationBrowseNode( const Reference< browse::XBrowseNode >& node )
    {
        m_sNodeName = node->getName();
        m_hBNA = NULL;
        m_origNode.set( node );

        OSL_TRACE ("LocationBrowseNode(): Creating LocationBrowseNode: %s",
            ::rtl::OUStringToOString( m_sNodeName,
                RTL_TEXTENCODING_ASCII_US ).pData->buffer );
    }

    ~LocationBrowseNode()
    {
        OSL_TRACE ("~LocationBrowseNode(): Destroying LocationBrowseNode: %s",
            ::rtl::OUStringToOString( m_sNodeName,
                RTL_TEXTENCODING_ASCII_US ).pData->buffer );
        // TODO missing delete

        delete m_hBNA;
    }

    // -------------------------------------------------------------------------
    // XBrowseNode
    // -------------------------------------------------------------------------

    virtual ::rtl::OUString SAL_CALL getName()
        throw ( RuntimeException )
    {
        return m_sNodeName;
    }

    virtual Sequence< Reference< browse::XBrowseNode > > SAL_CALL
    getChildNodes()
        throw ( RuntimeException )
    {
        OSL_TRACE ("LocationBrowseNode():getChildNodes() for : %s",
            ::rtl::OUStringToOString( m_sNodeName,
                RTL_TEXTENCODING_ASCII_US ).pData->buffer );

        if ( m_hBNA == NULL )
        {
            loadChildNodes();
        }

        OSL_TRACE ("GETCHILDNODES(): Creating new sequence");
        Sequence<  Reference< browse::XBrowseNode > > children( m_hBNA->size() );
        sal_Int32 index = 0;

        vString::const_iterator it = m_vStr.begin();

        for ( ; it != m_vStr.end(); ++it, index++ )
        {
            children[ index ].set( m_hBNA->find( *it )->second );
        }

        OSL_TRACE ("GETCHILDNODES(): Returning new sequence");
        return children;
    }

    virtual sal_Bool SAL_CALL hasChildNodes()
        throw ( RuntimeException )
    {
        return sal_True;
    }

    virtual sal_Int16 SAL_CALL getType()
        throw ( RuntimeException )
    {
        return browse::BrowseNodeTypes::CONTAINER;
    }

private:

    void loadChildNodes()
    {
        OSL_TRACE ("LOADCHILDNODES(): %s",
            ::rtl::OUStringToOString( m_sNodeName,
                RTL_TEXTENCODING_ASCII_US ).pData->buffer );

        m_hBNA = new BrowseNodeAggregatorHash();

        Sequence< Reference< browse::XBrowseNode > > langNodes =
            m_origNode->getChildNodes();

        OSL_TRACE ("LOADCHILDNODES(): Got providers");

        for ( sal_Int32 i = 0; i < langNodes.getLength(); i++ )
        {
            Reference< browse::XBrowseNode > xbn;
            if ( langNodes[ i ]->getName().equals(::rtl::OUString::createFromAscii("uno_packages")) )
            {
                xbn.set( new LocationBrowseNode( langNodes[ i ] ) );
            }
            else
            {
                xbn.set( langNodes[ i ] );
            }

            Sequence< Reference< browse::XBrowseNode > > grandchildren =
                xbn->getChildNodes();

            OSL_TRACE ("LOADCHILDNODES(): Got grandchildren");

            for ( sal_Int32 j = 0; j < grandchildren.getLength(); j++ )
            {
                Reference< browse::XBrowseNode > grandchild( grandchildren[ j ] );

                OSL_TRACE ("LOADCHILDNODES(): Got grandchild: %s",
                    ::rtl::OUStringToOString( grandchild->getName(),
                        RTL_TEXTENCODING_ASCII_US ).pData->buffer );

                BrowseNodeAggregatorHash::iterator h_it =
                    m_hBNA->find( grandchild->getName() );

                if ( h_it != m_hBNA->end() )
                {
                    OSL_TRACE ("LOADCHILDNODES(): Already got an aggregator");

                    BrowseNodeAggregator* bna = static_cast< BrowseNodeAggregator* >( h_it->second.get() );
                    bna->addBrowseNode( grandchild );
                }
                else
                {
                    OSL_TRACE ("LOADCHILDNODES(): Need a new aggregator");

                    Reference< browse::XBrowseNode > bna(
                        new BrowseNodeAggregator( grandchild ) );
                    (*m_hBNA)[ grandchild->getName() ].set( bna );
                    m_vStr.push_back( grandchild->getName() );
                }
            }
        }
        // sort children alpahbetically
        ::std::sort( m_vStr.begin(), m_vStr.end(), alphaSort() );
    }
};

Sequence < ::rtl::OUString >
tdocBugWorkAround( const Reference< XComponentContext >& xCtx )
{
    OSL_TRACE("In tdocBugWorkAround()");
    Sequence < ::rtl::OUString > result;
    Reference< lang::XMultiComponentFactory > mcf =
            xCtx->getServiceManager();
    Reference< frame::XDesktop > desktop (
        mcf->createInstanceWithContext(
            ::rtl::OUString::createFromAscii("com.sun.star.frame.Desktop"),                 xCtx ),
            UNO_QUERY );

    Reference< container::XEnumerationAccess > componentsAccess =
        desktop->getComponents();
    Reference< container::XEnumeration > components =
        componentsAccess->createEnumeration();
    sal_Int32 docIndex = 0;
    while (components->hasMoreElements())
    {
        Sequence< Any > args( 1 );

        Reference< frame::XModel > model(
            components->nextElement(), UNO_QUERY );
        if ( model.is() )
        {
            ::rtl::OUString sTdocUrl = MiscUtils::xModelToTdocUrl( model );
            if ( sTdocUrl.getLength() > 0 )
            {
                result.realloc( result.getLength() + 1 );
                result[ docIndex++ ] = sTdocUrl;
                OSL_TRACE("In tdocBugWorkAround() add doc title");
            }
        }
    }
    return result;
}

Sequence< Reference< browse::XBrowseNode > > getAllBrowseNodes( const Reference< XComponentContext >& xCtx )
{
        OSL_TRACE("getAllBrowseNodes");
        Reference< lang::XMultiComponentFactory > mcf =
            xCtx->getServiceManager();

        Sequence< ::rtl::OUString > openDocs =

        //    MiscUtils::allOpenTDocUrls( xCtx );
            tdocBugWorkAround( xCtx );
    Reference< provider::XScriptProviderFactory > xFac;
        sal_Int32 initialSize = openDocs.getLength() + 2;
        sal_Int32 mspIndex = 0;

        Sequence < Reference < browse::XBrowseNode > > locnBNs( initialSize );
        try
        {
        xFac.set(
                xCtx->getValueByName(
                    OUSTR("/singletons/drafts.com.sun.star.script.provider.theMasterScriptProviderFactory") ), UNO_QUERY_THROW );

            locnBNs[ mspIndex++ ] = Reference< browse::XBrowseNode >( xFac->createScriptProvider( makeAny( ::rtl::OUString::createFromAscii("user") ) ), UNO_QUERY_THROW );
            locnBNs[ mspIndex++ ] = Reference< browse::XBrowseNode >( xFac->createScriptProvider( makeAny( ::rtl::OUString::createFromAscii("share") ) ), UNO_QUERY_THROW );
        }
        // TODO proper exception handling, should throw
        catch( Exception& e )
        {
            OSL_TRACE("Caught Exception %s",
                ::rtl::OUStringToOString( e.Message , RTL_TEXTENCODING_ASCII_US ).pData->buffer );
        locnBNs.realloc( mspIndex );
            return locnBNs;
        }

    for ( sal_Int32 i = 0; i < openDocs.getLength(); i++ )
        {

            try
            {
                Reference< frame::XModel > model( MiscUtils::tDocUrlToModel( openDocs[ i ] ), UNO_QUERY_THROW );
            locnBNs[ mspIndex++ ] = Reference< browse::XBrowseNode >( xFac->createScriptProvider( makeAny( model ) ), UNO_QUERY_THROW );
            }
            catch( Exception& e )
            {

                OSL_TRACE("Caught Exception creating MSP for %s exception msg: %s",
                    ::rtl::OUStringToOString( openDocs[ i ] , RTL_TEXTENCODING_ASCII_US ).pData->buffer,
                    ::rtl::OUStringToOString( e.Message , RTL_TEXTENCODING_ASCII_US ).pData->buffer );
            }

        }
    return locnBNs;
}

typedef ::std::vector< Reference< browse::XBrowseNode > > vXBrowseNodes;

struct alphaSortForBNodes
{
    bool operator()( const Reference< browse::XBrowseNode >& a, const Reference< browse::XBrowseNode >& b )
    {
        return a->getName().compareTo( b->getName() ) < 0;
    }
};

typedef ::cppu::WeakImplHelper1< browse::XBrowseNode > t_BrowseNodeBase;
class DefaultBrowseNode :
    public t_BrowseNodeBase
{

private:
    Reference< browse::XBrowseNode > m_xWrappedBrowseNode;
    Reference< lang::XTypeProvider > m_xWrappedTypeProv;;
    Reference< XAggregation >        m_xAggProxy;
    Reference< XComponentContext >   m_xCtx;

    DefaultBrowseNode();
public:
    DefaultBrowseNode( const Reference< XComponentContext >& xCtx, const Reference< browse::XBrowseNode>& xNode ) : m_xWrappedBrowseNode( xNode ), m_xWrappedTypeProv( xNode, UNO_QUERY ), m_xCtx( xCtx, UNO_QUERY )
    {
        OSL_ENSURE( m_xWrappedBrowseNode.is(), "DefaultBrowseNode::DefaultBrowseNode(): No BrowseNode to wrap" );
        OSL_ENSURE( m_xWrappedTypeProv.is(), "DefaultBrowseNode::DefaultBrowseNode(): No BrowseNode to wrap" );
        OSL_ENSURE( m_xCtx.is(), "DefaultBrowseNode::DefaultBrowseNode(): No ComponentContext" );
    // Use proxy factory service to create aggregatable proxy.
        try
        {
            Reference< lang::XMultiComponentFactory > xMFac( m_xCtx->getServiceManager(), UNO_QUERY_THROW );
            Reference< reflection::XProxyFactory > xProxyFac(
                xMFac->createInstanceWithContext(
                        rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                            "com.sun.star.reflection.ProxyFactory" ) ),
                        m_xCtx  ), UNO_QUERY_THROW );
            m_xAggProxy = xProxyFac->createProxy( m_xWrappedBrowseNode );
        }
        catch(  uno::Exception const & )
        {
            OSL_ENSURE( false, "DefaultBrowseNode::DefaultBrowseNode: Caught exception!" );
        }
        OSL_ENSURE( m_xAggProxy.is(),
            "DefaultBrowseNode::DefaultBrowseNode: Wrapped BrowseNode cannot be aggregated!" );

        if ( m_xAggProxy.is() )
        {

            osl_incrementInterlockedCount( &m_refCount );
            m_xAggProxy->setDelegator( static_cast< cppu::OWeakObject * >( this ) );
            osl_decrementInterlockedCount( &m_refCount );
        }
    }

    ~DefaultBrowseNode()
    {
        if ( m_xAggProxy.is() )
        {
            m_xAggProxy->setDelegator( uno::Reference< uno::XInterface >() );
        }
    }

    virtual Sequence< Reference< browse::XBrowseNode > > SAL_CALL
                getChildNodes()
    throw ( RuntimeException )
    {
        if ( hasChildNodes() )
        {
            vXBrowseNodes m_vNodes;
            Sequence < Reference< browse::XBrowseNode > > nodes =
                m_xWrappedBrowseNode->getChildNodes();
            for ( sal_Int32 i=0; i<nodes.getLength(); i++ )
            {
                m_vNodes.push_back( new DefaultBrowseNode( m_xCtx, nodes[ i ] ) );
            }

            ::std::sort( m_vNodes.begin(), m_vNodes.end(), alphaSortForBNodes() );
            Sequence < Reference< browse::XBrowseNode > > children( m_vNodes.size() );
            vXBrowseNodes::const_iterator it = m_vNodes.begin();
            for ( sal_Int32 i=0; it != m_vNodes.end() && i<children.getLength(); i++, ++it )
            {
                children[ i ].set( *it );
            }
            return children;
        }
        else
        {
            // no nodes

            Sequence < Reference< browse::XBrowseNode > > none;
            return none;
        }
    }

    virtual sal_Int16 SAL_CALL getType()
        throw ( RuntimeException )
    {
        return m_xWrappedBrowseNode->getType();
    }

    virtual ::rtl::OUString
    SAL_CALL getName()
    throw ( RuntimeException )
    {
        return m_xWrappedBrowseNode->getName();
    }

    virtual sal_Bool SAL_CALL
    hasChildNodes()
        throw ( RuntimeException )
    {
        return m_xWrappedBrowseNode->hasChildNodes();
    }

    // XInterface
    virtual Any SAL_CALL queryInterface( const Type& aType )
        throw ( com::sun::star::uno::RuntimeException )
    {
        Any aRet = t_BrowseNodeBase::queryInterface( aType );
        if ( aRet.hasValue() )
        {
            return aRet;
        }
        if ( m_xAggProxy.is() )
        {
            return m_xAggProxy->queryAggregation( aType );
        }
        else
        {
            return Any();
        }
    }

    virtual void SAL_CALL acquire()
        throw ()

    {
        osl_incrementInterlockedCount( &m_refCount );
    }
    virtual void SAL_CALL release()
        throw ()
    {
        if ( osl_decrementInterlockedCount( &m_refCount ) == 0 )
        {
            delete this;
        }
    }
    // XTypeProvider (implemnented by base, but needs to be overridden for
    //                delegating to aggregate)
    virtual Sequence< Type > SAL_CALL getTypes()
        throw ( com::sun::star::uno::RuntimeException )
    {
        return m_xWrappedTypeProv->getTypes();
    }
    virtual Sequence< sal_Int8 > SAL_CALL getImplementationId()
        throw ( com::sun::star::uno::RuntimeException )
    {
        return m_xWrappedTypeProv->getImplementationId();

    }
};

class DefaultRootBrowseNode :
    public ::cppu::WeakImplHelper1< browse::XBrowseNode >
{

private:
    vXBrowseNodes m_vNodes;
    ::rtl::OUString m_Name;

    DefaultRootBrowseNode();
public:
    DefaultRootBrowseNode( const Reference< XComponentContext >& xCtx )
    {
        Sequence < Reference< browse::XBrowseNode > > nodes =
            getAllBrowseNodes( xCtx );
        for ( sal_Int32 i=0; i<nodes.getLength(); i++ )
        {
            m_vNodes.push_back( new DefaultBrowseNode( xCtx, nodes[ i ] ) );
        }
        m_Name = ::rtl::OUString::createFromAscii( "Root" );

        m_Name = ::rtl::OUString::createFromAscii( "Root" );
    }

    virtual Sequence< Reference< browse::XBrowseNode > > SAL_CALL
                getChildNodes()
    throw ( RuntimeException )
    {
        // no need to sort user, share, doc1...docN
        //::std::sort( m_vNodes.begin(), m_vNodes.end(), alphaSortForBNodes() );
        Sequence < Reference< browse::XBrowseNode > > children( m_vNodes.size() );
        vXBrowseNodes::const_iterator it = m_vNodes.begin();
        for ( sal_Int32 i=0; it != m_vNodes.end() && i<children.getLength(); i++, ++it )
        {
            children[ i ].set( *it );
        }
        return children;
    }

    virtual sal_Int16 SAL_CALL getType()
        throw ( RuntimeException )
    {
        return browse::BrowseNodeTypes::ROOT;
    }

    virtual ::rtl::OUString
    SAL_CALL getName()
    throw ( RuntimeException )
    {
        return m_Name;
    }

    virtual sal_Bool SAL_CALL
    hasChildNodes()
        throw ( RuntimeException )
    {
        sal_Bool result = sal_True;
        if ( !m_vNodes.size() )
        {
            result = sal_False;
        }
        return result;
    }
};


class SelectorBrowseNode :
    public ::cppu::WeakImplHelper1< browse::XBrowseNode >
{
private:
    Reference< XComponentContext > m_xComponentContext;

public:
    SelectorBrowseNode( const Reference< XComponentContext >& xContext )
      : m_xComponentContext( xContext )
    {
        OSL_TRACE("CREATING SelectorBrowseNode");
    }

    ~SelectorBrowseNode()
    {
        OSL_TRACE("DESTROYING SelectorBrowseNode");
    }

    virtual ::rtl::OUString SAL_CALL getName()
        throw ( RuntimeException )
    {
        return ::rtl::OUString::createFromAscii( "Root" );
    }

    virtual Sequence< Reference< browse::XBrowseNode > > SAL_CALL
    getChildNodes()
        throw ( RuntimeException )
    {

        Sequence < Reference < browse::XBrowseNode > > locnBNs = getAllBrowseNodes( m_xComponentContext );

        Sequence<  Reference< browse::XBrowseNode > > children(
            locnBNs.getLength() );

        for ( sal_Int32 j = 0; j < locnBNs.getLength(); j++ )
        {
            OSL_TRACE("SelectorNode::getChildNodes() Processing %d of %d", j, locnBNs.getLength() );
            children[j] = new LocationBrowseNode( locnBNs[j] );
        }

        return children;
    }

    virtual sal_Bool SAL_CALL hasChildNodes()
        throw ( RuntimeException )
    {
        return sal_True; // will always be user and share
    }

    virtual sal_Int16 SAL_CALL getType()
        throw ( RuntimeException )
    {
        return browse::BrowseNodeTypes::CONTAINER;
    }
};

BrowseNodeFactoryImpl::BrowseNodeFactoryImpl(
    Reference< XComponentContext > const & xComponentContext )
    : m_xComponentContext( xComponentContext )
{
    OSL_TRACE("BrowseNodeFactoryImpl is being created!");
}

BrowseNodeFactoryImpl::~BrowseNodeFactoryImpl()
{
    OSL_TRACE("BrowseNodeFactoryImpl is being destructed!");
}


//############################################################################
// Implementation of XBrowseNodeFactory
//############################################################################

/*
 * The selector hierarchy is the standard hierarchy for organizers with the
 * language nodes removed.
 */
Reference< browse::XBrowseNode > SAL_CALL
BrowseNodeFactoryImpl::getView( sal_Int16 viewType )
    throw (RuntimeException)
{
    switch( viewType )
    {
        case browse::BrowseNodeFactoryViewType::SCRIPTSELECTOR:
            return getSelectorHierarchy();
        case browse::BrowseNodeFactoryViewType::SCRIPTORGANIZER:
            return getOrganizerHierarchy();
        default:
            throw RuntimeException( OUSTR("Unknown view type" ), Reference< XInterface >() );
    }
}

Reference< browse::XBrowseNode >
BrowseNodeFactoryImpl::getSelectorHierarchy()
    throw (RuntimeException)
{
    OSL_TRACE("Getting selector hierarchy from BrowseNodeFactoryImpl");
    /*if ( !m_xSelectorBrowseNode.is() )
    {
        m_xSelectorBrowseNode = new SelectorBrowseNode( m_xComponentContext );
    }*/
    return new SelectorBrowseNode( m_xComponentContext );
}

Reference< browse::XBrowseNode >
BrowseNodeFactoryImpl::getOrganizerHierarchy()
    throw (RuntimeException)
{
    Reference< browse::XBrowseNode > xRet = new  DefaultRootBrowseNode( m_xComponentContext );
    return xRet;
}
//############################################################################
// Helper methods
//############################################################################

//############################################################################
// Namespace global methods for setting up BrowseNodeFactory service
//############################################################################

Sequence< ::rtl::OUString > SAL_CALL
bnf_getSupportedServiceNames( )
    SAL_THROW( () )
{
    ::rtl::OUString str_name = ::rtl::OUString::createFromAscii(
        "drafts.com.sun.star.script.browse.BrowseNodeFactory");

    return Sequence< ::rtl::OUString >( &str_name, 1 );
}

::rtl::OUString SAL_CALL
bnf_getImplementationName( )
    SAL_THROW( () )
{
    return ::rtl::OUString::createFromAscii(
        "drafts.com.sun.star.script.browse.BrowseNodeFactory" );
}

Reference< XInterface > SAL_CALL
bnf_create( Reference< XComponentContext > const & xComponentContext )
    SAL_THROW( (Exception) )
{
    return static_cast< ::cppu::OWeakObject * >(
        new BrowseNodeFactoryImpl( xComponentContext ) );
}

//############################################################################
// Implementation of XServiceInfo
//############################################################################

::rtl::OUString SAL_CALL
BrowseNodeFactoryImpl::getImplementationName()
    throw (RuntimeException)
{
    return bnf_getImplementationName();
}

Sequence< ::rtl::OUString > SAL_CALL
BrowseNodeFactoryImpl::getSupportedServiceNames()
    throw (RuntimeException)
{
    return bnf_getSupportedServiceNames();
}

sal_Bool BrowseNodeFactoryImpl::supportsService(
    ::rtl::OUString const & serviceName )
    throw (RuntimeException)
{
//     check();

    Sequence< ::rtl::OUString > supported_services(
        getSupportedServiceNames() );

    ::rtl::OUString const * ar = supported_services.getConstArray();

    for ( sal_Int32 pos = supported_services.getLength(); pos--; )
    {
        if (ar[ pos ].equals( serviceName ))
            return sal_True;
    }
    return sal_False;
}

} // namespace browsenodefactory
