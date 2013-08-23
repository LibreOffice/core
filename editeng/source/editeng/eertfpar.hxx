/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef _EERTFPAR_HXX
#define _EERTFPAR_HXX

#include <editeng/svxrtf.hxx>

#include <editdoc.hxx>
#include <impedit.hxx>

class EditEngine;

class EditNodeIdx : public SvxNodeIdx
{
private:
    EditEngine*   mpEditEngine;
    ContentNode*  mpNode;

public:
    EditNodeIdx(EditEngine* pEE, ContentNode* pNd = NULL);

    virtual sal_Int32   GetIdx() const;
    virtual SvxNodeIdx* Clone() const;
    ContentNode* GetNode() { return mpNode; }
};

class EditPosition : public SvxPosition
{
private:
    EditEngine*     mpEditEngine;
    EditSelection*  mpCurSel;

public:
    EditPosition(EditEngine* pIEE, EditSelection* pSel);

    virtual sal_Int32   GetNodeIdx() const;
    virtual sal_uInt16  GetCntIdx() const;

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
    EditEngine*         mpEditEngine;
    CharSet             eDestCharSet;
    MapMode             aRTFMapMode;
    MapMode             aEditMapMode;

    sal_uInt16              nDefFont;
    sal_uInt16              nDefTab;
    sal_uInt16              nDefFontHeight;
    sal_uInt8               nLastAction;

protected:
    virtual void        InsertPara();
    virtual void        InsertText();
    virtual void        MovePos( int bForward = sal_True );
    virtual void        SetEndPrevPara( SvxNodeIdx*& rpNodePos,
                                            sal_uInt16& rCntPos );

    virtual void        UnknownAttrToken( int nToken, SfxItemSet* pSet );
    virtual void        NextToken( int nToken );
    virtual void        SetAttrInDoc( SvxRTFItemStackType &rSet );
    virtual int         IsEndPara( SvxNodeIdx* pNd, sal_uInt16 nCnt ) const;
    virtual void        CalcValue();
    void                CreateStyleSheets();
    SfxStyleSheet*      CreateStyleSheet( SvxRTFStyleType* pRTFStyle );
    SvxRTFStyleType*    FindStyleSheet( const OUString& rName );
    void                AddRTFDefaultValues( const EditPaM& rStart, const EditPaM& rEnd );
    void                ReadField();
    void                SkipGroup();

public:
    EditRTFParser(SvStream& rIn, EditSelection aCurSel, SfxItemPool& rAttrPool, EditEngine* pEditEngine);
    ~EditRTFParser();

    virtual SvParserState   CallParser();


    void        SetDestCharSet( CharSet eCharSet )  { eDestCharSet = eCharSet; }
    CharSet     GetDestCharSet() const              { return eDestCharSet; }

    sal_uInt16      GetDefTab() const                   { return nDefTab; }
    Font        GetDefFont()                        { return GetFont( nDefFont ); }

    EditPaM     GetCurPaM() const                   { return aCurSel.Max(); }
};

SV_DECL_REF( EditRTFParser )
SV_IMPL_REF( EditRTFParser );


#endif  //_EERTFPAR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
