/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: swafopt.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:31:19 $
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
#ifndef _SVXSWAFOPT_HXX
#define _SVXSWAFOPT_HXX

#ifndef _VCL_FONT_HXX //autogen
#include <vcl/font.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

class SvStringsISortDtor;

// Klasse fuer Optionen vom Autoformat
struct SVX_DLLPUBLIC SvxSwAutoFmtFlags
{
    Font aBulletFont;
    Font aByInputBulletFont;
    const SvStringsISortDtor* pAutoCmpltList;  // only valid inside the Dialog!!!

    sal_Unicode cBullet;
    sal_Unicode cByInputBullet;

    USHORT nAutoCmpltWordLen, nAutoCmpltListLen;
    USHORT nAutoCmpltExpandKey;

    BYTE nRightMargin;

    BOOL bReplaceQuote : 1;
    BOOL bAutoCorrect : 1;
    BOOL bCptlSttSntnc : 1;
    BOOL bCptlSttWrd : 1;
    BOOL bChkFontAttr : 1;

    BOOL bChgUserColl : 1;
    BOOL bChgEnumNum : 1;

    BOOL bAFmtByInput : 1;
    BOOL bDelEmptyNode : 1;
    BOOL bSetNumRule : 1;

    BOOL bChgFracionSymbol : 1;
    BOOL bChgOrdinalNumber : 1;
    BOOL bChgToEnEmDash : 1;
    BOOL bChgWeightUnderl : 1;
    BOOL bSetINetAttr : 1;

    BOOL bSetBorder : 1;
    BOOL bCreateTable : 1;
    BOOL bReplaceStyles : 1;
    BOOL bDummy : 1;

    BOOL bWithRedlining : 1;

    BOOL bRightMargin : 1;

    BOOL bAutoCompleteWords : 1;
    BOOL bAutoCmpltCollectWords : 1;
    BOOL bAutoCmpltEndless : 1;
// -- under NT hier starts a new long
    BOOL bAutoCmpltAppendBlanc : 1;
    BOOL bAutoCmpltShowAsTip : 1;

    BOOL bAFmtDelSpacesAtSttEnd : 1;
    BOOL bAFmtDelSpacesBetweenLines : 1;
    BOOL bAFmtByInpDelSpacesAtSttEnd : 1;
    BOOL bAFmtByInpDelSpacesBetweenLines : 1;

    BOOL bAutoCmpltKeepList : 1;

    // some dummies for any new options
    BOOL bDummy5 : 1,
         bDummy6 : 1,
         bDummy7 : 1,
         bDummy8 : 1
         ;

    SvxSwAutoFmtFlags();
    SvxSwAutoFmtFlags( const SvxSwAutoFmtFlags& rAFFlags ) { *this = rAFFlags; }
    SvxSwAutoFmtFlags& operator=( const SvxSwAutoFmtFlags& );
};

#endif

