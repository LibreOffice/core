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
#ifndef _W4WSTK_HXX
#define _W4WSTK_HXX

#ifndef _SVARRAY_HXX //autogen
#include <bf_svtools/svarray.hxx>
#endif

#ifndef _SFXPOOLITEM_HXX //autogen
#include <bf_svtools/poolitem.hxx>
#endif

#ifndef _NDINDEX_HXX
#include <ndindex.hxx>
#endif
namespace binfilter {

class SwW4WParser;
class SwDoc;
struct SwPosition;
class SwPaM;
class SwFlyFrmFmt;

/* ------ Stack-Eintrag fuer die Attribute -----------
 * Es werden immer Pointer auf neue Attribute uebergeben.
 */

struct W4WStkEntry
{
    SfxPoolItem * pAttr;    // Format Attribute
    SwNodeIndex nMkNode;
    SwNodeIndex nPtNode;
    xub_StrLen nMkCntnt;        // Nachbildung von Mark()
    xub_StrLen nPtCntnt;        // Nachbildung von GetPoint()
    BOOL bLocked : 1;
    BOOL bCopied : 1;
    BOOL bClosed : 1;		// Zeigt ueber Child geschlossenes Attr. an.
    BOOL bNeverIntoDoc : 1;	// Dieser Entry soll nichts ins Doc sondern,
                            // nach dem Schliessen geloescht werden.

    W4WStkEntry( const SwPosition & rStartPos,
                    SfxPoolItem* pHt,
                    BOOL bInitCopied = FALSE,
                    BOOL bInitNID    = FALSE );

    ~W4WStkEntry();

    void SetEndPos( const SwPosition & rEndPos );
    BOOL MakeRegion( SwPaM & rRegion ) const;

private:
    W4WStkEntry( const W4WStkEntry& rEntry ); // niemals aufrufen, nur Complier-Test
};

// ein Stack fuer die gesamten Text-Attribute
typedef W4WStkEntry* W4WStkEntryPtr;
SV_DECL_PTRARR(W4WCtrlStkEntries,W4WStkEntryPtr,5,10)

class W4WCtrlStack : public W4WCtrlStkEntries
{
    SwW4WParser*  pParser;
    W4WCtrlStack* pParentStack;

public:
    W4WCtrlStack( SwW4WParser& rParser );
    W4WCtrlStack( W4WCtrlStack& rCpy, const SwPosition& rPos );
    ~W4WCtrlStack();

    void NewAttr( const SwPosition& rPos, const SfxPoolItem & rAttr );

    BOOL SetAttr( const SwPosition& rPos, USHORT nAttrId=0,
                    BOOL bTstEnde       = TRUE,
                    BOOL bDoNotSetInDoc = FALSE );
    void SetAttrInDoc( SwPaM& rRegion, const W4WStkEntry& rEntry );
    void SetLockedAttrClosed( USHORT nAttrId );
    void SetEndForClosedEntries( const SwPosition& rPos );
    void StealAttr( const SwPosition& rPos, USHORT nAttrId = 0 );
    void StealWWTabAttr( const SwPosition& rPos );

    SfxPoolItem* GetFmtStkAttr( USHORT nWhich, USHORT * pPos = 0 );
    const SfxPoolItem* GetFmtAttr( const SwPaM& rPaM, USHORT nWhich );
    BOOL IsAttrOpen( USHORT nAttrId );
};


// weitere Teile aus ehemaligem swrtf.hxx
class SwW4WStyle : public SfxPoolItem
{
public:
    USHORT nStyleId;
    SwW4WStyle( USHORT nId );
    SwW4WStyle( const SwW4WStyle & rRTFFld );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* = 0 ) const;

    USHORT GetStyleId() const { return nStyleId; }
};

class SwW4WAnchor : public SfxPoolItem
{
    SwFlyFrmFmt* pFlyFmt;
public:
    SwW4WAnchor( SwFlyFrmFmt* pFlyFmt );
    SwW4WAnchor( const SwW4WAnchor& );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* = 0 ) const;

    const SwFlyFrmFmt* GetFlyFmt() const { return pFlyFmt; }
          SwFlyFrmFmt* GetFlyFmt() 	  { return pFlyFmt; }
};

} //namespace binfilter
#endif
