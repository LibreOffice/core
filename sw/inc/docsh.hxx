/*************************************************************************
 *
 *  $RCSfile: docsh.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: os $ $Date: 2001-02-13 14:58:22 $
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
#ifndef _SWDOCSH_HXX
#define _SWDOCSH_HXX

#ifndef _TIMER_HXX //autogen
#include <vcl/timer.hxx>
#endif
#ifndef _SFX_OBJFAC_HXX //autogen
#include <sfx2/docfac.hxx>
#endif
#ifndef _SFX_INTERNO_HXX //autogen
#include <sfx2/interno.hxx>
#endif
#ifndef _SFX_OBJSH_HXX //autogen
#include <sfx2/objsh.hxx>
#endif
#ifndef SW_SWDLL_HXX
#include <swdll.hxx>
#endif
#ifndef _SHELLID_HXX
#include <shellid.hxx>
#endif

class   SwDoc;
class   Sw3Io;
class   SfxDocumentInfoDialog;
class   SfxStyleSheetBasePool;
class   FontList;
class   SwView;
class   SwWrtShell;
class   SwFEShell;
class   Reader;
class   SwReader;
class   SwCrsrShell;
class   SwSrcView;
class   SfxFileDialog;
class   PushButton;
class   FixedText;
class   SwPaM;
class   SwgReaderOption;

class SwDocShell: public SfxObjectShell, public SfxInPlaceObject,
                  public SfxListener
{
    SwDoc*                  pDoc;           // Document
    Sw3Io*                  pIo;            // Reader / Writer
    SfxStyleSheetBasePool*  pBasePool;      // Durchreiche fuer Formate
    FontList*               pFontList;      // aktuelle FontListe

    // Nix geht ohne die WrtShell (historische Gruende)
    // RuekwaertsPointer auf die View (historische Gruende)
    // Dieser gilt solange bis im Activate ein neuer gesetzt wird
    // oder dieser im Dtor der View geloescht wird
    //
    SwView*                 pView;
    SwWrtShell*             pWrtShell;

    Timer                   aFinishedTimer; // Timer fuers ueberpriefen der
                                            // Grafik-Links. Sind alle da,
                                            // dann ist Doc voll. geladen

    SvPersistRef            xOLEChildList;  // fuers RemoveOLEObjects

    // Methoden fuer den Zugriff aufs Doc
    void                    AddLink();
    void                    RemoveLink();

    // Hint abfangen fuer DocInfo
    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    // FileIO
    virtual BOOL            InitNew(SvStorage* pNewStor);
    virtual BOOL            Load(SvStorage* pStor);
    virtual BOOL            LoadFrom(SvStorage* pStor);
    virtual BOOL            ConvertFrom( SfxMedium &rMedium );
    virtual void            HandsOff();
    virtual BOOL            SaveAs(SvStorage * pNewStor );
    virtual BOOL            ConvertTo(SfxMedium &rMedium );
    virtual BOOL            SaveCompleted(SvStorage * pNewStor );
    virtual USHORT          PrepareClose( BOOL bUI = TRUE, BOOL bForBrowsing = FALSE );

    // DocInfo dem Doc melden
    //
    virtual SfxDocumentInfoDialog* CreateDocumentInfoDialog(
                                    Window *pParent, const SfxItemSet &);
    // Template-Btn
    Window*                 AddTemplateBtn(SfxFileDialog* pFileDlg);

    // OLE-Geraffel
    virtual void            Draw( OutputDevice*, const JobSetup&, USHORT);

    // Methoden fuer StyleSheets
    USHORT                  Edit( const String &rName, const String& rParent, USHORT nFamily,
                                    USHORT nMask, BOOL bNew,
                                    BOOL bColumn = FALSE,
                                    SwWrtShell* pActShell = 0,
                                    BOOL bBasic = FALSE );
    USHORT                  Delete(const String &rName, USHORT nFamily);
    USHORT                  ApplyStyles(const String &rName, USHORT nFamily, SwWrtShell* pShell = 0,
                                        USHORT nMode = 0 );
    USHORT                  DoWaterCan( const String &rName, USHORT nFamily);
    USHORT                  UpdateStyle(const String &rName, USHORT nFamily, SwWrtShell* pShell = 0);
    USHORT                  MakeByExample(const String &rName,
                                            USHORT nFamily, USHORT nMask, SwWrtShell* pShell = 0);

    void                    InitDraw();
    void                    SubInitNew();   // fuer InitNew und HtmlSourceModus
    inline void             SetWrtShell(SwWrtShell* pShell)
                                { pWrtShell = pShell; }

    void                    RemoveOLEObjects();

    DECL_STATIC_LINK( SwDocShell, IsLoadFinished, void* );
    DECL_LINK( SelTemplateHdl, PushButton * );

public:

    // aber selbst implementieren
    SFX_DECL_INTERFACE(SW_DOCSHELL);
    SFX_DECL_OBJECTFACTORY_DLL(SwDocShell, SW_DLL());
    TYPEINFO();

    static SfxInterface *_GetInterface() { return _GetInterfaceImpl(); }

    //Das Doc wird fuer SO-Datenaustausch benoetigt!
    SwDocShell(SfxObjectCreateMode eMode = SFX_CREATE_MODE_EMBEDDED);
    SwDocShell( SwDoc *pDoc, SfxObjectCreateMode eMode = SFX_CREATE_MODE_STANDARD );
    ~SwDocShell();

    // OLE 2.0-Benachrichtigung
    DECL_LINK( Ole2ModifiedHdl, void * );

    // OLE-Geraffel
    virtual void      SetVisArea( const Rectangle &rRect );
    virtual Rectangle GetVisArea( USHORT nAspect ) const;
    virtual Printer  *GetDocumentPrinter();
    virtual void      OnDocumentPrinterChanged( Printer * pNewPrinter );
    virtual ULONG     GetMiscStatus() const;

    virtual void            PrepareReload();
    virtual void            SetModified( BOOL = TRUE );

    // Dispatcher
    void                    Execute(SfxRequest &);
    void                    ExecStyleSheet(SfxRequest&);
    void                    ExecDB(SfxRequest&);

    void                    GetState(SfxItemSet &);
    void                    StateAlways(SfxItemSet &);
    void                    StateStyleSheet(SfxItemSet&, SwWrtShell* pSh = 0 );

    // Doc rausreichen aber VORSICHT
    inline SwDoc*           GetDoc() { return pDoc; }
    void                    UpdateFontList();
    void                    UpdateChildWindows();

    // DocumentInfo neu setzen
    BOOL                    SetDocumentInfo(const SfxDocumentInfo& rInfo);

    // globaler IO
    virtual BOOL            Save();
    inline BOOL                 SaveAsChilds( SvStorage *pStor );

    // fuer VorlagenPI
    virtual SfxStyleSheetBasePool*  GetStyleSheetPool();

    // Fuer Organizer
    virtual BOOL Insert(SfxObjectShell &rSource,
                        USHORT  nSourceIdx1,
                        USHORT  nSourceIdx2,
                        USHORT  nSourceIdx3,
                        USHORT& nIdx1,
                        USHORT& nIdx2,
                        USHORT& nIdx3,
                        USHORT& nRemovedIdx);

    virtual BOOL Remove(USHORT nIdx1,
                        USHORT nIdx2 = INDEX_IGNORE,
                        USHORT nIdx3 = INDEX_IGNORE);

    virtual Bitmap      GetStyleFamilyBitmap( SfxStyleFamily eFamily );

    // View setzen fuer Aktionen ueber Shell
    void          SetView(SwView* pVw);
    const SwView *GetView() const { return pView; }

    // Zugriff auf die zur SwView gehoerige SwWrtShell
          SwWrtShell *GetWrtShell()       { return pWrtShell; }
    const SwWrtShell *GetWrtShell() const { return pWrtShell; }

    // fuer die Core - die kennt die DocShell aber keine WrtShell!
          SwFEShell *GetFEShell();
    const SwFEShell *GetFEShell() const
                { return ((SwDocShell*)this)->GetFEShell(); }


    // Fuer Einfuegen Dokument
    Reader* StartConvertFrom(SfxMedium& rMedium, SwReader** ppRdr,
                            SwCrsrShell* pCrsrSh = 0, SwPaM* pPaM = 0);

    // Anforderung der pIo-Struktur fuer den Zugriff auf Substorages
    // und Streams
    Sw3Io* GetIoSystem() { return pIo; }

    virtual long DdeGetData( const String& rItem, SvData& rData );
    virtual long DdeSetData( const String& rItem, const SvData& rData );
    virtual SvPseudoObject* DdeCreateHotLink( const String& rItem );
    virtual void        FillClass( SvGlobalName * pClassName,
                                   ULONG * pClipFormat,
                                   String * pAppName,
                                   String * pLongUserName,
                                   String * pUserName,
                                   long nVersion = SOFFICE_FILEFORMAT_CURRENT ) const;
    virtual void    FillRegInfo( SvEmbeddedRegistryInfo * );

    virtual SvDataMemberObjectRef CreateSnapshot();

    virtual void LoadStyles( SfxObjectShell& rSource );

    // Seitenvorlagedialog anzeigen, ggf. auf Spaltenpage
    void    FormatPage( const String& rPage,
                        BOOL bColumn = FALSE,
                        SwWrtShell*     pActShell = 0 );

    // Timer starten fuers ueberpruefen der Grafik-Links. Sind alle
    // vollstaendig geladen, dann ist das Doc fertig
    void StartLoadFinishedTimer();

    // eine Uebertragung wird abgebrochen (wird aus dem SFX gerufen)
    virtual void CancelTransfers();

    // Doc aus Html-Source neu laden
    void    ReloadFromHtml( const String& rStreamName, SwSrcView* pSrcView );

    // embedded alle lokalen Links (Bereiche/Grafiken)
    BOOL EmbedAllLinks();

    //Activate wait cursor for all windows of this document
    //Optionally all dispatcher could be Locked
    //Usually locking should be done using the class: SwWaitObject!
    void EnterWait( BOOL bLockDispatcher );
    void LeaveWait( BOOL bLockDispatcher );

    void ToggleBrowserMode(BOOL bOn, SwView* pView = 0);

    ULONG LoadStylesFromFile( const String& rURL, SwgReaderOption& rOpt,
                                BOOL bUnoCall );
    void InvalidateModel();
    void ReactivateModel();

#if SUPD>620
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString >  GetEventNames();
#endif
};

inline BOOL SwDocShell::SaveAsChilds( SvStorage *pStor )
{
    return SfxInPlaceObject::SaveAsChilds( pStor );
}
#endif
