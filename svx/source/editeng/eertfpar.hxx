/*************************************************************************
 *
 *  $RCSfile: eertfpar.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:14 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _EERTFPAR_HXX
#define _EERTFPAR_HXX

#include <svxrtf.hxx>

#include <editdoc.hxx>
#include <impedit.hxx>

#ifndef SVX_LIGHT

class EditNodeIdx : public SvxNodeIdx
{
private:
    ContentNode*        pNode;
    ImpEditEngine*      pImpEditEngine;

public:
                        EditNodeIdx( ImpEditEngine* pIEE, ContentNode* pNd = 0)
                            { pImpEditEngine = pIEE; pNode = pNd; }
    virtual ULONG       GetIdx() const;
    virtual SvxNodeIdx* Clone() const;
    ContentNode*        GetNode() { return pNode; }
};

class EditPosition : public SvxPosition
{
private:
    EditSelection*  pCurSel;
    ImpEditEngine*  pImpEditEngine;

public:
                    EditPosition( ImpEditEngine* pIEE, EditSelection* pSel )
                            { pImpEditEngine = pIEE; pCurSel = pSel; }

    virtual ULONG   GetNodeIdx() const;
    virtual USHORT  GetCntIdx() const;

    // erzeuge von sich selbst eine Kopie
    virtual SvxPosition* Clone() const;

    // erzeuge vom NodeIndex eine Kopie
    virtual SvxNodeIdx* MakeNodeIdx() const;
};

#define ACTION_INSERTTEXT       1
#define ACTION_INSERTPARABRK    2

class EditRTFParser : public SvxRTFParser
{
private:
    EditSelection       aCurSel;
    ImpEditEngine*      pImpEditEngine;
    CharSet             eDestCharSet;
    MapMode             aRTFMapMode;
    MapMode             aEditMapMode;

    USHORT              nDefFont;
    USHORT              nDefTab;
    USHORT              nDefFontHeight;
    BYTE                nLastAction;

protected:
    virtual void        InsertPara();
    virtual void        InsertText();
    virtual void        MovePos( int bForward = TRUE );
    virtual void        SetEndPrevPara( SvxNodeIdx*& rpNodePos,
                                            USHORT& rCntPos );

    virtual void        UnknownAttrToken( int nToken, SfxItemSet* pSet );
    virtual void        NextToken( int nToken );
    virtual void        SetAttrInDoc( SvxRTFItemStackType &rSet );
    inline long         TwipsToLogic( long n );
    virtual int         IsEndPara( SvxNodeIdx* pNd, USHORT nCnt ) const;
    virtual void        CalcValue();
    void                CreateStyleSheets();
    SfxStyleSheet*      CreateStyleSheet( SvxRTFStyleType* pRTFStyle );
    SvxRTFStyleType*    FindStyleSheet( const String& rName );
    void                AddRTFDefaultValues( const EditPaM& rStart, const EditPaM& rEnd );
    void                ReadField();

public:
                EditRTFParser( SvStream& rIn, EditSelection aCurSel, SfxItemPool& rAttrPool, ImpEditEngine* pImpEditEngine );
                ~EditRTFParser();

    virtual SvParserState   CallParser();


    void        SetDestCharSet( CharSet eCharSet )  { eDestCharSet = eCharSet; }
    CharSet     GetDestCharSet() const              { return eDestCharSet; }

    USHORT      GetDefTab() const                   { return nDefTab; }
    Font        GetDefFont()                        { return GetFont( nDefFont ); }

    EditPaM     GetCurPaM() const                   { return aCurSel.Max(); }
};

SV_DECL_REF( EditRTFParser );
SV_IMPL_REF( EditRTFParser );

inline long EditRTFParser::TwipsToLogic( long nTwps )
{
    Size aSz( nTwps, 0 );
    aSz = pImpEditEngine->GetRefDevice()->LogicToLogic( aSz, &aRTFMapMode, &aEditMapMode );
    return aSz.Width();
}

#endif  // !SVX_LIGH
#endif  //_EERTFPAR_HXX
