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
#ifndef INCLUDED_CUI_SOURCE_FACTORY_DLGFACT_HXX
#define INCLUDED_CUI_SOURCE_FACTORY_DLGFACT_HXX

#include <svx/svxdlg.hxx>
#include <svx/zoom_def.hxx>
#include <com/sun/star/container/XNameReplace.hpp>

#include "tools/link.hxx"
#include <com/sun/star/frame/XFrame.hpp>

class SfxTabDialog;
class SfxModalDialog;
class Dialog;
class SfxItemPool;
class FmShowColsDialog;
class SvxZoomDialog;
class FmInputRecordNoDialog;
class SvxJSearchOptionsDialog;
class SvxNewDictionaryDialog;
class SvxNameDialog;

// #i68101#
class SvxObjectNameDialog;
class SvxObjectTitleDescDialog;

class SvxMessDialog;
class SvxMultiPathDialog;
class SvxHpLinkDlg;
class FmSearchDialog;
class Graphic;
class GraphicFilterDialog;
class SvxAreaTabDialog;
class InsertObjectDialog_Impl;
class SvPasteObjectDialog;
class SvBaseLinksDlg;
class SvxTransformTabDialog;
class SvxCaptionTabDialog;
class SvxThesaurusDialog;
class SvxHyphenWordDialog;

namespace svx{
class HangulHanjaConversionDialog;
}
using namespace svx;

#define DECL_ABSTDLG_BASE(Class,DialogClass)        \
    ScopedVclPtr<DialogClass> pDlg;                 \
public:                                             \
    explicit        Class( DialogClass* p)          \
                     : pDlg(p)                      \
                     {}                             \
    virtual short   Execute() SAL_OVERRIDE ;

#define IMPL_ABSTDLG_BASE(Class)                    \
short Class::Execute()                              \
{                                                   \
    return pDlg->Execute();                         \
}

class VclAbstractDialog2_Impl : public VclAbstractDialog2
{
    ScopedVclPtr<Dialog> m_pDlg;
    Link<>          m_aEndDlgHdl;
public:
    explicit        VclAbstractDialog2_Impl( Dialog* p ) : m_pDlg( p ) {}
    virtual         ~VclAbstractDialog2_Impl();
    virtual void    StartExecuteModal( const Link<>& rEndDialogHdl ) SAL_OVERRIDE;
    virtual long    GetResult() SAL_OVERRIDE;
private:
                    DECL_LINK( EndDialogHdl, Dialog* );
};

class CuiVclAbstractDialog_Impl : public VclAbstractDialog
{
    DECL_ABSTDLG_BASE(CuiVclAbstractDialog_Impl,Dialog)
};

class VclAbstractRefreshableDialog_Impl : public VclAbstractRefreshableDialog
{
    DECL_ABSTDLG_BASE(VclAbstractRefreshableDialog_Impl,Dialog)
    virtual void        Update() SAL_OVERRIDE ;
};

class CuiAbstractSfxDialog_Impl : public SfxAbstractDialog
{
    DECL_ABSTDLG_BASE(CuiAbstractSfxDialog_Impl,SfxModalDialog)
    virtual const SfxItemSet*   GetOutputItemSet() const SAL_OVERRIDE;

        //From class Window.
    virtual void          SetText( const OUString& rStr ) SAL_OVERRIDE;
    virtual OUString      GetText() const SAL_OVERRIDE ;
};

class CuiAbstractTabDialog_Impl : public SfxAbstractTabDialog
{
    DECL_ABSTDLG_BASE(CuiAbstractTabDialog_Impl,SfxTabDialog)
    virtual void                SetCurPageId( sal_uInt16 nId ) SAL_OVERRIDE;
    virtual void                SetCurPageId( const OString& rName ) SAL_OVERRIDE;
    virtual const SfxItemSet*   GetOutputItemSet() const SAL_OVERRIDE;
    virtual const sal_uInt16*       GetInputRanges( const SfxItemPool& pItem ) SAL_OVERRIDE;
    virtual void                SetInputSet( const SfxItemSet* pInSet ) SAL_OVERRIDE;
        //From class Window.
    virtual void        SetText( const OUString& rStr ) SAL_OVERRIDE;
    virtual OUString    GetText() const SAL_OVERRIDE;
};

class SvxDistributeDialog;
class AbstractSvxDistributeDialog_Impl: public AbstractSvxDistributeDialog
{
    DECL_ABSTDLG_BASE(AbstractSvxDistributeDialog_Impl,SvxDistributeDialog)
public:
    virtual SvxDistributeHorizontal GetDistributeHor() const SAL_OVERRIDE;
    virtual SvxDistributeVertical GetDistributeVer() const SAL_OVERRIDE;
};

class AbstractHangulHanjaConversionDialog_Impl: public AbstractHangulHanjaConversionDialog
{
    DECL_ABSTDLG_BASE(AbstractHangulHanjaConversionDialog_Impl,HangulHanjaConversionDialog)
    virtual void      EndDialog(long nResult = 0) SAL_OVERRIDE;
    virtual void      EnableRubySupport( bool _bVal ) SAL_OVERRIDE;
    virtual void      SetByCharacter( bool _bByCharacter ) SAL_OVERRIDE ;
    virtual void      SetConversionDirectionState( bool _bTryBothDirections, editeng::HangulHanjaConversion::ConversionDirection _ePrimaryConversionDirection ) SAL_OVERRIDE;
    virtual void      SetConversionFormat( editeng::HangulHanjaConversion::ConversionFormat _eType ) SAL_OVERRIDE;
    virtual void      SetOptionsChangedHdl( const Link<LinkParamNone*,void>& _rHdl ) SAL_OVERRIDE;
    virtual void      SetIgnoreHdl( const Link<Button*,void>& _rHdl ) SAL_OVERRIDE;
    virtual void      SetIgnoreAllHdl( const Link<Button*,void>& _rHdl ) SAL_OVERRIDE ;
    virtual void      SetChangeHdl( const Link<Button*,void>& _rHdl ) SAL_OVERRIDE ;
    virtual void      SetChangeAllHdl( const Link<Button*,void>& _rHdl ) SAL_OVERRIDE ;
    virtual void      SetClickByCharacterHdl( const Link<>& _rHdl ) SAL_OVERRIDE ;
    virtual void      SetConversionFormatChangedHdl( const Link<Button*,void>& _rHdl ) SAL_OVERRIDE ;
    virtual void      SetFindHdl( const Link<Button*,void>& _rHdl ) SAL_OVERRIDE;
    virtual bool      GetUseBothDirections( ) const SAL_OVERRIDE;
    virtual editeng::HangulHanjaConversion::ConversionDirection
                      GetDirection( editeng::HangulHanjaConversion::ConversionDirection _eDefaultDirection ) const SAL_OVERRIDE;
    virtual void      SetCurrentString(
                        const OUString& _rNewString,
                        const css::uno::Sequence< OUString >& _rSuggestions,
                        bool _bOriginatesFromDocument = true
                      ) SAL_OVERRIDE;
    virtual OUString  GetCurrentString( ) const SAL_OVERRIDE ;
    virtual editeng::HangulHanjaConversion::ConversionFormat
                      GetConversionFormat( ) const SAL_OVERRIDE ;
    virtual void      FocusSuggestion( ) SAL_OVERRIDE;
    virtual OUString  GetCurrentSuggestion( ) const SAL_OVERRIDE;
};

class AbstractThesaurusDialog_Impl : public AbstractThesaurusDialog
{
    DECL_ABSTDLG_BASE(AbstractThesaurusDialog_Impl,SvxThesaurusDialog)
    virtual OUString    GetWord() SAL_OVERRIDE;
};

class AbstractHyphenWordDialog_Impl: public AbstractHyphenWordDialog
{
    DECL_ABSTDLG_BASE(AbstractHyphenWordDialog_Impl,SvxHyphenWordDialog)
    virtual vcl::Window* GetWindow() SAL_OVERRIDE;
};

class FmShowColsDialog;
class AbstractFmShowColsDialog_Impl : public AbstractFmShowColsDialog
{
    DECL_ABSTDLG_BASE(AbstractFmShowColsDialog_Impl,FmShowColsDialog)
     virtual void   SetColumns(const css::uno::Reference< css::container::XIndexContainer>& xCols) SAL_OVERRIDE;
};

class SvxZoomDialog;
class AbstractSvxZoomDialog_Impl : public AbstractSvxZoomDialog
{
    DECL_ABSTDLG_BASE(AbstractSvxZoomDialog_Impl,SvxZoomDialog)
    virtual void    SetLimits( sal_uInt16 nMin, sal_uInt16 nMax ) SAL_OVERRIDE;
    virtual void    HideButton( ZoomButtonId nBtnId ) SAL_OVERRIDE;
    virtual const SfxItemSet*   GetOutputItemSet() const SAL_OVERRIDE ;

};

namespace svx{ class SpellDialog;}
class AbstractSpellDialog_Impl : public AbstractSpellDialog
{
 public:
    DECL_ABSTDLG_BASE(AbstractSpellDialog_Impl, svx::SpellDialog)
    virtual void        Invalidate() SAL_OVERRIDE;
    virtual vcl::Window*     GetWindow() SAL_OVERRIDE;
    virtual SfxBindings& GetBindings() SAL_OVERRIDE;
};

class SearchProgress;
class AbstractSearchProgress_Impl : public AbstractSearchProgress
{
    DECL_ABSTDLG_BASE(AbstractSearchProgress_Impl,SearchProgress)
    virtual void        Update() SAL_OVERRIDE ;

};

class TakeProgress;
class AbstractTakeProgress_Impl : public AbstractTakeProgress
{
    DECL_ABSTDLG_BASE(AbstractTakeProgress_Impl,TakeProgress)
    virtual void        Update() SAL_OVERRIDE ;

};

class TitleDialog;
class AbstractTitleDialog_Impl : public AbstractTitleDialog
{
    DECL_ABSTDLG_BASE(AbstractTitleDialog_Impl,TitleDialog)
    virtual OUString  GetTitle() const SAL_OVERRIDE ;

};

class SvxScriptSelectorDialog;
class AbstractScriptSelectorDialog_Impl : public AbstractScriptSelectorDialog
{
    DECL_ABSTDLG_BASE(
        AbstractScriptSelectorDialog_Impl, SvxScriptSelectorDialog)

    virtual OUString GetScriptURL() const SAL_OVERRIDE;

    virtual void SetRunLabel() SAL_OVERRIDE;
};

class GalleryIdDialog;
class AbstractGalleryIdDialog_Impl : public AbstractGalleryIdDialog
{
    DECL_ABSTDLG_BASE(AbstractGalleryIdDialog_Impl,GalleryIdDialog)
    virtual sal_uLong   GetId() const SAL_OVERRIDE ;

};

class URLDlg;
class AbstractURLDlg_Impl :public AbstractURLDlg
{
    DECL_ABSTDLG_BASE(AbstractURLDlg_Impl,URLDlg)
    virtual OUString      GetURL() const SAL_OVERRIDE;
    virtual OUString      GetAltText() const SAL_OVERRIDE;
    virtual OUString      GetDesc() const SAL_OVERRIDE;
    virtual OUString      GetTarget() const SAL_OVERRIDE;
    virtual OUString      GetName() const SAL_OVERRIDE;
};

class SvxHlinkDlgMarkWnd;
class AbstractSvxHlinkDlgMarkWnd_Impl : public AbstractSvxHlinkDlgMarkWnd
{
    DECL_ABSTDLG_BASE(AbstractSvxHlinkDlgMarkWnd_Impl,SvxHlinkDlgMarkWnd)

};

class SvxSearchSimilarityDialog;
class AbstractSvxSearchSimilarityDialog_Impl :public AbstractSvxSearchSimilarityDialog
{
    DECL_ABSTDLG_BASE(AbstractSvxSearchSimilarityDialog_Impl,SvxSearchSimilarityDialog)
    virtual sal_uInt16              GetOther() SAL_OVERRIDE;
    virtual sal_uInt16              GetShorter() SAL_OVERRIDE;
    virtual sal_uInt16              GetLonger() SAL_OVERRIDE;
    virtual bool                    IsRelaxed() SAL_OVERRIDE;
};

class SvxJSearchOptionsDialog;
class AbstractSvxJSearchOptionsDialog_Impl :public AbstractSvxJSearchOptionsDialog
{
    DECL_ABSTDLG_BASE(AbstractSvxJSearchOptionsDialog_Impl,SvxJSearchOptionsDialog)
    virtual sal_Int32           GetTransliterationFlags() const SAL_OVERRIDE;
};

class AbstractSvxTransformTabDialog_Impl : public AbstractSvxTransformTabDialog
{
    DECL_ABSTDLG_BASE(AbstractSvxTransformTabDialog_Impl,SvxTransformTabDialog)
    virtual void SetValidateFramePosLink( const Link<>& rLink ) SAL_OVERRIDE;
    virtual void                SetCurPageId( sal_uInt16 nId ) SAL_OVERRIDE;
    virtual void                SetCurPageId( const OString& rName ) SAL_OVERRIDE;
    virtual const SfxItemSet*   GetOutputItemSet() const SAL_OVERRIDE;
    virtual const sal_uInt16*       GetInputRanges( const SfxItemPool& pItem ) SAL_OVERRIDE;
    virtual void                SetInputSet( const SfxItemSet* pInSet ) SAL_OVERRIDE;
    virtual void        SetText( const OUString& rStr ) SAL_OVERRIDE;
    virtual OUString    GetText() const SAL_OVERRIDE;
};

class AbstractSvxCaptionDialog_Impl : public AbstractSvxCaptionDialog
{
    DECL_ABSTDLG_BASE(AbstractSvxCaptionDialog_Impl,SvxCaptionTabDialog)
    virtual void SetValidateFramePosLink( const Link<>& rLink ) SAL_OVERRIDE;
    virtual void                SetCurPageId( sal_uInt16 nId ) SAL_OVERRIDE;
    virtual void                SetCurPageId( const OString& rName ) SAL_OVERRIDE;
    virtual const SfxItemSet*   GetOutputItemSet() const SAL_OVERRIDE;
    virtual const sal_uInt16*       GetInputRanges( const SfxItemPool& pItem ) SAL_OVERRIDE;
    virtual void                SetInputSet( const SfxItemSet* pInSet ) SAL_OVERRIDE;
    virtual void        SetText( const OUString& rStr ) SAL_OVERRIDE;
    virtual OUString    GetText() const SAL_OVERRIDE;
};

class FmInputRecordNoDialog;
class AbstractFmInputRecordNoDialog_Impl :public AbstractFmInputRecordNoDialog
{
    DECL_ABSTDLG_BASE(AbstractFmInputRecordNoDialog_Impl,FmInputRecordNoDialog)
    virtual void SetValue(long nNew) SAL_OVERRIDE ;
    virtual long GetValue() const SAL_OVERRIDE ;
};

class SvxNewDictionaryDialog;
class AbstractSvxNewDictionaryDialog_Impl :public AbstractSvxNewDictionaryDialog
{
    DECL_ABSTDLG_BASE(AbstractSvxNewDictionaryDialog_Impl,SvxNewDictionaryDialog)
    virtual css::uno::Reference< css::linguistic2::XDictionary >  GetNewDictionary() SAL_OVERRIDE;
};

class SvxNameDialog;
class AbstractSvxNameDialog_Impl :public AbstractSvxNameDialog
{
    DECL_ABSTDLG_BASE(AbstractSvxNameDialog_Impl,SvxNameDialog)
    virtual void    GetName( OUString& rName ) SAL_OVERRIDE ;
    virtual void    SetCheckNameHdl( const Link<>& rLink, bool bCheckImmediately = false ) SAL_OVERRIDE ;
    virtual void    SetEditHelpId(const OString&) SAL_OVERRIDE ;
    //from class Window
    virtual void    SetHelpId( const OString& ) SAL_OVERRIDE ;
    virtual void    SetText( const OUString& rStr ) SAL_OVERRIDE ;
private:
    Link<> aCheckNameHdl;
    DECL_LINK_TYPED(CheckNameHdl, SvxNameDialog&, bool);
};

class SvxObjectNameDialog;
class SvxObjectTitleDescDialog;

class AbstractSvxObjectNameDialog_Impl :public AbstractSvxObjectNameDialog
{
    DECL_ABSTDLG_BASE(AbstractSvxObjectNameDialog_Impl, SvxObjectNameDialog)
    virtual void GetName(OUString& rName) SAL_OVERRIDE ;
    virtual void SetCheckNameHdl(const Link<>& rLink, bool bCheckImmediately = false) SAL_OVERRIDE;

private:
    Link<> aCheckNameHdl;
    DECL_LINK_TYPED(CheckNameHdl, SvxObjectNameDialog&, bool);
};

class AbstractSvxObjectTitleDescDialog_Impl :public AbstractSvxObjectTitleDescDialog
{
    DECL_ABSTDLG_BASE(AbstractSvxObjectTitleDescDialog_Impl, SvxObjectTitleDescDialog)
    virtual void GetTitle(OUString& rName) SAL_OVERRIDE;
    virtual void GetDescription(OUString& rName) SAL_OVERRIDE;
};

class SvxMessDialog;
class AbstractSvxMessDialog_Impl :public AbstractSvxMessDialog
{
    DECL_ABSTDLG_BASE(AbstractSvxMessDialog_Impl,SvxMessDialog)
};

class SvxMultiPathDialog;
class AbstractSvxMultiPathDialog_Impl : public AbstractSvxMultiPathDialog
{
    DECL_ABSTDLG_BASE(AbstractSvxMultiPathDialog_Impl,SvxMultiPathDialog)
    virtual OUString        GetPath() const SAL_OVERRIDE;
    virtual void            SetPath( const OUString& rPath ) SAL_OVERRIDE;
    virtual void            SetTitle( const OUString& rNewTitle ) SAL_OVERRIDE;
};

class SvxPathSelectDialog;
class AbstractSvxPathSelectDialog_Impl : public AbstractSvxMultiPathDialog
{
    DECL_ABSTDLG_BASE(AbstractSvxPathSelectDialog_Impl,SvxPathSelectDialog)
    virtual OUString        GetPath() const SAL_OVERRIDE;
    virtual void            SetPath( const OUString& rPath ) SAL_OVERRIDE;
    virtual void            SetTitle( const OUString& rNewTitle ) SAL_OVERRIDE;
};

class SvxHpLinkDlg;
class AbstractSvxHpLinkDlg_Impl :public AbstractSvxHpLinkDlg
{
    DECL_ABSTDLG_BASE(AbstractSvxHpLinkDlg_Impl,SvxHpLinkDlg)
    virtual vcl::Window*     GetWindow() SAL_OVERRIDE;
    virtual bool        QueryClose() SAL_OVERRIDE;
};

class FmSearchDialog;
class AbstractFmSearchDialog_Impl :public AbstractFmSearchDialog
{
    DECL_ABSTDLG_BASE(AbstractFmSearchDialog_Impl,FmSearchDialog)
    virtual void SetFoundHandler(const Link<>& lnk) SAL_OVERRIDE ;
    virtual void SetCanceledNotFoundHdl(const Link<>& lnk) SAL_OVERRIDE;
    virtual void SetActiveField(const OUString& strField) SAL_OVERRIDE;
};

class AbstractGraphicFilterDialog_Impl :public AbstractGraphicFilterDialog
{
    DECL_ABSTDLG_BASE(AbstractGraphicFilterDialog_Impl,GraphicFilterDialog)
    virtual Graphic GetFilteredGraphic( const Graphic& rGraphic, double fScaleX, double fScaleY ) SAL_OVERRIDE;
};

class SvxAreaTabDialog;
class AbstractSvxAreaTabDialog_Impl :public AbstractSvxAreaTabDialog
{
    DECL_ABSTDLG_BASE(AbstractSvxAreaTabDialog_Impl,SvxAreaTabDialog)
    virtual void                SetCurPageId( sal_uInt16 nId ) SAL_OVERRIDE;
    virtual void                SetCurPageId( const OString& rName ) SAL_OVERRIDE;
    virtual const SfxItemSet*   GetOutputItemSet() const SAL_OVERRIDE;
    virtual const sal_uInt16*       GetInputRanges( const SfxItemPool& pItem ) SAL_OVERRIDE;
    virtual void                SetInputSet( const SfxItemSet* pInSet ) SAL_OVERRIDE;
    // From class Window.
    virtual void        SetText( const OUString& rStr ) SAL_OVERRIDE;
    virtual OUString    GetText() const SAL_OVERRIDE;
};

class AbstractInsertObjectDialog_Impl : public SfxAbstractInsertObjectDialog
{
    DECL_ABSTDLG_BASE(AbstractInsertObjectDialog_Impl, InsertObjectDialog_Impl)
    virtual css::uno::Reference < css::embed::XEmbeddedObject > GetObject() SAL_OVERRIDE;
    virtual css::uno::Reference< css::io::XInputStream > GetIconIfIconified( OUString* pGraphicMediaType ) SAL_OVERRIDE;
    virtual bool IsCreateNew() SAL_OVERRIDE;
};

class AbstractPasteDialog_Impl : public SfxAbstractPasteDialog
{
public:
    DECL_ABSTDLG_BASE(AbstractPasteDialog_Impl, SvPasteObjectDialog )
    virtual void Insert( SotClipboardFormatId nFormat, const OUString & rFormatName ) SAL_OVERRIDE;
    virtual void SetObjName( const SvGlobalName & rClass, const OUString & rObjName ) SAL_OVERRIDE;
    virtual SotClipboardFormatId GetFormat( const TransferableDataHelper& aHelper,
                        const DataFlavorExVector* pFormats=0,
                        const TransferableObjectDescriptor* pDesc=0 ) SAL_OVERRIDE;
};

class AbstractLinksDialog_Impl : public SfxAbstractLinksDialog
{
public:
    DECL_ABSTDLG_BASE(AbstractLinksDialog_Impl, SvBaseLinksDlg )
};

class SvxPostItDialog;
class AbstractSvxPostItDialog_Impl :public AbstractSvxPostItDialog
{
    DECL_ABSTDLG_BASE( AbstractSvxPostItDialog_Impl, SvxPostItDialog )
    virtual void                SetText( const OUString& rStr ) SAL_OVERRIDE;  //From class Window
    virtual const SfxItemSet*   GetOutputItemSet() const SAL_OVERRIDE;
    virtual void                SetPrevHdl( const Link<>& rLink ) SAL_OVERRIDE ;
    virtual void                SetNextHdl( const Link<>& rLink ) SAL_OVERRIDE ;
    virtual void                EnableTravel(bool bNext, bool bPrev) SAL_OVERRIDE ;
    virtual OUString            GetNote() SAL_OVERRIDE ;
    virtual void                SetNote(const OUString& rTxt) SAL_OVERRIDE ;
    virtual void                ShowLastAuthor(const OUString& rAuthor, const OUString& rDate) SAL_OVERRIDE ;
    virtual void                DontChangeAuthor() SAL_OVERRIDE ;
    virtual void                HideAuthor() SAL_OVERRIDE ;
    virtual vcl::Window *            GetWindow() SAL_OVERRIDE;
private:
    Link<> aNextHdl;
    Link<> aPrevHdl;
    DECL_LINK(NextHdl, void *);
    DECL_LINK(PrevHdl, void *);
};

class PasswordToOpenModifyDialog;
class AbstractPasswordToOpenModifyDialog_Impl : public AbstractPasswordToOpenModifyDialog
{
    DECL_ABSTDLG_BASE( AbstractPasswordToOpenModifyDialog_Impl, PasswordToOpenModifyDialog )

    virtual OUString  GetPasswordToOpen() const SAL_OVERRIDE;
    virtual OUString  GetPasswordToModify() const SAL_OVERRIDE;
    virtual bool      IsRecommendToOpenReadonly() const SAL_OVERRIDE;
};

//AbstractDialogFactory_Impl implementations
class AbstractDialogFactory_Impl : public SvxAbstractDialogFactory
{
public:
    virtual VclAbstractDialog*          CreateVclDialog( vcl::Window* pParent, sal_uInt32 nResId ) SAL_OVERRIDE;

    virtual SfxAbstractDialog*          CreateSfxDialog( vcl::Window* pParent,
                                            const SfxItemSet& rAttr,
                                            const SdrView* pView,
                                            sal_uInt32 nResId ) SAL_OVERRIDE;
    virtual SfxAbstractDialog*          CreateSfxDialog( vcl::Window* pParent,
                                                             const SfxItemSet& rAttr,
                                                             const css::uno::Reference< css::frame::XFrame >& _rxFrame,
                                                             sal_uInt32 nResId ) SAL_OVERRIDE;
    virtual VclAbstractDialog*          CreateFrameDialog( vcl::Window* pParent,
                                                           const css::uno::Reference< css::frame::XFrame >& rxFrame,
                                                           sal_uInt32 nResId,
                                                           const OUString& rParameter ) SAL_OVERRIDE;
    virtual SfxAbstractTabDialog*       CreateTabDialog( sal_uInt32 nResId,
                                            vcl::Window* pParent,
                                            const SfxItemSet* pAttrSet,
                                            SfxViewFrame* pViewFrame,
                                            bool bEditFmt=false,
                                            const OUString *pUserButtonText=0 ) SAL_OVERRIDE;
    virtual SfxAbstractTabDialog*       CreateTabDialog( sal_uInt32 nResId,
                                            vcl::Window* pParent,
                                            const SfxItemSet* pAttrSet,
                                            const css::uno::Reference< css::frame::XFrame >& xViewFrame,
                                            bool bEditFmt=false,
                                            const OUString *pUserButtonText=0 ) SAL_OVERRIDE;
    virtual SfxAbstractTabDialog*       CreateTextTabDialog( vcl::Window* pParent,
                                            const SfxItemSet* pAttrSet,
                                            SdrView* pView,
                                            SdrModel* pModel=0 ) SAL_OVERRIDE;
    virtual SfxAbstractTabDialog*       CreateTabItemDialog(vcl::Window* pParent,
                                            const SfxItemSet& rSet) SAL_OVERRIDE;
    virtual AbstractSvxCaptionDialog*   CreateCaptionDialog( vcl::Window* pParent,
                                            const SdrView* pView,
                                            sal_uInt16 nAnchorTypes = 0 ) SAL_OVERRIDE;
    virtual AbstractSvxDistributeDialog*    CreateSvxDistributeDialog(vcl::Window* pParent,
                                            const SfxItemSet& rAttr,
                                            SvxDistributeHorizontal eHor = SvxDistributeHorizontalNone,
                                            SvxDistributeVertical eVer = SvxDistributeVerticalNone) SAL_OVERRIDE;
    virtual SfxAbstractInsertObjectDialog* CreateInsertObjectDialog( vcl::Window* pParent, const OUString& rCommmand,
            const css::uno::Reference < css::embed::XStorage >& xStor,
            const SvObjectServerList* pList = 0 ) SAL_OVERRIDE;
    virtual VclAbstractDialog*          CreateEditObjectDialog( vcl::Window* pParent, const OUString& rCommmand,
            const css::uno::Reference < css::embed::XEmbeddedObject >& xObj ) SAL_OVERRIDE;
   virtual  SfxAbstractPasteDialog*         CreatePasteDialog( vcl::Window* pParent ) SAL_OVERRIDE;
   virtual  SfxAbstractLinksDialog*         CreateLinksDialog( vcl::Window* pParent, sfx2::LinkManager* pMgr, bool bHTML, sfx2::SvBaseLink* p=0  ) SAL_OVERRIDE;

   virtual AbstractHangulHanjaConversionDialog * CreateHangulHanjaConversionDialog( vcl::Window* _pParent,
                                            editeng::HangulHanjaConversion::ConversionDirection _ePrimaryDirection ) SAL_OVERRIDE;
   virtual AbstractThesaurusDialog*        CreateThesaurusDialog( vcl::Window*, css::uno::Reference< css::linguistic2::XThesaurus >  xThesaurus,
                                                const OUString &rWord, sal_Int16 nLanguage ) SAL_OVERRIDE;

   virtual AbstractHyphenWordDialog*       CreateHyphenWordDialog( vcl::Window*,
                                                const OUString &rWord, LanguageType nLang,
                                                css::uno::Reference< css::linguistic2::XHyphenator >  &xHyphen,
                                                SvxSpellWrapper* pWrapper ) SAL_OVERRIDE;

   virtual AbstractFmShowColsDialog * CreateFmShowColsDialog( vcl::Window* pParent ) SAL_OVERRIDE;
   virtual AbstractSvxZoomDialog * CreateSvxZoomDialog( vcl::Window* pParent,
                                            const SfxItemSet& rCoreSet) SAL_OVERRIDE;
   //UUUU add for SvxBorderBackgroundDlg
   virtual SfxAbstractTabDialog* CreateSvxBorderBackgroundDlg(
       vcl::Window* pParent,
       const SfxItemSet& rCoreSet,
       bool bEnableSelector = false,
       bool bEnableDrawingLayerFillStyles = false) SAL_OVERRIDE;

   virtual AbstractSvxTransformTabDialog* CreateSvxTransformTabDialog( vcl::Window* pParent,
                                                                const SfxItemSet* pAttr,
                                                                const SdrView* pView,
                                                                sal_uInt16 nAnchorTypes = 0) SAL_OVERRIDE ;
    virtual SfxAbstractTabDialog* CreateSchTransformTabDialog( vcl::Window* pParent,
                                                                const SfxItemSet* pAttr,
                                                                const SdrView* pSdrView,
                                                                sal_uInt32 nResId,
                                                                bool bSizeTabPage = false
                                                                 ) SAL_OVERRIDE;
    virtual AbstractSpellDialog *  CreateSvxSpellDialog(
                            vcl::Window* pParent,
                            SfxBindings* pBindings,
                            svx::SpellDialogChildWindow* pSpellChildWindow ) SAL_OVERRIDE;

    virtual VclAbstractRefreshableDialog * CreateActualizeProgressDialog( vcl::Window* pParent, GalleryTheme* pThm ) SAL_OVERRIDE;
    virtual AbstractTitleDialog * CreateTitleDialog( vcl::Window* pParent,
                                             const OUString& rOldText) SAL_OVERRIDE;
    virtual AbstractGalleryIdDialog * CreateGalleryIdDialog( vcl::Window* pParent,
                                            GalleryTheme* pThm) SAL_OVERRIDE;
    virtual VclAbstractDialog2 * CreateGalleryThemePropertiesDialog( vcl::Window* pParent,
                                            ExchangeData* pData,
                                            SfxItemSet* pItemSet) SAL_OVERRIDE;
    virtual AbstractURLDlg * CreateURLDialog( vcl::Window* pParent,
                                            const OUString& rURL, const OUString& rAltText, const OUString& rDescription,
                                            const OUString& rTarget, const OUString& rName,
                                            TargetList& rTargetList ) SAL_OVERRIDE;

    virtual VclAbstractDialog*      CreateSvxSearchAttributeDialog( vcl::Window* pParent,
                                            SearchAttrItemList& rLst,
                                            const sal_uInt16* pWhRanges) SAL_OVERRIDE;
    virtual AbstractSvxSearchSimilarityDialog * CreateSvxSearchSimilarityDialog( vcl::Window* pParent,
                                                            bool bRelax,
                                                            sal_uInt16 nOther,
                                                            sal_uInt16 nShorter,
                                                            sal_uInt16 nLonger) SAL_OVERRIDE;
    virtual AbstractSvxJSearchOptionsDialog * CreateSvxJSearchOptionsDialog( vcl::Window* pParent,
                                                            const SfxItemSet& rOptionsSet,
                                                            sal_Int32 nInitialFlags) SAL_OVERRIDE;
    virtual AbstractFmInputRecordNoDialog * CreateFmInputRecordNoDialog( vcl::Window* pParent ) SAL_OVERRIDE;
    virtual AbstractSvxNewDictionaryDialog* CreateSvxNewDictionaryDialog( vcl::Window* pParent,
                                            css::uno::Reference< css::linguistic2::XSpellChecker1 >  &xSpl ) SAL_OVERRIDE;
    virtual VclAbstractDialog *     CreateSvxEditDictionaryDialog( vcl::Window* pParent,
                                            const OUString& rName,
                                            css::uno::Reference< css::linguistic2::XSpellChecker1> &xSpl,
                                            sal_uInt32 nResId) SAL_OVERRIDE;
    virtual AbstractSvxNameDialog *     CreateSvxNameDialog( vcl::Window* pParent,
                                            const OUString& rName, const OUString& rDesc ) SAL_OVERRIDE;
    // #i68101#
    virtual AbstractSvxObjectNameDialog* CreateSvxObjectNameDialog(vcl::Window* pParent, const OUString& rName ) SAL_OVERRIDE;
    virtual AbstractSvxObjectTitleDescDialog* CreateSvxObjectTitleDescDialog(vcl::Window* pParent, const OUString& rTitle, const OUString& rDescription) SAL_OVERRIDE;

    virtual AbstractSvxMultiPathDialog *    CreateSvxMultiPathDialog(vcl::Window* pParent) SAL_OVERRIDE;
    virtual AbstractSvxMultiPathDialog *    CreateSvxPathSelectDialog(vcl::Window* pParent) SAL_OVERRIDE;
    virtual AbstractSvxHpLinkDlg *          CreateSvxHpLinkDlg (vcl::Window* pParent,
                                                SfxBindings* pBindings,
                                                sal_uInt32 nResId) SAL_OVERRIDE;
    virtual AbstractFmSearchDialog*         CreateFmSearchDialog(vcl::Window* pParent,
                                                        const OUString& strInitialText,
                                                        const ::std::vector< OUString >& _rContexts,
                                                        sal_Int16 nInitialContext,
                                                        const Link<>& lnkContextSupplier) SAL_OVERRIDE;
    virtual AbstractGraphicFilterDialog *   CreateGraphicFilterEmboss(vcl::Window* pParent,
                                                const Graphic& rGraphic, RECT_POINT eLightSource) SAL_OVERRIDE;
    virtual AbstractGraphicFilterDialog *   CreateGraphicFilterPoster(vcl::Window* pParent,
                                                const Graphic& rGraphic, sal_uInt16 nCount) SAL_OVERRIDE;
    virtual AbstractGraphicFilterDialog *   CreateGraphicFilterSepia (vcl::Window* pParent,
                                                const Graphic& rGraphic, sal_uInt16 nCount) SAL_OVERRIDE;
    virtual AbstractGraphicFilterDialog *   CreateGraphicFilterSmooth (vcl::Window* pParent,
                                                const Graphic& rGraphic, double nRadius) SAL_OVERRIDE;
    virtual AbstractGraphicFilterDialog *   CreateGraphicFilterSolarize (vcl::Window* pParent,
                                                const Graphic& rGraphic, sal_uInt8 nGreyThreshold,
                                                bool bInvert) SAL_OVERRIDE;
    virtual AbstractGraphicFilterDialog *   CreateGraphicFilterMosaic (vcl::Window* pParent,
                                                const Graphic& rGraphic, sal_uInt16 nTileWidth, sal_uInt16 nTileHeight,
                                                bool bEnhanceEdges) SAL_OVERRIDE;
    virtual AbstractSvxAreaTabDialog*       CreateSvxAreaTabDialog( vcl::Window* pParent,
                                                            const SfxItemSet* pAttr,
                                                            SdrModel* pModel,
                                                            bool bShadow ) SAL_OVERRIDE;
    virtual SfxAbstractTabDialog*           CreateSvxLineTabDialog( vcl::Window* pParent, const SfxItemSet* pAttr,
                                                                 SdrModel* pModel,
                                                                 const SdrObject* pObj = NULL,
                                                                 bool bHasObj = true ) SAL_OVERRIDE;
    virtual AbstractSvxPostItDialog*        CreateSvxPostItDialog( vcl::Window* pParent,
                                                                        const SfxItemSet& rCoreSet,
                                                                        bool bPrevNext = false ) SAL_OVERRIDE;

    // For TabPage
    virtual CreateTabPage               GetTabPageCreatorFunc( sal_uInt16 nId ) SAL_OVERRIDE;

    virtual GetTabPageRanges            GetTabPageRangesFunc( sal_uInt16 nId ) SAL_OVERRIDE;
    virtual DialogGetRanges             GetDialogGetRangesFunc() SAL_OVERRIDE;
    virtual VclAbstractDialog*          CreateSvxScriptOrgDialog( vcl::Window* pParent, const OUString& rLanguage ) SAL_OVERRIDE;

    virtual AbstractScriptSelectorDialog*
        CreateScriptSelectorDialog(
            vcl::Window* pParent,
            bool bShowSlots,
            const css::uno::Reference< css::frame::XFrame >& _rxFrame
        ) SAL_OVERRIDE;

    virtual VclAbstractDialog* CreateScriptErrorDialog(
            vcl::Window* pParent, const css::uno::Any& rException) SAL_OVERRIDE;

    virtual VclAbstractDialog*  CreateSvxMacroAssignDlg(
                vcl::Window* _pParent,
                const css::uno::Reference< css::frame::XFrame >& _rxDocumentFrame,
                const bool _bUnoDialogMode,
                const css::uno::Reference< css::container::XNameReplace >& _rxEvents,
                const sal_uInt16 _nInitiallySelectedEvent
            ) SAL_OVERRIDE;

    virtual SfxAbstractTabDialog* CreateSvxFormatCellsDialog( vcl::Window* pParent, const SfxItemSet* pAttr, SdrModel* pModel, const SdrObject* pObj = NULL ) SAL_OVERRIDE;

    virtual SvxAbstractSplittTableDialog* CreateSvxSplittTableDialog( vcl::Window* pParent, bool bIsTableVertical, long nMaxVertical, long nMaxHorizontal ) SAL_OVERRIDE;

    virtual SvxAbstractNewTableDialog* CreateSvxNewTableDialog( vcl::Window* pParent ) SAL_OVERRIDE ;

    virtual VclAbstractDialog*          CreateOptionsDialog(
        vcl::Window* pParent, const OUString& rExtensionId, const OUString& rApplicationContext ) SAL_OVERRIDE;

    virtual SvxAbstractInsRowColDlg* CreateSvxInsRowColDlg( vcl::Window* pParent, bool bCol, const OString& sHelpId ) SAL_OVERRIDE;

    virtual AbstractPasswordToOpenModifyDialog *    CreatePasswordToOpenModifyDialog( vcl::Window * pParent, sal_uInt16 nMinPasswdLen, sal_uInt16 nMaxPasswdLen, bool bIsPasswordToModify ) SAL_OVERRIDE;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
