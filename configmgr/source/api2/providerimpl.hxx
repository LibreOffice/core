/*************************************************************************
 *
 *  $RCSfile: providerimpl.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jb $ $Date: 2001-04-03 16:31:26 $
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

#ifndef CONFIGMGR_API_PROVIDERIMPL_HXX_
#define CONFIGMGR_API_PROVIDERIMPL_HXX_

#ifndef CONFIGMGR_API_EVENTS_HXX_
#include "confevents.hxx"
#endif

#ifndef _COM_SUN_STAR_SCRIPT_XTYPECONVERTER_HPP_
#include <com/sun/star/script/XTypeConverter.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XEVENTLISTENER_HPP_
#include <com/sun/star/lang/XEventListener.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

#include <com/sun/star/lang/XComponent.hpp>

#include "options.hxx"

namespace com { namespace sun { namespace star { namespace uno {
    class Any;
    class XInterface;
    template <class> class Sequence;
    template <class> class Reference;
}   }   }   }

namespace rtl { class OUString; }

namespace configmgr
{
    namespace css       = ::com::sun::star;
    namespace uno       = css::uno;
    namespace script    = css::script;
    namespace lang      = css::lang;
    using ::rtl::OUString;

    class ISubtree;
    class IConfigSession;
    class TreeManager;
    class ConnectionSettings;

    namespace configapi
    {
        class NodeElement;
        class ApiProviderInstances;
        class Factory;
    }
    namespace configuration
    {
        class TemplateProvider;
    }

    // -----------------------------------------------------------------------------
    class OProviderImpl : public ITreeManager, public IInterface
    {
        friend class OProvider;
        OProvider*                          m_pProvider;        /// used for ref counting, uno representation
    public:
        //==========================================================================
        //= FactoryArguments
        //==========================================================================
        /** Helper class for evaluation of the arguments for creating a service instance.*/
        struct FactoryArguments
        {
            /// possible arguments, given only in small letters.
            static rtl::OUString sUser;         // name of the user
            static rtl::OUString sNodePath;     // requested node path
            static rtl::OUString sDepth;        // depth of the tree
            static rtl::OUString sLocale;       // desired locale
            static rtl::OUString sNoCache;      // cache disabling
            static rtl::OUString sLazyWrite;    // lasy write data

        public:
            static void extractArgs(const uno::Sequence<uno::Any>& _rArgs,
                                    ::rtl::OUString& /* [out] */ _rNodeAccessor,
                                    ::rtl::OUString& /* [out] */ _rUser,
                                    ::rtl::OUString& /* [out] */ _rLocale,
                                    sal_Int32& /* [out] */ _nLevels,
                                    bool& /* [out] */ _bNoCache,
                                    bool& /* [out] */ _bLazyWrite) throw (lang::IllegalArgumentException);

        };

    protected:
        vos::ORef<OOptions>                 m_xDefaultOptions;
        configapi::ApiProviderInstances*    m_pNewProviders;    /// order depedency - this must be after the TreeManager
        TreeManager*                        m_pTreeMgr;         /// the tree cache. Will hold a reference to us as long as it life
        IConfigSession*                     m_pSession;

    public:
        OProviderImpl(OProvider* _pProvider,
                      const uno::Reference< lang::XMultiServiceFactory >& _xServiceFactory);


        virtual ~OProviderImpl();

        /// ITreeProvider
        virtual ISubtree * requestSubtree(OUString const& aSubtreePath, const vos::ORef < OOptions >& _xOptions,
                                          sal_Int16 nMinLevels = ALL_LEVELS) throw (uno::Exception);
        virtual void updateTree(TreeChangeList& aChanges) throw (uno::Exception);

        /// ITreeManager
        virtual void releaseSubtree( OUString const& aSubtreePath, const vos::ORef < OOptions >& _xOptions ) throw ();
        virtual void notifyUpdate(TreeChangeList const& aChanges) throw (uno::RuntimeException);
        virtual void disposeData(const vos::ORef < OOptions >& _xOptions) throw();
        virtual void fetchSubtree(OUString const& aSubtreePath, const vos::ORef < OOptions >& _xOptions, sal_Int16 nMinLevels = ALL_LEVELS) throw();

        // IInterface
        virtual void SAL_CALL acquire(  ) throw ();
        virtual void SAL_CALL release(  ) throw ();

        // TemplateProvider access
        configuration::TemplateProvider  getTemplateProvider() const;

    protected:
        ISynchronizedData      & getTreeLock();
        ISynchronizedData const& getTreeLock() const;

        // ISyncronizedData
        virtual void acquireReadAccess() const;
        virtual void releaseReadAccess() const;
        virtual void acquireWriteAccess();
        virtual void releaseWriteAccess();

        static OUString getBasePath(OUString const& _rAccessor);
        static OUString getErrorMessage(OUString const& _rAccessor, const vos::ORef < OOptions >& _xOptions);

        virtual void SAL_CALL dispose() throw();
    public:
        const OOptions&     getDefaultOptions() const {return *m_xDefaultOptions;}
        uno::Reference< script::XTypeConverter > getTypeConverter() const {return m_xDefaultOptions->getTypeConverter();}
        configapi::Factory& getWriterFactory();
        IConfigBroadcaster* getNotifier();
        uno::XInterface*    getProviderInstance();

        // actual factory methods
        // the returned object (if any) has to be acquired once)
        configapi::NodeElement* buildReadAccess( OUString const& _rAccessor, const vos::ORef < OOptions >& _xOptions, sal_Int32 nMinLevels) throw (uno::Exception, uno::RuntimeException);
        // the returned object (if any) has to be acquired once)
        configapi::NodeElement* buildUpdateAccess(OUString const& _rAccessor, const vos::ORef < OOptions >& _xOptions, sal_Int32 nMinLevels) throw (uno::Exception, uno::RuntimeException);

    private:
        void initSession(IConfigSession* pSession, const ConnectionSettings& _rSettings);
    private:
        void implInitFromSettings(const ConnectionSettings& _rSettings, bool& rNeedProfile);
        void implInitFromProfile(ISubtree const* pProfile);

        virtual void initFromSettings(const ConnectionSettings& _rSettings, bool& rNeedProfile);
        virtual void initFromProfile(ISubtree const* pProfile);
    };
} // namespace configmgr

#endif // CONFIGMGR_API_PROVIDERIMPL_HXX_


