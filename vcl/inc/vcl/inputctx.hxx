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

#ifndef _VCL_INPUTCTX_HXX
#define _VCL_INPUTCTX_HXX

#include <vcl/sv.h>
#include <vcl/dllapi.h>
#include <vcl/font.hxx>

// ----------------------
// - InputContext-Flags -
// ----------------------

#define INPUTCONTEXT_TEXT               ((sal_uLong)0x00000001)
#define INPUTCONTEXT_EXTTEXTINPUT       ((sal_uLong)0x00000002)
#define INPUTCONTEXT_EXTTEXTINPUT_ON    ((sal_uLong)0x00000004)
#define INPUTCONTEXT_EXTTEXTINPUT_OFF   ((sal_uLong)0x00000008)

// ----------------
// - InputContext -
// ----------------

class VCL_DLLPUBLIC InputContext
{
private:
    Font            maFont;
    sal_uLong           mnOptions;

public:
                    InputContext() { mnOptions = 0; }
                    InputContext( const InputContext& rInputContext ) :
                        maFont( rInputContext.maFont )
                    { mnOptions = rInputContext.mnOptions; }
                    InputContext( const Font& rFont, sal_uLong nOptions = 0 ) :
                        maFont( rFont )
                    { mnOptions = nOptions; }

    void            SetFont( const Font& rFont ) { maFont = rFont; }
    const Font&     GetFont() const { return maFont; }

    void            SetOptions( sal_uLong nOptions ) { mnOptions = nOptions; }
    sal_uLong           GetOptions() const { return mnOptions; }

    InputContext&   operator=( const InputContext& rInputContext );
    sal_Bool            operator==( const InputContext& rInputContext ) const;
    sal_Bool            operator!=( const InputContext& rInputContext ) const
                        { return !(InputContext::operator==( rInputContext )); }
};

inline InputContext& InputContext::operator=( const InputContext& rInputContext )
{
    maFont      = rInputContext.maFont;
    mnOptions   = rInputContext.mnOptions;
    return *this;
}

inline sal_Bool InputContext::operator==( const InputContext& rInputContext ) const
{
    return ((mnOptions  == rInputContext.mnOptions) &&
            (maFont     == rInputContext.maFont));
}

#endif // _VCL_INPUTCTX_HXX
