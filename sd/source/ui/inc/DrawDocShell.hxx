/*************************************************************************
 *
 *  $RCSfile: DrawDocShell.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2004-08-02 10:09:39 $
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

#ifndef SD_DRAW_DOC_SHELL_HXX
#define SD_DRAW_DOC_SHELL_HXX

#ifndef _SFX_OBJFAC_HXX //autogen
#include <sfx2/docfac.hxx>
#endif
#ifndef _SFX_INTERNO_HXX //autogen
#include <sfx2/interno.hxx>
#endif
#ifndef _SFX_OBJSH_HXX //autogen
#include <sfx2/objsh.hxx>
#endif
#ifndef _SVX_SVXIFACT_HXX //autogen
#include <svx/svxifact.hxx>
#endif
#include <sot/factory.hxx>
#include <so3/factory.hxx>

#ifndef _SD_GLOB_HXX
#include "glob.hxx"
#endif
#ifndef _SDMOD_HXX
#include "sdmod.hxx"
#endif
#ifndef _PRESENTATION_HXX
#include "pres.hxx"
#endif

class SfxStyleSheetBasePool;
class SfxStatusBarManager;
class SdStyleSheetPool;
class FontList;
class SdDrawDocument;
class SvxItemFactory;
class SdPage;
class SfxPrinter;
struct SdrDocumentStreamInfo;
struct SpellCallbackInfo;
class AbstractSvxNameDialog; //CHINA001 class SvxNameDialog;
class SdFormatClipboard;

#ifndef SO2_DECL_SVSTORAGESTREAM_DEFINED
#define SO2_DECL_SVSTORAGESTREAM_DEFINED
SO2_DECL_REF(SvStorageStream)
#endif

namespace sd {

class FuPoor;
class FrameView;
class View;
class ViewShell;

// ------------------
// - DrawDocShell -
// ------------------

class DrawDocShell
    : public SfxObjectShell,
      public SfxInPlaceObject
{
public:
    TYPEINFO();
    SFX_DECL_INTERFACE(SD_IF_SDDRAWDOCSHELL);
    SFX_DECL_OBJECTFACTORY(DrawDocShell);

    DrawDocShell (
        SfxObjectCreateMode eMode = SFX_CREATE_MODE_EMBEDDED,
        BOOL bSdDataObj=FALSE,
        DocumentType=DOCUMENT_TYPE_IMPRESS);
    DrawDocShell (
        SdDrawDocument* pDoc,
        SfxObjectCreateMode eMode = SFX_CREATE_MODE_EMBEDDED,
        BOOL bSdDataObj=FALSE,
        DocumentType=DOCUMENT_TYPE_IMPRESS);
    virtual ~DrawDocShell();

    void                    UpdateRefDevice();
    virtual void            Activate( BOOL bMDI );
    virtual void            Deactivate( BOOL bMDI );
    virtual BOOL            InitNew( SvStorage * );
    virtual BOOL            Load( SvStorage * );
    virtual BOOL            LoadFrom(SvStorage* pStor);
    virtual BOOL            ConvertFrom( SfxMedium &rMedium );
    virtual void            HandsOff();
    virtual BOOL            Save();
    virtual BOOL            SaveAs( SvStorage * pNewStor );
    virtual BOOL            SaveAsOwnFormat( SfxMedium& rMedium );
    virtual BOOL            ConvertTo( SfxMedium &rMedium );
    virtual BOOL            SaveCompleted( SvStorage * pNewStor );
    virtual void            UIActivate( BOOL bActive );
    virtual void            SetVisArea(const Rectangle& rRect);
    virtual Rectangle       GetVisArea(USHORT nAspect) const;
    virtual void            Draw(OutputDevice*, const JobSetup& rSetup, USHORT nAspect = ASPECT_CONTENT);
    virtual SfxUndoManager* GetUndoManager();
    virtual Printer*        GetDocumentPrinter();
    virtual void            OnDocumentPrinterChanged(Printer* pNewPrinter);
    virtual SfxStyleSheetBasePool* GetStyleSheetPool();
    virtual void            SetOrganizerSearchMask(SfxStyleSheetBasePool* pBasePool) const;
    virtual Size            GetFirstPageSize();
    virtual void            FillClass(SvGlobalName* pClassName, ULONG*  pFormat, String* pAppName, String* pFullTypeName, String* pShortTypeName, long nFileFormat = SOFFICE_FILEFORMAT_CURRENT) const;
    virtual void            SetModified( BOOL = TRUE );



    sd::ViewShell* GetViewShell() { return pViewShell; }
    ::sd::FrameView* GetFrameView();
    ::Window* GetWindow() const;
    ::sd::FuPoor* GetActualFunction() const { return pFuActual; }

    SdDrawDocument*         GetDoc();
    DocumentType            GetDocumentType() const { return eDocType; }

    SfxPrinter*             GetPrinter(BOOL bCreate);
    void                    SetPrinter(SfxPrinter *pNewPrinter);

    BOOL                    IsUIActive() { return bUIActive; }
    BOOL                    IsInDestruction() const { return bInDestruction; }

    void                    CancelSearching();

    void                    Execute( SfxRequest& rReq );
    void                    GetState(SfxItemSet&);

    void                    Connect(sd::ViewShell* pViewSh);
    void                    Disconnect(sd::ViewShell* pViewSh);
    void                    UpdateTablePointers();

    BOOL                    GotoBookmark(const String& rBookmark);

    Bitmap                  GetPagePreviewBitmap(SdPage* pPage, USHORT nMaxEdgePixel);

    /** checks, if the given name is a valid new name for a slide

        <p>If the name is invalid, an <type>SvxNameDialog</type> pops up that
        queries again for a new name until it is ok or the user chose
        Cancel.</p>

        @param pWin is necessary to pass to the <type>SvxNameDialog</type> in
                    case an invalid name was entered.
        @param rName the new name that is to be set for a slide.  This string
                     may be set to an empty string (see below).

        @return TRUE, if the new name is unique.  Note that if the user entered
                a default name of a not-yet-existing slide (e.g. 'Slide 17'),
                TRUE is returned, but rName is set to an empty string.
     */
    BOOL                    CheckPageName(::Window* pWin, String& rName );

    void                    SetSlotFilter(BOOL bEnable = FALSE, USHORT nCount = 0, const USHORT* pSIDs = NULL) { bFilterEnable = bEnable; nFilterCount = nCount; pFilterSIDs = pSIDs; }
    void                    ApplySlotFilter() const;

    UINT16                  GetStyleFamily() const { return nStyleFamily; }
    void                    SetStyleFamily( UINT16 nSF ) { nStyleFamily = nSF; }

    BOOL                    HasSpecialProgress() const { return ( mpSpecialProgress != NULL && mpSpecialProgressHdl != NULL ); }
    void                    ReleaseSpecialProgress() { mpSpecialProgress = NULL; mpSpecialProgressHdl = NULL; }
    void                    SetSpecialProgress( SfxProgress* pProgress, Link* pLink ) { mpSpecialProgress = pProgress; mpSpecialProgressHdl = pLink; }
    SfxProgress*            GetSpecialProgress() { return( HasSpecialProgress() ? mpSpecialProgress : NULL ); }

    sal_Bool                IsNewDocument() const;

    /** checks, if the given name is a valid new name for a slide

        <p>This method does not pop up any dialog (like CheckPageName).</p>

        @param rInOutPageName the new name for a slide that is to be renamed.
                    This string will be set to an empty string if
                    bResetStringIfStandardName is true and the name is of the
                    form of any, possibly not-yet existing, standard slide
                    (e.g. 'Slide 17')

        @param bResetStringIfStandardName if true allows setting rInOutPageName
                    to an empty string, which returns true and implies that the
                    slide will later on get a new standard name (with a free
                    slide number).

        @return true, if the new name is unique.  If bResetStringIfStandardName
                    is true, the return value is also true, if the slide name is
                    a standard name (see above)
     */
    bool                    IsNewPageNameValid( String & rInOutPageName, bool bResetStringIfStandardName = false );


    /** Return the reference device for the current document.  When the
        inherited implementation returns a device then this is passed to the
        caller.  Otherwise the returned value depends on the printer
        independent layout mode and will usually be either a printer or a
        virtual device used for screen rendering.
        @return
            Returns NULL when the current document has no reference device.
    */
    virtual OutputDevice* GetDocumentRefDev (void);

    DECL_LINK( RenameSlideHdl, AbstractSvxNameDialog* );

#ifndef SVX_LIGHT
                            // #91457# ExecuteSpellPopup now handled by DrawDocShell
                            DECL_LINK( OnlineSpellCallback, SpellCallbackInfo* );
#endif

public:
    SdFormatClipboard*      pFormatClipboard;

protected:

    SfxProgress*            pProgress;
    SdDrawDocument*         pDoc;
    SfxUndoManager*         pUndoManager;
    SfxPrinter*             pPrinter;
    ::sd::ViewShell* pViewShell;
    FontList*               pFontList;
    ::sd::FuPoor* pFuActual;
    DocumentType            eDocType;
    UINT16                  nStyleFamily;
    const USHORT*           pFilterSIDs;
    USHORT                  nFilterCount;
    BOOL                    bFilterEnable;
    BOOL                    bUIActive;
    BOOL                    bSdDataObj;
    BOOL                    bInDestruction;
    BOOL                    bOwnPrinter;
    BOOL                    mbNewDocument;

    static SfxProgress*     mpSpecialProgress;
    static Link*            mpSpecialProgressHdl;

    void                    Construct();
    virtual void            InPlaceActivate( BOOL bActive );
};

#ifndef SV_DECL_DRAW_DOC_SHELL_DEFINED
#define SV_DECL_DRAW_DOC_SHELL_DEFINED
SV_DECL_REF(DrawDocShell)
#endif

SV_IMPL_REF (DrawDocShell)

} // end of namespace sd

#endif
