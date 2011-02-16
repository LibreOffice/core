/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef _EERTFPAR_HXX
#define _EERTFPAR_HXX

#include <editeng/svxrtf.hxx>

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

    // clone
    virtual SvxPosition* Clone() const;

    // clone NodeIndex
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
    virtual int         IsEndPara( SvxNodeIdx* pNd, USHORT nCnt ) const;
    virtual void        CalcValue();
    void                CreateStyleSheets();
    SfxStyleSheet*      CreateStyleSheet( SvxRTFStyleType* pRTFStyle );
    SvxRTFStyleType*    FindStyleSheet( const String& rName );
    void                AddRTFDefaultValues( const EditPaM& rStart, const EditPaM& rEnd );
    void                ReadField();
    void                SkipGroup();

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

SV_DECL_REF( EditRTFParser )
SV_IMPL_REF( EditRTFParser );


#endif  // !SVX_LIGH
#endif  //_EERTFPAR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
