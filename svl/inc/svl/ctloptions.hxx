/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef _SVTOOLS_CTLOPTIONS_HXX
#define _SVTOOLS_CTLOPTIONS_HXX

#include "svl/svldllapi.h"
#include <sal/types.h>
#include <svl/brdcst.hxx>
#include <svl/lstner.hxx>
#include <unotools/options.hxx>

class SvtCTLOptions_Impl;

// class SvtCTLOptions --------------------------------------------------------

class SVL_DLLPUBLIC SvtCTLOptions : public utl::detail::Options
{
private:
    SvtCTLOptions_Impl*    m_pImp;

public:

    // bDontLoad is for referencing purposes only
    SvtCTLOptions( sal_Bool bDontLoad = sal_False );
    virtual ~SvtCTLOptions();

    void            SetCTLFontEnabled( sal_Bool _bEnabled );
    sal_Bool        IsCTLFontEnabled() const;

    void            SetCTLSequenceChecking( sal_Bool _bEnabled );
    sal_Bool        IsCTLSequenceChecking() const;

    void            SetCTLSequenceCheckingRestricted( sal_Bool _bEnable );
    sal_Bool        IsCTLSequenceCheckingRestricted( void ) const;

    void            SetCTLSequenceCheckingTypeAndReplace( sal_Bool _bEnable );
    sal_Bool        IsCTLSequenceCheckingTypeAndReplace() const;

    enum CursorMovement
    {
        MOVEMENT_LOGICAL = 0,
        MOVEMENT_VISUAL
    };
    void            SetCTLCursorMovement( CursorMovement _eMovement );
    CursorMovement  GetCTLCursorMovement() const;

    enum TextNumerals
    {
        NUMERALS_ARABIC = 0,
        NUMERALS_HINDI,
        NUMERALS_SYSTEM,
        NUMERALS_CONTEXT
    };
    void            SetCTLTextNumerals( TextNumerals _eNumerals );
    TextNumerals    GetCTLTextNumerals() const;

    enum EOption
    {
        E_CTLFONT,
        E_CTLSEQUENCECHECKING,
        E_CTLCURSORMOVEMENT,
        E_CTLTEXTNUMERALS,
        E_CTLSEQUENCECHECKINGRESTRICTED,
        E_CTLSEQUENCECHECKINGTYPEANDREPLACE
    };
    sal_Bool IsReadOnly(EOption eOption) const;
};

#endif // _SVTOOLS_CTLOPTIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
