/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ctloptions.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:15:17 $
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
#ifndef _SVTOOLS_CTLOPTIONS_HXX
#define _SVTOOLS_CTLOPTIONS_HXX

#ifndef INCLUDED_SVTDLLAPI_H
#include "svtools/svtdllapi.h"
#endif

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _SFXBRDCST_HXX
#include <svtools/brdcst.hxx>
#endif
#ifndef _SFXLSTNER_HXX
#include <svtools/lstner.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_OPTIONS_HXX
#include <svtools/options.hxx>
#endif

class SvtCTLOptions_Impl;

// class SvtCTLOptions --------------------------------------------------------

class SVT_DLLPUBLIC SvtCTLOptions:
    public svt::detail::Options, public SfxBroadcaster, public SfxListener
{
private:
    SvtCTLOptions_Impl*    m_pImp;

public:

    // bDontLoad is for referencing purposes only
    SvtCTLOptions( sal_Bool bDontLoad = sal_False );
    virtual ~SvtCTLOptions();

    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

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
        NUMERALS_SYSTEM
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

