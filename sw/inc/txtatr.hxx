/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: txtatr.hxx,v $
 * $Revision: 1.12 $
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
#include <tools/gen.hxx>
#include <tools/color.hxx>
#include <txatbase.hxx>     // SwTxtAttr/SwTxtAttrEnd
#include <calbck.hxx>

class SwTxtNode;    // fuer SwTxtFld
class SwCharFmt;
class SvxTwoLinesItem;

// ATT_CHARFMT *********************************************

class SwTxtCharFmt : public SwTxtAttrEnd
{
    SwTxtNode* pMyTxtNd;
    USHORT mnSortNumber;

public:
    SwTxtCharFmt( const SwFmtCharFmt& rAttr, xub_StrLen nStart, xub_StrLen nEnd );
    ~SwTxtCharFmt( );

    // werden vom SwFmtCharFmt hierher weitergeleitet
    virtual void Modify( SfxPoolItem*, SfxPoolItem* );    // SwClient
    virtual BOOL GetInfo( SfxPoolItem& rInfo ) const;

    // erfrage und setze den TxtNode Pointer
    void ChgTxtNode( const SwTxtNode* pNew ) { pMyTxtNd = (SwTxtNode*)pNew; }

    void SetSortNumber( USHORT nSortNumber ) { mnSortNumber = nSortNumber; }
    USHORT GetSortNumber() const { return mnSortNumber; }
};

// ATT_HARDBLANK ******************************

class SwTxtHardBlank : public SwTxtAttr
{
    sal_Unicode cChar;
public:
    SwTxtHardBlank( const SwFmtHardBlank& rAttr, xub_StrLen nStart );
    inline sal_Unicode GetChar() const  { return cChar; }
};

// ATT_XNLCONTAINERITEM ******************************

class SwTxtXMLAttrContainer : public SwTxtAttrEnd
{
public:
    SwTxtXMLAttrContainer( const SvXMLAttrContainerItem& rAttr,
                        xub_StrLen nStart, xub_StrLen nEnd );
};

// ******************************

class SW_DLLPUBLIC SwTxtRuby : public SwTxtAttrEnd, public SwClient
{
    SwTxtNode* pMyTxtNd;

public:
    SwTxtRuby( const SwFmtRuby& rAttr, xub_StrLen nStart, xub_StrLen nEnd );
    virtual ~SwTxtRuby();
    TYPEINFO();

    virtual void Modify( SfxPoolItem *pOld, SfxPoolItem *pNew);
    virtual BOOL GetInfo( SfxPoolItem& rInfo ) const;

    // erfrage und setze den TxtNode Pointer
    const SwTxtNode* GetpTxtNode() const            { return pMyTxtNd; }
    inline const SwTxtNode& GetTxtNode() const;
    void ChgTxtNode( const SwTxtNode* pNew ) { pMyTxtNd = (SwTxtNode*)pNew; }

          SwCharFmt* GetCharFmt();
    const SwCharFmt* GetCharFmt() const
            { return ((SwTxtRuby*)this)->GetCharFmt(); }
};

// ******************************

class SwTxt2Lines : public SwTxtAttrEnd
{
public:
    SwTxt2Lines( const SvxTwoLinesItem& rAttr,
                    xub_StrLen nStart, xub_StrLen nEnd );
};

// --------------- Inline Implementierungen ------------------------

inline const SwTxtNode& SwTxtRuby::GetTxtNode() const
{
    ASSERT( pMyTxtNd, "SwTxtRuby:: wo ist mein TextNode?" );
    return *pMyTxtNd;
}

#endif
