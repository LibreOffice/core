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
#pragma once

#include <editeng/svxrtf.hxx>

#include <editdoc.hxx>

class EditEngine;

class EditRTFParser final : public SvxRTFParser
{
private:
    EditSelection       aCurSel;
    EditEngine*         mpEditEngine;
    MapMode             aEditMapMode;

    sal_uInt16          nDefFont;
    bool                bLastActionInsertParaBreak;

    virtual void        InsertPara() override;
    virtual void        InsertText() override;
    virtual void        MovePos( bool bForward = true ) override;
    virtual void        SetEndPrevPara( EditNodeIdx*& rpNodePos,
                                        sal_Int32& rCntPos ) override;

    virtual void        UnknownAttrToken( int nToken ) override;
    virtual void        NextToken( int nToken ) override;
    virtual void        SetAttrInDoc( SvxRTFItemStackType &rSet ) override;
    virtual bool        IsEndPara( EditNodeIdx* pNd, sal_Int32 nCnt ) const override;
    virtual void        CalcValue() override;
    void                CreateStyleSheets();
    SfxStyleSheet*      CreateStyleSheet( SvxRTFStyleType const * pRTFStyle );
    SvxRTFStyleType*    FindStyleSheet( const OUString& rName );
    void                AddRTFDefaultValues( const EditPaM& rStart, const EditPaM& rEnd );
    void                ReadField();
    void                SkipGroup();

public:
    EditRTFParser(SvStream& rIn, EditSelection aCurSel, SfxItemPool& rAttrPool, EditEngine* pEditEngine);
    virtual ~EditRTFParser() override;

    virtual SvParserState   CallParser() override;

    EditPaM const &         GetCurPaM() const                   { return aCurSel.Max(); }
};

typedef tools::SvRef<EditRTFParser> EditRTFParserRef;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
