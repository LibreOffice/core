/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _UIVWIMP_HXX
#define _UIVWIMP_HXX

#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <view.hxx>

#include <sfx2/objsh.hxx>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboardListener.hpp>
#include <cppuhelper/implbase1.hxx> // helper for implementations
#include <swunodef.hxx>
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

class SwScannerEventListener : public ::cppu::WeakImplHelper1<
    STAR_NMSPC::lang::XEventListener >
{
    SwView* pView;

public:

    SwScannerEventListener( SwView& rView ) : pView( &rView )  {}
    virtual ~SwScannerEventListener();

    // XEventListener
    virtual void SAL_CALL disposing(
                    const ::com::sun::star::lang::EventObject& rEventObject ) throw(::com::sun::star::uno::RuntimeException);

    void ViewDestroyed() { pView = 0; }
};

// --------------------------- Clipboard EventListener ------------------
class SwClipboardChangeListener : public ::cppu::WeakImplHelper1<
    CLIP_NMSPC::XClipboardListener >
{
    SwView* pView;

    // XEventListener
    virtual void SAL_CALL disposing( const STAR_NMSPC::lang::EventObject& rEventObject )
        throw ( com::sun::star::uno::RuntimeException );

    // XClipboardListener
    virtual void SAL_CALL changedContents( const CLIP_NMSPC::ClipboardEvent& rEventObject )
        throw ( com::sun::star::uno::RuntimeException );

public:
    SwClipboardChangeListener( SwView& rView ) : pView( &rView ) {}
    virtual ~SwClipboardChangeListener();

    void ViewDestroyed() { pView = 0; }

    void AddRemoveListener( sal_Bool bAdd );
};

class SwMailMergeConfigItem;

class SwView_Impl
{
    STAR_REFERENCE( lang::XEventListener )  xScanEvtLstnr;
    STAR_REFERENCE( lang::XEventListener )  xClipEvtLstnr;
    STAR_REFERENCE( frame::XDispatchProviderInterceptor )   xDisProvInterceptor;
    STAR_REFERENCE( view::XSelectionSupplier )              *pxXTextView;       // UNO object
    com::sun::star::uno::WeakReference< com::sun::star::lang::XUnoTunnel > xTransferable;

    // temporary document for printing text of selection / multi selection
    // in PDF export.
    SfxObjectShellLock           xTmpSelDocSh;

    SwView* pView;
    SwScannerEventListener*     pScanEvtLstnr;
    SwClipboardChangeListener*  pClipEvtLstnr;
    ShellModes                  eShellMode;

    SwMailMergeConfigItem*      pConfigItem;
    sal_uInt16                  nMailMergeRestartPage;
    sal_Bool                    bMailMergeSourceView;

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

    ::com::sun::star::view::XSelectionSupplier* GetUNOObject();
    SwXTextView*                    GetUNOObject_Impl();
    void                            Invalidate();

    ShellModes                      GetShellMode() {return eShellMode;}

    void                            ExecuteScan(SfxRequest& rReq);
    SwScannerEventListener&         GetScannerEventListener();

    void                            AddClipboardListener();

    SfxObjectShellLock&             GetTmpSelectionDoc()    { return xTmpSelDocSh; }

    void                            AddTransferable(SwTransferable& rTransferable);

    void   SetMailMergeConfigItem(SwMailMergeConfigItem*  pItem,
                                                sal_uInt16 nRestart, sal_Bool bIsSource)
                            {   pConfigItem = pItem;
                                nMailMergeRestartPage = nRestart;
                                bMailMergeSourceView = bIsSource;
                            }
    SwMailMergeConfigItem*  GetMailMergeConfigItem() {return pConfigItem;}
    sal_uInt16              GetMailMergeRestartPage() const {return nMailMergeRestartPage;}
    sal_Bool                IsMailMergeSourceView() const { return bMailMergeSourceView;  }

    //#i33307# restore editing position
    void                    SetRestorePosition(const Point& rCrsrPos, bool bSelectObj)
                            {
                                m_aEditingPosition = rCrsrPos;
                                m_bSelectObject = bSelectObj;
                                m_bEditingPositionSet = true;
                            }
    bool                    GetRestorePosition(Point& rCrsrPos, bool& rbSelectObj)
                            {
                                rCrsrPos = m_aEditingPosition;
                                rbSelectObj = m_bSelectObject;
                                return m_bEditingPositionSet;
                            }


    void                    StartDocumentInserter( const String& rFactory, const Link& rEndDialogHdl );
    SfxMedium*              CreateMedium();
    void                    InitRequest( const SfxRequest& rRequest );

    inline SfxRequest*      GetRequest() const { return m_pRequest; }
    inline sal_Int16        GetParam() const { return m_nParam; }
    inline void             SetParam( sal_Int16 nParam ) { m_nParam = nParam; }
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
