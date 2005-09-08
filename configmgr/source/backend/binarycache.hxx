/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: binarycache.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 03:25:56 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef CONFIGMGR_BINARYCACHE_HXX
#define CONFIGMGR_BINARYCACHE_HXX

#ifndef CONFIGMGR_MATCHLOCALE_HXX
#include "matchlocale.hxx"
#endif

#include <rtl/ustring.hxx>

#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XLAYER_HPP_
#include <com/sun/star/configuration/backend/XLayer.hpp>
#endif // _COM_SUN_STAR_CONFIGURATION_BACKEND_XLAYER_HPP_

namespace configmgr
{
    // -----------------------------------------------------------------------------
    namespace backend
    {
        namespace css = com::sun::star ;
        namespace uno  = css::uno ;
        namespace lang = css::lang ;
        namespace backenduno = css::configuration::backend  ;

        // -----------------------------------------------------------------------------
        class MergedComponentData;

        typedef uno::Reference< lang::XMultiServiceFactory > MultiServiceFactory;
        // -----------------------------------------------------------------------------

        class BinaryCache
        {
        public:
            explicit
            BinaryCache(const uno::Reference<uno::XComponentContext>& xContext);

            void setOwnerEntity(rtl::OUString const & aOwnerEntity);
            void disableCache();

            bool isCacheEnabled(rtl::OUString const & aEntity) const;

            rtl::OUString getCacheFileURL(rtl::OUString const & aComponent) const;

            bool readComponentData(MergedComponentData & aComponentData,
                                    MultiServiceFactory const & aFactory,
                                    rtl::OUString const & aComponent,
                                    rtl::OUString const & aSchemaVersion,
                                    rtl::OUString const & aEntity,
                                    localehelper::Locale const & aRequestedLocale,
                                    localehelper::LocaleSequence & outKnownLocales,
                                    const uno::Reference<backenduno::XLayer> * pLayers,
                                    sal_Int32 nNumLayers,
                                    bool bIncludeTemplates = true);

            bool writeComponentData(MergedComponentData const & aComponentData,
                                    MultiServiceFactory const & aFactory,
                                    rtl::OUString const & aComponent,
                                    rtl::OUString const & aSchemaVersion,
                                    rtl::OUString const & aEntity,
                                    localehelper::LocaleSequence const & aKnownLocales,
                                    const uno::Reference<backenduno::XLayer> * pLayers,
                                    sal_Int32 nNumLayers);
        private:
            rtl::OUString mBaseURL;
            rtl::OUString mOwnerEntity;
            bool mbCacheEnabled;
        };

    // -----------------------------------------------------------------------------------
    // helper for cache reader implementations - see backendaccess.cxx for implementation
        uno::Sequence< rtl::OUString >
            getAvailableLocales(const uno::Reference<backenduno::XLayer> * pLayers, sal_Int32 nNumLayers);
    // -----------------------------------------------------------------------------------
    }
// -----------------------------------------------------------------------------------
}

#endif //

