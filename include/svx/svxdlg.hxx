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
#ifndef INCLUDED_SVX_SVXDLG_HXX
#define INCLUDED_SVX_SVXDLG_HXX

#include <sfx2/sfxdlg.hxx>
#include <svx/zoom_def.hxx>
#include <editeng/edtdlg.hxx>

class SdrModel;
class SdrView;

#include <svx/dstribut_enum.hxx>
#include <svx/rectenum.hxx>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/container/XNameReplace.hpp>
#include <svx/svxdllapi.h>
#include <vector>

namespace com{namespace sun{namespace star{
namespace linguistic2{
    class XDictionary;
    class XSpellChecker1;
    class XSpellChecker;
    class XThesaurus;
    class XHyphenator;
}}}}

class SvxSpellWrapper;
typedef VclPtr<SfxTabPage> (*CreateSvxDistributePage)(vcl::Window *pParent, const SfxItemSet &rAttrSet, SvxDistributeHorizontal eHor, SvxDistributeVertical eVer);
typedef const sal_uInt16*  (*DialogGetRanges)();

struct ExchangeData;
class INetURLObject;
class GalleryTheme;
class SvxHyperlinkTabPageBase;
class SearchAttrItemList;
class FmFormShell;
class Graphic;
class SdrObject;
class SvxSpellWrapper;
struct FmSearchContext;

typedef ::std::vector< OUString > TargetList;

namespace svx{ class SpellDialogChildWindow;}

class AbstractSvxDistributeDialog :public VclAbstractDialog
{
public:
    virtual SvxDistributeHorizontal GetDistributeHor() const = 0;
    virtual SvxDistributeVertical GetDistributeVer() const = 0;
};

class AbstractFmShowColsDialog : public VclAbstractDialog
{
public:
     virtual void SetColumns(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer>& xCols)= 0;
};

class AbstractSvxZoomDialog : public VclAbstractDialog
{
public:
    virtual void    SetLimits( sal_uInt16 nMin, sal_uInt16 nMax ) = 0;
    virtual void    HideButton( ZoomButtonId nBtnId )= 0;
    virtual const SfxItemSet*   GetOutputItemSet() const = 0 ;
};

class AbstractSpellDialog : public VclAbstractDialog
{
public:
    virtual void        Invalidate() = 0;
    virtual vcl::Window*     GetWindow()  = 0;
    virtual SfxBindings& GetBindings() = 0;
};

typedef sal_IntPtr (*PLinkStub)( void*, void* );

class AbstractSearchProgress :public VclAbstractRefreshableDialog
{
};

class AbstractTakeProgress :public VclAbstractRefreshableDialog
{
};

class AbstractTitleDialog :public VclAbstractDialog
{
public:
    virtual OUString  GetTitle() const =0;
};

class AbstractGalleryIdDialog :public VclAbstractDialog
{
public:
    virtual sal_uIntPtr GetId() const =0;
};

class AbstractURLDlg :public VclAbstractDialog
{
public:
    virtual OUString      GetURL() const = 0;
    virtual OUString      GetAltText() const = 0;
    virtual OUString      GetDesc() const = 0;
    virtual OUString      GetTarget() const = 0;
    virtual OUString      GetName() const = 0;
};

class AbstractSvxHlinkDlgMarkWnd :public VclAbstractDialog
{
};

class AbstractSvxSearchSimilarityDialog :public VclAbstractDialog
{
public:
    virtual sal_uInt16  GetOther() =0 ;
    virtual sal_uInt16  GetShorter() =0 ;
    virtual sal_uInt16  GetLonger() =0 ;
    virtual bool        IsRelaxed() =0 ;
};

class AbstractSvxJSearchOptionsDialog :public VclAbstractDialog
{
public:
    virtual sal_Int32           GetTransliterationFlags() const = 0;
};

class AbstractFmInputRecordNoDialog :public VclAbstractDialog
{
public:
    virtual void SetValue(long dNew) = 0;
    virtual long GetValue() const = 0;
};

class AbstractSvxNewDictionaryDialog :public VclAbstractDialog
{
public:
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XDictionary >  GetNewDictionary() = 0;
};

class AbstractSvxNameDialog :public VclAbstractDialog
{
public:
    virtual void    GetName( OUString& rName ) = 0;
    virtual void    SetCheckNameHdl( const Link<AbstractSvxNameDialog&,bool>& rLink, bool bCheckImmediately = false ) = 0;
    virtual void    SetEditHelpId(const OString&) = 0;
    //from class Window
    virtual void    SetHelpId( const OString& ) = 0;
    virtual void    SetText( const OUString& rStr ) = 0;
};

class AbstractSvxObjectNameDialog :public VclAbstractDialog
{
public:
    virtual void GetName(OUString& rName) = 0;
    virtual void SetCheckNameHdl(const Link<>& rLink, bool bCheckImmediately = false) = 0;
};

class AbstractSvxObjectTitleDescDialog :public VclAbstractDialog
{
public:
    virtual void GetTitle(OUString& rTitle) = 0;
    virtual void GetDescription(OUString& rDescription) = 0;
};

class AbstractSvxMessDialog :public VclAbstractDialog
{
};

class AbstractSvxMultiPathDialog : public VclAbstractDialog
{
public:
    virtual OUString        GetPath() const = 0;
    virtual void            SetPath( const OUString& rPath ) = 0;
    virtual void            SetTitle( const OUString& rNewTitle ) = 0;
};

class AbstractSvxHpLinkDlg : public VclAbstractDialog
{
public:
    virtual vcl::Window*     GetWindow()  = 0;
    virtual bool       QueryClose() = 0;
};

class AbstractFmSearchDialog :public VclAbstractDialog
{
public:
    virtual void SetFoundHandler(const Link<>& lnk) = 0;
    virtual void SetCanceledNotFoundHdl(const Link<>& lnk)=0;
    virtual void SetActiveField(const OUString& strField)=0;
};

class AbstractGraphicFilterDialog :public VclAbstractDialog
{
public:
    virtual Graphic GetFilteredGraphic( const Graphic& rGraphic, double fScaleX, double fScaleY ) = 0;
};

class AbstractSvxAreaTabDialog :public SfxAbstractTabDialog
{
};

struct SvxSwFrameValidation;
class AbstractSvxTransformTabDialog : public SfxAbstractTabDialog
{
public:
    virtual void SetValidateFramePosLink( const Link<SvxSwFrameValidation&,void>& rLink ) = 0;
};

class AbstractSvxCaptionDialog : public SfxAbstractTabDialog
{
public:
    virtual void SetValidateFramePosLink( const Link<SvxSwFrameValidation&,void>& rLink ) = 0;
};

class AbstractSvxPostItDialog :public VclAbstractDialog
{
public:
    virtual void                SetText( const OUString& rStr ) = 0;  //From class Window
    virtual const SfxItemSet*   GetOutputItemSet() const = 0;
    virtual void                SetPrevHdl( const Link<>& rLink ) = 0;
    virtual void                SetNextHdl( const Link<>& rLink ) = 0;
    virtual void                EnableTravel(bool bNext, bool bPrev) = 0;
    virtual OUString            GetNote()  = 0;
    virtual void                SetNote(const OUString& rTxt)  = 0;
    virtual void                ShowLastAuthor(const OUString& rAuthor, const OUString& rDate) = 0;
    virtual void                DontChangeAuthor() = 0;
    virtual void                HideAuthor() = 0;
    virtual vcl::Window *            GetWindow() = 0;
};

class SvxAbstractSplittTableDialog : public VclAbstractDialog
{
public:
    virtual bool IsHorizontal() const = 0;
    virtual bool IsProportional() const = 0;
    virtual long GetCount() const = 0;
};

class SvxAbstractNewTableDialog : public VclAbstractDialog
{
public:
    virtual sal_Int32 getRows() const = 0;
    virtual sal_Int32 getColumns() const = 0;
};

class SvxAbstractInsRowColDlg : public VclAbstractDialog
{
public:
    virtual bool isInsertBefore() const = 0;
    virtual sal_uInt16 getInsertCount() const = 0;
};

class SVX_DLLPUBLIC SvxAbstractDialogFactory : public SfxAbstractDialogFactory, public EditAbstractDialogFactory
{
public:
    static SvxAbstractDialogFactory* Create();

    // define dtor as this will create typeinfo and in svx library and export vtable
    virtual                             ~SvxAbstractDialogFactory();

    virtual SfxAbstractTabDialog*       CreateTextTabDialog( vcl::Window* pParent,
                                            const SfxItemSet* pAttrSet,
                                            SdrView* pView,
                                            SdrModel* pModel=0 ) = 0 ;

    virtual AbstractSvxCaptionDialog* CreateCaptionDialog( vcl::Window* pParent,
                                        const SdrView* pView,
                                        sal_uInt16 nAnchorTypes = 0 ) = 0;

    virtual AbstractSvxDistributeDialog*    CreateSvxDistributeDialog(vcl::Window* pParent,
                                            const SfxItemSet& rAttr,
                                            SvxDistributeHorizontal eHor = SvxDistributeHorizontalNone,
                                            SvxDistributeVertical eVer = SvxDistributeVerticalNone)= 0;

    virtual AbstractFmShowColsDialog * CreateFmShowColsDialog( vcl::Window* pParent ) = 0;

    virtual AbstractSvxZoomDialog * CreateSvxZoomDialog( vcl::Window* pParent,
                                            const SfxItemSet& rCoreSet )=0;

    virtual AbstractSpellDialog *   CreateSvxSpellDialog(vcl::Window* pParent,
                                            SfxBindings* pBindings,
                                            svx::SpellDialogChildWindow* pSpellChildWindow )=0;

    virtual VclAbstractRefreshableDialog * CreateActualizeProgressDialog( vcl::Window* pParent,
                                            GalleryTheme* pThm ) = 0;
    virtual AbstractTitleDialog * CreateTitleDialog( vcl::Window* pParent,
                                             const OUString& rOldText ) = 0;
    virtual AbstractGalleryIdDialog * CreateGalleryIdDialog( vcl::Window* pParent,
                                            GalleryTheme* pThm ) = 0;
    virtual VclAbstractDialog2 * CreateGalleryThemePropertiesDialog( vcl::Window* pParent,
                                            ExchangeData* pData,
                                            SfxItemSet* pItemSet ) = 0;
    virtual AbstractURLDlg * CreateURLDialog( vcl::Window* pParent,
                                            const OUString& rURL, const OUString& rAltText, const OUString& rDescription,
                                            const OUString& rTarget, const OUString& rName,
                                            TargetList& rTargetList ) = 0;

    virtual SfxAbstractTabDialog* CreateTabItemDialog(vcl::Window* pParent,
                                            const SfxItemSet& rSet) = 0;
    virtual VclAbstractDialog*      CreateSvxSearchAttributeDialog( vcl::Window* pParent,
                                            SearchAttrItemList& rLst,
                                            const sal_uInt16* pWhRanges)=0;
    virtual AbstractSvxSearchSimilarityDialog * CreateSvxSearchSimilarityDialog( vcl::Window* pParent,
                                                            bool bRelax,
                                                            sal_uInt16 nOther,
                                                            sal_uInt16 nShorter,
                                                            sal_uInt16 nLonger ) = 0;
    //UUUU add for SvxBorderBackgroundDlg
    virtual SfxAbstractTabDialog* CreateSvxBorderBackgroundDlg(
        vcl::Window* pParent,
        const SfxItemSet& rCoreSet,
        bool bEnableSelector = false,
        bool bEnableDrawingLayerFillStyles = false) = 0;

    virtual AbstractSvxTransformTabDialog* CreateSvxTransformTabDialog( vcl::Window* pParent,
                                                                const SfxItemSet* pAttr,
                                                                const SdrView* pView,
                                                                sal_uInt16 nAnchorTypes = 0) = 0;
    virtual SfxAbstractTabDialog* CreateSchTransformTabDialog( vcl::Window* pParent,
                                                                const SfxItemSet* pAttr,
                                                                const SdrView* pSdrView,
                                                                sal_uInt32 nResId,
                                                                bool bSizeTabPage = false
                                                                 )=0;
    virtual AbstractSvxJSearchOptionsDialog * CreateSvxJSearchOptionsDialog( vcl::Window* pParent,
                                                            const SfxItemSet& rOptionsSet,
                                                            sal_Int32 nInitialFlags )=0;
    virtual AbstractFmInputRecordNoDialog * CreateFmInputRecordNoDialog( vcl::Window* pParent ) = 0;
    virtual AbstractSvxNewDictionaryDialog* CreateSvxNewDictionaryDialog( vcl::Window* pParent,
                                            ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XSpellChecker1 >  &xSpl ) = 0;
    virtual VclAbstractDialog *     CreateSvxEditDictionaryDialog( vcl::Window* pParent,
                                            const OUString& rName,
                                            ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XSpellChecker1> &xSpl,
                                            sal_uInt32 nResId) = 0;
    virtual AbstractSvxNameDialog *     CreateSvxNameDialog( vcl::Window* pParent,
                                            const OUString& rName, const OUString& rDesc ) = 0;

    // #i68101#
    virtual AbstractSvxObjectNameDialog* CreateSvxObjectNameDialog(vcl::Window* pParent, const OUString& rName ) = 0;
    virtual AbstractSvxObjectTitleDescDialog* CreateSvxObjectTitleDescDialog(vcl::Window* pParent, const OUString& rTitle, const OUString& rDescription) = 0;

    virtual AbstractSvxMultiPathDialog *    CreateSvxMultiPathDialog(vcl::Window* pParent) = 0 ;
    virtual AbstractSvxMultiPathDialog *    CreateSvxPathSelectDialog(vcl::Window* pParent) = 0 ;
    virtual AbstractSvxHpLinkDlg *  CreateSvxHpLinkDlg (vcl::Window* pParent,
                                            SfxBindings* pBindings,
                                            sal_uInt32 nResId)=0;
    virtual AbstractFmSearchDialog* CreateFmSearchDialog(vcl::Window* pParent,
                                                        const OUString& strInitialText,
                                                        const ::std::vector< OUString >& _rContexts,
                                                        sal_Int16 nInitialContext,
                                                        const Link<FmSearchContext&,sal_uInt32>& lnkContextSupplier)=0;
    virtual AbstractGraphicFilterDialog *   CreateGraphicFilterEmboss(vcl::Window* pParent,
                                                const Graphic& rGraphic,
                                                RECT_POINT eLightSource)=0;
    virtual AbstractGraphicFilterDialog *   CreateGraphicFilterPoster(vcl::Window* pParent,
                                                const Graphic& rGraphic,
                                                sal_uInt16 nCount)=0;
    virtual AbstractGraphicFilterDialog *   CreateGraphicFilterSepia (vcl::Window* pParent,
                                                const Graphic& rGraphic,
                                                sal_uInt16 nCount)=0;
    virtual AbstractGraphicFilterDialog *   CreateGraphicFilterSmooth (vcl::Window* pParent,
                                                const Graphic& rGraphic,
                                                double nRadius)=0;
    virtual AbstractGraphicFilterDialog *   CreateGraphicFilterSolarize (vcl::Window* pParent,
                                                const Graphic& rGraphic,
                                                sal_uInt8 nGreyThreshold, bool bInvert)=0;
    virtual AbstractGraphicFilterDialog *   CreateGraphicFilterMosaic (vcl::Window* pParent,
                                                const Graphic& rGraphic,
                                                sal_uInt16 nTileWidth, sal_uInt16 nTileHeight, bool bEnhanceEdges)=0;
    virtual AbstractSvxAreaTabDialog*       CreateSvxAreaTabDialog( vcl::Window* pParent,
                                                            const SfxItemSet* pAttr,
                                                            SdrModel* pModel,
                                                            bool bShadow) = 0 ;
    virtual SfxAbstractTabDialog*           CreateSvxLineTabDialog( vcl::Window* pParent, const SfxItemSet* pAttr, //add forSvxLineTabDialog
                                                                 SdrModel* pModel,
                                                                 const SdrObject* pObj = NULL,
                                                                 bool bHasObj = true )=0;
    virtual SfxAbstractDialog*              CreateSfxDialog( vcl::Window* pParent,
                                                                        const SfxItemSet& rAttr,
                                                                        const SdrView* pView,
                                                                        sal_uInt32 nResId
                                                                        )=0;
    virtual SfxAbstractDialog*              CreateSfxDialog( vcl::Window* pParent,
                                                                        const SfxItemSet& rAttr,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _rxFrame,
                                                                        sal_uInt32 nResId
                                                                        )=0;
    virtual AbstractSvxPostItDialog*        CreateSvxPostItDialog( vcl::Window* pParent,
                                                                        const SfxItemSet& rCoreSet,
                                                                        bool bPrevNext = false) = 0;
    virtual VclAbstractDialog*          CreateSvxScriptOrgDialog( vcl::Window* pParent, const OUString& rLanguage ) SAL_OVERRIDE = 0;

    virtual DialogGetRanges             GetDialogGetRangesFunc() = 0;

    virtual AbstractScriptSelectorDialog*
        CreateScriptSelectorDialog(
            vcl::Window* pParent,
            bool bShowSlots,
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _rxFrame
        ) SAL_OVERRIDE = 0;

    virtual VclAbstractDialog* CreateScriptErrorDialog(
            vcl::Window* pParent, const css::uno::Any& rException) SAL_OVERRIDE = 0;

    virtual VclAbstractDialog*  CreateSvxMacroAssignDlg(
                vcl::Window* _pParent,
                const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _rxDocumentFrame,
                const bool _bUnoDialogMode,
                const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameReplace >& _rxEvents,
                const sal_uInt16 _nInitiallySelectedEvent
            ) = 0;

    virtual SfxAbstractTabDialog* CreateSvxFormatCellsDialog( vcl::Window* pParent, const SfxItemSet* pAttr, SdrModel* pModel, const SdrObject* pObj = NULL )=0;

    virtual SvxAbstractSplittTableDialog* CreateSvxSplittTableDialog( vcl::Window* pParent, bool bIsTableVertical, long nMaxVertical, long nMaxHorizontal )=0;

    virtual SvxAbstractNewTableDialog* CreateSvxNewTableDialog( vcl::Window* pParent ) = 0;

    virtual SvxAbstractInsRowColDlg* CreateSvxInsRowColDlg( vcl::Window* pParent, bool bCol, const OString& sHelpId ) = 0;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
