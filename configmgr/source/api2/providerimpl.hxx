/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: providerimpl.hxx,v $
 * $Revision: 1.20 $
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

#include "treeprovider.hxx"
#include "defaultprovider.hxx"
#include "commontypes.hxx" // IInterface
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
    using ::rtl::OUString;

    class ISubtree;
    class ITemplateManager;
    class IConfigDefaultProvider;
    class TreeManager;
    class ContextReader;
    class OProvider;

    struct IConfigBroadcaster;

    namespace configapi
    {
        class NodeElement;
        class ApiProviderInstances;
        class Factory;
    }

    // -----------------------------------------------------------------------------
    class OProviderImpl : public ITreeManager, public IDefaultableTreeManager, public IInterface
    {
        friend class OProvider;
        friend class OConfigurationProvider;
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

            static OUString getArgumentName(Argument _which)                CFG_NOTHROW();
            static Argument lookupArgument(OUString const& sArgumentName)   CFG_NOTHROW();

            static OUString getUserArgumentNameD()              CFG_NOTHROW() { return getArgumentName(ARG_USER_DEPRECATED);     }
            static OUString getEntityArgumentName()             CFG_NOTHROW() { return getArgumentName(ARG_ENTITY);     }
            static OUString getNodePathArgumentName()           CFG_NOTHROW() { return getArgumentName(ARG_NODEPATH); }
            static OUString getDepthArgumentNameArgumentName()  CFG_NOTHROW() { return getArgumentName(ARG_DEPTH);    }
            static OUString getLocaleArgumentName()             CFG_NOTHROW() { return getArgumentName(ARG_LOCALE);   }
            static OUString getNoCacheArgumentNameD()           CFG_NOTHROW() { return getArgumentName(ARG_NOCACHE_OBSOLETE);  }
            static OUString getAsyncArgumentName()              CFG_NOTHROW() { return getArgumentName(ARG_ASYNC);    }
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
                                        OUString&   /* [out] */ _rNodeAccessor,
                                        sal_Int32&  /* [out] */ _nLevels,
                                        RequestOptions& /* [in/out] */ xOptions
                                   ) CFG_THROW1(lang::IllegalArgumentException);

            static bool extractOneArgument( OUString const& aName, uno::Any const& aValue,
                                            OUString&   /* [out] */ _rNodeAccessor,
                                            sal_Int32&  /* [out] */ _nLevels,
                                            RequestOptions& /* [in/out] */ xOptions
                                          ) CFG_NOTHROW();

        };

    public:
        typedef uno::Reference< uno::XComponentContext >        CreationContext;
        typedef uno::Reference< script::XTypeConverter >        TypeConverterRef;
    private:
        OProvider*                          m_pProvider;        /// used for ref counting, uno representation

        TypeConverterRef                    m_xTypeConverter;
        RequestOptions                      m_aDefaultOptions;
        configapi::ApiProviderInstances*    m_pNewProviders;
        mutable osl::Mutex                  m_aTreeManagerMutex;
        TreeManager*                        m_pTreeManager;     /// the tree cache. Will hold a reference to us as long as it life

        rtl::Reference< TreeManager > maybeGetTreeManager() const CFG_NOTHROW();
        rtl::Reference< TreeManager > getTreeManager() const CFG_UNO_THROW_RTE();
        void setTreeManager(TreeManager * pTreeManager) CFG_UNO_THROW_RTE();
        void clearTreeManager() CFG_NOTHROW();
    public:
        OProviderImpl(OProvider* _pProvider, CreationContext const & _xContext);


        virtual ~OProviderImpl();

        /// ITreeManager
        virtual data::NodeAccess requestSubtree(AbsolutePath const& aSubtreePath, const RequestOptions& _aOptions) CFG_UNO_THROW_ALL(  );
        virtual void updateTree(TreeChangeList& aChanges) CFG_UNO_THROW_ALL(  );

        virtual void releaseSubtree( AbsolutePath const& aSubtreePath, const RequestOptions& _aOptions ) CFG_NOTHROW();
        virtual void saveAndNotifyUpdate(TreeChangeList const& aChanges) CFG_UNO_THROW_ALL(  );
        virtual void disposeData(const RequestOptions& _aOptions) CFG_NOTHROW();
        virtual void fetchSubtree(AbsolutePath const& aSubtreePath, const RequestOptions& _aOptions) CFG_NOTHROW();

        virtual void refreshAll() CFG_UNO_THROW_ALL(  );
        virtual void flushAll() CFG_NOTHROW();
        virtual void enableAsync(const sal_Bool& bEnableAsync) CFG_NOTHROW();

        /// IDefaultableTreeManager
        virtual sal_Bool fetchDefaultData(AbsolutePath const& aSubtreePath, const RequestOptions& _aOptions
                                          ) CFG_UNO_THROW_ALL(  );

        // IInterface
        virtual void SAL_CALL acquire(  ) throw ();
        virtual void SAL_CALL release(  ) throw ();

        // DefaultProvider access
        rtl::Reference< IConfigDefaultProvider >  getDefaultProvider() const CFG_UNO_THROW_RTE( );

        // TemplateManager access
        rtl::Reference< IConfigTemplateManager >  getTemplateProvider() const CFG_UNO_THROW_RTE( );

    protected:
        static OUString getErrorMessage(AbsolutePath const& _rAccessor, const RequestOptions& _aOptions);

        virtual void SAL_CALL dispose() throw();
    public:
        void setDefaultLocale( RequestOptions::Locale const & aLocale );

        RequestOptions const& getDefaultOptions() const {return m_aDefaultOptions;}
        TypeConverterRef getTypeConverter() const {return m_xTypeConverter;}
        configapi::Factory& getWriterFactory();
        IConfigBroadcaster* getNotifier() CFG_NOTHROW();
        uno::XInterface*    getProviderInstance();

        // actual factory methods
        // the returned object (if any) has to be acquired once)
        configapi::NodeElement* buildReadAccess( OUString const& _rAccessor, const RequestOptions& _aOptions, sal_Int32 nMinLevels) CFG_UNO_THROW_ALL(  );
        // the returned object (if any) has to be acquired once)
        configapi::NodeElement* buildUpdateAccess(OUString const& _rAccessor, const RequestOptions& _aOptions, sal_Int32 nMinLevels) CFG_UNO_THROW_ALL(  );

    private:
        bool initSession(const ContextReader& _rSettings);
    private:
        void implInitFromSettings(const ContextReader& _rSettings, bool& rNeedProfile);
        void implInitFromProfile(data::NodeAccess const& aProfile);

        virtual void initFromSettings(const ContextReader& _rSettings, bool& rNeedProfile);
        virtual void initFromProfile(data::NodeAccess const& aProfile);
    };
} // namespace configmgr

#endif // CONFIGMGR_API_PROVIDERIMPL_HXX_


