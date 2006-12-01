/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: txtatr.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: rt $ $Date: 2006-12-01 15:34:27 $
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
#ifndef _TXTATR_HXX
#define _TXTATR_HXX
#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif
#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif
#ifndef _TXATBASE_HXX
#include <txatbase.hxx>     // SwTxtAttr/SwTxtAttrEnd
#endif
#ifndef _CALBCK_HXX
#include <calbck.hxx>
#endif

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

class SwTxtRuby : public SwTxtAttrEnd, public SwClient
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
