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
#ifndef _SVX_DIALOG_HXX
#define _SVX_DIALOG_HXX

#include <sfx2/sfxdlg.hxx>
#include <editeng/edtdlg.hxx>

class SdrModel;
class SdrView;

#include <svx/dstribut_enum.hxx>
#include <svx/rectenum.hxx>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/container/XNameReplace.hpp>
#include "svx/svxdllapi.h"
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
typedef SfxTabPage* (*CreateSvxDistributePage)(Window *pParent, const SfxItemSet &rAttrSet, SvxDistributeHorizontal eHor, SvxDistributeVertical eVer);
typedef sal_uInt16*     (*DialogGetRanges)();

struct ExchangeData;
class INetURLObject;
class GalleryTheme;
class SvxHyperlinkTabPageBase;
class SearchAttrItemList;
class FmFormShell;
class Graphic;
class SdrObject;
class SvxSpellWrapper;

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
    virtual void    HideButton( sal_uInt16 nBtnId )= 0;
    virtual const SfxItemSet*   GetOutputItemSet() const = 0 ;
};

class AbstractSpellDialog : public VclAbstractDialog //
{
 public:
    virtual void        SetLanguage( sal_uInt16 nLang ) = 0;
    virtual sal_Bool    Close() = 0;
    virtual void        Invalidate() = 0;
    virtual Window*     GetWindow()  = 0;
    virtual SfxBindings& GetBindings() = 0;
};

typedef long (*PLinkStub)( void*, void* );

class AbstractSearchProgress :public VclAbstractRefreshableDialog
{
public:
    virtual void    SetFileType( const String& rType ) = 0;
    virtual void    SetDirectory( const INetURLObject& rURL ) = 0;
    virtual PLinkStub   GetLinkStubCleanUpHdl() = 0;
};

class AbstractTakeProgress :public VclAbstractRefreshableDialog
{
public:
    virtual void    SetFile( const INetURLObject& rURL ) = 0;
    virtual PLinkStub   GetLinkStubCleanUpHdl() = 0;
};

class AbstractTitleDialog :public VclAbstractDialog
{
public:
    virtual String  GetTitle() const =0;
};

class AbstractGalleryIdDialog :public VclAbstractDialog
{
public:
    virtual sal_uIntPtr GetId() const =0;
};

class AbstractURLDlg :public VclAbstractDialog
{
public:
    virtual String      GetURL() const = 0;
    virtual String      GetAltText() const = 0;
    virtual String      GetDesc() const = 0;
    virtual String      GetTarget() const = 0;
    virtual String      GetName() const = 0;
};

class AbstractSvxHlinkDlgMarkWnd :public VclAbstractDialog
{
public:
    virtual sal_Bool    MoveTo ( Point aNewPos )const = 0;
    virtual sal_Bool    ConnectToDialog( sal_Bool bDoit = sal_True )const = 0;
    virtual void    RefreshTree ( String aStrURL ) = 0;
    virtual void    SelectEntry ( String aStrMark ) = 0;
    virtual sal_uInt16  SetError( sal_uInt16 nError) = 0;
    // in class Window
    virtual void    SetSizePixel( const Size& rNewSize ) = 0;
    virtual Size    GetSizePixel() const = 0;
    virtual void    Hide( sal_uInt16 nFlags = 0 ) = 0;
    virtual sal_Bool    IsVisible() const = 0;
    virtual void    Invalidate( sal_uInt16 nFlags = 0 ) = 0;
};

class AbstractSvxSearchFormatDialog : public SfxAbstractTabDialog   //for SvxSearchFormatDialog
{
public:
    //From class Window.
    virtual void        SetText( const OUString& rStr ) =0 ;
    virtual OUString    GetText() const =0 ;
};

class AbstractSvxSearchSimilarityDialog :public VclAbstractDialog
{
public:
    virtual sal_uInt16  GetOther() =0 ;
    virtual sal_uInt16  GetShorter() =0 ;
    virtual sal_uInt16  GetLonger() =0 ;
    virtual sal_Bool    IsRelaxed() =0 ;
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
    virtual void    SetCheckNameHdl( const Link& rLink, bool bCheckImmediately = false ) = 0;
    virtual void    SetEditHelpId(const OString&) = 0;
    //from class Window
    virtual void    SetHelpId( const OString& ) = 0;
    virtual void    SetText( const OUString& rStr ) = 0;
};

class AbstractSvxObjectNameDialog :public VclAbstractDialog
{
public:
    virtual void GetName(OUString& rName) = 0;
    virtual void SetCheckNameHdl(const Link& rLink, bool bCheckImmediately = false) = 0;
};

class AbstractSvxObjectTitleDescDialog :public VclAbstractDialog
{
public:
    virtual void GetTitle(OUString& rTitle) = 0;
    virtual void GetDescription(OUString& rDescription) = 0;
};

class AbstractSvxMessDialog :public VclAbstractDialog
{
public:
    virtual void    SetButtonText( sal_uInt16 nBtnId, const String& rNewTxt ) = 0;
};

class AbstractSvxMultiPathDialog :public VclAbstractDialog
{
public:
    virtual String          GetPath() const = 0;
    virtual void            SetPath( const String& rPath ) = 0;
    virtual void            EnableRadioButtonMode() = 0;
    virtual void            SetTitle( const String& rNewTitle ) = 0;
};

class AbstractSvxMultiFileDialog :public AbstractSvxMultiPathDialog
{
public:
    virtual String  GetFiles() const = 0;
    virtual void    SetFiles( const String& rPath ) = 0;
    //from SvxMultiPathDialog
    virtual void    SetClassPathMode() = 0;
    //From Class Window
    virtual void    SetHelpId( const OString& ) = 0;
};

class AbstractSvxHpLinkDlg : public VclAbstractDialog
{
public:
    virtual Window*     GetWindow()  = 0;
    virtual sal_Bool    QueryClose() = 0;
};

class AbstractFmSearchDialog :public VclAbstractDialog
{
public:
    virtual void SetFoundHandler(const Link& lnk) = 0;
    virtual void SetCanceledNotFoundHdl(const Link& lnk)=0;
    virtual void SetActiveField(const String& strField)=0;
};

class AbstractGraphicFilterDialog :public VclAbstractDialog
{
public:
    virtual Graphic GetFilteredGraphic( const Graphic& rGraphic, double fScaleX, double fScaleY ) = 0;
};

class AbstractSvxAreaTabDialog :public SfxAbstractTabDialog
{
};

class AbstractSvxTransformTabDialog : public SfxAbstractTabDialog
{
public:
    virtual void SetValidateFramePosLink( const Link& rLink ) = 0;
};

class AbstractSvxCaptionDialog : public SfxAbstractTabDialog
{
public:
    virtual void SetValidateFramePosLink( const Link& rLink ) = 0;
};

class AbstractSvxPostItDialog :public VclAbstractDialog
{
public:
    virtual void                SetText( const OUString& rStr ) = 0;  //From class Window
    virtual const SfxItemSet*   GetOutputItemSet() const = 0;
    virtual void                SetPrevHdl( const Link& rLink ) = 0;
    virtual void                SetNextHdl( const Link& rLink ) = 0;
    virtual void                EnableTravel(sal_Bool bNext, sal_Bool bPrev) = 0;
    virtual String              GetNote()  = 0;
    virtual void                SetNote(const String& rTxt)  = 0;
    virtual void                ShowLastAuthor(const String& rAuthor, const String& rDate) = 0;
    virtual void                DontChangeAuthor() = 0;
    virtual void                HideAuthor() = 0;
    virtual void                SetReadonlyPostIt(sal_Bool bDisable) = 0;
    virtual sal_Bool                IsOkEnabled() const  = 0;
    virtual Window *            GetWindow() = 0;
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

    virtual SfxAbstractTabDialog*       CreateTextTabDialog( Window* pParent,
                                            const SfxItemSet* pAttrSet,
                                            SdrView* pView,
                                            SdrModel* pModel=0 ) = 0 ;

    virtual AbstractSvxCaptionDialog* CreateCaptionDialog( Window* pParent,
                                        const SdrView* pView,
                                        sal_uInt16 nAnchorTypes = 0 ) = 0;

    virtual AbstractSvxDistributeDialog*    CreateSvxDistributeDialog(Window* pParent,
                                            const SfxItemSet& rAttr,
                                            SvxDistributeHorizontal eHor = SvxDistributeHorizontalNone,
                                            SvxDistributeVertical eVer = SvxDistributeVerticalNone)= 0;

    virtual AbstractFmShowColsDialog * CreateFmShowColsDialog( Window* pParent ) = 0;

    virtual AbstractSvxZoomDialog * CreateSvxZoomDialog( Window* pParent,
                                            const SfxItemSet& rCoreSet )=0;

    virtual AbstractSpellDialog *   CreateSvxSpellDialog(Window* pParent,
                                            SfxBindings* pBindings,
                                            svx::SpellDialogChildWindow* pSpellChildWindow )=0;

    virtual VclAbstractRefreshableDialog * CreateActualizeProgressDialog( Window* pParent,
                                            GalleryTheme* pThm ) = 0;
    virtual AbstractSearchProgress * CreateSearchProgressDialog( Window* pParent,
                                            const INetURLObject& rStartURL ) = 0;
    virtual AbstractTakeProgress * CreateTakeProgressDialog( Window* pParent ) = 0;
    virtual AbstractTitleDialog * CreateTitleDialog( Window* pParent,
                                             const String& rOldText ) = 0;
    virtual AbstractGalleryIdDialog * CreateGalleryIdDialog( Window* pParent,
                                            GalleryTheme* pThm ) = 0;
    virtual VclAbstractDialog2 * CreateGalleryThemePropertiesDialog( Window* pParent,
                                            ExchangeData* pData,
                                            SfxItemSet* pItemSet ) = 0;
    virtual AbstractURLDlg * CreateURLDialog( Window* pParent,
                                            const String& rURL, const String& rAltText, const String& rDescription,
                                            const String& rTarget, const String& rName,
                                            TargetList& rTargetList ) = 0;
    virtual AbstractSvxHlinkDlgMarkWnd* CreateSvxHlinkDlgMarkWndDialog( SvxHyperlinkTabPageBase* pParent, sal_uInt32 nResId ) =0;

    virtual SfxAbstractTabDialog* CreateTabItemDialog( Window* pParent,
                                            const SfxItemSet& rSet,
                                            sal_uInt32 nResId) = 0;
    virtual VclAbstractDialog*      CreateSvxSearchAttributeDialog( Window* pParent,
                                            SearchAttrItemList& rLst,
                                            const sal_uInt16* pWhRanges)=0;
    virtual AbstractSvxSearchSimilarityDialog * CreateSvxSearchSimilarityDialog( Window* pParent,
                                                            sal_Bool bRelax,
                                                            sal_uInt16 nOther,
                                                            sal_uInt16 nShorter,
                                                            sal_uInt16 nLonger ) = 0;
    virtual SfxAbstractTabDialog* CreateSvxBorderBackgroundDlg( Window* pParent,
                                            const SfxItemSet& rCoreSet,
                                            sal_Bool bEnableSelector = sal_False) = 0;
    virtual AbstractSvxTransformTabDialog* CreateSvxTransformTabDialog( Window* pParent,
                                                                const SfxItemSet* pAttr,
                                                                const SdrView* pView,
                                                                sal_uInt16 nAnchorTypes = 0) = 0;
    virtual SfxAbstractTabDialog* CreateSchTransformTabDialog( Window* pParent,
                                                                const SfxItemSet* pAttr,
                                                                const SdrView* pSdrView,
                                                                sal_uInt32 nResId,
                                                                bool bSizeTabPage = false
                                                                 )=0;
    virtual AbstractSvxJSearchOptionsDialog * CreateSvxJSearchOptionsDialog( Window* pParent,
                                                            const SfxItemSet& rOptionsSet,
                                                            sal_Int32 nInitialFlags )=0;
    virtual AbstractFmInputRecordNoDialog * CreateFmInputRecordNoDialog( Window* pParent ) = 0;
    virtual AbstractSvxNewDictionaryDialog* CreateSvxNewDictionaryDialog( Window* pParent,
                                            ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XSpellChecker1 >  &xSpl ) = 0;
    virtual VclAbstractDialog *     CreateSvxEditDictionaryDialog( Window* pParent,
                                            const String& rName,
                                            ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XSpellChecker1> &xSpl,
                                            sal_uInt32 nResId) = 0;
    virtual AbstractSvxNameDialog *     CreateSvxNameDialog( Window* pParent,
                                            const String& rName, const String& rDesc ) = 0;

    // #i68101#
    virtual AbstractSvxObjectNameDialog* CreateSvxObjectNameDialog(Window* pParent, const String& rName ) = 0;
    virtual AbstractSvxObjectTitleDescDialog* CreateSvxObjectTitleDescDialog(Window* pParent, const String& rTitle, const String& rDescription) = 0;

    virtual AbstractSvxMessDialog *     CreateSvxMessDialog( Window* pParent, sal_uInt32 nResId,
                                            const String& rText, const String& rDesc,
                                            Image* pImg = NULL ) = 0;

    virtual AbstractSvxMultiPathDialog *    CreateSvxMultiPathDialog( Window* pParent, sal_Bool bEmptyAllowed = sal_False ) = 0 ;
    virtual AbstractSvxMultiFileDialog *    CreateSvxMultiFileDialog( Window* pParent, sal_Bool bEmptyAllowed = sal_False ) = 0 ;
    virtual AbstractSvxHpLinkDlg *  CreateSvxHpLinkDlg (Window* pParent,
                                            SfxBindings* pBindings,
                                            sal_uInt32 nResId)=0;
    virtual AbstractFmSearchDialog* CreateFmSearchDialog(Window* pParent,
                                                        const OUString& strInitialText,
                                                        const ::std::vector< OUString >& _rContexts,
                                                        sal_Int16 nInitialContext,
                                                        const Link& lnkContextSupplier)=0;
    virtual AbstractGraphicFilterDialog *   CreateGraphicFilterEmboss (Window* pParent,
                                                const Graphic& rGraphic,
                                                RECT_POINT eLightSource, sal_uInt32 nResId )=0;
    virtual AbstractGraphicFilterDialog *   CreateGraphicFilterPosterSepia (Window* pParent,
                                                const Graphic& rGraphic,
                                                sal_uInt16 nCount,
                                                sal_uInt32 nResId)=0;
    virtual AbstractGraphicFilterDialog *   CreateGraphicFilterSmooth (Window* pParent,
                                                const Graphic& rGraphic,
                                                double nRadius, sal_uInt32 nResId)=0;
    virtual AbstractGraphicFilterDialog *   CreateGraphicFilterSolarize (Window* pParent,
                                                const Graphic& rGraphic,
                                                sal_uInt8 nGreyThreshold, sal_Bool bInvert, sal_uInt32 nResId)=0;
    virtual AbstractGraphicFilterDialog *   CreateGraphicFilterMosaic (Window* pParent,
                                                const Graphic& rGraphic,
                                                sal_uInt16 nTileWidth, sal_uInt16 nTileHeight, sal_Bool bEnhanceEdges, sal_uInt32 nResId)=0;
    virtual AbstractSvxAreaTabDialog*       CreateSvxAreaTabDialog( Window* pParent,
                                                            const SfxItemSet* pAttr,
                                                            SdrModel* pModel,
                                                            const SdrView* pSdrView = NULL ) = 0 ;
    virtual SfxAbstractTabDialog*           CreateSvxLineTabDialog( Window* pParent, const SfxItemSet* pAttr, //add forSvxLineTabDialog
                                                                 SdrModel* pModel,
                                                                 const SdrObject* pObj = NULL,
                                                                sal_Bool bHasObj = sal_True )=0;
    virtual VclAbstractDialog*              CreateSfxDialog( Window* pParent, const SfxBindings& rBindings, sal_uInt32 nResId ) = 0;
    virtual SfxAbstractDialog*              CreateSfxDialog( Window* pParent,
                                                                        const SfxItemSet& rAttr,
                                                                        const SdrView* pView,
                                                                        sal_uInt32 nResId
                                                                        )=0;
    virtual SfxAbstractDialog*              CreateSfxDialog( Window* pParent,
                                                                        const SfxItemSet& rAttr,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _rxFrame,
                                                                        sal_uInt32 nResId
                                                                        )=0;
    virtual AbstractSvxPostItDialog*        CreateSvxPostItDialog( Window* pParent,
                                                                        const SfxItemSet& rCoreSet,
                                                                        sal_Bool bPrevNext = sal_False) = 0;
    virtual VclAbstractDialog*          CreateSvxScriptOrgDialog( Window* pParent, const OUString& rLanguage ) = 0;

    virtual CreateSvxDistributePage     GetSvxDistributePageCreatorFunc() = 0;
    virtual DialogGetRanges             GetDialogGetRangesFunc() = 0;

    virtual AbstractScriptSelectorDialog*
        CreateScriptSelectorDialog(
            Window* pParent,
            sal_Bool bShowSlots,
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _rxFrame
        ) = 0;

    virtual VclAbstractDialog* CreateScriptErrorDialog(
            Window* pParent, com::sun::star::uno::Any aException) = 0;

    virtual VclAbstractDialog*  CreateSvxMacroAssignDlg(
                Window* _pParent,
                const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _rxDocumentFrame,
                const bool _bUnoDialogMode,
                const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameReplace >& _rxEvents,
                const sal_uInt16 _nInitiallySelectedEvent
            ) = 0;

    virtual SfxAbstractTabDialog* CreateSvxFormatCellsDialog( Window* pParent, const SfxItemSet* pAttr, SdrModel* pModel, const SdrObject* pObj = NULL )=0;

    virtual SvxAbstractSplittTableDialog* CreateSvxSplittTableDialog( Window* pParent, bool bIsTableVertical, long nMaxVertical, long nMaxHorizontal )=0;

    virtual SvxAbstractNewTableDialog* CreateSvxNewTableDialog( Window* pParent ) = 0;

    virtual SvxAbstractInsRowColDlg* CreateSvxInsRowColDlg( Window* pParent, bool bCol, const OString& sHelpId ) = 0;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
