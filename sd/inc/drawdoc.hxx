/*************************************************************************
 *
 *  $RCSfile: drawdoc.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: dl $ $Date: 2000-11-27 09:06:55 $
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

#ifndef _DRAWDOC_HXX
#define _DRAWDOC_HXX

#ifndef _SV_PRINT_HXX
#include <vcl/print.hxx>
#endif
#ifndef _FM_FMMODEL_HXX
#include <svx/fmmodel.hxx>
#endif
#ifndef _PRESENTATION_HXX
#include "pres.hxx"
#endif
#ifndef _SVX_PAGEITEM_HXX //autogen
#include <svx/pageitem.hxx>
#endif
#ifndef _UNOTOOLS_CHARCLASS_HXX
#include <unotools/charclass.hxx>
#endif
#ifndef _TOOLS_INTN_HXX
#include <tools/intn.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif

#ifdef SVX_LIGHT
#define SdOutliner Outliner
#endif
#ifndef _RSCSFX_HXX
#include <rsc/rscsfx.hxx>
#endif


class Timer;
class SfxObjectShell;
class SdPage;
class FrameView;
class SdDrawDocShell;
class SdOutliner;
class SdAnimationInfo;
class SdIMapInfo;
class IMapObject;
class SdStyleSheetPool;
class SfxMedium;
class SvxSearchItem;
class SdrOle2Obj;
class EditStatus;
class Graphic;
class Point;
class Window;
struct SpellCallbackInfo;
struct StyleRequestData;

#ifndef SVX_LIGHT
#ifndef SV_DECL_SDDRAWDOCSHELL_DEFINED
#define SV_DECL_SDDRAWDOCSHELL_DEFINED
SV_DECL_REF(SdDrawDocShell)
#endif
#endif

struct StyleReplaceData
{
    SfxStyleFamily  nFamily;
    SfxStyleFamily  nNewFamily;
    String          aName;
    String          aNewName;
};

enum DocCreationMode
{
    NEW_DOC,
    DOC_LOADED
};

#ifdef SVX_LIGHT
class SvStream;

class SdDrawDocShell
{
private:
    SvStream* pStream;
    Printer* pPrinter;
public:
    SdDrawDocShell( SvStream* pS ) { pPrinter = NULL; pStream = pS; }
    ~SdDrawDocShell() { delete pPrinter; }

    virtual void SetPrinter( Printer* pPrntr ) { pPrinter = pPrntr; }
    virtual Printer* GetPrinter( BOOL bCreate ) { if( pPrinter == NULL && bCreate ) pPrinter = new Printer(); return pPrinter; }

    virtual SvStream* GetDocumentStream(FASTBOOL& rbDeleteAfterUse) const { return pStream; }
};

#endif


//==================================================================

class SdDrawDocument : public FmFormModel
{
protected:
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > createUnoModel();

public:
    static SdDrawDocument* pDocLockedInsertingLinks;  // static to prevent recursions while resolving links

private:
    SdOutliner*       pOutliner;          // Lokaler Outliner fuer den Gliederungsmodus
    SdOutliner*       pInternalOutliner;  // Interner Outliner zum Erzeugen von Textobjekten
    Timer*            pWorkStartupTimer;
    Timer*            pOnlineSpellingTimer;
    List*             pOnlineSpellingList;
    List*             pDeletedPresObjList;
    SvxSearchItem*    pOnlineSearchItem;
    List*             pFrameViewList;
    List*             pCustomShowList;
    SdDrawDocShell*   pDocSh;
    BOOL              bHasOnlineSpellErrors;
    BOOL              bInitialOnlineSpellingEnabled;
    String            aBookmarkFile;      // Zum Laden von Bookmarks
#ifndef SVX_LIGHT
    SdDrawDocShellRef xBookmarkDocShRef;  // Zum Laden von Bookmarks
#endif
    String            aPresPage;
    BOOL              bNewOrLoadCompleted;
    BOOL              bPresAll;
    BOOL              bPresEndless;
    BOOL              bPresManual;
    BOOL              bPresMouseVisible;
    BOOL              bPresMouseAsPen;
    BOOL              bStartPresWithNavigator;
    BOOL              bAnimationAllowed;
    BOOL              bPresLockedPages;
    BOOL              bPresAlwaysOnTop;
    BOOL              bPresFullScreen;
    ULONG             nPresPause;
    BOOL              bPresShowLogo;
    BOOL              bSdDataObj;
    BOOL              bOnlineSpell;
    BOOL              bHideSpell;
    BOOL              bCustomShow;
    ULONG             nPresFirstPage;
    LanguageType      eLanguage;
    LanguageType      eLanguageCJK;
    LanguageType      eLanguageCTL;
    SvxNumType        ePageNumType;
    Link              aOldNotifyUndoActionHdl;
#ifndef SVX_LIGHT
    SdDrawDocShellRef xAllocedDocShRef;   // Fuer AllocModel()
    BOOL              bAllocDocSh;        // Fuer AllocModel()
#endif
    DocumentType      eDocType;
    UINT16            nFileFormatVersion;

    void  UpdatePageObjectsInNotes(USHORT nStartPos);
    DECL_LINK(NotifyUndoActionHdl, SfxUndoAction*);
    DECL_LINK(WorkStartupHdl, Timer*);
    DECL_LINK(OnlineSpellingHdl, Timer*);
    DECL_LINK(OnlineSpellEventHdl, EditStatus*);
    void FillOnlineSpellingList(SdPage* pPage);
    void SpellObject(SdrTextObj* pObj);

    International*                      mpInternational;
    ::com::sun::star::lang::Locale*     mpLocale;
    CharClass*                          mpCharClass;

public:
    TYPEINFO();

    SdDrawDocument(DocumentType eType, SfxObjectShell* pDocSh);
    ~SdDrawDocument();

    virtual SdrModel* AllocModel() const;
    virtual SdrPage*  AllocPage(FASTBOOL bMasterPage);
#ifndef SVX_LIGHT
    virtual const SdrModel* LoadModel(const String& rFileName);
    virtual void DisposeLoadedModels();
#endif
    virtual FASTBOOL IsReadOnly() const;

    SfxItemPool&    GetPool() { return( *pItemPool ); }

    SdOutliner*     GetOutliner(BOOL bCreateOutliner=TRUE);
    SdOutliner*     GetInternalOutliner(BOOL bCreateOutliner=TRUE);

    SdDrawDocShell* GetDocSh() { return(pDocSh) ; }

    LanguageType    GetLanguage() const { return eLanguage; }
    void            SetLanguage( LanguageType eNewLang );
    LanguageType    GetLanguageCJK() const { return eLanguageCJK; }
    void            SetLanguageCJK( LanguageType eNewLang );
    LanguageType    GetLanguageCTL() const { return eLanguageCTL; }
    void            SetLanguageCTL( LanguageType eNewLang );

    SvxNumType      GetPageNumType() const { return ePageNumType; }
    void            SetPageNumType(SvxNumType eType) { ePageNumType = eType; }
    String          CreatePageNumValue(USHORT nNum) const;

    DocumentType    GetDocumentType() const { return eDocType; }

#ifndef SVX_LIGHT
    void            SetAllocDocSh(BOOL bAlloc);
#endif

    void     CreatingDataObj(BOOL bDataObj) { bSdDataObj = bDataObj; }

    void     CreateFirstPages();
    BOOL     CreateMissingNotesAndHandoutPages();

    void     MovePage(USHORT nPgNum, USHORT nNewPos);
    void     InsertPage(SdrPage* pPage, USHORT nPos=0xFFFF);
    void     DeletePage(USHORT nPgNum);
    SdrPage* RemovePage(USHORT nPgNum);
    void     RemoveUnnessesaryMasterPages( SdPage* pMaster=NULL, BOOL bOnlyDuplicatePages=FALSE, BOOL bUndo=TRUE );
    void     SetMasterPage(USHORT nSdPageNum, const String& rLayoutName,
                           SdDrawDocument* pSourceDoc, BOOL bMaster, BOOL bCheckMasters);

    SdDrawDocument* OpenBookmarkDoc(const String& rBookmarkFile);
    SdDrawDocument* OpenBookmarkDoc(SfxMedium& rMedium);
    BOOL     InsertBookmark(List* pBookmarkList, List* pExchangeList, BOOL bLink,
                            BOOL bReplace, USHORT nPgPos, BOOL bNoDialogs,
                            SdDrawDocShell* pBookmarkDocSh, BOOL bCopy,
                            Point* pObjPos);
    BOOL     InsertBookmarkAsPage(List* pBookmarkList, List* pExchangeList,
                                  BOOL bLink, BOOL bReplace, USHORT nPgPos,
                                  BOOL bNoDialogs, SdDrawDocShell* pBookmarkDocSh,
                                  BOOL bCopy, BOOL bMergeMasterPages);
    BOOL     InsertBookmarkAsObject(List* pBookmarkList, List* pExchangeListL,
                                    BOOL bLink, SdDrawDocShell* pBookmarkDocSh,
                                    Point* pObjPos);
    void     CloseBookmarkDoc();

    SdrObject* GetObj(const String& rObjName) const;

    USHORT  GetPageByName(const String& rPgName) const;
    SdPage* GetSdPage(USHORT nPgNum, PageKind ePgKind) const;
    USHORT  GetSdPageCount(PageKind ePgKind) const;
    void    SetSelected(SdPage* pPage, BOOL bSelect);
    BOOL    MovePages(USHORT nTargetPage);

    SdPage* GetMasterSdPage(USHORT nPgNum, PageKind ePgKind);
    USHORT  GetMasterSdPageCount(PageKind ePgKind) const;

    USHORT  GetMasterPageUserCount(SdrPage* pMaster) const;

    void            SetPresPage( const String& rPresPage ) { aPresPage = rPresPage; }
    const String&   GetPresPage() const { return aPresPage; }

    void SetPresAll(BOOL bNewPresAll);
    BOOL GetPresAll() const          { return bPresAll; }

    void SetPresEndless(BOOL bNewPresEndless);
    BOOL GetPresEndless() const      { return bPresEndless; }

    void SetPresManual(BOOL bNewPresManual);
    BOOL GetPresManual() const       { return bPresManual; }

    void SetPresMouseVisible(BOOL bNewPresMouseVisible);
    BOOL GetPresMouseVisible() const { return bPresMouseVisible; }

    void SetPresMouseAsPen(BOOL bNewPresMouseAsPen);
    BOOL GetPresMouseAsPen() const   { return bPresMouseAsPen; }

    void  SetPresFirstPage (ULONG nNewFirstPage);
    ULONG GetPresFirstPage() const { return nPresFirstPage; }

    void  SetStartPresWithNavigator (BOOL bStart);
    BOOL  GetStartPresWithNavigator() const { return bStartPresWithNavigator; }

    void  SetAnimationAllowed (BOOL bAllowed) { bAnimationAllowed = bAllowed; }
    BOOL  IsAnimationAllowed() const { return bAnimationAllowed; }

    void  SetPresPause( ULONG nSecondsToWait ) { nPresPause = nSecondsToWait; }
    ULONG GetPresPause() const { return nPresPause; }

    void  SetPresShowLogo( BOOL bShowLogo ) { bPresShowLogo = bShowLogo; }
    BOOL  IsPresShowLogo() const { return bPresShowLogo; }

    void  SetPresLockedPages (BOOL bLock);
    BOOL  GetPresLockedPages() const { return bPresLockedPages; }

    void  SetPresAlwaysOnTop (BOOL bOnTop);
    BOOL  GetPresAlwaysOnTop() const { return bPresAlwaysOnTop; }

    void  SetPresFullScreen (BOOL bNewFullScreen);
    BOOL  GetPresFullScreen() const { return bPresFullScreen; }

#ifndef SVX_LIGHT
    void SetOnlineSpell( BOOL bIn );
    BOOL GetOnlineSpell() const { return bOnlineSpell; }
    void StopOnlineSpelling();
    void StartOnlineSpelling(BOOL bForceSpelling=TRUE);
    DECL_LINK(OnlineSpellCallback, SpellCallbackInfo*);

    void InsertObject(SdrObject* pObj, SdPage* pPage);
    void RemoveObject(SdrObject* pObj, SdPage* pPage);

    void SetHideSpell( BOOL bIn );
    BOOL GetHideSpell() const { return bHideSpell; }
#endif

    ULONG GetLinkCount();

    List* GetFrameViewList() const { return pFrameViewList; }
    List* GetCustomShowList(BOOL bCreate = FALSE);

    void  SetCustomShow(BOOL bCustShow) { bCustomShow = bCustShow; }
    BOOL  IsCustomShow() const { return bCustomShow; }

    friend SvStream& operator << (SvStream& rOut, SdDrawDocument& rDoc);
    friend SvStream& operator >> (SvStream& rIn, SdDrawDocument& rDoc);

    virtual void SetChanged(FASTBOOL bFlag = TRUE);
    virtual void NbcSetChanged(FASTBOOL bFlag = TRUE) { bChanged = bFlag; }
    virtual SvStream* GetDocumentStream(FASTBOOL& rbDeleteAfterUse) const;

    void SetTextDefaults() const;

    void CreateLayoutTemplates();
    void RenameLayoutTemplate(const String& rOldLayoutName, const String& rNewName);

    void StopWorkStartupDelay();

    void NewOrLoadCompleted(DocCreationMode eMode);
    BOOL IsNewOrLoadCompleted() const {return bNewOrLoadCompleted; }

    FrameView* GetFrameView(ULONG nPos)
               { return (FrameView*) pFrameViewList->GetObject(nPos); }

    SdAnimationInfo* GetAnimationInfo(SdrObject* pObject) const;

    SdIMapInfo* GetIMapInfo( SdrObject* pObject ) const;
    IMapObject* GetHitIMapObject( SdrObject* pObject, const Point& rWinPoint, const Window& rCmpWnd );

    Graphic     GetGraphicFromOle2Obj( const SdrOle2Obj* pOle2Obj );

    List*       GetDeletedPresObjList();

    CharClass*  GetCharClass() const { return mpCharClass; }
    International* GetInternational() const { return mpInternational; }

#ifndef SVX_LIGHT
    void        RestoreLayerNames();
#endif
    void        MakeUniqueLayerNames();
};



#endif // _DRAWDOC_HXX


