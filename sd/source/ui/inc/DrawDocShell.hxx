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

#ifndef INCLUDED_SD_SOURCE_UI_INC_DRAWDOCSHELL_HXX
#define INCLUDED_SD_SOURCE_UI_INC_DRAWDOCSHELL_HXX

#include <o3tl/span.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/objsh.hxx>

#include <glob.hxx>
#include <pres.hxx>
#include <sddllapi.h>
#include "fupoor.hxx"

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
class DrawViewShell;

// DrawDocShell
class SD_DLLPUBLIC DrawDocShell : public SfxObjectShell
{
public:
    SFX_DECL_INTERFACE(SD_IF_SDDRAWDOCSHELL)
    SFX_DECL_OBJECTFACTORY();

private:
    /// SfxInterface initializer.
    static void InitInterface_Impl();

public:
    DrawDocShell (
        SfxObjectCreateMode eMode,
        bool bSdDataObj,
        DocumentType);

    DrawDocShell (
        SfxModelFlags nModelCreationFlags,
        bool bSdDataObj,
        DocumentType);

    DrawDocShell (
        SdDrawDocument* pDoc,
        SfxObjectCreateMode eMode,
        bool bSdDataObj,
        DocumentType);
    virtual ~DrawDocShell() override;

    void                    UpdateRefDevice();
    virtual void            Activate( bool bMDI ) override;
    virtual void            Deactivate( bool bMDI ) override;
    virtual bool            InitNew( const css::uno::Reference< css::embed::XStorage >& xStorage ) override;
    virtual bool            ImportFrom(SfxMedium &rMedium,
            css::uno::Reference<css::text::XTextRange> const& xInsertPosition)
        override;
    virtual bool            ConvertFrom( SfxMedium &rMedium ) override;
    virtual bool            Save() override;
    virtual bool            SaveAsOwnFormat( SfxMedium& rMedium ) override;
    virtual bool            ConvertTo( SfxMedium &rMedium ) override;
    virtual bool            SaveCompleted( const css::uno::Reference< css::embed::XStorage >& xStorage ) override;

    virtual bool            Load( SfxMedium &rMedium  ) override;
    virtual bool            LoadFrom( SfxMedium& rMedium ) override;
    virtual bool            SaveAs( SfxMedium &rMedium  ) override;

    virtual ::tools::Rectangle       GetVisArea(sal_uInt16 nAspect) const override;
    virtual void            Draw(OutputDevice*, const JobSetup& rSetup, sal_uInt16 nAspect) override;
    virtual SfxUndoManager* GetUndoManager() override;
    virtual Printer*        GetDocumentPrinter() override;
    virtual void            OnDocumentPrinterChanged(Printer* pNewPrinter) override;
    virtual SfxStyleSheetBasePool* GetStyleSheetPool() override;
    virtual void            FillClass(SvGlobalName* pClassName, SotClipboardFormatId* pFormat, OUString* pAppName, OUString* pFullTypeName, OUString* pShortTypeName, sal_Int32 nFileFormat, bool bTemplate = false ) const override;
    virtual void            SetModified( bool = true ) override;
    virtual VclPtr<SfxDocumentInfoDialog> CreateDocumentInfoDialog( const SfxItemSet &rSet ) override;

    using SfxObjectShell::GetVisArea;
    using SfxShell::GetViewShell;

    sd::ViewShell* GetViewShell() { return mpViewShell; }
    ::sd::FrameView* GetFrameView();
    const rtl::Reference<FuPoor>& GetDocShellFunction() const { return mxDocShellFunction; }
    void SetDocShellFunction( const rtl::Reference<FuPoor>& xFunction );

    SdDrawDocument*         GetDoc() { return mpDoc;}
    DocumentType            GetDocumentType() const { return meDocType; }

    SfxPrinter*             GetPrinter(bool bCreate);
    void                    SetPrinter(SfxPrinter *pNewPrinter);
    void                    UpdateFontList();

    bool                    IsInDestruction() const { return mbInDestruction; }

    void                    CancelSearching();

    void                    Execute( SfxRequest& rReq );
    void                    GetState(SfxItemSet&);

    void                    Connect(sd::ViewShell* pViewSh);
    void                    Disconnect(sd::ViewShell const * pViewSh);
    void                    UpdateTablePointers();

    void                    GotoBookmark(const OUString& rBookmark);

    BitmapEx                GetPagePreviewBitmap(SdPage* pPage);

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
    bool                    CheckPageName(weld::Window* pWin, OUString& rName );

    void                    SetSlotFilter(bool bEnable = false, o3tl::span<sal_uInt16 const> pSIDs = o3tl::span<sal_uInt16 const>()) { mbFilterEnable = bEnable; mpFilterSIDs = pSIDs; }
    void                    ApplySlotFilter() const;

    SfxStyleFamily          GetStyleFamily() const { return mnStyleFamily; }
    void                    SetStyleFamily( SfxStyleFamily nSF ) { mnStyleFamily = nSF; }

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

    /** checks, if the given name is a *unique* name for an *existing* slide

        @param rPageName the name of an existing slide

        @return true, if the name is unique and the slide exists
    */
    bool                    IsPageNameUnique(const OUString& rPagName) const;

    /** Return the reference device for the current document.  When the
        inherited implementation returns a device then this is passed to the
        caller.  Otherwise the returned value depends on the printer
        independent layout mode and will usually be either a printer or a
        virtual device used for screen rendering.
        @return
            Returns NULL when the current document has no reference device.
    */
    virtual OutputDevice* GetDocumentRefDev() override;

    DECL_LINK( RenameSlideHdl, AbstractSvxNameDialog&, bool );

    // ExecuteSpellPopup now handled by DrawDocShell
    DECL_LINK( OnlineSpellCallback, SpellCallbackInfo&, void );

    void                    ClearUndoBuffer();

protected:

    SdDrawDocument*         mpDoc;
    std::unique_ptr<SfxUndoManager> mpUndoManager;
    VclPtr<SfxPrinter>      mpPrinter;
    ::sd::ViewShell*        mpViewShell;
    std::unique_ptr<FontList> mpFontList;
    rtl::Reference<FuPoor> mxDocShellFunction;
    DocumentType const      meDocType;
    SfxStyleFamily          mnStyleFamily;
    o3tl::span<sal_uInt16 const>
                            mpFilterSIDs;
    bool                    mbFilterEnable;
    bool const              mbSdDataObj;
    bool                    mbInDestruction;
    bool                    mbOwnPrinter;

    bool                    mbOwnDocument;          // if true, we own mpDoc and will delete it in our d'tor
    void                    Construct(bool bClipboard);
private:
    static void setEditMode(DrawViewShell* pDrawViewShell, bool isMasterPage);
};

#ifndef SV_DECL_DRAW_DOC_SHELL_DEFINED
#define SV_DECL_DRAW_DOC_SHELL_DEFINED
typedef ::tools::SvRef<DrawDocShell> DrawDocShellRef;
#endif

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
