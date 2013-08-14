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
#ifndef _OUTLEENG_HXX
#define _OUTLEENG_HXX

#include <editeng/outliner.hxx>
#include <editeng/editeng.hxx>

typedef std::vector<EENotify> NotifyList;

class OutlinerEditEng : public EditEngine
{
    Outliner*           pOwner;

protected:

    // derived from EditEngine. Allows Outliner objetcs to provide
    // bullet access to the EditEngine.
    virtual const SvxNumberFormat*  GetNumberFormat( sal_Int32 nPara ) const;

public:
                        OutlinerEditEng( Outliner* pOwner, SfxItemPool* pPool );
                        ~OutlinerEditEng();

    virtual void        PaintingFirstLine( sal_Int32 nPara, const Point& rStartPos, long nBaseLineY, const Point& rOrigin, short nOrientation, OutputDevice* pOutDev );

    virtual void        ParagraphInserted( sal_Int32 nNewParagraph );
    virtual void        ParagraphDeleted( sal_Int32 nDeletedParagraph );
    virtual void        ParagraphConnected( sal_Int32 nLeftParagraph, sal_Int32 nRightParagraph );

    virtual void DrawingText(
        const Point& rStartPos, const OUString& rText, sal_uInt16 nTextStart, sal_uInt16 nTextLen, const sal_Int32* pDXArray, const SvxFont& rFont,
        sal_Int32 nPara, sal_uInt16 nIndex, sal_uInt8 nRightToLeft,
        const EEngineData::WrongSpellVector* pWrongSpellVector,
        const SvxFieldData* pFieldData,
        bool bEndOfLine,
        bool bEndOfParagraph,
        bool bEndOfBullet,
        const ::com::sun::star::lang::Locale* pLocale,
        const Color& rOverlineColor,
        const Color& rTextLineColor);

    virtual void DrawingTab(
        const Point& rStartPos, long nWidth, const String& rChar,
        const SvxFont& rFont, sal_Int32 nPara, xub_StrLen nIndex, sal_uInt8 nRightToLeft,
        bool bEndOfLine,
        bool bEndOfParagraph,
        const Color& rOverlineColor,
        const Color& rTextLineColor);

    virtual void        StyleSheetChanged( SfxStyleSheet* pStyle );
    virtual void        ParaAttribsChanged( sal_Int32 nPara );
    virtual sal_Bool    SpellNextDocument();
    virtual OUString    GetUndoComment( sal_uInt16 nUndoId ) const;

    // for text conversion
    virtual sal_Bool        ConvertNextDocument();

    virtual void        FieldClicked( const SvxFieldItem& rField, sal_Int32 nPara, sal_uInt16 nPos );
    virtual void        FieldSelected( const SvxFieldItem& rField, sal_Int32 nPara, sal_uInt16 nPos );
    virtual OUString    CalcFieldValue( const SvxFieldItem& rField, sal_Int32 nPara, sal_uInt16 nPos, Color*& rTxtColor, Color*& rFldColor );

    virtual Rectangle   GetBulletArea( sal_Int32 nPara );

       virtual void        SetParaAttribs( sal_Int32 nPara, const SfxItemSet& rSet );

    // belongs into class Outliner, move there before incompatible update!
    Link                aOutlinerNotifyHdl;
    NotifyList          aNotifyCache;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
