/*************************************************************************
 *
 *  $RCSfile: providerimpl.cxx,v $
 *
 *  $Revision: 1.26 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-21 12:12:15 $
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
#include "providerimpl.hxx"
#include "options.hxx"
#include "apifactoryimpl.hxx"
#include "apitreeimplobj.hxx"
#include "apitreeaccess.hxx"
#include "roottree.hxx"
#include "noderef.hxx"
#include "objectregistry.hxx"

#ifndef CONFIGMGR_BOOTSTRAP_HXX_
#include "bootstrap.hxx"
#endif

#ifndef CONFIGMGR_API_PROVIDER_HXX_
#include "provider.hxx"
#endif
#ifndef _CONFIGMGR_TREECACHE_HXX_
#include "treecache.hxx"
#endif
#ifndef _CONFIGMGR_TRACER_HXX_
#include "tracer.hxx"
#endif
#include <osl/interlck.h>

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

static ::rtl::OUString ssUserProfile(RTL_CONSTASCII_USTRINGPARAM("org.openoffice.UserProfile"));
static ::rtl::OUString ssDefaultLocale(RTL_CONSTASCII_USTRINGPARAM("en-US"));
static ::rtl::OUString ssInternational(RTL_CONSTASCII_USTRINGPARAM("International"));
static ::rtl::OUString ssLocale(RTL_CONSTASCII_USTRINGPARAM("Locale"));

namespace configmgr
{
    namespace css   = ::com::sun::star;
    namespace uno   = css::uno;
    namespace beans = css::beans;
    using ::rtl::OUString;

    using configapi::NodeElement;
    using configuration::RootTree;
    using configuration::TemplateProvider;

    namespace configapi
    {

        class ApiProviderInstances
        {
            ObjectRegistryHolder    m_aObjectRegistry;
            ReadOnlyObjectFactory   m_aReaderFactory;
            UpdateObjectFactory     m_aWriterFactory;
            ApiProvider             m_aReaderProvider;
            ApiProvider             m_aWriterProvider;
            TemplateProvider        m_aTemplateProvider;
        public:
            ApiProviderInstances(OProviderImpl& rProviderImpl, ITemplateProvider& rTemplateProvider)
            : m_aObjectRegistry(new ObjectRegistry())
            , m_aReaderFactory(m_aReaderProvider,m_aObjectRegistry)
            , m_aWriterFactory(m_aWriterProvider,m_aObjectRegistry)
            , m_aReaderProvider(m_aReaderFactory,rProviderImpl)
            , m_aWriterProvider(m_aWriterFactory,rProviderImpl)
            , m_aTemplateProvider(rTemplateProvider)
            {
            }

            ~ApiProviderInstances()
            {}

            ApiProvider&    getReaderProvider() { return m_aReaderProvider; }
            ApiProvider&    getWriterProvider() { return m_aWriterProvider; }
            Factory&        getReaderFactory()  { return m_aReaderFactory; }
            Factory&        getWriterFactory()  { return m_aWriterFactory; }
            TemplateProvider& getTemplateProvider() { return m_aTemplateProvider; }

        };
    }

    //=============================================================================
    //= OProviderImpl
    //=============================================================================
    //-----------------------------------------------------------------------------
    OProviderImpl::OProviderImpl(OProvider* _pProvider,
                                 IConfigSession* _pSession,
                                 const uno::Reference< lang::XMultiServiceFactory >& _xServiceFactory,
                                 const ConnectionSettings& _rSettings)
                  :m_pNewProviders(0)
                  ,m_pProvider(_pProvider)
                  ,m_pSession(_pSession)
    {
        uno::Reference< script::XTypeConverter > xConverter(
            _xServiceFactory->createInstance( OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.script.Converter" ))), uno::UNO_QUERY);
        OSL_ENSURE(xConverter.is(), "Module::Module : could not create an instance of the type converter !");

        m_xDefaultOptions = new OOptions(xConverter);

        // this is a hack asking the session if caching should be supported or not
        // at the time we have complete notification support this hack isn't necessary anymore
        if (!_pSession->allowsCachingHack())
            m_xDefaultOptions->setNoCache(sal_True);

        m_pTreeMgr = new TreeManager(_pSession, m_xDefaultOptions); //new OOptions(xConverter));
        m_pTreeMgr->acquire();

        // put out of line to get rid of the order dependency (and to have a acquired configuration)
        m_pNewProviders   = new configapi::ApiProviderInstances(*this,*m_pTreeMgr);

        // read the default locale for the user
        rtl::OUString sDefaultLocale(ssDefaultLocale);
        rtl::OUString sDefaultUser;
        try
        {
            // if we have a user name, we have to add it for the request and we remember it for the session
            if (_rSettings.hasUser())
            {
                // the username is also part of the connection settings
                sDefaultUser = _rSettings.getUser();
            }

            ISubtree* pSubTree = m_pTreeMgr->requestSubtree(ssUserProfile, m_xDefaultOptions);
            if (pSubTree)
            {
                INode* pNode = pSubTree->getChild(ssInternational);
                pSubTree = pNode ? pNode->asISubtree() : NULL;
                if (pSubTree)
                {
                    pNode = pSubTree->getChild(ssLocale);
                    ValueNode* pValueNode = pNode ? pNode->asValueNode() : NULL;
                    if (pValueNode)
                        pValueNode->getValue() >>= sDefaultLocale;
                }

                // should we clean this up ?
                // m_pTreeMgr->releaseSubtree(ssUserProfile, m_xDefaultOptions);
            }
        }
        catch (uno::Exception&)
        {
            // could not read default locale
            // default locale is en-US
        }
        m_xDefaultOptions->setDefaultLocale(sDefaultLocale);
        m_xDefaultOptions->setDefaultUser(sDefaultUser);
    }

    //-----------------------------------------------------------------------------
    OProviderImpl::~OProviderImpl()
    {
        m_pTreeMgr->release();
        m_pTreeMgr = NULL;
        delete m_pNewProviders;
        delete m_pSession;
    }

    // --------------------------------- disposing ---------------------------------
    void SAL_CALL OProviderImpl::dispose() throw()
    {
        m_pTreeMgr->dispose();
        m_pSession->close();
    }

    //-----------------------------------------------------------------------------
    // access to the factory for writable elements
    configapi::Factory& OProviderImpl::getWriterFactory() {return m_pNewProviders->getWriterFactory();}

    //-----------------------------------------------------------------------------
    // access to the raw notifications
    IConfigBroadcaster* OProviderImpl::getNotifier() { OSL_ASSERT(m_pTreeMgr); return m_pTreeMgr->getBroadcaster(); }

    // TemplateProvider access
    //-----------------------------------------------------------------------------

    TemplateProvider  OProviderImpl::getTemplateProvider() const
    {
        return m_pNewProviders->getTemplateProvider();
    }

    // ITreeProvider /ITreeManager
    //-----------------------------------------------------------------------------
    ISubtree* OProviderImpl::requestSubtree( OUString const& aSubtreePath, const vos::ORef < OOptions >& _xOptions,
                                             sal_Int16 nMinLevels) throw (uno::Exception)
    {
        return m_pTreeMgr->requestSubtree(aSubtreePath, _xOptions, nMinLevels);
    }

    //-----------------------------------------------------------------------------
    void OProviderImpl::updateTree(TreeChangeList& aChanges) throw (uno::Exception)
    {
        m_pTreeMgr->updateTree(aChanges);
    }

    //-----------------------------------------------------------------------------
    void OProviderImpl::releaseSubtree( OUString const& aSubtreePath, const vos::ORef < OOptions >& _xOptions ) throw ()
    {
        m_pTreeMgr->releaseSubtree(aSubtreePath, _xOptions);
    }

    //-----------------------------------------------------------------------------
    void OProviderImpl::disposeData(const vos::ORef < OOptions >& _xOptions) throw ()
    {
        m_pTreeMgr->disposeData(_xOptions);
    }

    //-----------------------------------------------------------------------------
    void OProviderImpl::notifyUpdate(TreeChangeList const& aChanges) throw (uno::RuntimeException)
    {
        m_pTreeMgr->notifyUpdate(aChanges);
    }

    //-----------------------------------------------------------------------------
    void OProviderImpl::fetchSubtree(OUString const& aSubtreePath, const vos::ORef < OOptions >& _xOptions, sal_Int16 nMinLevels) throw()
    {
        m_pTreeMgr->fetchSubtree(aSubtreePath, _xOptions, nMinLevels);
    }

    // IInterface
    //-----------------------------------------------------------------------------
    void SAL_CALL OProviderImpl::acquire(  ) throw ()
    {
        m_pProvider->acquire();
    }

    //-----------------------------------------------------------------------------
    void SAL_CALL OProviderImpl::release(  ) throw ()
    {
        m_pProvider->release();
    }

    //-----------------------------------------------------------------------------
    uno::XInterface* OProviderImpl::getProviderInstance()
    {
        return static_cast<com::sun::star::lang::XMultiServiceFactory*>(m_pProvider);
    }

    //-----------------------------------------------------------------------------
    ISynchronizedData& OProviderImpl::getTreeLock()
    {
        return *m_pTreeMgr;
    }

    //-----------------------------------------------------------------------------
    ISynchronizedData const& OProviderImpl::getTreeLock() const
    {
        return *m_pTreeMgr;
    }

    // ISyncronizedData
    //-----------------------------------------------------------------------------
    void OProviderImpl::acquireReadAccess() const
    {
        getTreeLock().acquireReadAccess();
    }

    //-----------------------------------------------------------------------------
    void OProviderImpl::releaseReadAccess() const
    {
        getTreeLock().releaseReadAccess();
    }

    //-----------------------------------------------------------------------------
    void OProviderImpl::acquireWriteAccess()
    {
        getTreeLock().acquireWriteAccess();
    }

    //-----------------------------------------------------------------------------
    void OProviderImpl::releaseWriteAccess()
    {
        getTreeLock().releaseWriteAccess();
    }

    //-----------------------------------------------------------------------------------
    OUString OProviderImpl::getBasePath(OUString const& _rAccessor)
    {
        sal_Int32 nNameStart = _rAccessor.lastIndexOf(ConfigurationName::delimiter);
        if (nNameStart == (_rAccessor.getLength() - 1))
            nNameStart = _rAccessor.lastIndexOf(ConfigurationName::delimiter, nNameStart);
        return nNameStart > 0 ? _rAccessor.copy(0,nNameStart) : ConfigurationName::rootname();
    }

    //-----------------------------------------------------------------------------------
    OUString OProviderImpl::getErrorMessage(OUString const& _rAccessor, const vos::ORef < OOptions >& _xOptions)
    {
        CFG_TRACE_ERROR("config provider: the cache manager could not provide the tree (neither from the cache nor from the session)");
        ::rtl::OUString sMessage;
        ::rtl::OUString sUser(_xOptions->getUser());
        ::rtl::OUString sLocale(_xOptions->getLocale());
        CFG_TRACE_INFO_NI("config provider: the user we tried this for is \"%s\", the locale \"%s\", the path \"%s\"", OUSTRING2ASCII(sUser), OUSTRING2ASCII(sLocale), OUSTRING2ASCII(_rAccessor));
        sMessage += _rAccessor;

        if (sUser.getLength())
        {
            sMessage += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" (and for user "));
            sMessage += sUser;
            sMessage += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(")"));
        }

        if (sLocale.getLength())
        {
            sMessage += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" (and for locale "));
            sMessage += sLocale;
            sMessage += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(")"));
        }

        sMessage += ::rtl::OUString::createFromAscii(" could not be created. Unable to retrieve the node from the configuration server.");
        return sMessage;
    }

    // actual factory methods
    //-----------------------------------------------------------------------------------
    NodeElement* OProviderImpl::buildReadAccess(OUString const& _rAccessor, const vos::ORef < OOptions >& _xOptions, sal_Int32 nMinLevels)  throw (uno::Exception, uno::RuntimeException)
    {
        CFG_TRACE_INFO("config provider: requesting the tree from the cache manager");

        ISubtree*   pTree = NULL;
        ::rtl::OUString sErrorMessage;
        try
        {
            OSL_ASSERT(sal_Int16(nMinLevels) == nMinLevels);
            pTree = requestSubtree(_rAccessor,_xOptions, sal_Int16(nMinLevels));
        }
        catch(uno::Exception&e)
        {
            sErrorMessage = e.Message;
        }

        if (!pTree)
        {
            ::rtl::OUString sMessage = getErrorMessage(_rAccessor, _xOptions);

            // append the error message given by the tree provider
            if (sErrorMessage.getLength())
            {
                sMessage += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\n\nThe server returned the following error:\n"));
                sMessage += sErrorMessage;
            }
            throw uno::Exception(sMessage, getProviderInstance());
        }
        using namespace configuration;
        TreeDepth nDepth = (nMinLevels == ALL_LEVELS) ? C_TreeDepthAll : TreeDepth(nMinLevels);

        RootTree aRootTree( createReadOnlyTree(
                AbsolutePath(getBasePath(_rAccessor), Path::NoValidate()),
                *pTree, nDepth, getTemplateProvider()
            ));

        return m_pNewProviders->getReaderFactory().makeAccessRoot(aRootTree, _xOptions);
    }


    //-----------------------------------------------------------------------------------
    NodeElement* OProviderImpl::buildUpdateAccess(OUString const& _rAccessor, const vos::ORef < OOptions >& _xOptions,
                                                  sal_Int32 nMinLevels) throw (uno::Exception, uno::RuntimeException)
    {
        CFG_TRACE_INFO("config provider: requesting the tree from the cache manager");

        ISubtree*   pTree = NULL;
        ::rtl::OUString sErrorMessage;
        try
        {
            OSL_ASSERT(sal_Int16(nMinLevels) == nMinLevels);
            pTree = requestSubtree(_rAccessor, _xOptions, sal_Int16(nMinLevels));
        }
        catch(uno::Exception &e)
        {
            sErrorMessage = e.Message;
        }

        if (!pTree)
        {
            ::rtl::OUString sMessage = getErrorMessage(_rAccessor, _xOptions);

            // append the error message given by the tree provider
            if (sErrorMessage.getLength())
            {
                sMessage += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\n\nThe server returned the following error:\n"));
                sMessage += sErrorMessage;
            }
            throw uno::Exception(sMessage, getProviderInstance());
        }
        //ConfigurationName aPathToUpdateRoot(getBasePath(_rAccessor), ConfigurationName::Absolute());

        using namespace configuration;
        TreeDepth nDepth = (nMinLevels == ALL_LEVELS) ? C_TreeDepthAll : TreeDepth(nMinLevels);

        RootTree aRootTree( createUpdatableTree(
                                AbsolutePath(getBasePath(_rAccessor),Path::NoValidate()),
                                *pTree, nDepth, getTemplateProvider()
                            ));


        return m_pNewProviders->getWriterFactory().makeAccessRoot(aRootTree, _xOptions);
    }

    //=============================================================================
    //= OProvider::FactoryArguments
    //=============================================================================
    rtl::OUString OProviderImpl::FactoryArguments::sUser(ASCII("user"));
    rtl::OUString OProviderImpl::FactoryArguments::sNodePath(ASCII("nodepath"));
    rtl::OUString OProviderImpl::FactoryArguments::sDepth(ASCII("depth"));
    rtl::OUString OProviderImpl::FactoryArguments::sLocale(ASCII("locale"));
    rtl::OUString OProviderImpl::FactoryArguments::sNoCache(ASCII("nocache"));
    rtl::OUString OProviderImpl::FactoryArguments::sLazyWrite(ASCII("lazywrite"));

#ifdef DBG_UTIL
    //-----------------------------------------------------------------------------
    bool lookup(const rtl::OUString& rName)
    {
        // allowed arguments
        static HashSet aArgs;
        if (aArgs.empty())
        {
            aArgs.insert(OProviderImpl::FactoryArguments::sUser);
            aArgs.insert(OProviderImpl::FactoryArguments::sNodePath);
            aArgs.insert(OProviderImpl::FactoryArguments::sDepth);
            aArgs.insert(OProviderImpl::FactoryArguments::sLocale);
            aArgs.insert(OProviderImpl::FactoryArguments::sNoCache);
            aArgs.insert(OProviderImpl::FactoryArguments::sLazyWrite);
        }

        HashSet::const_iterator it = aArgs.find(rName);
        return it != aArgs.end() ? true : false;
    }

    //-----------------------------------------------------------------------------
    bool checkArgs(const uno::Sequence<uno::Any>& _rArgs) throw (lang::IllegalArgumentException)
    {
        // PRE: a Sequence with some possible arguments
        beans::PropertyValue aCurrent;
        const uno::Any* pCurrent = _rArgs.getConstArray();

        bool bParamOk = false;
        for (sal_Int32 i=0; i<_rArgs.getLength(); ++i, ++pCurrent)
        {
            if (*pCurrent >>= aCurrent)
            {
                if (!lookup(aCurrent.Name))
                {
                    rtl::OString aStr = "The argument '";
                    aStr += rtl::OUStringToOString(aCurrent.Name,RTL_TEXTENCODING_ASCII_US).getStr();
                    aStr += "' could not be extracted.";
                    OSL_ENSURE(false, aStr.getStr());
                }
            }
            else if (i > 0 || pCurrent->getValueTypeClass() != uno::TypeClass_STRING)
            {
                OSL_ENSURE(false, "operator >>= failed.");
            }
            else
            {
                OSL_ENSURE(_rArgs.getLength() <= 2, "Too many arguments for legacy parameters.");
                break;
            }
        }
        return true;
    }
#endif

    //-----------------------------------------------------------------------------------
    void OProviderImpl::FactoryArguments::extractArgs(const uno::Sequence<uno::Any>& _rArgs,
                                                      OUString& /* [out] */ _rNodeAccessor,
                                                      OUString& /* [out] */ _rUser,
                                                      OUString& /* [out] */ _rLocale,
                                                      sal_Int32& /* [out] */ _nLevels,
                                                      bool& /* [out] */ _bNoCache,
                                                      bool& /* [out] */ _bLazyWrite)
        throw (lang::IllegalArgumentException)
    {

#ifdef DBG_UTIL
        checkArgs(_rArgs);
#endif
        ::rtl::OUString sUser, sPath, sLocale;
        sal_Int32 nLevelDepth = ITreeProvider::ALL_LEVELS;
        sal_Bool bNoCache = sal_False;
        sal_Bool bLazyWrite = sal_False;

        // the args have to be a sequence of property values, currently three property names are recognized
        beans::PropertyValue aCurrent;
        sal_Bool bAnyPropValue = sal_False;
        const uno::Any* pCurrent = _rArgs.getConstArray();
        for (sal_Int32 i=0; i<_rArgs.getLength(); ++i, ++pCurrent)
        {
            if (*pCurrent >>= aCurrent)
            {
                sal_Bool bExtractSuccess = sal_True;    // defaulted to TRUE, so we skip unknown arguments
                if (aCurrent.Name.equalsIgnoreCase(OProviderImpl::FactoryArguments::sNodePath))
                    bExtractSuccess = (aCurrent.Value >>= sPath);
                else if (aCurrent.Name.equalsIgnoreCase(OProviderImpl::FactoryArguments::sUser))
                    bExtractSuccess = (aCurrent.Value >>= sUser);
                else if (aCurrent.Name.equalsIgnoreCase(OProviderImpl::FactoryArguments::sDepth))
                    bExtractSuccess = (aCurrent.Value >>= nLevelDepth);
                else if (aCurrent.Name.equalsIgnoreCase(OProviderImpl::FactoryArguments::sLocale))
                    bExtractSuccess = (aCurrent.Value >>= sLocale);
                else if (aCurrent.Name.equalsIgnoreCase(OProviderImpl::FactoryArguments::sNoCache))
                    bExtractSuccess = (aCurrent.Value >>= bNoCache);
                else if (aCurrent.Name.equalsIgnoreCase(OProviderImpl::FactoryArguments::sLazyWrite))
                    bExtractSuccess = (aCurrent.Value >>= bLazyWrite);
/*
#ifdef DBG_UTIL
                else
                {
                    ::rtl::OString sMessage(RTL_CONSTASCII_STRINGPARAM("OProviderImpl::extractArgs : unknown argument name: "));
                    sMessage += ::rtl::OString(aCurrent.Name.getStr(), aCurrent.Name.getLength(), RTL_TEXTENCODING_ASCII_US);
                    sMessage += ::rtl::OString(RTL_CONSTASCII_STRINGPARAM("!"));
                    OSL_ENSURE(sal_False, sMessage.getStr());
                }
#endif
*/

                if (!bExtractSuccess)
                    throw   lang::IllegalArgumentException(
                                    (OUString(RTL_CONSTASCII_USTRINGPARAM("The argument ")) += aCurrent.Name) += OUString(RTL_CONSTASCII_USTRINGPARAM(" could not be extracted.")),
                                    uno::Reference<uno::XInterface>(),
                                    sal_Int16(i)
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

        _rNodeAccessor = sPath; //IConfigSession::composeNodeAccessor(sPath, sUser);
        _nLevels = nLevelDepth;
        _rLocale = sLocale;
        _rUser = sUser;
        _bNoCache = (bNoCache != sal_False);
        _bLazyWrite = bLazyWrite;
    }
// class OOptions
    //..........................................................................
    static sal_Int32 getNextCacheID()
    {
        static oslInterlockedCount nNextID = 0;

        oslInterlockedCount nNewID = osl_incrementInterlockedCount(&nNextID);

        if (nNewID == 0)
        {
            CFG_TRACE_WARNING("Cache ID overflow - restarting sequence !");
            OSL_ENSURE(false, "Cache ID overflow - restarting sequence !");
        }

        return static_cast<sal_Int32>(nNewID);
    }

    void OOptions::setNoCache(bool _bNoCache)
    {
        if (_bNoCache)
        {
            m_nCacheID = getNextCacheID();
        }
        else
        {
            m_nCacheID = 0;
        }
    }

} // namespace configmgr


