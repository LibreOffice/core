/*************************************************************************
 *
 *  $RCSfile: confignode.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: fs $ $Date: 2001-08-21 12:40:41 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <unotools/confignode.hxx>

#ifndef UNOTOOLS_CONFIGPATHES_HXX_INCLUDED
#include <unotools/configpathes.hxx>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XHIERARCHICALNAME_HPP_
#include <com/sun/star/container/XHierarchicalName.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XSTRINGESCAPE_HPP_
#include <com/sun/star/util/XStringEscape.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _RTL_STRING_HXX_
#include <rtl/string.hxx>
#endif

//........................................................................
namespace utl
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::container;

    //========================================================================
    //= OConfigurationNode
    //========================================================================
    //------------------------------------------------------------------------
    OConfigurationNode::OConfigurationNode(const Reference< XInterface >& _rxNode, const Reference< XMultiServiceFactory >& _rxProvider)
        :m_xProvider(_rxProvider)
        ,m_bEscapeNames(sal_False)
    {
        OSL_ENSURE(_rxNode.is(), "OConfigurationNode::OConfigurationNode: invalid node interface!");
        if (_rxNode.is())
        {
            // collect all interfaces necessary
            m_xHierarchyAccess = Reference< XHierarchicalNameAccess >(_rxNode, UNO_QUERY);
            m_xDirectAccess = Reference< XNameAccess >(_rxNode, UNO_QUERY);

            // reset _all_ interfaces if _one_ of them is not supported
            if (!m_xHierarchyAccess.is() || !m_xDirectAccess.is())
            {
                m_xHierarchyAccess = NULL;
                m_xDirectAccess = NULL;
            }

            // now for the non-critical interfaces
            m_xReplaceAccess = Reference< XNameReplace >(_rxNode, UNO_QUERY);
            m_xContainerAccess = Reference< XNameContainer >(_rxNode, UNO_QUERY);
        }

        Reference< XComponent > xConfigNodeComp(m_xDirectAccess, UNO_QUERY);
        if (xConfigNodeComp.is())
            startComponentListening(xConfigNodeComp);

        if (isValid())
            setEscape(isSetNode());
    }

    //------------------------------------------------------------------------
    OConfigurationNode::OConfigurationNode(const OConfigurationNode& _rSource)
        :OEventListenerAdapter()
        ,m_xHierarchyAccess(_rSource.m_xHierarchyAccess)
        ,m_xDirectAccess(_rSource.m_xDirectAccess)
        ,m_xContainerAccess(_rSource.m_xContainerAccess)
        ,m_xReplaceAccess(_rSource.m_xReplaceAccess)
        ,m_xProvider(_rSource.m_xProvider)
        ,m_bEscapeNames(_rSource.m_bEscapeNames)
        ,m_sCompletePath(_rSource.m_sCompletePath)
    {
        Reference< XComponent > xConfigNodeComp(m_xDirectAccess, UNO_QUERY);
        if (xConfigNodeComp.is())
            startComponentListening(xConfigNodeComp);
    }

    //------------------------------------------------------------------------
    const OConfigurationNode& OConfigurationNode::operator=(const OConfigurationNode& _rSource)
    {
        stopAllComponentListening();

        m_xHierarchyAccess = _rSource.m_xHierarchyAccess;
        m_xDirectAccess = _rSource.m_xDirectAccess;
        m_xContainerAccess = _rSource.m_xContainerAccess;
        m_xReplaceAccess = _rSource.m_xReplaceAccess;
        m_xProvider = _rSource.m_xProvider;
        m_bEscapeNames = _rSource.m_bEscapeNames;
        m_sCompletePath = _rSource.m_sCompletePath;

        Reference< XComponent > xConfigNodeComp(m_xDirectAccess, UNO_QUERY);
        if (xConfigNodeComp.is())
            startComponentListening(xConfigNodeComp);

        return *this;
    }

    //------------------------------------------------------------------------
    OConfigurationNode::~OConfigurationNode()
    {
    }

    //------------------------------------------------------------------------
    void OConfigurationNode::_disposing( const EventObject& _rSource )
    {
        Reference< XComponent > xDisposingSource(_rSource.Source, UNO_QUERY);
        Reference< XComponent > xConfigNodeComp(m_xDirectAccess, UNO_QUERY);
        if (xDisposingSource.get() == xConfigNodeComp.get())
            clear();
    }

    //------------------------------------------------------------------------
    ::rtl::OUString OConfigurationNode::normalizeName(const ::rtl::OUString& _rName, NAMEORIGIN _eOrigin) const
    {
        ::rtl::OUString sName(_rName);
        if (getEscape())
        {
            Reference< XStringEscape > xEscaper(m_xDirectAccess, UNO_QUERY);
            OSL_ENSURE(xEscaper.is(), "OConfigurationNode::normalizeName: missing an interface!");
            if (xEscaper.is() && sName.getLength())
            {
                try
                {
                    if (NO_CALLER == _eOrigin)
                        sName = xEscaper->escapeString(sName);
                    else
                        sName = xEscaper->unescapeString(sName);
                }
                catch(IllegalArgumentException&)
                {
                    OSL_ENSURE(sal_False, "OConfigurationNode::normalizeName: illegal argument (caught an exception saying so)!");
                }
                catch(Exception&)
                {
                    OSL_ENSURE(sal_False, "OConfigurationNode::normalizeName: caught an exception!");
                }
            }
        }
        return sName;
    }

    //------------------------------------------------------------------------
    Sequence< ::rtl::OUString > OConfigurationNode::getNodeNames() const throw()
    {
        OSL_ENSURE(m_xDirectAccess.is(), "OConfigurationNode::getNodeNames: object is invalid!");
        Sequence< ::rtl::OUString > aReturn;
        if (m_xDirectAccess.is())
        {
            try
            {
                aReturn = m_xDirectAccess->getElementNames();
                // normalize the names
                ::rtl::OUString* pNames = aReturn.getArray();
                for (sal_Int32 i=0; i<aReturn.getLength(); ++i, ++pNames)
                    *pNames = normalizeName(*pNames, NO_CONFIGURATION);
            }
            catch(Exception&)
            {
                OSL_ENSURE(sal_False, "OConfigurationNode::getNodeNames: caught a generic exception!");
            }
        }

        return aReturn;
    }

    //------------------------------------------------------------------------
    sal_Bool OConfigurationNode::removeNode(const ::rtl::OUString& _rName) const throw()
    {
        OSL_ENSURE(m_xContainerAccess.is(), "OConfigurationNode::removeNode: object is invalid!");
        if (m_xContainerAccess.is())
        {
            try
            {
                ::rtl::OUString sName = normalizeName(_rName, NO_CALLER);
                m_xContainerAccess->removeByName(sName);
                return sal_True;
            }
            catch (NoSuchElementException&)
            {
                OSL_ENSURE(sal_False,
                        ::rtl::OString("OConfigurationNode::removeNode: there is no element named!")
                    +=  ::rtl::OString(_rName.getStr(), _rName.getLength(), RTL_TEXTENCODING_ASCII_US)
                    +=  ::rtl::OString("!"));
            }
            catch (WrappedTargetException&)
            {
                OSL_ENSURE(sal_False, "OConfigurationNode::removeNode: caught a WrappedTargetException!");
            }
            catch(Exception&)
            {
                OSL_ENSURE(sal_False, "OConfigurationNode::removeNode: caught a generic exception!");
            }
        }
        return sal_False;
    }
    //------------------------------------------------------------------------
    OConfigurationNode OConfigurationNode::insertNode(const ::rtl::OUString& _rName,const Reference< XInterface >& _xNode) const throw()
    {
        if(_xNode.is())
        {
            try
            {
                ::rtl::OUString sName = normalizeName(_rName, NO_CALLER);
                m_xContainerAccess->insertByName(sName, makeAny(_xNode));
                // if we're here, all was ok ...
                return OConfigurationNode(_xNode, m_xProvider);
            }
            catch (IllegalArgumentException&)
            {
                OSL_ENSURE(sal_False, "OConfigurationNode::createNode: caught an IllegalArgumentException!");
            }
            catch (ElementExistException&)
            {
                OSL_ENSURE(sal_False, "OConfigurationNode::createNode: caught an ElementExistException!");
            }
            catch (WrappedTargetException&)
            {
                OSL_ENSURE(sal_False, "OConfigurationNode::createNode: caught a WrappedTargetException!");
            }
            catch(Exception&)
            {
                OSL_ENSURE(sal_False, "OConfigurationNode::createNode: caught a generic exception!");
            }

            // dispose the child if it has already been created, but could not be inserted
            Reference< XComponent > xChildComp(_xNode, UNO_QUERY);
            if (xChildComp.is())
                try { xChildComp->dispose(); } catch(Exception&) { }
        }

        return OConfigurationNode();
    }
    //------------------------------------------------------------------------
    OConfigurationNode OConfigurationNode::createNode(const ::rtl::OUString& _rName) const throw()
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
            catch(Exception&)
            {
                OSL_ENSURE(sal_False, "OConfigurationNode::createNode: caught a generic exception!");
            }
            return insertNode(_rName,xNewChild);
        }

        return OConfigurationNode();
    }

    //------------------------------------------------------------------------
    OConfigurationNode OConfigurationNode::appendNode(const ::rtl::OUString& _rName,const OConfigurationNode& _aNewNode) const throw()
    {
        return insertNode(_rName,_aNewNode.m_xDirectAccess);
    }
    //------------------------------------------------------------------------
    OConfigurationNode OConfigurationNode::openNode(const ::rtl::OUString& _rPath) const throw()
    {
        OSL_ENSURE(m_xDirectAccess.is(), "OConfigurationNode::openNode: object is invalid!");
        OSL_ENSURE(m_xHierarchyAccess.is(), "OConfigurationNode::openNode: object is invalid!");
        try
        {
            ::rtl::OUString sNormalized = normalizeName(_rPath, NO_CALLER);

            Reference< XInterface > xNode;
            if (m_xDirectAccess.is() && m_xDirectAccess->hasByName(sNormalized))
            {
                if (!::cppu::extractInterface(xNode, m_xDirectAccess->getByName(sNormalized)))
                    OSL_ENSURE(sal_False, "OConfigurationNode::openNode: could not open the node!");
            }
            else if (m_xHierarchyAccess.is())
            {
                if (!::cppu::extractInterface(xNode, m_xHierarchyAccess->getByHierarchicalName(_rPath)))
                    OSL_ENSURE(sal_False, "OConfigurationNode::openNode: could not open the node!");
            }
            if (xNode.is())
                return OConfigurationNode(xNode, m_xProvider);
        }
        catch(NoSuchElementException& e)
        {
            e; // make compiler happy (in the pro version)
            OSL_ENSURE(sal_False,
                        ::rtl::OString("OConfigurationNode::openNode: there is no element named ")
                    +=  ::rtl::OString(_rPath.getStr(), _rPath.getLength(), RTL_TEXTENCODING_ASCII_US)
                    +=  ::rtl::OString("!"));
        }
        catch(Exception&)
        {
            OSL_ENSURE(sal_False, "OConfigurationNode::openNode: caught an exception while retrieving the node!");
        }
        return OConfigurationNode();
    }

    //------------------------------------------------------------------------
    void OConfigurationNode::setEscape(sal_Bool _bEnable)
    {
        m_bEscapeNames = _bEnable && Reference< XStringEscape >::query(m_xDirectAccess).is();
        OSL_ENSURE(m_bEscapeNames || !_bEnable,
            "OConfigurationNode::setEscape: escaping not enabled - missing the appropriate interface on the node!");
    }

    //------------------------------------------------------------------------
    sal_Bool OConfigurationNode::isSetNode() const
    {
        sal_Bool bIsSet = sal_False;
        Reference< XServiceInfo > xSI(m_xHierarchyAccess, UNO_QUERY);
        if (xSI.is())
        {
            try { bIsSet = xSI->supportsService(::rtl::OUString::createFromAscii("com.sun.star.configuration.SetAccess")); }
            catch(Exception&) { }
        }
        return bIsSet;
    }

    //---------------------------------------------------------------------
    //--- 20.08.01 19:03:20 -----------------------------------------------

    sal_Bool OConfigurationNode::hasByHierarchicalName( const ::rtl::OUString& _rName ) const throw()
    {
        OSL_ENSURE( m_xHierarchyAccess.is(), "OConfigurationNode::hasByHierarchicalName: no hierarchy access!" );
        try
        {
            if ( m_xHierarchyAccess.is() )
            {
                ::rtl::OUString sName = normalizeName( _rName, NO_CALLER );
                return m_xHierarchyAccess->hasByHierarchicalName( sName );
            }
        }
        catch(Exception&)
        {
        }
        return sal_False;
    }

    //------------------------------------------------------------------------
    sal_Bool OConfigurationNode::hasByName(const ::rtl::OUString& _rName) const throw()
    {
        OSL_ENSURE(m_xDirectAccess.is(), "OConfigurationNode::hasByName: object is invalid!");
        try
        {
            ::rtl::OUString sName = normalizeName(_rName, NO_CALLER);
            if (m_xDirectAccess.is())
                return m_xDirectAccess->hasByName(sName);
        }
        catch(Exception&)
        {
        }
        return sal_False;
    }

    //------------------------------------------------------------------------
    sal_Bool OConfigurationNode::setNodeValue(const ::rtl::OUString& _rPath, const Any& _rValue) const throw()
    {
        sal_Bool bResult = false;

        OSL_ENSURE(m_xReplaceAccess.is(), "OConfigurationNode::setNodeValue: object is invalid!");
        if (m_xReplaceAccess.is())
        {
            try
            {
                // check if _rPath is a level-1 path
                ::rtl::OUString sNormalizedName = normalizeName(_rPath, NO_CALLER);
                if (m_xReplaceAccess->hasByName(sNormalizedName))
                {
                    m_xReplaceAccess->replaceByName(sNormalizedName, _rValue);
                    bResult = true;
                }

                // check if the name refers to a indirect descendant
                else if (m_xHierarchyAccess.is() && m_xHierarchyAccess->hasByHierarchicalName(_rPath))
                {
                    OSL_ASSERT(_rPath.getLength());

                    ::rtl::OUString sParentPath, sLocalName;

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
                OSL_ENSURE(sal_False, "OConfigurationNode::setNodeValue: could not replace the value: caught an IllegalArgumentException!");
            }
            catch(NoSuchElementException&)
            {
                OSL_ENSURE(sal_False, "OConfigurationNode::setNodeValue: could not replace the value: caught a NoSuchElementException!");
            }
            catch(WrappedTargetException&)
            {
                OSL_ENSURE(sal_False, "OConfigurationNode::setNodeValue: could not replace the value: caught a WrappedTargetException!");
            }
            catch(Exception&)
            {
                OSL_ENSURE(sal_False, "OConfigurationNode::setNodeValue: could not replace the value: caught a generic Exception!");
            }


        }
        return bResult;
    }

    //------------------------------------------------------------------------
    Any OConfigurationNode::getNodeValue(const ::rtl::OUString& _rPath) const throw()
    {
        OSL_ENSURE(m_xDirectAccess.is(), "OConfigurationNode::hasByName: object is invalid!");
        OSL_ENSURE(m_xHierarchyAccess.is(), "OConfigurationNode::hasByName: object is invalid!");
        Any aReturn;
        try
        {
            ::rtl::OUString sNormalizedPath = normalizeName(_rPath, NO_CALLER);
            if (m_xDirectAccess.is() && m_xDirectAccess->hasByName(sNormalizedPath) )
            {
                aReturn = m_xDirectAccess->getByName(sNormalizedPath);
            }
            else if (m_xHierarchyAccess.is())
            {
                aReturn = m_xHierarchyAccess->getByHierarchicalName(_rPath);
            }
        }
        catch(NoSuchElementException& e)
        {
            e;  // make compiler happy
            OSL_ENSURE(sal_False,
                ::rtl::OString("OConfigurationNode::getNodeValue: caught a NoSuchElementException while trying to open ")
            +=  ::rtl::OString(e.Message.getStr(), e.Message.getLength(), RTL_TEXTENCODING_ASCII_US)
            +=  ::rtl::OString("!"));
        }
        return aReturn;
    }

    //------------------------------------------------------------------------
    OConfigurationTreeRoot OConfigurationNode::cloneAsRoot() const throw()
    {
        OSL_ENSURE(m_xHierarchyAccess.is(), "OConfigurationNode::cloneAsRoot: object is invalid!");
        if (m_xHierarchyAccess.is())
        {
            // first get the complete path of the node we represent
            ::rtl::OUString sCompletePath;
            Reference< XHierarchicalName > xNodeNameAccess(m_xHierarchyAccess, UNO_QUERY);
            if (xNodeNameAccess.is())
            {
                try
                {
                    sCompletePath = xNodeNameAccess->getHierarchicalName();
                    OSL_ENSURE(sCompletePath.getLength(), "OConfigurationNode::cloneAsRoot: invalid path retrieved!");
                }
                catch(Exception&)
                {
                    OSL_ENSURE(sal_False, "OConfigurationNode::cloneAsRoot: could not retrieve the node path!");
                }
            }

            // then create a new tree root object with that path and our provider
            OSL_ENSURE(m_xProvider.is(), "OConfigurationNode::cloneAsRoot: have an invalid provider!");
            if (sCompletePath.getLength() && m_xProvider.is())
            {
                return OConfigurationTreeRoot::createWithProvider(m_xProvider, sCompletePath, -1, isReadonly() ? OConfigurationTreeRoot::CM_READONLY : OConfigurationTreeRoot::CM_PREFER_UPDATABLE);
            }
        }
        return OConfigurationTreeRoot();
    }

    //------------------------------------------------------------------------
    void OConfigurationNode::clear() throw()
    {
        m_xHierarchyAccess.clear();
        m_xDirectAccess.clear();
        m_xReplaceAccess.clear();
        m_xContainerAccess.clear();
    }

    //========================================================================
    //= OConfigurationTreeRoot
    //========================================================================
    //------------------------------------------------------------------------
    OConfigurationTreeRoot::OConfigurationTreeRoot(const Reference< XChangesBatch >& _rxRootNode, const Reference< XMultiServiceFactory >& _rxProvider)
        :OConfigurationNode(_rxRootNode.get(), _rxProvider)
        ,m_xCommitter(_rxRootNode)
    {
    }

    //------------------------------------------------------------------------
    OConfigurationTreeRoot::OConfigurationTreeRoot(const Reference< XInterface >& _rxRootNode, const Reference< XMultiServiceFactory >& _rxProvider)
        :OConfigurationNode(_rxRootNode.get(), _rxProvider)
    {
    }

    //------------------------------------------------------------------------
    void OConfigurationTreeRoot::clear() throw()
    {
        OConfigurationNode::clear();
        m_xCommitter.clear();
    }

    //------------------------------------------------------------------------
    sal_Bool OConfigurationTreeRoot::commit() const throw()
    {
        OSL_ENSURE(isValid(), "OConfigurationTreeRoot::commit: object is invalid!");
        if (!isValid())
            return sal_False;
        OSL_ENSURE(m_xCommitter.is(), "OConfigurationTreeRoot::commit: I'm a readonly node!");
        if (!m_xCommitter.is())
            return sal_False;

        try
        {
            m_xCommitter->commitChanges();
            return sal_True;
        }
        catch(WrappedTargetException&)
        {
            OSL_ENSURE(sal_False, "OConfigurationTreeRoot::commit: caught a WrappedTargetException!");
        }
        catch(RuntimeException&)
        {
            OSL_ENSURE(sal_False, "OConfigurationTreeRoot::commit: caught a RuntimeException!");
        }
        return sal_False;
    }

    //------------------------------------------------------------------------
    OConfigurationTreeRoot OConfigurationTreeRoot::createWithProvider(const Reference< XMultiServiceFactory >& _rxConfProvider, const ::rtl::OUString& _rPath, sal_Int32 _nDepth, CREATION_MODE _eMode, sal_Bool _bLazyWrite)
    {
        OSL_ENSURE(_rxConfProvider.is(), "OConfigurationTreeRoot::createWithProvider: invalid provider!");

#ifdef DBG_UTIL
        if (_rxConfProvider.is())
        {
            try
            {
                Reference< XServiceInfo > xSI(_rxConfProvider, UNO_QUERY);
                if (!xSI.is())
                {
                    OSL_ENSURE(sal_False, "OConfigurationTreeRoot::createWithProvider: no XServiceInfo interface on the provider!");
                }
                else
                {
                    OSL_ENSURE(xSI->supportsService(::rtl::OUString::createFromAscii("com.sun.star.configuration.ConfigurationProvider")),
                        "OConfigurationTreeRoot::createWithProvider: sure this is a provider? Missing the ConfigurationProvider service!");
                }
            }
            catch(const Exception&)
            {
                OSL_ENSURE(sal_False, "OConfigurationTreeRoot::createWithProvider: unable to check the service conformance of the provider given!");
            }
        }
#endif

        sal_Bool bTryAgain(sal_False);
        do
        {
            if (_rxConfProvider.is())
            {
                try
                {
                    Sequence< Any > aCreationArgs(3);
                    aCreationArgs[0] = makeAny(PropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("nodepath")), 0, makeAny(_rPath), PropertyState_DIRECT_VALUE));
                    aCreationArgs[1] = makeAny(PropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("depth")), 0, makeAny((sal_Int32)_nDepth), PropertyState_DIRECT_VALUE));
                    aCreationArgs[2] = makeAny(PropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("lazywrite")), 0, ::cppu::bool2any(_bLazyWrite), PropertyState_DIRECT_VALUE));

                    ::rtl::OUString sAccessService = ::rtl::OUString::createFromAscii(CM_READONLY == _eMode
                        ? "com.sun.star.configuration.ConfigurationAccess"
                        : "com.sun.star.configuration.ConfigurationUpdateAccess");

                    Reference< XInterface > xRoot = _rxConfProvider->createInstanceWithArguments(sAccessService, aCreationArgs);
                    if (!xRoot.is())
                    {
                        OSL_ENSURE(sal_False, "OConfigurationTreeRoot::createWithProvider: could not create the node access!");
                    }
                    else if (CM_READONLY == _eMode)
                    {
                        return OConfigurationTreeRoot(xRoot, _rxConfProvider);
                    }
                    else
                    {   // get the changes batch interface
                        Reference< XChangesBatch > xCommitter(xRoot, UNO_QUERY);
                        if (xCommitter.is())
                            return OConfigurationTreeRoot(xCommitter, _rxConfProvider);
                        else
                            OSL_ENSURE(sal_False, "OConfigurationTreeRoot::createWithProvider: invalid root object (missing interface XChangesBatch)!");

                        // dispose the object if it is already created, but unusable
                        Reference< XComponent > xComp(xRoot, UNO_QUERY);
                        if (xComp.is())
                            try { xComp->dispose(); } catch(Exception&) { }
                    }
                }
                catch(Exception&)
                {
                    OSL_ENSURE(sal_False, "OConfigurationTreeRoot::createWithProvider: caught an exception while creating the access object!");
                }
            }
            bTryAgain = CM_PREFER_UPDATABLE == _eMode;
            if (bTryAgain)
                _eMode = CM_READONLY;
        }
        while (bTryAgain);

        return OConfigurationTreeRoot();
    }

    //------------------------------------------------------------------------
    OConfigurationTreeRoot OConfigurationTreeRoot::createWithServiceFactory(const Reference< XMultiServiceFactory >& _rxORB, const ::rtl::OUString& _rPath, sal_Int32 _nDepth, CREATION_MODE _eMode, sal_Bool _bLazyWrite)
    {
        OSL_ENSURE(_rxORB.is(), "OConfigurationTreeRoot::createWithServiceFactory: invalid provider!");
        if (_rxORB.is())
        {
            try
            {
                Reference< XInterface > xProvider = _rxORB->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.configuration.ConfigurationProvider")));
                OSL_ENSURE(xProvider.is(), "OConfigurationTreeRoot::createWithServiceFactory: could not instantiate the config provider service!");
                Reference< XMultiServiceFactory > xProviderAsFac(xProvider, UNO_QUERY);
                OSL_ENSURE(xProviderAsFac.is() || !xProvider.is(), "OConfigurationTreeRoot::createWithServiceFactory: the provider is missing an interface!");
                if (xProviderAsFac.is())
                    return createWithProvider(xProviderAsFac, _rPath, _nDepth, _eMode, _bLazyWrite);
            }
            catch(Exception&)
            {
                OSL_ENSURE(sal_False, "OConfigurationTreeRoot::createWithServiceFactory: error while instantiating the provider service!");
            }
        }
        return OConfigurationTreeRoot();
    }

//........................................................................
}   // namespace utl
//........................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.4  2001/07/26 09:10:58  oj
 *  #89831# new method to append an existing node with different name
 *
 *  Revision 1.3  2001/07/10 11:30:37  jb
 *  #87904# Use public helpers for handling of new configuration pathes
 *
 *  Revision 1.2  2001/07/05 15:43:16  jb
 *  #87904# Adjusted to new configuration path format
 *
 *  Revision 1.1  2001/06/13 16:27:29  fs
 *  initial checkin - non-UNO wrapper for configuration nodes
 *
 *
 *  Revision 1.0 13.06.01 17:05:36  fs
 ************************************************************************/

