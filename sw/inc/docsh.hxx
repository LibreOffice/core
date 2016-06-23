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
#ifndef INCLUDED_SW_INC_DOCSH_HXX
#define INCLUDED_SW_INC_DOCSH_HXX

#include <rtl/ref.hxx>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/uno/Sequence.h>
#include <sfx2/docfac.hxx>
#include <sfx2/objsh.hxx>
#include "swdllapi.h"
#include <shellid.hxx>

#include <svl/lstner.hxx>
#include <svtools/embedhlp.hxx>
#include <LibreOfficeKit/LibreOfficeKitTypes.h>

#include <sfx2/StyleManager.hxx>

class SwDoc;
class SfxDocumentInfoDialog;
class SfxStyleSheetBasePool;
class SfxInPlaceClient;
class FontList;
class SwEditShell;
class SwView;
class SwWrtShell;
class SwFEShell;
class Reader;
class SwReader;
class SwCursorShell;
class SwSrcView;
class SwPaM;
class SwgReaderOption;
class SwOLEObj;
class IDocumentDeviceAccess;
class IDocumentSettingAccess;
class IDocumentChartDataProviderAccess;
class SwDocShell;
class SwDrawModel;

// initialize DrawModel (in form of a SwDrawModel) and DocShell (in form of a SwDocShell)
// as needed, one or both parameters may be zero
void SAL_DLLPRIVATE InitDrawModelAndDocShell(SwDocShell* pSwDocShell, SwDrawModel* pSwDrawModel);

class SW_DLLPUBLIC SwDocShell
    : public SfxObjectShell
    , public SfxListener
{
    SwDoc*      m_pDoc;               ///< Document.
    rtl::Reference< SfxStyleSheetBasePool > m_xBasePool; ///< Passing through for formats.
    FontList*   m_pFontList;          ///< Current Fontlist.
    bool        m_IsInUpdateFontList; ///< prevent nested calls of UpdateFontList

    std::unique_ptr<sfx2::StyleManager> m_pStyleManager;

    /** For "historical reasons" nothing can be done without the WrtShell.
     Back-pointer on View (again "for historical reasons").
     Back-pointer is valid until in Activate a new one is set
     or until it is deleted in the View's Dtor. */

    SwView*     m_pView;
    SwWrtShell* m_pWrtShell;

    comphelper::EmbeddedObjectContainer* m_pOLEChildList;
    sal_Int16   m_nUpdateDocMode;   ///< contains the css::document::UpdateDocMode
    bool        m_IsATemplate;      ///< prevent nested calls of UpdateFontList

    bool m_IsRemovedInvisibleContent;
        ///< whether SID_MAIL_PREPAREEXPORT removed content that
        ///< SID_MAIL_EXPORT_FINISHED needs to restore

    /// Methods for access to doc.
    SAL_DLLPRIVATE void                  AddLink();
    SAL_DLLPRIVATE void                  RemoveLink();

    /// Catch hint for DocInfo.
    SAL_DLLPRIVATE virtual void          Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    /// FileIO
    SAL_DLLPRIVATE virtual bool InitNew( const css::uno::Reference< css::embed::XStorage >& xStorage ) override;
    SAL_DLLPRIVATE virtual bool Load( SfxMedium& rMedium ) override;
    SAL_DLLPRIVATE virtual bool LoadFrom( SfxMedium& rMedium ) override;
    SAL_DLLPRIVATE virtual bool ConvertFrom( SfxMedium &rMedium ) override;
    SAL_DLLPRIVATE virtual bool ConvertTo( SfxMedium &rMedium ) override;
    SAL_DLLPRIVATE virtual bool SaveAs( SfxMedium& rMedium ) override;
    SAL_DLLPRIVATE virtual bool SaveCompleted( const css::uno::Reference< css::embed::XStorage >& xStorage ) override;

    SAL_DLLPRIVATE virtual bool     PrepareClose( bool bUI = true ) override;

    SAL_DLLPRIVATE virtual bool     InsertGeneratedStream(SfxMedium& rMedium,
            css::uno::Reference<css::text::XTextRange> const& xInsertPosition)
        override;

    /// Make DocInfo known to the Doc.
    SAL_DLLPRIVATE virtual VclPtr<SfxDocumentInfoDialog> CreateDocumentInfoDialog(
                                    vcl::Window *pParent, const SfxItemSet &) override;
    /// OLE-stuff
    SAL_DLLPRIVATE virtual void          Draw( OutputDevice*, const JobSetup&, sal_uInt16) override;

    /// Methods for StyleSheets

    /// @param nSlot
    /// Only used for nFamily == SFX_STYLE_FAMILY_PAGE. Identifies optional Slot by which the edit is triggered.
    /// Used to activate certain dialog pane
    SAL_DLLPRIVATE sal_uInt16 Edit(
        const OUString &rName,
        const OUString& rParent,
        const sal_uInt16 nFamily,
        sal_uInt16 nMask,
        const bool bNew,
        const OString& sPageId = OString(),
        SwWrtShell* pActShell = nullptr,
        const bool bBasic = false );

    SAL_DLLPRIVATE bool                  Delete(const OUString &rName, sal_uInt16 nFamily);
    SAL_DLLPRIVATE bool                  Hide(const OUString &rName, sal_uInt16 nFamily, bool bHidden);
    SAL_DLLPRIVATE sal_uInt16            ApplyStyles(const OUString &rName,
        const sal_uInt16 nFamily,
        SwWrtShell* pShell = nullptr,
        sal_uInt16 nMode = 0);
    SAL_DLLPRIVATE sal_uInt16            DoWaterCan( const OUString &rName, sal_uInt16 nFamily);
    SAL_DLLPRIVATE sal_uInt16            UpdateStyle(const OUString &rName, sal_uInt16 nFamily, SwWrtShell* pShell = nullptr);
    SAL_DLLPRIVATE sal_uInt16            MakeByExample(const OUString &rName,
                                        sal_uInt16 nFamily, sal_uInt16 nMask, SwWrtShell* pShell = nullptr);

    SAL_DLLPRIVATE void                  SubInitNew();   ///< for InitNew and HtmlSourceMode.

    SAL_DLLPRIVATE void                  RemoveOLEObjects();
    SAL_DLLPRIVATE void                  CalcLayoutForOLEObjects();

    SAL_DLLPRIVATE void                  Init_Impl();

    using SfxObjectShell::GetVisArea;

protected:
    /// override to update text fields
    virtual void                DoFlushDocInfo() override;

public:
    /// but we implement this ourselves.
    SFX_DECL_INTERFACE(SW_DOCSHELL)
    SFX_DECL_OBJECTFACTORY()

private:
    /// SfxInterface initializer.
    static void InitInterface_Impl();

public:
    static OUString GetEventName( sal_Int32 nId );

    /// Doc is required for SO data exchange!
    SwDocShell( SfxObjectCreateMode eMode = SfxObjectCreateMode::EMBEDDED );
    SwDocShell( SfxModelFlags i_nSfxCreationFlags );
    SwDocShell( SwDoc *pDoc, SfxObjectCreateMode eMode = SfxObjectCreateMode::STANDARD );
    virtual ~SwDocShell();

    /// OLE 2.0-notification.
    DECL_LINK_TYPED( Ole2ModifiedHdl, bool, void );

    /// OLE-stuff.
    virtual void      SetVisArea( const Rectangle &rRect ) override;
    virtual Rectangle GetVisArea( sal_uInt16 nAspect ) const override;
    virtual Printer  *GetDocumentPrinter() override;
    virtual OutputDevice* GetDocumentRefDev() override;
    virtual void      OnDocumentPrinterChanged( Printer * pNewPrinter ) override;
    virtual sal_uLong     GetMiscStatus() const override;

    virtual void            PrepareReload() override;
    virtual void            SetModified( bool = true ) override;

    /// Dispatcher
    void                    Execute(SfxRequest &);
    void                    ExecStyleSheet(SfxRequest&);

    void                    GetState(SfxItemSet &);
    void                    StateStyleSheet(SfxItemSet&, SwWrtShell* pSh = nullptr );

    /// returns Doc. But be careful!
    inline SwDoc*                   GetDoc() { return m_pDoc; }
    inline const SwDoc*             GetDoc() const { return m_pDoc; }
    IDocumentDeviceAccess&          getIDocumentDeviceAccess();
    IDocumentChartDataProviderAccess& getIDocumentChartDataProviderAccess();

    void                    UpdateFontList();
    void                    UpdateChildWindows();

    /// global IO.
    virtual bool            Save() override;

    /// For Style PI.
    virtual SfxStyleSheetBasePool*  GetStyleSheetPool() override;
    virtual sfx2::StyleManager* GetStyleManager() override;

    /// Set View for actions via Shell.
    void          SetView(SwView* pVw);
    const SwView *GetView() const { return m_pView; }
    SwView       *GetView()       { return m_pView; }

    /// Access to the SwWrtShell belonging to SwView.
          SwWrtShell *GetWrtShell()       { return m_pWrtShell; }
    const SwWrtShell *GetWrtShell() const { return m_pWrtShell; }
    // Same as GetWrtShell, but return pointer to SwEditShell base of
    // (potentially incomplete) SwWrtShell:
    SwEditShell * GetEditShell();

    /// For Core - it knows the DocShell but not the WrtShell!
          SwFEShell *GetFEShell();
    const SwFEShell *GetFEShell() const
                { return const_cast<SwDocShell*>(this)->GetFEShell(); }

    /// For inserting document.
    Reader* StartConvertFrom(SfxMedium& rMedium, SwReader** ppRdr,
                            SwCursorShell* pCursorSh = nullptr, SwPaM* pPaM = nullptr);

#if defined WNT
    virtual bool DdeGetData( const OUString& rItem, const OUString& rMimeType,
                             css::uno::Any & rValue ) override;
    virtual bool DdeSetData( const OUString& rItem, const OUString& rMimeType,
                             const css::uno::Any & rValue ) override;
#endif
    virtual ::sfx2::SvLinkSource* DdeCreateLinkSource( const OUString& rItem ) override;
    virtual void ReconnectDdeLink(SfxObjectShell& rServer) override;

    virtual void FillClass( SvGlobalName * pClassName,
                                   SotClipboardFormatId * pClipFormat,
                                   OUString * pAppName,
                                   OUString * pLongUserName,
                                   OUString * pUserName,
                                   sal_Int32 nFileFormat,
                                   bool bTemplate = false ) const override;

    virtual std::set<Color> GetDocColors() override;

    virtual void LoadStyles( SfxObjectShell& rSource ) override;

    void _LoadStyles( SfxObjectShell& rSource, bool bPreserveCurrentDocument );

    /// Show page style format dialog
    /// @param nSlot
    /// Identifies slot by which the dialog is triggered. Used to activate certain dialog pane
    void FormatPage(
        const OUString& rPage,
        const OString& rPageId,
        SwWrtShell& rActShell );

    // #i59688#
    /** linked graphics are now loaded on demand.
     Thus, loading of linked graphics no longer needed and necessary for
     the load of document being finished. */

    void LoadingFinished();

    /// Cancel transfer (called from SFX).
    virtual void CancelTransfers() override;

    /// Re-read Doc from Html-source.
    void    ReloadFromHtml( const OUString& rStreamName, SwSrcView* pSrcView );

    sal_Int16   GetUpdateDocMode() const { return m_nUpdateDocMode; }

    void ToggleLayoutMode(SwView* pView);

    sal_uLong LoadStylesFromFile( const OUString& rURL, SwgReaderOption& rOpt,
                                  bool bUnoCall );
    void InvalidateModel();
    void ReactivateModel();

    virtual css::uno::Sequence< OUString >  GetEventNames() override;

    /// #i20883# Digital Signatures and Encryption
    virtual HiddenInformation GetHiddenInformationState( HiddenInformation nStates ) override;

    /** #i42634# Overwrites SfxObjectShell::UpdateLinks
     This new function is necessary to trigger update of links in docs
     read by the binary filter: */
    virtual void UpdateLinks() override;

    css::uno::Reference< css::frame::XController >
                                GetController();

    SfxInPlaceClient* GetIPClient( const ::svt::EmbeddedObjectRef& xObjRef );
    SAL_DLLPRIVATE bool IsTemplate() { return m_IsATemplate; }
    SAL_DLLPRIVATE void SetIsTemplate( bool bValue ) { m_IsATemplate = bValue; }

    virtual const ::sfx2::IXmlIdRegistry* GetXmlIdRegistry() const override;

    /** passwword protection for Writer (derived from SfxObjectShell)
     see also:    FN_REDLINE_ON, FN_REDLINE_ON */
    virtual bool    IsChangeRecording() const override;
    virtual bool    HasChangeRecordProtection() const override;
    virtual void    SetChangeRecording( bool bActivate ) override;
    virtual bool    SetProtectionPassword( const OUString &rPassword ) override;
    virtual bool    GetProtectionHash( /*out*/ css::uno::Sequence< sal_Int8 > &rPasswordHash ) override;
};

/** Find the right DocShell and create a new one:
    The return value specifies what should happen to the Shell
    0 - Error, could not find the DocShell
    1 - DocShell is an existing Document
    2 - DocShell was created anew, thus it needs to be closed again
        (will be assigned to xLockRef additionally)
*/
int SwFindDocShell( SfxObjectShellRef& xDocSh,
                    SfxObjectShellLock& xLockRef,
                    const OUString& rFileName,
                    const OUString& rPasswd,
                    const OUString& rFilter,
                    sal_Int16 nVersion,
                    SwDocShell* pDestSh );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
