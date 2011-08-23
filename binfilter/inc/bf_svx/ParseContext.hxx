/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#ifndef SVX_QUERYDESIGNCONTEXT_HXX
#define SVX_QUERYDESIGNCONTEXT_HXX

#ifndef CONNECTIVITY_IPARSECONTEXT_HXX
#include <connectivity/IParseContext.hxx>
#endif

#ifndef _UNOTOOLS_LOCALEDATAWRAPPER_HXX 
#include <unotools/localedatawrapper.hxx>
#endif
namespace binfilter {

namespace svxform
{
    //==========================================================================
    //= OSystemParseContext
    //==========================================================================
    class OSystemParseContext : public ::connectivity::IParseContext
    {
    private:
        ByteString m_aSQLInternationals;

    public:
        OSystemParseContext();

        virtual ~OSystemParseContext();
        // retrieves language specific error messages
        virtual ::rtl::OUString getErrorMessage(ErrorCode _eCodes) const;

        // retrieves language specific keyword strings (only ASCII allowed)
        virtual ::rtl::OString getIntlKeywordAscii(InternationalKeyCode _eKey) const;

        // finds out, if we have an international keyword (only ASCII allowed)
        virtual InternationalKeyCode getIntlKeyCode(const ::rtl::OString& rToken) const;

        /** get's a locale instance which should be used when parsing in the context specified by this instance
            <p>if this is not overridden by derived classes, it returns the static default locale.</p>
        */
        virtual ::com::sun::star::lang::Locale getPreferredLocale( ) const;

    public:
        // helper methods to ease access to some of the characteristics of the locale
        sal_Unicode getNumDecimalSep( ) const;
        sal_Unicode getNumThousandSep( ) const;
    };

    //==========================================================================
    //= OParseContextClient
    //==========================================================================
    /** helper class which needs access to a (shared and ref-counted) OSystemParseContext
        instance.
    */
    class OParseContextClient
    {
    protected:
        OParseContextClient();
        virtual ~OParseContextClient();
    };
}
}//end of namespace binfilter
#endif // SVX_QUERYDESIGNCONTEXT_HXX

