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
#ifndef _TXTATR_HXX
#define _TXTATR_HXX

#ifndef _TXATBASE_HXX
#include <txatbase.hxx>     // SwTxtAttr/SwTxtAttrEnd
#endif
#ifndef _CALBCK_HXX
#include <calbck.hxx>
#endif
namespace binfilter {

class SwTxtNode;	// fuer SwTxtFld
class SvxFont;
class SwCharSetCol;
class SwCharFmt;
class SvxTwoLinesItem;

// ATT_CHARFMT *********************************************

class SwTxtCharFmt : public SwTxtAttrEnd
{
    SwTxtNode* pMyTxtNd;
    BOOL bPrevNoHyph	: 1;
    BOOL bPrevBlink		: 1;
    BOOL bPrevURL		: 1;
    BOOL bColor         : 1;

public:
    SwTxtCharFmt( const SwFmtCharFmt& rAttr, xub_StrLen nStart, xub_StrLen nEnd );
    ~SwTxtCharFmt( );

    // werden vom SwFmtCharFmt hierher weitergeleitet
    virtual void Modify( SfxPoolItem*, SfxPoolItem* );    // SwClient
    virtual BOOL GetInfo( SfxPoolItem& rInfo ) const;

    // erfrage und setze den TxtNode Pointer
    inline const SwTxtNode& GetTxtNode() const;
    void ChgTxtNode( const SwTxtNode* pNew ) { pMyTxtNd = (SwTxtNode*)pNew; }

};

// ATT_HARDBLANK ******************************


// ATT_XNLCONTAINERITEM ******************************


// ******************************

class SwTxtRuby : public SwTxtAttrEnd, public SwClient
{

public:
    SwTxtRuby( const SwFmtRuby& rAttr, xub_StrLen nStart, xub_StrLen nEnd ): SwTxtAttrEnd( (SfxPoolItem &)rAttr, nStart, nEnd ){DBG_BF_ASSERT(0, "STRIP");} ;//STRIP001 	SwTxtRuby( const SwFmtRuby& rAttr, xub_StrLen nStart, xub_StrLen nEnd );

};

// ******************************


// --------------- Inline Implementierungen ------------------------

inline const SwTxtNode& SwTxtCharFmt::GetTxtNode() const
{
    ASSERT( pMyTxtNd, "SwTxtCharFmt:: wo ist mein TextNode?" );
    return *pMyTxtNd;
}


} //namespace binfilter
#endif
