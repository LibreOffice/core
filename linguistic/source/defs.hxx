/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: defs.hxx,v $
 * $Revision: 1.4 $
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

#ifndef _LINGUISTIC_DEFS_HXX_
#define _LINGUISTIC_DEFS_HXX_

///////////////////////////////////////////////////////////////////////////

#define A2OU(x) ::rtl::OUString::createFromAscii( x )

///////////////////////////////////////////////////////////////////////////

struct SvcFlags
{
    INT16   nLastTriedSvcIndex;     // index in sequence of the last
                                    // service tried to instantiate
                                    // (used for cascading)
    BOOL    bAlreadyWarned  : 1;
    BOOL    bDoWarnAgain    : 1;

    SvcFlags() :
        nLastTriedSvcIndex(-1), bAlreadyWarned(FALSE), bDoWarnAgain(FALSE)
    {
    }
};

///////////////////////////////////////////////////////////////////////////

// virtual base class for the different dispatchers
class LinguDispatcher
{
public:
    enum DspType    { DSP_SPELL, DSP_HYPH, DSP_THES };

    virtual void
        SetServiceList( const ::com::sun::star::lang::Locale &rLocale,
                const ::com::sun::star::uno::Sequence<
                    rtl::OUString > &rSvcImplNames ) = 0;
    virtual ::com::sun::star::uno::Sequence< rtl::OUString >
        GetServiceList( const ::com::sun::star::lang::Locale &rLocale ) const = 0;
    virtual DspType
        GetDspType() const = 0;
};

///////////////////////////////////////////////////////////////////////////

#endif

