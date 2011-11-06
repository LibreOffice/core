/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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

/* -----------------------------29.05.00 08:22--------------------------------

 ---------------------------------------------------------------------------*/
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


/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
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

