/*************************************************************************
 *
 *  $RCSfile: BrowseNodeFactoryImpl.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-05-19 08:27:46 $
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
#include <cppuhelper/implbase4.hxx>

#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/script/XInvocation.hpp>

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

    Reference< provider::XScriptProviderFactory > xFac;

        Reference< frame::XDesktop > desktop (
            mcf->createInstanceWithContext(
                ::rtl::OUString::createFromAscii("com.sun.star.frame.Desktop"),                 xCtx ),
            UNO_QUERY );

        Reference< container::XEnumerationAccess > componentsAccess =
            desktop->getComponents();

        Reference< container::XEnumeration > components =
            componentsAccess->createEnumeration();

        ::std::vector< Reference< frame::XModel > > vXModels;

        while (components->hasMoreElements())
        {
            Reference< frame::XModel > xModel(
                components->nextElement(), UNO_QUERY );

            if ( xModel.is() )
            {
                vXModels.push_back( xModel );
            }

        }
        sal_Int32 initialSize = vXModels.size() + 2;
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

        ::std::vector< Reference< frame::XModel > >::const_iterator it  = vXModels.begin();


    for ( ; it != vXModels.end() ; ++it )
        {

            try
            {
                Any aCtx = makeAny( *it );
            locnBNs[ mspIndex++ ] = Reference< browse::XBrowseNode >( xFac->createScriptProvider( aCtx ), UNO_QUERY_THROW );
            }
            catch( Exception& e )
            {

                OSL_TRACE("Caught Exception creating MSP for %s exception msg: %s",
                    ::rtl::OUStringToOString( MiscUtils::xModelToDocTitle( *it ), RTL_TEXTENCODING_ASCII_US ).pData->buffer,
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


class DefaultBrowseNode :
    public ::cppu::WeakImplHelper4< browse::XBrowseNode, beans::XPropertySet,
            script::XInvocation, provider::XScriptProvider >
{

private:
    Reference< browse::XBrowseNode > m_xNode;

public:
    DefaultBrowseNode() {}
    DefaultBrowseNode( const Reference< browse::XBrowseNode>& xNode ) : m_xNode( xNode )
    {
    }

    virtual Sequence< Reference< browse::XBrowseNode > > SAL_CALL
            getChildNodes()
    throw ( RuntimeException )
    {
        if ( hasChildNodes() )
        {
            vXBrowseNodes m_vNodes;
            Sequence < Reference< browse::XBrowseNode > > nodes =
                m_xNode->getChildNodes();
            for ( sal_Int32 i=0; i<nodes.getLength(); i++ )
            {
                m_vNodes.push_back( new DefaultBrowseNode( nodes[ i ] ) );
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
        return m_xNode->getType();
    }

    virtual ::rtl::OUString
    SAL_CALL getName()
    throw ( RuntimeException )
    {
        return m_xNode->getName();
    }

    virtual sal_Bool SAL_CALL
    hasChildNodes()
        throw ( RuntimeException )
    {
        return m_xNode->hasChildNodes();
    }

    // XPropertySet
    virtual Reference< beans::XPropertySetInfo > SAL_CALL getPropertySetInfo()
        throw (RuntimeException)
    {
        Reference< beans::XPropertySet > xPropertySet( m_xNode, UNO_QUERY_THROW);
        return xPropertySet->getPropertySetInfo();
    }

    virtual void SAL_CALL setPropertyValue(
        const ::rtl::OUString& aPropertyName, const Any& aValue )
        throw (beans::UnknownPropertyException, beans::PropertyVetoException,
                lang::IllegalArgumentException, lang::WrappedTargetException,
                RuntimeException)
    {
        Reference< beans::XPropertySet > xPropertySet( m_xNode, UNO_QUERY_THROW);
        xPropertySet->setPropertyValue( aPropertyName, aValue );
    }

    virtual Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName )
        throw (beans::UnknownPropertyException, lang::WrappedTargetException,
                RuntimeException)
    {
        Reference< beans::XPropertySet > xPropertySet( m_xNode, UNO_QUERY_THROW);
        return xPropertySet->getPropertyValue( PropertyName );
    }

    virtual void SAL_CALL addPropertyChangeListener(
        const ::rtl::OUString& aPropertyName,
        const Reference< beans::XPropertyChangeListener >& xListener )
        throw (beans::UnknownPropertyException, lang::WrappedTargetException,
                RuntimeException)
    {
        Reference< beans::XPropertySet > xPropertySet( m_xNode, UNO_QUERY_THROW);
        xPropertySet->addPropertyChangeListener( aPropertyName, xListener );
    }

    virtual void SAL_CALL removePropertyChangeListener(
        const ::rtl::OUString& aPropertyName,
        const Reference< beans::XPropertyChangeListener >& aListener )
        throw (beans::UnknownPropertyException, lang::WrappedTargetException,
                RuntimeException)
    {
        Reference< beans::XPropertySet > xPropertySet( m_xNode, UNO_QUERY_THROW);
        xPropertySet->removePropertyChangeListener( aPropertyName, aListener );
    }

    virtual void SAL_CALL addVetoableChangeListener(
        const ::rtl::OUString& PropertyName,
        const Reference< beans::XVetoableChangeListener >& aListener )
        throw (beans::UnknownPropertyException, lang::WrappedTargetException,
                RuntimeException)
    {
        Reference< beans::XPropertySet > xPropertySet( m_xNode, UNO_QUERY_THROW);
        xPropertySet->addVetoableChangeListener( PropertyName, aListener );
    }

    virtual void SAL_CALL removeVetoableChangeListener(
        const ::rtl::OUString& PropertyName,
        const Reference< beans::XVetoableChangeListener >& aListener )
        throw (beans::UnknownPropertyException, lang::WrappedTargetException,
                RuntimeException)
    {
        Reference< beans::XPropertySet > xPropertySet( m_xNode, UNO_QUERY_THROW);
        xPropertySet->removeVetoableChangeListener( PropertyName, aListener );
    }

    //XInvocation
    virtual Reference< beans::XIntrospectionAccess > SAL_CALL
        getIntrospection(  ) throw (RuntimeException)
    {
        return NULL;
    }

    virtual Any SAL_CALL invoke( const ::rtl::OUString& aFunctionName,
        const Sequence< Any >& aParams, Sequence< ::sal_Int16 >& aOutParamIndex,
        Sequence< Any >& aOutParam )
        throw (lang::IllegalArgumentException, script::CannotConvertException,
            reflection::InvocationTargetException, RuntimeException)
    {
        Reference< script::XInvocation > xInvocation( m_xNode, UNO_QUERY_THROW);
        return xInvocation->invoke( aFunctionName, aParams, aOutParamIndex,
            aOutParam );
    }

    virtual void SAL_CALL setValue( const ::rtl::OUString& aPropertyName,
        const Any& aValue )
        throw ( beans::UnknownPropertyException, script::CannotConvertException,
                reflection::InvocationTargetException, RuntimeException)
    {
    }

    virtual Any SAL_CALL getValue( const ::rtl::OUString& aPropertyName )
        throw (beans::UnknownPropertyException, RuntimeException)
    {
        return Any();
    }

    virtual ::sal_Bool SAL_CALL hasMethod( const ::rtl::OUString& aName )
        throw (RuntimeException)
    {
        return sal_False;
    }
    virtual ::sal_Bool SAL_CALL hasProperty( const ::rtl::OUString& aName )
        throw (RuntimeException)
    {
        return sal_False;
    }

    //XScriptProvider
    virtual Reference< provider::XScript > SAL_CALL getScript( const ::rtl::OUString& sScriptURI ) throw ( lang::IllegalArgumentException, RuntimeException )
    {
        Reference< provider::XScriptProvider > xSP( m_xNode, UNO_QUERY_THROW);
        return xSP->getScript( sScriptURI );
    }

};

class DefaultRootBrowseNode :
    public ::cppu::WeakImplHelper1< browse::XBrowseNode >
{

private:
    vXBrowseNodes m_vNodes;
    ::rtl::OUString m_Name;

public:
    DefaultRootBrowseNode() {}
    DefaultRootBrowseNode( const Reference< XComponentContext >& xCtx )
    {
        Sequence < Reference< browse::XBrowseNode > > nodes =
            getAllBrowseNodes( xCtx );
        for ( sal_Int32 i=0; i<nodes.getLength(); i++ )
        {
            m_vNodes.push_back( new DefaultBrowseNode( nodes[ i ] ) );
        }
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
