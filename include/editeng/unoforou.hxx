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

#ifndef _SVX_UNOFOROU_HXX
#define _SVX_UNOFOROU_HXX

#include <editeng/unoedsrc.hxx>
#include "editeng/editengdllapi.h"

#include <editeng/editdata.hxx>

class Outliner;

//  SvxOutlinerForwarder - SvxTextForwarder for Outliner

class EDITENG_DLLPUBLIC SvxOutlinerForwarder : public SvxTextForwarder
{
private:
    Outliner&           rOutliner;
    sal_Bool                bOutlinerText;

    /** this pointer may be null or point to an item set for the attribs of
        the selection maAttribsSelection */
    mutable SfxItemSet* mpAttribsCache;

    /** if we have a chached attribute item set, this is the selection of it */
    mutable ESelection  maAttribCacheSelection;

    /** this pointer may be null or point to an item set for the paragraph
        mnParaAttribsCache */
    mutable SfxItemSet* mpParaAttribsCache;

    /** if we have a cached para attribute item set, this is the paragraph of it */
    mutable sal_Int32   mnParaAttribsCache;

public:
                        SvxOutlinerForwarder( Outliner& rOutl, sal_Bool bOutlText = sal_False );
    virtual             ~SvxOutlinerForwarder();

    virtual sal_Int32   GetParagraphCount() const;
    virtual sal_uInt16  GetTextLen( sal_Int32 nParagraph ) const;
    virtual OUString    GetText( const ESelection& rSel ) const;
    virtual SfxItemSet  GetAttribs( const ESelection& rSel, sal_Bool bOnlyHardAttrib = 0 ) const;
    virtual SfxItemSet  GetParaAttribs( sal_Int32 nPara ) const;
    virtual void        SetParaAttribs( sal_Int32 nPara, const SfxItemSet& rSet );
    virtual void        RemoveAttribs( const ESelection& rSelection, sal_Bool bRemoveParaAttribs, sal_uInt16 nWhich );
    virtual void        GetPortions( sal_Int32 nPara, std::vector<sal_uInt16>& rList ) const;

    virtual sal_uInt16      GetItemState( const ESelection& rSel, sal_uInt16 nWhich ) const;
    virtual sal_uInt16      GetItemState( sal_Int32 nPara, sal_uInt16 nWhich ) const;

    virtual void        QuickInsertText( const OUString& rText, const ESelection& rSel );
    virtual void        QuickInsertField( const SvxFieldItem& rFld, const ESelection& rSel );
    virtual void        QuickSetAttribs( const SfxItemSet& rSet, const ESelection& rSel );
    virtual void        QuickInsertLineBreak( const ESelection& rSel );

    virtual SfxItemPool* GetPool() const;

    virtual OUString    CalcFieldValue( const SvxFieldItem& rField, sal_Int32 nPara, sal_uInt16 nPos, Color*& rpTxtColor, Color*& rpFldColor );
    virtual void         FieldClicked( const SvxFieldItem& rField, sal_Int32 nPara, xub_StrLen nPos );

    virtual sal_Bool        IsValid() const;

    Outliner&           GetOutliner() const { return rOutliner; }

    virtual LanguageType    GetLanguage( sal_Int32, sal_uInt16 ) const;
    virtual sal_uInt16          GetFieldCount( sal_Int32 nPara ) const;
    virtual EFieldInfo      GetFieldInfo( sal_Int32 nPara, sal_uInt16 nField ) const;
    virtual EBulletInfo     GetBulletInfo( sal_Int32 nPara ) const;
    virtual Rectangle       GetCharBounds( sal_Int32 nPara, sal_uInt16 nIndex ) const;
    virtual Rectangle       GetParaBounds( sal_Int32 nPara ) const;
    virtual MapMode         GetMapMode() const;
    virtual OutputDevice*   GetRefDevice() const;
    virtual sal_Bool        GetIndexAtPoint( const Point&, sal_Int32& nPara, sal_uInt16& nIndex ) const;
    virtual sal_Bool        GetWordIndices( sal_Int32 nPara, sal_uInt16 nIndex, sal_uInt16& nStart, sal_uInt16& nEnd ) const;
    virtual sal_Bool        GetAttributeRun( sal_uInt16& nStartIndex, sal_uInt16& nEndIndex, sal_Int32 nPara, sal_uInt16 nIndex ) const;
    virtual sal_uInt16          GetLineCount( sal_Int32 nPara ) const;
    virtual sal_uInt16          GetLineLen( sal_Int32 nPara, sal_uInt16 nLine ) const;
    virtual void            GetLineBoundaries( /*out*/sal_uInt16 &rStart, /*out*/sal_uInt16 &rEnd, sal_Int32 nPara, sal_uInt16 nLine ) const;
    virtual sal_uInt16          GetLineNumberAtIndex( sal_Int32 nPara, sal_uInt16 nIndex ) const;
    virtual sal_Bool        Delete( const ESelection& );
    virtual sal_Bool        InsertText( const OUString&, const ESelection& );
    virtual sal_Bool        QuickFormatDoc( sal_Bool bFull=sal_False );
    virtual sal_Int16       GetDepth( sal_Int32 nPara ) const;
    virtual sal_Bool        SetDepth( sal_Int32 nPara, sal_Int16 nNewDepth );
    virtual sal_Int16       GetNumberingStartValue( sal_Int32 nPara );
    virtual void            SetNumberingStartValue( sal_Int32 nPara, sal_Int16 nNumberingStartValue );

    virtual sal_Bool        IsParaIsNumberingRestart( sal_Int32 nPara );
    virtual void            SetParaIsNumberingRestart( sal_Int32 nPara, sal_Bool bParaIsNumberingRestart );

    /* this method flushes internal caches for this forwarder */
    void                flushCache();

    virtual const SfxItemSet*   GetEmptyItemSetPtr();

    // implementation functions for XParagraphAppend and XTextPortionAppend
    virtual void        AppendParagraph();
    virtual sal_uInt16  AppendTextPortion( sal_Int32 nPara, const OUString &rText, const SfxItemSet &rSet );
    //XTextCopy
    virtual void        CopyText(const SvxTextForwarder& rSource);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
