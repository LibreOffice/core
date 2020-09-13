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

#include <editeng/outliner.hxx>
#include <editeng/editeng.hxx>

typedef std::vector<EENotify> NotifyList;

class OutlinerEditEng : public EditEngine
{
    Outliner*           pOwner;

protected:

    // derived from EditEngine. Allows Outliner objects to provide
    // bullet access to the EditEngine.
    virtual const SvxNumberFormat*  GetNumberFormat( sal_Int32 nPara ) const override;

public:
                        OutlinerEditEng( Outliner* pOwner, SfxItemPool* pPool );
                        virtual ~OutlinerEditEng() override;

    virtual void        PaintingFirstLine( sal_Int32 nPara, const Point& rStartPos, long nBaseLineY, const Point& rOrigin, short nOrientation, OutputDevice* pOutDev ) override;

    virtual void        ParagraphInserted( sal_Int32 nNewParagraph ) override;
    virtual void        ParagraphDeleted( sal_Int32 nDeletedParagraph ) override;
    virtual void        ParagraphConnected( sal_Int32 nLeftParagraph, sal_Int32 nRightParagraph ) override;

    virtual void DrawingText( const Point& rStartPos, const OUString& rText, sal_Int32 nTextStart,
                              sal_Int32 nTextLen, const long* pDXArray, const SvxFont& rFont,
                              sal_Int32 nPara, sal_uInt8 nRightToLeft,
                              const EEngineData::WrongSpellVector* pWrongSpellVector,
                              const SvxFieldData* pFieldData,
                              bool bEndOfLine,
                              bool bEndOfParagraph,
                              const css::lang::Locale* pLocale,
                              const Color& rOverlineColor,
                              const Color& rTextLineColor) override;

    virtual void DrawingTab(
        const Point& rStartPos, long nWidth, const OUString& rChar,
        const SvxFont& rFont, sal_Int32 nPara, sal_uInt8 nRightToLeft,
        bool bEndOfLine,
        bool bEndOfParagraph,
        const Color& rOverlineColor,
        const Color& rTextLineColor) override;

    virtual void        StyleSheetChanged( SfxStyleSheet* pStyle ) override;
    virtual void        ParaAttribsChanged( sal_Int32 nPara ) override;
    virtual bool        SpellNextDocument() override;
    virtual OUString    GetUndoComment( sal_uInt16 nUndoId ) const override;

    // for text conversion
    virtual bool        ConvertNextDocument() override;

    virtual OUString    CalcFieldValue( const SvxFieldItem& rField, sal_Int32 nPara, sal_Int32 nPos, std::optional<Color>& rTxtColor, std::optional<Color>& rFldColor ) override;

    virtual tools::Rectangle   GetBulletArea( sal_Int32 nPara ) override;

       virtual void        SetParaAttribs( sal_Int32 nPara, const SfxItemSet& rSet ) override;

    // belongs into class Outliner, move there before incompatible update!
    Link<EENotify&,void> aOutlinerNotifyHdl;
    NotifyList          aNotifyCache;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
