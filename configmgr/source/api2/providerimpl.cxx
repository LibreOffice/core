/*************************************************************************
 *
 *  $RCSfile: providerimpl.cxx,v $
 *
 *  $Revision: 1.39 $
 *
 *  last change: $Author: dg $ $Date: 2001-09-18 19:27:13 $
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

#ifndef _RTL_LOGFILE_HXX_
#include <rtl/logfile.hxx>
#endif

namespace configmgr
{
    namespace css   = ::com::sun::star;
    namespace uno   = css::uno;
    namespace beans = css::beans;
    using ::rtl::OUString;

    using configapi::NodeElement;
    using configuration::RootTree;
    using configuration::SpecialTemplateProvider;

    namespace configapi
    {

        class ApiProviderInstances
        {
            ObjectRegistryHolder    m_aObjectRegistry;
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
    //= OProviderImpl
    //=============================================================================
    //-----------------------------------------------------------------------------
    OProviderImpl::OProviderImpl(OProvider* _pProvider,
                                 const uno::Reference< lang::XMultiServiceFactory >& _xServiceFactory)
                  :m_pNewProviders(0)
                  ,m_pProvider(_pProvider)
                  ,m_pSession(NULL)
                  ,m_pTreeMgr(NULL)
    {
        uno::Reference< script::XTypeConverter > xConverter(
            _xServiceFactory->createInstance( OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.script.Converter" ))), uno::UNO_QUERY);
        OSL_ENSURE(xConverter.is(), "Module::Module : could not create an instance of the type converter !");

        m_xDefaultOptions = new OOptions(xConverter);
    }
    //-----------------------------------------------------------------------------
    //-----------------------------------------------------------------------------
    void OProviderImpl::initSession(IConfigSession* _pSession,const ConnectionSettings& _rSettings)
    {
        m_pSession = _pSession;

    // prepare the options
        // this is a hack asking the session if caching should be supported or not
        // at the time we have complete notification support this hack isn't necessary anymore
        if (!_pSession->allowsCachingHack())
            m_xDefaultOptions->setNoCache(sal_True);

        bool bNeedProfile = false;
        this->implInitFromSettings(_rSettings,bNeedProfile);

        m_pTreeMgr = new TreeManager(_pSession, m_xDefaultOptions); //new OOptions(xConverter));
        m_pTreeMgr->acquire();

        // put out of line to get rid of the order dependency (and to have a acquired configuration)
        m_pNewProviders   = new configapi::ApiProviderInstances(*this);

    // now complete our state from the user's profile, if necessary
        if (bNeedProfile)
        try
        {
#ifdef TF_CFGDATA
            static ::rtl::OUString ssUserProfile(RTL_CONSTASCII_USTRINGPARAM("org.openoffice.Setup"));
#else
            static ::rtl::OUString ssUserProfile(RTL_CONSTASCII_USTRINGPARAM("org.openoffice.UserProfile"));
#endif
            AbsolutePath aProfileModule = AbsolutePath::makeModulePath(ssUserProfile, AbsolutePath::NoValidate());
            if (ISubtree* pSubTree = m_pTreeMgr->requestSubtree(aProfileModule, m_xDefaultOptions))
                implInitFromProfile(pSubTree);

            // should we clean this up ?
            // m_pTreeMgr->releaseSubtree(ssUserProfile, m_xDefaultOptions);
        }
        catch (uno::Exception&)
        {
            // could not read profile
        }
    }
    //-----------------------------------------------------------------------------

    // these can be overridden. default does nothing
    void OProviderImpl::initFromSettings(const ConnectionSettings& , bool& )
    {
    }
    //-----------------------------------------------------------------------------
    void OProviderImpl::initFromProfile(ISubtree const* )
    {
    }
    //-----------------------------------------------------------------------------
    // these implement the base class behavior
    void OProviderImpl::implInitFromSettings(const ConnectionSettings& _rSettings, bool& rNeedProfile)
    {
        bool bIntrinsicNeedProfile = true;

        // if we have a user name, we have to add and remember it for the session
        if (_rSettings.hasUser())
        {
            // the username is also part of the connection settings
            rtl::OUString sDefaultUser = _rSettings.getUser();
            m_xDefaultOptions->setDefaultUser(sDefaultUser);
        }

        if  (_rSettings.hasLocale())
        {
            bIntrinsicNeedProfile = false;
            rtl::OUString sDefaultLocale = _rSettings.getLocale();
            m_xDefaultOptions->setDefaultLocale(sDefaultLocale);
        }
        else
            OSL_ASSERT(m_xDefaultOptions->getDefaultLocale().getLength() == 0);

        if  (_rSettings.hasAsyncSetting())
        {
            m_xDefaultOptions->setLazyWrite( !!_rSettings.getAsyncSetting() );
        }

    // call the template method
        this->initFromSettings(_rSettings, rNeedProfile);

        if (bIntrinsicNeedProfile)
            rNeedProfile = true; // to get locale
    }
    //-----------------------------------------------------------------------------
    void OProviderImpl::implInitFromProfile(ISubtree const* pProfile)
    {
        OSL_ASSERT(pProfile);

        // read the default locale for the user
        if (m_xDefaultOptions->getDefaultLocale().getLength() == 0)
        {
#ifdef TF_CFGDATA
            static ::rtl::OUString ssSubGroup(RTL_CONSTASCII_USTRINGPARAM("L10N"));
            static ::rtl::OUString ssLocale(RTL_CONSTASCII_USTRINGPARAM("ooLocale"));
#else
            static ::rtl::OUString ssSubGroup(RTL_CONSTASCII_USTRINGPARAM("International"));
            static ::rtl::OUString ssLocale(RTL_CONSTASCII_USTRINGPARAM("Locale"));
#endif

            INode const* pNode = pProfile->getChild(ssSubGroup);
            ISubtree const* pSubTree = pNode ? pNode->asISubtree() : NULL;
            if (pSubTree)
            {
                pNode = pSubTree->getChild(ssLocale);
                ValueNode const * pValueNode = pNode ? pNode->asValueNode() : NULL;
                if (pValueNode)
                {
                    rtl::OUString sDefaultLocale;
                    if (pValueNode->getValue() >>= sDefaultLocale)
                    {
                        m_xDefaultOptions->setDefaultLocale(sDefaultLocale);
                    }
                    else
                        OSL_ENSURE(false, "Could not extract locale parameter into string");
                }
            }
        }

    // call the template method
        this->initFromProfile(pProfile);
    }

    //-----------------------------------------------------------------------------
    OProviderImpl::~OProviderImpl()
    {
        if (m_pTreeMgr)
        {
            m_pTreeMgr->release();
            m_pTreeMgr = NULL;
        }
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

    ITemplateProvider&  OProviderImpl::getTemplateProvider() const
    {
        return *m_pTreeMgr;
    }

    // ITreeProvider /ITreeManager
    //-----------------------------------------------------------------------------
    ISubtree* OProviderImpl::requestSubtree( AbsolutePath const& aSubtreePath, const vos::ORef < OOptions >& _xOptions,
                                             sal_Int16 nMinLevels) throw (uno::Exception)
    {
        ISubtree* pTree = NULL;
        try
        {
            pTree = m_pTreeMgr->requestSubtree(aSubtreePath, _xOptions, nMinLevels);
        }
        catch(uno::Exception&e)
        {
            ::rtl::OUString sMessage = getErrorMessage(aSubtreePath, _xOptions);
            // append the error message given by the tree provider
            sMessage += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\n\nThe backend returned the following error:\n"));
            sMessage += e.Message;

            throw lang::WrappedTargetException(sMessage, getProviderInstance(), uno::makeAny(e));
        }

        if (!pTree)
        {
            ::rtl::OUString sMessage = getErrorMessage(aSubtreePath, _xOptions);

            sMessage += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\n\nNo backend error message available\n"));

            throw uno::Exception(sMessage, getProviderInstance());
        }

        return pTree;
    }

    //-----------------------------------------------------------------------------
    void OProviderImpl::updateTree(TreeChangeList& aChanges) throw (uno::Exception)
    {
        m_pTreeMgr->updateTree(aChanges);
    }

    //-----------------------------------------------------------------------------
    void OProviderImpl::releaseSubtree( AbsolutePath const& aSubtreePath, const vos::ORef < OOptions >& _xOptions ) throw ()
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
    void OProviderImpl::fetchSubtree(AbsolutePath const& aSubtreePath, const vos::ORef < OOptions >& _xOptions, sal_Int16 nMinLevels) throw()
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
    OUString OProviderImpl::getErrorMessage(AbsolutePath const& _rAccessor, const vos::ORef < OOptions >& _xOptions)
    {
        OUString const sAccessor = _rAccessor.toString();

        CFG_TRACE_ERROR("config provider: the cache manager could not provide the tree (neither from the cache nor from the session)");
        ::rtl::OUString sMessage;
        ::rtl::OUString sUser(_xOptions->getUser());
        ::rtl::OUString sLocale(_xOptions->getLocale());
        CFG_TRACE_INFO_NI("config provider: the user we tried this for is \"%s\", the locale \"%s\", the path \"%s\"", OUSTRING2ASCII(sUser), OUSTRING2ASCII(sLocale), OUSTRING2ASCII(sAccessor));
        sMessage += sAccessor;

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

        OSL_ASSERT(sal_Int16(nMinLevels) == nMinLevels);

        try
        {
            using namespace configuration;

            AbsolutePath aAccessorPath = AbsolutePath::parse(_rAccessor);

            ISubtree*   pTree = this->requestSubtree(aAccessorPath,_xOptions, sal_Int16(nMinLevels));

            TreeDepth nDepth = (nMinLevels == ALL_LEVELS) ? C_TreeDepthAll : TreeDepth(nMinLevels);

            RootTree aRootTree( createReadOnlyTree(
                    aAccessorPath,
                    *pTree, nDepth,
                    TemplateProvider( this->getTemplateProvider(), _xOptions )
                ));

            return m_pNewProviders->getReaderFactory().makeAccessRoot(aRootTree, _xOptions);
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
    NodeElement* OProviderImpl::buildUpdateAccess(OUString const& _rAccessor, const vos::ORef < OOptions >& _xOptions,
                                                  sal_Int32 nMinLevels) throw (uno::Exception, uno::RuntimeException)
    {
        CFG_TRACE_INFO("config provider: requesting the tree from the cache manager");
        OSL_ASSERT(sal_Int16(nMinLevels) == nMinLevels);

        try
        {
            using namespace configuration;

            AbsolutePath aAccessorPath = AbsolutePath::parse(_rAccessor);

            ISubtree*   pTree = requestSubtree(aAccessorPath, _xOptions, sal_Int16(nMinLevels));

            TreeDepth nDepth = (nMinLevels == ALL_LEVELS) ? C_TreeDepthAll : TreeDepth(nMinLevels);

            RootTree aRootTree( createUpdatableTree(
                                    aAccessorPath,
                                    *pTree, nDepth,
                                    TemplateProvider( this->getTemplateProvider(), _xOptions )
                                ));


            return m_pNewProviders->getWriterFactory().makeAccessRoot(aRootTree, _xOptions);
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
       "nodepath",  // ARG_NODEPATH,   // requested node path
       "depth",     // ARG_DEPTH,      // depth of the tree
       "user",      // ARG_USER,       // name of the user - only for admin
       "locale",    // ARG_LOCALE,     // desired locale
       "nocache",   // ARG_NOCACHE,    // cache disabling
       "lazywrite"  // ARG_ASYNC,      // lasy write data
    };

    OUString OProviderImpl::FactoryArguments::getArgumentName(Argument _which) SAL_THROW( () )
    {
        OSL_ASSERT(sizeof asciiArgumentNames/sizeof 0[asciiArgumentNames] == _arg_count);
        OSL_PRECOND(_which < _arg_count, "Illegal argument selector in OProviderImpl::FactoryArguments::getArgumentName");

        return OUString::createFromAscii(asciiArgumentNames[_which]);
    }

    OProviderImpl::FactoryArguments::Argument
        OProviderImpl::FactoryArguments::lookupArgument(const rtl::OUString& rName)
            SAL_THROW( () )
    {
        OUString sCheck( rName.toAsciiLowerCase() );

        typedef sal_Char const * const * ArgNameIter;

        ArgNameIter const pFirst = asciiArgumentNames;
        ArgNameIter const pLast  = pFirst + _arg_count;

        ArgNameIter it = pFirst;

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

    bool OProviderImpl::FactoryArguments::extractOneArgument(beans::PropertyValue const& aCurrent,
                            OUString&   /* [out] */ _rNodeAccessor,
                            sal_Int32&  /* [out] */ _nLevels,
                            vos::ORef<OOptions> /* [in/out] */ _xOptions )
        SAL_THROW( () )
    {
        switch ( lookupArgument(aCurrent.Name) )
        {
        case ARG_NODEPATH:
            {
                OUString sStringVal;
                if (aCurrent.Value >>= sStringVal)
                    _rNodeAccessor = sStringVal;
                else
                    return false;
            }
            break;

        case ARG_DEPTH:
            {
                sal_Int32 nIntVal;
                if (aCurrent.Value >>= nIntVal)
                    _nLevels = nIntVal;
                else
                    return false;
            }
            break;

        case ARG_USER:
            {
                OUString sStringVal;
                if (aCurrent.Value >>= sStringVal)
                    _xOptions->setUser(sStringVal);
                else
                    return false;
            }
            break;

        case ARG_LOCALE:
            {
                OUString    sStringVal;
                if (aCurrent.Value >>= sStringVal)
                {
                    _xOptions->setLocale(sStringVal);
                    break;
                }
                #if 0
                lang::Locale aLocale;
                if (aCurrent.Value >>= aLocale)
                {
                    _xOptions->setLocale(aLocale);
                    break;
                }
                #endif

                return false;
            }
            break;

        case ARG_NOCACHE:
            {
                sal_Bool bBoolVal;
                if (aCurrent.Value >>= bBoolVal)
                    _xOptions->setNoCache(!!bBoolVal);
                else
                    return false;
            }
            break;

        case ARG_ASYNC:
            {
                sal_Bool bBoolVal;
                if (aCurrent.Value >>= bBoolVal)
                    _xOptions->setLazyWrite(!!bBoolVal);
                else
                    return false;
            }
            break;

        case ARG_NOT_FOUND:
                {
                    rtl::OString sMessage(RTL_CONSTASCII_STRINGPARAM("Unknown argument \""));
                    sMessage += rtl::OUStringToOString(aCurrent.Name, RTL_TEXTENCODING_ASCII_US);
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
    void failIllegal(sal_Int32 _nArg = -1)
        SAL_THROW( (lang::IllegalArgumentException) )
    {
        OSL_ENSURE( sal_Int16(_nArg) == _nArg, "Argument number out of range. Raising imprecise exception.");
         throw lang::IllegalArgumentException(
                    OUString(RTL_CONSTASCII_USTRINGPARAM("Configuration Provider: Arguments must be PropertyValues.")),
                    uno::Reference<uno::XInterface>(),
                    sal_Int16(_nArg)
                );
   }

     //-----------------------------------------------------------------------------------
    static
    bool extractLegacyArguments(    const uno::Sequence<uno::Any>& _rArgs,
                                    OUString&   /* [out] */ _rNodeAccessor,
                                    sal_Int32&  /* [out] */ _nLevels )
        SAL_THROW( () )
    {
          // compatibility : formerly, you could specify the node path as first arg and the (optional) depth
            // as second arg
            switch (_rArgs.getLength() )
            {
            default:
                return false;

            case 2:
                if (! (_rArgs[1] >>= _nLevels) )
                    return false;

                // fall thru
            case 1:
                if (! (_rArgs[0] >>= _rNodeAccessor) )
                    return false;

                // fall thru
            case 0:
                return true;
            }
    }

    //-----------------------------------------------------------------------------------
    void OProviderImpl::FactoryArguments::extractArgs(  const uno::Sequence<uno::Any>& _rArgs,
                                                        OUString&   /* [out] */ _rNodeAccessor,
                                                        sal_Int32&  /* [out] */ _nLevels,
                                                        vos::ORef<OOptions> /* [in/out] */ _xOptions )
        SAL_THROW( (lang::IllegalArgumentException) )
    {
        _nLevels = ITreeProvider::ALL_LEVELS; // setting a fallback

        // the args have to be a sequence of property values
        bool bLegacyFormat = false;

        for (sal_Int32 i=0; i<_rArgs.getLength(); ++i)
        {
            beans::PropertyValue aCurrent;
            if (_rArgs[i] >>= aCurrent)
            {
                if ( !extractOneArgument(aCurrent,_rNodeAccessor,_nLevels,_xOptions) )
                {
                    OUString sMessage(RTL_CONSTASCII_USTRINGPARAM("The argument "));
                    sMessage += aCurrent.Name;
                    sMessage += OUString(RTL_CONSTASCII_USTRINGPARAM(" has the wrong type.\n- Found type "));
                    sMessage += aCurrent.Value.getValueType().getTypeName();
                    throw   lang::IllegalArgumentException(sMessage,uno::Reference<uno::XInterface>(),sal_Int16(i));
                }
            }
            else
            {
                if (i == 0)// try compatibility format
                    if ( extractLegacyArguments(_rArgs,_rNodeAccessor,_nLevels))
                        break;

                failIllegal(i);
                OSL_ASSERT(false);
            }
        }

        if (_rNodeAccessor.getLength() == 0)
        {
            OUString sMessage(RTL_CONSTASCII_USTRINGPARAM("Configuration Provider: Missing argument: no nodepath was provided"));
            throw   lang::IllegalArgumentException(sMessage,uno::Reference<uno::XInterface>(),-1);
        }
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


