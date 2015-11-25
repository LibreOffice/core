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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_UIVWIMP_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_UIVWIMP_HXX

#include <config_features.h>

#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <view.hxx>

#include <sfx2/objsh.hxx>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboardListener.hpp>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/weakref.hxx>

class SwXTextView;
class SfxRequest;
class SwTransferable;
class SfxRequest;

namespace sfx2 { class DocumentInserter; }
namespace com{ namespace sun{ namespace star {
    namespace frame {
        class XDispatchProviderInterceptor;
    }
    namespace lang {
        class XUnoTunnel;
    }
}}}

class SwScannerEventListener : public ::cppu::WeakImplHelper<
    css::lang::XEventListener >
{
    SwView* pView;

public:

    SwScannerEventListener( SwView& rView ) : pView( &rView )  {}
    virtual ~SwScannerEventListener();

    // XEventListener
    virtual void SAL_CALL disposing(
                    const css::lang::EventObject& rEventObject ) throw(css::uno::RuntimeException, std::exception) override;

    void ViewDestroyed() { pView = nullptr; }
};

// Clipboard EventListener
class SwClipboardChangeListener : public ::cppu::WeakImplHelper<
    css::datatransfer::clipboard::XClipboardListener >
{
    SwView* pView;

    // XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& rEventObject )
        throw ( css::uno::RuntimeException, std::exception ) override;

    // XClipboardListener
    virtual void SAL_CALL changedContents( const css::datatransfer::clipboard::ClipboardEvent& rEventObject )
        throw (css::uno::RuntimeException,
               std::exception) override;

public:
    SwClipboardChangeListener( SwView& rView ) : pView( &rView ) {}
    virtual ~SwClipboardChangeListener();

    void ViewDestroyed() { pView = nullptr; }

    void AddRemoveListener( bool bAdd );
};

class SwMailMergeConfigItem;

class SwView_Impl
{
    css::uno::Reference< css::lang::XEventListener >  xScanEvtLstnr;
    css::uno::Reference< css::lang::XEventListener >  xClipEvtLstnr;
    css::uno::Reference< css::frame::XDispatchProviderInterceptor >   xDisProvInterceptor;
    css::uno::Reference< css::view::XSelectionSupplier >              mxXTextView;       // UNO object
    css::uno::WeakReference< css::lang::XUnoTunnel > xTransferable;

    // temporary document for printing text of selection / multi selection
    // in PDF export.
    SfxObjectShellLock           xTmpSelDocSh;

    SwView* pView;
    SwScannerEventListener*     pScanEvtLstnr;
    SwClipboardChangeListener*  pClipEvtLstnr;
    ShellModes                  eShellMode;

#if HAVE_FEATURE_DBCONNECTIVITY
    SwMailMergeConfigItem*      pConfigItem;
    sal_uInt16                  nMailMergeRestartPage;
    bool                    bMailMergeSourceView;
#endif

    sfx2::DocumentInserter*     m_pDocInserter;
    SfxRequest*                 m_pRequest;
    sal_Int16                   m_nParam;

    Point                       m_aEditingPosition;
    bool                        m_bSelectObject;
    bool                        m_bEditingPositionSet;

public:
    SwView_Impl(SwView* pShell);
    ~SwView_Impl();

    void                            SetShellMode(ShellModes eSet);

    css::view::XSelectionSupplier* GetUNOObject();
    SwXTextView*                    GetUNOObject_Impl();
    void                            Invalidate();

    ShellModes                      GetShellMode() {return eShellMode;}

    void                            ExecuteScan(SfxRequest& rReq);
    SwScannerEventListener&         GetScannerEventListener();

    void                            AddClipboardListener();

    void                            AddTransferable(SwTransferable& rTransferable);

#if HAVE_FEATURE_DBCONNECTIVITY
    void   SetMailMergeConfigItem(SwMailMergeConfigItem*  pItem,
                                                sal_uInt16 nRestart, bool bIsSource)
                            {   pConfigItem = pItem;
                                nMailMergeRestartPage = nRestart;
                                bMailMergeSourceView = bIsSource;
                            }
    SwMailMergeConfigItem*  GetMailMergeConfigItem() {return pConfigItem;}
    sal_uInt16              GetMailMergeRestartPage() const {return nMailMergeRestartPage;}
    bool                IsMailMergeSourceView() const { return bMailMergeSourceView;  }
#endif

    //#i33307# restore editing position
    void                    SetRestorePosition(const Point& rCursorPos, bool bSelectObj)
                            {
                                m_aEditingPosition = rCursorPos;
                                m_bSelectObject = bSelectObj;
                                m_bEditingPositionSet = true;
                            }
    bool                    GetRestorePosition(Point& rCursorPos, bool& rbSelectObj)
                            {
                                rCursorPos = m_aEditingPosition;
                                rbSelectObj = m_bSelectObject;
                                return m_bEditingPositionSet;
                            }

    void                    StartDocumentInserter( const OUString& rFactory, const Link<sfx2::FileDialogHelper*,void>& rEndDialogHdl );
    SfxMedium*              CreateMedium();
    void                    InitRequest( const SfxRequest& rRequest );

    inline SfxRequest*      GetRequest() const { return m_pRequest; }
    inline sal_Int16        GetParam() const { return m_nParam; }
    inline void             SetParam( sal_Int16 nParam ) { m_nParam = nParam; }
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
