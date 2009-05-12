/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: binarycache.hxx,v $
 * $Revision: 1.6 $
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
#ifndef CONFIGMGR_BINARYCACHE_HXX
#define CONFIGMGR_BINARYCACHE_HXX

#include "matchlocale.hxx"

#include <rtl/ustring.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/configuration/backend/XLayer.hpp>

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
                                    uno::Reference< lang::XMultiServiceFactory > const & aFactory,
                                    rtl::OUString const & aComponent,
                                    rtl::OUString const & aSchemaVersion,
                                    rtl::OUString const & aEntity,
                                    com::sun::star::lang::Locale const & aRequestedLocale,
                                    std::vector< com::sun::star::lang::Locale > & outKnownLocales,
                                    const uno::Reference<backenduno::XLayer> * pLayers,
                                    sal_Int32 nNumLayers,
                                    bool bIncludeTemplates = true);

            bool writeComponentData(MergedComponentData const & aComponentData,
                                    uno::Reference< lang::XMultiServiceFactory > const & aFactory,
                                    rtl::OUString const & aComponent,
                                    rtl::OUString const & aSchemaVersion,
                                    rtl::OUString const & aEntity,
                                    std::vector< com::sun::star::lang::Locale > const & aKnownLocales,
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

