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
#ifndef _SWDOCSH_HXX
#define _SWDOCSH_HXX

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
    SW_DLLPRIVATE void                  AddLink();
    SW_DLLPRIVATE void                  RemoveLink();

    /// Catch hint for DocInfo.
    SW_DLLPRIVATE virtual void          Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    /// FileIO
    SW_DLLPRIVATE virtual sal_Bool InitNew( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage );
    SW_DLLPRIVATE virtual sal_Bool Load( SfxMedium& rMedium );
    SW_DLLPRIVATE virtual sal_Bool LoadFrom( SfxMedium& rMedium );
    SW_DLLPRIVATE virtual sal_Bool            ConvertFrom( SfxMedium &rMedium );
    SW_DLLPRIVATE virtual sal_Bool            ConvertTo( SfxMedium &rMedium );
    SW_DLLPRIVATE virtual sal_Bool SaveAs( SfxMedium& rMedium );
    SW_DLLPRIVATE virtual sal_Bool SaveCompleted( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage );

    SW_DLLPRIVATE virtual sal_uInt16            PrepareClose( sal_Bool bUI = sal_True, sal_Bool bForBrowsing = sal_False );


    /// Make DocInfo known to the Doc.
    SW_DLLPRIVATE virtual SfxDocumentInfoDialog* CreateDocumentInfoDialog(
                                    Window *pParent, const SfxItemSet &);
    /// OLE-stuff
    SW_DLLPRIVATE virtual void          Draw( OutputDevice*, const JobSetup&, sal_uInt16);

    /// Methods for StyleSheets

    // @param nSlot
    // Only used for nFamily == SFX_STYLE_FAMILY_PAGE. Identifies optional Slot by which the edit is triggered.
    // Used to activate certain dialog pane
    SW_DLLPRIVATE sal_uInt16 Edit(
        const String &rName,
        const String& rParent,
        const sal_uInt16 nFamily,
        sal_uInt16 nMask,
        const sal_Bool bNew,
        const sal_uInt16 nSlot = 0,
        SwWrtShell* pActShell = 0,
        const sal_Bool bBasic = sal_False );

    SW_DLLPRIVATE sal_uInt16                    Delete(const String &rName, sal_uInt16 nFamily);
    SW_DLLPRIVATE sal_uInt16                    Hide(const String &rName, sal_uInt16 nFamily, bool bHidden);
    SW_DLLPRIVATE sal_uInt16                    ApplyStyles(const String &rName, sal_uInt16 nFamily, SwWrtShell* pShell = 0,
                                        sal_uInt16 nMode = 0 );
    SW_DLLPRIVATE sal_uInt16                    DoWaterCan( const String &rName, sal_uInt16 nFamily);
    SW_DLLPRIVATE sal_uInt16                    UpdateStyle(const String &rName, sal_uInt16 nFamily, SwWrtShell* pShell = 0);
    SW_DLLPRIVATE sal_uInt16                    MakeByExample(const String &rName,
                                            sal_uInt16 nFamily, sal_uInt16 nMask, SwWrtShell* pShell = 0);

    SW_DLLPRIVATE void                  InitDraw();
    SW_DLLPRIVATE void                  SubInitNew();   ///< for InitNew and HtmlSourceMode.

    SW_DLLPRIVATE void                  RemoveOLEObjects();
    SW_DLLPRIVATE void                  CalcLayoutForOLEObjects();

    SW_DLLPRIVATE void                    Init_Impl();
    SW_DLLPRIVATE DECL_STATIC_LINK( SwDocShell, IsLoadFinished, void* );


    using SfxObjectShell::GetVisArea;

protected:
    /// override to update text fields
    virtual void                DoFlushDocInfo();

public:
    using SotObject::GetInterface;

    /// but we implement this ourselves.
    SFX_DECL_INTERFACE(SW_DOCSHELL)
    SFX_DECL_OBJECTFACTORY()
    TYPEINFO();

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
    virtual void            SetModified( sal_Bool = sal_True );

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
    virtual sal_Bool            Save();

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

    virtual long DdeGetData( const OUString& rItem, const OUString& rMimeType,
                             ::com::sun::star::uno::Any & rValue );
    virtual long DdeSetData( const OUString& rItem, const OUString& rMimeType,
                             const ::com::sun::star::uno::Any & rValue );
    virtual ::sfx2::SvLinkSource* DdeCreateLinkSource( const OUString& rItem );
    virtual void ReconnectDdeLink(SfxObjectShell& rServer);

    virtual void FillClass( SvGlobalName * pClassName,
                                   sal_uInt32 * pClipFormat,
                                   OUString * pAppName,
                                   OUString * pLongUserName,
                                   OUString * pUserName,
                                   sal_Int32 nFileFormat,
                                   sal_Bool bTemplate = sal_False ) const;

    virtual void LoadStyles( SfxObjectShell& rSource );

    void _LoadStyles( SfxObjectShell& rSource, sal_Bool bPreserveCurrentDocument );

    // Show page style format dialog
    // @param nSlot
    // Identifies slot by which the dialog is triggered. Used to activate certain dialog pane
    void FormatPage(
        const String& rPage,
        const sal_uInt16 nSlot,
        SwWrtShell& rActShell );

    // #i59688#
    /** linked graphics are now loaded on demand.
     Thus, loading of linked graphics no longer needed and necessary for
     the load of document being finished. */

    void LoadingFinished();

    /// Cancel transfer (called from SFX).
    virtual void CancelTransfers();

    /// Re-read Doc from Html-source.
    void    ReloadFromHtml( const String& rStreamName, SwSrcView* pSrcView );

    sal_Int16   GetUpdateDocMode() const {return nUpdateDocMode;}

    /** Activate wait cursor for all windows of this document
    Optionally all dispatcher could be Locked
    Usually locking should be done using the class: SwWaitObject! */
    void EnterWait( sal_Bool bLockDispatcher );
    void LeaveWait( sal_Bool bLockDispatcher );

    void ToggleBrowserMode(sal_Bool bOn, SwView* pView);

    sal_uLong LoadStylesFromFile( const String& rURL, SwgReaderOption& rOpt,
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

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >
                                GetController();

    SfxInPlaceClient* GetIPClient( const ::svt::EmbeddedObjectRef& xObjRef );
    SW_DLLPRIVATE sal_Bool IsTemplate() { return bIsATemplate; }
    SW_DLLPRIVATE void SetIsTemplate( bool bValue ) { bIsATemplate = bValue; }

    virtual const ::sfx2::IXmlIdRegistry* GetXmlIdRegistry() const;

    /** passwword protection for Writer (derived from SfxObjectShell)
     see also:    FN_REDLINE_ON, FN_REDLINE_ON */
    virtual bool    IsChangeRecording() const;
    virtual bool    HasChangeRecordProtection() const;
    virtual void    SetChangeRecording( bool bActivate );
    virtual bool    SetProtectionPassword( const String &rPassword );
    virtual bool    GetProtectionHash( /*out*/ ::com::sun::star::uno::Sequence< sal_Int8 > &rPasswordHash );
};

class Graphic;
 /// implemented in source/ui/docvw/romenu.cxx
String ExportGraphic( const Graphic &rGraphic, const String &rGrfName );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
