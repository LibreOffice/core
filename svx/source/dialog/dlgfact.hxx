/*************************************************************************
 *
 *  $RCSfile: dlgfact.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: hr $ $Date: 2004-07-23 14:15:39 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _CUI_DLGFACT_HXX
#define _CUI_DLGFACT_HXX
// include ---------------------------------------------------------------
#include "svxdlg.hxx" //CHINA001 #include <svx/svxdlg.hxx>

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEREPLACE_HPP_
#include <com/sun/star/container/XNameReplace.hpp>
#endif

//#include <sfx2/basedlgs.hxx>
//#include "dstribut.hxx"
//#include "cuigaldlg.hxx"
//#include "cuiimapwnd.hxx"
//#include "hlmarkwn.hxx"
//#include "cuicharmap.hxx"
//#include "srchxtra.hxx"

class SfxTabDialog;
class Dialog;
class SfxItemPool;
class FmShowColsDialog;
class SvxZoomDialog;
class FmInputRecordNoDialog;
class SvxJSearchOptionsDialog;
class FmFormShell;
class SvxNewDictionaryDialog;
class SvxNameDialog;
class SvxMessDialog;
class SvxMultiPathDialog;
class SvxMultiFileDialog;
class SvxHpLinkDlg;
class FmSearchDialog;
class Graphic;
class GraphicFilterDialog;
class SvxAreaTabDialog;
class SvxTransformTabDialog;
class SvxCaptionTabDialog;

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
    virtual USHORT  Execute() ;
//  virtual void    Show( BOOL bVisible = TRUE, USHORT nFlags = 0 )

#define IMPL_ABSTDLG_BASE(Class)                    \
Class::~Class()                                     \
{                                                   \
    delete pDlg;                                    \
}                                                   \
USHORT Class::Execute()                             \
{                                                   \
    return pDlg->Execute();                         \
}
//void Class::Show( BOOL bVisible, USHORT nFlags )    \
//{                                                   \
//  pDlg->Show( bVisible, nFlags );                 \
//}

//for GalleryThemeProperties begin
class VclAbstractDialog_Impl : public VclAbstractDialog
{
    DECL_ABSTDLG_BASE(VclAbstractDialog_Impl,Dialog);
};
//for GalleryThemeProperties end

//for ActualizeProgress begin
class VclAbstractRefreshableDialog_Impl : public VclAbstractRefreshableDialog
{
    DECL_ABSTDLG_BASE(VclAbstractRefreshableDialog_Impl,Dialog);
    virtual void        Update() ;
    virtual void        Sync() ;
};
//for ActualizeProgress end

class AbstractTabDialog_Impl : public SfxAbstractTabDialog
{
    DECL_ABSTDLG_BASE(AbstractTabDialog_Impl,SfxTabDialog);
    virtual void                SetCurPageId( USHORT nId );
    virtual const SfxItemSet*   GetOutputItemSet() const;
    virtual const USHORT*       GetInputRanges( const SfxItemPool& pItem ); //add by CHINA001
    virtual void                SetInputSet( const SfxItemSet* pInSet );   //add by CHINA001
        //From class Window.
    virtual void        SetText( const XubString& rStr ); //add by CHINA001
    virtual String      GetText() const; //add by CHINA001
};

//for SvxDistributeDialog begin
class SvxDistributeDialog;
class AbstractSvxDistributeDialog_Impl: public AbstractSvxDistributeDialog
{
    DECL_ABSTDLG_BASE(AbstractSvxDistributeDialog_Impl,SvxDistributeDialog);
public:
    virtual SvxDistributeHorizontal GetDistributeHor() const;
    virtual SvxDistributeVertical GetDistributeVer() const;
};
//for SvxDistributeDialog end

// for HangulHanjaConversionDialog begin
class AbstractHangulHanjaConversionDialog_Impl: public AbstractHangulHanjaConversionDialog
{
    DECL_ABSTDLG_BASE(AbstractHangulHanjaConversionDialog_Impl,HangulHanjaConversionDialog);
    virtual void    EndDialog(long nResult = 0);
    virtual void    EnableRubySupport( sal_Bool _bVal );
    virtual void    SetByCharacter( sal_Bool _bByCharacter ) ;
    virtual void    SetConversionFormat( HangulHanjaConversion::ConversionFormat _eType );
    virtual void    SetIgnoreHdl( const Link& _rHdl );
    virtual void    SetIgnoreAllHdl( const Link& _rHdl ) ;
    virtual void    SetChangeHdl( const Link& _rHdl ) ;
    virtual void    SetChangeAllHdl( const Link& _rHdl ) ;
    virtual void    SetClickByCharacterHdl( const Link& _rHdl ) ;
    virtual void    SetConversionFormatChangedHdl( const Link& _rHdl ) ;
    virtual void    SetFindHdl( const Link& _rHdl );
    virtual sal_Bool        GetUseBothDirections( ) const;
    virtual void    SetCurrentString(
                    const String& _rNewString,
                    const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rSuggestions,
                    bool _bOriginatesFromDocument = true
                );
    virtual String  GetCurrentString( ) const ;
    virtual HangulHanjaConversion::ConversionFormat    GetConversionFormat( ) const ;
    virtual void    FocusSuggestion( );
    virtual String  GetCurrentSuggestion( ) const;
};

// for HangulHanjaConversionDialog end

// for FmShowColsDialog begin
class FmShowColsDialog;
class AbstractFmShowColsDialog_Impl : public AbstractFmShowColsDialog
{
    DECL_ABSTDLG_BASE(AbstractFmShowColsDialog_Impl,FmShowColsDialog);
     virtual void   SetColumns(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer>& xCols);
};
//for FmShowColsDialog end


//for SvxZoomDialog begin
class SvxZoomDialog;
class AbstractSvxZoomDialog_Impl : public AbstractSvxZoomDialog
{
    DECL_ABSTDLG_BASE(AbstractSvxZoomDialog_Impl,SvxZoomDialog);
    virtual void    SetLimits( USHORT nMin, USHORT nMax );
    virtual void    HideButton( USHORT nBtnId );
    virtual const SfxItemSet*   GetOutputItemSet() const ;

};
//for SvxZoomDialog end

//for SvxSpellCheckDialog begin
//STRIP001 class AbstractSvxSpellCheckDialog_Impl : public AbstractSvxSpellCheckDialog //add for FmShowColsDialog
//STRIP001 {
//STRIP001 SvxSpellCheckDialog * pDlg;
//STRIP001 public
//STRIP001 AbstractSvxSpellCheckDialog_Impl ( SvxSpellCheckDialog* p)
//STRIP001 : pDlg(p)
//STRIP001 {}
//STRIP001 virtual  USHORT          Execute() ;
//STRIP001 virtual  void SetNewEditWord( const String& _rNew ) ;
//STRIP001 virtual void SetLanguage( sal_uInt16 nLang ) ;
//STRIP001 virtual void HideAutoCorrect() ;
//STRIP001 virtual String   GetNewEditWord();
//STRIP001 virtual void SetNewEditWord( const String& _rNew );
//STRIP001 }
//for SvxSpellCheckDialog end


//for SearchProgress begin
class SearchProgress;
class AbstractSearchProgress_Impl : public AbstractSearchProgress
{
    DECL_ABSTDLG_BASE(AbstractSearchProgress_Impl,SearchProgress);
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
    DECL_ABSTDLG_BASE(AbstractTakeProgress_Impl,TakeProgress);
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
    DECL_ABSTDLG_BASE(AbstractTitleDialog_Impl,TitleDialog);
    virtual String  GetTitle() const ;

};
//for TitleDialog end

//for SvxScriptSelectorDialog begin
class SvxScriptSelectorDialog;
class AbstractScriptSelectorDialog_Impl : public AbstractScriptSelectorDialog
{
    DECL_ABSTDLG_BASE(
        AbstractScriptSelectorDialog_Impl, SvxScriptSelectorDialog);

    virtual String GetScriptURL();

    virtual void SetRunLabel();
};

//for GalleryIdDialog begin
class GalleryIdDialog;
class AbstractGalleryIdDialog_Impl : public AbstractGalleryIdDialog
{
    DECL_ABSTDLG_BASE(AbstractGalleryIdDialog_Impl,GalleryIdDialog);
    virtual ULONG   GetId() const ;

};
//for GalleryIdDialog end

//for URLDlg start
class URLDlg;
class AbstractURLDlg_Impl :public AbstractURLDlg
{
    DECL_ABSTDLG_BASE(AbstractURLDlg_Impl,URLDlg);
    virtual String      GetURL() const;
    virtual String      GetDescription() const;
    virtual String      GetTarget() const;
    virtual String      GetName() const;
};
//for URLDlg end

//for SvxHlinkDlgMarkWnd begin
class SvxHlinkDlgMarkWnd;
class AbstractSvxHlinkDlgMarkWnd_Impl : public AbstractSvxHlinkDlgMarkWnd
{
    DECL_ABSTDLG_BASE(AbstractSvxHlinkDlgMarkWnd_Impl,SvxHlinkDlgMarkWnd);
    virtual void                Hide( USHORT nFlags = 0 );
    virtual BOOL                IsVisible() const ;
    virtual void                Invalidate( USHORT nFlags = 0 );
    virtual void                SetSizePixel( const Size& rNewSize );
    virtual Size                GetSizePixel() const;
    virtual BOOL                MoveTo ( Point aNewPos )const;
    virtual BOOL                ConnectToDialog( BOOL bDoit = TRUE )const;
    virtual void                RefreshTree ( String aStrURL ) ;
    virtual void                SelectEntry ( String aStrMark );
    virtual USHORT              SetError( USHORT nError) ;

};
//for SvxHlinkDlgMarkWnd end

//for SvxCharacterMap begin
class SvxCharacterMap;
class AbstractSvxCharacterMap_Impl :public AbstractSvxCharacterMap
{
    DECL_ABSTDLG_BASE(AbstractSvxCharacterMap_Impl,SvxCharacterMap);
    virtual void      SetText( const XubString& rStr ); //From Class Window
    virtual void               DisableFontSelection();

    virtual const Font&     GetCharFont() const ;
    virtual void            SetCharFont( const Font& rFont );
    virtual void            SetFont( const Font& rFont ) ;

    virtual void            SetChar( sal_Unicode c );
    virtual sal_Unicode     GetChar() const;

    virtual String          GetCharacters() const;
};
//for SvxCharacterMap end

//for SvxSearchSimilarityDialog begin
class SvxSearchSimilarityDialog;
class AbstractSvxSearchSimilarityDialog_Impl :public AbstractSvxSearchSimilarityDialog
{
    DECL_ABSTDLG_BASE(AbstractSvxSearchSimilarityDialog_Impl,SvxSearchSimilarityDialog);
    virtual USHORT              GetOther();
    virtual USHORT              GetShorter();
    virtual USHORT              GetLonger();
    virtual BOOL                IsRelaxed();
};
//for SvxSearchSimilarityDialog end

//for SvxJSearchOptionsDialog end
class SvxJSearchOptionsDialog;
class AbstractSvxJSearchOptionsDialog_Impl :public AbstractSvxJSearchOptionsDialog
{
    DECL_ABSTDLG_BASE(AbstractSvxJSearchOptionsDialog_Impl,SvxJSearchOptionsDialog);
    virtual INT32           GetTransliterationFlags() const;
};
//for SvxJSearchOptionsDialog end

class AbstractSvxTransformTabDialog_Impl : public AbstractSvxTransformTabDialog
{
    DECL_ABSTDLG_BASE(AbstractSvxTransformTabDialog_Impl,SvxTransformTabDialog);
    virtual void SetValidateFramePosLink( const Link& rLink );
    virtual void                SetCurPageId( USHORT nId );
    virtual const SfxItemSet*   GetOutputItemSet() const;
    virtual const USHORT*       GetInputRanges( const SfxItemPool& pItem );
    virtual void                SetInputSet( const SfxItemSet* pInSet );
    virtual void        SetText( const XubString& rStr );
    virtual String      GetText() const;
};
class AbstractSvxCaptionDialog_Impl : public AbstractSvxCaptionDialog
{
    DECL_ABSTDLG_BASE(AbstractSvxCaptionDialog_Impl,SvxCaptionTabDialog);
    virtual void SetValidateFramePosLink( const Link& rLink );
    virtual void                SetCurPageId( USHORT nId );
    virtual const SfxItemSet*   GetOutputItemSet() const;
    virtual const USHORT*       GetInputRanges( const SfxItemPool& pItem );
    virtual void                SetInputSet( const SfxItemSet* pInSet );
    virtual void        SetText( const XubString& rStr );
    virtual String      GetText() const;
};

//for FmInputRecordNoDialog end
class FmInputRecordNoDialog;
class AbstractFmInputRecordNoDialog_Impl :public AbstractFmInputRecordNoDialog
{
    DECL_ABSTDLG_BASE(AbstractFmInputRecordNoDialog_Impl,FmInputRecordNoDialog);
    virtual void SetValue(double dNew) ;
    virtual long GetValue() const ;
};
//for FmInputRecordNoDialog end

//for SvxNewDictionaryDialog end
class SvxNewDictionaryDialog;
class AbstractSvxNewDictionaryDialog_Impl :public AbstractSvxNewDictionaryDialog
{
    DECL_ABSTDLG_BASE(AbstractSvxNewDictionaryDialog_Impl,SvxNewDictionaryDialog);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XDictionary1 > GetNewDictionary();
};
//for SvxNewDictionaryDialog end

//for SvxNameDialog end
class SvxNameDialog;
class AbstractSvxNameDialog_Impl :public AbstractSvxNameDialog
{
    DECL_ABSTDLG_BASE(AbstractSvxNameDialog_Impl,SvxNameDialog);
    virtual void    GetName( String& rName ) ;
    virtual void    SetCheckNameHdl( const Link& rLink, bool bCheckImmediately = false ) ;
    virtual void    SetEditHelpId(ULONG nHelpId) ;
    //from class Window
    virtual void    SetHelpId( ULONG nHelpId ) ;
    virtual void    SetText( const XubString& rStr ) ;
private:
    Link aCheckNameHdl;
    DECL_LINK( CheckNameHdl, Window*);
};
//for SvxNameDialog end

//for SvxMessDialog end
class SvxMessDialog;
class AbstractSvxMessDialog_Impl :public AbstractSvxMessDialog
{
    DECL_ABSTDLG_BASE(AbstractSvxMessDialog_Impl,SvxMessDialog);
    virtual void    SetButtonText( USHORT nBtnId, const String& rNewTxt );
};
//for SvxMessDialog end

//for SvxMultiPathDialog end
class SvxMultiPathDialog;
class AbstractSvxMultiPathDialog_Impl :public AbstractSvxMultiPathDialog
{
    DECL_ABSTDLG_BASE(AbstractSvxMultiPathDialog_Impl,SvxMultiPathDialog);
    virtual String          GetPath() const;
    virtual void            SetPath( const String& rPath );
};
//for SvxMultiPathDialog end

//for SvxMultiFileDialog begin
class SvxMultiFileDialog;
class AbstractSvxMultiFileDialog_Impl :public AbstractSvxMultiFileDialog
{
    DECL_ABSTDLG_BASE(AbstractSvxMultiFileDialog_Impl,SvxMultiFileDialog);
    virtual String          GetFiles() const ;
    virtual void            SetFiles( const String& rPath ) ;
    //from SvxMultiPathDialog
    virtual String          GetPath() const;
    virtual void            SetPath( const String& rPath );
    virtual void            SetClassPathMode();
    //From Class Window
    virtual void    SetHelpId( ULONG nHelpId ) ;

};
//for SvxMultiFileDialog end

//for SvxHpLinkDlg begin
class SvxHpLinkDlg;
class AbstractSvxHpLinkDlg_Impl :public AbstractSvxHpLinkDlg
{
    DECL_ABSTDLG_BASE(AbstractSvxHpLinkDlg_Impl,SvxHpLinkDlg);
    virtual Window * GetWindow() ;

};
//for SvxHpLinkDlg end

//for FmSearchDialog begin
class FmSearchDialog;
class AbstractFmSearchDialog_Impl :public AbstractFmSearchDialog
{
    DECL_ABSTDLG_BASE(AbstractFmSearchDialog_Impl,FmSearchDialog);
    virtual void SetFoundHandler(const Link& lnk) ;
    virtual void SetCanceledNotFoundHdl(const Link& lnk);
    virtual void SetActiveField(const String& strField);
};
//for FmSearchDialog end

//for GraphicFilterDialog begin
class GraphicFilterDialog;
class AbstractGraphicFilterDialog_Impl :public AbstractGraphicFilterDialog
{
    DECL_ABSTDLG_BASE(AbstractGraphicFilterDialog_Impl,GraphicFilterDialog);
    virtual Graphic GetFilteredGraphic( const Graphic& rGraphic, double fScaleX, double fScaleY );
};
//for GraphicFilterDialog end

//add for SvxAreaTabDialog begin
class SvxAreaTabDialog;
class AbstractSvxAreaTabDialog_Impl :public AbstractSvxAreaTabDialog
{
    DECL_ABSTDLG_BASE(AbstractSvxAreaTabDialog_Impl,SvxAreaTabDialog);
    virtual void                SetCurPageId( USHORT nId );
    virtual const SfxItemSet*   GetOutputItemSet() const;
    virtual const USHORT*       GetInputRanges( const SfxItemPool& pItem );
    virtual void                SetInputSet( const SfxItemSet* pInSet );
        //From class Window.
    virtual void        SetText( const XubString& rStr );
    virtual String      GetText() const;
    virtual void     DontDeleteColorTable() ;
};
//add for SvxAreaTabDialog end

//add for SvxMeasureDialog & SvxConnectionDialog begin
class SfxSingleTabDialog;
class AbstractSfxSingleTabDialog_Impl :public AbstractSfxSingleTabDialog
{
    DECL_ABSTDLG_BASE(AbstractSfxSingleTabDialog_Impl,SfxSingleTabDialog);
    virtual const SfxItemSet*   GetOutputItemSet() const;

};
//add for SvxMeasureDialog & SvxConnectionDialog end

//add for SvxPostItDialog begin
class SvxPostItDialog;
class AbstractSvxPostItDialog_Impl :public AbstractSvxPostItDialog
{
    DECL_ABSTDLG_BASE( AbstractSvxPostItDialog_Impl, SvxPostItDialog );
    virtual void                SetText( const XubString& rStr );  //From class Window
    virtual const SfxItemSet*   GetOutputItemSet() const;
    virtual void                SetPrevHdl( const Link& rLink ) ;
    virtual void                SetNextHdl( const Link& rLink ) ;
    virtual void                EnableTravel(BOOL bNext, BOOL bPrev) ;
    virtual String              GetNote() ;
    virtual void                SetNote(const String& rTxt) ;
    virtual void                ShowLastAuthor(const String& rAuthor, const String& rDate) ;
    virtual void                DontChangeAuthor() ;
    virtual void                HideAuthor() ;
    virtual void                SetReadonlyPostIt(BOOL bDisable) ;
    virtual BOOL                IsOkEnabled() const  ;
    virtual Window *            GetWindow();
private:
    Link aNextHdl;
    Link aPrevHdl;
    DECL_LINK( NextHdl, Window*);
    DECL_LINK( PrevHdl, Window*);
};
//add for SvxPostItDialog end

//------------------------------------------------------------------------
//AbstractDialogFactory_Impl implementations
class AbstractDialogFactory_Impl : public SvxAbstractDialogFactory
{

public:
    virtual VclAbstractDialog*          CreateVclDialog( Window* pParent, const ResId& rResId );
    virtual VclAbstractDialog*          CreateSfxDialog( Window* pParent, const SfxBindings& rBindings, const ResId& rResId );
    virtual SfxAbstractTabDialog*       CreateTabDialog( const ResId& rResId,
                                            Window* pParent,
                                            const SfxItemSet* pAttrSet,
                                            SfxViewFrame* pViewFrame,
                                            bool bEditFmt=false,
                                            const String *pUserButtonText=0 );
    virtual SfxAbstractTabDialog*       CreateTextTabDialog( Window* pParent,
                                            const SfxItemSet* pAttrSet,
                                            const ResId& rResId,
                                            SdrView* pView,
                                            SdrModel* pModel=0 ); //add for SvxTextTabDialog
    virtual SfxAbstractTabDialog*       CreateTabItemDialog( Window* pParent,
                                            const SfxItemSet& rSet,
                                            const ResId& rResId); //add by CHINA001
    virtual AbstractSvxCaptionDialog*   CreateCaptionDialog( Window* pParent,
                                            const SdrView* pView,
                                            const ResId& rResId,
                                            USHORT nAnchorTypes = 0 );      //add for SvxCaptionTabDialog CHINA001
    virtual AbstractSvxDistributeDialog*    CreateSvxDistributeDialog(Window* pParent,
                                            const SfxItemSet& rAttr,
                                            const ResId& rResId,
                                            SvxDistributeHorizontal eHor = SvxDistributeHorizontalNone,
                                            SvxDistributeVertical eVer = SvxDistributeVerticalNone);

   virtual AbstractHangulHanjaConversionDialog * CreateHangulHanjaConversionDialog( Window* _pParent,  //add for HangulHanjaConversionDialog CHINA001
                                            HangulHanjaConversion::ConversionDirection _ePrimaryDirection,
                                            const ResId& rResId);

   virtual AbstractFmShowColsDialog * CreateFmShowColsDialog( Window* pParent,  //add for FmShowColsDialog
                                             const ResId& rResId);
   virtual AbstractSvxZoomDialog * CreateSvxZoomDialog( Window* pParent,  //add for SvxZoomDialog
                                            const SfxItemSet& rCoreSet,
                                            const ResId& rResId);
   virtual SfxAbstractTabDialog* CreateSvxBorderBackgroundDlg( Window* pParent, //add for SvxBorderBackgroundDlg
                                            const SfxItemSet& rCoreSet,
                                            const ResId& rResId,
                                            BOOL bEnableSelector = FALSE) ;
   virtual AbstractSvxTransformTabDialog* CreateSvxTransformTabDialog( Window* pParent, //add for SvxTransformTabDialog
                                                                const SfxItemSet* pAttr,
                                                                const SdrView* pView,
                                                                const ResId& rResId,
                                                                USHORT nAnchorTypes = 0) ;
    virtual SfxAbstractTabDialog* CreateSchTransformTabDialog( Window* pParent,  //add for SchTransformTabDialog
                                                                const SfxItemSet* pAttr,
                                                                const SdrView* pSdrView,
                                                                const ResId& rResId,
                                                                bool bSizeTabPage = false
                                                                 );
//STRIP001    virtual AbstractSvxSpellCheckDialog * CreateSvxSpellCheckDialog( Window* pParent, //add for SvxSpellCheckDialog
//STRIP001    ::com::sun::star::uno::Reference<
//STRIP001    ::com::sun::star::linguistic2::XSpellChecker1 >  &xChecker,
//STRIP001    SvxSpellWrapper* pWrapper,
//STRIP001    const ResId& rResId);
    virtual VclAbstractRefreshableDialog * CreateActualizeProgressDialog( Window* pParent,  //add for ActualizeProgress
                                            GalleryTheme* pThm,
                                            const ResId& rResId);
    virtual AbstractSearchProgress * CreateSearchProgressDialog( Window* pParent,  //add for SearchProgress
                                            const INetURLObject& rStartURL,
                                            const ResId& rResId);
    virtual AbstractTakeProgress * CreateTakeProgressDialog( Window* pParent,  //add for TakeProgress
                                            const ResId& rResId);
    virtual AbstractTitleDialog * CreateTitleDialog( Window* pParent,  //add for TitleDialog
                                             const String& rOldText,
                                            const ResId& rResId);
    virtual AbstractGalleryIdDialog * CreateGalleryIdDialog( Window* pParent,  //add for SvxZoomDialog
                                            GalleryTheme* pThm,
                                            const ResId& rResId);
    virtual VclAbstractDialog * CreateGalleryThemePropertiesDialog( Window* pParent,  //add for GalleryThemeProperties
                                            ExchangeData* pData,
                                            SfxItemSet* pItemSet,
                                            const ResId& rResId);
    virtual AbstractURLDlg * CreateURLDialog( Window* pParent,  //add for URLDlg
                                            const String& rURL, const String& rDescription,
                                            const String& rTarget, const String& rName,
                                            TargetList& rTargetList,
                                            const ResId& rResId);
    virtual AbstractSvxHlinkDlgMarkWnd* CreateSvxHlinkDlgMarkWndDialog( SvxHyperlinkTabPageBase* pParent, const ResId& rResId ); //add for SvxHlinkDlgMarkWnd

    virtual AbstractSvxCharacterMap * CreateSvxCharacterMap( Window* pParent,
                                                            const ResId& rResId,
                                                            BOOL bOne = TRUE ) ; //add for SvxCharacterMap
    virtual VclAbstractDialog*      CreateSvxSearchAttributeDialog( Window* pParent,
                                            SearchAttrItemList& rLst,
                                            const USHORT* pWhRanges,
                                            const ResId& rResId ); //add for SvxSearchAttributeDialog
    virtual AbstractSvxSearchSimilarityDialog * CreateSvxSearchSimilarityDialog( Window* pParent,
                                                            BOOL bRelax,
                                                            USHORT nOther,
                                                            USHORT nShorter,
                                                            USHORT nLonger,
                                                            const ResId& rResId); //add for SvxSearchSimilarityDialog
    virtual AbstractSvxJSearchOptionsDialog * CreateSvxJSearchOptionsDialog( Window* pParent,
                                                            const SfxItemSet& rOptionsSet, USHORT nUniqueId,
                                                            INT32 nInitialFlags,
                                                            const ResId& rResId); //add for SvxJSearchOptionsDialog
    virtual AbstractFmInputRecordNoDialog * CreateFmInputRecordNoDialog( Window* pParent,
                                                            const ResId& rResId); //add for FmInputRecordNoDialog
    virtual AbstractSvxNewDictionaryDialog* CreateSvxNewDictionaryDialog( Window* pParent,
                                            ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XSpellChecker1 >  &xSpl,
                                            const ResId& rResId ); //add for SvxNewDictionaryDialog
    virtual VclAbstractDialog *     CreateSvxEditDictionaryDialog( Window* pParent,
                                            const String& rName,
                                            ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XSpellChecker1> &xSpl,
                                            const ResId& rResId);//add for SvxEditDictionaryDialog
    virtual AbstractSvxNameDialog *     CreateSvxNameDialog( Window* pParent,
                                            const String& rName, const String& rDesc,
                                            const ResId& rResId ); //add for SvxNameDialog
    virtual AbstractSvxMessDialog *     CreateSvxMessDialog( Window* pParent, const ResId& rResId,
                                            const String& rText, const String& rDesc,
                                            Image* pImg = NULL ); //add for SvxMessDialog
    virtual AbstractSvxMultiPathDialog *    CreateSvxMultiPathDialog( Window* pParent, const ResId& rResId,
                                                BOOL bEmptyAllowed = FALSE ); //add for SvxMultiPathDialog
    virtual AbstractSvxMultiFileDialog *    CreateSvxMultiFileDialog( Window* pParent, const ResId& rResId,
                                                BOOL bEmptyAllowed = FALSE ); //add for SvxMultiFileDialog
    virtual AbstractSvxHpLinkDlg *          CreateSvxHpLinkDlg (Window* pParent,  //add for SvxHpLink
                                            SfxBindings* pBindings,
                                            const ResId& rResId);
    virtual AbstractFmSearchDialog* CreateFmSearchDialog(Window* pParent, //add for FmSearchDialog
                                                        const String& strInitialText,
                                                        const String& strContexts,
                                                        sal_Int16 nInitialContext,
                                                        const Link& lnkContextSupplier,
                                                        const ResId& rResId,
                                                        FMSEARCH_MODE eMode = SM_ALLOWSCHEDULE);
    virtual AbstractGraphicFilterDialog *   CreateGraphicFilterEmboss (Window* pParent,  //add for GraphicFilterEmboss
                                            const Graphic& rGraphic,
                                            RECT_POINT eLightSource,
                                            const ResId& rResId);
    virtual AbstractGraphicFilterDialog *   CreateGraphicFilterPosterSepia (Window* pParent,  //add for GraphicFilterPoster & GraphicFilterSepia
                                            const Graphic& rGraphic,
                                            USHORT nCount,
                                            const ResId& rResId);
    virtual AbstractGraphicFilterDialog *   CreateGraphicFilterSolarize (Window* pParent,  //add for GraphicFilterSolarize
                                            const Graphic& rGraphic,
                                            BYTE nGreyThreshold, BOOL bInvert,
                                            const ResId& rResId);
    virtual AbstractGraphicFilterDialog *   CreateGraphicFilterMosaic (Window* pParent,  //add for GraphicFilterMosaic
                                            const Graphic& rGraphic,
                                            USHORT nTileWidth, USHORT nTileHeight, BOOL bEnhanceEdges,
                                            const ResId& rResId);
    virtual AbstractSvxAreaTabDialog*       CreateSvxAreaTabDialog( Window* pParent,//add for SvxAreaTabDialog
                                                            const SfxItemSet* pAttr,
                                                            SdrModel* pModel,
                                                            const ResId& rResId,
                                                            const SdrView* pSdrView = NULL ); //add for SvxAreaTabDialog
    virtual SfxAbstractTabDialog*           CreateSvxLineTabDialog( Window* pParent, const SfxItemSet* pAttr, //add for SvxLineTabDialog
                                                                 SdrModel* pModel,
                                                                 const ResId& rResId,
                                                                 const SdrObject* pObj = NULL,
                                                                BOOL bHasObj = TRUE );
    virtual AbstractSfxSingleTabDialog*     CreateSfxSingleTabDialog( Window* pParent, //add for SvxMeasureDialog & SvxConnectionDialog
                                                                        const SfxItemSet& rAttr,
                                                                        const SdrView* pView,
                                                                        const ResId& rResId
                                                                        );
    virtual AbstractSvxPostItDialog*        CreateSvxPostItDialog( Window* pParent, //add for SvxPostItDialog
                                                                        const SfxItemSet& rCoreSet,
                                                                        const ResId& rResId,
                                                                        BOOL bPrevNext = FALSE, BOOL bRedline = FALSE );

    // For TabPage
    virtual CreateTabPage               GetTabPageCreatorFunc( USHORT nId );
    virtual CreateSvxDistributePage     GetSvxDistributePageCreatorFunc(USHORT nId );


    virtual GetTabPageRanges            GetTabPageRangesFunc( USHORT nId );
    virtual DialogGetRanges         GetDialogGetRangesFunc( USHORT nId ); //add for SvxPostItDialog
    virtual VclAbstractDialog*          CreateSvxScriptOrgDialog( Window* pParent, const String& rLanguage );

    virtual AbstractScriptSelectorDialog*
        CreateScriptSelectorDialog(
            Window* pParent, BOOL bShowSlots = FALSE );

    virtual VclAbstractDialog* CreateScriptErrorDialog(
            Window* pParent, ::com::sun::star::uno::Any aException);

    virtual VclAbstractDialog*          CreateSvxMacroAssignDlg( Window* pParent, SfxItemSet& rSet, ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameReplace > xNameReplace, sal_uInt16 nSelectedIndex = 0 );
};

#endif

