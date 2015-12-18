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

#include <unotools/confignode.hxx>
#include <unotools/configpaths.hxx>
#include <tools/diagnose_ex.h>
#include <osl/diagnose.h>
#include <com/sun/star/container/XHierarchicalName.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/util/XStringEscape.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <comphelper/namedvaluecollection.hxx>
#include <rtl/string.hxx>
#if OSL_DEBUG_LEVEL > 0
#include <rtl/strbuf.hxx>
#endif

namespace utl
{

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::configuration;

    //= OConfigurationNode

    OConfigurationNode::OConfigurationNode(const Reference< XInterface >& _rxNode )
        :m_bEscapeNames(false)
    {
        OSL_ENSURE(_rxNode.is(), "OConfigurationNode::OConfigurationNode: invalid node interface!");
        if (_rxNode.is())
        {
            // collect all interfaces necessary
            m_xHierarchyAccess.set(_rxNode, UNO_QUERY);
            m_xDirectAccess.set(_rxNode, UNO_QUERY);

            // reset _all_ interfaces if _one_ of them is not supported
            if (!m_xHierarchyAccess.is() || !m_xDirectAccess.is())
            {
                m_xHierarchyAccess = nullptr;
                m_xDirectAccess = nullptr;
            }

            // now for the non-critical interfaces
            m_xReplaceAccess.set(_rxNode, UNO_QUERY);
            m_xContainerAccess.set(_rxNode, UNO_QUERY);
        }

        Reference< XComponent > xConfigNodeComp(m_xDirectAccess, UNO_QUERY);
        if (xConfigNodeComp.is())
            startComponentListening(xConfigNodeComp);

        if (isValid())
            setEscape(isSetNode());
    }

    OConfigurationNode::OConfigurationNode(const OConfigurationNode& _rSource)
        :OEventListenerAdapter()
        ,m_xHierarchyAccess(_rSource.m_xHierarchyAccess)
        ,m_xDirectAccess(_rSource.m_xDirectAccess)
        ,m_xReplaceAccess(_rSource.m_xReplaceAccess)
        ,m_xContainerAccess(_rSource.m_xContainerAccess)
        ,m_bEscapeNames(_rSource.m_bEscapeNames)
        ,m_sCompletePath(_rSource.m_sCompletePath)
    {
        Reference< XComponent > xConfigNodeComp(m_xDirectAccess, UNO_QUERY);
        if (xConfigNodeComp.is())
            startComponentListening(xConfigNodeComp);
    }

    const OConfigurationNode& OConfigurationNode::operator=(const OConfigurationNode& _rSource)
    {
        stopAllComponentListening();

        m_xHierarchyAccess = _rSource.m_xHierarchyAccess;
        m_xDirectAccess = _rSource.m_xDirectAccess;
        m_xContainerAccess = _rSource.m_xContainerAccess;
        m_xReplaceAccess = _rSource.m_xReplaceAccess;
        m_bEscapeNames = _rSource.m_bEscapeNames;
        m_sCompletePath = _rSource.m_sCompletePath;

        Reference< XComponent > xConfigNodeComp(m_xDirectAccess, UNO_QUERY);
        if (xConfigNodeComp.is())
            startComponentListening(xConfigNodeComp);

        return *this;
    }

    void OConfigurationNode::_disposing( const EventObject& _rSource )
    {
        Reference< XComponent > xDisposingSource(_rSource.Source, UNO_QUERY);
        Reference< XComponent > xConfigNodeComp(m_xDirectAccess, UNO_QUERY);
        if (xDisposingSource.get() == xConfigNodeComp.get())
            clear();
    }

    OUString OConfigurationNode::getLocalName() const
    {
        OUString sLocalName;
        try
        {
            Reference< XNamed > xNamed( m_xDirectAccess, UNO_QUERY_THROW );
            sLocalName = xNamed->getName();
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return sLocalName;
    }

    OUString OConfigurationNode::normalizeName(const OUString& _rName, NAMEORIGIN _eOrigin) const
    {
        OUString sName(_rName);
        if (getEscape())
        {
            Reference< XStringEscape > xEscaper(m_xDirectAccess, UNO_QUERY);
            if (xEscaper.is() && !sName.isEmpty())
            {
                try
                {
                    if (NO_CALLER == _eOrigin)
                        sName = xEscaper->escapeString(sName);
                    else
                        sName = xEscaper->unescapeString(sName);
                }
                catch(Exception&)
                {
                    DBG_UNHANDLED_EXCEPTION();
                }
            }
        }
        return sName;
    }

    Sequence< OUString > OConfigurationNode::getNodeNames() const throw()
    {
        OSL_ENSURE(m_xDirectAccess.is(), "OConfigurationNode::getNodeNames: object is invalid!");
        Sequence< OUString > aReturn;
        if (m_xDirectAccess.is())
        {
            try
            {
                aReturn = m_xDirectAccess->getElementNames();
                // normalize the names
                OUString* pNames = aReturn.getArray();
                for (sal_Int32 i=0; i<aReturn.getLength(); ++i, ++pNames)
                    *pNames = normalizeName(*pNames, NO_CONFIGURATION);
            }
            catch(Exception&)
            {
                OSL_FAIL("OConfigurationNode::getNodeNames: caught a generic exception!");
            }
        }

        return aReturn;
    }

    bool OConfigurationNode::removeNode(const OUString& _rName) const throw()
    {
        OSL_ENSURE(m_xContainerAccess.is(), "OConfigurationNode::removeNode: object is invalid!");
        if (m_xContainerAccess.is())
        {
            try
            {
                OUString sName = normalizeName(_rName, NO_CALLER);
                m_xContainerAccess->removeByName(sName);
                return true;
            }
            catch (NoSuchElementException&)
            {
                #if OSL_DEBUG_LEVEL > 0
                OStringBuffer aBuf( 256 );
                aBuf.append("OConfigurationNode::removeNode: there is no element named!");
                aBuf.append( OUStringToOString( _rName, RTL_TEXTENCODING_ASCII_US ) );
                aBuf.append( "!" );
                OSL_FAIL(aBuf.getStr());
                #endif
            }
            catch (WrappedTargetException&)
            {
                OSL_FAIL("OConfigurationNode::removeNode: caught a WrappedTargetException!");
            }
            catch(Exception&)
            {
                OSL_FAIL("OConfigurationNode::removeNode: caught a generic exception!");
            }
        }
        return false;
    }

    OConfigurationNode OConfigurationNode::insertNode(const OUString& _rName,const Reference< XInterface >& _xNode) const throw()
    {
        if(_xNode.is())
        {
            try
            {
                OUString sName = normalizeName(_rName, NO_CALLER);
                m_xContainerAccess->insertByName(sName, makeAny(_xNode));
                // if we're here, all was ok ...
                return OConfigurationNode( _xNode );
            }
            catch(const Exception&)
            {
                DBG_UNHANDLED_EXCEPTION();
            }

            // dispose the child if it has already been created, but could not be inserted
            Reference< XComponent > xChildComp(_xNode, UNO_QUERY);
            if (xChildComp.is())
                try { xChildComp->dispose(); } catch(Exception&) { }
        }

        return OConfigurationNode();
    }

    OConfigurationNode OConfigurationNode::createNode(const OUString& _rName) const throw()
    {
        Reference< XSingleServiceFactory > xChildFactory(m_xContainerAccess, UNO_QUERY);
        OSL_ENSURE(xChildFactory.is(), "OConfigurationNode::createNode: object is invalid or read-only!");

        if (xChildFactory.is()) // implies m_xContainerAccess.is()
        {
            Reference< XInterface > xNewChild;
            try
            {
                xNewChild = xChildFactory->createInstance();
            }
            catch(const Exception&)
            {
                DBG_UNHANDLED_EXCEPTION();
            }
            return insertNode(_rName,xNewChild);
        }

        return OConfigurationNode();
    }

    OConfigurationNode OConfigurationNode::openNode(const OUString& _rPath) const throw()
    {
        OSL_ENSURE(m_xDirectAccess.is(), "OConfigurationNode::openNode: object is invalid!");
        OSL_ENSURE(m_xHierarchyAccess.is(), "OConfigurationNode::openNode: object is invalid!");
        try
        {
            OUString sNormalized = normalizeName(_rPath, NO_CALLER);

            Reference< XInterface > xNode;
            if (m_xDirectAccess.is() && m_xDirectAccess->hasByName(sNormalized))
            {
                xNode.set(
                    m_xDirectAccess->getByName(sNormalized), css::uno::UNO_QUERY);
                if (!xNode.is())
                    OSL_FAIL("OConfigurationNode::openNode: could not open the node!");
            }
            else if (m_xHierarchyAccess.is())
            {
                xNode.set(
                    m_xHierarchyAccess->getByHierarchicalName(_rPath),
                    css::uno::UNO_QUERY);
                if (!xNode.is())
                    OSL_FAIL("OConfigurationNode::openNode: could not open the node!");
            }
            if (xNode.is())
                return OConfigurationNode( xNode );
        }
        catch(const NoSuchElementException&)
        {
            #if OSL_DEBUG_LEVEL > 0
            OStringBuffer aBuf( 256 );
            aBuf.append("OConfigurationNode::openNode: there is no element named ");
            aBuf.append( OUStringToOString( _rPath, RTL_TEXTENCODING_ASCII_US ) );
            aBuf.append("!");
            OSL_FAIL(aBuf.getStr());
            #endif
        }
        catch(Exception&)
        {
            OSL_FAIL("OConfigurationNode::openNode: caught an exception while retrieving the node!");
        }
        return OConfigurationNode();
    }

    void OConfigurationNode::setEscape(bool _bEnable)
    {
        m_bEscapeNames = _bEnable && Reference< XStringEscape >::query(m_xDirectAccess).is();
    }

    bool OConfigurationNode::isSetNode() const
    {
        bool bIsSet = false;
        Reference< XServiceInfo > xSI(m_xHierarchyAccess, UNO_QUERY);
        if (xSI.is())
        {
            try { bIsSet = xSI->supportsService("com.sun.star.configuration.SetAccess"); }
            catch(Exception&) { }
        }
        return bIsSet;
    }

    bool OConfigurationNode::hasByHierarchicalName( const OUString& _rName ) const throw()
    {
        OSL_ENSURE( m_xHierarchyAccess.is(), "OConfigurationNode::hasByHierarchicalName: no hierarchy access!" );
        try
        {
            if ( m_xHierarchyAccess.is() )
            {
                OUString sName = normalizeName( _rName, NO_CALLER );
                return m_xHierarchyAccess->hasByHierarchicalName( sName );
            }
        }
        catch(Exception&)
        {
        }
        return false;
    }

    bool OConfigurationNode::hasByName(const OUString& _rName) const throw()
    {
        OSL_ENSURE(m_xDirectAccess.is(), "OConfigurationNode::hasByName: object is invalid!");
        try
        {
            OUString sName = normalizeName(_rName, NO_CALLER);
            if (m_xDirectAccess.is())
                return m_xDirectAccess->hasByName(sName);
        }
        catch(Exception&)
        {
        }
        return false;
    }

    bool OConfigurationNode::setNodeValue(const OUString& _rPath, const Any& _rValue) const throw()
    {
        bool bResult = false;

        OSL_ENSURE(m_xReplaceAccess.is(), "OConfigurationNode::setNodeValue: object is invalid!");
        if (m_xReplaceAccess.is())
        {
            try
            {
                // check if _rPath is a level-1 path
                OUString sNormalizedName = normalizeName(_rPath, NO_CALLER);
                if (m_xReplaceAccess->hasByName(sNormalizedName))
                {
                    m_xReplaceAccess->replaceByName(sNormalizedName, _rValue);
                    bResult = true;
                }

                // check if the name refers to a indirect descendant
                else if (m_xHierarchyAccess.is() && m_xHierarchyAccess->hasByHierarchicalName(_rPath))
                {
                    OSL_ASSERT(!_rPath.isEmpty());

                    OUString sParentPath, sLocalName;

                    if ( splitLastFromConfigurationPath(_rPath, sParentPath, sLocalName) )
                    {
                        OConfigurationNode aParentAccess = openNode(sParentPath);
                        if (aParentAccess.isValid())
                            bResult = aParentAccess.setNodeValue(sLocalName, _rValue);
                    }
                    else
                    {
                        m_xReplaceAccess->replaceByName(sLocalName, _rValue);
                        bResult = true;
                    }
                }

            }
            catch(IllegalArgumentException&)
            {
                OSL_FAIL("OConfigurationNode::setNodeValue: could not replace the value: caught an IllegalArgumentException!");
            }
            catch(NoSuchElementException&)
            {
                OSL_FAIL("OConfigurationNode::setNodeValue: could not replace the value: caught a NoSuchElementException!");
            }
            catch(WrappedTargetException&)
            {
                OSL_FAIL("OConfigurationNode::setNodeValue: could not replace the value: caught a WrappedTargetException!");
            }
            catch(Exception&)
            {
                OSL_FAIL("OConfigurationNode::setNodeValue: could not replace the value: caught a generic Exception!");
            }

        }
        return bResult;
    }

    Any OConfigurationNode::getNodeValue(const OUString& _rPath) const throw()
    {
        OSL_ENSURE(m_xDirectAccess.is(), "OConfigurationNode::hasByName: object is invalid!");
        OSL_ENSURE(m_xHierarchyAccess.is(), "OConfigurationNode::hasByName: object is invalid!");
        Any aReturn;
        try
        {
            OUString sNormalizedPath = normalizeName(_rPath, NO_CALLER);
            if (m_xDirectAccess.is() && m_xDirectAccess->hasByName(sNormalizedPath) )
            {
                aReturn = m_xDirectAccess->getByName(sNormalizedPath);
            }
            else if (m_xHierarchyAccess.is())
            {
                aReturn = m_xHierarchyAccess->getByHierarchicalName(_rPath);
            }
        }
        catch(const NoSuchElementException&)
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return aReturn;
    }

    void OConfigurationNode::clear() throw()
    {
        m_xHierarchyAccess.clear();
        m_xDirectAccess.clear();
        m_xReplaceAccess.clear();
        m_xContainerAccess.clear();
    }

    //= helper

    namespace
    {

        Reference< XMultiServiceFactory > lcl_getConfigProvider( const Reference<XComponentContext> & i_rContext )
        {
            try
            {
                Reference< XMultiServiceFactory > xProvider = theDefaultProvider::get( i_rContext );
                return xProvider;
            }
            catch ( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
            return nullptr;
        }

        Reference< XInterface > lcl_createConfigurationRoot( const Reference< XMultiServiceFactory >& i_rxConfigProvider,
            const OUString& i_rNodePath, const bool i_bUpdatable, const sal_Int32 i_nDepth, const bool i_bLazyWrite )
        {
            ENSURE_OR_RETURN( i_rxConfigProvider.is(), "invalid provider", nullptr );
            try
            {
                ::comphelper::NamedValueCollection aArgs;
                aArgs.put( "nodepath", i_rNodePath );
                aArgs.put( "lazywrite", i_bLazyWrite );
                aArgs.put( "depth", i_nDepth );

                OUString sAccessService( i_bUpdatable ?
                                OUString( "com.sun.star.configuration.ConfigurationUpdateAccess" ) :
                                OUString( "com.sun.star.configuration.ConfigurationAccess" ));

                Reference< XInterface > xRoot(
                    i_rxConfigProvider->createInstanceWithArguments( sAccessService, aArgs.getWrappedPropertyValues() ),
                    UNO_SET_THROW
                );
                return xRoot;
            }
            catch ( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
            return nullptr;
        }
    }

    OConfigurationTreeRoot::OConfigurationTreeRoot( const Reference< XInterface >& _rxRootNode )
        :OConfigurationNode( _rxRootNode )
        ,m_xCommitter( _rxRootNode, UNO_QUERY )
    {
    }

    OConfigurationTreeRoot::OConfigurationTreeRoot( const Reference<XComponentContext> & i_rContext, const OUString& i_rNodePath, const bool i_bUpdatable )
        :OConfigurationNode( lcl_createConfigurationRoot( lcl_getConfigProvider( i_rContext ),
            i_rNodePath, i_bUpdatable, -1, false ).get() )
        ,m_xCommitter()
    {
        if ( i_bUpdatable )
        {
            m_xCommitter.set( getUNONode(), UNO_QUERY );
            OSL_ENSURE( m_xCommitter.is(), "OConfigurationTreeRoot::OConfigurationTreeRoot: could not create an updatable node!" );
        }
    }

    void OConfigurationTreeRoot::clear() throw()
    {
        OConfigurationNode::clear();
        m_xCommitter.clear();
    }

    bool OConfigurationTreeRoot::commit() const throw()
    {
        OSL_ENSURE(isValid(), "OConfigurationTreeRoot::commit: object is invalid!");
        if (!isValid())
            return false;
        OSL_ENSURE(m_xCommitter.is(), "OConfigurationTreeRoot::commit: I'm a readonly node!");
        if (!m_xCommitter.is())
            return false;

        try
        {
            m_xCommitter->commitChanges();
            return true;
        }
        catch(const Exception&)
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return false;
    }

    OConfigurationTreeRoot OConfigurationTreeRoot::createWithProvider(const Reference< XMultiServiceFactory >& _rxConfProvider, const OUString& _rPath, sal_Int32 _nDepth, CREATION_MODE _eMode, bool _bLazyWrite)
    {
        Reference< XInterface > xRoot( lcl_createConfigurationRoot(
            _rxConfProvider, _rPath, _eMode != CM_READONLY, _nDepth, _bLazyWrite ) );
        if ( xRoot.is() )
            return OConfigurationTreeRoot( xRoot );
        return OConfigurationTreeRoot();
    }

    OConfigurationTreeRoot OConfigurationTreeRoot::createWithComponentContext( const Reference< XComponentContext >& _rxContext, const OUString& _rPath, sal_Int32 _nDepth, CREATION_MODE _eMode, bool _bLazyWrite )
    {
        return createWithProvider( lcl_getConfigProvider( _rxContext ), _rPath, _nDepth, _eMode, _bLazyWrite );
    }

    OConfigurationTreeRoot OConfigurationTreeRoot::tryCreateWithComponentContext( const Reference< XComponentContext >& rxContext,
        const OUString& _rPath, sal_Int32 _nDepth , CREATION_MODE _eMode , bool _bLazyWrite )
    {
        OSL_ENSURE( rxContext.is(), "OConfigurationTreeRoot::tryCreateWithComponentContext: invalid XComponentContext!" );
        try
        {
            Reference< XMultiServiceFactory > xConfigFactory = theDefaultProvider::get( rxContext );
            return createWithProvider( xConfigFactory, _rPath, _nDepth, _eMode, _bLazyWrite );
        }
        catch(const Exception&)
        {
            // silence this, 'cause the contract of this method states "no assertions"
        }
        return OConfigurationTreeRoot();
    }

}   // namespace utl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
