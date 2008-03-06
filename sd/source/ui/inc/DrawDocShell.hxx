/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DrawDocShell.hxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 18:58:00 $
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

#ifndef SD_DRAW_DOC_SHELL_HXX
#define SD_DRAW_DOC_SHELL_HXX

#ifndef _SFX_OBJFAC_HXX //autogen
#include <sfx2/docfac.hxx>
#endif
#ifndef _SFX_OBJSH_HXX //autogen
#include <sfx2/objsh.hxx>
#endif

#include <vcl/jobset.hxx>

#ifndef _SD_GLOB_HXX
#include "glob.hxx"
#endif
#ifndef _SDMOD_HXX
#include "sdmod.hxx"
#endif
#ifndef _PRESENTATION_HXX
#include "pres.hxx"
#endif
#ifndef INCLUDED_SDDLLAPI_H
#include "sddllapi.h"
#endif
#ifndef SD_FU_POOR_HXX
#include "fupoor.hxx"
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
class AbstractSvxNameDialog;
class SdFormatClipboard;

namespace sd {

class FrameView;
class View;
class ViewShell;

// ------------------
// - DrawDocShell -
// ------------------

class SD_DLLPUBLIC DrawDocShell : public SfxObjectShell
{
public:
    TYPEINFO();
    SFX_DECL_INTERFACE(SD_IF_SDDRAWDOCSHELL)
    SFX_DECL_OBJECTFACTORY();

    DrawDocShell (
        SfxObjectCreateMode eMode = SFX_CREATE_MODE_EMBEDDED,
        BOOL bSdDataObj=FALSE,
        DocumentType=DOCUMENT_TYPE_IMPRESS,
        BOOL bScriptSupport=TRUE);

    DrawDocShell (
        SdDrawDocument* pDoc,
        SfxObjectCreateMode eMode = SFX_CREATE_MODE_EMBEDDED,
        BOOL bSdDataObj=FALSE,
        DocumentType=DOCUMENT_TYPE_IMPRESS);
    virtual ~DrawDocShell();

    void                    UpdateRefDevice();
    virtual void            Activate( BOOL bMDI );
    virtual void            Deactivate( BOOL bMDI );
    virtual BOOL            InitNew( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage );
    virtual BOOL            ConvertFrom( SfxMedium &rMedium );
    virtual BOOL            Save();
    virtual BOOL            SaveAsOwnFormat( SfxMedium& rMedium );
    virtual BOOL            ConvertTo( SfxMedium &rMedium );
    virtual BOOL            SaveCompleted( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage );

    virtual sal_Bool        Load( SfxMedium &rMedium  );
    virtual sal_Bool        LoadFrom( SfxMedium& rMedium );
    virtual sal_Bool        SaveAs( SfxMedium &rMedium  );

    virtual Rectangle       GetVisArea(USHORT nAspect) const;
    virtual void            Draw(OutputDevice*, const JobSetup& rSetup, USHORT nAspect = ASPECT_CONTENT);
    virtual SfxUndoManager* GetUndoManager();
    virtual Printer*        GetDocumentPrinter();
    virtual void            OnDocumentPrinterChanged(Printer* pNewPrinter);
    virtual SfxStyleSheetBasePool* GetStyleSheetPool();
    virtual void            SetOrganizerSearchMask(SfxStyleSheetBasePool* pBasePool) const;
    virtual Size            GetFirstPageSize();
    virtual void            FillClass(SvGlobalName* pClassName, sal_uInt32*  pFormat, String* pAppName, String* pFullTypeName, String* pShortTypeName, sal_Int32 nFileFormat ) const;
    virtual void            SetModified( BOOL = TRUE );

    using SotObject::GetInterface;
    using SfxObjectShell::GetVisArea;
    using SfxShell::GetViewShell;

    sd::ViewShell* GetViewShell() { return mpViewShell; }
    ::sd::FrameView* GetFrameView();
    ::Window* GetWindow() const;
    ::sd::FunctionReference GetDocShellFunction() const { return mxDocShellFunction; }
    void SetDocShellFunction( const ::sd::FunctionReference& xFunction );

    SdDrawDocument*         GetDoc();
    DocumentType            GetDocumentType() const { return meDocType; }

    SfxPrinter*             GetPrinter(BOOL bCreate);
    void                    SetPrinter(SfxPrinter *pNewPrinter);
    void                    UpdateFontList();

    BOOL                    IsInDestruction() const { return mbInDestruction; }

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

    void                    SetSlotFilter(BOOL bEnable = FALSE, USHORT nCount = 0, const USHORT* pSIDs = NULL) { mbFilterEnable = bEnable; mnFilterCount = nCount; mpFilterSIDs = pSIDs; }
    void                    ApplySlotFilter() const;

    UINT16                  GetStyleFamily() const { return mnStyleFamily; }
    void                    SetStyleFamily( UINT16 nSF ) { mnStyleFamily = nSF; }

    BOOL                    HasSpecialProgress() const { return ( mpSpecialProgress != NULL && mpSpecialProgressHdl != NULL ); }
    void                    ReleaseSpecialProgress() { mpSpecialProgress = NULL; mpSpecialProgressHdl = NULL; }
    void                    SetSpecialProgress( SfxProgress* _pProgress, Link* pLink ) { mpSpecialProgress = _pProgress; mpSpecialProgressHdl = pLink; }
    SfxProgress*            GetSpecialProgress() { return( HasSpecialProgress() ? mpSpecialProgress : NULL ); }

    sal_Bool                IsNewDocument() const;

    /** executes the SID_OPENDOC slot to let the framework open a document
        with the given URL and this document as a referer */
    void                    OpenBookmark( const String& rBookmarkURL );

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

                            // #91457# ExecuteSpellPopup now handled by DrawDocShell
                            DECL_LINK( OnlineSpellCallback, SpellCallbackInfo* );

    void                    ClearUndoBuffer();

public:
    SdFormatClipboard*      mpFormatClipboard;

protected:

    SdDrawDocument*         mpDoc;
    SfxUndoManager*         mpUndoManager;
    SfxPrinter*             mpPrinter;
    ::sd::ViewShell*        mpViewShell;
    FontList*               mpFontList;
    ::sd::FunctionReference mxDocShellFunction;
    DocumentType            meDocType;
    UINT16                  mnStyleFamily;
    const USHORT*           mpFilterSIDs;
    USHORT                  mnFilterCount;
    BOOL                    mbFilterEnable;
    BOOL                    mbSdDataObj;
    BOOL                    mbInDestruction;
    BOOL                    mbOwnPrinter;
    BOOL                    mbNewDocument;

    static SfxProgress*     mpSpecialProgress;
    static Link*            mpSpecialProgressHdl;

    bool                    mbOwnDocument;          // if true, we own mpDoc and will delete it in our d'tor
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
