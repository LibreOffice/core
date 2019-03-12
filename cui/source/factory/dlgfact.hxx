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

#include <tools/link.hxx>
#include <com/sun/star/frame/XFrame.hpp>

class SfxTabDialog;
class SfxModalDialog;
class SfxSingleTabDialogController;
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
    virtual short   Execute() override;             \
    virtual bool    StartExecuteAsync(VclAbstractDialog::AsyncContext &rCtx) override;

#define IMPL_ABSTDLG_BASE(Class)                    \
short Class::Execute()                              \
{                                                   \
    return pDlg->Execute();                         \
}                                                   \
bool Class::StartExecuteAsync(VclAbstractDialog::AsyncContext &rCtx) \
{                                                   \
    return pDlg->StartExecuteAsync(rCtx);           \
}

class CuiVclAbstractDialog_Impl : public VclAbstractDialog
{
    DECL_ABSTDLG_BASE(CuiVclAbstractDialog_Impl,Dialog)
};

class CuiAbstractController_Impl : public VclAbstractDialog
{
    std::unique_ptr<weld::DialogController> m_xDlg;
public:
    explicit CuiAbstractController_Impl(std::unique_ptr<weld::DialogController> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
};

class CuiAbstractSfxDialog_Impl : public SfxAbstractDialog
{
    DECL_ABSTDLG_BASE(CuiAbstractSfxDialog_Impl,SfxModalDialog)
    virtual const SfxItemSet*   GetOutputItemSet() const override;

        //From class Window.
    virtual void          SetText( const OUString& rStr ) override;
};

class CuiAbstractSingleTabController_Impl : public SfxAbstractDialog
{
    std::unique_ptr<SfxSingleTabDialogController> m_xDlg;
public:
    explicit CuiAbstractSingleTabController_Impl(std::unique_ptr<SfxSingleTabDialogController> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual const SfxItemSet*   GetOutputItemSet() const override;

    //From class Window.
    virtual void          SetText( const OUString& rStr ) override;
};

class CuiAbstractTabController_Impl : public SfxAbstractTabDialog
{
    std::shared_ptr<SfxTabDialogController> m_xDlg;
public:
    explicit CuiAbstractTabController_Impl(std::unique_ptr<SfxTabDialogController> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual bool StartExecuteAsync(AsyncContext &rCtx) override;
    virtual void                SetCurPageId( const OString &rName ) override;
    virtual const SfxItemSet*   GetOutputItemSet() const override;
    virtual const sal_uInt16*   GetInputRanges( const SfxItemPool& pItem ) override;
    virtual void                SetInputSet( const SfxItemSet* pInSet ) override;
        //From class Window.
    virtual void        SetText( const OUString& rStr ) override;
};

class SvxDistributeDialog;
class AbstractSvxDistributeDialog_Impl: public AbstractSvxDistributeDialog
{
    std::unique_ptr<SvxDistributeDialog> m_xDlg;
public:
    explicit AbstractSvxDistributeDialog_Impl(std::unique_ptr<SvxDistributeDialog> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
public:
    virtual SvxDistributeHorizontal GetDistributeHor() const override;
    virtual SvxDistributeVertical GetDistributeVer() const override;
};

class AbstractHangulHanjaConversionDialog_Impl: public AbstractHangulHanjaConversionDialog
{
private:
    std::unique_ptr<HangulHanjaConversionDialog> m_xDlg;
public:
    explicit AbstractHangulHanjaConversionDialog_Impl(std::unique_ptr<HangulHanjaConversionDialog> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short     Execute() override;
    virtual void      EndDialog(sal_Int32 nResult) override;
    virtual void      EnableRubySupport( bool _bVal ) override;
    virtual void      SetByCharacter( bool _bByCharacter ) override ;
    virtual void      SetConversionDirectionState( bool _bTryBothDirections, editeng::HangulHanjaConversion::ConversionDirection _ePrimaryConversionDirection ) override;
    virtual void      SetConversionFormat( editeng::HangulHanjaConversion::ConversionFormat _eType ) override;
    virtual void      SetOptionsChangedHdl( const Link<LinkParamNone*,void>& _rHdl ) override;
    virtual void      SetIgnoreHdl( const Link<weld::Button&,void>& _rHdl ) override;
    virtual void      SetIgnoreAllHdl( const Link<weld::Button&,void>& _rHdl ) override ;
    virtual void      SetChangeHdl( const Link<weld::Button&,void>& _rHdl ) override ;
    virtual void      SetChangeAllHdl( const Link<weld::Button&,void>& rHdl ) override ;
    virtual void      SetClickByCharacterHdl( const Link<weld::ToggleButton&,void>& rHdl ) override ;
    virtual void      SetConversionFormatChangedHdl( const Link<weld::Button&,void>& _rHdl ) override ;
    virtual void      SetFindHdl( const Link<weld::Button&,void>& _rHdl ) override;
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
    std::unique_ptr<SvxHyphenWordDialog> m_xDlg;
public:
    explicit AbstractHyphenWordDialog_Impl(std::unique_ptr<SvxHyphenWordDialog> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
};

class FmShowColsDialog;
class AbstractFmShowColsDialog_Impl : public AbstractFmShowColsDialog
{
    std::unique_ptr<FmShowColsDialog> m_xDlg;
public:
    explicit AbstractFmShowColsDialog_Impl(std::unique_ptr<FmShowColsDialog> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual void SetColumns(const css::uno::Reference< css::container::XIndexContainer>& xCols) override;
};

class SvxZoomDialog;
class AbstractSvxZoomDialog_Impl : public AbstractSvxZoomDialog
{
    std::unique_ptr<SvxZoomDialog> m_xDlg;
public:
    explicit AbstractSvxZoomDialog_Impl(std::unique_ptr<SvxZoomDialog> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual void  SetLimits( sal_uInt16 nMin, sal_uInt16 nMax ) override;
    virtual void  HideButton( ZoomButtonId nBtnId ) override;
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
protected:
    std::unique_ptr<TitleDialog> m_xDlg;
public:
    explicit AbstractTitleDialog_Impl(std::unique_ptr<TitleDialog> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual OUString  GetTitle() const override ;

};

class SvxScriptSelectorDialog;
class AbstractScriptSelectorDialog_Impl : public AbstractScriptSelectorDialog
{
    std::unique_ptr<SvxScriptSelectorDialog> m_xDlg;
public:
    explicit AbstractScriptSelectorDialog_Impl(std::unique_ptr<SvxScriptSelectorDialog> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual OUString GetScriptURL() const override;
    virtual void SetRunLabel() override;
};

class GalleryIdDialog;
class AbstractGalleryIdDialog_Impl : public AbstractGalleryIdDialog
{
protected:
    std::unique_ptr<GalleryIdDialog> m_xDlg;
public:
    explicit AbstractGalleryIdDialog_Impl(std::unique_ptr<GalleryIdDialog> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual sal_uInt32 GetId() const override;
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
    std::unique_ptr<SvxSearchSimilarityDialog> m_xDlg;
public:
    explicit AbstractSvxSearchSimilarityDialog_Impl(std::unique_ptr<SvxSearchSimilarityDialog> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual sal_uInt16              GetOther() override;
    virtual sal_uInt16              GetShorter() override;
    virtual sal_uInt16              GetLonger() override;
    virtual bool                    IsRelaxed() override;
};

class SvxJSearchOptionsDialog;
class AbstractSvxJSearchOptionsDialog_Impl : public AbstractSvxJSearchOptionsDialog
{
    std::unique_ptr<SvxJSearchOptionsDialog> m_xDlg;
public:
    explicit AbstractSvxJSearchOptionsDialog_Impl(std::unique_ptr<SvxJSearchOptionsDialog> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual TransliterationFlags    GetTransliterationFlags() const override;
};

class AbstractSvxTransformTabDialog_Impl : public AbstractSvxTransformTabDialog
{
    std::shared_ptr<SvxTransformTabDialog> m_xDlg;
public:
    explicit AbstractSvxTransformTabDialog_Impl(SvxTransformTabDialog* p)
        : m_xDlg(p)
    {
    }
    virtual short Execute() override;
    virtual bool StartExecuteAsync(AsyncContext &rCtx) override;
    virtual void SetValidateFramePosLink( const Link<SvxSwFrameValidation&,void>& rLink ) override;
    virtual void                SetCurPageId( const OString& rName ) override;
    virtual const SfxItemSet*   GetOutputItemSet() const override;
    virtual const sal_uInt16*       GetInputRanges( const SfxItemPool& pItem ) override;
    virtual void                SetInputSet( const SfxItemSet* pInSet ) override;
    virtual void        SetText( const OUString& rStr ) override;
};

class AbstractSvxCaptionDialog_Impl : public AbstractSvxCaptionDialog
{
    std::shared_ptr<SvxCaptionTabDialog> m_xDlg;
public:
    explicit AbstractSvxCaptionDialog_Impl(std::unique_ptr<SvxCaptionTabDialog> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual bool StartExecuteAsync(AsyncContext &rCtx) override;
    virtual void SetValidateFramePosLink( const Link<SvxSwFrameValidation&,void>& rLink ) override;
    virtual void                SetCurPageId( const OString& rName ) override;
    virtual const SfxItemSet*   GetOutputItemSet() const override;
    virtual const sal_uInt16*       GetInputRanges( const SfxItemPool& pItem ) override;
    virtual void                SetInputSet( const SfxItemSet* pInSet ) override;
    virtual void        SetText( const OUString& rStr ) override;
};

class FmInputRecordNoDialog;
class AbstractFmInputRecordNoDialog_Impl :public AbstractFmInputRecordNoDialog
{
    std::unique_ptr<FmInputRecordNoDialog> m_xDlg;
public:
    explicit AbstractFmInputRecordNoDialog_Impl(std::unique_ptr<FmInputRecordNoDialog> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual void SetValue(long nNew) override ;
    virtual long GetValue() const override ;
};

class SvxNewDictionaryDialog;
class AbstractSvxNewDictionaryDialog_Impl :public AbstractSvxNewDictionaryDialog
{
    std::unique_ptr<SvxNewDictionaryDialog> m_xDlg;
public:
    explicit AbstractSvxNewDictionaryDialog_Impl(std::unique_ptr<SvxNewDictionaryDialog> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual css::uno::Reference< css::linguistic2::XDictionary >  GetNewDictionary() override;
};

class SvxNameDialog;
class AbstractSvxNameDialog_Impl :public AbstractSvxNameDialog
{
public:
    explicit AbstractSvxNameDialog_Impl(std::unique_ptr<SvxNameDialog> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual void    GetName( OUString& rName ) override ;
    virtual void    SetCheckNameHdl( const Link<AbstractSvxNameDialog&,bool>& rLink, bool bCheckImmediately = false ) override ;
    virtual void    SetEditHelpId(const OString&) override ;
    //from class Window
    virtual void    SetHelpId( const OString& ) override ;
    virtual void    SetText( const OUString& rStr ) override ;
private:
    std::unique_ptr<SvxNameDialog> m_xDlg;
    Link<AbstractSvxNameDialog&,bool> aCheckNameHdl;
    DECL_LINK(CheckNameHdl, SvxNameDialog&, bool);
};

class SvxObjectNameDialog;
class SvxObjectTitleDescDialog;

class AbstractSvxObjectNameDialog_Impl : public AbstractSvxObjectNameDialog
{
public:
    explicit AbstractSvxObjectNameDialog_Impl(std::unique_ptr<SvxObjectNameDialog> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual void GetName(OUString& rName) override ;
    virtual void SetCheckNameHdl(const Link<AbstractSvxObjectNameDialog&,bool>& rLink) override;

private:
    std::unique_ptr<SvxObjectNameDialog> m_xDlg;
    Link<AbstractSvxObjectNameDialog&,bool> aCheckNameHdl;
    DECL_LINK(CheckNameHdl, SvxObjectNameDialog&, bool);
};

class AbstractSvxObjectTitleDescDialog_Impl :public AbstractSvxObjectTitleDescDialog
{
    std::unique_ptr<SvxObjectTitleDescDialog> m_xDlg;
public:
    explicit AbstractSvxObjectTitleDescDialog_Impl(std::unique_ptr<SvxObjectTitleDescDialog> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual void GetTitle(OUString& rName) override;
    virtual void GetDescription(OUString& rName) override;
};

class AbstractSvxMultiPathDialog_Impl : public AbstractSvxMultiPathDialog
{
    std::unique_ptr<SvxMultiPathDialog> m_xDlg;
public:
    explicit AbstractSvxMultiPathDialog_Impl(std::unique_ptr<SvxMultiPathDialog> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual OUString        GetPath() const override;
    virtual void            SetPath( const OUString& rPath ) override;
    virtual void            SetTitle( const OUString& rNewTitle ) override;
};

class SvxPathSelectDialog;
class AbstractSvxPathSelectDialog_Impl : public AbstractSvxMultiPathDialog
{
protected:
    std::unique_ptr<SvxPathSelectDialog> m_xDlg;
public:
    explicit AbstractSvxPathSelectDialog_Impl(std::unique_ptr<SvxPathSelectDialog> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
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
    std::unique_ptr<FmSearchDialog> m_xDlg;
public:
    explicit AbstractFmSearchDialog_Impl(std::unique_ptr<FmSearchDialog> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual void SetFoundHandler(const Link<FmFoundRecordInformation&,void>& lnk) override ;
    virtual void SetCanceledNotFoundHdl(const Link<FmFoundRecordInformation&,void>& lnk) override;
    virtual void SetActiveField(const OUString& strField) override;
};

class AbstractGraphicFilterDialog_Impl : public AbstractGraphicFilterDialog
{
    std::unique_ptr<GraphicFilterDialog> m_xDlg;
public:
    explicit AbstractGraphicFilterDialog_Impl(std::unique_ptr<GraphicFilterDialog> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual Graphic GetFilteredGraphic( const Graphic& rGraphic, double fScaleX, double fScaleY ) override;
};

class SvxAreaTabDialog;
class AbstractSvxAreaTabDialog_Impl : public AbstractSvxAreaTabDialog
{
    std::shared_ptr<SvxAreaTabDialog> m_xDlg;
public:
    explicit AbstractSvxAreaTabDialog_Impl(std::unique_ptr<SvxAreaTabDialog> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual bool StartExecuteAsync(AsyncContext &rCtx) override;
    virtual void SetCurPageId(const OString& rName) override;
    virtual const SfxItemSet* GetOutputItemSet() const override;
    virtual const sal_uInt16* GetInputRanges(const SfxItemPool& pItem) override;
    virtual void SetInputSet(const SfxItemSet* pInSet) override;
    virtual void SetText(const OUString& rStr) override;
};

class AbstractInsertObjectDialog_Impl : public SfxAbstractInsertObjectDialog
{
    std::unique_ptr<InsertObjectDialog_Impl> m_xDlg;
public:
    explicit AbstractInsertObjectDialog_Impl(std::unique_ptr<InsertObjectDialog_Impl> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual css::uno::Reference < css::embed::XEmbeddedObject > GetObject() override;
    virtual css::uno::Reference< css::io::XInputStream > GetIconIfIconified( OUString* pGraphicMediaType ) override;
    virtual bool IsCreateNew() override;
};

class AbstractPasteDialog_Impl : public SfxAbstractPasteDialog
{
    std::unique_ptr<SvPasteObjectDialog> m_xDlg;
public:
    explicit AbstractPasteDialog_Impl(std::unique_ptr<SvPasteObjectDialog> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
public:
    virtual void Insert( SotClipboardFormatId nFormat, const OUString & rFormatName ) override;
    virtual void SetObjName( const SvGlobalName & rClass, const OUString & rObjName ) override;
    virtual SotClipboardFormatId GetFormat( const TransferableDataHelper& aHelper ) override;
};

class AbstractLinksDialog_Impl : public SfxAbstractLinksDialog
{
protected:
    std::unique_ptr<SvBaseLinksDlg> m_xDlg;
public:
    explicit AbstractLinksDialog_Impl(std::unique_ptr<SvBaseLinksDlg> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
};

class SvxPostItDialog;
class AbstractSvxPostItDialog_Impl :public AbstractSvxPostItDialog
{
public:
    AbstractSvxPostItDialog_Impl(std::unique_ptr<SvxPostItDialog> pDlg)
        : m_xDlg(std::move(pDlg))
    {
    }
    virtual short               Execute() override;
    virtual void                SetText( const OUString& rStr ) override;  //From class Window
    virtual const SfxItemSet*   GetOutputItemSet() const override;
    virtual void                SetPrevHdl( const Link<AbstractSvxPostItDialog&,void>& rLink ) override;
    virtual void                SetNextHdl( const Link<AbstractSvxPostItDialog&,void>& rLink ) override;
    virtual void                EnableTravel(bool bNext, bool bPrev) override;
    virtual OUString            GetNote() override;
    virtual void                SetNote(const OUString& rTxt) override;
    virtual void                ShowLastAuthor(const OUString& rAuthor, const OUString& rDate) override;
    virtual void                DontChangeAuthor() override;
    virtual void                HideAuthor() override;
    virtual std::shared_ptr<weld::Dialog> GetDialog() override;
private:
    std::unique_ptr<SvxPostItDialog> m_xDlg;
    Link<AbstractSvxPostItDialog&,void> aNextHdl;
    Link<AbstractSvxPostItDialog&,void> aPrevHdl;
    DECL_LINK(NextHdl, SvxPostItDialog&, void);
    DECL_LINK(PrevHdl, SvxPostItDialog&, void);
};

class PasswordToOpenModifyDialog;
class AbstractPasswordToOpenModifyDialog_Impl : public AbstractPasswordToOpenModifyDialog
{
    std::unique_ptr<PasswordToOpenModifyDialog> m_xDlg;
public:
    explicit AbstractPasswordToOpenModifyDialog_Impl(std::unique_ptr<PasswordToOpenModifyDialog> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual OUString  GetPasswordToOpen() const override;
    virtual OUString  GetPasswordToModify() const override;
    virtual bool      IsRecommendToOpenReadonly() const override;
};

class SvxCharacterMap;
class AbstractSvxCharacterMapDialog_Impl : public SfxAbstractDialog
{
    std::unique_ptr<SvxCharacterMap> m_xDlg;
public:
    explicit AbstractSvxCharacterMapDialog_Impl(std::unique_ptr<SvxCharacterMap> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual const SfxItemSet* GetOutputItemSet() const override;
    virtual void  SetText(const OUString& rStr) override;
};

class ScreenshotAnnotationDlg;
class AbstractScreenshotAnnotationDlg_Impl : public AbstractScreenshotAnnotationDlg
{
    DECL_ABSTDLG_BASE(AbstractScreenshotAnnotationDlg_Impl, ScreenshotAnnotationDlg)
};

class SignatureLineDialog;
class AbstractSignatureLineDialog_Impl : public AbstractSignatureLineDialog
{
    std::unique_ptr<SignatureLineDialog> m_xDlg;

public:
    explicit AbstractSignatureLineDialog_Impl(std::unique_ptr<SignatureLineDialog> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
};

class SignSignatureLineDialog;
class AbstractSignSignatureLineDialog_Impl : public AbstractSignSignatureLineDialog
{
protected:
    std::unique_ptr<SignSignatureLineDialog> m_xDlg;

public:
    explicit AbstractSignSignatureLineDialog_Impl(std::unique_ptr<SignSignatureLineDialog> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
};

//AbstractDialogFactory_Impl implementations
class AbstractDialogFactory_Impl : public SvxAbstractDialogFactory
{
public:
    virtual VclPtr<VclAbstractDialog>    CreateVclDialog( vcl::Window* pParent, sal_uInt32 nResId ) override;

    virtual VclPtr<SfxAbstractDialog>    CreateSfxDialog( weld::Window* pParent,
                                            const SfxItemSet& rAttr,
                                            const SdrView* pView,
                                            sal_uInt32 nResId ) override;
    virtual VclPtr<SfxAbstractDialog>    CreateCharMapDialog(weld::Window* pParent,
                                                             const SfxItemSet& rAttr,
                                                             bool bInsert) override;
    virtual VclPtr<SfxAbstractDialog>    CreateEventConfigDialog(weld::Window* pParent,
                                                                 const SfxItemSet& rAttr,
                                                                 const css::uno::Reference< css::frame::XFrame >& _rxFrame) override;
    virtual VclPtr<VclAbstractDialog>    CreateFrameDialog(vcl::Window* pParent, const css::uno::Reference< css::frame::XFrame >& rxFrame,
                                                           sal_uInt32 nResId,
                                                           const OUString& rParameter ) override;
    virtual VclPtr<SfxAbstractTabDialog> CreateAutoCorrTabDialog(weld::Window* pParent, const SfxItemSet* pAttrSet) override;
    virtual VclPtr<SfxAbstractTabDialog> CreateCustomizeTabDialog(weld::Window* pParent,
                                            const SfxItemSet* pAttrSet,
                                            const css::uno::Reference< css::frame::XFrame >& xViewFrame ) override;
    virtual VclPtr<SfxAbstractTabDialog> CreateTextTabDialog( weld::Window* pParent,
                                            const SfxItemSet* pAttrSet,
                                            SdrView* pView ) override;
    virtual VclPtr<SfxAbstractTabDialog> CreateTabItemDialog(weld::Window* pParent, const SfxItemSet& rSet) override;
    virtual VclPtr<AbstractSvxCaptionDialog>
                                          CreateCaptionDialog(weld::Window* pParent,
                                            const SdrView* pView,
                                            SvxAnchorIds nAnchorTypes = SvxAnchorIds::NONE) override;
    virtual VclPtr<AbstractSvxDistributeDialog>
                                          CreateSvxDistributeDialog(weld::Window* pParent, const SfxItemSet& rAttr) override;
    virtual VclPtr<SfxAbstractInsertObjectDialog>
                                           CreateInsertObjectDialog(weld::Window* pParent, const OUString& rCommmand,
                                            const css::uno::Reference < css::embed::XStorage >& xStor,
                                            const SvObjectServerList* pList ) override;
    virtual VclPtr<VclAbstractDialog>      CreateEditObjectDialog(weld::Window* pParent, const OUString& rCommmand,
                                            const css::uno::Reference < css::embed::XEmbeddedObject >& xObj ) override;
    virtual VclPtr<SfxAbstractPasteDialog> CreatePasteDialog(weld::Window* pParent) override;
    virtual VclPtr<SfxAbstractLinksDialog> CreateLinksDialog(weld::Window* pParent, sfx2::LinkManager* pMgr, bool bHTML = false, sfx2::SvBaseLink* p=nullptr) override;

    virtual VclPtr<AbstractHangulHanjaConversionDialog> CreateHangulHanjaConversionDialog(weld::Window* pParent) override;
    virtual VclPtr<AbstractThesaurusDialog>  CreateThesaurusDialog( vcl::Window*, css::uno::Reference< css::linguistic2::XThesaurus >  xThesaurus,
                                                const OUString &rWord, LanguageType nLanguage ) override;

    virtual VclPtr<AbstractHyphenWordDialog> CreateHyphenWordDialog(weld::Window*,
                                                const OUString &rWord, LanguageType nLang,
                                                css::uno::Reference< css::linguistic2::XHyphenator >  &xHyphen,
                                                SvxSpellWrapper* pWrapper) override;

    virtual VclPtr<AbstractFmShowColsDialog> CreateFmShowColsDialog(weld::Window* pParent) override;
    virtual VclPtr<AbstractSvxZoomDialog> CreateSvxZoomDialog(weld::Window* pParent, const SfxItemSet& rCoreSet) override;
   // add for SvxBorderBackgroundDlg
    virtual VclPtr<SfxAbstractTabDialog> CreateSvxBorderBackgroundDlg(
       weld::Window* pParent,
       const SfxItemSet& rCoreSet,
       bool bEnableDrawingLayerFillStyles) override;

    virtual VclPtr<AbstractSvxTransformTabDialog> CreateSvxTransformTabDialog(weld::Window* pParent,
                                                                              const SfxItemSet* pAttr,
                                                                              const SdrView* pView,
                                                                              SvxAnchorIds nAnchorTypes = SvxAnchorIds::NONE) override ;
    virtual VclPtr<SfxAbstractTabDialog> CreateSchTransformTabDialog(weld::Window* pParent,
                                                                     const SfxItemSet* pAttr,
                                                                     const SdrView* pSdrView,
                                                                     bool bSizeTabPage) override;
    virtual VclPtr<AbstractSpellDialog>  CreateSvxSpellDialog(
                            vcl::Window* pParent,
                            SfxBindings* pBindings,
                            svx::SpellDialogChildWindow* pSpellChildWindow ) override;

    virtual VclPtr<VclAbstractDialog> CreateActualizeProgressDialog(weld::Window* pParent, GalleryTheme* pThm) override;
    virtual VclPtr<AbstractTitleDialog> CreateTitleDialog(weld::Window* pParent, const OUString& rOldText) override;
    virtual VclPtr<AbstractGalleryIdDialog> CreateGalleryIdDialog(weld::Window* pParent,
                                            GalleryTheme* pThm) override;
    virtual VclPtr<VclAbstractDialog> CreateGalleryThemePropertiesDialog(weld::Window* pParent,
                                            ExchangeData* pData,
                                            SfxItemSet* pItemSet) override;
    virtual VclPtr<AbstractURLDlg> CreateURLDialog( vcl::Window* pParent,
                                            const OUString& rURL, const OUString& rAltText, const OUString& rDescription,
                                            const OUString& rTarget, const OUString& rName,
                                            TargetList& rTargetList ) override;

    virtual VclPtr<VclAbstractDialog>      CreateSvxSearchAttributeDialog(weld::Window* pParent,
                                            SearchAttrItemList& rLst,
                                            const sal_uInt16* pWhRanges) override;
    virtual VclPtr<AbstractSvxSearchSimilarityDialog> CreateSvxSearchSimilarityDialog( weld::Window* pParent,
                                                            bool bRelax,
                                                            sal_uInt16 nOther,
                                                            sal_uInt16 nShorter,
                                                            sal_uInt16 nLonger) override;
    virtual VclPtr<AbstractSvxJSearchOptionsDialog> CreateSvxJSearchOptionsDialog(weld::Window* pParent,
                                                                                  const SfxItemSet& rOptionsSet,
                                                                                  TransliterationFlags nInitialFlags) override;
    virtual VclPtr<AbstractFmInputRecordNoDialog> CreateFmInputRecordNoDialog(weld::Window* pParent) override;
    virtual VclPtr<AbstractSvxNewDictionaryDialog> CreateSvxNewDictionaryDialog(weld::Window* pParent) override;
    virtual VclPtr<VclAbstractDialog>     CreateSvxEditDictionaryDialog(weld::Window* pParent, const OUString& rName) override;
    virtual VclPtr<AbstractSvxNameDialog> CreateSvxNameDialog(weld::Window* pParent,
                                            const OUString& rName, const OUString& rDesc) override;
    // #i68101#
    virtual VclPtr<AbstractSvxObjectNameDialog> CreateSvxObjectNameDialog(weld::Window* pParent, const OUString& rName) override;
    virtual VclPtr<AbstractSvxObjectTitleDescDialog> CreateSvxObjectTitleDescDialog(weld::Window* pParent, const OUString& rTitle, const OUString& rDescription) override;

    virtual VclPtr<AbstractSvxMultiPathDialog>    CreateSvxMultiPathDialog(weld::Window* pParent) override;
    virtual VclPtr<AbstractSvxMultiPathDialog>    CreateSvxPathSelectDialog(weld::Window* pParent) override;
    virtual VclPtr<AbstractSvxHpLinkDlg>          CreateSvxHpLinkDlg(vcl::Window* pParent, SfxBindings* pBindings) override;
    virtual VclPtr<AbstractFmSearchDialog>         CreateFmSearchDialog(weld::Window* pParent,
                                                        const OUString& strInitialText,
                                                        const std::vector< OUString >& _rContexts,
                                                        sal_Int16 nInitialContext,
                                                        const Link<FmSearchContext&,sal_uInt32>& lnkContextSupplier) override;
    virtual VclPtr<AbstractGraphicFilterDialog>   CreateGraphicFilterEmboss(weld::Window* pParent,
                                                const Graphic& rGraphic) override;
    virtual VclPtr<AbstractGraphicFilterDialog>   CreateGraphicFilterPoster(weld::Window* pParent,
                                                const Graphic& rGraphic) override;
    virtual VclPtr<AbstractGraphicFilterDialog>   CreateGraphicFilterSepia(weld::Window* pParent,
                                                const Graphic& rGraphic) override;
    virtual VclPtr<AbstractGraphicFilterDialog>   CreateGraphicFilterSmooth(weld::Window* pParent,
                                                const Graphic& rGraphic, double nRadius) override;
    virtual VclPtr<AbstractGraphicFilterDialog>  CreateGraphicFilterSolarize(weld::Window* pParent,
                                                const Graphic& rGraphic) override;
    virtual VclPtr<AbstractGraphicFilterDialog>   CreateGraphicFilterMosaic(weld::Window* pParent,
                                                const Graphic& rGraphic) override;
    virtual VclPtr<AbstractSvxAreaTabDialog>       CreateSvxAreaTabDialog(weld::Window* pParent,
                                                                          const SfxItemSet* pAttr,
                                                                          SdrModel* pModel,
                                                                          bool bShadow) override;
    virtual VclPtr<SfxAbstractTabDialog>           CreateSvxLineTabDialog(weld::Window* pParent, const SfxItemSet* pAttr,
                                                                 SdrModel* pModel,
                                                                 const SdrObject* pObj,
                                                                 bool bHasObj ) override;
    virtual VclPtr<AbstractSvxPostItDialog>        CreateSvxPostItDialog( weld::Window* pParent,
                                                                        const SfxItemSet& rCoreSet,
                                                                        bool bPrevNext = false ) override;

    // For TabPage
    virtual CreateTabPage               GetTabPageCreatorFunc( sal_uInt16 nId ) override;

    virtual GetTabPageRanges            GetTabPageRangesFunc( sal_uInt16 nId ) override;
    virtual DialogGetRanges             GetDialogGetRangesFunc() override;
    virtual VclPtr<VclAbstractDialog>   CreateSvxScriptOrgDialog(weld::Window* pParent, const OUString& rLanguage) override;

    virtual VclPtr<AbstractScriptSelectorDialog> CreateScriptSelectorDialog(weld::Window* pParent,
            const css::uno::Reference< css::frame::XFrame >& rxFrame) override;

    virtual VclPtr<VclAbstractDialog> CreateScriptErrorDialog(const css::uno::Any& rException) override;

    virtual VclPtr<VclAbstractDialog>  CreateSvxMacroAssignDlg(
                weld::Window* _pParent,
                const css::uno::Reference< css::frame::XFrame >& _rxDocumentFrame,
                const bool _bUnoDialogMode,
                const css::uno::Reference< css::container::XNameReplace >& _rxEvents,
                const sal_uInt16 _nInitiallySelectedEvent
            ) override;

    virtual VclPtr<SfxAbstractTabDialog> CreateSvxFormatCellsDialog(weld::Window* pParent, const SfxItemSet* pAttr, const SdrModel& rModel, const SdrObject* pObj) override;

    virtual VclPtr<SvxAbstractSplitTableDialog> CreateSvxSplitTableDialog(weld::Window* pParent, bool bIsTableVertical, long nMaxVertical) override;

    virtual VclPtr<SvxAbstractNewTableDialog> CreateSvxNewTableDialog(weld::Window* pParent) override ;

    virtual VclPtr<VclAbstractDialog>          CreateOptionsDialog(
        vcl::Window* pParent, const OUString& rExtensionId ) override;

    virtual VclPtr<SvxAbstractInsRowColDlg> CreateSvxInsRowColDlg(weld::Window* pParent, bool bCol, const OString& rHelpId) override;

    virtual VclPtr<AbstractPasswordToOpenModifyDialog> CreatePasswordToOpenModifyDialog(weld::Window * pParent, sal_uInt16 nMaxPasswdLen, bool bIsPasswordToModify) override;

    virtual VclPtr<AbstractScreenshotAnnotationDlg> CreateScreenshotAnnotationDlg(vcl::Window * pParent, Dialog& rParentDialog) override;

    virtual VclPtr<AbstractSignatureLineDialog>
    CreateSignatureLineDialog(weld::Window* pParent,
                              const css::uno::Reference<css::frame::XModel> xModel, bool bEditExisting) override;

    virtual VclPtr<AbstractSignSignatureLineDialog>
    CreateSignSignatureLineDialog(weld::Window* pParent,
                                  const css::uno::Reference<css::frame::XModel> xModel) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
