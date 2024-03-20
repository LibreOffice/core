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

#ifndef INCLUDED_EDITENG_UNOFOROU_HXX
#define INCLUDED_EDITENG_UNOFOROU_HXX

#include <editeng/unoedsrc.hxx>
#include <editeng/editengdllapi.h>
#include <editeng/editdata.hxx>
#include <svl/itemset.hxx>
#include <optional>

class Outliner;

//  SvxOutlinerForwarder - SvxTextForwarder for Outliner

class EDITENG_DLLPUBLIC SvxOutlinerForwarder : public SvxTextForwarder
{
private:
    Outliner&           rOutliner;
    bool                bOutlinerText;

    /** this pointer may be null or point to an item set for the attribs of
        the selection maAttribsSelection */
    mutable std::optional<SfxItemSet> moAttribsCache;

    /** if we have a cached attribute item set, this is the selection of it */
    mutable ESelection  maAttribCacheSelection;

    /** this pointer may be null or point to an item set for the paragraph
        mnParaAttribsCache */
    mutable std::optional<SfxItemSet> moParaAttribsCache;

    /** if we have a cached para attribute item set, this is the paragraph of it */
    mutable sal_Int32   mnParaAttribsCache;

public:
                        SvxOutlinerForwarder( Outliner& rOutl, bool bOutlText );
    virtual             ~SvxOutlinerForwarder() override;

    virtual sal_Int32   GetParagraphCount() const override final;
    virtual sal_Int32   GetTextLen( sal_Int32 nParagraph ) const override final;
    virtual OUString    GetText( const ESelection& rSel ) const override final;
    virtual SfxItemSet  GetAttribs( const ESelection& rSel, EditEngineAttribs nOnlyHardAttrib = EditEngineAttribs::All ) const override final;
    virtual SfxItemSet  GetParaAttribs( sal_Int32 nPara ) const override final;
    virtual void        SetParaAttribs( sal_Int32 nPara, const SfxItemSet& rSet ) override final;
    virtual void        RemoveAttribs( const ESelection& rSelection ) override final;
    virtual void        GetPortions( sal_Int32 nPara, std::vector<sal_Int32>& rList ) const override final;

    virtual OUString    GetStyleSheet(sal_Int32 nPara) const override;
    virtual void        SetStyleSheet(sal_Int32 nPara, const OUString& rStyleName) override;

    virtual SfxItemState    GetItemState( const ESelection& rSel, sal_uInt16 nWhich ) const override final;
    virtual SfxItemState    GetItemState( sal_Int32 nPara, sal_uInt16 nWhich ) const override final;

    virtual void        QuickInsertText( const OUString& rText, const ESelection& rSel ) override final;
    virtual void        QuickInsertField( const SvxFieldItem& rFld, const ESelection& rSel ) override final;
    virtual void        QuickSetAttribs( const SfxItemSet& rSet, const ESelection& rSel ) override final;
    virtual void        QuickInsertLineBreak( const ESelection& rSel ) override final;

    virtual SfxItemPool* GetPool() const override final;

    virtual OUString    CalcFieldValue( const SvxFieldItem& rField, sal_Int32 nPara, sal_Int32 nPos, std::optional<Color>& rpTxtColor, std::optional<Color>& rpFldColor, std::optional<FontLineStyle>& rpFldLineStyle ) override final;
    virtual void        FieldClicked( const SvxFieldItem& rField ) override final;

    virtual bool        IsValid() const override final;

    virtual LanguageType    GetLanguage( sal_Int32, sal_Int32 ) const override final;
    virtual sal_Int32       GetFieldCount( sal_Int32 nPara ) const override final;
    virtual EFieldInfo      GetFieldInfo( sal_Int32 nPara, sal_uInt16 nField ) const override final;
    virtual EBulletInfo     GetBulletInfo( sal_Int32 nPara ) const override final;
    virtual tools::Rectangle       GetCharBounds( sal_Int32 nPara, sal_Int32 nIndex ) const override final;
    virtual tools::Rectangle       GetParaBounds( sal_Int32 nPara ) const override final;
    virtual MapMode         GetMapMode() const override final;
    virtual OutputDevice*   GetRefDevice() const override final;
    virtual bool            GetIndexAtPoint( const Point&, sal_Int32& nPara, sal_Int32& nIndex ) const override final;
    virtual bool            GetWordIndices( sal_Int32 nPara, sal_Int32 nIndex, sal_Int32& nStart, sal_Int32& nEnd ) const override final;
    virtual bool            GetAttributeRun( sal_Int32& nStartIndex, sal_Int32& nEndIndex, sal_Int32 nPara, sal_Int32 nIndex, bool bInCell = false ) const override final;
    virtual sal_Int32       GetLineCount( sal_Int32 nPara ) const override final;
    virtual sal_Int32       GetLineLen( sal_Int32 nPara, sal_Int32 nLine ) const override final;
    virtual void            GetLineBoundaries( /*out*/sal_Int32& rStart, /*out*/sal_Int32& rEnd, sal_Int32 nPara, sal_Int32 nLine ) const override final;
    virtual sal_Int32       GetLineNumberAtIndex( sal_Int32 nPara, sal_Int32 nIndex ) const override final;
    virtual bool            Delete( const ESelection& ) override final;
    virtual bool            InsertText( const OUString&, const ESelection& ) override final;
    virtual bool            QuickFormatDoc( bool bFull = false ) override final;
    virtual sal_Int16       GetDepth( sal_Int32 nPara ) const override final;
    virtual bool            SetDepth( sal_Int32 nPara, sal_Int16 nNewDepth ) override final;
    virtual sal_Int32       GetNumberingStartValue( sal_Int32 nPara ) override final;
    virtual void            SetNumberingStartValue( sal_Int32 nPara, sal_Int32 nNumberingStartValue ) override final;

    virtual bool            IsParaIsNumberingRestart( sal_Int32 nPara ) override final;
    virtual void            SetParaIsNumberingRestart( sal_Int32 nPara, bool bParaIsNumberingRestart ) override final;

    /* this method flushes internal caches for this forwarder */
    void                flushCache();

    virtual const SfxItemSet*   GetEmptyItemSetPtr() override final;

    // implementation functions for XParagraphAppend and XTextPortionAppend
    virtual void        AppendParagraph() override final;
    virtual sal_Int32   AppendTextPortion( sal_Int32 nPara, const OUString &rText, const SfxItemSet &rSet ) override final;
    //XTextCopy
    virtual void        CopyText(const SvxTextForwarder& rSource) override final;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
