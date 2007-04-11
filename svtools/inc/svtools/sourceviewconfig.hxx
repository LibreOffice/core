/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sourceviewconfig.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:34:40 $
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
#ifndef _SVTOOLS_SOURCEVIEWCONFIG_HXX
#define _SVTOOLS_SOURCEVIEWCONFIG_HXX

// include ---------------------------------------------------------------

#ifndef INCLUDED_SVLDLLAPI_H
#include "svtools/svldllapi.h"
#endif

#ifndef _SFXBRDCST_HXX
#include <svtools/brdcst.hxx>
#endif
#ifndef _SFXLSTNER_HXX
#include <svtools/lstner.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_OPTIONS_HXX
#include <svtools/options.hxx>
#endif

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

