/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: providerimpl.hxx,v $
 * $Revision: 1.21 $
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

#ifndef CONFIGMGR_API_PROVIDERIMPL_HXX_
#define CONFIGMGR_API_PROVIDERIMPL_HXX_

#include "defaultprovider.hxx"
#include "requestoptions.hxx"
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/script/XTypeConverter.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <cppuhelper/implbase1.hxx>
#include <osl/mutex.hxx>

namespace com { namespace sun { namespace star {
    namespace uno
    {
        class Any;
        class XInterface;
        template <class> class Sequence;
        template <class> class Reference;
    }
    namespace beans
    {
        struct PropertyValue;
    }
}   }   }

namespace rtl
{
    class OUString;
    template <class RCType> class Reference;
}

namespace configmgr
{
    namespace css       = ::com::sun::star;
    namespace uno       = css::uno;
    namespace script    = css::script;
    namespace lang      = css::lang;
    namespace beans     = css::beans;

    class ISubtree;
    struct TreeChangeList;
    class TreeManager;
    class ContextReader;
    class OProvider;

    namespace configapi
    {
        class NodeElement;
        class ApiProviderInstances;
        class Factory;
    }
    namespace sharable { union Node; }

    // -----------------------------------------------------------------------------
    class OProviderImpl : public IDefaultableTreeManager
    {
        friend class OProvider;
    public:
        //==========================================================================
        //= FactoryArguments
        //==========================================================================
        /** Helper class for evaluation of the arguments for creating a service instance.*/
        struct FactoryArguments
        {
            /// possible arguments, given only in small letters.
            enum Argument
            {
                ARG_NODEPATH,           // requested node path
                ARG_DEPTH,              // depth of the tree
                ARG_USER_DEPRECATED,    // name of the entity to be manipulated - deprecated version
                ARG_LOCALE,             // desired locale
                ARG_NOCACHE_OBSOLETE,   // cache disabling - obsolete and nonfunctional
                ARG_ASYNC_DEPRECATED,   // lasy write data - deprecated version
                ARG_ASYNC,              // lazy write data
                ARG_ENTITY,             // name of the entity to be manipulated - only for admin
                ARG_REFRESH,            // force refresh of data into cache

                _arg_count,
                ARG_NOT_FOUND = _arg_count
            };
            static sal_Char const * const asciiArgumentNames[];

            static Argument lookupArgument(rtl::OUString const& sArgumentName)   SAL_THROW(());
        public:
            /** extracts arguments from the argument sequence into to the parameter variables

                <p>unknown arguments are ignored</p>

                @throws com::sun::star::lang::IllegalArgumentException
                    if an element of _rArgs had the wrong type or
                    if the value of a known argument has the wrong type or
                    if the value of a known argument is out of range (sometimes)
                    or if no non-empty node path argument could be extracted,
            */
            static void extractArgs(    const uno::Sequence<uno::Any>& _rArgs,
                                        rtl::OUString&   /* [out] */ _rNodeAccessor,
                                        sal_Int32&  /* [out] */ _nLevels,
                                        RequestOptions& /* [in/out] */ xOptions)
                SAL_THROW((lang::IllegalArgumentException));

            static bool extractOneArgument( rtl::OUString const& aName, uno::Any const& aValue,
                                            rtl::OUString&  /* [out] */ _rNodeAccessor,
                                            sal_Int32&  /* [out] */ _nLevels,
                                            RequestOptions& /* [in/out] */ xOptions
                                          ) SAL_THROW(());

        };

    private:
        OProvider*                          m_pProvider;        /// used for ref counting, uno representation

        uno::Reference< script::XTypeConverter >                    m_xTypeConverter;
        RequestOptions                      m_aDefaultOptions;
        configapi::ApiProviderInstances*    m_pNewProviders;
        mutable osl::Mutex                  m_aTreeManagerMutex;
        TreeManager*                        m_pTreeManager;     /// the tree cache. Will hold a reference to us as long as it life

        rtl::Reference< TreeManager > maybeGetTreeManager() const SAL_THROW(());
        rtl::Reference< TreeManager > getTreeManager() const SAL_THROW((com::sun::star::uno::RuntimeException));
        void setTreeManager(TreeManager * pTreeManager) SAL_THROW((com::sun::star::uno::RuntimeException));
        void clearTreeManager() SAL_THROW(());

        OProviderImpl(OProvider* _pProvider,
                                   uno::Reference< uno::XComponentContext > const & xContext);

    public:
        virtual ~OProviderImpl();

       /** request that the tree named by a path is added to the collection of managed trees
            respecting certain options and requiring a specific loading depth.
            Return a reference to that managed tree.
            The reference must later be released by calling releaseSubtree with the same path and options.
        */
        sharable::Node * requestSubtree(configuration::AbsolutePath const& aSubtreePath, const RequestOptions& _aOptions) SAL_THROW((com::sun::star::uno::Exception));

        /// update the managed data according to a changes list - update the changes list accordingly with old values
        void updateTree(TreeChangeList& aChanges) SAL_THROW((com::sun::star::uno::Exception));

        // bookkeeping support
        void releaseSubtree( configuration::AbsolutePath const& aSubtreePath, const RequestOptions& _aOptions ) SAL_THROW(());

        // notification
        void saveAndNotifyUpdate(TreeChangeList const& aChanges) SAL_THROW((com::sun::star::uno::Exception));

        /** request that the tree named by a path is added to the collection of managed trees
            respecting certain options and requiring a specific loading depth.
        */
        void fetchSubtree(configuration::AbsolutePath const& aSubtreePath, const RequestOptions& _aOptions) SAL_THROW(());

        //Refresh all components in the cache
        void refreshAll() SAL_THROW((com::sun::star::uno::Exception));

        //Flush all components in the cache
        void flushAll() SAL_THROW(());

        //Enable/Disable Asynchronous write-back to cache
        void enableAsync(const sal_Bool& bEnableAsync) SAL_THROW(());

        /// IDefaultableTreeManager
        virtual sal_Bool fetchDefaultData(configuration::AbsolutePath const& aSubtreePath, const RequestOptions& _aOptions
                                          ) SAL_THROW((com::sun::star::uno::Exception));

        // DefaultProvider access
        rtl::Reference< TreeManager >  getDefaultProvider() const SAL_THROW((com::sun::star::uno::RuntimeException));

    protected:
        static rtl::OUString getErrorMessage(configuration::AbsolutePath const& _rAccessor, const RequestOptions& _aOptions);

        void SAL_CALL dispose() throw();
    public:
        void setDefaultLocale( com::sun::star::lang::Locale const & aLocale );

        RequestOptions const& getDefaultOptions() const {return m_aDefaultOptions;}
        uno::Reference< script::XTypeConverter > getTypeConverter() const {return m_xTypeConverter;}
        TreeManager * getNotifier() SAL_THROW(());
        uno::XInterface*    getProviderInstance();

        // actual factory methods
        // the returned object (if any) has to be acquired once)
        configapi::NodeElement* buildReadAccess( rtl::OUString const& _rAccessor, const RequestOptions& _aOptions, sal_Int32 nMinLevels) SAL_THROW((com::sun::star::uno::Exception));
        // the returned object (if any) has to be acquired once)
        configapi::NodeElement* buildUpdateAccess(rtl::OUString const& _rAccessor, const RequestOptions& _aOptions, sal_Int32 nMinLevels) SAL_THROW((com::sun::star::uno::Exception));
        // factory methods
        uno::Reference<uno::XInterface>  createReadAccess( uno::Sequence<uno::Any> const& aArgs) SAL_THROW((com::sun::star::uno::Exception));
        uno::Reference<uno::XInterface>  createUpdateAccess( uno::Sequence<uno::Any> const& aArgs) SAL_THROW((com::sun::star::uno::Exception));

    private:
        bool initSession(const ContextReader& _rSettings);
    private:
        void implInitFromSettings(const ContextReader& _rSettings, bool& rNeedProfile);
        void implInitFromProfile(sharable::Node * aProfile);

        void initFromSettings(const ContextReader& _rSettings, bool& rNeedProfile);
        void initFromProfile(sharable::Node * aProfile);
    };
} // namespace configmgr

#endif // CONFIGMGR_API_PROVIDERIMPL_HXX_


