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
    virtual const SvxNumberFormat*  GetNumberFormat( USHORT nPara ) const;

public:
                        OutlinerEditEng( Outliner* pOwner, SfxItemPool* pPool );
                        ~OutlinerEditEng();

    virtual void        PaintingFirstLine( USHORT nPara, const Point& rStartPos, long nBaseLineY, const Point& rOrigin, short nOrientation, OutputDevice* pOutDev );

    virtual void        ParagraphInserted( USHORT nNewParagraph );
    virtual void        ParagraphDeleted( USHORT nDeletedParagraph );
    virtual void        ParagraphConnected( USHORT nLeftParagraph, USHORT nRightParagraph );

    virtual void DrawingText(
        const Point& rStartPos, const XubString& rText, USHORT nTextStart, USHORT nTextLen, const sal_Int32* pDXArray, const SvxFont& rFont,
        USHORT nPara, USHORT nIndex, BYTE nRightToLeft,
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
    virtual void        ParaAttribsChanged( USHORT nPara );
    virtual BOOL        SpellNextDocument();
    virtual XubString   GetUndoComment( USHORT nUndoId ) const;

    // for text conversion
    virtual BOOL        ConvertNextDocument();

    virtual void        FieldClicked( const SvxFieldItem& rField, USHORT nPara, USHORT nPos );
    virtual void        FieldSelected( const SvxFieldItem& rField, USHORT nPara, USHORT nPos );
    virtual XubString   CalcFieldValue( const SvxFieldItem& rField, USHORT nPara, USHORT nPos, Color*& rTxtColor, Color*& rFldColor );

    virtual Rectangle   GetBulletArea( USHORT nPara );

       virtual void        SetParaAttribs( USHORT nPara, const SfxItemSet& rSet );

    // belongs into class Outliner, move there before incompatible update!
    Link                aOutlinerNotifyHdl;
    NotifyList          aNotifyCache;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
