/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dlgfact.hxx,v $
 *
 *  $Revision: 1.34 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 09:37:45 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _CUI_DLGFACT_HXX
#define _CUI_DLGFACT_HXX
// include ---------------------------------------------------------------
#include <svx/svxdlg.hxx> //CHINA001 #include <svx/svxdlg.hxx>

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEREPLACE_HPP_
#include <com/sun/star/container/XNameReplace.hpp>
#endif

#include "tools/link.hxx"

#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
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
//  virtual void    Show( BOOL bVisible = TRUE, USHORT nFlags = 0 )

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

class AbstractTabDialog_Impl : public SfxAbstractTabDialog
{
    DECL_ABSTDLG_BASE(AbstractTabDialog_Impl,SfxTabDialog)
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
    virtual void    SetConversionDirectionState( sal_Bool _bTryBothDirections, HangulHanjaConversion::ConversionDirection _ePrimaryConversionDirection );
    virtual void    SetConversionFormat( HangulHanjaConversion::ConversionFormat _eType );
    virtual void    SetOptionsChangedHdl( const Link& _rHdl );
    virtual void    SetIgnoreHdl( const Link& _rHdl );
    virtual void    SetIgnoreAllHdl( const Link& _rHdl ) ;
    virtual void    SetChangeHdl( const Link& _rHdl ) ;
    virtual void    SetChangeAllHdl( const Link& _rHdl ) ;
    virtual void    SetClickByCharacterHdl( const Link& _rHdl ) ;
    virtual void    SetConversionFormatChangedHdl( const Link& _rHdl ) ;
    virtual void    SetFindHdl( const Link& _rHdl );
    virtual sal_Bool        GetUseBothDirections( ) const;
    virtual HangulHanjaConversion::ConversionDirection    GetDirection( HangulHanjaConversion::ConversionDirection _eDefaultDirection ) const;
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
    DECL_ABSTDLG_BASE(AbstractFmShowColsDialog_Impl,FmShowColsDialog)
     virtual void   SetColumns(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer>& xCols);
};
//for FmShowColsDialog end


//for SvxZoomDialog begin
class SvxZoomDialog;
class AbstractSvxZoomDialog_Impl : public AbstractSvxZoomDialog
{
    DECL_ABSTDLG_BASE(AbstractSvxZoomDialog_Impl,SvxZoomDialog)
    virtual void    SetLimits( USHORT nMin, USHORT nMax );
    virtual void    HideButton( USHORT nBtnId );
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
    virtual ULONG   GetId() const ;

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
    DECL_ABSTDLG_BASE(AbstractSvxCharacterMap_Impl,SvxCharacterMap)
    virtual void      SetText( const XubString& rStr ); //From Class Window
    virtual void               DisableFontSelection();

    virtual const Font&     GetCharFont() const ;
    virtual void            SetCharFont( const Font& rFont );
    virtual void            SetFont( const Font& rFont ) ;

    // TODO: change to UCS4
    virtual void            SetChar( sal_Unicode );
    virtual sal_Unicode     GetChar() const;

    virtual String          GetCharacters() const;
};
//for SvxCharacterMap end

//for SvxSearchSimilarityDialog begin
class SvxSearchSimilarityDialog;
class AbstractSvxSearchSimilarityDialog_Impl :public AbstractSvxSearchSimilarityDialog
{
    DECL_ABSTDLG_BASE(AbstractSvxSearchSimilarityDialog_Impl,SvxSearchSimilarityDialog)
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
    DECL_ABSTDLG_BASE(AbstractSvxJSearchOptionsDialog_Impl,SvxJSearchOptionsDialog)
    virtual INT32           GetTransliterationFlags() const;
};
//for SvxJSearchOptionsDialog end

class AbstractSvxTransformTabDialog_Impl : public AbstractSvxTransformTabDialog
{
    DECL_ABSTDLG_BASE(AbstractSvxTransformTabDialog_Impl,SvxTransformTabDialog)
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
    DECL_ABSTDLG_BASE(AbstractSvxCaptionDialog_Impl,SvxCaptionTabDialog)
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
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XDictionary1 > GetNewDictionary();
};
//for SvxNewDictionaryDialog end

//for SvxNameDialog end
class SvxNameDialog;
class AbstractSvxNameDialog_Impl :public AbstractSvxNameDialog
{
    DECL_ABSTDLG_BASE(AbstractSvxNameDialog_Impl,SvxNameDialog)
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
    DECL_LINK(CheckNameHdl, Window*);
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
    virtual void    SetButtonText( USHORT nBtnId, const String& rNewTxt );
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
    virtual void    SetHelpId( ULONG nHelpId ) ;

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

//add for SvxAreaTabDialog begin
class SvxAreaTabDialog;
class AbstractSvxAreaTabDialog_Impl :public AbstractSvxAreaTabDialog
{
    DECL_ABSTDLG_BASE(AbstractSvxAreaTabDialog_Impl,SvxAreaTabDialog)
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
    DECL_ABSTDLG_BASE(AbstractSfxSingleTabDialog_Impl,SfxSingleTabDialog)
    virtual const SfxItemSet*   GetOutputItemSet() const;

};
//add for SvxMeasureDialog & SvxConnectionDialog end

class AbstractInsertObjectDialog_Impl : public SfxAbstractInsertObjectDialog
{
    DECL_ABSTDLG_BASE(AbstractInsertObjectDialog_Impl, InsertObjectDialog_Impl)
    virtual com::sun::star::uno::Reference < com::sun::star::embed::XEmbeddedObject > GetObject();
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > GetIconIfIconified( ::rtl::OUString* pGraphicMediaType );
    virtual BOOL IsCreateNew();
};

class AbstractPasteDialog_Impl : public SfxAbstractPasteDialog
{
public:
    DECL_ABSTDLG_BASE(AbstractPasteDialog_Impl, SvPasteObjectDialog )
    virtual void Insert( SotFormatStringId nFormat, const String & rFormatName );
    virtual void SetObjName( const SvGlobalName & rClass, const String & rObjName );
    virtual ULONG GetFormat( const TransferableDataHelper& aHelper,
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
    virtual VclAbstractDialog*          CreateVclDialog( Window* pParent, sal_uInt32 nResId );
    virtual VclAbstractDialog*          CreateSfxDialog( Window* pParent, const SfxBindings& rBindings, sal_uInt32 nResId );
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
                                            sal_uInt32 nResId,
                                            SdrView* pView,
                                            SdrModel* pModel=0 ); //add for SvxTextTabDialog
    virtual SfxAbstractTabDialog*       CreateTabItemDialog( Window* pParent,
                                            const SfxItemSet& rSet,
                                            sal_uInt32 nResId); //add by CHINA001
    virtual AbstractSvxCaptionDialog*   CreateCaptionDialog( Window* pParent,
                                            const SdrView* pView,
                                            sal_uInt32 nResId,
                                            USHORT nAnchorTypes = 0 );      //add for SvxCaptionTabDialog CHINA001
    virtual AbstractSvxDistributeDialog*    CreateSvxDistributeDialog(Window* pParent,
                                            const SfxItemSet& rAttr,
                                            sal_uInt32 nResId,
                                            SvxDistributeHorizontal eHor = SvxDistributeHorizontalNone,
                                            SvxDistributeVertical eVer = SvxDistributeVerticalNone);
    virtual SfxAbstractInsertObjectDialog* CreateInsertObjectDialog( Window* pParent, USHORT nSlotId,
            const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& xStor,
            const SvObjectServerList* pList = 0 );
    virtual VclAbstractDialog*          CreateEditObjectDialog( Window* pParent, USHORT nSlotId,
            const com::sun::star::uno::Reference < com::sun::star::embed::XEmbeddedObject >& xObj );
   virtual  SfxAbstractPasteDialog*         CreatePasteDialog( Window* pParent );
   virtual  SfxAbstractLinksDialog*         CreateLinksDialog( Window* pParent, sfx2::SvLinkManager* pMgr, BOOL bHTML, sfx2::SvBaseLink* p=0  );

   virtual AbstractHangulHanjaConversionDialog * CreateHangulHanjaConversionDialog( Window* _pParent,  //add for HangulHanjaConversionDialog CHINA001
                                            HangulHanjaConversion::ConversionDirection _ePrimaryDirection,
                                            sal_uInt32 nResId);

   virtual AbstractFmShowColsDialog * CreateFmShowColsDialog( Window* pParent,  //add for FmShowColsDialog
                                             sal_uInt32 nResId);
   virtual AbstractSvxZoomDialog * CreateSvxZoomDialog( Window* pParent,  //add for SvxZoomDialog
                                            const SfxItemSet& rCoreSet,
                                            sal_uInt32 nResId);
   virtual SfxAbstractTabDialog* CreateSvxBorderBackgroundDlg( Window* pParent, //add for SvxBorderBackgroundDlg
                                            const SfxItemSet& rCoreSet,
                                            sal_uInt32 nResId,
                                            BOOL bEnableSelector = FALSE) ;
   virtual AbstractSvxTransformTabDialog* CreateSvxTransformTabDialog( Window* pParent, //add for SvxTransformTabDialog
                                                                const SfxItemSet* pAttr,
                                                                const SdrView* pView,
                                                                sal_uInt32 nResId,
                                                                USHORT nAnchorTypes = 0) ;
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

//STRIP001    virtual AbstractSvxSpellCheckDialog * CreateSvxSpellCheckDialog( Window* pParent, //add for SvxSpellCheckDialog
//STRIP001    ::com::sun::star::uno::Reference<
//STRIP001    ::com::sun::star::linguistic2::XSpellChecker1 >  &xChecker,
//STRIP001    SvxSpellWrapper* pWrapper,
//STRIP001    sal_uInt32 nResId);
    virtual VclAbstractRefreshableDialog * CreateActualizeProgressDialog( Window* pParent,  //add for ActualizeProgress
                                            GalleryTheme* pThm,
                                            sal_uInt32 nResId);
    virtual AbstractSearchProgress * CreateSearchProgressDialog( Window* pParent,  //add for SearchProgress
                                            const INetURLObject& rStartURL,
                                            sal_uInt32 nResId);
    virtual AbstractTakeProgress * CreateTakeProgressDialog( Window* pParent,  //add for TakeProgress
                                            sal_uInt32 nResId);
    virtual AbstractTitleDialog * CreateTitleDialog( Window* pParent,  //add for TitleDialog
                                             const String& rOldText,
                                            sal_uInt32 nResId);
    virtual AbstractGalleryIdDialog * CreateGalleryIdDialog( Window* pParent,  //add for SvxZoomDialog
                                            GalleryTheme* pThm,
                                            sal_uInt32 nResId);
    virtual VclAbstractDialog2 * CreateGalleryThemePropertiesDialog( Window* pParent,  //add for GalleryThemeProperties
                                            ExchangeData* pData,
                                            SfxItemSet* pItemSet,
                                            sal_uInt32 nResId);
    virtual AbstractURLDlg * CreateURLDialog( Window* pParent,  //add for URLDlg
                                            const String& rURL, const String& rAltText, const String& rDescription,
                                            const String& rTarget, const String& rName,
                                            TargetList& rTargetList,
                                            sal_uInt32 nResId);
    virtual AbstractSvxHlinkDlgMarkWnd* CreateSvxHlinkDlgMarkWndDialog( SvxHyperlinkTabPageBase* pParent, sal_uInt32 nResId ); //add for SvxHlinkDlgMarkWnd

    virtual AbstractSvxCharacterMap * CreateSvxCharacterMap( Window* pParent,
                                                            sal_uInt32 nResId,
                                                            BOOL bOne = TRUE ) ; //add for SvxCharacterMap
    virtual VclAbstractDialog*      CreateSvxSearchAttributeDialog( Window* pParent,
                                            SearchAttrItemList& rLst,
                                            const USHORT* pWhRanges,
                                            sal_uInt32 nResId ); //add for SvxSearchAttributeDialog
    virtual AbstractSvxSearchSimilarityDialog * CreateSvxSearchSimilarityDialog( Window* pParent,
                                                            BOOL bRelax,
                                                            USHORT nOther,
                                                            USHORT nShorter,
                                                            USHORT nLonger); //add for SvxSearchSimilarityDialog
    virtual AbstractSvxJSearchOptionsDialog * CreateSvxJSearchOptionsDialog( Window* pParent,
                                                            const SfxItemSet& rOptionsSet, USHORT nUniqueId,
                                                            INT32 nInitialFlags,
                                                            sal_uInt32 nResId); //add for SvxJSearchOptionsDialog
    virtual AbstractFmInputRecordNoDialog * CreateFmInputRecordNoDialog( Window* pParent,
                                                            sal_uInt32 nResId); //add for FmInputRecordNoDialog
    virtual AbstractSvxNewDictionaryDialog* CreateSvxNewDictionaryDialog( Window* pParent,
                                            ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XSpellChecker1 >  &xSpl,
                                            sal_uInt32 nResId ); //add for SvxNewDictionaryDialog
    virtual VclAbstractDialog *     CreateSvxEditDictionaryDialog( Window* pParent,
                                            const String& rName,
                                            ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XSpellChecker1> &xSpl,
                                            sal_uInt32 nResId);//add for SvxEditDictionaryDialog
    virtual AbstractSvxNameDialog *     CreateSvxNameDialog( Window* pParent,
                                            const String& rName, const String& rDesc,
                                            sal_uInt32 nResId ); //add for SvxNameDialog

    // #i68101#
    virtual AbstractSvxObjectNameDialog* CreateSvxObjectNameDialog(Window* pParent, const String& rName, sal_uInt32 nResId);
    virtual AbstractSvxObjectTitleDescDialog* CreateSvxObjectTitleDescDialog(Window* pParent, const String& rTitle, const String& rDescription, sal_uInt32 nResId);

    virtual AbstractSvxMessDialog *     CreateSvxMessDialog( Window* pParent, sal_uInt32 nResId,
                                            const String& rText, const String& rDesc,
                                            Image* pImg = NULL ); //add for SvxMessDialog
    virtual AbstractSvxMultiPathDialog *    CreateSvxMultiPathDialog( Window* pParent, sal_uInt32 nResId,
                                                BOOL bEmptyAllowed = FALSE ); //add for SvxMultiPathDialog
    virtual AbstractSvxMultiFileDialog *    CreateSvxMultiFileDialog( Window* pParent, sal_uInt32 nResId,
                                                BOOL bEmptyAllowed = FALSE ); //add for SvxMultiFileDialog
    virtual AbstractSvxHpLinkDlg *          CreateSvxHpLinkDlg (Window* pParent,  //add for SvxHpLink
                                            SfxBindings* pBindings,
                                            sal_uInt32 nResId);
    virtual AbstractFmSearchDialog* CreateFmSearchDialog(Window* pParent, //add for FmSearchDialog
                                                        const String& strInitialText,
                                                        const ::std::vector< String >& _rContexts,
                                                        sal_Int16 nInitialContext,
                                                        const Link& lnkContextSupplier);
    virtual AbstractGraphicFilterDialog *   CreateGraphicFilterEmboss (Window* pParent,  //add for GraphicFilterEmboss
                                            const Graphic& rGraphic,
                                            RECT_POINT eLightSource,
                                            sal_uInt32 nResId);
    virtual AbstractGraphicFilterDialog *   CreateGraphicFilterPosterSepia (Window* pParent,  //add for GraphicFilterPoster & GraphicFilterSepia
                                            const Graphic& rGraphic,
                                            USHORT nCount,
                                            sal_uInt32 nResId);
    virtual AbstractGraphicFilterDialog *   CreateGraphicFilterSolarize (Window* pParent,  //add for GraphicFilterSolarize
                                            const Graphic& rGraphic,
                                            BYTE nGreyThreshold, BOOL bInvert,
                                            sal_uInt32 nResId);
    virtual AbstractGraphicFilterDialog *   CreateGraphicFilterMosaic (Window* pParent,  //add for GraphicFilterMosaic
                                            const Graphic& rGraphic,
                                            USHORT nTileWidth, USHORT nTileHeight, BOOL bEnhanceEdges,
                                            sal_uInt32 nResId);
    virtual AbstractSvxAreaTabDialog*       CreateSvxAreaTabDialog( Window* pParent,//add for SvxAreaTabDialog
                                                            const SfxItemSet* pAttr,
                                                            SdrModel* pModel,
                                                            sal_uInt32 nResId,
                                                            const SdrView* pSdrView = NULL ); //add for SvxAreaTabDialog
    virtual SfxAbstractTabDialog*           CreateSvxLineTabDialog( Window* pParent, const SfxItemSet* pAttr, //add for SvxLineTabDialog
                                                                 SdrModel* pModel,
                                                                 sal_uInt32 nResId,
                                                                 const SdrObject* pObj = NULL,
                                                                BOOL bHasObj = TRUE );
    virtual AbstractSfxSingleTabDialog*     CreateSfxSingleTabDialog( Window* pParent, //add for SvxMeasureDialog & SvxConnectionDialog
                                                                        const SfxItemSet& rAttr,
                                                                        const SdrView* pView,
                                                                        sal_uInt32 nResId
                                                                        );
    virtual AbstractSvxPostItDialog*        CreateSvxPostItDialog( Window* pParent, //add for SvxPostItDialog
                                                                        const SfxItemSet& rCoreSet,
                                                                        sal_uInt32 nResId,
                                                                        BOOL bPrevNext = FALSE, BOOL bRedline = FALSE );

    // For TabPage
    virtual CreateTabPage               GetTabPageCreatorFunc( USHORT nId );
    virtual CreateSvxDistributePage     GetSvxDistributePageCreatorFunc(USHORT nId );


    virtual GetTabPageRanges            GetTabPageRangesFunc( USHORT nId );
    virtual DialogGetRanges         GetDialogGetRangesFunc( USHORT nId ); //add for SvxPostItDialog
    virtual VclAbstractDialog*          CreateSvxScriptOrgDialog( Window* pParent, const String& rLanguage );

    virtual AbstractScriptSelectorDialog*
        CreateScriptSelectorDialog(
            Window* pParent,
            BOOL bShowSlots,
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
};

#endif

