/*************************************************************************
 *
 *  $RCSfile: providerimpl.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: jb $ $Date: 2002-06-12 16:28:27 $
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

#ifndef CONFIGMGR_TREEPROVIDER_HXX
#include "treeprovider.hxx"
#endif
#ifndef CONFIGMGR_DEFAULTPROVIDER_HXX
#include "defaultprovider.hxx"
#endif

#ifndef CONFIGMGR_MISC_OPTIONS_HXX_
#include "options.hxx"
#endif

#ifndef _COM_SUN_STAR_SCRIPT_XTYPECONVERTER_HPP_
#include <com/sun/star/script/XTypeConverter.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XEVENTLISTENER_HPP_
#include <com/sun/star/lang/XEventListener.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

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

namespace rtl { class OUString; }

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
    class IDefaultProvider;
    class IConfigSession;
    class TreeManager;
    class ConnectionSettings;

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
        OProvider*                          m_pProvider;        /// used for ref counting, uno representation
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
                ARG_NODEPATH,   // requested node path
                ARG_DEPTH,      // depth of the tree
                ARG_USER,       // name of the user - only for admin
                ARG_LOCALE,     // desired locale
                ARG_NOCACHE,    // cache disabling
                ARG_ASYNC,      // lasy write data

                _arg_count,
                ARG_NOT_FOUND = _arg_count
            };
            static sal_Char const * const asciiArgumentNames[];

            static OUString getArgumentName(Argument _which)                CFG_NOTHROW();
            static Argument lookupArgument(OUString const& sArgumentName)   CFG_NOTHROW();

            static OUString getUserArgumentName()               CFG_NOTHROW() { return getArgumentName(ARG_USER);     }
            static OUString getNodePathArgumentName()           CFG_NOTHROW() { return getArgumentName(ARG_NODEPATH); }
            static OUString getDepthArgumentNameArgumentName()  CFG_NOTHROW() { return getArgumentName(ARG_DEPTH);    }
            static OUString getLocaleArgumentName()             CFG_NOTHROW() { return getArgumentName(ARG_LOCALE);   }
            static OUString getNoCacheArgumentName()            CFG_NOTHROW() { return getArgumentName(ARG_NOCACHE);  }
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
                                        vos::ORef<OOptions> /* [in/out] */ xOptions
                                   ) CFG_THROW1(lang::IllegalArgumentException);

            static bool extractOneArgument( beans::PropertyValue const& aCurrent,
                                            OUString&   /* [out] */ _rNodeAccessor,
                                            sal_Int32&  /* [out] */ _nLevels,
                                            vos::ORef<OOptions> /* [in/out] */ _xOptions
                                          ) CFG_NOTHROW();

        };

    private:
        typedef uno::Reference< lang::XMultiServiceFactory >    CreationContext;
        typedef uno::Reference< script::XTypeConverter >        TypeConverterRef;
        CreationContext                     m_xContext;
        TypeConverterRef                    m_xTypeConverter;
        vos::ORef<OOptions>                 m_xDefaultOptions;
        configapi::ApiProviderInstances*    m_pNewProviders;    /// order depedency - this must be after the TreeManager
        TreeManager*                        m_pTreeMgr;         /// the tree cache. Will hold a reference to us as long as it life
        IConfigSession*                     m_pSession;

    protected:
        IConfigSession*   getSession() const;
    public:
        OProviderImpl(OProvider* _pProvider, CreationContext const & _xContext);


        virtual ~OProviderImpl();

        /// ITreeManager
        virtual memory::Segment* getDataSegment(AbsolutePath const& _rAccessor, const vos::ORef < OOptions >& _xOptions);
        virtual data::NodeAccess requestSubtree(AbsolutePath const& aSubtreePath, const vos::ORef < OOptions >& _xOptions,
                                          sal_Int16 nMinLevels = ALL_LEVELS) CFG_UNO_THROW_ALL(  );
        virtual void updateTree(memory::UpdateAccessor& _aAccessToken, TreeChangeList& aChanges) CFG_UNO_THROW_ALL(  );

        virtual void releaseSubtree( AbsolutePath const& aSubtreePath, const vos::ORef < OOptions >& _xOptions ) CFG_NOTHROW();
        virtual void saveAndNotifyUpdate(memory::Accessor const& _aChangedDataAccessor, TreeChangeList const& aChanges) CFG_UNO_THROW_ALL(  );
        virtual void disposeData(const vos::ORef < OOptions >& _xOptions) CFG_NOTHROW();
        virtual void fetchSubtree(AbsolutePath const& aSubtreePath, const vos::ORef < OOptions >& _xOptions, sal_Int16 nMinLevels = ALL_LEVELS) CFG_NOTHROW();
        /// IDefaultableTreeManager
        virtual sal_Bool fetchDefaultData(  memory::UpdateAccessor& _aAccessToken,
                                            AbsolutePath const& aSubtreePath, const vos::ORef < OOptions >& _xOptions,
                                            sal_Int16 nMinLevels) CFG_UNO_THROW_ALL(  );

        // IInterface
        virtual void SAL_CALL acquire(  ) throw ();
        virtual void SAL_CALL release(  ) throw ();

        // DefaultProvider access
        IDefaultProvider&  getDefaultProvider() const;

        // TemplateManager access
        ITemplateManager&  getTemplateProvider() const;

    protected:
        static OUString getErrorMessage(AbsolutePath const& _rAccessor, const vos::ORef < OOptions >& _xOptions);

        virtual void SAL_CALL dispose() throw();
    public:
        const OOptions&     getDefaultOptions() const {return *m_xDefaultOptions;}
        TypeConverterRef getTypeConverter() const {return m_xTypeConverter;}
        configapi::Factory& getWriterFactory();
        IConfigBroadcaster* getNotifier();
        uno::XInterface*    getProviderInstance();

        // actual factory methods
        // the returned object (if any) has to be acquired once)
        configapi::NodeElement* buildReadAccess( OUString const& _rAccessor, const vos::ORef < OOptions >& _xOptions, sal_Int32 nMinLevels) CFG_UNO_THROW_ALL(  );
        // the returned object (if any) has to be acquired once)
        configapi::NodeElement* buildUpdateAccess(OUString const& _rAccessor, const vos::ORef < OOptions >& _xOptions, sal_Int32 nMinLevels) CFG_UNO_THROW_ALL(  );

    private:
        bool initSession(const ConnectionSettings& _rSettings);
    private:
        void implInitFromSettings(const ConnectionSettings& _rSettings, bool& rNeedProfile);
        void implInitFromProfile(data::NodeAccess const& aProfile);

        virtual void initFromSettings(const ConnectionSettings& _rSettings, bool& rNeedProfile);
        virtual void initFromProfile(data::NodeAccess const& aProfile);
    };
} // namespace configmgr

#endif // CONFIGMGR_API_PROVIDERIMPL_HXX_


