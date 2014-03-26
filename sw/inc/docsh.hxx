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
#include <swdll.hxx>
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

class SW_DLLPUBLIC SwDocShell: public SfxObjectShell, public SfxListener
{
    SwDoc*                  pDoc;                           ///< Document.
    rtl::Reference< SfxStyleSheetBasePool > mxBasePool;     ///< Passing through for formats.
    FontList*               pFontList;                      ///< Current Fontlist.

    /** For "historical reasons" nothing can be done without the WrtShell.
     Back-pointer on View (again "for historical reasons").
     Back-pointer is valid until in Activate a new one is set
     or until it is deleted in the View's Dtor. */

    SwView*                 pView;
    SwWrtShell*             pWrtShell;

    Timer                   aFinishedTimer; /**< Timer for checking graphics-links.
                                             If all are present, the doc is loaded completely. */

    comphelper::EmbeddedObjectContainer*    pOLEChildList;
    sal_Int16               nUpdateDocMode;    ///< contains the com::sun::star::document::UpdateDocMode
    bool                    bInUpdateFontList; ///< prevent nested calls of UpdateFontList
    bool                    bIsATemplate;      ///< prevent nested calls of UpdateFontList

    /// Methods for access to doc.
    SAL_DLLPRIVATE void                  AddLink();
    SAL_DLLPRIVATE void                  RemoveLink();

    /// Catch hint for DocInfo.
    SAL_DLLPRIVATE virtual void          Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    /// FileIO
    SAL_DLLPRIVATE virtual bool InitNew( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage );
    SAL_DLLPRIVATE virtual bool Load( SfxMedium& rMedium );
    SAL_DLLPRIVATE virtual bool LoadFrom( SfxMedium& rMedium );
    SAL_DLLPRIVATE virtual bool ConvertFrom( SfxMedium &rMedium );
    SAL_DLLPRIVATE virtual bool ConvertTo( SfxMedium &rMedium );
    SAL_DLLPRIVATE virtual bool SaveAs( SfxMedium& rMedium );
    SAL_DLLPRIVATE virtual bool SaveCompleted( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage );

    SAL_DLLPRIVATE virtual bool     PrepareClose( sal_Bool bUI = sal_True ) SAL_OVERRIDE;

    SAL_DLLPRIVATE virtual bool     InsertGeneratedStream(SfxMedium& rMedium,
            css::uno::Reference<css::text::XTextRange> const& xInsertPosition)
        SAL_OVERRIDE;

    /// Make DocInfo known to the Doc.
    SAL_DLLPRIVATE virtual SfxDocumentInfoDialog* CreateDocumentInfoDialog(
                                    Window *pParent, const SfxItemSet &);
    /// OLE-stuff
    SAL_DLLPRIVATE virtual void          Draw( OutputDevice*, const JobSetup&, sal_uInt16);

    /// Methods for StyleSheets

    // @param nSlot
    // Only used for nFamily == SFX_STYLE_FAMILY_PAGE. Identifies optional Slot by which the edit is triggered.
    // Used to activate certain dialog pane
    SAL_DLLPRIVATE sal_uInt16 Edit(
        const OUString &rName,
        const OUString& rParent,
        const sal_uInt16 nFamily,
        sal_uInt16 nMask,
        const sal_Bool bNew,
        const OString& sPageId = OString(),
        SwWrtShell* pActShell = 0,
        const sal_Bool bBasic = sal_False );

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
    virtual void                DoFlushDocInfo();

public:
    using SotObject::GetInterface;

    /// but we implement this ourselves.
    SFX_DECL_INTERFACE(SW_DOCSHELL)
    SFX_DECL_OBJECTFACTORY()
    TYPEINFO_OVERRIDE();

    static SfxInterface *_GetInterface() { return GetStaticInterface(); }

    static OUString GetEventName( sal_Int32 nId );

    /// Doc is required for SO data exchange!
    SwDocShell( SfxObjectCreateMode eMode = SFX_CREATE_MODE_EMBEDDED );
    SwDocShell( const sal_uInt64 i_nSfxCreationFlags );
    SwDocShell( SwDoc *pDoc, SfxObjectCreateMode eMode = SFX_CREATE_MODE_STANDARD );
    ~SwDocShell();

    /// OLE 2.0-notification.
    DECL_LINK( Ole2ModifiedHdl, void * );

    /// OLE-stuff.
    virtual void      SetVisArea( const Rectangle &rRect );
    virtual Rectangle GetVisArea( sal_uInt16 nAspect ) const;
    virtual Printer  *GetDocumentPrinter();
    virtual OutputDevice* GetDocumentRefDev();
    virtual void      OnDocumentPrinterChanged( Printer * pNewPrinter );
    virtual sal_uLong     GetMiscStatus() const;

    virtual void            PrepareReload();
    virtual void            SetModified( bool = true );

    /// Dispatcher
    void                    Execute(SfxRequest &);
    void                    ExecStyleSheet(SfxRequest&);
    void                    ExecDB(SfxRequest&);

    void                    GetState(SfxItemSet &);
    void                    StateAlways(SfxItemSet &);
    void                    StateStyleSheet(SfxItemSet&, SwWrtShell* pSh = 0 );

    /// returns Doc. But be careful!
    inline SwDoc*                   GetDoc() { return pDoc; }
    inline const SwDoc*             GetDoc() const { return pDoc; }
    IDocumentDeviceAccess*          getIDocumentDeviceAccess();
    const IDocumentSettingAccess*   getIDocumentSettingAccess() const;
    IDocumentChartDataProviderAccess*       getIDocumentChartDataProviderAccess();

    void                    UpdateFontList();
    void                    UpdateChildWindows();

    /// global IO.
    virtual bool            Save();

    /// For Style PI.
    virtual SfxStyleSheetBasePool*  GetStyleSheetPool();

    /// Set View for actions via Shell.
    void          SetView(SwView* pVw);
    const SwView *GetView() const { return pView; }
    SwView       *GetView()       { return pView; }

    /// Accress to the SwWrtShell belonging to SwView.
          SwWrtShell *GetWrtShell()       { return pWrtShell; }
    const SwWrtShell *GetWrtShell() const { return pWrtShell; }

    /// For Core - it knows the DocShell but not the WrtShell!
          SwFEShell *GetFEShell();
    const SwFEShell *GetFEShell() const
                { return ((SwDocShell*)this)->GetFEShell(); }

    /// For inserting document.
    Reader* StartConvertFrom(SfxMedium& rMedium, SwReader** ppRdr,
                            SwCrsrShell* pCrsrSh = 0, SwPaM* pPaM = 0);

    virtual bool DdeGetData( const OUString& rItem, const OUString& rMimeType,
                             ::com::sun::star::uno::Any & rValue );
    virtual bool DdeSetData( const OUString& rItem, const OUString& rMimeType,
                             const ::com::sun::star::uno::Any & rValue );
    virtual ::sfx2::SvLinkSource* DdeCreateLinkSource( const OUString& rItem );
    virtual void ReconnectDdeLink(SfxObjectShell& rServer);

    virtual void FillClass( SvGlobalName * pClassName,
                                   sal_uInt32 * pClipFormat,
                                   OUString * pAppName,
                                   OUString * pLongUserName,
                                   OUString * pUserName,
                                   sal_Int32 nFileFormat,
                                   bool bTemplate = false ) const;

    virtual void LoadStyles( SfxObjectShell& rSource );

    void _LoadStyles( SfxObjectShell& rSource, sal_Bool bPreserveCurrentDocument );

    // Show page style format dialog
    // @param nSlot
    // Identifies slot by which the dialog is triggered. Used to activate certain dialog pane
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
    virtual void CancelTransfers();

    /// Re-read Doc from Html-source.
    void    ReloadFromHtml( const OUString& rStreamName, SwSrcView* pSrcView );

    sal_Int16   GetUpdateDocMode() const {return nUpdateDocMode;}

    void ToggleBrowserMode(sal_Bool bOn, SwView* pView);

    sal_uLong LoadStylesFromFile( const OUString& rURL, SwgReaderOption& rOpt,
                                sal_Bool bUnoCall );
    void InvalidateModel();
    void ReactivateModel();

    virtual ::com::sun::star::uno::Sequence< OUString >  GetEventNames();

    /// #i20883# Digital Signatures and Encryption
    virtual sal_uInt16 GetHiddenInformationState( sal_uInt16 nStates );

    /** #i42634# Overwrites SfxObjectShell::UpdateLinks
     This new function is necessary to trigger update of links in docs
     read by the binary filter: */
    virtual void UpdateLinks();

    virtual void setDocAccTitle( const OUString& rTitle );
    virtual const OUString getDocAccTitle() const;

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >
                                GetController();

    SfxInPlaceClient* GetIPClient( const ::svt::EmbeddedObjectRef& xObjRef );
    SAL_DLLPRIVATE sal_Bool IsTemplate() { return bIsATemplate; }
    SAL_DLLPRIVATE void SetIsTemplate( bool bValue ) { bIsATemplate = bValue; }

    virtual const ::sfx2::IXmlIdRegistry* GetXmlIdRegistry() const;

    /** passwword protection for Writer (derived from SfxObjectShell)
     see also:    FN_REDLINE_ON, FN_REDLINE_ON */
    virtual bool    IsChangeRecording() const;
    virtual bool    HasChangeRecordProtection() const;
    virtual void    SetChangeRecording( bool bActivate );
    virtual bool    SetProtectionPassword( const OUString &rPassword );
    virtual bool    GetProtectionHash( /*out*/ ::com::sun::star::uno::Sequence< sal_Int8 > &rPasswordHash );
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
