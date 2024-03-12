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

#include <memory>
#include <vector>
#include <rtl/ref.hxx>
#include <com/sun/star/uno/Sequence.h>
#include <sfx2/docfac.hxx>
#include <sfx2/objsh.hxx>
#include "swdllapi.h"
#include "shellid.hxx"

#include <svl/lstner.hxx>
#include <sfx2/StyleManager.hxx>
#include <o3tl/deleter.hxx>

class SwDoc;
class SfxDocumentInfoDialog;
class SfxInPlaceClient;
class FontList;
class SwEditShell;
class SwView;
class SwWrtShell;
class SwFEShell;
class Reader;
class SwReader;
typedef std::unique_ptr<SwReader, o3tl::default_delete<SwReader>> SwReaderPtr;
class SwCursorShell;
class SwSrcView;
class SwPaM;
class SwgReaderOption;
class IDocumentDeviceAccess;
class IDocumentChartDataProviderAccess;
class SwDocShell;
class SwDrawModel;
class SwViewShell;
class SwDocStyleSheetPool;
namespace svt
{
class EmbeddedObjectRef;
}
namespace com::sun::star::frame { class XController; }
namespace ooo::vba { class XSinkCaller; }
namespace ooo::vba::word { class XDocument; }

// initialize DrawModel (in form of a SwDrawModel) and DocShell (in form of a SwDocShell)
// as needed, one or both parameters may be zero
void InitDrawModelAndDocShell(SwDocShell* pSwDocShell, SwDrawModel* pSwDrawModel);

class SAL_DLLPUBLIC_RTTI SwDocShell
    : public SfxObjectShell
    , public SfxListener
{
    rtl::Reference< SwDoc >                 m_xDoc;      ///< Document.
    rtl::Reference< SwDocStyleSheetPool > m_xBasePool; ///< Passing through for formats.
    std::unique_ptr<FontList> m_pFontList;          ///< Current Fontlist.
    bool        m_IsInUpdateFontList; ///< prevent nested calls of UpdateFontList

    std::unique_ptr<sfx2::StyleManager> m_pStyleManager;

    /** For "historical reasons" nothing can be done without the WrtShell.
     Back-pointer on View (again "for historical reasons").
     Back-pointer is valid until in Activate a new one is set
     or until it is deleted in the View's Dtor. */

    SwView*     m_pView;
    SwWrtShell* m_pWrtShell;

    std::unique_ptr<comphelper::EmbeddedObjectContainer> m_pOLEChildList;
    sal_Int16   m_nUpdateDocMode;   ///< contains the css::document::UpdateDocMode
    bool        m_IsATemplate;      ///< prevent nested calls of UpdateFontList

    bool m_IsRemovedInvisibleContent;
        ///< whether SID_MAIL_PREPAREEXPORT removed content that
        ///< SID_MAIL_EXPORT_FINISHED needs to restore

    css::uno::Reference< ooo::vba::XSinkCaller > mxAutomationDocumentEventsCaller;
    css::uno::Reference< ooo::vba::word::XDocument> mxAutomationDocumentObject;

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
    SAL_DLLPRIVATE virtual std::shared_ptr<SfxDocumentInfoDialog> CreateDocumentInfoDialog(weld::Window* pParent,
                                                                                           const SfxItemSet &rSet) override;
    /// OLE-stuff
    SAL_DLLPRIVATE virtual void Draw(OutputDevice*, const JobSetup&, sal_uInt16 nAspect, bool bOutputToWindow) override;

    /// Methods for StyleSheets

    /// @param nSlot
    /// Only used for nFamily == SfxStyleFamily::Page. Identifies optional Slot by which the edit is triggered.
    /// Used to activate certain dialog pane
    SAL_DLLPRIVATE void Edit(
        weld::Window* pDialogParent,
        const OUString &rName,
        const OUString& rParent,
        const SfxStyleFamily nFamily,
        SfxStyleSearchBits nMask,
        const bool bNew,
        const OUString& sPageId,
        SwWrtShell* pActShell,
        SfxRequest* pRequest = nullptr,
        sal_uInt16 nSlot = 0);

    SAL_DLLPRIVATE void                  Delete(const OUString &rName, SfxStyleFamily nFamily);
    SAL_DLLPRIVATE void                  Hide(const OUString &rName, SfxStyleFamily nFamily, bool bHidden);
    SAL_DLLPRIVATE SfxStyleFamily        ApplyStyles(const OUString &rName,
        const SfxStyleFamily nFamily,
        SwWrtShell* pShell,
        sal_uInt16 nMode);
    SAL_DLLPRIVATE SfxStyleFamily        DoWaterCan( const OUString &rName, SfxStyleFamily nFamily);
    SAL_DLLPRIVATE void                  UpdateStyle(const OUString &rName, SfxStyleFamily nFamily, SwWrtShell* pShell);
    SAL_DLLPRIVATE void                  MakeByExample(const OUString &rName,
                                               SfxStyleFamily nFamily, SfxStyleSearchBits nMask, SwWrtShell* pShell);

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
    SW_DLLPUBLIC static SfxObjectFactory& Factory();
    virtual SfxObjectFactory& GetFactory() const override { return Factory(); }

private:
    /// SfxInterface initializer.
    static void InitInterface_Impl();

public:
    SW_DLLPUBLIC static OUString GetEventName( sal_Int32 nId );

    /// Doc is required for SO data exchange!
    SW_DLLPUBLIC SwDocShell( SfxObjectCreateMode eMode = SfxObjectCreateMode::EMBEDDED );
    SwDocShell( SfxModelFlags i_nSfxCreationFlags );
    SW_DLLPUBLIC SwDocShell( SwDoc& rDoc, SfxObjectCreateMode eMode );
    virtual ~SwDocShell() override;

    /// OLE 2.0-notification.
    DECL_DLLPRIVATE_LINK( Ole2ModifiedHdl, bool, void );

    /// OLE-stuff.
    virtual void      SetVisArea( const tools::Rectangle &rRect ) override;
    virtual tools::Rectangle GetVisArea( sal_uInt16 nAspect ) const override;
    virtual Printer  *GetDocumentPrinter() override;
    virtual OutputDevice* GetDocumentRefDev() override;
    virtual void      OnDocumentPrinterChanged( Printer * pNewPrinter ) override;

    virtual void            SetModified( bool = true ) override;

    /// Dispatcher
    void                    Execute(SfxRequest &);
    void                    ExecStyleSheet(SfxRequest&);

    void                    GetState(SfxItemSet &);
    void                    StateStyleSheet(SfxItemSet&, SwWrtShell* pSh = nullptr );

    /// returns Doc. But be careful!
    SwDoc*                   GetDoc() { return m_xDoc.get(); }
    const SwDoc*             GetDoc() const { return m_xDoc.get(); }
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
    SW_DLLPUBLIC void SetView(SwView* pVw);
    const SwView *GetView() const { return m_pView; }
    SwView       *GetView()       { return m_pView; }

    /// Access to the SwWrtShell belonging to SwView.
          SwWrtShell *GetWrtShell()       { return m_pWrtShell; }
    const SwWrtShell *GetWrtShell() const { return m_pWrtShell; }
    // Same as GetWrtShell, but return pointer to SwEditShell base of
    // (potentially incomplete) SwWrtShell:
    SW_DLLPUBLIC SwEditShell * GetEditShell();

    /// For Core - it knows the DocShell but not the WrtShell!
    SW_DLLPUBLIC SwFEShell *GetFEShell();
    const SwFEShell *GetFEShell() const
                { return const_cast<SwDocShell*>(this)->GetFEShell(); }

    /// For inserting document.
    Reader* StartConvertFrom(SfxMedium& rMedium, SwReaderPtr& rpRdr,
                            SwCursorShell const * pCursorSh = nullptr, SwPaM* pPaM = nullptr);

#if defined(_WIN32)
    virtual bool DdeGetData( const OUString& rItem, const OUString& rMimeType,
                             css::uno::Any & rValue ) override;
    virtual bool DdeSetData( const OUString& rItem, const OUString& rMimeType,
                             const css::uno::Any & rValue ) override;
#endif
    virtual ::sfx2::SvLinkSource* DdeCreateLinkSource( const OUString& rItem ) override;
    virtual void ReconnectDdeLink(SfxObjectShell& rServer) override;

    virtual void FillClass( SvGlobalName * pClassName,
                                   SotClipboardFormatId * pClipFormat,
                                   OUString * pLongUserName,
                                   sal_Int32 nFileFormat,
                                   bool bTemplate = false ) const override;

    virtual std::set<Color> GetDocColors() override;

    virtual std::shared_ptr<model::ColorSet> GetThemeColors() override;

    sfx::AccessibilityIssueCollection runAccessibilityCheck() override;

    virtual void LoadStyles( SfxObjectShell& rSource ) override;

    SW_DLLPUBLIC void LoadStyles_( SfxObjectShell& rSource, bool bPreserveCurrentDocument );

    /// Show page style format dialog
    /// @param nSlot
    /// Identifies slot by which the dialog is triggered. Used to activate certain dialog pane
    SW_DLLPUBLIC void FormatPage(
        weld::Window* pDialogParent,
        const OUString& rPage,
        const OUString& rPageId,
        SwWrtShell& rActShell,
        SfxRequest* pRequest = nullptr);

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

    SW_DLLPUBLIC ErrCodeMsg LoadStylesFromFile(const OUString& rURL, SwgReaderOption& rOpt, bool bUnoCall);
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
    SAL_DLLPRIVATE bool IsTemplate() const { return m_IsATemplate; }
    SAL_DLLPRIVATE void SetIsTemplate( bool bValue ) { m_IsATemplate = bValue; }

    virtual const ::sfx2::IXmlIdRegistry* GetXmlIdRegistry() const override;

    /** passwword protection for Writer (derived from SfxObjectShell)
     see also:    FN_REDLINE_ON, FN_REDLINE_ON */
    virtual bool    IsChangeRecording() const override;
    virtual bool    HasChangeRecordProtection() const override;
    virtual void    SetChangeRecording( bool bActivate, bool bLockAllViews = false ) override;
    virtual void    SetProtectionPassword( const OUString &rPassword ) override;
    virtual bool    GetProtectionHash( /*out*/ css::uno::Sequence< sal_Int8 > &rPasswordHash ) override;

    SW_DLLPUBLIC void RegisterAutomationDocumentEventsCaller(css::uno::Reference< ooo::vba::XSinkCaller > const& xCaller);
    void CallAutomationDocumentEventSinks(const OUString& Method, css::uno::Sequence< css::uno::Any >& Arguments);
    SW_DLLPUBLIC void RegisterAutomationDocumentObject(css::uno::Reference< ooo::vba::word::XDocument > const& xDocument);

    // Lock all unlocked views, and returns a guard object which unlocks those views when destructed
    virtual std::unique_ptr<LockAllViewsGuard> LockAllViews() override;

protected:
    class LockAllViewsGuard_Impl final : public LockAllViewsGuard
    {
        std::vector<SwViewShell*> m_aViewWasUnLocked;

    public:
        explicit LockAllViewsGuard_Impl(SwViewShell* pViewShell);
        ~LockAllViewsGuard_Impl();
    };

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
                    std::u16string_view rFileName,
                    const OUString& rPasswd,
                    const OUString& rFilter,
                    sal_Int16 nVersion,
                    SwDocShell* pDestSh );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
