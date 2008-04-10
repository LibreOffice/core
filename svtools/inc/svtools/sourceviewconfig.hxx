/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: sourceviewconfig.hxx,v $
 * $Revision: 1.3 $
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
#ifndef _SVTOOLS_SOURCEVIEWCONFIG_HXX
#define _SVTOOLS_SOURCEVIEWCONFIG_HXX

// include ---------------------------------------------------------------

#include "svtools/svldllapi.h"
#include <svtools/brdcst.hxx>
#include <svtools/lstner.hxx>
#include <osl/mutex.hxx>
#include <rtl/ustring.hxx>
#include <svtools/options.hxx>

/* -----------------------------12.10.00 11:40--------------------------------

 ---------------------------------------------------------------------------*/
namespace svt
{
    class SourceViewConfig_Impl;
    class SVL_DLLPUBLIC SourceViewConfig:
        public svt::detail::Options, public SfxBroadcaster, private SfxListener
    {
        static SourceViewConfig_Impl* m_pImplConfig;
        static sal_Int32              m_nRefCount;

        public:
            SourceViewConfig();
            virtual ~SourceViewConfig();

            const rtl::OUString&    GetFontName() const;
            void                    SetFontName(const rtl::OUString& rName);

            sal_Int16               GetFontHeight() const;
            void                    SetFontHeight(sal_Int16 nHeight);

            sal_Bool                IsShowProportionalFontsOnly() const;
            void                    SetShowProportionalFontsOnly(sal_Bool bSet);

            //SfxListener:
            virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
    };
}
#endif

