/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _OUTLEENG_HXX
#define _OUTLEENG_HXX

#include <editeng/outliner.hxx>
#ifndef _EDITENG_HXX
#include <editeng/editeng.hxx>
#endif

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

    // #101498#
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

