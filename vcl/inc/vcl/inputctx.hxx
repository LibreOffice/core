/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: inputctx.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 17:57:49 $
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

#ifndef _VCL_INPUTCTX_HXX
#define _VCL_INPUTCTX_HXX

#ifndef _SV_SV_H
#include <vcl/sv.h>
#endif

#ifndef _VCL_DLLAPI_H
#include <vcl/dllapi.h>
#endif

#ifndef _SV_FONT_HXX
#include <vcl/font.hxx>
#endif

// ----------------------
// - InputContext-Flags -
// ----------------------

#define INPUTCONTEXT_TEXT               ((ULONG)0x00000001)
#define INPUTCONTEXT_EXTTEXTINPUT       ((ULONG)0x00000002)
#define INPUTCONTEXT_EXTTEXTINPUT_ON    ((ULONG)0x00000004)
#define INPUTCONTEXT_EXTTEXTINPUT_OFF   ((ULONG)0x00000008)

// ----------------
// - InputContext -
// ----------------

class VCL_DLLPUBLIC InputContext
{
private:
    Font            maFont;
    ULONG           mnOptions;

public:
                    InputContext() { mnOptions = 0; }
                    InputContext( const InputContext& rInputContext ) :
                        maFont( rInputContext.maFont )
                    { mnOptions = rInputContext.mnOptions; }
                    InputContext( const Font& rFont, ULONG nOptions = 0 ) :
                        maFont( rFont )
                    { mnOptions = nOptions; }

    void            SetFont( const Font& rFont ) { maFont = rFont; }
    const Font&     GetFont() const { return maFont; }

    void            SetOptions( ULONG nOptions ) { mnOptions = nOptions; }
    ULONG           GetOptions() const { return mnOptions; }

    InputContext&   operator=( const InputContext& rInputContext );
    BOOL            operator==( const InputContext& rInputContext ) const;
    BOOL            operator!=( const InputContext& rInputContext ) const
                        { return !(InputContext::operator==( rInputContext )); }
};

inline InputContext& InputContext::operator=( const InputContext& rInputContext )
{
    maFont      = rInputContext.maFont;
    mnOptions   = rInputContext.mnOptions;
    return *this;
}

inline BOOL InputContext::operator==( const InputContext& rInputContext ) const
{
    return ((mnOptions  == rInputContext.mnOptions) &&
            (maFont     == rInputContext.maFont));
}

#endif // _VCL_INPUTCTX_HXX
