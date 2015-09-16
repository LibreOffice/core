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
#ifndef INCLUDED_EDITENG_SOURCE_OUTLINER_OUTLEENG_HXX
#define INCLUDED_EDITENG_SOURCE_OUTLINER_OUTLEENG_HXX

#include <editeng/outliner.hxx>
#include <editeng/editeng.hxx>

typedef std::vector<EENotify> NotifyList;

class OutlinerEditEng : public EditEngine
{
    Outliner*           pOwner;

protected:

    // derived from EditEngine. Allows Outliner objetcs to provide
    // bullet access to the EditEngine.
    virtual const SvxNumberFormat*  GetNumberFormat( sal_Int32 nPara ) const SAL_OVERRIDE;

public:
                        OutlinerEditEng( Outliner* pOwner, SfxItemPool* pPool );
                        virtual ~OutlinerEditEng();

    virtual void        PaintingFirstLine( sal_Int32 nPara, const Point& rStartPos, long nBaseLineY, const Point& rOrigin, short nOrientation, OutputDevice* pOutDev ) SAL_OVERRIDE;

    virtual void        ParagraphInserted( sal_Int32 nNewParagraph ) SAL_OVERRIDE;
    virtual void        ParagraphDeleted( sal_Int32 nDeletedParagraph ) SAL_OVERRIDE;
    virtual void        ParagraphConnected( sal_Int32 nLeftParagraph, sal_Int32 nRightParagraph ) SAL_OVERRIDE;

    virtual void DrawingText( const Point& rStartPos, const OUString& rText, sal_Int32 nTextStart,
                              sal_Int32 nTextLen, const long* pDXArray, const SvxFont& rFont,
                              sal_Int32 nPara, sal_Int32 nIndex, sal_uInt8 nRightToLeft,
                              const EEngineData::WrongSpellVector* pWrongSpellVector,
                              const SvxFieldData* pFieldData,
        bool bEndOfLine,
        bool bEndOfParagraph,
        bool bEndOfBullet,
        const css::lang::Locale* pLocale,
        const Color& rOverlineColor,
        const Color& rTextLineColor) SAL_OVERRIDE;

    virtual void DrawingTab(
        const Point& rStartPos, long nWidth, const OUString& rChar,
        const SvxFont& rFont, sal_Int32 nPara, sal_Int32 nIndex, sal_uInt8 nRightToLeft,
        bool bEndOfLine,
        bool bEndOfParagraph,
        const Color& rOverlineColor,
        const Color& rTextLineColor) SAL_OVERRIDE;

    virtual void        StyleSheetChanged( SfxStyleSheet* pStyle ) SAL_OVERRIDE;
    virtual void        ParaAttribsChanged( sal_Int32 nPara ) SAL_OVERRIDE;
    virtual bool        SpellNextDocument() SAL_OVERRIDE;
    virtual OUString    GetUndoComment( sal_uInt16 nUndoId ) const SAL_OVERRIDE;

    // for text conversion
    virtual bool        ConvertNextDocument() SAL_OVERRIDE;

    virtual void        FieldClicked( const SvxFieldItem& rField, sal_Int32 nPara, sal_Int32 nPos ) SAL_OVERRIDE;
    virtual OUString    CalcFieldValue( const SvxFieldItem& rField, sal_Int32 nPara, sal_Int32 nPos, Color*& rTxtColor, Color*& rFldColor ) SAL_OVERRIDE;

    virtual Rectangle   GetBulletArea( sal_Int32 nPara ) SAL_OVERRIDE;

       virtual void        SetParaAttribs( sal_Int32 nPara, const SfxItemSet& rSet ) SAL_OVERRIDE;

    // belongs into class Outliner, move there before incompatible update!
    Link<EENotify&,void> aOutlinerNotifyHdl;
    NotifyList          aNotifyCache;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
