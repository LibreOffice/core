/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: providerimpl.cxx,v $
 * $Revision: 1.69 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_configmgr.hxx"

#include <stdio.h>
#include "providerimpl.hxx"
#include "options.hxx"
#include "apifactoryimpl.hxx"
#include "apitreeimplobj.hxx"
#include "apitreeaccess.hxx"
#include "roottree.hxx"
#include "node.hxx"
#include "noderef.hxx"
#include "objectregistry.hxx"
#include "bootstrap.hxx"
#include "cachefactory.hxx"
#include "provider.hxx"
#include "treemanager.hxx"
#include "tracer.hxx"
#include <osl/interlck.h>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <rtl/ustrbuf.hxx>
#include <rtl/logfile.hxx>

#define RTL_LOGFILE_OU2A(rtlOUString)   (::rtl::OUStringToOString((rtlOUString), RTL_TEXTENCODING_ASCII_US).getStr())

namespace configmgr
{
    namespace css   = ::com::sun::star;
    namespace uno   = css::uno;
    namespace beans = css::beans;

    namespace configapi
    {

        class ApiProviderInstances
        {
            rtl::Reference<ObjectRegistry>  m_aObjectRegistry;
            ReadOnlyObjectFactory   m_aReaderFactory;
            UpdateObjectFactory     m_aWriterFactory;
            ApiProvider             m_aReaderProvider;
            ApiProvider             m_aWriterProvider;
        public:
            ApiProviderInstances(OProviderImpl& rProviderImpl)
            : m_aObjectRegistry(new ObjectRegistry())
            , m_aReaderFactory(m_aReaderProvider,m_aObjectRegistry)
            , m_aWriterFactory(m_aWriterProvider,m_aObjectRegistry)
            , m_aReaderProvider(m_aReaderFactory,rProviderImpl)
            , m_aWriterProvider(m_aWriterFactory,rProviderImpl)
            {
            }

            ~ApiProviderInstances()
            {}

            ApiProvider&        getReaderProvider()     { return m_aReaderProvider; }
            ApiProvider&        getWriterProvider()     { return m_aWriterProvider; }
            Factory&            getReaderFactory()      { return m_aReaderFactory; }
            Factory&            getWriterFactory()      { return m_aWriterFactory; }
        };
    }

    //=============================================================================

    //=============================================================================
    //= OProviderImpl
    //=============================================================================
    //-----------------------------------------------------------------------------
    OProviderImpl::OProviderImpl(OProvider* _pProvider, uno::Reference< uno::XComponentContext > const & _xContext)
                  :m_pProvider(_pProvider)
                  ,m_xTypeConverter()
                  ,m_aDefaultOptions()
                  ,m_pNewProviders(NULL)
                  ,m_aTreeManagerMutex()
                  ,m_pTreeManager(NULL)
    {
        OSL_ENSURE(_xContext.is(), "OProviderImpl : NULL context !");

        uno::Reference< lang::XMultiComponentFactory > xFactory = _xContext->getServiceManager();
        OSL_ENSURE(xFactory.is(), "OProviderImpl : missing service factory !");

        m_xTypeConverter = m_xTypeConverter.query(
            xFactory->createInstanceWithContext( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.script.Converter" )),
                                                 _xContext));

        OSL_ENSURE(m_xTypeConverter.is(), "Module::Module : could not create an instance of the type converter !");
    }
    //-----------------------------------------------------------------------------
    rtl::Reference< TreeManager > OProviderImpl::maybeGetTreeManager() const SAL_THROW(())
    {
        osl::MutexGuard aGuard(m_aTreeManagerMutex);
        rtl::Reference< TreeManager > xResult(m_pTreeManager);
        return xResult;
    }
    //-----------------------------------------------------------------------------
    rtl::Reference< TreeManager > OProviderImpl::getTreeManager() const SAL_THROW((com::sun::star::uno::RuntimeException))
    {
        osl::MutexGuard aGuard(m_aTreeManagerMutex);
        if (m_pTreeManager == NULL)
        {
            rtl::OUString sMsg = rtl::OUString::createFromAscii("OProviderImpl: No cache available - provider was already disposed.");
            throw com::sun::star::lang::DisposedException(sMsg,static_cast<lang::XMultiServiceFactory*>(m_pProvider));
        }
        rtl::Reference< TreeManager > xResult(m_pTreeManager);
        return xResult;
    }
    //-----------------------------------------------------------------------------
    void OProviderImpl::setTreeManager(TreeManager * pTreeManager) SAL_THROW((com::sun::star::uno::RuntimeException))
    {
        osl::MutexGuard aGuard(m_aTreeManagerMutex);

        OSL_PRECOND(m_pTreeManager == NULL, "OProviderImpl: TreeManager is already set");
        OSL_PRECOND(pTreeManager != NULL, "OProviderImpl: Trying to set a NULL TreeManager");

        if (pTreeManager == NULL)
        {
            rtl::OUString sMsg = rtl::OUString::createFromAscii("OProviderImpl: No cache available - cache creation failed.");
            throw com::sun::star::uno::RuntimeException(sMsg,NULL);
        }

        (m_pTreeManager = pTreeManager) -> acquire();
    }
    //-----------------------------------------------------------------------------
    void OProviderImpl::clearTreeManager() SAL_THROW(())
    {
        osl::ClearableMutexGuard aGuard(m_aTreeManagerMutex);
        if (TreeManager * pTM = m_pTreeManager)
        {
            m_pTreeManager = NULL;
            pTM->release();
        }
    }
    //-----------------------------------------------------------------------------
    bool OProviderImpl::initSession(const ContextReader& _rSettings)
    {
        bool bNeedProfile = false;
        rtl::Reference< TreeManager > xNewTreeManager;
        if (_rSettings.isUnoBackend())
        {
            this->implInitFromSettings(_rSettings,bNeedProfile);

            xNewTreeManager = CacheFactory::instance().createCacheManager(_rSettings.getBaseContext());
        }
        else
        {
            throw com::sun::star::uno::RuntimeException(rtl::OUString::createFromAscii("OProviderImpl: Only UNO Backends Supported"),NULL);
        }

        setTreeManager( xNewTreeManager.get() );
        OSL_ASSERT( xNewTreeManager.get() );

        // put out of line to get rid of the order dependency (and to have a acquired configuration)
        m_pNewProviders   = new configapi::ApiProviderInstances(*this);

    // now complete our state from the user's profile, if necessary
        if (bNeedProfile)
        try
        {
            static ::rtl::OUString ssUserProfile(RTL_CONSTASCII_USTRINGPARAM("org.openoffice.Setup"));
            configuration::AbsolutePath aProfileModule = configuration::AbsolutePath::makeModulePath(ssUserProfile);

            sharable::Node * profileTree = xNewTreeManager->requestSubtree(aProfileModule, m_aDefaultOptions);
            if (profileTree != 0)
            {
                implInitFromProfile(profileTree);

                // should we clean this up ?
                // xNewTreeManager->releaseSubtree(aProfileModule, xTempOptions);
            }
        }
        catch (uno::Exception& e)
        {
            // could not read profile
                   (void)e;
            CFG_TRACE_ERROR_NI("Provider bootstrapping: Caught an exception trying to get 'Setup' data: %s", OUSTRING2ASCII(e.Message));
        }

        return true;
    }
    //-----------------------------------------------------------------------------

    // these can be overridden. default does nothing
    void OProviderImpl::initFromSettings(const ContextReader& , bool& )
    {
    }
    //-----------------------------------------------------------------------------
    void OProviderImpl::initFromProfile(sharable::Node *)
    {
    }
    //-----------------------------------------------------------------------------
    // these implement the base class behavior
    void OProviderImpl::implInitFromSettings(const ContextReader& _rSettings, bool& rNeedProfile)
    {
        bool bIntrinsicNeedProfile = true;

        if  (_rSettings.hasLocale())
        {
            bIntrinsicNeedProfile = false;
            rtl::OUString sDefaultLocale = _rSettings.getLocale();
            m_aDefaultOptions.setIsoLocale(sDefaultLocale);
        }
        else if (_rSettings.isAdminService())
        {
            bIntrinsicNeedProfile = false;
            m_aDefaultOptions.setAllLocales();
        }
        else
            OSL_ASSERT(!m_aDefaultOptions.hasLocale());

        if  (_rSettings.hasAsyncSetting())
        {
            m_aDefaultOptions.enableAsync( !!_rSettings.getAsyncSetting() );
        }


    // call the template method
        this->initFromSettings(_rSettings, rNeedProfile);

        if (bIntrinsicNeedProfile)
            rNeedProfile = true; // to get locale
    }
    //-----------------------------------------------------------------------------
    void OProviderImpl::implInitFromProfile(sharable::Node * profile)
    {
        OSL_ASSERT(profile != 0);

        sharable::GroupNode * profileNode = sharable::GroupNode::from(profile);

        OSL_ASSERT(profileNode != 0);

        // read the default locale for the user
        if (!m_aDefaultOptions.hasLocale())
        {
            static rtl::OUString ssSubGroup(RTL_CONSTASCII_USTRINGPARAM("L10N"));
            static rtl::OUString ssLocale(RTL_CONSTASCII_USTRINGPARAM("ooLocale"));

            sharable::GroupNode * l10nNode = sharable::GroupNode::from(profileNode->getChild(ssSubGroup));
            if (l10nNode != 0)
            {
                sharable::ValueNode * value = sharable::ValueNode::from(l10nNode->getChild(ssLocale));

                if (value != 0)
                {
                    rtl::OUString sDefaultLocale;
                    if (value->getValue() >>= sDefaultLocale)
                    {
                        m_aDefaultOptions.setIsoLocale(sDefaultLocale);
                    }
                    else
                        OSL_ENSURE(false, "Could not extract locale parameter into string");
                }
            }
        }

    // call the template method
        this->initFromProfile(profile);

        // last fallback, if there is no locale - even in ooLocale
        m_aDefaultOptions.ensureLocaleSet();
    }

    //-----------------------------------------------------------------------------
    void OProviderImpl::setDefaultLocale( com::sun::star::lang::Locale const & aLocale )
    {
        m_aDefaultOptions.setLocale( aLocale );
        // ensure that the locale is never cleared to 'empty'
        m_aDefaultOptions.ensureLocaleSet();
    }

    //-----------------------------------------------------------------------------
    OProviderImpl::~OProviderImpl()
    {
        UnoApiLock aLock; // hmm...
        clearTreeManager();

        delete m_pNewProviders;
    }

    // --------------------------------- disposing ---------------------------------
    void SAL_CALL OProviderImpl::dispose() throw()
    {
        try
        {
            rtl::Reference< TreeManager > xTM = maybeGetTreeManager();

            if (xTM.is())
                xTM->dispose();

            clearTreeManager();
        }
        catch (uno::Exception& e)
        {
            (void)e;
            CFG_TRACE_ERROR("Disposing the TreeManager or closing the session caused an exception: %s", OUSTRING2ASCII(e.Message));
            clearTreeManager();
        }
    }

    //-----------------------------------------------------------------------------
    // access to the raw notifications
    TreeManager * OProviderImpl::getNotifier() SAL_THROW(())
    {
        rtl::Reference< TreeManager > xTM = maybeGetTreeManager();
        return xTM.get();
    }

    // DefaultProvider access
    //-----------------------------------------------------------------------------
    rtl::Reference< TreeManager > OProviderImpl::getDefaultProvider() const SAL_THROW((com::sun::star::uno::RuntimeException))
    {
        return getTreeManager().get();
    }

    //-----------------------------------------------------------------------------
    sharable::Node * OProviderImpl::requestSubtree( configuration::AbsolutePath const& aSubtreePath,
                                                    RequestOptions const & _aOptions
                                                  ) SAL_THROW((com::sun::star::uno::Exception))
    {
        rtl::Reference< TreeManager > xTreeManager = getTreeManager();

        sharable::Node * tree = 0;
        try
        {
            tree = xTreeManager->requestSubtree(aSubtreePath, _aOptions);
        }
        catch(uno::Exception&e)
        {
            ::rtl::OUString sMessage = getErrorMessage(aSubtreePath, _aOptions);
            // append the error message given by the tree provider
            sMessage += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\n\nThe backend returned the following error:\n"));
            sMessage += e.Message;

            throw lang::WrappedTargetException(sMessage, getProviderInstance(), uno::makeAny(e));
        }

        if (tree == 0)
        {
            ::rtl::OUString sMessage = getErrorMessage(aSubtreePath, _aOptions);

            sMessage += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\n\nNo backend error message available\n"));

            throw uno::Exception(sMessage, getProviderInstance());
        }

        return tree;
    }

    //-----------------------------------------------------------------------------
    void OProviderImpl::updateTree(TreeChangeList& aChanges) SAL_THROW((com::sun::star::uno::Exception))
    {
        getTreeManager()->updateTree(aChanges);
    }

    //-----------------------------------------------------------------------------
    void OProviderImpl::releaseSubtree( configuration::AbsolutePath const& aSubtreePath, RequestOptions const& _aOptions ) SAL_THROW(())
    {
        rtl::Reference< TreeManager > xTM = maybeGetTreeManager();
        if (xTM.is())
            xTM->releaseSubtree(aSubtreePath, _aOptions);
    }

    //-----------------------------------------------------------------------------
    void OProviderImpl::saveAndNotifyUpdate(TreeChangeList const& aChanges) SAL_THROW((com::sun::star::uno::Exception))
    {
        getTreeManager()->saveAndNotifyUpdate(aChanges);
    }

    //-----------------------------------------------------------------------------
    void OProviderImpl::fetchSubtree(configuration::AbsolutePath const& aSubtreePath, RequestOptions const& _aOptions) SAL_THROW(())
    {
        rtl::Reference< TreeManager > xTM = maybeGetTreeManager();
        if (xTM.is())
            xTM->fetchSubtree(aSubtreePath, _aOptions);
    }

    //-----------------------------------------------------------------------------
    sal_Bool OProviderImpl::fetchDefaultData(configuration::AbsolutePath const& aSubtreePath, RequestOptions const& _aOptions) SAL_THROW((com::sun::star::uno::Exception))
    {
        return getTreeManager()->fetchDefaultData(aSubtreePath, _aOptions);
    }
    //-----------------------------------------------------------------------------------
    void OProviderImpl::refreshAll()SAL_THROW((com::sun::star::uno::Exception))
    {
        m_pTreeManager->refreshAll();
    }
    //-----------------------------------------------------------------------------------
    void OProviderImpl::flushAll()SAL_THROW(())
    {
        m_pTreeManager->flushAll();
    }
    //-----------------------------------------------------------------------------------
    void OProviderImpl::enableAsync(const sal_Bool& bEnableAsync) SAL_THROW(())
    {
         m_pTreeManager->enableAsync(bEnableAsync);
    }
    //-----------------------------------------------------------------------------
    uno::XInterface* OProviderImpl::getProviderInstance()
    {
        return static_cast<com::sun::star::lang::XMultiServiceFactory*>(m_pProvider);
    }

    //-----------------------------------------------------------------------------------
    rtl::OUString OProviderImpl::getErrorMessage(configuration::AbsolutePath const& _rAccessor, RequestOptions const& _aOptions)
    {
        rtl::OUString const sAccessor = _rAccessor.toString();

        CFG_TRACE_ERROR("config provider: the cache manager could not provide the tree (neither from the cache nor from the session)");
        ::rtl::OUString sMessage;
        ::rtl::OUString sEntity(_aOptions.getEntity());
        ::rtl::OUString sLocale(_aOptions.getLocale());
        CFG_TRACE_INFO_NI("config provider: the entity we tried this for is \"%s\", the locale \"%s\", the path \"%s\"", OUSTRING2ASCII(sEntity), OUSTRING2ASCII(sLocale), OUSTRING2ASCII(sAccessor));
        sMessage += sAccessor;

        if (sEntity.getLength())
        {
            sMessage += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" (and for entity "));
            sMessage += sEntity;
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
    configapi::NodeElement* OProviderImpl::buildReadAccess(rtl::OUString const& _rAccessor, RequestOptions const& _aOptions, sal_Int32 nMinLevels)  SAL_THROW((com::sun::star::uno::Exception))
    {
        CFG_TRACE_INFO("config provider: requesting the tree from the cache manager");

        OSL_ASSERT(sal_Int16(nMinLevels) == nMinLevels);

        RTL_LOGFILE_CONTEXT_AUTHOR(aLog, "configmgr::OProviderImpl", "jb99855", "configmgr::OProviderImpl::buildReadAccess()");
        RTL_LOGFILE_CONTEXT_TRACE1(aLog, "request path: %s", RTL_LOGFILE_OU2A(_rAccessor) );

        try
        {
            configuration::AbsolutePath aAccessorPath = configuration::AbsolutePath::parse(_rAccessor);

            sharable::Node * tree = requestSubtree(aAccessorPath,_aOptions);

            RTL_LOGFILE_CONTEXT_TRACE(aLog, "data loaded" );

            unsigned int nDepth = (nMinLevels == treeop::ALL_LEVELS) ? configuration::C_TreeDepthAll : (unsigned int)(nMinLevels);

            RTL_LOGFILE_CONTEXT_AUTHOR(aLog2, "configmgr::OProviderImpl", "jb99855", "configmgr: createReadOnlyTree()");

            rtl::Reference< configuration::Tree > aRootTree( configuration::createReadOnlyTree(
                    aAccessorPath, tree, nDepth,
                    configuration::TemplateProvider( getTreeManager(), _aOptions )
                ));

            return m_pNewProviders->getReaderFactory().makeAccessRoot(aRootTree, _aOptions);
        }
        catch (configuration::Exception& e)
        {
            configapi::ExceptionMapper ec(e);
            ec.setContext(this->getProviderInstance());
            //ec.unhandled();
            throw lang::WrappedTargetException(ec.message(), ec.context(), uno::Any());
        }
    }


    //-----------------------------------------------------------------------------------
    configapi::NodeElement* OProviderImpl::buildUpdateAccess(rtl::OUString const& _rAccessor, RequestOptions const& _aOptions,
                                                  sal_Int32 nMinLevels) SAL_THROW((com::sun::star::uno::Exception))
    {
        CFG_TRACE_INFO("config provider: requesting the tree from the cache manager");
        OSL_ASSERT(sal_Int16(nMinLevels) == nMinLevels);

        RTL_LOGFILE_CONTEXT_AUTHOR(aLog, "configmgr::OProviderImpl", "jb99855", "configmgr: buildUpdateAccess()");
        RTL_LOGFILE_CONTEXT_TRACE1(aLog, "request path: %s", RTL_LOGFILE_OU2A(_rAccessor) );

        try
        {
            configuration::AbsolutePath aAccessorPath = configuration::AbsolutePath::parse(_rAccessor);

            sharable::Node * tree = requestSubtree(aAccessorPath, _aOptions);

            RTL_LOGFILE_CONTEXT_TRACE(aLog, "data loaded" );

            unsigned int nDepth = (nMinLevels == treeop::ALL_LEVELS) ? configuration::C_TreeDepthAll : (unsigned int)(nMinLevels);

            RTL_LOGFILE_CONTEXT_AUTHOR(aLog2, "configmgr::OProviderImpl", "jb99855", "createUpdatableTree()");

            rtl::Reference< configuration::Tree > aRootTree( configuration::createUpdatableTree(
                                    aAccessorPath, tree, nDepth,
                                    configuration::TemplateProvider( getTreeManager(), _aOptions )
                                ));


            return m_pNewProviders->getWriterFactory().makeAccessRoot(aRootTree, _aOptions);
        }
        catch (configuration::Exception& e)
        {
            configapi::ExceptionMapper ec(e);
            ec.setContext(this->getProviderInstance());
            //ec.unhandled();
            throw lang::WrappedTargetException(ec.message(), ec.context(), uno::Any());
        }
    }

    //=============================================================================
    //= OProvider::FactoryArguments
    //=============================================================================

    sal_Char const * const  OProviderImpl::FactoryArguments::asciiArgumentNames[] =
    {
       "nodepath",  // ARG_NODEPATH,            // requested node path
       "depth",     // ARG_DEPTH,               // depth of the tree
       "user",      // ARG_USER_DEPRECATED,     // name of the user - only for admin
       "locale",    // ARG_LOCALE,              // desired locale
       "nocache",   // ARG_NOCACHE_OBSOLETE,    // cache disabling
       "lazywrite", // ARG_ASYNC_DEPRECATED,    // lasy write data
       "enableasync",   // ARG_ASYNC,           // lasy write data
       "entity",    // ARG_ENTITY,              // name of the entity to be manipulated - only for admin
       "reload",    //ARG_REFRESH            // refresh component

    };

    OProviderImpl::FactoryArguments::Argument
        OProviderImpl::FactoryArguments::lookupArgument(const rtl::OUString& rName)
            SAL_THROW(())
    {
        rtl::OUString sCheck( rName.toAsciiLowerCase() );

        sal_Char const * const * const pFirst = asciiArgumentNames;
        sal_Char const * const * const pLast  = pFirst + _arg_count;

        sal_Char const * const * it = pFirst;

        for(; it != pLast; ++it)
        {
            if (0 == sCheck.compareToAscii(*it))
            {
                break;
            }
        }

        OSL_ASSERT( Argument(pLast-pFirst) ==  ARG_NOT_FOUND );
        return static_cast<Argument>(it - pFirst);
    }

    //-----------------------------------------------------------------------------------
    bool OProviderImpl::FactoryArguments::extractOneArgument(
                            rtl::OUString const& aName, uno::Any const& aValue,
                            rtl::OUString&  /* [out] */ _rNodeAccessor,
                            sal_Int32&  /* [out] */ _nLevels,
                            RequestOptions& /* [in/out] */ _rOptions )
        SAL_THROW(())
    {
        switch ( lookupArgument(aName) )
        {
        case ARG_NODEPATH:
            {
                rtl::OUString sStringVal;
                if (aValue >>= sStringVal)
                    _rNodeAccessor = sStringVal;
                else
                    return false;
            }
            break;

        case ARG_DEPTH:
            {
                sal_Int32 nIntVal = 0;
                if (aValue >>= nIntVal)
                    _nLevels = nIntVal;
                else
                    return false;
            }
            break;

        case ARG_ENTITY:
        case ARG_USER_DEPRECATED:
            {
                rtl::OUString sStringVal;
                if (aValue >>= sStringVal)
                    _rOptions.setEntity(sStringVal);
                else
                    return false;
            }
            break;

        case ARG_LOCALE:
            {
                rtl::OUString    sStringVal;
                if (aValue >>= sStringVal)
                {
                    _rOptions.setIsoLocale(sStringVal);
                    break;
                }

                lang::Locale aLocale;
                if (aValue >>= aLocale)
                {
                    _rOptions.setLocale(aLocale);
                    break;
                }

                return false;
            }

        case ARG_NOCACHE_OBSOLETE:
            {
                sal_Bool bBoolVal;
                if (aValue >>= bBoolVal)
                    OSL_ENSURE(false,"ConfigurationProvider: Parameter \"nocache\" is obsolete and has no effect");
                else
                    return false;
            }
            break;

        case ARG_ASYNC:
        case ARG_ASYNC_DEPRECATED:
            {
                sal_Bool bBoolVal = sal_False;
                if (aValue >>= bBoolVal)
                    _rOptions.enableAsync(!!bBoolVal);
                else
                    return false;
            }
            break;
         case ARG_REFRESH:
            {
                sal_Bool bBoolVal = sal_False;
                if (aValue >>= bBoolVal)
                    _rOptions.forceRefresh(!!bBoolVal);
                else
                    return false;
            }
            break;

        case ARG_NOT_FOUND:
                {
                    rtl::OString sMessage(RTL_CONSTASCII_STRINGPARAM("Unknown argument \""));
                    sMessage += rtl::OUStringToOString(aName, RTL_TEXTENCODING_ASCII_US);
                    sMessage += rtl::OString(RTL_CONSTASCII_STRINGPARAM("\" !\n- Parameter will be ignored -\n"));
                    CFG_TRACE_WARNING( "provider: %s", sMessage.getStr() );
            #ifdef DBG_UTIL
                    OSL_ENSURE(false, sMessage.getStr());
            #endif
                }
            break;

        default:
            CFG_TRACE_ERROR( "Known argument is not handled" );
            OSL_ENSURE(false, "Known argument is not handled");
            break;
        }
        return true;
    }

     //-----------------------------------------------------------------------------------
    static
    void failInvalidArg(uno::Any const & aArg, sal_Int32 _nArg = -1)
        SAL_THROW((lang::IllegalArgumentException))
    {
        OSL_ENSURE( sal_Int16(_nArg) == _nArg, "Argument number out of range. Raising imprecise exception.");

        rtl::OUStringBuffer sMessage;
        sMessage.appendAscii("Configuration Provider: An argument");
        sMessage.appendAscii(" has the wrong type.");
        sMessage.appendAscii("\n- Expected a NamedValue or PropertyValue");
        sMessage.appendAscii("\n- Found type ").append( aArg.getValueType().getTypeName() );

        throw lang::IllegalArgumentException(   sMessage.makeStringAndClear(),
                                                uno::Reference<uno::XInterface>(),
                                                sal_Int16(_nArg+1));
   }

     //-----------------------------------------------------------------------------------
    static
    void failInvalidArgValue(rtl::OUString const & aName, uno::Any const & aValue, sal_Int32 _nArg = -1)
        SAL_THROW((lang::IllegalArgumentException))
    {
        OSL_ENSURE( sal_Int16(_nArg) == _nArg, "Argument number out of range. Raising imprecise exception.");

        rtl::OUStringBuffer sMessage;
        sMessage.appendAscii("Configuration Provider: The argument ").append(aName);
        sMessage.appendAscii(" has the wrong type.");
        sMessage.appendAscii("\n- Found type ").append( aValue.getValueType().getTypeName() );

        throw lang::IllegalArgumentException(   sMessage.makeStringAndClear(),
                                                uno::Reference<uno::XInterface>(),
                                                sal_Int16(_nArg+1));
   }

     //-----------------------------------------------------------------------------------
    static
    bool extractLegacyArguments(    const uno::Sequence<uno::Any>& _rArgs,
                                    rtl::OUString&  /* [out] */ _rNodeAccessor,
                                    sal_Int32&  /* [out] */ _nLevels )
        SAL_THROW((lang::IllegalArgumentException))
    {
        OSL_ASSERT( _rArgs.getLength() != 0 );

        // compatibility : formerly, you could specify the node path as first arg and the (optional) depth
        // as second arg
        if (! (_rArgs[0] >>= _rNodeAccessor) )
            return false;

        switch (_rArgs.getLength() )
        {
        case 1:
            // valid single argument
            return true;

        case 2:
            // valid second argument
            if (_rArgs[1] >>= _nLevels)
                return true;

            break;

        default:
            if (_rArgs[1] >>= _nLevels)
            {
                // valid second argument, but too many arguments altogether
                 throw lang::IllegalArgumentException(
                            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                "Configuration Provider: Too many arguments. "
                                "Additional arguments are not supported when passing the node path as string (deprecated convention).")),
                            uno::Reference<uno::XInterface>(),
                            sal_Int16(3)
                );
            }

            break;

        }
        // here we have an invalid second argument

        if (_rArgs[1].getValueTypeClass() != uno::TypeClass_STRUCT)
        {
            // completely invalid second argument
            failInvalidArgValue(rtl::OUString::createFromAscii("<depth>"),_rArgs[1],1);
        }

        // Assume PropertyValue or NamedValue,
        // which should be handled consistently by caller
        return false;
    }

    //-----------------------------------------------------------------------------------
    void OProviderImpl::FactoryArguments::extractArgs(  const uno::Sequence<uno::Any>& _rArgs,
                                                        rtl::OUString&  /* [out] */ _rNodeAccessor,
                                                        sal_Int32&  /* [out] */ _nLevels,
                                                        RequestOptions & /* [in/out] */ _aOptions )
        SAL_THROW((lang::IllegalArgumentException))
    {
        UnoApiLock aLock;

        _nLevels = treeop::ALL_LEVELS; // setting a fallback

        // the args have to be a sequence of property or named values
        beans::NamedValue    aNV;
        beans::PropertyValue aPV;
        for (sal_Int32 i=0; i<_rArgs.getLength(); ++i)
        {
            if (_rArgs[i] >>= aPV)
            {
                if ( !extractOneArgument(aPV.Name,aPV.Value,_rNodeAccessor,_nLevels,_aOptions) )
                    failInvalidArgValue(aPV.Name,aPV.Value,i);
            }
            else if (_rArgs[i] >>= aNV)
            {
                if ( !extractOneArgument(aNV.Name,aNV.Value,_rNodeAccessor,_nLevels,_aOptions) )
                    failInvalidArgValue(aNV.Name,aNV.Value,i);
            }
            else
            {
                if (i == 0)// try compatibility format
                    if ( extractLegacyArguments(_rArgs,_rNodeAccessor,_nLevels))
                        break;

                failInvalidArg(_rArgs[i],i);
                OSL_ASSERT(false);
            }
        }

        if (_rNodeAccessor.getLength() == 0)
        {
            rtl::OUString sMessage(RTL_CONSTASCII_USTRINGPARAM("Configuration Provider: Missing argument: no nodepath was provided"));
            throw   lang::IllegalArgumentException(sMessage,uno::Reference<uno::XInterface>(),0);
        }
    }

    //--------------------------------------------------------------------------
    uno::Reference<uno::XInterface>  OProviderImpl::createReadAccess( uno::Sequence<uno::Any> const& aArgs)
        SAL_THROW((com::sun::star::uno::Exception))
    {
        CFG_TRACE_INFO("config provider: going to create a read access instance");

        // extract the args
        rtl::OUString sPath;
        sal_Int32 nLevels;

        RequestOptions aOptions = getDefaultOptions();

        OProviderImpl::FactoryArguments::extractArgs(aArgs, sPath, nLevels, aOptions);

        CFG_TRACE_INFO_NI("config provider: node accessor extracted from the args is %s", OUSTRING2ASCII(sPath));
        CFG_TRACE_INFO_NI("config provider: level depth extracted from the args is %i", nLevels);

        // create the access object
        uno::Reference< uno::XInterface > xReturn;

        configapi::NodeElement* pElement = buildReadAccess(sPath, aOptions, nLevels);
        if (pElement != 0)
        {
            xReturn = pElement->getUnoInstance();
            if (xReturn.is())
                // acquired once by buildReadAccess
                xReturn->release();
        }

        return xReturn;
    }

    //-----------------------------------------------------------------------------------
    uno::Reference<uno::XInterface> OProviderImpl::createUpdateAccess( uno::Sequence<uno::Any> const& aArgs)
        SAL_THROW((com::sun::star::uno::Exception))
    {
        CFG_TRACE_INFO("config provider: going to create an update access instance");

        // extract the args
        rtl::OUString sPath;
        sal_Int32 nLevels;

        RequestOptions aOptions = getDefaultOptions();

        OProviderImpl::FactoryArguments::extractArgs(aArgs, sPath, nLevels, aOptions);

        CFG_TRACE_INFO_NI("config provider: node accessor extracted from the args is %s", OUSTRING2ASCII(sPath));
        CFG_TRACE_INFO_NI("config provider: level depth extracted from the args is %i", nLevels);

        // create the access object
        uno::Reference< uno::XInterface > xReturn;

        configapi::NodeElement* pElement = buildUpdateAccess(sPath, aOptions, nLevels);
        if (pElement != 0)
        {
            xReturn = pElement->getUnoInstance();
            if (xReturn.is())
                // acquired once by buildReadAccess
                xReturn->release();
        }

        return xReturn;
    }
//-----------------------------------------------------------------------------------


} // namespace configmgr


