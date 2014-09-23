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
#include <vcl/timer.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/objsh.hxx>
#include "swdllapi.h"
#include <shellid.hxx>

#include <svl/lstner.hxx>
#include <svtools/embedhlp.hxx>

class SwDoc;
class SfxDocumentInfoDialog;
class SfxStyleSheetBasePool;
class SfxInPlaceClient;
class FontList;
class SwView;
class SwWrtShell;
class SwFEShell;
class Reader;
class SwReader;
class SwCrsrShell;
class SwSrcView;
class SwPaM;
class SwgReaderOption;
class SwOLEObj;
class IDocumentDeviceAccess;
class IDocumentSettingAccess;
class IDocumentTimerAccess;
class IDocumentChartDataProviderAccess;
class SwDocShell;
class SwDrawModel;

// initialize DrawModel (in form of a SwDrawModel) and DocShell (in form of a SwDocShell)
// as needed, one or both parameters may be zero
void SAL_DLLPRIVATE InitDrawModelAndDocShell(SwDocShell* pSwDocShell, SwDrawModel* pSwDrawModel);

class SW_DLLPUBLIC SwDocShell: public SfxObjectShell, public SfxListener
{
    SwDoc*                  mpDoc;                          ///< Document.
    rtl::Reference< SfxStyleSheetBasePool > mxBasePool;     ///< Passing through for formats.
    FontList*               mpFontList;                     ///< Current Fontlist.
    bool                    mbInUpdateFontList; ///< prevent nested calls of UpdateFontList

    /** For "historical reasons" nothing can be done without the WrtShell.
     Back-pointer on View (again "for historical reasons").
     Back-pointer is valid until in Activate a new one is set
     or until it is deleted in the View's Dtor. */

    SwView*                 mpView;
    SwWrtShell*             mpWrtShell;

    Timer                   aFinishedTimer; /**< Timer for checking graphics-links.
                                             If all are present, the doc is loaded completely. */

    comphelper::EmbeddedObjectContainer*    mpOLEChildList;
    sal_Int16               mnUpdateDocMode;    ///< contains the com::sun::star::document::UpdateDocMode
    bool                    bIsATemplate;      ///< prevent nested calls of UpdateFontList

    /// Methods for access to doc.
    SAL_DLLPRIVATE void                  AddLink();
    SAL_DLLPRIVATE void                  RemoveLink();

    /// Catch hint for DocInfo.
    SAL_DLLPRIVATE virtual void          Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) SAL_OVERRIDE;

    /// FileIO
    SAL_DLLPRIVATE virtual bool InitNew( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage ) SAL_OVERRIDE;
    SAL_DLLPRIVATE virtual bool Load( SfxMedium& rMedium ) SAL_OVERRIDE;
    SAL_DLLPRIVATE virtual bool LoadFrom( SfxMedium& rMedium ) SAL_OVERRIDE;
    SAL_DLLPRIVATE virtual bool ConvertFrom( SfxMedium &rMedium ) SAL_OVERRIDE;
    SAL_DLLPRIVATE virtual bool ConvertTo( SfxMedium &rMedium ) SAL_OVERRIDE;
    SAL_DLLPRIVATE virtual bool SaveAs( SfxMedium& rMedium ) SAL_OVERRIDE;
    SAL_DLLPRIVATE virtual bool SaveCompleted( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage ) SAL_OVERRIDE;

    SAL_DLLPRIVATE virtual bool     PrepareClose( bool bUI = true ) SAL_OVERRIDE;

    SAL_DLLPRIVATE virtual bool     InsertGeneratedStream(SfxMedium& rMedium,
            css::uno::Reference<css::text::XTextRange> const& xInsertPosition)
        SAL_OVERRIDE;

    /// Make DocInfo known to the Doc.
    SAL_DLLPRIVATE virtual SfxDocumentInfoDialog* CreateDocumentInfoDialog(
                                    vcl::Window *pParent, const SfxItemSet &) SAL_OVERRIDE;
    /// OLE-stuff
    SAL_DLLPRIVATE virtual void          Draw( OutputDevice*, const JobSetup&, sal_uInt16) SAL_OVERRIDE;

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
        SwWrtShell* pActShell = 0,
        const bool bBasic = false );

    SAL_DLLPRIVATE sal_uInt16            Delete(const OUString &rName, sal_uInt16 nFamily);
    SAL_DLLPRIVATE sal_uInt16            Hide(const OUString &rName, sal_uInt16 nFamily, bool bHidden);
    SAL_DLLPRIVATE sal_uInt16            ApplyStyles(const OUString &rName,
        const sal_uInt16 nFamily,
        SwWrtShell* pShell = 0,
        sal_uInt16 nMode = 0);
    SAL_DLLPRIVATE sal_uInt16            DoWaterCan( const OUString &rName, sal_uInt16 nFamily);
    SAL_DLLPRIVATE sal_uInt16            UpdateStyle(const OUString &rName, sal_uInt16 nFamily, SwWrtShell* pShell = 0);
    SAL_DLLPRIVATE sal_uInt16            MakeByExample(const OUString &rName,
                                        sal_uInt16 nFamily, sal_uInt16 nMask, SwWrtShell* pShell = 0);

    SAL_DLLPRIVATE void                  InitDraw();
    SAL_DLLPRIVATE void                  SubInitNew();   ///< for InitNew and HtmlSourceMode.

    SAL_DLLPRIVATE void                  RemoveOLEObjects();
    SAL_DLLPRIVATE void                  CalcLayoutForOLEObjects();

    SAL_DLLPRIVATE void                  Init_Impl();
    SAL_DLLPRIVATE DECL_STATIC_LINK( SwDocShell, IsLoadFinished, void* );

    using SfxObjectShell::GetVisArea;

protected:
    /// override to update text fields
    virtual void                DoFlushDocInfo() SAL_OVERRIDE;

public:
    using SotObject::GetInterface;

    /// but we implement this ourselves.
    SFX_DECL_INTERFACE(SW_DOCSHELL)
    SFX_DECL_OBJECTFACTORY()
    TYPEINFO_OVERRIDE();

private:
    /// SfxInterface initializer.
    static void InitInterface_Impl();

public:
    static SfxInterface *_GetInterface() { return GetStaticInterface(); }

    static OUString GetEventName( sal_Int32 nId );

    /// Doc is required for SO data exchange!
    SwDocShell( SfxObjectCreateMode eMode = SFX_CREATE_MODE_EMBEDDED );
    SwDocShell( const sal_uInt64 i_nSfxCreationFlags );
    SwDocShell( SwDoc *pDoc, SfxObjectCreateMode eMode = SFX_CREATE_MODE_STANDARD );
    virtual ~SwDocShell();

    /// OLE 2.0-notification.
    DECL_LINK( Ole2ModifiedHdl, void * );

    /// OLE-stuff.
    virtual void      SetVisArea( const Rectangle &rRect ) SAL_OVERRIDE;
    virtual Rectangle GetVisArea( sal_uInt16 nAspect ) const SAL_OVERRIDE;
    virtual Printer  *GetDocumentPrinter() SAL_OVERRIDE;
    virtual OutputDevice* GetDocumentRefDev() SAL_OVERRIDE;
    virtual void      OnDocumentPrinterChanged( Printer * pNewPrinter ) SAL_OVERRIDE;
    virtual sal_uLong     GetMiscStatus() const SAL_OVERRIDE;

    virtual void            PrepareReload() SAL_OVERRIDE;
    virtual void            SetModified( bool = true ) SAL_OVERRIDE;

    /// Dispatcher
    void                    Execute(SfxRequest &);
    void                    ExecStyleSheet(SfxRequest&);
    void                    ExecDB(SfxRequest&);

    void                    GetState(SfxItemSet &);
    void                    StateAlways(SfxItemSet &);
    void                    StateStyleSheet(SfxItemSet&, SwWrtShell* pSh = 0 );

    /// returns Doc. But be careful!
    inline SwDoc*                   GetDoc() { return mpDoc; }
    inline const SwDoc*             GetDoc() const { return mpDoc; }
    IDocumentDeviceAccess*          getIDocumentDeviceAccess();
    const IDocumentSettingAccess*   getIDocumentSettingAccess() const;
    IDocumentChartDataProviderAccess*       getIDocumentChartDataProviderAccess();

    void                    UpdateFontList();
    void                    UpdateChildWindows();

    /// global IO.
    virtual bool            Save() SAL_OVERRIDE;

    /// For Style PI.
    virtual SfxStyleSheetBasePool*  GetStyleSheetPool() SAL_OVERRIDE;

    /// Set View for actions via Shell.
    void          SetView(SwView* pVw);
    const SwView *GetView() const { return mpView; }
    SwView       *GetView()       { return mpView; }

    /// Accress to the SwWrtShell belonging to SwView.
          SwWrtShell *GetWrtShell()       { return mpWrtShell; }
    const SwWrtShell *GetWrtShell() const { return mpWrtShell; }

    /// For Core - it knows the DocShell but not the WrtShell!
          SwFEShell *GetFEShell();
    const SwFEShell *GetFEShell() const
                { return ((SwDocShell*)this)->GetFEShell(); }

    /// For inserting document.
    Reader* StartConvertFrom(SfxMedium& rMedium, SwReader** ppRdr,
                            SwCrsrShell* pCrsrSh = 0, SwPaM* pPaM = 0);

    virtual bool DdeGetData( const OUString& rItem, const OUString& rMimeType,
                             ::com::sun::star::uno::Any & rValue ) SAL_OVERRIDE;
    virtual bool DdeSetData( const OUString& rItem, const OUString& rMimeType,
                             const ::com::sun::star::uno::Any & rValue ) SAL_OVERRIDE;
    virtual ::sfx2::SvLinkSource* DdeCreateLinkSource( const OUString& rItem ) SAL_OVERRIDE;
    virtual void ReconnectDdeLink(SfxObjectShell& rServer) SAL_OVERRIDE;

    virtual void FillClass( SvGlobalName * pClassName,
                                   sal_uInt32 * pClipFormat,
                                   OUString * pAppName,
                                   OUString * pLongUserName,
                                   OUString * pUserName,
                                   sal_Int32 nFileFormat,
                                   bool bTemplate = false ) const SAL_OVERRIDE;

    virtual std::vector<Color> GetDocColors() SAL_OVERRIDE;

    virtual void LoadStyles( SfxObjectShell& rSource ) SAL_OVERRIDE;

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
    virtual void CancelTransfers() SAL_OVERRIDE;

    /// Re-read Doc from Html-source.
    void    ReloadFromHtml( const OUString& rStreamName, SwSrcView* pSrcView );

    sal_Int16   GetUpdateDocMode() const {return mnUpdateDocMode;}

    void ToggleBrowserMode(bool bOn, SwView* pView);

    sal_uLong LoadStylesFromFile( const OUString& rURL, SwgReaderOption& rOpt,
                                  bool bUnoCall );
    void InvalidateModel();
    void ReactivateModel();

    virtual ::com::sun::star::uno::Sequence< OUString >  GetEventNames() SAL_OVERRIDE;

    /// #i20883# Digital Signatures and Encryption
    virtual sal_uInt16 GetHiddenInformationState( sal_uInt16 nStates ) SAL_OVERRIDE;

    /** #i42634# Overwrites SfxObjectShell::UpdateLinks
     This new function is necessary to trigger update of links in docs
     read by the binary filter: */
    virtual void UpdateLinks() SAL_OVERRIDE;

    virtual void setDocAccTitle( const OUString& rTitle );
    virtual const OUString getDocAccTitle() const;

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >
                                GetController();

    SfxInPlaceClient* GetIPClient( const ::svt::EmbeddedObjectRef& xObjRef );
    SAL_DLLPRIVATE bool IsTemplate() { return bIsATemplate; }
    SAL_DLLPRIVATE void SetIsTemplate( bool bValue ) { bIsATemplate = bValue; }

    virtual const ::sfx2::IXmlIdRegistry* GetXmlIdRegistry() const SAL_OVERRIDE;

    /** passwword protection for Writer (derived from SfxObjectShell)
     see also:    FN_REDLINE_ON, FN_REDLINE_ON */
    virtual bool    IsChangeRecording() const SAL_OVERRIDE;
    virtual bool    HasChangeRecordProtection() const SAL_OVERRIDE;
    virtual void    SetChangeRecording( bool bActivate ) SAL_OVERRIDE;
    virtual bool    SetProtectionPassword( const OUString &rPassword ) SAL_OVERRIDE;
    virtual bool    GetProtectionHash( /*out*/ ::com::sun::star::uno::Sequence< sal_Int8 > &rPasswordHash ) SAL_OVERRIDE;
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
