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
    virtual short   Execute() override ;

#define IMPL_ABSTDLG_BASE(Class)                    \
short Class::Execute()                              \
{                                                   \
    return pDlg->Execute();                         \
}

class VclAbstractDialog2_Impl : public VclAbstractDialog2
{
    ScopedVclPtr<Dialog> m_pDlg;
    Link<Dialog&,void>   m_aEndDlgHdl;
public:
    explicit        VclAbstractDialog2_Impl( Dialog* p ) : m_pDlg( p ) {}
    virtual         ~VclAbstractDialog2_Impl();
    virtual void    StartExecuteModal( const Link<Dialog&,void>& rEndDialogHdl ) override;
    virtual long    GetResult() override;
private:
    DECL_LINK_TYPED( EndDialogHdl, Dialog&, void );
};

class CuiVclAbstractDialog_Impl : public VclAbstractDialog
{
    DECL_ABSTDLG_BASE(CuiVclAbstractDialog_Impl,Dialog)
};

class VclAbstractRefreshableDialog_Impl : public VclAbstractRefreshableDialog
{
    DECL_ABSTDLG_BASE(VclAbstractRefreshableDialog_Impl,Dialog)
    virtual void        Update() override ;
};

class CuiAbstractSfxDialog_Impl : public SfxAbstractDialog
{
    DECL_ABSTDLG_BASE(CuiAbstractSfxDialog_Impl,SfxModalDialog)
    virtual const SfxItemSet*   GetOutputItemSet() const override;

        //From class Window.
    virtual void          SetText( const OUString& rStr ) override;
    virtual OUString      GetText() const override ;
};

class CuiAbstractTabDialog_Impl : public SfxAbstractTabDialog
{
    DECL_ABSTDLG_BASE(CuiAbstractTabDialog_Impl,SfxTabDialog)
    virtual void                SetCurPageId( sal_uInt16 nId ) override;
    virtual void                SetCurPageId( const OString& rName ) override;
    virtual const SfxItemSet*   GetOutputItemSet() const override;
    virtual const sal_uInt16*       GetInputRanges( const SfxItemPool& pItem ) override;
    virtual void                SetInputSet( const SfxItemSet* pInSet ) override;
        //From class Window.
    virtual void        SetText( const OUString& rStr ) override;
    virtual OUString    GetText() const override;
};

class SvxDistributeDialog;
class AbstractSvxDistributeDialog_Impl: public AbstractSvxDistributeDialog
{
    DECL_ABSTDLG_BASE(AbstractSvxDistributeDialog_Impl,SvxDistributeDialog)
public:
    virtual SvxDistributeHorizontal GetDistributeHor() const override;
    virtual SvxDistributeVertical GetDistributeVer() const override;
};

class AbstractHangulHanjaConversionDialog_Impl: public AbstractHangulHanjaConversionDialog
{
    DECL_ABSTDLG_BASE(AbstractHangulHanjaConversionDialog_Impl,HangulHanjaConversionDialog)
    virtual void      EndDialog(long nResult) override;
    virtual void      EnableRubySupport( bool _bVal ) override;
    virtual void      SetByCharacter( bool _bByCharacter ) override ;
    virtual void      SetConversionDirectionState( bool _bTryBothDirections, editeng::HangulHanjaConversion::ConversionDirection _ePrimaryConversionDirection ) override;
    virtual void      SetConversionFormat( editeng::HangulHanjaConversion::ConversionFormat _eType ) override;
    virtual void      SetOptionsChangedHdl( const Link<LinkParamNone*,void>& _rHdl ) override;
    virtual void      SetIgnoreHdl( const Link<Button*,void>& _rHdl ) override;
    virtual void      SetIgnoreAllHdl( const Link<Button*,void>& _rHdl ) override ;
    virtual void      SetChangeHdl( const Link<Button*,void>& _rHdl ) override ;
    virtual void      SetChangeAllHdl( const Link<Button*,void>& _rHdl ) override ;
    virtual void      SetClickByCharacterHdl( const Link<CheckBox*,void>& _rHdl ) override ;
    virtual void      SetConversionFormatChangedHdl( const Link<Button*,void>& _rHdl ) override ;
    virtual void      SetFindHdl( const Link<Button*,void>& _rHdl ) override;
    virtual bool      GetUseBothDirections( ) const override;
    virtual editeng::HangulHanjaConversion::ConversionDirection
                      GetDirection( editeng::HangulHanjaConversion::ConversionDirection _eDefaultDirection ) const override;
    virtual void      SetCurrentString(
                        const OUString& _rNewString,
                        const css::uno::Sequence< OUString >& _rSuggestions,
                        bool _bOriginatesFromDocument = true
                      ) override;
    virtual OUString  GetCurrentString( ) const override ;
    virtual editeng::HangulHanjaConversion::ConversionFormat
                      GetConversionFormat( ) const override ;
    virtual void      FocusSuggestion( ) override;
    virtual OUString  GetCurrentSuggestion( ) const override;
};

class AbstractThesaurusDialog_Impl : public AbstractThesaurusDialog
{
    DECL_ABSTDLG_BASE(AbstractThesaurusDialog_Impl,SvxThesaurusDialog)
    virtual OUString    GetWord() override;
};

class AbstractHyphenWordDialog_Impl: public AbstractHyphenWordDialog
{
    DECL_ABSTDLG_BASE(AbstractHyphenWordDialog_Impl,SvxHyphenWordDialog)
    virtual vcl::Window* GetWindow() override;
};

class FmShowColsDialog;
class AbstractFmShowColsDialog_Impl : public AbstractFmShowColsDialog
{
    DECL_ABSTDLG_BASE(AbstractFmShowColsDialog_Impl,FmShowColsDialog)
     virtual void   SetColumns(const css::uno::Reference< css::container::XIndexContainer>& xCols) override;
};

class SvxZoomDialog;
class AbstractSvxZoomDialog_Impl : public AbstractSvxZoomDialog
{
    DECL_ABSTDLG_BASE(AbstractSvxZoomDialog_Impl,SvxZoomDialog)
    virtual void    SetLimits( sal_uInt16 nMin, sal_uInt16 nMax ) override;
    virtual void    HideButton( ZoomButtonId nBtnId ) override;
    virtual const SfxItemSet*   GetOutputItemSet() const override ;

};

namespace svx{ class SpellDialog;}
class AbstractSpellDialog_Impl : public AbstractSpellDialog
{
 public:
    DECL_ABSTDLG_BASE(AbstractSpellDialog_Impl, svx::SpellDialog)
    virtual void        Invalidate() override;
    virtual vcl::Window*     GetWindow() override;
    virtual SfxBindings& GetBindings() override;
};

class TitleDialog;
class AbstractTitleDialog_Impl : public AbstractTitleDialog
{
    DECL_ABSTDLG_BASE(AbstractTitleDialog_Impl,TitleDialog)
    virtual OUString  GetTitle() const override ;

};

class SvxScriptSelectorDialog;
class AbstractScriptSelectorDialog_Impl : public AbstractScriptSelectorDialog
{
    DECL_ABSTDLG_BASE(
        AbstractScriptSelectorDialog_Impl, SvxScriptSelectorDialog)

    virtual OUString GetScriptURL() const override;

    virtual void SetRunLabel() override;
};

class GalleryIdDialog;
class AbstractGalleryIdDialog_Impl : public AbstractGalleryIdDialog
{
    DECL_ABSTDLG_BASE(AbstractGalleryIdDialog_Impl,GalleryIdDialog)
    virtual sal_uLong   GetId() const override ;

};

class URLDlg;
class AbstractURLDlg_Impl :public AbstractURLDlg
{
    DECL_ABSTDLG_BASE(AbstractURLDlg_Impl,URLDlg)
    virtual OUString      GetURL() const override;
    virtual OUString      GetAltText() const override;
    virtual OUString      GetDesc() const override;
    virtual OUString      GetTarget() const override;
    virtual OUString      GetName() const override;
};

class SvxSearchSimilarityDialog;
class AbstractSvxSearchSimilarityDialog_Impl :public AbstractSvxSearchSimilarityDialog
{
    DECL_ABSTDLG_BASE(AbstractSvxSearchSimilarityDialog_Impl,SvxSearchSimilarityDialog)
    virtual sal_uInt16              GetOther() override;
    virtual sal_uInt16              GetShorter() override;
    virtual sal_uInt16              GetLonger() override;
    virtual bool                    IsRelaxed() override;
};

class SvxJSearchOptionsDialog;
class AbstractSvxJSearchOptionsDialog_Impl :public AbstractSvxJSearchOptionsDialog
{
    DECL_ABSTDLG_BASE(AbstractSvxJSearchOptionsDialog_Impl,SvxJSearchOptionsDialog)
    virtual sal_Int32           GetTransliterationFlags() const override;
};

class AbstractSvxTransformTabDialog_Impl : public AbstractSvxTransformTabDialog
{
    DECL_ABSTDLG_BASE(AbstractSvxTransformTabDialog_Impl,SvxTransformTabDialog)
    virtual void SetValidateFramePosLink( const Link<SvxSwFrameValidation&,void>& rLink ) override;
    virtual void                SetCurPageId( sal_uInt16 nId ) override;
    virtual void                SetCurPageId( const OString& rName ) override;
    virtual const SfxItemSet*   GetOutputItemSet() const override;
    virtual const sal_uInt16*       GetInputRanges( const SfxItemPool& pItem ) override;
    virtual void                SetInputSet( const SfxItemSet* pInSet ) override;
    virtual void        SetText( const OUString& rStr ) override;
    virtual OUString    GetText() const override;
};

class AbstractSvxCaptionDialog_Impl : public AbstractSvxCaptionDialog
{
    DECL_ABSTDLG_BASE(AbstractSvxCaptionDialog_Impl,SvxCaptionTabDialog)
    virtual void SetValidateFramePosLink( const Link<SvxSwFrameValidation&,void>& rLink ) override;
    virtual void                SetCurPageId( sal_uInt16 nId ) override;
    virtual void                SetCurPageId( const OString& rName ) override;
    virtual const SfxItemSet*   GetOutputItemSet() const override;
    virtual const sal_uInt16*       GetInputRanges( const SfxItemPool& pItem ) override;
    virtual void                SetInputSet( const SfxItemSet* pInSet ) override;
    virtual void        SetText( const OUString& rStr ) override;
    virtual OUString    GetText() const override;
};

class FmInputRecordNoDialog;
class AbstractFmInputRecordNoDialog_Impl :public AbstractFmInputRecordNoDialog
{
    DECL_ABSTDLG_BASE(AbstractFmInputRecordNoDialog_Impl,FmInputRecordNoDialog)
    virtual void SetValue(long nNew) override ;
    virtual long GetValue() const override ;
};

class SvxNewDictionaryDialog;
class AbstractSvxNewDictionaryDialog_Impl :public AbstractSvxNewDictionaryDialog
{
    DECL_ABSTDLG_BASE(AbstractSvxNewDictionaryDialog_Impl,SvxNewDictionaryDialog)
    virtual css::uno::Reference< css::linguistic2::XDictionary >  GetNewDictionary() override;
};

class SvxNameDialog;
class AbstractSvxNameDialog_Impl :public AbstractSvxNameDialog
{
    DECL_ABSTDLG_BASE(AbstractSvxNameDialog_Impl,SvxNameDialog)
    virtual void    GetName( OUString& rName ) override ;
    virtual void    SetCheckNameHdl( const Link<AbstractSvxNameDialog&,bool>& rLink, bool bCheckImmediately = false ) override ;
    virtual void    SetEditHelpId(const OString&) override ;
    //from class Window
    virtual void    SetHelpId( const OString& ) override ;
    virtual void    SetText( const OUString& rStr ) override ;
private:
    Link<AbstractSvxNameDialog&,bool> aCheckNameHdl;
    DECL_LINK_TYPED(CheckNameHdl, SvxNameDialog&, bool);
};

class SvxObjectNameDialog;
class SvxObjectTitleDescDialog;

class AbstractSvxObjectNameDialog_Impl :public AbstractSvxObjectNameDialog
{
    DECL_ABSTDLG_BASE(AbstractSvxObjectNameDialog_Impl, SvxObjectNameDialog)
    virtual void GetName(OUString& rName) override ;
    virtual void SetCheckNameHdl(const Link<AbstractSvxObjectNameDialog&,bool>& rLink) override;

private:
    Link<AbstractSvxObjectNameDialog&,bool> aCheckNameHdl;
    DECL_LINK_TYPED(CheckNameHdl, SvxObjectNameDialog&, bool);
};

class AbstractSvxObjectTitleDescDialog_Impl :public AbstractSvxObjectTitleDescDialog
{
    DECL_ABSTDLG_BASE(AbstractSvxObjectTitleDescDialog_Impl, SvxObjectTitleDescDialog)
    virtual void GetTitle(OUString& rName) override;
    virtual void GetDescription(OUString& rName) override;
};

class SvxMultiPathDialog;
class AbstractSvxMultiPathDialog_Impl : public AbstractSvxMultiPathDialog
{
    DECL_ABSTDLG_BASE(AbstractSvxMultiPathDialog_Impl,SvxMultiPathDialog)
    virtual OUString        GetPath() const override;
    virtual void            SetPath( const OUString& rPath ) override;
    virtual void            SetTitle( const OUString& rNewTitle ) override;
};

class SvxPathSelectDialog;
class AbstractSvxPathSelectDialog_Impl : public AbstractSvxMultiPathDialog
{
    DECL_ABSTDLG_BASE(AbstractSvxPathSelectDialog_Impl,SvxPathSelectDialog)
    virtual OUString        GetPath() const override;
    virtual void            SetPath( const OUString& rPath ) override;
    virtual void            SetTitle( const OUString& rNewTitle ) override;
};

class SvxHpLinkDlg;
class AbstractSvxHpLinkDlg_Impl :public AbstractSvxHpLinkDlg
{
    DECL_ABSTDLG_BASE(AbstractSvxHpLinkDlg_Impl,SvxHpLinkDlg)
    virtual vcl::Window*     GetWindow() override;
    virtual bool        QueryClose() override;
};

class FmSearchDialog;
struct FmFoundRecordInformation;
class AbstractFmSearchDialog_Impl :public AbstractFmSearchDialog
{
    DECL_ABSTDLG_BASE(AbstractFmSearchDialog_Impl,FmSearchDialog)
    virtual void SetFoundHandler(const Link<FmFoundRecordInformation&,void>& lnk) override ;
    virtual void SetCanceledNotFoundHdl(const Link<FmFoundRecordInformation&,void>& lnk) override;
    virtual void SetActiveField(const OUString& strField) override;
};

class AbstractGraphicFilterDialog_Impl :public AbstractGraphicFilterDialog
{
    DECL_ABSTDLG_BASE(AbstractGraphicFilterDialog_Impl,GraphicFilterDialog)
    virtual Graphic GetFilteredGraphic( const Graphic& rGraphic, double fScaleX, double fScaleY ) override;
};

class SvxAreaTabDialog;
class AbstractSvxAreaTabDialog_Impl :public AbstractSvxAreaTabDialog
{
    DECL_ABSTDLG_BASE(AbstractSvxAreaTabDialog_Impl,SvxAreaTabDialog)
    virtual void                SetCurPageId( sal_uInt16 nId ) override;
    virtual void                SetCurPageId( const OString& rName ) override;
    virtual const SfxItemSet*   GetOutputItemSet() const override;
    virtual const sal_uInt16*       GetInputRanges( const SfxItemPool& pItem ) override;
    virtual void                SetInputSet( const SfxItemSet* pInSet ) override;
    // From class Window.
    virtual void        SetText( const OUString& rStr ) override;
    virtual OUString    GetText() const override;
};

class AbstractInsertObjectDialog_Impl : public SfxAbstractInsertObjectDialog
{
    DECL_ABSTDLG_BASE(AbstractInsertObjectDialog_Impl, InsertObjectDialog_Impl)
    virtual css::uno::Reference < css::embed::XEmbeddedObject > GetObject() override;
    virtual css::uno::Reference< css::io::XInputStream > GetIconIfIconified( OUString* pGraphicMediaType ) override;
    virtual bool IsCreateNew() override;
};

class AbstractPasteDialog_Impl : public SfxAbstractPasteDialog
{
public:
    DECL_ABSTDLG_BASE(AbstractPasteDialog_Impl, SvPasteObjectDialog )
    virtual void Insert( SotClipboardFormatId nFormat, const OUString & rFormatName ) override;
    virtual void SetObjName( const SvGlobalName & rClass, const OUString & rObjName ) override;
    virtual SotClipboardFormatId GetFormat( const TransferableDataHelper& aHelper ) override;
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
    virtual void                SetText( const OUString& rStr ) override;  //From class Window
    virtual const SfxItemSet*   GetOutputItemSet() const override;
    virtual void                SetPrevHdl( const Link<AbstractSvxPostItDialog&,void>& rLink ) override ;
    virtual void                SetNextHdl( const Link<AbstractSvxPostItDialog&,void>& rLink ) override ;
    virtual void                EnableTravel(bool bNext, bool bPrev) override ;
    virtual OUString            GetNote() override ;
    virtual void                SetNote(const OUString& rTxt) override ;
    virtual void                ShowLastAuthor(const OUString& rAuthor, const OUString& rDate) override ;
    virtual void                DontChangeAuthor() override ;
    virtual void                HideAuthor() override ;
    virtual vcl::Window *       GetWindow() override;
private:
    Link<AbstractSvxPostItDialog&,void> aNextHdl;
    Link<AbstractSvxPostItDialog&,void> aPrevHdl;
    DECL_LINK_TYPED(NextHdl, SvxPostItDialog&, void);
    DECL_LINK_TYPED(PrevHdl, SvxPostItDialog&, void);
};

class PasswordToOpenModifyDialog;
class AbstractPasswordToOpenModifyDialog_Impl : public AbstractPasswordToOpenModifyDialog
{
    DECL_ABSTDLG_BASE( AbstractPasswordToOpenModifyDialog_Impl, PasswordToOpenModifyDialog )

    virtual OUString  GetPasswordToOpen() const override;
    virtual OUString  GetPasswordToModify() const override;
    virtual bool      IsRecommendToOpenReadonly() const override;
};

//AbstractDialogFactory_Impl implementations
class AbstractDialogFactory_Impl : public SvxAbstractDialogFactory
{
public:
    virtual VclAbstractDialog*          CreateVclDialog( vcl::Window* pParent, sal_uInt32 nResId ) override;

    virtual SfxAbstractDialog*          CreateSfxDialog( vcl::Window* pParent,
                                            const SfxItemSet& rAttr,
                                            const SdrView* pView,
                                            sal_uInt32 nResId ) override;
    virtual SfxAbstractDialog*          CreateSfxDialog( vcl::Window* pParent,
                                                             const SfxItemSet& rAttr,
                                                             const css::uno::Reference< css::frame::XFrame >& _rxFrame,
                                                             sal_uInt32 nResId ) override;
    virtual VclAbstractDialog*          CreateFrameDialog( const css::uno::Reference< css::frame::XFrame >& rxFrame,
                                                           sal_uInt32 nResId,
                                                           const OUString& rParameter ) override;
    virtual SfxAbstractTabDialog*       CreateTabDialog( sal_uInt32 nResId,
                                            vcl::Window* pParent,
                                            const SfxItemSet* pAttrSet,
                                            SfxViewFrame* pViewFrame ) override;
    virtual SfxAbstractTabDialog*       CreateTabDialog( sal_uInt32 nResId,
                                            vcl::Window* pParent,
                                            const SfxItemSet* pAttrSet,
                                            const css::uno::Reference< css::frame::XFrame >& xViewFrame ) override;
    virtual SfxAbstractTabDialog*       CreateTextTabDialog( vcl::Window* pParent,
                                            const SfxItemSet* pAttrSet,
                                            SdrView* pView ) override;
    virtual SfxAbstractTabDialog*       CreateTabItemDialog(vcl::Window* pParent,
                                            const SfxItemSet& rSet) override;
    virtual AbstractSvxCaptionDialog*   CreateCaptionDialog( vcl::Window* pParent,
                                            const SdrView* pView,
                                            sal_uInt16 nAnchorTypes = 0 ) override;
    virtual AbstractSvxDistributeDialog*    CreateSvxDistributeDialog(const SfxItemSet& rAttr) override;
    virtual SfxAbstractInsertObjectDialog* CreateInsertObjectDialog( vcl::Window* pParent, const OUString& rCommmand,
            const css::uno::Reference < css::embed::XStorage >& xStor,
            const SvObjectServerList* pList = nullptr ) override;
    virtual VclAbstractDialog*          CreateEditObjectDialog( const OUString& rCommmand,
            const css::uno::Reference < css::embed::XEmbeddedObject >& xObj ) override;
   virtual  SfxAbstractPasteDialog*         CreatePasteDialog( vcl::Window* pParent ) override;
   virtual  SfxAbstractLinksDialog*         CreateLinksDialog( vcl::Window* pParent, sfx2::LinkManager* pMgr, bool bHTML = false, sfx2::SvBaseLink* p=nullptr  ) override;

   virtual AbstractHangulHanjaConversionDialog * CreateHangulHanjaConversionDialog( vcl::Window* _pParent,
                                            editeng::HangulHanjaConversion::ConversionDirection _ePrimaryDirection ) override;
   virtual AbstractThesaurusDialog*        CreateThesaurusDialog( vcl::Window*, css::uno::Reference< css::linguistic2::XThesaurus >  xThesaurus,
                                                const OUString &rWord, sal_Int16 nLanguage ) override;

   virtual AbstractHyphenWordDialog*       CreateHyphenWordDialog( vcl::Window*,
                                                const OUString &rWord, LanguageType nLang,
                                                css::uno::Reference< css::linguistic2::XHyphenator >  &xHyphen,
                                                SvxSpellWrapper* pWrapper ) override;

   virtual AbstractFmShowColsDialog * CreateFmShowColsDialog() override;
   virtual AbstractSvxZoomDialog * CreateSvxZoomDialog( vcl::Window* pParent,
                                            const SfxItemSet& rCoreSet) override;
   //UUUU add for SvxBorderBackgroundDlg
   virtual SfxAbstractTabDialog* CreateSvxBorderBackgroundDlg(
       vcl::Window* pParent,
       const SfxItemSet& rCoreSet,
       bool bEnableSelector = false,
       bool bEnableDrawingLayerFillStyles = false) override;

   virtual AbstractSvxTransformTabDialog* CreateSvxTransformTabDialog( vcl::Window* pParent,
                                                                const SfxItemSet* pAttr,
                                                                const SdrView* pView,
                                                                sal_uInt16 nAnchorTypes = 0) override ;
    virtual SfxAbstractTabDialog* CreateSchTransformTabDialog( vcl::Window* pParent,
                                                                const SfxItemSet* pAttr,
                                                                const SdrView* pSdrView,
                                                                sal_uInt32 nResId,
                                                                bool bSizeTabPage = false
                                                                 ) override;
    virtual AbstractSpellDialog *  CreateSvxSpellDialog(
                            vcl::Window* pParent,
                            SfxBindings* pBindings,
                            svx::SpellDialogChildWindow* pSpellChildWindow ) override;

    virtual VclAbstractRefreshableDialog * CreateActualizeProgressDialog( vcl::Window* pParent, GalleryTheme* pThm ) override;
    virtual AbstractTitleDialog * CreateTitleDialog( vcl::Window* pParent,
                                             const OUString& rOldText) override;
    virtual AbstractGalleryIdDialog * CreateGalleryIdDialog( vcl::Window* pParent,
                                            GalleryTheme* pThm) override;
    virtual VclAbstractDialog2 * CreateGalleryThemePropertiesDialog(
                                            ExchangeData* pData,
                                            SfxItemSet* pItemSet) override;
    virtual AbstractURLDlg * CreateURLDialog( vcl::Window* pParent,
                                            const OUString& rURL, const OUString& rAltText, const OUString& rDescription,
                                            const OUString& rTarget, const OUString& rName,
                                            TargetList& rTargetList ) override;

    virtual VclAbstractDialog*      CreateSvxSearchAttributeDialog( vcl::Window* pParent,
                                            SearchAttrItemList& rLst,
                                            const sal_uInt16* pWhRanges) override;
    virtual AbstractSvxSearchSimilarityDialog * CreateSvxSearchSimilarityDialog( vcl::Window* pParent,
                                                            bool bRelax,
                                                            sal_uInt16 nOther,
                                                            sal_uInt16 nShorter,
                                                            sal_uInt16 nLonger) override;
    virtual AbstractSvxJSearchOptionsDialog * CreateSvxJSearchOptionsDialog( vcl::Window* pParent,
                                                            const SfxItemSet& rOptionsSet,
                                                            sal_Int32 nInitialFlags) override;
    virtual AbstractFmInputRecordNoDialog * CreateFmInputRecordNoDialog() override;
    virtual AbstractSvxNewDictionaryDialog* CreateSvxNewDictionaryDialog( vcl::Window* pParent ) override;
    virtual VclAbstractDialog *     CreateSvxEditDictionaryDialog( vcl::Window* pParent,
                                            const OUString& rName,
                                            sal_uInt32 nResId) override;
    virtual AbstractSvxNameDialog *     CreateSvxNameDialog( vcl::Window* pParent,
                                            const OUString& rName, const OUString& rDesc ) override;
    // #i68101#
    virtual AbstractSvxObjectNameDialog* CreateSvxObjectNameDialog(const OUString& rName ) override;
    virtual AbstractSvxObjectTitleDescDialog* CreateSvxObjectTitleDescDialog(const OUString& rTitle, const OUString& rDescription) override;

    virtual AbstractSvxMultiPathDialog *    CreateSvxMultiPathDialog(vcl::Window* pParent) override;
    virtual AbstractSvxMultiPathDialog *    CreateSvxPathSelectDialog(vcl::Window* pParent) override;
    virtual AbstractSvxHpLinkDlg *          CreateSvxHpLinkDlg(vcl::Window* pParent, SfxBindings* pBindings) override;
    virtual AbstractFmSearchDialog*         CreateFmSearchDialog(vcl::Window* pParent,
                                                        const OUString& strInitialText,
                                                        const ::std::vector< OUString >& _rContexts,
                                                        sal_Int16 nInitialContext,
                                                        const Link<FmSearchContext&,sal_uInt32>& lnkContextSupplier) override;
    virtual AbstractGraphicFilterDialog *   CreateGraphicFilterEmboss(vcl::Window* pParent,
                                                const Graphic& rGraphic, RECT_POINT eLightSource) override;
    virtual AbstractGraphicFilterDialog *   CreateGraphicFilterPoster(vcl::Window* pParent,
                                                const Graphic& rGraphic, sal_uInt16 nCount) override;
    virtual AbstractGraphicFilterDialog *   CreateGraphicFilterSepia (vcl::Window* pParent,
                                                const Graphic& rGraphic, sal_uInt16 nCount) override;
    virtual AbstractGraphicFilterDialog *   CreateGraphicFilterSmooth (vcl::Window* pParent,
                                                const Graphic& rGraphic, double nRadius) override;
    virtual AbstractGraphicFilterDialog *   CreateGraphicFilterSolarize (vcl::Window* pParent,
                                                const Graphic& rGraphic, sal_uInt8 nGreyThreshold) override;
    virtual AbstractGraphicFilterDialog *   CreateGraphicFilterMosaic (vcl::Window* pParent,
                                                const Graphic& rGraphic, sal_uInt16 nTileWidth, sal_uInt16 nTileHeight) override;
    virtual AbstractSvxAreaTabDialog*       CreateSvxAreaTabDialog( vcl::Window* pParent,
                                                            const SfxItemSet* pAttr,
                                                            SdrModel* pModel,
                                                            bool bShadow ) override;
    virtual SfxAbstractTabDialog*           CreateSvxLineTabDialog( vcl::Window* pParent, const SfxItemSet* pAttr,
                                                                 SdrModel* pModel,
                                                                 const SdrObject* pObj = nullptr,
                                                                 bool bHasObj = true ) override;
    virtual AbstractSvxPostItDialog*        CreateSvxPostItDialog( vcl::Window* pParent,
                                                                        const SfxItemSet& rCoreSet,
                                                                        bool bPrevNext = false ) override;

    // For TabPage
    virtual CreateTabPage               GetTabPageCreatorFunc( sal_uInt16 nId ) override;

    virtual GetTabPageRanges            GetTabPageRangesFunc( sal_uInt16 nId ) override;
    virtual DialogGetRanges             GetDialogGetRangesFunc() override;
    virtual VclAbstractDialog*          CreateSvxScriptOrgDialog( vcl::Window* pParent, const OUString& rLanguage ) override;

    virtual AbstractScriptSelectorDialog*
        CreateScriptSelectorDialog(
            vcl::Window* pParent,
            const css::uno::Reference< css::frame::XFrame >& _rxFrame
        ) override;

    virtual VclAbstractDialog* CreateScriptErrorDialog(const css::uno::Any& rException) override;

    virtual VclAbstractDialog*  CreateSvxMacroAssignDlg(
                vcl::Window* _pParent,
                const css::uno::Reference< css::frame::XFrame >& _rxDocumentFrame,
                const bool _bUnoDialogMode,
                const css::uno::Reference< css::container::XNameReplace >& _rxEvents,
                const sal_uInt16 _nInitiallySelectedEvent
            ) override;

    virtual SfxAbstractTabDialog* CreateSvxFormatCellsDialog( const SfxItemSet* pAttr, SdrModel* pModel, const SdrObject* pObj = nullptr ) override;

    virtual SvxAbstractSplittTableDialog* CreateSvxSplittTableDialog( vcl::Window* pParent, bool bIsTableVertical, long nMaxVertical, long nMaxHorizontal ) override;

    virtual SvxAbstractNewTableDialog* CreateSvxNewTableDialog() override ;

    virtual VclAbstractDialog*          CreateOptionsDialog(
        vcl::Window* pParent, const OUString& rExtensionId, const OUString& rApplicationContext ) override;

    virtual SvxAbstractInsRowColDlg* CreateSvxInsRowColDlg( vcl::Window* pParent, bool bCol, const OString& sHelpId ) override;

    virtual AbstractPasswordToOpenModifyDialog *    CreatePasswordToOpenModifyDialog( vcl::Window * pParent, sal_uInt16 nMinPasswdLen, sal_uInt16 nMaxPasswdLen, bool bIsPasswordToModify ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
