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
#ifndef _OUTLEENG_HXX
#define _OUTLEENG_HXX

#include <editeng/outliner.hxx>
#include <editeng/editeng.hxx>

typedef EENotify* EENotifyPtr;
SV_DECL_PTRARR_DEL( NotifyList, EENotifyPtr, 1, 1 )

class OutlinerEditEng : public EditEngine
{
    Outliner*           pOwner;

protected:

    // derived from EditEngine. Allows Outliner objetcs to provide
    // bullet access to the EditEngine.
    virtual const SvxNumberFormat*  GetNumberFormat( sal_uInt16 nPara ) const;

public:
                        OutlinerEditEng( Outliner* pOwner, SfxItemPool* pPool );
                        ~OutlinerEditEng();

    virtual void        PaintingFirstLine( sal_uInt16 nPara, const Point& rStartPos, long nBaseLineY, const Point& rOrigin, short nOrientation, OutputDevice* pOutDev );

    virtual void        ParagraphInserted( sal_uInt16 nNewParagraph );
    virtual void        ParagraphDeleted( sal_uInt16 nDeletedParagraph );
    virtual void        ParagraphConnected( sal_uInt16 nLeftParagraph, sal_uInt16 nRightParagraph );

    virtual void DrawingText(
        const Point& rStartPos, const XubString& rText, sal_uInt16 nTextStart, sal_uInt16 nTextLen, const sal_Int32* pDXArray, const SvxFont& rFont,
        sal_uInt16 nPara, sal_uInt16 nIndex, sal_uInt8 nRightToLeft,
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
        const SvxFont& rFont, USHORT nPara, xub_StrLen nIndex, BYTE nRightToLeft,
        bool bEndOfLine,
        bool bEndOfParagraph,
        const Color& rOverlineColor,
        const Color& rTextLineColor);

    virtual void        StyleSheetChanged( SfxStyleSheet* pStyle );
    virtual void        ParaAttribsChanged( sal_uInt16 nPara );
    virtual sal_Bool        SpellNextDocument();
    virtual XubString   GetUndoComment( sal_uInt16 nUndoId ) const;

    // for text conversion
    virtual sal_Bool        ConvertNextDocument();

    virtual void        FieldClicked( const SvxFieldItem& rField, sal_uInt16 nPara, sal_uInt16 nPos );
    virtual void        FieldSelected( const SvxFieldItem& rField, sal_uInt16 nPara, sal_uInt16 nPos );
    virtual XubString   CalcFieldValue( const SvxFieldItem& rField, sal_uInt16 nPara, sal_uInt16 nPos, Color*& rTxtColor, Color*& rFldColor );

    virtual Rectangle   GetBulletArea( sal_uInt16 nPara );

       virtual void        SetParaAttribs( sal_uInt16 nPara, const SfxItemSet& rSet );

    // belongs into class Outliner, move there before incompatible update!
    Link                aOutlinerNotifyHdl;
    NotifyList          aNotifyCache;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
