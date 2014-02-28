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

#ifndef INCLUDED_EDITENG_UNOFORED_HXX
#define INCLUDED_EDITENG_UNOFORED_HXX

#include <editeng/editeng.hxx>
#include <editeng/unoedsrc.hxx>
#include <editeng/editengdllapi.h>

//  SvxEditEngineForwarder - SvxTextForwarder for EditEngine

class EDITENG_DLLPUBLIC SvxEditEngineForwarder : public SvxTextForwarder
{
private:
    EditEngine&         rEditEngine;

public:
                        SvxEditEngineForwarder( EditEngine& rEngine );
    virtual             ~SvxEditEngineForwarder();

    virtual sal_Int32   GetParagraphCount() const;
    virtual sal_Int32   GetTextLen( sal_Int32 nParagraph ) const;
    virtual OUString    GetText( const ESelection& rSel ) const;
    virtual SfxItemSet  GetAttribs( const ESelection& rSel, sal_Bool bOnlyHardAttrib = EditEngineAttribs_All ) const;
    virtual SfxItemSet  GetParaAttribs( sal_Int32 nPara ) const;
    virtual void        SetParaAttribs( sal_Int32 nPara, const SfxItemSet& rSet );
    virtual void        RemoveAttribs( const ESelection& rSelection, bool bRemoveParaAttribs, sal_uInt16 nWhich );
    virtual void        GetPortions( sal_Int32 nPara, std::vector<sal_Int32>& rList ) const;

    virtual sal_uInt16      GetItemState( const ESelection& rSel, sal_uInt16 nWhich ) const;
    virtual sal_uInt16      GetItemState( sal_Int32 nPara, sal_uInt16 nWhich ) const;

    virtual void        QuickInsertText( const OUString& rText, const ESelection& rSel );
    virtual void        QuickInsertField( const SvxFieldItem& rFld, const ESelection& rSel );
    virtual void        QuickSetAttribs( const SfxItemSet& rSet, const ESelection& rSel );
    virtual void        QuickInsertLineBreak( const ESelection& rSel );

    virtual SfxItemPool* GetPool() const;

    virtual OUString     CalcFieldValue( const SvxFieldItem& rField, sal_Int32 nPara, sal_Int32 nPos, Color*& rpTxtColor, Color*& rpFldColor );
    virtual void         FieldClicked( const SvxFieldItem& rField, sal_Int32 nPara, sal_Int32 nPos );
    virtual bool         IsValid() const;

    virtual LanguageType    GetLanguage( sal_Int32, sal_Int32 ) const;
    virtual sal_Int32       GetFieldCount( sal_Int32 nPara ) const;
    virtual EFieldInfo      GetFieldInfo( sal_Int32 nPara, sal_uInt16 nField ) const;
    virtual EBulletInfo     GetBulletInfo( sal_Int32 nPara ) const;
    virtual Rectangle       GetCharBounds( sal_Int32 nPara, sal_Int32 nIndex ) const;
    virtual Rectangle       GetParaBounds( sal_Int32 nPara ) const;
    virtual MapMode         GetMapMode() const;
    virtual OutputDevice*   GetRefDevice() const;
    virtual bool            GetIndexAtPoint( const Point&, sal_Int32& nPara, sal_Int32& nIndex ) const;
    virtual bool            GetWordIndices( sal_Int32 nPara, sal_Int32 nIndex, sal_Int32& nStart, sal_Int32& nEnd ) const;
    virtual bool            GetAttributeRun( sal_Int32& nStartIndex, sal_Int32& nEndIndex, sal_Int32 nPara, sal_Int32 nIndex, bool bInCell = false ) const;
    virtual sal_Int32       GetLineCount( sal_Int32 nPara ) const;
    virtual sal_Int32       GetLineLen( sal_Int32 nPara, sal_Int32 nLine ) const;
    virtual void            GetLineBoundaries( /*out*/sal_Int32 &rStart, /*out*/sal_Int32 &rEnd, sal_Int32 nParagraph, sal_Int32 nLine ) const;
    virtual sal_Int32       GetLineNumberAtIndex( sal_Int32 nPara, sal_Int32 nIndex ) const;
    virtual bool            Delete( const ESelection& );
    virtual bool            InsertText( const OUString&, const ESelection& );
    virtual bool            QuickFormatDoc( bool bFull = false );
    virtual sal_Int16       GetDepth( sal_Int32 nPara ) const;
    virtual bool            SetDepth( sal_Int32 nPara, sal_Int16 nNewDepth );

    virtual const SfxItemSet*   GetEmptyItemSetPtr();

    // implementation functions for XParagraphAppend and XTextPortionAppend
    virtual void        AppendParagraph();
    virtual sal_Int32  AppendTextPortion( sal_Int32 nPara, const OUString &rText, const SfxItemSet &rSet );
    //XTextCopy
    virtual void        CopyText(const SvxTextForwarder& rSource);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
