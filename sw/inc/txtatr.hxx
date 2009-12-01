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
    SwTxtNode * m_pTxtNode;
    USHORT m_nSortNumber;

public:
    SwTxtCharFmt( SwFmtCharFmt& rAttr, xub_StrLen nStart, xub_StrLen nEnd );
    virtual ~SwTxtCharFmt( );

    // werden vom SwFmtCharFmt hierher weitergeleitet
    virtual void Modify( SfxPoolItem*, SfxPoolItem* );    // SwClient
    virtual BOOL GetInfo( SfxPoolItem& rInfo ) const;

    // get and set TxtNode pointer
    void ChgTxtNode( SwTxtNode* pNew ) { m_pTxtNode = pNew; }

    void SetSortNumber( USHORT nSortNumber ) { m_nSortNumber = nSortNumber; }
    USHORT GetSortNumber() const { return m_nSortNumber; }
};

// ATT_HARDBLANK ******************************

class SwTxtHardBlank : public SwTxtAttr
{
    sal_Unicode m_Char;

public:
    SwTxtHardBlank( const SwFmtHardBlank& rAttr, xub_StrLen nStart );
    inline sal_Unicode GetChar() const  { return m_Char; }
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
    SwTxtNode* m_pTxtNode;

public:
    SwTxtRuby( SwFmtRuby& rAttr, xub_StrLen nStart, xub_StrLen nEnd );
    virtual ~SwTxtRuby();
    TYPEINFO();

    virtual void Modify( SfxPoolItem *pOld, SfxPoolItem *pNew);
    virtual BOOL GetInfo( SfxPoolItem& rInfo ) const;

    /// get and set TxtNode pointer
           const SwTxtNode* GetpTxtNode() const { return m_pTxtNode; }
    inline const SwTxtNode& GetTxtNode() const;
    void ChgTxtNode( SwTxtNode* pNew ) { m_pTxtNode = pNew; }

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
    ASSERT( m_pTxtNode, "SwTxtRuby: where is my TxtNode?" );
    return *m_pTxtNode;
}

#endif
