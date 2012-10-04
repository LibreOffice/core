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
#ifndef _CUI_DLGFACT_HXX
#define _CUI_DLGFACT_HXX

#include <svx/svxdlg.hxx>
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
class SvxMultiFileDialog;
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
    DialogClass*        pDlg;                       \
public:                                             \
                    Class( DialogClass* p)          \
                     : pDlg(p)                      \
                     {}                             \
    virtual         ~Class();                       \
    virtual short   Execute() ;

#define IMPL_ABSTDLG_BASE(Class)                    \
Class::~Class()                                     \
{                                                   \
    delete pDlg;                                    \
}                                                   \
short Class::Execute()                             \
{                                                   \
    return pDlg->Execute();                         \
}

//for GalleryThemeProperties begin
class VclAbstractDialog2_Impl : public VclAbstractDialog2
{
    Dialog*         m_pDlg;
    Link            m_aEndDlgHdl;
public:
                    VclAbstractDialog2_Impl( Dialog* p ) : m_pDlg( p ) {}                             \
    virtual         ~VclAbstractDialog2_Impl();
    virtual void    StartExecuteModal( const Link& rEndDialogHdl );
    virtual long    GetResult();
private:
                    DECL_LINK( EndDialogHdl, Dialog* );
};
//for GalleryThemeProperties end

class VclAbstractDialog_Impl : public VclAbstractDialog
{
    DECL_ABSTDLG_BASE(VclAbstractDialog_Impl,Dialog)
};

//for ActualizeProgress begin
class VclAbstractRefreshableDialog_Impl : public VclAbstractRefreshableDialog
{
    DECL_ABSTDLG_BASE(VclAbstractRefreshableDialog_Impl,Dialog)
    virtual void        Update() ;
    virtual void        Sync() ;
};
//for ActualizeProgress end

class AbstractSfxDialog_Impl : public SfxAbstractDialog
{
    DECL_ABSTDLG_BASE(AbstractSfxDialog_Impl,SfxModalDialog)
    virtual const SfxItemSet*   GetOutputItemSet() const;

        //From class Window.
    virtual void        SetText( const XubString& rStr );
    virtual String      GetText() const ;
};

class AbstractTabDialog_Impl : public SfxAbstractTabDialog
{
    DECL_ABSTDLG_BASE(AbstractTabDialog_Impl,SfxTabDialog)
    virtual void                SetCurPageId( sal_uInt16 nId );
    virtual const SfxItemSet*   GetOutputItemSet() const;
    virtual const sal_uInt16*       GetInputRanges( const SfxItemPool& pItem );
    virtual void                SetInputSet( const SfxItemSet* pInSet );
        //From class Window.
    virtual void        SetText( const XubString& rStr );
    virtual String      GetText() const;
};

//for SvxDistributeDialog begin
class SvxDistributeDialog;
class AbstractSvxDistributeDialog_Impl: public AbstractSvxDistributeDialog
{
    DECL_ABSTDLG_BASE(AbstractSvxDistributeDialog_Impl,SvxDistributeDialog)
public:
    virtual SvxDistributeHorizontal GetDistributeHor() const;
    virtual SvxDistributeVertical GetDistributeVer() const;
};
//for SvxDistributeDialog end

// for HangulHanjaConversionDialog begin
class AbstractHangulHanjaConversionDialog_Impl: public AbstractHangulHanjaConversionDialog
{
    DECL_ABSTDLG_BASE(AbstractHangulHanjaConversionDialog_Impl,HangulHanjaConversionDialog)
    virtual void    EndDialog(long nResult = 0);
    virtual void    EnableRubySupport( sal_Bool _bVal );
    virtual void    SetByCharacter( sal_Bool _bByCharacter ) ;
    virtual void    SetConversionDirectionState( sal_Bool _bTryBothDirections, editeng::HangulHanjaConversion::ConversionDirection _ePrimaryConversionDirection );
    virtual void    SetConversionFormat( editeng::HangulHanjaConversion::ConversionFormat _eType );
    virtual void    SetOptionsChangedHdl( const Link& _rHdl );
    virtual void    SetIgnoreHdl( const Link& _rHdl );
    virtual void    SetIgnoreAllHdl( const Link& _rHdl ) ;
    virtual void    SetChangeHdl( const Link& _rHdl ) ;
    virtual void    SetChangeAllHdl( const Link& _rHdl ) ;
    virtual void    SetClickByCharacterHdl( const Link& _rHdl ) ;
    virtual void    SetConversionFormatChangedHdl( const Link& _rHdl ) ;
    virtual void    SetFindHdl( const Link& _rHdl );
    virtual sal_Bool        GetUseBothDirections( ) const;
    virtual editeng::HangulHanjaConversion::ConversionDirection    GetDirection( editeng::HangulHanjaConversion::ConversionDirection _eDefaultDirection ) const;
    virtual void    SetCurrentString(
                    const String& _rNewString,
                    const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rSuggestions,
                    bool _bOriginatesFromDocument = true
                );
    virtual String  GetCurrentString( ) const ;
    virtual editeng::HangulHanjaConversion::ConversionFormat    GetConversionFormat( ) const ;
    virtual void    FocusSuggestion( );
    virtual String  GetCurrentSuggestion( ) const;
};

class AbstractThesaurusDialog_Impl : public AbstractThesaurusDialog
{
    DECL_ABSTDLG_BASE(AbstractThesaurusDialog_Impl,SvxThesaurusDialog)
    virtual String      GetWord();
    virtual sal_uInt16  GetLanguage() const;
    virtual Window*     GetWindow();
};


class AbstractHyphenWordDialog_Impl: public AbstractHyphenWordDialog
{
    DECL_ABSTDLG_BASE(AbstractHyphenWordDialog_Impl,SvxHyphenWordDialog)
    virtual void    SelLeft();
    virtual void    SelRight();
    virtual Window* GetWindow();
};

// for FmShowColsDialog begin
class FmShowColsDialog;
class AbstractFmShowColsDialog_Impl : public AbstractFmShowColsDialog
{
    DECL_ABSTDLG_BASE(AbstractFmShowColsDialog_Impl,FmShowColsDialog)
     virtual void   SetColumns(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer>& xCols);
};
//for FmShowColsDialog end


//for SvxZoomDialog begin
class SvxZoomDialog;
class AbstractSvxZoomDialog_Impl : public AbstractSvxZoomDialog
{
    DECL_ABSTDLG_BASE(AbstractSvxZoomDialog_Impl,SvxZoomDialog)
    virtual void    SetLimits( sal_uInt16 nMin, sal_uInt16 nMax );
    virtual void    HideButton( sal_uInt16 nBtnId );
    virtual const SfxItemSet*   GetOutputItemSet() const ;

};
//for SvxZoomDialog end

//for SvxSpellDialog begin
namespace svx{ class SpellDialog;}
class AbstractSpellDialog_Impl : public AbstractSpellDialog
{
 public:
    DECL_ABSTDLG_BASE(AbstractSpellDialog_Impl, svx::SpellDialog)
    virtual void        SetLanguage( sal_uInt16 nLang );
    virtual sal_Bool    Close();
    virtual void        Invalidate();
    virtual Window*     GetWindow();
    virtual SfxBindings& GetBindings();
};
//for SvxSpellDialog end
//for SearchProgress begin
class SearchProgress;
class AbstractSearchProgress_Impl : public AbstractSearchProgress
{
    DECL_ABSTDLG_BASE(AbstractSearchProgress_Impl,SearchProgress)
    virtual void        Update() ;
    virtual void        Sync() ;
    virtual void    SetFileType( const String& rType ) ;
    virtual void    SetDirectory( const INetURLObject& rURL ) ;
    virtual PLinkStub   GetLinkStubCleanUpHdl() ;

};
//for SearchProgress end

//for TakeProgress begin
class TakeProgress;
class AbstractTakeProgress_Impl : public AbstractTakeProgress
{
    DECL_ABSTDLG_BASE(AbstractTakeProgress_Impl,TakeProgress)
    virtual void        Update() ;
    virtual void        Sync() ;
    virtual void        SetFile( const INetURLObject& rURL ) ;
    virtual PLinkStub   GetLinkStubCleanUpHdl() ;

};
//for TakeProgress end

//for TitleDialog begin
class TitleDialog;
class AbstractTitleDialog_Impl : public AbstractTitleDialog
{
    DECL_ABSTDLG_BASE(AbstractTitleDialog_Impl,TitleDialog)
    virtual String  GetTitle() const ;

};
//for TitleDialog end

//for SvxScriptSelectorDialog begin
class SvxScriptSelectorDialog;
class AbstractScriptSelectorDialog_Impl : public AbstractScriptSelectorDialog
{
    DECL_ABSTDLG_BASE(
        AbstractScriptSelectorDialog_Impl, SvxScriptSelectorDialog)

    virtual String GetScriptURL() const;

    virtual void SetRunLabel();
};

//for GalleryIdDialog begin
class GalleryIdDialog;
class AbstractGalleryIdDialog_Impl : public AbstractGalleryIdDialog
{
    DECL_ABSTDLG_BASE(AbstractGalleryIdDialog_Impl,GalleryIdDialog)
    virtual sal_uLong   GetId() const ;

};
//for GalleryIdDialog end

//for URLDlg start
class URLDlg;
class AbstractURLDlg_Impl :public AbstractURLDlg
{
    DECL_ABSTDLG_BASE(AbstractURLDlg_Impl,URLDlg)
    virtual String      GetURL() const;
    virtual String      GetAltText() const;
    virtual String      GetDesc() const;
    virtual String      GetTarget() const;
    virtual String      GetName() const;
};
//for URLDlg end

//for SvxHlinkDlgMarkWnd begin
class SvxHlinkDlgMarkWnd;
class AbstractSvxHlinkDlgMarkWnd_Impl : public AbstractSvxHlinkDlgMarkWnd
{
    DECL_ABSTDLG_BASE(AbstractSvxHlinkDlgMarkWnd_Impl,SvxHlinkDlgMarkWnd)
    virtual void                Hide( sal_uInt16 nFlags = 0 );
    virtual sal_Bool                IsVisible() const ;
    virtual void                Invalidate( sal_uInt16 nFlags = 0 );
    virtual void                SetSizePixel( const Size& rNewSize );
    virtual Size                GetSizePixel() const;
    virtual sal_Bool                MoveTo ( Point aNewPos )const;
    virtual sal_Bool                ConnectToDialog( sal_Bool bDoit = sal_True )const;
    virtual void                RefreshTree ( String aStrURL ) ;
    virtual void                SelectEntry ( String aStrMark );
    virtual sal_uInt16              SetError( sal_uInt16 nError) ;

};
//for SvxHlinkDlgMarkWnd end

//for SvxSearchSimilarityDialog begin
class SvxSearchSimilarityDialog;
class AbstractSvxSearchSimilarityDialog_Impl :public AbstractSvxSearchSimilarityDialog
{
    DECL_ABSTDLG_BASE(AbstractSvxSearchSimilarityDialog_Impl,SvxSearchSimilarityDialog)
    virtual sal_uInt16              GetOther();
    virtual sal_uInt16              GetShorter();
    virtual sal_uInt16              GetLonger();
    virtual sal_Bool                IsRelaxed();
};
//for SvxSearchSimilarityDialog end

//for SvxJSearchOptionsDialog end
class SvxJSearchOptionsDialog;
class AbstractSvxJSearchOptionsDialog_Impl :public AbstractSvxJSearchOptionsDialog
{
    DECL_ABSTDLG_BASE(AbstractSvxJSearchOptionsDialog_Impl,SvxJSearchOptionsDialog)
    virtual sal_Int32           GetTransliterationFlags() const;
};
//for SvxJSearchOptionsDialog end

class AbstractSvxTransformTabDialog_Impl : public AbstractSvxTransformTabDialog
{
    DECL_ABSTDLG_BASE(AbstractSvxTransformTabDialog_Impl,SvxTransformTabDialog)
    virtual void SetValidateFramePosLink( const Link& rLink );
    virtual void                SetCurPageId( sal_uInt16 nId );
    virtual const SfxItemSet*   GetOutputItemSet() const;
    virtual const sal_uInt16*       GetInputRanges( const SfxItemPool& pItem );
    virtual void                SetInputSet( const SfxItemSet* pInSet );
    virtual void        SetText( const XubString& rStr );
    virtual String      GetText() const;
};
class AbstractSvxCaptionDialog_Impl : public AbstractSvxCaptionDialog
{
    DECL_ABSTDLG_BASE(AbstractSvxCaptionDialog_Impl,SvxCaptionTabDialog)
    virtual void SetValidateFramePosLink( const Link& rLink );
    virtual void                SetCurPageId( sal_uInt16 nId );
    virtual const SfxItemSet*   GetOutputItemSet() const;
    virtual const sal_uInt16*       GetInputRanges( const SfxItemPool& pItem );
    virtual void                SetInputSet( const SfxItemSet* pInSet );
    virtual void        SetText( const XubString& rStr );
    virtual String      GetText() const;
};

//for FmInputRecordNoDialog end
class FmInputRecordNoDialog;
class AbstractFmInputRecordNoDialog_Impl :public AbstractFmInputRecordNoDialog
{
    DECL_ABSTDLG_BASE(AbstractFmInputRecordNoDialog_Impl,FmInputRecordNoDialog)
    virtual void SetValue(long nNew) ;
    virtual long GetValue() const ;
};
//for FmInputRecordNoDialog end

//for SvxNewDictionaryDialog end
class SvxNewDictionaryDialog;
class AbstractSvxNewDictionaryDialog_Impl :public AbstractSvxNewDictionaryDialog
{
    DECL_ABSTDLG_BASE(AbstractSvxNewDictionaryDialog_Impl,SvxNewDictionaryDialog)
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XDictionary >  GetNewDictionary();
};
//for SvxNewDictionaryDialog end

//for SvxNameDialog end
class SvxNameDialog;
class AbstractSvxNameDialog_Impl :public AbstractSvxNameDialog
{
    DECL_ABSTDLG_BASE(AbstractSvxNameDialog_Impl,SvxNameDialog)
    virtual void    GetName( String& rName ) ;
    virtual void    SetCheckNameHdl( const Link& rLink, bool bCheckImmediately = false ) ;
    virtual void    SetEditHelpId(const rtl::OString&) ;
    //from class Window
    virtual void    SetHelpId( const rtl::OString& ) ;
    virtual void    SetText( const XubString& rStr ) ;
private:
    Link aCheckNameHdl;
    DECL_LINK(CheckNameHdl, void *);
};
//for SvxNameDialog end

///////////////////////////////////////////////////////////////////////////////////////////////
// #i68101#

// predefines
class SvxObjectNameDialog;
class SvxObjectTitleDescDialog;

class AbstractSvxObjectNameDialog_Impl :public AbstractSvxObjectNameDialog
{
    DECL_ABSTDLG_BASE(AbstractSvxObjectNameDialog_Impl, SvxObjectNameDialog)
    virtual void GetName(String& rName) ;
    virtual void SetCheckNameHdl(const Link& rLink, bool bCheckImmediately = false);

private:
    Link aCheckNameHdl;
    DECL_LINK(CheckNameHdl, void *);
};

class AbstractSvxObjectTitleDescDialog_Impl :public AbstractSvxObjectTitleDescDialog
{
    DECL_ABSTDLG_BASE(AbstractSvxObjectTitleDescDialog_Impl, SvxObjectTitleDescDialog)
    virtual void GetTitle(String& rName);
    virtual void GetDescription(String& rName);
};

///////////////////////////////////////////////////////////////////////////////////////////////

//for SvxMessDialog end
class SvxMessDialog;
class AbstractSvxMessDialog_Impl :public AbstractSvxMessDialog
{
    DECL_ABSTDLG_BASE(AbstractSvxMessDialog_Impl,SvxMessDialog)
    virtual void    SetButtonText( sal_uInt16 nBtnId, const String& rNewTxt );
};
//for SvxMessDialog end

//for SvxMultiPathDialog end
class SvxMultiPathDialog;
class AbstractSvxMultiPathDialog_Impl :public AbstractSvxMultiPathDialog
{
    DECL_ABSTDLG_BASE(AbstractSvxMultiPathDialog_Impl,SvxMultiPathDialog)
    virtual String          GetPath() const;
    virtual void            SetPath( const String& rPath );
    virtual void            EnableRadioButtonMode();
    virtual void            SetTitle( const String& rNewTitle );
};
//for SvxMultiPathDialog end

//for SvxMultiFileDialog begin
class SvxMultiFileDialog;
class AbstractSvxMultiFileDialog_Impl :public AbstractSvxMultiFileDialog
{
    DECL_ABSTDLG_BASE(AbstractSvxMultiFileDialog_Impl,SvxMultiFileDialog)
    virtual String          GetFiles() const ;
    virtual void            SetFiles( const String& rPath ) ;
    //from SvxMultiPathDialog
    virtual String          GetPath() const;
    virtual void            SetPath( const String& rPath );
    virtual void            SetClassPathMode();
    virtual void            EnableRadioButtonMode();
    virtual void            SetTitle( const String& rNewTitle );
    //From Class Window
    virtual void    SetHelpId( const rtl::OString& ) ;

};
//for SvxMultiFileDialog end

//for SvxHpLinkDlg begin
class SvxHpLinkDlg;
class AbstractSvxHpLinkDlg_Impl :public AbstractSvxHpLinkDlg
{
    DECL_ABSTDLG_BASE(AbstractSvxHpLinkDlg_Impl,SvxHpLinkDlg)
    virtual Window*     GetWindow();
    virtual sal_Bool    QueryClose();
};
//for SvxHpLinkDlg end

//for FmSearchDialog begin
class FmSearchDialog;
class AbstractFmSearchDialog_Impl :public AbstractFmSearchDialog
{
    DECL_ABSTDLG_BASE(AbstractFmSearchDialog_Impl,FmSearchDialog)
    virtual void SetFoundHandler(const Link& lnk) ;
    virtual void SetCanceledNotFoundHdl(const Link& lnk);
    virtual void SetActiveField(const String& strField);
};
//for FmSearchDialog end

//for GraphicFilterDialog begin
class GraphicFilterDialog;
class AbstractGraphicFilterDialog_Impl :public AbstractGraphicFilterDialog
{
    DECL_ABSTDLG_BASE(AbstractGraphicFilterDialog_Impl,GraphicFilterDialog)
    virtual Graphic GetFilteredGraphic( const Graphic& rGraphic, double fScaleX, double fScaleY );
};
//for GraphicFilterDialog end

// add for SvxAreaTabDialog begin
class SvxAreaTabDialog;
class AbstractSvxAreaTabDialog_Impl :public AbstractSvxAreaTabDialog
{
    DECL_ABSTDLG_BASE(AbstractSvxAreaTabDialog_Impl,SvxAreaTabDialog)
    virtual void                SetCurPageId( sal_uInt16 nId );
    virtual const SfxItemSet*   GetOutputItemSet() const;
    virtual const sal_uInt16*       GetInputRanges( const SfxItemPool& pItem );
    virtual void                SetInputSet( const SfxItemSet* pInSet );
    // From class Window.
    virtual void        SetText( const XubString& rStr );
    virtual String      GetText() const;
};
// add for SvxAreaTabDialog end

class AbstractInsertObjectDialog_Impl : public SfxAbstractInsertObjectDialog
{
    DECL_ABSTDLG_BASE(AbstractInsertObjectDialog_Impl, InsertObjectDialog_Impl)
    virtual com::sun::star::uno::Reference < com::sun::star::embed::XEmbeddedObject > GetObject();
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > GetIconIfIconified( ::rtl::OUString* pGraphicMediaType );
    virtual sal_Bool IsCreateNew();
};

class AbstractPasteDialog_Impl : public SfxAbstractPasteDialog
{
public:
    DECL_ABSTDLG_BASE(AbstractPasteDialog_Impl, SvPasteObjectDialog )
    virtual void Insert( SotFormatStringId nFormat, const String & rFormatName );
    virtual void SetObjName( const SvGlobalName & rClass, const String & rObjName );
    virtual sal_uLong GetFormat( const TransferableDataHelper& aHelper,
                        const DataFlavorExVector* pFormats=0,
                        const TransferableObjectDescriptor* pDesc=0 );
};

class AbstractLinksDialog_Impl : public SfxAbstractLinksDialog
{
public:
    DECL_ABSTDLG_BASE(AbstractLinksDialog_Impl, SvBaseLinksDlg )
};


//add for SvxPostItDialog begin
class SvxPostItDialog;
class AbstractSvxPostItDialog_Impl :public AbstractSvxPostItDialog
{
    DECL_ABSTDLG_BASE( AbstractSvxPostItDialog_Impl, SvxPostItDialog )
    virtual void                SetText( const XubString& rStr );  //From class Window
    virtual const SfxItemSet*   GetOutputItemSet() const;
    virtual void                SetPrevHdl( const Link& rLink ) ;
    virtual void                SetNextHdl( const Link& rLink ) ;
    virtual void                EnableTravel(sal_Bool bNext, sal_Bool bPrev) ;
    virtual String              GetNote() ;
    virtual void                SetNote(const String& rTxt) ;
    virtual void                ShowLastAuthor(const String& rAuthor, const String& rDate) ;
    virtual void                DontChangeAuthor() ;
    virtual void                HideAuthor() ;
    virtual void                SetReadonlyPostIt(sal_Bool bDisable) ;
    virtual sal_Bool                IsOkEnabled() const  ;
    virtual Window *            GetWindow();
private:
    Link aNextHdl;
    Link aPrevHdl;
    DECL_LINK(NextHdl, void *);
    DECL_LINK(PrevHdl, void *);
};
//add for SvxPostItDialog end

//for PasswordToOpenModifyDialog begin
class PasswordToOpenModifyDialog;
class AbstractPasswordToOpenModifyDialog_Impl : public AbstractPasswordToOpenModifyDialog
{
    DECL_ABSTDLG_BASE( AbstractPasswordToOpenModifyDialog_Impl, PasswordToOpenModifyDialog )

    virtual String  GetPasswordToOpen() const;
    virtual String  GetPasswordToModify() const;
    virtual bool    IsRecommendToOpenReadonly() const;
};
//for PasswordToOpenModifyDialog end


//------------------------------------------------------------------------
//AbstractDialogFactory_Impl implementations
class AbstractDialogFactory_Impl : public SvxAbstractDialogFactory
{
public:
    virtual VclAbstractDialog*          CreateVclDialog( Window* pParent, sal_uInt32 nResId );
    virtual VclAbstractDialog*          CreateSfxDialog( Window* pParent, const SfxBindings& rBindings, sal_uInt32 nResId );
    virtual SfxAbstractDialog*          CreateSfxDialog( sal_uInt32 nResId,
                                            Window* pParent,
                                            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xViewFrame,
                                            const SfxItemSet* pAttrSet=0 );
    virtual SfxAbstractDialog*          CreateSfxDialog( Window* pParent,
                                            const SfxItemSet& rAttr,
                                            const SdrView* pView,
                                            sal_uInt32 nResId );
    virtual SfxAbstractDialog*              CreateSfxDialog( Window* pParent, //add for SvxMeasureDialog & SvxConnectionDialog
                                                                        const SfxItemSet& rAttr,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _rxFrame,
                                                                        sal_uInt32 nResId
                                                                        );
    virtual VclAbstractDialog*          CreateFrameDialog( Window* pParent, const com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& rxFrame, sal_uInt32 nResId, const String& rParameter );
    virtual SfxAbstractTabDialog*       CreateTabDialog( sal_uInt32 nResId,
                                            Window* pParent,
                                            const SfxItemSet* pAttrSet,
                                            SfxViewFrame* pViewFrame,
                                            bool bEditFmt=false,
                                            const String *pUserButtonText=0 );
    virtual SfxAbstractTabDialog*       CreateTabDialog( sal_uInt32 nResId,
                                            Window* pParent,
                                            const SfxItemSet* pAttrSet,
                                            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xViewFrame,
                                            bool bEditFmt=false,
                                            const String *pUserButtonText=0 );
    virtual SfxAbstractTabDialog*       CreateTextTabDialog( Window* pParent,
                                            const SfxItemSet* pAttrSet,
                                            SdrView* pView,
                                            SdrModel* pModel=0 ); //add for SvxTextTabDialog
    virtual SfxAbstractTabDialog*       CreateTabItemDialog( Window* pParent,
                                            const SfxItemSet& rSet,
                                            sal_uInt32 nResId);
    virtual AbstractSvxCaptionDialog*   CreateCaptionDialog( Window* pParent,
                                            const SdrView* pView,
                                            sal_uInt16 nAnchorTypes = 0 );
    virtual AbstractSvxDistributeDialog*    CreateSvxDistributeDialog(Window* pParent,
                                            const SfxItemSet& rAttr,
                                            SvxDistributeHorizontal eHor = SvxDistributeHorizontalNone,
                                            SvxDistributeVertical eVer = SvxDistributeVerticalNone);
    virtual SfxAbstractInsertObjectDialog* CreateInsertObjectDialog( Window* pParent, const rtl::OUString& rCommmand,
            const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& xStor,
            const SvObjectServerList* pList = 0 );
    virtual VclAbstractDialog*          CreateEditObjectDialog( Window* pParent, const rtl::OUString& rCommmand,
            const com::sun::star::uno::Reference < com::sun::star::embed::XEmbeddedObject >& xObj );
   virtual  SfxAbstractPasteDialog*         CreatePasteDialog( Window* pParent );
   virtual  SfxAbstractLinksDialog*         CreateLinksDialog( Window* pParent, sfx2::LinkManager* pMgr, sal_Bool bHTML, sfx2::SvBaseLink* p=0  );

   virtual AbstractHangulHanjaConversionDialog * CreateHangulHanjaConversionDialog( Window* _pParent,  //add for HangulHanjaConversionDialog
                                            editeng::HangulHanjaConversion::ConversionDirection _ePrimaryDirection );
    virtual AbstractThesaurusDialog*        CreateThesaurusDialog( Window*, ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XThesaurus >  xThesaurus,
                                                const String &rWord, sal_Int16 nLanguage );

    virtual AbstractHyphenWordDialog*       CreateHyphenWordDialog( Window*,
                                                const String &rWord, LanguageType nLang,
                                                ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XHyphenator >  &xHyphen,
                                                SvxSpellWrapper* pWrapper );

   virtual AbstractFmShowColsDialog * CreateFmShowColsDialog( Window* pParent );  //add for FmShowColsDialog
   virtual AbstractSvxZoomDialog * CreateSvxZoomDialog( Window* pParent,  //add for SvxZoomDialog
                                            const SfxItemSet& rCoreSet);
   virtual SfxAbstractTabDialog* CreateSvxBorderBackgroundDlg( Window* pParent, //add for SvxBorderBackgroundDlg
                                            const SfxItemSet& rCoreSet,
                                            sal_Bool bEnableSelector = sal_False) ;
   virtual AbstractSvxTransformTabDialog* CreateSvxTransformTabDialog( Window* pParent, //add for SvxTransformTabDialog
                                                                const SfxItemSet* pAttr,
                                                                const SdrView* pView,
                                                                sal_uInt16 nAnchorTypes = 0) ;
    virtual SfxAbstractTabDialog* CreateSchTransformTabDialog( Window* pParent,  //add for SchTransformTabDialog
                                                                const SfxItemSet* pAttr,
                                                                const SdrView* pSdrView,
                                                                sal_uInt32 nResId,
                                                                bool bSizeTabPage = false
                                                                 );
    virtual AbstractSpellDialog *  CreateSvxSpellDialog(
                            Window* pParent,
                            SfxBindings* pBindings,
                            svx::SpellDialogChildWindow* pSpellChildWindow );

    virtual VclAbstractRefreshableDialog * CreateActualizeProgressDialog( Window* pParent, GalleryTheme* pThm );
    virtual AbstractSearchProgress * CreateSearchProgressDialog( Window* pParent,
                                            const INetURLObject& rStartURL);
    virtual AbstractTakeProgress * CreateTakeProgressDialog( Window* pParent );
    virtual AbstractTitleDialog * CreateTitleDialog( Window* pParent,  //add for TitleDialog
                                             const String& rOldText);
    virtual AbstractGalleryIdDialog * CreateGalleryIdDialog( Window* pParent,  //add for SvxZoomDialog
                                            GalleryTheme* pThm);
    virtual VclAbstractDialog2 * CreateGalleryThemePropertiesDialog( Window* pParent,  //add for GalleryThemeProperties
                                            ExchangeData* pData,
                                            SfxItemSet* pItemSet);
    virtual AbstractURLDlg * CreateURLDialog( Window* pParent,  //add for URLDlg
                                            const String& rURL, const String& rAltText, const String& rDescription,
                                            const String& rTarget, const String& rName,
                                            TargetList& rTargetList );
    virtual AbstractSvxHlinkDlgMarkWnd* CreateSvxHlinkDlgMarkWndDialog( SvxHyperlinkTabPageBase* pParent, sal_uInt32 nResId ); //add for SvxHlinkDlgMarkWnd

    virtual VclAbstractDialog*      CreateSvxSearchAttributeDialog( Window* pParent,
                                            SearchAttrItemList& rLst,
                                            const sal_uInt16* pWhRanges);
    virtual AbstractSvxSearchSimilarityDialog * CreateSvxSearchSimilarityDialog( Window* pParent,
                                                            sal_Bool bRelax,
                                                            sal_uInt16 nOther,
                                                            sal_uInt16 nShorter,
                                                            sal_uInt16 nLonger); //add for SvxSearchSimilarityDialog
    virtual AbstractSvxJSearchOptionsDialog * CreateSvxJSearchOptionsDialog( Window* pParent,
                                                            const SfxItemSet& rOptionsSet,
                                                            sal_Int32 nInitialFlags);
    virtual AbstractFmInputRecordNoDialog * CreateFmInputRecordNoDialog( Window* pParent );
    virtual AbstractSvxNewDictionaryDialog* CreateSvxNewDictionaryDialog( Window* pParent,
                                            ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XSpellChecker1 >  &xSpl,
                                            sal_uInt32 nResId ); //add for SvxNewDictionaryDialog
    virtual VclAbstractDialog *     CreateSvxEditDictionaryDialog( Window* pParent,
                                            const String& rName,
                                            ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XSpellChecker1> &xSpl,
                                            sal_uInt32 nResId);//add for SvxEditDictionaryDialog
    virtual AbstractSvxNameDialog *     CreateSvxNameDialog( Window* pParent,
                                            const String& rName, const String& rDesc );
    // #i68101#
    virtual AbstractSvxObjectNameDialog* CreateSvxObjectNameDialog(Window* pParent, const String& rName );
    virtual AbstractSvxObjectTitleDescDialog* CreateSvxObjectTitleDescDialog(Window* pParent, const String& rTitle, const String& rDescription);

    virtual AbstractSvxMessDialog *         CreateSvxMessDialog( Window* pParent, sal_uInt32 nResId,
                                            const String& rText, const String& rDesc,
                                            Image* pImg = NULL ); //add for SvxMessDialog
    virtual AbstractSvxMultiPathDialog *    CreateSvxMultiPathDialog( Window* pParent, sal_Bool bEmptyAllowed = sal_False ); //add for SvxMultiPathDialog
    virtual AbstractSvxMultiFileDialog *    CreateSvxMultiFileDialog( Window* pParent, sal_Bool bEmptyAllowed = sal_False ); //add for SvxMultiFileDialog
    virtual AbstractSvxHpLinkDlg *          CreateSvxHpLinkDlg (Window* pParent,  //add for SvxHpLink
                                            SfxBindings* pBindings,
                                            sal_uInt32 nResId);
    virtual AbstractFmSearchDialog*         CreateFmSearchDialog(Window* pParent, //add for FmSearchDialog
                                                        const String& strInitialText,
                                                        const ::std::vector< String >& _rContexts,
                                                        sal_Int16 nInitialContext,
                                                        const Link& lnkContextSupplier);
    virtual AbstractGraphicFilterDialog *   CreateGraphicFilterEmboss (Window* pParent,  //add for GraphicFilterEmboss
                                                const Graphic& rGraphic, RECT_POINT eLightSource,
                                                sal_uInt32 nResId);
    virtual AbstractGraphicFilterDialog *   CreateGraphicFilterPosterSepia (Window* pParent,  //add for GraphicFilterPoster & GraphicFilterSepia
                                                const Graphic& rGraphic, sal_uInt16 nCount,
                                                sal_uInt32 nResId);
    virtual AbstractGraphicFilterDialog *   CreateGraphicFilterSmooth (Window* pParent,  //add for GraphicFilterSolarize
                                                const Graphic& rGraphic, double nRadius,
                                                sal_uInt32 nResId);
    virtual AbstractGraphicFilterDialog *   CreateGraphicFilterSolarize (Window* pParent,  //add for GraphicFilterSmooth
                                                const Graphic& rGraphic, sal_uInt8 nGreyThreshold,
                                                sal_Bool bInvert, sal_uInt32 nResId);
    virtual AbstractGraphicFilterDialog *   CreateGraphicFilterMosaic (Window* pParent,  //add for GraphicFilterMosaic
                                                const Graphic& rGraphic, sal_uInt16 nTileWidth, sal_uInt16 nTileHeight,
                                                sal_Bool bEnhanceEdges, sal_uInt32 nResId);
    virtual AbstractSvxAreaTabDialog*       CreateSvxAreaTabDialog( Window* pParent,//add for SvxAreaTabDialog
                                                            const SfxItemSet* pAttr,
                                                            SdrModel* pModel,
                                                            const SdrView* pSdrView = NULL ); //add for SvxAreaTabDialog
    virtual SfxAbstractTabDialog*           CreateSvxLineTabDialog( Window* pParent, const SfxItemSet* pAttr, //add for SvxLineTabDialog
                                                                 SdrModel* pModel,
                                                                 const SdrObject* pObj = NULL,
                                                                sal_Bool bHasObj = sal_True );
    virtual AbstractSvxPostItDialog*        CreateSvxPostItDialog( Window* pParent, //add for SvxPostItDialog
                                                                        const SfxItemSet& rCoreSet,
                                                                        sal_Bool bPrevNext = sal_False, sal_Bool bRedline = sal_False );

    // For TabPage
    virtual CreateTabPage               GetTabPageCreatorFunc( sal_uInt16 nId );
    virtual CreateSvxDistributePage     GetSvxDistributePageCreatorFunc();

    virtual GetTabPageRanges            GetTabPageRangesFunc( sal_uInt16 nId );
    virtual DialogGetRanges         GetDialogGetRangesFunc( sal_uInt16 nId ); //add for SvxPostItDialog
    virtual VclAbstractDialog*          CreateSvxScriptOrgDialog( Window* pParent, const String& rLanguage );

    virtual AbstractScriptSelectorDialog*
        CreateScriptSelectorDialog(
            Window* pParent,
            sal_Bool bShowSlots,
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _rxFrame
        );

    virtual VclAbstractDialog* CreateScriptErrorDialog(
            Window* pParent, ::com::sun::star::uno::Any aException);

    virtual VclAbstractDialog*  CreateSvxMacroAssignDlg(
                Window* _pParent,
                const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _rxDocumentFrame,
                const bool _bUnoDialogMode,
                const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameReplace >& _rxEvents,
                const sal_uInt16 _nInitiallySelectedEvent
            );

    virtual SfxAbstractTabDialog* CreateSvxFormatCellsDialog( Window* pParent, const SfxItemSet* pAttr, SdrModel* pModel, const SdrObject* pObj = NULL );

    virtual SvxAbstractSplittTableDialog* CreateSvxSplittTableDialog( Window* pParent, bool bIsTableVertical, long nMaxVertical, long nMaxHorizontal );

    virtual SvxAbstractNewTableDialog* CreateSvxNewTableDialog( Window* pParent ) ;

    virtual VclAbstractDialog*          CreateOptionsDialog(
        Window* pParent, const rtl::OUString& rExtensionId, const rtl::OUString& rApplicationContext );

    virtual SvxAbstractInsRowColDlg* CreateSvxInsRowColDlg( Window* pParent, bool bCol, const rtl::OString& sHelpId );

    virtual AbstractPasswordToOpenModifyDialog *    CreatePasswordToOpenModifyDialog( Window * pParent, sal_uInt16 nMinPasswdLen, sal_uInt16 nMaxPasswdLen, bool bIsPasswordToModify );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
