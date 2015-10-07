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
#ifndef INCLUDED_EDITENG_SOURCE_EDITENG_EERTFPAR_HXX
#define INCLUDED_EDITENG_SOURCE_EDITENG_EERTFPAR_HXX

#include <editeng/svxrtf.hxx>

#include <editdoc.hxx>
#include <impedit.hxx>

class EditEngine;

class EditPosition : public SvxPosition
{
private:
    EditEngine*     mpEditEngine;
    EditSelection*  mpCurSel;

public:
    EditPosition(EditEngine* pIEE, EditSelection* pSel);

    virtual sal_Int32   GetNodeIdx() const SAL_OVERRIDE;
    virtual sal_Int32   GetCntIdx() const SAL_OVERRIDE;

    // clone
    virtual SvxPosition* Clone() const SAL_OVERRIDE;

    // clone NodeIndex
    virtual EditNodeIdx* MakeNodeIdx() const SAL_OVERRIDE;
};

#define ACTION_INSERTTEXT       1
#define ACTION_INSERTPARABRK    2

class EditRTFParser : public SvxRTFParser
{
private:
    EditSelection       aCurSel;
    EditEngine*         mpEditEngine;
    rtl_TextEncoding    eDestCharSet;
    MapMode             aRTFMapMode;
    MapMode             aEditMapMode;

    sal_uInt16              nDefFont;
    sal_uInt16              nDefTab;
    sal_uInt16              nDefFontHeight;
    sal_uInt8               nLastAction;

protected:
    virtual void        InsertPara() SAL_OVERRIDE;
    virtual void        InsertText() SAL_OVERRIDE;
    virtual void        MovePos( bool bForward = true ) SAL_OVERRIDE;
    virtual void        SetEndPrevPara( EditNodeIdx*& rpNodePos,
                                        sal_Int32& rCntPos ) SAL_OVERRIDE;

    virtual void        UnknownAttrToken( int nToken, SfxItemSet* pSet ) SAL_OVERRIDE;
    virtual void        NextToken( int nToken ) SAL_OVERRIDE;
    virtual void        SetAttrInDoc( SvxRTFItemStackType &rSet ) SAL_OVERRIDE;
    virtual bool        IsEndPara( EditNodeIdx* pNd, sal_Int32 nCnt ) const SAL_OVERRIDE;
    virtual void        CalcValue() SAL_OVERRIDE;
    void                CreateStyleSheets();
    SfxStyleSheet*      CreateStyleSheet( SvxRTFStyleType* pRTFStyle );
    SvxRTFStyleType*    FindStyleSheet( const OUString& rName );
    void                AddRTFDefaultValues( const EditPaM& rStart, const EditPaM& rEnd );
    void                ReadField();
    void                SkipGroup();

public:
    EditRTFParser(SvStream& rIn, EditSelection aCurSel, SfxItemPool& rAttrPool, EditEngine* pEditEngine);
    virtual ~EditRTFParser();

    virtual SvParserState   CallParser() SAL_OVERRIDE;

    vcl::Font       GetDefFont()                        { return GetFont( nDefFont ); }

    EditPaM         GetCurPaM() const                   { return aCurSel.Max(); }
};

typedef tools::SvRef<EditRTFParser> EditRTFParserRef;

#endif // INCLUDED_EDITENG_SOURCE_EDITENG_EERTFPAR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
