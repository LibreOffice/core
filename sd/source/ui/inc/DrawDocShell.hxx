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

#ifndef SD_DRAW_DOC_SHELL_HXX
#define SD_DRAW_DOC_SHELL_HXX

#include <sfx2/docfac.hxx>
#include <sfx2/objsh.hxx>

#include <vcl/jobset.hxx>
#include "glob.hxx"
#include "sdmod.hxx"
#include "pres.hxx"
#include "sddllapi.h"
#include "fupoor.hxx"

class SfxStyleSheetBasePool;
class FontList;
class SdDrawDocument;
class SdPage;
class SfxPrinter;
struct SpellCallbackInfo;
class AbstractSvxNameDialog;
class SfxUndoManager;

namespace sd {

class FrameView;
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
        sal_Bool bSdDataObj=sal_False,
        DocumentType=DOCUMENT_TYPE_IMPRESS);

    DrawDocShell (
        const sal_uInt64 nModelCreationFlags,
        sal_Bool bSdDataObj=sal_False,
        DocumentType=DOCUMENT_TYPE_IMPRESS);

    DrawDocShell (
        SdDrawDocument* pDoc,
        SfxObjectCreateMode eMode = SFX_CREATE_MODE_EMBEDDED,
        sal_Bool bSdDataObj=sal_False,
        DocumentType=DOCUMENT_TYPE_IMPRESS);
    virtual ~DrawDocShell();

    void                    UpdateRefDevice();
    virtual void            Activate( sal_Bool bMDI );
    virtual void            Deactivate( sal_Bool bMDI );
    virtual sal_Bool            InitNew( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage );
    virtual sal_Bool        ImportFrom( SfxMedium &rMedium, bool bInsert=false );
    virtual sal_Bool            ConvertFrom( SfxMedium &rMedium );
    virtual sal_Bool            Save();
    virtual sal_Bool            SaveAsOwnFormat( SfxMedium& rMedium );
    virtual sal_Bool            ConvertTo( SfxMedium &rMedium );
    virtual sal_Bool            SaveCompleted( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage );

    virtual sal_Bool        Load( SfxMedium &rMedium  );
    virtual sal_Bool        LoadFrom( SfxMedium& rMedium );
    virtual sal_Bool        SaveAs( SfxMedium &rMedium  );

    virtual Rectangle       GetVisArea(sal_uInt16 nAspect) const;
    virtual void            Draw(OutputDevice*, const JobSetup& rSetup, sal_uInt16 nAspect = ASPECT_CONTENT);
    virtual ::svl::IUndoManager*
                            GetUndoManager();
    virtual Printer*        GetDocumentPrinter();
    virtual void            OnDocumentPrinterChanged(Printer* pNewPrinter);
    virtual SfxStyleSheetBasePool* GetStyleSheetPool();
    virtual Size            GetFirstPageSize();
    virtual void            FillClass(SvGlobalName* pClassName, sal_uInt32*  pFormat, OUString* pAppName, OUString* pFullTypeName, OUString* pShortTypeName, sal_Int32 nFileFormat, sal_Bool bTemplate = sal_False ) const;
    virtual void            SetModified( sal_Bool = sal_True );
    virtual SfxDocumentInfoDialog*  CreateDocumentInfoDialog( ::Window *pParent,
                                                              const SfxItemSet &rSet );

    using SotObject::GetInterface;
    using SfxObjectShell::GetVisArea;
    using SfxShell::GetViewShell;

    sd::ViewShell* GetViewShell() { return mpViewShell; }
    ::sd::FrameView* GetFrameView();
    ::sd::FunctionReference GetDocShellFunction() const { return mxDocShellFunction; }
    void SetDocShellFunction( const ::sd::FunctionReference& xFunction );

    SdDrawDocument*         GetDoc();
    DocumentType            GetDocumentType() const { return meDocType; }

    SfxPrinter*             GetPrinter(sal_Bool bCreate);
    void                    SetPrinter(SfxPrinter *pNewPrinter);
    void                    UpdateFontList();

    sal_Bool                    IsInDestruction() const { return mbInDestruction; }

    void                    CancelSearching();

    void                    Execute( SfxRequest& rReq );
    void                    GetState(SfxItemSet&);

    void                    Connect(sd::ViewShell* pViewSh);
    void                    Disconnect(sd::ViewShell* pViewSh);
    void                    UpdateTablePointers();

    sal_Bool                    GotoBookmark(const OUString& rBookmark);

    Bitmap                  GetPagePreviewBitmap(SdPage* pPage, sal_uInt16 nMaxEdgePixel);

    /** checks, if the given name is a valid new name for a slide

        <p>If the name is invalid, an <type>SvxNameDialog</type> pops up that
        queries again for a new name until it is ok or the user chose
        Cancel.</p>

        @param pWin is necessary to pass to the <type>SvxNameDialog</type> in
                    case an invalid name was entered.
        @param rName the new name that is to be set for a slide.  This string
                     may be set to an empty string (see below).

        @return sal_True, if the new name is unique.  Note that if the user entered
                a default name of a not-yet-existing slide (e.g. 'Slide 17'),
                sal_True is returned, but rName is set to an empty string.
     */
    sal_Bool                    CheckPageName(::Window* pWin, OUString& rName );

    void                    SetSlotFilter(sal_Bool bEnable = sal_False, sal_uInt16 nCount = 0, const sal_uInt16* pSIDs = NULL) { mbFilterEnable = bEnable; mnFilterCount = nCount; mpFilterSIDs = pSIDs; }
    void                    ApplySlotFilter() const;

    sal_uInt16                  GetStyleFamily() const { return mnStyleFamily; }
    void                    SetStyleFamily( sal_uInt16 nSF ) { mnStyleFamily = nSF; }

    /** executes the SID_OPENDOC slot to let the framework open a document
        with the given URL and this document as a referer */
    void                    OpenBookmark( const OUString& rBookmarkURL );

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
    bool                    IsNewPageNameValid( OUString & rInOutPageName, bool bResetStringIfStandardName = false );


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

                            // ExecuteSpellPopup now handled by DrawDocShell
                            DECL_LINK( OnlineSpellCallback, SpellCallbackInfo* );

    void                    ClearUndoBuffer();

protected:

    SdDrawDocument*         mpDoc;
    SfxUndoManager*         mpUndoManager;
    SfxPrinter*             mpPrinter;
    ::sd::ViewShell*        mpViewShell;
    FontList*               mpFontList;
    ::sd::FunctionReference mxDocShellFunction;
    DocumentType            meDocType;
    sal_uInt16                  mnStyleFamily;
    const sal_uInt16*           mpFilterSIDs;
    sal_uInt16                  mnFilterCount;
    sal_Bool                    mbFilterEnable;
    sal_Bool                    mbSdDataObj;
    sal_Bool                    mbInDestruction;
    sal_Bool                    mbOwnPrinter;
    sal_Bool                    mbNewDocument;

    bool                    mbOwnDocument;          // if true, we own mpDoc and will delete it in our d'tor
    void                    Construct(bool bClipboard);
    virtual void            InPlaceActivate( sal_Bool bActive );
};

#ifndef SV_DECL_DRAW_DOC_SHELL_DEFINED
#define SV_DECL_DRAW_DOC_SHELL_DEFINED
SV_DECL_REF(DrawDocShell)
#endif

SV_IMPL_REF (DrawDocShell)

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
