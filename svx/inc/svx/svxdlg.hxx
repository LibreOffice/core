/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svxdlg.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 17:15:16 $
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
#ifndef _SVX_DIALOG_HXX
#define _SVX_DIALOG_HXX
// include ---------------------------------------------------------------

#include <sfx2/sfxdlg.hxx>
#ifndef SVX_HANGUL_HANJA_CONVERSION_HXX
#include <svx/hangulhanja.hxx> //add for HangulHanjaConversionDialog
#endif
class SdrModel;
class SdrView;
using namespace svx;
#include <svx/dstribut_enum.hxx>
#include <svx/rectenum.hxx> //add for enum RECT_POINT
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXCONTAINER_HPP_
#include <com/sun/star/container/XIndexContainer.hpp> //add for FmShowColsDialog
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEREPLACE_HPP_
#include <com/sun/star/container/XNameReplace.hpp> //add for SvxMacroAssignDlg
#endif
#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

namespace com{namespace sun{namespace star{
namespace linguistic2{
    class XDictionary;
    class XDictionary1;
    class XSpellChecker1;
    class XSpellChecker;
}}}}
class SvxSpellWrapper; //add for SvxSpellCheckDialog
typedef SfxTabPage* (*CreateSvxDistributePage)(Window *pParent, const SfxItemSet &rAttrSet, SvxDistributeHorizontal eHor, SvxDistributeVertical eVer);
typedef USHORT*     (*DialogGetRanges)();

struct ExchangeData;
class INetURLObject;
class GalleryTheme;
class TargetList;
class SvxHyperlinkTabPageBase;
class SearchAttrItemList;
class FmFormShell;
class Graphic;
class SdrObject;
namespace svx{ class SpellDialogChildWindow;}

//#define SVX_DIALOGFACTORY_CLASSID 0xf8e5fd97, 0x49e2, 0x4ae5, 0xac, 0x31, 0x4, 0xcb, 0xf7, 0xf3, 0xcf, 0x69
class AbstractSvxDistributeDialog :public VclAbstractDialog  //add for SvxDistributeDialog
{
public:
    virtual SvxDistributeHorizontal GetDistributeHor() const = 0;
    virtual SvxDistributeVertical GetDistributeVer() const = 0;
};

class AbstractHangulHanjaConversionDialog : public VclAbstractTerminatedDialog //add for HangulHanjaConversionDialog
{
 public:
    virtual void EnableRubySupport( sal_Bool _bVal ) = 0;
     virtual void SetByCharacter( sal_Bool _bByCharacter ) = 0;
    virtual void SetConversionDirectionState( sal_Bool _bTryBothDirections, HangulHanjaConversion::ConversionDirection _ePrimaryConversionDirection ) = 0;
     virtual void SetConversionFormat( HangulHanjaConversion::ConversionFormat _eType ) = 0;
    virtual void    SetOptionsChangedHdl( const Link& _rHdl ) = 0;
     virtual void   SetIgnoreHdl( const Link& _rHdl ) = 0;
     virtual void   SetIgnoreAllHdl( const Link& _rHdl ) = 0;
     virtual void   SetChangeHdl( const Link& _rHdl ) = 0;
     virtual void   SetChangeAllHdl( const Link& _rHdl ) = 0;
    virtual void    SetClickByCharacterHdl( const Link& _rHdl ) = 0;
     virtual void   SetConversionFormatChangedHdl( const Link& _rHdl ) = 0;
     virtual void   SetFindHdl( const Link& _rHdl ) = 0;
    virtual sal_Bool        GetUseBothDirections( ) const= 0;
    virtual HangulHanjaConversion::ConversionDirection    GetDirection( HangulHanjaConversion::ConversionDirection _eDefaultDirection ) const = 0;
    virtual void    SetCurrentString(
                    const String& _rNewString,
                    const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rSuggestions,
                    bool _bOriginatesFromDocument = true )=0;
    virtual String  GetCurrentString( ) const =0;
    virtual HangulHanjaConversion::ConversionFormat    GetConversionFormat( ) const =0;
    virtual void    FocusSuggestion( )= 0;
    virtual String  GetCurrentSuggestion( ) const =0;
};

class AbstractFmShowColsDialog : public VclAbstractDialog //add for FmShowColsDialog
{
 public:
     virtual void SetColumns(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer>& xCols)= 0;
};

class AbstractSvxZoomDialog : public VclAbstractDialog //add for SvxZoomDialog
{
 public:
    virtual void    SetLimits( USHORT nMin, USHORT nMax ) = 0;
    virtual void    HideButton( USHORT nBtnId )= 0;
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

class AbstractSearchProgress :public VclAbstractRefreshableDialog  //add for SearchProgress
{
public:
    virtual void    SetFileType( const String& rType ) = 0;
    virtual void    SetDirectory( const INetURLObject& rURL ) = 0;
    virtual PLinkStub   GetLinkStubCleanUpHdl() = 0;
};

class AbstractTakeProgress :public VclAbstractRefreshableDialog  //add for TakeProgress
{
public:
    virtual void    SetFile( const INetURLObject& rURL ) = 0;
    virtual PLinkStub   GetLinkStubCleanUpHdl() = 0;
};

class AbstractTitleDialog :public VclAbstractDialog  //add for TitleDialog
{
public:
    virtual String  GetTitle() const =0;
};

class AbstractGalleryIdDialog :public VclAbstractDialog  //add for GalleryIdDialog
{
public:
    virtual ULONG   GetId() const =0;
};

class AbstractURLDlg :public VclAbstractDialog  //add for URLDlg
{
public:
    virtual String      GetURL() const = 0;
    virtual String      GetAltText() const = 0;
    virtual String      GetDesc() const = 0;
    virtual String      GetTarget() const = 0;
    virtual String      GetName() const = 0;
};

class AbstractSvxHlinkDlgMarkWnd :public VclAbstractDialog  //add for SvxHlinkDlgMarkWnd
{
public:
    virtual BOOL    MoveTo ( Point aNewPos )const = 0;
    virtual BOOL    ConnectToDialog( BOOL bDoit = TRUE )const = 0;
    virtual void    RefreshTree ( String aStrURL ) = 0;
    virtual void    SelectEntry ( String aStrMark ) = 0;
    virtual USHORT  SetError( USHORT nError) = 0;
    // in class Window
    virtual void    SetSizePixel( const Size& rNewSize ) = 0;
    virtual Size    GetSizePixel() const = 0;
    virtual void    Hide( USHORT nFlags = 0 ) = 0;
    virtual BOOL    IsVisible() const = 0;
    virtual void    Invalidate( USHORT nFlags = 0 ) = 0;
};

class AbstractSvxCharacterMap :public VclAbstractDialog  //add for SvxCharacterMap
{
public:
     virtual void      SetText( const XubString& rStr )=0;  //From class Window
     virtual void            DisableFontSelection()=0;

    virtual const Font&     GetCharFont() const =0;
    virtual void            SetCharFont( const Font& rFont )=0;
    virtual void            SetFont( const Font& rFont ) =0;

    virtual void            SetChar( sal_Unicode c )=0;
    virtual sal_Unicode     GetChar() const=0;

    virtual String          GetCharacters() const=0;
};

class AbstractSvxSearchFormatDialog : public SfxAbstractTabDialog   //for SvxSearchFormatDialog
{
public:
    //From class Window.
    virtual void        SetText( const XubString& rStr ) =0 ;
    virtual String      GetText() const =0 ;
};

class AbstractSvxSearchSimilarityDialog :public VclAbstractDialog  //add for SvxSearchSimilarityDialog
{
public:
    virtual USHORT  GetOther() =0 ;
    virtual USHORT  GetShorter() =0 ;
    virtual USHORT  GetLonger() =0 ;
    virtual BOOL    IsRelaxed() =0 ;
};

class AbstractSvxJSearchOptionsDialog :public VclAbstractDialog  //add for SvxJSearchOptionsDialog
{
public:
    virtual INT32           GetTransliterationFlags() const = 0;
};

class AbstractFmInputRecordNoDialog :public VclAbstractDialog  //add for FmInputRecordNoDialog
{
public:
    virtual void SetValue(long dNew) = 0;
    virtual long GetValue() const = 0;
};

class AbstractSvxNewDictionaryDialog :public VclAbstractDialog  //add for SvxNewDictionaryDialog
{
public:
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XDictionary1 > GetNewDictionary() = 0;
};

class AbstractSvxNameDialog :public VclAbstractDialog  //add for SvxNameDialog
{
public:
    virtual void    GetName( String& rName ) = 0;
    virtual void    SetCheckNameHdl( const Link& rLink, bool bCheckImmediately = false ) = 0;
    virtual void    SetEditHelpId(ULONG nHelpId) = 0;
    //from class Window
    virtual void    SetHelpId( ULONG nHelpId ) = 0;
    virtual void    SetText( const XubString& rStr ) = 0;
};

///////////////////////////////////////////////////////////////////////////////////////////////
// #i68101#

class AbstractSvxObjectNameDialog :public VclAbstractDialog
{
public:
    virtual void GetName(String& rName) = 0;
    virtual void SetCheckNameHdl(const Link& rLink, bool bCheckImmediately = false) = 0;
};

class AbstractSvxObjectTitleDescDialog :public VclAbstractDialog
{
public:
    virtual void GetTitle(String& rTitle) = 0;
    virtual void GetDescription(String& rDescription) = 0;
};

///////////////////////////////////////////////////////////////////////////////////////////////

class AbstractSvxMessDialog :public VclAbstractDialog  //add for SvxMessDialog
{
public:
    virtual void    SetButtonText( USHORT nBtnId, const String& rNewTxt ) = 0;
};

class AbstractSvxMultiPathDialog :public VclAbstractDialog  //add for SvxMultiPathDialog
{
public:
    virtual String          GetPath() const = 0;
    virtual void            SetPath( const String& rPath ) = 0;
    virtual void            EnableRadioButtonMode() = 0;
    virtual void            SetTitle( const String& rNewTitle ) = 0;
};

class AbstractSvxMultiFileDialog :public AbstractSvxMultiPathDialog  //add for SvxMultiFileDialog
{
public:
    virtual String  GetFiles() const = 0;
    virtual void    SetFiles( const String& rPath ) = 0;
    //from SvxMultiPathDialog
    virtual void    SetClassPathMode() = 0;
    //From Class Window
    virtual void    SetHelpId( ULONG nHelpId ) = 0;
};

class AbstractSvxHpLinkDlg : public VclAbstractDialog  //add for SvxHpLinkDlg
{
public:
    virtual Window*     GetWindow()  = 0;
    virtual sal_Bool    QueryClose() = 0;
};

class AbstractFmSearchDialog :public VclAbstractDialog  //add for FmSearchDialog
{
public:
    virtual void SetFoundHandler(const Link& lnk) = 0;
    virtual void SetCanceledNotFoundHdl(const Link& lnk)=0;
    virtual void SetActiveField(const String& strField)=0;
};
class AbstractGraphicFilterDialog :public VclAbstractDialog  //add for GraphicFilterDialog
{
public:
    virtual Graphic GetFilteredGraphic( const Graphic& rGraphic, double fScaleX, double fScaleY ) = 0;
};

class AbstractSvxAreaTabDialog :public SfxAbstractTabDialog  //add for SvxAreaTabDialog
{
public:
    virtual void     DontDeleteColorTable() = 0;
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

class AbstractSvxPostItDialog :public VclAbstractDialog  //add for SvxPostItDialog
{
public:
    virtual void                SetText( const XubString& rStr ) = 0;  //From class Window
    virtual const SfxItemSet*   GetOutputItemSet() const = 0;
    virtual void                SetPrevHdl( const Link& rLink ) = 0;
    virtual void                SetNextHdl( const Link& rLink ) = 0;
    virtual void                EnableTravel(BOOL bNext, BOOL bPrev) = 0;
    virtual String              GetNote()  = 0;
    virtual void                SetNote(const String& rTxt)  = 0;
    virtual void                ShowLastAuthor(const String& rAuthor, const String& rDate) = 0;
    virtual void                DontChangeAuthor() = 0;
    virtual void                HideAuthor() = 0;
    virtual void                SetReadonlyPostIt(BOOL bDisable) = 0;
    virtual BOOL                IsOkEnabled() const  = 0;
    virtual Window *            GetWindow() = 0;
};

//-------------------------------------------------------------
class SVX_DLLPUBLIC SvxAbstractDialogFactory : public SfxAbstractDialogFactory
{
public:
    static SvxAbstractDialogFactory* Create();

    virtual SfxAbstractTabDialog*       CreateTextTabDialog( Window* pParent,
                                            const SfxItemSet* pAttrSet,
                                            sal_uInt32 nResId,
                                            SdrView* pView,
                                            SdrModel* pModel=0 ) = 0 ; //add for SvxTextTabDialog

    virtual AbstractSvxCaptionDialog* CreateCaptionDialog( Window* pParent,
                                        const SdrView* pView,
                                        sal_uInt32 nResId,
                                        USHORT nAnchorTypes = 0 ) = 0;      //add for SvxCaptionTabDialog CHINA001

    virtual AbstractSvxDistributeDialog*    CreateSvxDistributeDialog(Window* pParent,
                                            const SfxItemSet& rAttr,
                                            sal_uInt32 nResId,
                                            SvxDistributeHorizontal eHor = SvxDistributeHorizontalNone,
                                            SvxDistributeVertical eVer = SvxDistributeVerticalNone)= 0;

    virtual AbstractHangulHanjaConversionDialog * CreateHangulHanjaConversionDialog( Window* _pParent,  //add for HangulHanjaConversionDialog CHINA001
                                            HangulHanjaConversion::ConversionDirection _ePrimaryDirection,
                                            sal_uInt32 nResId) = 0;

    virtual AbstractFmShowColsDialog * CreateFmShowColsDialog( Window* pParent,  //add for FmShowColsDialog
                                             sal_uInt32 nResId) = 0 ;

    virtual AbstractSvxZoomDialog * CreateSvxZoomDialog( Window* pParent,  //add for SvxZoomDialog
                                            const SfxItemSet& rCoreSet,
                                            sal_uInt32 nResId) = 0 ;

    virtual AbstractSpellDialog *   CreateSvxSpellDialog(Window* pParent,
                                            SfxBindings* pBindings,
                                            svx::SpellDialogChildWindow* pSpellChildWindow )=0;

    virtual VclAbstractRefreshableDialog * CreateActualizeProgressDialog( Window* pParent,  //add for ActualizeProgress
                                            GalleryTheme* pThm,
                                            sal_uInt32 nResId) = 0;
    virtual AbstractSearchProgress * CreateSearchProgressDialog( Window* pParent,  //add for SearchProgress
                                            const INetURLObject& rStartURL,
                                            sal_uInt32 nResId) = 0;
    virtual AbstractTakeProgress * CreateTakeProgressDialog( Window* pParent,  //add for TakeProgress
                                            sal_uInt32 nResId) = 0;
    virtual AbstractTitleDialog * CreateTitleDialog( Window* pParent,  //add for TitleDialog
                                             const String& rOldText,
                                            sal_uInt32 nResId) = 0;
    virtual AbstractGalleryIdDialog * CreateGalleryIdDialog( Window* pParent,  //add for SvxZoomDialog
                                            GalleryTheme* pThm,
                                            sal_uInt32 nResId) = 0;
    virtual VclAbstractDialog2 * CreateGalleryThemePropertiesDialog( Window* pParent,  //add for GalleryThemeProperties
                                            ExchangeData* pData,
                                            SfxItemSet* pItemSet,
                                            sal_uInt32 nResId) = 0;
    virtual AbstractURLDlg * CreateURLDialog( Window* pParent,
                                            const String& rURL, const String& rAltText, const String& rDescription,
                                            const String& rTarget, const String& rName,
                                            TargetList& rTargetList,
                                            sal_uInt32 nResId) = 0;
    virtual AbstractSvxHlinkDlgMarkWnd* CreateSvxHlinkDlgMarkWndDialog( SvxHyperlinkTabPageBase* pParent, sal_uInt32 nResId ) =0; //add for SvxHlinkDlgMarkWnd

    virtual AbstractSvxCharacterMap * CreateSvxCharacterMap( Window* pParent,  //add for SvxCharacterMap
                                                            sal_uInt32 nResId,
                                                            BOOL bOne = TRUE ) = 0;
    virtual SfxAbstractTabDialog* CreateTabItemDialog( Window* pParent, //add for SvxSearchFormatDialog
                                            const SfxItemSet& rSet,
                                            sal_uInt32 nResId) = 0;
    virtual VclAbstractDialog*      CreateSvxSearchAttributeDialog( Window* pParent,  //add for SvxSearchAttributeDialog
                                            SearchAttrItemList& rLst,
                                            const USHORT* pWhRanges,
                                            sal_uInt32 nResId )= 0;
    virtual AbstractSvxSearchSimilarityDialog * CreateSvxSearchSimilarityDialog( Window* pParent,  //add for SvxSearchSimilarityDialog
                                                            BOOL bRelax,
                                                            USHORT nOther,
                                                            USHORT nShorter,
                                                            USHORT nLonger ) = 0;
    virtual SfxAbstractTabDialog* CreateSvxBorderBackgroundDlg( Window* pParent, //add for SvxBorderBackgroundDlg
                                            const SfxItemSet& rCoreSet,
                                            sal_uInt32 nResId,
                                            BOOL bEnableSelector = FALSE) = 0;
    virtual AbstractSvxTransformTabDialog* CreateSvxTransformTabDialog( Window* pParent, //add for SvxTransformTabDialog
                                                                const SfxItemSet* pAttr,
                                                                const SdrView* pView,
                                                                sal_uInt32 nResId,
                                                                USHORT nAnchorTypes = 0) = 0;
    virtual SfxAbstractTabDialog* CreateSchTransformTabDialog( Window* pParent,  //add for SchTransformTabDialog
                                                                const SfxItemSet* pAttr,
                                                                const SdrView* pSdrView,
                                                                sal_uInt32 nResId,
                                                                bool bSizeTabPage = false
                                                                 )=0;
    virtual AbstractSvxJSearchOptionsDialog * CreateSvxJSearchOptionsDialog( Window* pParent,
                                                            const SfxItemSet& rOptionsSet, USHORT nUniqueId,
                                                            INT32 nInitialFlags,
                                                            sal_uInt32 nResId) = 0; //add for SvxJSearchOptionsDialog
    virtual AbstractFmInputRecordNoDialog * CreateFmInputRecordNoDialog( Window* pParent,
                                                            sal_uInt32 nResId) = 0; //add for FmInputRecordNoDialog
    virtual AbstractSvxNewDictionaryDialog* CreateSvxNewDictionaryDialog( Window* pParent,
                                            ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XSpellChecker1 >  &xSpl,
                                            sal_uInt32 nResId ) = 0; //add for SvxNewDictionaryDialog
    virtual VclAbstractDialog *     CreateSvxEditDictionaryDialog( Window* pParent,
                                            const String& rName,
                                            ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XSpellChecker1> &xSpl,
                                            sal_uInt32 nResId) = 0;//add for SvxEditDictionaryDialog
    virtual AbstractSvxNameDialog *     CreateSvxNameDialog( Window* pParent,
                                            const String& rName, const String& rDesc,
                                            sal_uInt32 nResId ) = 0; //add for SvxNameDialog

    // #i68101#
    virtual AbstractSvxObjectNameDialog* CreateSvxObjectNameDialog(Window* pParent, const String& rName, sal_uInt32 nResId) = 0;
    virtual AbstractSvxObjectTitleDescDialog* CreateSvxObjectTitleDescDialog(Window* pParent, const String& rTitle, const String& rDescription, sal_uInt32 nResId) = 0;

    virtual AbstractSvxMessDialog *     CreateSvxMessDialog( Window* pParent, sal_uInt32 nResId,
                                            const String& rText, const String& rDesc,
                                            Image* pImg = NULL ) = 0; //add for SvxMessDialog

    virtual AbstractSvxMultiPathDialog *    CreateSvxMultiPathDialog( Window* pParent, sal_uInt32 nResId,
                                                BOOL bEmptyAllowed = FALSE ) = 0 ; //add for SvxMultiPathDialog
    virtual AbstractSvxMultiFileDialog *    CreateSvxMultiFileDialog( Window* pParent, sal_uInt32 nResId,
                                                BOOL bEmptyAllowed = FALSE ) = 0 ; //add for SvxMultiFileDialog
    virtual AbstractSvxHpLinkDlg *  CreateSvxHpLinkDlg (Window* pParent,  //add for SvxMultiFileDialog
                                            SfxBindings* pBindings,
                                            sal_uInt32 nResId)=0;
    virtual AbstractFmSearchDialog* CreateFmSearchDialog(Window* pParent, //add for FmSearchDialog
                                                        const String& strInitialText,
                                                        const ::std::vector< String >& _rContexts,
                                                        sal_Int16 nInitialContext,
                                                        const Link& lnkContextSupplier)=0;
    virtual AbstractGraphicFilterDialog *   CreateGraphicFilterEmboss (Window* pParent,  //add for GraphicFilterEmboss
                                                const Graphic& rGraphic,
                                                RECT_POINT eLightSource,
                                                sal_uInt32 nResId)=0;
    virtual AbstractGraphicFilterDialog *   CreateGraphicFilterPosterSepia (Window* pParent,  //add for GraphicFilterPoster & GraphicFilterSepia
                                                const Graphic& rGraphic,
                                                USHORT nCount,
                                                sal_uInt32 nResId)=0;
    virtual AbstractGraphicFilterDialog *   CreateGraphicFilterSolarize (Window* pParent,  //add for GraphicFilterSolarize
                                                const Graphic& rGraphic,
                                                BYTE nGreyThreshold, BOOL bInvert,
                                                sal_uInt32 nResId)=0;
    virtual AbstractGraphicFilterDialog *   CreateGraphicFilterMosaic (Window* pParent,  //add for GraphicFilterMosaic
                                                const Graphic& rGraphic,
                                                USHORT nTileWidth, USHORT nTileHeight, BOOL bEnhanceEdges,
                                                sal_uInt32 nResId)=0;
    virtual AbstractSvxAreaTabDialog*       CreateSvxAreaTabDialog( Window* pParent,//add for SvxAreaTabDialog
                                                            const SfxItemSet* pAttr,
                                                            SdrModel* pModel,
                                                            sal_uInt32 nResId,
                                                            const SdrView* pSdrView = NULL ) = 0 ;
    virtual SfxAbstractTabDialog*           CreateSvxLineTabDialog( Window* pParent, const SfxItemSet* pAttr, //add forSvxLineTabDialog
                                                                 SdrModel* pModel,
                                                                 sal_uInt32 nResId,
                                                                 const SdrObject* pObj = NULL,
                                                                BOOL bHasObj = TRUE )=0;
    virtual AbstractSfxSingleTabDialog*     CreateSfxSingleTabDialog( Window* pParent, //add for SvxMeasureDialog & SvxConnectionDialog
                                                                        const SfxItemSet& rAttr,
                                                                        const SdrView* pView,
                                                                        sal_uInt32 nResId
                                                                        )=0;
    virtual AbstractSvxPostItDialog*        CreateSvxPostItDialog( Window* pParent, //add for SvxPostItDialog
                                                                        const SfxItemSet& rCoreSet,
                                                                        sal_uInt32 nResId,
                                                                        BOOL bPrevNext = FALSE, BOOL bRedline = FALSE )=0;
    virtual VclAbstractDialog*          CreateSvxScriptOrgDialog( Window* pParent, const String& rLanguage ) = 0;

    virtual CreateSvxDistributePage     GetSvxDistributePageCreatorFunc(USHORT nId ) = 0; //  add for SvxDistributePage
    virtual DialogGetRanges         GetDialogGetRangesFunc( USHORT nId ) = 0; //add for SvxPostItDialog

    virtual AbstractScriptSelectorDialog*
        CreateScriptSelectorDialog(
            Window* pParent,
            BOOL bShowSlots,
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
};

#endif

