/*************************************************************************
 *
 *  $RCSfile: confproviderimpl2.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jb $ $Date: 2000-11-07 14:34:32 $
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

#include <stdio.h>
#include "confproviderimpl2.hxx"
#include "apifactoryimpl.hxx"
#include "apitreeimplobj.hxx"
#include "apitreeaccess.hxx"
#include "roottree.hxx"
#include "noderef.hxx"
#include "objectregistry.hxx"

#ifndef CONFIGMGR_API_PROVIDER2_HXX_
#include "confprovider2.hxx"
#endif
#ifndef CONFIGMGR_MODULE_HXX_
#include "configmodule.hxx"
#endif

#ifndef _CONFIGMGR_TREECACHE_HXX_
#include "treecache.hxx"
#endif
#ifndef _CONFIGMGR_TRACER_HXX_
#include "tracer.hxx"
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

namespace configmgr
{
    namespace css   = ::com::sun::star;
    namespace uno   = css::uno;
    namespace beans = css::beans;

    using ::rtl::OUString;

    using configapi::NodeElement;
    using configuration::RootTree;
    namespace configapi
    {
        class ApiProviderInstances
        {

            typedef configuration::TemplateProvider TemplateProvider;
            ObjectRegistryHolder    m_aObjectRegistry;
            ReadOnlyObjectFactory   m_aReaderFactory;
            UpdateObjectFactory     m_aWriterFactory;
            ApiProvider             m_aReaderProvider;
            ApiProvider             m_aWriterProvider;
        public:
            ApiProviderInstances(ConfigurationProviderImpl2& rProviderImpl)
            : m_aObjectRegistry(new ObjectRegistry())
            , m_aReaderFactory(m_aReaderProvider,m_aObjectRegistry)
            , m_aWriterFactory(m_aWriterProvider,m_aObjectRegistry)
            , m_aReaderProvider(m_aReaderFactory,rProviderImpl)
            , m_aWriterProvider(m_aWriterFactory,rProviderImpl)
            {
            }

            ~ApiProviderInstances()
            {}

            ApiProvider&    getReaderProvider() { return m_aReaderProvider; }
            ApiProvider&    getWriterProvider() { return m_aWriterProvider; }
            Factory&    getReaderFactory() { return m_aReaderFactory; }
            Factory&    getWriterFactory() { return m_aWriterFactory; }
            Factory&    getAdminFactory() { return m_aWriterFactory; }
        };
    }

    ConfigurationProviderImpl2::ConfigurationProviderImpl2(ConfigurationProvider2* _pProvider, IConfigSession* _pSession, const uno::Reference< script::XTypeConverter >& _rxTypeConverter)
        : m_pConfiguration(new TreeManager(_pSession, _rxTypeConverter))
        , m_aNotifier(m_pConfiguration)
        , m_pProvider(_pProvider)
        , m_xConverter(_rxTypeConverter)
        , m_pNewProviders(new configapi::ApiProviderInstances(*this))
    {
        m_pConfiguration->acquire();
    }

    ConfigurationProviderImpl2::~ConfigurationProviderImpl2()
    {
        delete m_pNewProviders;
        m_aNotifier.dispose();
        m_pConfiguration->release();
        m_pConfiguration = NULL;
    }

    ISubtree const* ConfigurationProviderImpl2::getSubtree( OUString const& aComponentName ) const
    {
        return m_pConfiguration->getSubtree(aComponentName);
    }

    ISubtree* ConfigurationProviderImpl2::requestSubtree( OUString const& aSubtreePath, sal_Int16 nMinLevels) throw (container::NoSuchElementException)
    {
        return m_pConfiguration->requestSubtree(aSubtreePath, nMinLevels);
    }

    void ConfigurationProviderImpl2::updateTree(TreeChangeList& aChanges) throw (lang::WrappedTargetException, uno::RuntimeException)
    {
        m_pConfiguration->updateTree(aChanges);
    }

    ::std::auto_ptr<INode> ConfigurationProviderImpl2::createInstance(const ::rtl::OUString& _rTemplateName) throw (uno::Exception)
    {
        return m_pConfiguration->createInstance(_rTemplateName);
    }

    const INode* ConfigurationProviderImpl2::getNode(const OUString& _rPath)
    {
        return m_pConfiguration->getNode(_rPath);
    }

    void SAL_CALL ConfigurationProviderImpl2::acquire(  ) throw ()
    {
        m_pProvider->acquire();
    }

    void SAL_CALL ConfigurationProviderImpl2::release(  ) throw ()
    {
        m_pProvider->release();
    }

    uno::XInterface* ConfigurationProviderImpl2::getProviderInstance()
    {
        return static_cast<com::sun::star::lang::XMultiServiceFactory*>(m_pProvider);
    }

    //==========================================================================
    ISynchronizedData& ConfigurationProviderImpl2::getTreeLock()
    {
        return *m_pConfiguration;
    }

    ISynchronizedData const& ConfigurationProviderImpl2::getTreeLock() const
    {
        return *m_pConfiguration;
    }

    //==========================================================================
    void ConfigurationProviderImpl2::acquireReadAccess() const
    {
        getTreeLock().acquireReadAccess();
    }

    void ConfigurationProviderImpl2::releaseReadAccess() const
    {
        getTreeLock().releaseReadAccess();
    }

    void ConfigurationProviderImpl2::acquireWriteAccess()
    {
        getTreeLock().acquireWriteAccess();
    }

    void ConfigurationProviderImpl2::releaseWriteAccess()
    {
        getTreeLock().releaseWriteAccess();
    }
    //==========================================================================

    //--------------------------------------------------------------------------
    uno::Reference<uno::XInterface>  ConfigurationProviderImpl2::createReadAccess( uno::Sequence<uno::Any> const& aArgs)
    {
        CFG_TRACE_INFO("config provider: going to create a read access instance");

        // extract the args
        sal_Int32 nLevels;
        ::rtl::OUString sNodeAccessor;
        extractArgs(aArgs, sNodeAccessor, nLevels);

        rtl::OUString sLocale;
        extractLocale(aArgs, sLocale);
        m_pConfiguration->setLocale(sLocale);

        CFG_TRACE_INFO_NI("config provider: node accessor extracted from the args is %s", OUSTRING2ASCII(sNodeAccessor));
        CFG_TRACE_INFO_NI("config provider: level depth extracted from the args is %i", nLevels);

        // create the access object
        uno::Reference< uno::XInterface > xReturn;
        if (aArgs.getLength() != 0)
        {
            NodeElement* pElement = buildReadAccess(sNodeAccessor, nLevels);
            if (pElement != 0)
            {
                xReturn = pElement->getUnoInstance();
                if (xReturn.is())
                    // acquired once by buildReadAccess
                    xReturn->release();
            }
        }
        return xReturn;
    }


    //--------------------------------------------------------------------------
    uno::Reference<uno::XInterface>  ConfigurationProviderImpl2::createUpdateAccess( uno::Sequence<uno::Any> const& aArgs)
    {
        CFG_TRACE_INFO("config provider: going to create an update access instance");

        // extract the args
        sal_Int32 nLevels;
        ::rtl::OUString sNodeAccessor;
        extractArgs(aArgs, sNodeAccessor, nLevels);
        CFG_TRACE_INFO_NI("config provider: node accessor extracted from the args is %s", OUSTRING2ASCII(sNodeAccessor));
        CFG_TRACE_INFO_NI("config provider: level depth extracted from the args is %i", nLevels);

        rtl::OUString sLocale;
        extractLocale(aArgs, sLocale);
        m_aI18n.setLocale(sLocale);

        // create the access object
        uno::Reference< uno::XInterface > xReturn;
        if (aArgs.getLength() != 0)
        {
            NodeElement* pElement = buildUpdateAccess(sNodeAccessor, nLevels);
            if (pElement != 0)
            {
                xReturn = pElement->getUnoInstance();
                if (xReturn.is())
                    // acquired once by buildReadAccess
                    xReturn->release();
            }
        }

        return xReturn;
    }

    //--------------------------------------------------------------------------
    uno::Reference<uno::XInterface>  ConfigurationProviderImpl2::createUserAdminAccess( uno::Sequence<uno::Any> const& aArgs)
    {
        CFG_TRACE_INFO("config provider: going to create an user admin access instance");

        // no args supported
        if (aArgs.getLength() != 0)
        {
            CFG_TRACE_ERROR_NI("config provider: no arguments supported creating user admin access instance - %i arguments ignored.",aArgs.getLength());
        }
//      extractArgs(aArgs, sNodeAccessor, nLevels);

        sal_Int32 const nLevels = 1;
        ::rtl::OUString sNodeAccessor = IConfigSession::getUserAdminAccessor();

        CFG_TRACE_INFO_NI("config provider: fixed node accessor is %s", OUSTRING2ASCII(sNodeAccessor));
        CFG_TRACE_INFO_NI("config provider: fixed level depth is %i", nLevels);

        // create the access object
        uno::Reference< uno::XInterface > xReturn;
        if (aArgs.getLength() != 0)
        {
            NodeElement* pElement = buildUpdateAccess(sNodeAccessor, nLevels);
            if (pElement != 0)
            {
                xReturn = pElement->getUnoInstance();
                if (xReturn.is())
                    // acquired once by buildReadAccess
                    xReturn->release();
            }
        }
        return xReturn;
    }


    // parser methods
    //--------------------------------------------------------------------------
    void createAndFillAHashSet(HashSet& aArgs)
    {
        static bFilled = false;
        if (!bFilled)
        {
            aArgs.insert(ASCII("nodepath"));
            aArgs.insert(ASCII("user"));
            aArgs.insert(ASCII("depth"));
            aArgs.insert(ASCII("locale"));

            bFilled = true;
        }
    }

    bool lookup(HashSet &aSet, rtl::OUString aWord)
    {
        HashSet::const_iterator it = aSet.find(aWord);
        return it != aSet.end() ? true : false;
    }

    bool ConfigurationProviderImpl2::checkArgs(const uno::Sequence<uno::Any>& _rArgs) throw (lang::IllegalArgumentException)
    {
        // PRE: a Sequence with some possible arguments
        beans::PropertyValue aCurrent;
        const uno::Any* pCurrent = _rArgs.getConstArray();

        static HashSet aSet;
        createAndFillAHashSet(aSet);
        bool bParamOk = false;
        for (sal_Int32 i=0; i<_rArgs.getLength(); ++i, ++pCurrent)
        {
            if (*pCurrent >>= aCurrent)
            {
                if (!lookup(aSet, aCurrent.Name))
                {
                    rtl::OString aStr = "The argument '";
                    aStr += rtl::OUStringToOString(aCurrent.Name,RTL_TEXTENCODING_ASCII_US).getStr();
                    aStr += "' could not be extracted.";
                    OSL_ENSHURE(false, aStr.getStr());
                }
            }
            else
            {
                OSL_ENSHURE(false, "operator >>= failed.");
            }
        }
        return true;
    }

    //--------------------------------------------------------------------------
    void ConfigurationProviderImpl2::extractArgs(
        const uno::Sequence<uno::Any>& _rArgs,
        ::rtl::OUString& /* [out] */ _rNodeAccessor,
        sal_Int32& /* [out] */ _nLevels)
        throw (lang::IllegalArgumentException)
    {

#ifdef DBG_UTIL
        checkArgs(_rArgs);
#endif
        ::rtl::OUString sUser, sPath;
        sal_Int32 nLevelDepth = ITreeProvider::ALL_LEVELS;

        // the args have to be a sequence of property values, currently three property names are recognized
        beans::PropertyValue aCurrent;
        sal_Bool bAnyPropValue = sal_False;
        const uno::Any* pCurrent = _rArgs.getConstArray();
        for (sal_Int32 i=0; i<_rArgs.getLength(); ++i, ++pCurrent)
        {
            if (*pCurrent >>= aCurrent)
            {
                static const ::rtl::OUString sNodePathParam(RTL_CONSTASCII_USTRINGPARAM("nodepath"));
                static const ::rtl::OUString sUserParam(RTL_CONSTASCII_USTRINGPARAM("user"));
                static const ::rtl::OUString sDepthParam(RTL_CONSTASCII_USTRINGPARAM("depth"));

                sal_Bool bExtractSuccess = sal_True;    // defaulted to TRUE, so we skip unknown arguments
                if (aCurrent.Name.equalsIgnoreCase(sNodePathParam))
                    bExtractSuccess = (aCurrent.Value >>= sPath);
                else if (aCurrent.Name.equalsIgnoreCase(sUserParam))
                    bExtractSuccess = (aCurrent.Value >>= sUser);
                else if (aCurrent.Name.equalsIgnoreCase(sDepthParam))
                    bExtractSuccess = (aCurrent.Value >>= nLevelDepth);
#ifdef DBG_UTIL
                else
                {
                    ::rtl::OString sMessage(RTL_CONSTASCII_STRINGPARAM("ConfigurationProviderImpl2::extractArgs : unknown argument name: "));
                    sMessage += ::rtl::OString(aCurrent.Name.getStr(), aCurrent.Name.getLength(), RTL_TEXTENCODING_ASCII_US);
                    sMessage += ::rtl::OString(RTL_CONSTASCII_STRINGPARAM("!"));
                    OSL_ENSHURE(sal_False, sMessage.getStr());
                }
#endif

                if (!bExtractSuccess)
                    throw   lang::IllegalArgumentException(
                                    (OUString(RTL_CONSTASCII_USTRINGPARAM("The argument ")) += aCurrent.Name) += OUString(RTL_CONSTASCII_USTRINGPARAM(" could not be extracted.")),
                                    uno::Reference<uno::XInterface>(),
                                    i
                            );

                bAnyPropValue = sal_True;
            }
        }

        if (!bAnyPropValue)
        {
            // compatibility : formerly, you could specify the node path as first arg and the (optional) depth
            // as second arg
            if (_rArgs.getLength() > 0)
                if (! (_rArgs[0] >>= sPath) )
                    throw   lang::IllegalArgumentException(
                                    OUString(RTL_CONSTASCII_USTRINGPARAM("The node path specified is invalid.")),
                                    uno::Reference<uno::XInterface>(),
                                    0
                            );

            if (_rArgs.getLength() > 1)
                if (! (_rArgs[1] >>= nLevelDepth) )
                    throw   lang::IllegalArgumentException(
                                    OUString(RTL_CONSTASCII_USTRINGPARAM("The fetch depth specified is invalid.")),
                                    uno::Reference<uno::XInterface>(),
                                    1
                            );
        }

        _rNodeAccessor = IConfigSession::composeNodeAccessor(sPath, sUser);
        _nLevels = nLevelDepth;
    }

    // -----------------------------------------------------------------------------
    void ConfigurationProviderImpl2::extractLocale(
        const uno::Sequence<uno::Any>& _rArgs,
        ::rtl::OUString& /* [out] */ _rLocale)
        throw (lang::IllegalArgumentException)
    {
        // the args have to be a sequence of property values, currently three property names are recognized
        rtl::OUString sLocale;
        beans::PropertyValue aCurrent;
        sal_Bool bAnyPropValue = sal_False;
        const uno::Any* pCurrent = _rArgs.getConstArray();
        for (sal_Int32 i=0; i<_rArgs.getLength(); ++i, ++pCurrent)
        {
            if (*pCurrent >>= aCurrent)
            {
                static const ::rtl::OUString sLocaleParam(RTL_CONSTASCII_USTRINGPARAM("locale"));

                sal_Bool bExtractSuccess = sal_True;    // defaulted to TRUE, so we skip unknown arguments
                if (aCurrent.Name.equalsIgnoreCase(sLocaleParam))
                    bExtractSuccess = (aCurrent.Value >>= sLocale);
/*
#ifdef DBG_UTIL
                else
                {
                    ::rtl::OString sMessage(RTL_CONSTASCII_STRINGPARAM("ConfigurationProviderImpl::extractArgs : unknown argument name: "));
                    sMessage += ::rtl::OString(aCurrent.Name.getStr(), aCurrent.Name.getLength(), RTL_TEXTENCODING_ASCII_US);
                    sMessage += ::rtl::OString(RTL_CONSTASCII_STRINGPARAM("!"));
                    OSL_ENSHURE(sal_False, sMessage.getStr());
                }
#endif
*/
                if (!bExtractSuccess)
                    throw   lang::IllegalArgumentException(
                                    (OUString(RTL_CONSTASCII_USTRINGPARAM("The argument ")) += aCurrent.Name) += OUString(RTL_CONSTASCII_USTRINGPARAM(" could not be extracted.")),
                                    uno::Reference<uno::XInterface>(),
                                    i
                            );

                bAnyPropValue = sal_True;
            }
        }
        _rLocale = sLocale;
    }

    //--------------------------------------------------------------------------
    ISubtree *  ConfigurationProviderImpl2::requestSubtree(OUString const& _rAccessor, sal_Int32 nMinLevels ) throw (container::NoSuchElementException)
    {
        return m_pConfiguration->requestSubtree(_rAccessor,nMinLevels);
    }

    //--------------------------------------------------------------------------
    static OUString getBasePath(OUString const& _rAccessor)
    {
        sal_Int32 nNameStart = _rAccessor.lastIndexOf(ConfigurationName::delimiter);
        if (nNameStart == (_rAccessor.getLength() - 1))
            nNameStart = _rAccessor.lastIndexOf(ConfigurationName::delimiter, nNameStart);
        return nNameStart > 0 ? _rAccessor.copy(0,nNameStart) : ConfigurationName::rootname();
    }

        // actual factory methods
    //--------------------------------------------------------------------------
    NodeElement*  ConfigurationProviderImpl2::buildReadAccess( OUString const& _rAccessor, sal_Int32 nMinLevels)
    {
        CFG_TRACE_INFO("config provider: requesting the tree from the cache manager");

        ISubtree*   pTree = NULL;
        ::rtl::OUString sErrorMessage;
        try
        {
            pTree = requestSubtree(_rAccessor,nMinLevels);
        }
        catch(container::NoSuchElementException&e)
        {
            sErrorMessage = e.Message;
        }

        if (!pTree)
        {
            CFG_TRACE_ERROR("config provider: the cache manager could not provide the tree (neither from the cache nor from the session)");

            ::rtl::OUString sMessage = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("The read access for node "));
            ::rtl::OUString sPurePath, sUser;
            sal_Bool bHadUser = IConfigSession::splitNodeAccessor(_rAccessor, sPurePath, sUser);
            CFG_TRACE_INFO_NI("config provider: the user we tried this for is \"%s\", the path \"%s\"", OUSTRING2ASCII(sUser), OUSTRING2ASCII(sPurePath));
            sMessage += sPurePath;
            if (bHadUser)
            {
                sMessage += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" (and for user "));
                sMessage += sUser;
                sMessage += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(")"));
            }
            sMessage += ::rtl::OUString::createFromAscii(" could not be created. Unable to retrieve the node from the configuration server.");

            // append the error message given by the tree provider
            if (sErrorMessage.getLength())
            {
                sMessage += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\n\nThe server returned the following error:\n"));
                sMessage += sErrorMessage;
            }
            throw uno::Exception(sMessage, static_cast< ::cppu::OWeakObject* > (m_pProvider));
        }
        using namespace configuration;

        RootTree aRootTree( createReadOnlyTree(
                AbsolutePath(getBasePath(_rAccessor), Path::NoValidate()),
                *pTree, nMinLevels
            ));

        return m_pNewProviders->getReaderFactory().makeAccessRoot(aRootTree);
    }


    //--------------------------------------------------------------------------
    NodeElement* ConfigurationProviderImpl2::buildUpdateAccess(OUString const& _rAccessor, sal_Int32 nMinLevels)
    {
        m_pConfiguration->setInternationalHelper(getInternationalHelper());

        CFG_TRACE_INFO("config provider: requesting the tree from the cache manager");

        ISubtree*   pTree = NULL;
        ::rtl::OUString sErrorMessage;
        try
        {
            pTree = requestSubtree(_rAccessor, nMinLevels);
        }
        catch(container::NoSuchElementException&e)
        {
            sErrorMessage = e.Message;
        }

        if (!pTree)
        {
            CFG_TRACE_ERROR("config provider: the cache manager could not provide the tree (neither from the cache nor from the session)");

            ::rtl::OUString sMessage = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("The update access for node "));
            ::rtl::OUString sPurePath, sUser;
            sal_Bool bHadUser = IConfigSession::splitNodeAccessor(_rAccessor, sPurePath, sUser);
            CFG_TRACE_INFO_NI("config provider: the user we tried this for is \"%s\", the path \"%s\"", OUSTRING2ASCII(sUser), OUSTRING2ASCII(sPurePath));
            sMessage += sPurePath;
            if (bHadUser)
            {
                sMessage += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" (and for user "));
                sMessage += sUser;
                sMessage += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(")"));
            }
            sMessage += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" could not be created. Unable to retrieve the node from the configuration server."));

            // append the error message given by the tree provider
            if (sErrorMessage.getLength())
            {
                sMessage += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\n\nThe server returned the following error:\n"));
                sMessage += sErrorMessage;
            }
            throw uno::Exception(sMessage, static_cast< ::cppu::OWeakObject* > (m_pProvider));
        }
        //ConfigurationName aPathToUpdateRoot(getBasePath(_rAccessor), ConfigurationName::Absolute());

        using namespace configuration;

        RootTree aRootTree( createUpdatableTree(
                                AbsolutePath(getBasePath(_rAccessor),Path::NoValidate()),
                                *pTree, nMinLevels
                            ));

        NodeElement* pReturn = 0;
        if (pTree->isSetNode())
        {
            if (_rAccessor == IConfigSession::getUserAdminAccessor())
                pReturn = m_pNewProviders->getAdminFactory().makeAccessRoot(aRootTree);
            else
                pReturn = m_pNewProviders->getWriterFactory().makeAccessRoot(aRootTree);
        }
        else
        {
    #if defined(DEBUG) || defined(_DEBUG)
            if (_rAccessor == IConfigSession::getUserAdminAccessor())
            {   CFG_TRACE_ERROR("config provider: DATA ERROR: User Admin Accessor points to a non-set node"); }
    #endif
            pReturn = m_pNewProviders->getWriterFactory().makeAccessRoot(aRootTree);
        }

        return pReturn;
    }


} // namespace configmgr


