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

#ifndef INCLUDED_EDITENG_UNOEDPRX_HXX
#define INCLUDED_EDITENG_UNOEDPRX_HXX

#include <memory>
#include <svl/SfxBroadcaster.hxx>
#include <editeng/unoedsrc.hxx>

#include <editeng/editdata.hxx>
#include <editeng/editengdllapi.h>

class SvxAccessibleTextAdapter : public SvxTextForwarder
{
public:
    SvxAccessibleTextAdapter();
    virtual ~SvxAccessibleTextAdapter();

    virtual sal_Int32       GetParagraphCount() const SAL_OVERRIDE;
    virtual sal_Int32       GetTextLen( sal_Int32 nParagraph ) const SAL_OVERRIDE;
    virtual OUString        GetText( const ESelection& rSel ) const SAL_OVERRIDE;
    virtual SfxItemSet      GetAttribs( const ESelection& rSel, EditEngineAttribs nOnlyHardAttrib = EditEngineAttribs_All ) const SAL_OVERRIDE;
    virtual SfxItemSet      GetParaAttribs( sal_Int32 nPara ) const SAL_OVERRIDE;
    virtual void            SetParaAttribs( sal_Int32 nPara, const SfxItemSet& rSet ) SAL_OVERRIDE;
    virtual void            RemoveAttribs( const ESelection& rSelection, bool bRemoveParaAttribs, sal_uInt16 nWhich ) SAL_OVERRIDE;
    virtual void            GetPortions( sal_Int32 nPara, std::vector<sal_Int32>& rList ) const SAL_OVERRIDE;

    sal_uInt16              CalcEditEngineIndex( sal_Int32 nPara, sal_Int32 nLogicalIndex );

    virtual SfxItemState    GetItemState( const ESelection& rSel, sal_uInt16 nWhich ) const SAL_OVERRIDE;
    virtual SfxItemState    GetItemState( sal_Int32 nPara, sal_uInt16 nWhich ) const SAL_OVERRIDE;

    virtual void            QuickInsertText( const OUString& rText, const ESelection& rSel ) SAL_OVERRIDE;
    virtual void            QuickInsertField( const SvxFieldItem& rFld, const ESelection& rSel ) SAL_OVERRIDE;
    virtual void            QuickSetAttribs( const SfxItemSet& rSet, const ESelection& rSel ) SAL_OVERRIDE;
    virtual void            QuickInsertLineBreak( const ESelection& rSel ) SAL_OVERRIDE;

    virtual SfxItemPool*    GetPool() const SAL_OVERRIDE;

    virtual OUString        CalcFieldValue( const SvxFieldItem& rField, sal_Int32 nPara, sal_Int32 nPos, Color*& rpTxtColor, Color*& rpFldColor ) SAL_OVERRIDE;
    virtual void            FieldClicked( const SvxFieldItem& rField, sal_Int32 nPara, sal_Int32 nPos ) SAL_OVERRIDE;

    virtual bool            IsValid() const SAL_OVERRIDE;

    virtual LanguageType    GetLanguage( sal_Int32, sal_Int32 ) const SAL_OVERRIDE;
    virtual sal_Int32       GetFieldCount( sal_Int32 nPara ) const SAL_OVERRIDE;
    virtual EFieldInfo      GetFieldInfo( sal_Int32 nPara, sal_uInt16 nField ) const SAL_OVERRIDE;
    virtual EBulletInfo     GetBulletInfo( sal_Int32 nPara ) const SAL_OVERRIDE;
    virtual Rectangle       GetCharBounds( sal_Int32 nPara, sal_Int32 nIndex ) const SAL_OVERRIDE;
    virtual Rectangle       GetParaBounds( sal_Int32 nPara ) const SAL_OVERRIDE;
    virtual MapMode         GetMapMode() const SAL_OVERRIDE;
    virtual OutputDevice*   GetRefDevice() const SAL_OVERRIDE;
    virtual bool            GetIndexAtPoint( const Point&, sal_Int32& nPara, sal_Int32& nIndex ) const SAL_OVERRIDE;
    virtual bool            GetWordIndices( sal_Int32 nPara, sal_Int32 nIndex, sal_Int32& nStart, sal_Int32& nEnd ) const SAL_OVERRIDE;
    virtual bool            GetAttributeRun( sal_Int32& nStartIndex, sal_Int32& nEndIndex, sal_Int32 nPara, sal_Int32 nIndex, bool bInCell = false ) const SAL_OVERRIDE;
    virtual sal_Int32       GetLineCount( sal_Int32 nPara ) const SAL_OVERRIDE;
    virtual sal_Int32       GetLineLen( sal_Int32 nPara, sal_Int32 nLine ) const SAL_OVERRIDE;
    virtual void            SetUpdateModeForAcc( bool bUp) SAL_OVERRIDE;
    virtual bool            GetUpdateModeForAcc() const SAL_OVERRIDE;
    virtual void            GetLineBoundaries( /*out*/sal_Int32 &rStart, /*out*/sal_Int32 &rEnd, sal_Int32 nParagraph, sal_Int32 nLine ) const SAL_OVERRIDE;
    virtual sal_Int32       GetLineNumberAtIndex( sal_Int32 nPara, sal_Int32 nIndex ) const SAL_OVERRIDE;

    virtual bool            Delete( const ESelection& ) SAL_OVERRIDE;
    virtual bool            InsertText( const OUString&, const ESelection& ) SAL_OVERRIDE;
    virtual bool            QuickFormatDoc( bool bFull = false ) SAL_OVERRIDE;
    virtual sal_Int16       GetDepth( sal_Int32 nPara ) const SAL_OVERRIDE;
    virtual bool            SetDepth( sal_Int32 nPara, sal_Int16 nNewDepth ) SAL_OVERRIDE;

    virtual const SfxItemSet*   GetEmptyItemSetPtr() SAL_OVERRIDE;

    // implementation functions for XParagraphAppend and XTextPortionAppend
    // (not needed for accessibility, only for new import API)
    virtual void        AppendParagraph() SAL_OVERRIDE;
    virtual sal_Int32   AppendTextPortion( sal_Int32 nPara, const OUString &rText, const SfxItemSet &rSet ) SAL_OVERRIDE;

    //XTextCopy
    virtual void        CopyText(const SvxTextForwarder& rSource) SAL_OVERRIDE;

    void                SetForwarder( SvxTextForwarder& );
    bool                HaveImageBullet( sal_Int32 nPara ) const;
    bool                HaveTextBullet( sal_Int32 nPara ) const;

    /** Query whether all text in given selection is editable

        @return sal_True if every character in the given selection can
        be changed, and sal_False if e.g. a field or a bullet is
        contained therein.
     */
    bool                IsEditable( const ESelection& rSelection );

private:
    SvxTextForwarder* mpTextForwarder;
};

class SvxAccessibleTextEditViewAdapter : public SvxEditViewForwarder
{
public:

                        SvxAccessibleTextEditViewAdapter();
    virtual             ~SvxAccessibleTextEditViewAdapter();

    // SvxViewForwarder interface
    virtual bool        IsValid() const SAL_OVERRIDE;
    virtual Rectangle   GetVisArea() const SAL_OVERRIDE;
    virtual Point       LogicToPixel( const Point& rPoint, const MapMode& rMapMode ) const SAL_OVERRIDE;
    virtual Point       PixelToLogic( const Point& rPoint, const MapMode& rMapMode ) const SAL_OVERRIDE;

    // SvxEditViewForwarder interface
    virtual bool        GetSelection( ESelection& rSelection ) const SAL_OVERRIDE;
    virtual bool        SetSelection( const ESelection& rSelection ) SAL_OVERRIDE;
    virtual bool        Copy() SAL_OVERRIDE;
    virtual bool        Cut() SAL_OVERRIDE;
    virtual bool        Paste() SAL_OVERRIDE;

    void                SetForwarder( SvxEditViewForwarder&, SvxAccessibleTextAdapter& );

private:
    SvxEditViewForwarder*       mpViewForwarder;
    SvxAccessibleTextAdapter*   mpTextForwarder;
};

class EDITENG_DLLPUBLIC SvxEditSourceAdapter : public SvxEditSource
{
public:
    SvxEditSourceAdapter();
    virtual ~SvxEditSourceAdapter();

    virtual SvxEditSource*                      Clone() const SAL_OVERRIDE;
    virtual SvxTextForwarder*                   GetTextForwarder() SAL_OVERRIDE;
    SvxAccessibleTextAdapter*                   GetTextForwarderAdapter(); // covariant return types don't work on MSVC
    virtual SvxViewForwarder*                   GetViewForwarder() SAL_OVERRIDE;
    virtual SvxEditViewForwarder*               GetEditViewForwarder( bool bCreate = false ) SAL_OVERRIDE;
    SvxAccessibleTextEditViewAdapter*           GetEditViewForwarderAdapter( bool bCreate = false ); // covariant return types don't work on MSVC
    virtual void                                UpdateData() SAL_OVERRIDE;
    virtual SfxBroadcaster&                     GetBroadcaster() const SAL_OVERRIDE;

    void        SetEditSource( ::std::unique_ptr< SvxEditSource > && pAdaptee );

    bool        IsValid() const { return mbEditSourceValid;}

private:
    SvxEditSourceAdapter( const SvxEditSourceAdapter& ) = delete;
    SvxEditSourceAdapter& operator= ( const SvxEditSourceAdapter& ) = delete;

    ::std::unique_ptr< SvxEditSource >    mpAdaptee;

    SvxAccessibleTextAdapter            maTextAdapter;
    SvxAccessibleTextEditViewAdapter    maEditViewAdapter;

    mutable SfxBroadcaster              maDummyBroadcaster;

    bool                                mbEditSourceValid;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
