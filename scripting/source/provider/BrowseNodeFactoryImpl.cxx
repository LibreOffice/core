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


#include <cppuhelper/weakref.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <unotools/mediadescriptor.hxx>

#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/reflection/ProxyFactory.hpp>

#include <com/sun/star/script/provider/theMasterScriptProviderFactory.hpp>
#include <com/sun/star/script/browse/BrowseNodeFactoryViewTypes.hpp>
#include <com/sun/star/script/browse/BrowseNodeTypes.hpp>
#include <com/sun/star/document/XScriptInvocationContext.hpp>

#include <tools/diagnose_ex.h>

#include "BrowseNodeFactoryImpl.hxx"
#include "MasterScriptProvider.hxx"
#include "ActiveMSPList.hxx"
#include <util/MiscUtils.hxx>

#include <vector>
#include <algorithm>
#include <memory>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::script;
using namespace ::sf_misc;

namespace browsenodefactory
{
class BrowseNodeAggregator :
    public ::cppu::WeakImplHelper< browse::XBrowseNode >
{
private:
    OUString m_Name;
    std::vector< Reference< browse::XBrowseNode > > m_Nodes;

public:

    explicit BrowseNodeAggregator( const Reference< browse::XBrowseNode >& node )
    {
        m_Name = node->getName();
        m_Nodes.resize( 1 );
        m_Nodes[ 0 ] = node;
    }

    void addBrowseNode( const Reference< browse::XBrowseNode>& node )
    {
        m_Nodes.push_back( node );
    }

    virtual OUString
    SAL_CALL getName() override
    {
        return m_Name;
    }

    virtual Sequence< Reference< browse::XBrowseNode > > SAL_CALL
    getChildNodes() override
    {
        std::vector<  Sequence< Reference < browse::XBrowseNode > > > seqs;
        seqs.reserve( m_Nodes.size() );

        sal_Int32 numChildren = 0;

        for (Reference<XBrowseNode> & xNode : m_Nodes)
        {
            Sequence< Reference < browse::XBrowseNode > > children;
            try
            {
                children = xNode->getChildNodes();
                seqs.push_back( children );
                numChildren += children.getLength();
            }
            catch ( Exception& )
            {
                // some form of exception getting child nodes so they
                // won't be displayed
            }
        }

        std::vector<  Sequence< Reference < browse::XBrowseNode > > >::const_iterator it = seqs.begin();
        std::vector<  Sequence< Reference < browse::XBrowseNode > > >::const_iterator it_end = seqs.end();

        Sequence< Reference < browse::XBrowseNode > > result( numChildren );
        for ( sal_Int32 index = 0; it != it_end && index < numChildren ; ++it )
        {
            Sequence< Reference < browse::XBrowseNode > > children = *it;
            for ( sal_Int32 j = 0; j < children.getLength(); j++ )
            {
                result[ index++ ] = children[ j ];
            }
        }
        return result;
    }

    virtual sal_Bool SAL_CALL
    hasChildNodes() override
    {
        if ( !m_Nodes.empty() )
        {
            for (Reference<XBrowseNode> & xNode : m_Nodes)
            {
                try
                {
                    if ( xNode->hasChildNodes() )
                    {
                        return true;
                    }
                }
                catch ( Exception& )
                {
                    // some form of exception getting child nodes so move
                    // on to the next one
                }
            }
        }

        return false;
    }

    virtual sal_Int16 SAL_CALL getType() override
    {
        return browse::BrowseNodeTypes::CONTAINER;
    }
};

//typedef std::map< OUString, Reference< browse::XBrowseNode > >
typedef std::unordered_map< OUString, Reference< browse::XBrowseNode >,
    OUStringHash >
        BrowseNodeAggregatorHash;
typedef std::vector< OUString > vString;

struct alphaSort
{
    bool operator()( const OUString& a, const OUString& b )
    {
        return a.compareTo( b ) < 0;
    }
};
class LocationBrowseNode :
    public ::cppu::WeakImplHelper< browse::XBrowseNode >
{
private:
    std::unique_ptr<BrowseNodeAggregatorHash> m_hBNA;
    vString m_vStr;
    OUString m_sNodeName;
    Reference< browse::XBrowseNode > m_origNode;

public:

    explicit LocationBrowseNode( const Reference< browse::XBrowseNode >& node )
    {
        m_sNodeName = node->getName();
        m_hBNA = nullptr;
        m_origNode.set( node );
    }


    // XBrowseNode

    virtual OUString SAL_CALL getName() override
    {
        return m_sNodeName;
    }

    virtual Sequence< Reference< browse::XBrowseNode > > SAL_CALL
    getChildNodes() override
    {
        if ( m_hBNA == nullptr )
        {
            loadChildNodes();
        }

        Sequence<  Reference< browse::XBrowseNode > > children( m_hBNA->size() );
        sal_Int32 index = 0;

        vString::const_iterator it = m_vStr.begin();

        for ( ; it != m_vStr.end(); ++it, index++ )
        {
            children[ index ].set( m_hBNA->find( *it )->second );
        }

        return children;
    }

    virtual sal_Bool SAL_CALL hasChildNodes() override
    {
        return true;
    }

    virtual sal_Int16 SAL_CALL getType() override
    {
        return browse::BrowseNodeTypes::CONTAINER;
    }

private:

    void loadChildNodes()
    {
        m_hBNA.reset( new BrowseNodeAggregatorHash );

        Sequence< Reference< browse::XBrowseNode > > langNodes =
            m_origNode->getChildNodes();

        for ( sal_Int32 i = 0; i < langNodes.getLength(); i++ )
        {
            Reference< browse::XBrowseNode > xbn;
            if ( langNodes[ i ]->getName() == "uno_packages" )
            {
                xbn.set( new LocationBrowseNode( langNodes[ i ] ) );
            }
            else
            {
                xbn.set( langNodes[ i ] );
            }

            Sequence< Reference< browse::XBrowseNode > > grandchildren =
                xbn->getChildNodes();

            for ( sal_Int32 j = 0; j < grandchildren.getLength(); j++ )
            {
                Reference< browse::XBrowseNode > grandchild(grandchildren[j]);

                BrowseNodeAggregatorHash::iterator h_it =
                    m_hBNA->find( grandchild->getName() );

                if ( h_it != m_hBNA->end() )
                {
                    BrowseNodeAggregator* bna = static_cast< BrowseNodeAggregator* >( h_it->second.get() );
                    bna->addBrowseNode( grandchild );
                }
                else
                {
                    Reference< browse::XBrowseNode > bna(
                        new BrowseNodeAggregator( grandchild ) );
                    (*m_hBNA)[ grandchild->getName() ].set( bna );
                    m_vStr.push_back( grandchild->getName() );
                }
            }
        }
        // sort children alphabetically
        ::std::sort( m_vStr.begin(), m_vStr.end(), alphaSort() );
    }
};

namespace
{

std::vector< Reference< browse::XBrowseNode > > getAllBrowseNodes( const Reference< XComponentContext >& xCtx )
{
    Sequence< OUString > openDocs =
        MiscUtils::allOpenTDocUrls( xCtx );

    Reference< provider::XScriptProviderFactory > xFac;
    sal_Int32 initialSize = openDocs.getLength() + 2;
    sal_Int32 mspIndex = 0;

    std::vector< Reference < browse::XBrowseNode > > locnBNs( initialSize );
    try
    {
        xFac = provider::theMasterScriptProviderFactory::get( xCtx );

        locnBNs[ mspIndex++ ].set( xFac->createScriptProvider( makeAny( OUString("user") ) ), UNO_QUERY_THROW );
        locnBNs[ mspIndex++ ].set( xFac->createScriptProvider( makeAny( OUString("share") ) ), UNO_QUERY_THROW );
    }
    // TODO proper exception handling, should throw
    catch( const Exception& e )
    {
        SAL_WARN("scripting", "Caught " << e );
        locnBNs.resize( mspIndex );
        return locnBNs;
    }

    for ( sal_Int32 i = 0; i < openDocs.getLength(); i++ )
    {
        try
        {
            Reference< frame::XModel > model( MiscUtils::tDocUrlToModel( openDocs[ i ] ), UNO_QUERY_THROW );

            // #i44599 Check if it's a real document or something special like Hidden/Preview
            css::uno::Reference< css::frame::XController > xCurrentController = model->getCurrentController();
            if( xCurrentController.is() )
            {
                utl::MediaDescriptor aMD( model->getArgs() );
                bool bDefault = false;
                bool bHidden  = aMD.getUnpackedValueOrDefault( utl::MediaDescriptor::PROP_HIDDEN(),  bDefault );
                bool bPreview = aMD.getUnpackedValueOrDefault( utl::MediaDescriptor::PROP_PREVIEW(), bDefault );
                if( !bHidden && !bPreview )
                {
                    Reference< document::XEmbeddedScripts > xScripts( model, UNO_QUERY );
                    if ( xScripts.is() )
                        locnBNs[ mspIndex++ ].set( xFac->createScriptProvider( makeAny( model ) ), UNO_QUERY_THROW );
                }
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

    }

    std::vector< Reference < browse::XBrowseNode > > locnBNs_Return( mspIndex );
    for ( sal_Int32 j = 0; j < mspIndex; j++ )
        locnBNs_Return[j] = locnBNs[j];

    return locnBNs_Return;
}

} // namespace

typedef ::std::vector< Reference< browse::XBrowseNode > > vXBrowseNodes;

struct alphaSortForBNodes
{
    bool operator()( const Reference< browse::XBrowseNode >& a, const Reference< browse::XBrowseNode >& b )
    {
        return a->getName().compareTo( b->getName() ) < 0;
    }
};

typedef ::cppu::WeakImplHelper< browse::XBrowseNode > t_BrowseNodeBase;
class DefaultBrowseNode :
    public t_BrowseNodeBase
{

private:
    Reference< browse::XBrowseNode > m_xWrappedBrowseNode;
    Reference< lang::XTypeProvider > m_xWrappedTypeProv;
    Reference< XAggregation >        m_xAggProxy;
    Reference< XComponentContext >   m_xCtx;

public:
    DefaultBrowseNode( const Reference< XComponentContext >& xCtx, const Reference< browse::XBrowseNode>& xNode ) : m_xWrappedBrowseNode( xNode ), m_xWrappedTypeProv( xNode, UNO_QUERY ), m_xCtx( xCtx )
    {
        OSL_ENSURE( m_xWrappedBrowseNode.is(), "DefaultBrowseNode::DefaultBrowseNode(): No BrowseNode to wrap" );
        OSL_ENSURE( m_xWrappedTypeProv.is(), "DefaultBrowseNode::DefaultBrowseNode(): No BrowseNode to wrap" );
        OSL_ENSURE( m_xCtx.is(), "DefaultBrowseNode::DefaultBrowseNode(): No ComponentContext" );
    // Use proxy factory service to create aggregatable proxy.
        try
        {
            Reference< reflection::XProxyFactory > xProxyFac =
                reflection::ProxyFactory::create( m_xCtx );
            m_xAggProxy = xProxyFac->createProxy( m_xWrappedBrowseNode );
        }
        catch(  uno::Exception& )
        {
            OSL_FAIL( "DefaultBrowseNode::DefaultBrowseNode: Caught exception!" );
        }
        OSL_ENSURE( m_xAggProxy.is(),
            "DefaultBrowseNode::DefaultBrowseNode: Wrapped BrowseNode cannot be aggregated!" );

        if ( m_xAggProxy.is() )
        {
            osl_atomic_increment( &m_refCount );

            /* i35609 - Fix crash on Solaris. The setDelegator call needs
               to be in its own block to ensure that all temporary Reference
               instances that are acquired during the call are released
               before m_refCount is decremented again */
            {
                m_xAggProxy->setDelegator(
                    static_cast< cppu::OWeakObject * >( this ) );
            }

            osl_atomic_decrement( &m_refCount );
        }
    }

    virtual ~DefaultBrowseNode() override
    {
        if ( m_xAggProxy.is() )
        {
            m_xAggProxy->setDelegator( uno::Reference< uno::XInterface >() );
        }
    }

    virtual Sequence< Reference< browse::XBrowseNode > > SAL_CALL
                getChildNodes() override
    {
        if ( hasChildNodes() )
        {
            vXBrowseNodes aVNodes;
            Sequence < Reference< browse::XBrowseNode > > nodes =
                m_xWrappedBrowseNode->getChildNodes();
            for ( sal_Int32 i=0; i<nodes.getLength(); i++ )
            {
                Reference< browse::XBrowseNode > xBrowseNode = nodes[ i ];
                OSL_ENSURE( xBrowseNode.is(), "DefaultBrowseNode::getChildNodes(): Invalid BrowseNode" );
                if( xBrowseNode.is() )
                    aVNodes.push_back( new DefaultBrowseNode( m_xCtx, xBrowseNode ) );
            }

            ::std::sort( aVNodes.begin(), aVNodes.end(), alphaSortForBNodes() );
            Sequence < Reference< browse::XBrowseNode > > children( aVNodes.size() );
            vXBrowseNodes::const_iterator it = aVNodes.begin();
            for ( sal_Int32 i=0; it != aVNodes.end() && i<children.getLength(); i++, ++it )
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

    virtual sal_Int16 SAL_CALL getType() override
    {
        return m_xWrappedBrowseNode->getType();
    }

    virtual OUString
    SAL_CALL getName() override
    {
        return m_xWrappedBrowseNode->getName();
    }

    virtual sal_Bool SAL_CALL
    hasChildNodes() override
    {
        return m_xWrappedBrowseNode->hasChildNodes();
    }

    // XInterface
    virtual Any SAL_CALL queryInterface( const Type& aType ) override
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

    // XTypeProvider (implemnented by base, but needs to be overridden for
    //                delegating to aggregate)
    virtual Sequence< Type > SAL_CALL getTypes() override
    {
        return m_xWrappedTypeProv->getTypes();
    }
    virtual Sequence< sal_Int8 > SAL_CALL getImplementationId() override
    {
        return css::uno::Sequence<sal_Int8>();
    }
};

class DefaultRootBrowseNode :
    public ::cppu::WeakImplHelper< browse::XBrowseNode >
{

private:
    vXBrowseNodes m_vNodes;
    OUString m_Name;

public:
    explicit DefaultRootBrowseNode( const Reference< XComponentContext >& xCtx )
    {
        std::vector< Reference< browse::XBrowseNode > > nodes =
            getAllBrowseNodes( xCtx );

        for (Reference< browse::XBrowseNode > & xNode : nodes)
        {
            m_vNodes.push_back( new DefaultBrowseNode( xCtx, xNode ) );
        }
        m_Name = "Root";
    }

    virtual Sequence< Reference< browse::XBrowseNode > > SAL_CALL
                getChildNodes() override
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

    virtual sal_Int16 SAL_CALL getType() override
    {
        return browse::BrowseNodeTypes::ROOT;
    }

    virtual OUString
    SAL_CALL getName() override
    {
        return m_Name;
    }

    virtual sal_Bool SAL_CALL
    hasChildNodes() override
    {
        bool result = true;
        if ( m_vNodes.empty() )
        {
            result = false;
        }
        return result;
    }
};


class SelectorBrowseNode :
    public ::cppu::WeakImplHelper< browse::XBrowseNode >
{
private:
    Reference< XComponentContext > m_xComponentContext;

public:
    explicit SelectorBrowseNode( const Reference< XComponentContext >& xContext )
      : m_xComponentContext( xContext )
    {
    }

    virtual OUString SAL_CALL getName() override
    {
        return OUString("Root");
    }

    virtual Sequence< Reference< browse::XBrowseNode > > SAL_CALL
    getChildNodes() override
    {

        std::vector< Reference < browse::XBrowseNode > > locnBNs = getAllBrowseNodes( m_xComponentContext );

        Sequence<  Reference< browse::XBrowseNode > > children(
            locnBNs.size() );

        for ( size_t j = 0; j < locnBNs.size(); j++ )
        {
            children[j] = new LocationBrowseNode( locnBNs[j] );
        }

        return children;
    }

    virtual sal_Bool SAL_CALL hasChildNodes() override
    {
        return true; // will always be user and share
    }

    virtual sal_Int16 SAL_CALL getType() override
    {
        return browse::BrowseNodeTypes::CONTAINER;
    }
};

BrowseNodeFactoryImpl::BrowseNodeFactoryImpl(
    Reference< XComponentContext > const & xComponentContext )
    : m_xComponentContext( xComponentContext )
{
}

BrowseNodeFactoryImpl::~BrowseNodeFactoryImpl()
{
}


// Implementation of XBrowseNodeFactory


/*
 * The selector hierarchy is the standard hierarchy for organizers with the
 * language nodes removed.
 */
Reference< browse::XBrowseNode > SAL_CALL
BrowseNodeFactoryImpl::createView( sal_Int16 viewType )
{
    switch( viewType )
    {
        case browse::BrowseNodeFactoryViewTypes::MACROSELECTOR:
            return new SelectorBrowseNode( m_xComponentContext );
        case browse::BrowseNodeFactoryViewTypes::MACROORGANIZER:
            return getOrganizerHierarchy();
        default:
            throw RuntimeException( "Unknown view type" );
    }
}

Reference< browse::XBrowseNode >
BrowseNodeFactoryImpl::getOrganizerHierarchy()
{
    Reference< browse::XBrowseNode > xRet = new  DefaultRootBrowseNode( m_xComponentContext );
    return xRet;
}

// Helper methods


// Namespace global methods for setting up BrowseNodeFactory service


Sequence< OUString > SAL_CALL
bnf_getSupportedServiceNames( )
{
    OUString str_name(
        "com.sun.star.script.browse.BrowseNodeFactory");

    return Sequence< OUString >( &str_name, 1 );
}

OUString SAL_CALL
bnf_getImplementationName( )
{
    return OUString(
        "com.sun.star.script.browse.BrowseNodeFactory" );
}

Reference< XInterface > SAL_CALL
bnf_create( Reference< XComponentContext > const & xComponentContext )
{
    return static_cast< ::cppu::OWeakObject * >(
        new BrowseNodeFactoryImpl( xComponentContext ) );
}


// Implementation of XServiceInfo


OUString SAL_CALL
BrowseNodeFactoryImpl::getImplementationName()
{
    return bnf_getImplementationName();
}

Sequence< OUString > SAL_CALL
BrowseNodeFactoryImpl::getSupportedServiceNames()
{
    return bnf_getSupportedServiceNames();
}

sal_Bool BrowseNodeFactoryImpl::supportsService(OUString const & serviceName )
{
    return cppu::supportsService(this, serviceName);
}

} // namespace browsenodefactory

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
