/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: uivwimp.hxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: hr $ $Date: 2005-09-23 15:07:08 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _UIVWIMP_HXX
#define _UIVWIMP_HXX

#ifndef _COM_SUN_STAR_EMBED_XEMBEDDEDOBJECT_HPP_
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#endif

#ifndef _SWVIEW_HXX //autogen
#include <view.hxx>
#endif

#include <sfx2/objsh.hxx>

#ifndef _COM_SUN_STAR_VIEW_XSELECTIONSUPPLIER_HPP_
#include <com/sun/star/view/XSelectionSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBOARD_XCLIPBOARDLISTENER_HPP_
#include <com/sun/star/datatransfer/clipboard/XClipboardListener.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx> // helper for implementations
#endif
#ifndef _SWUNODEF_HXX
#include <swunodef.hxx>
#endif
#ifndef _CPPUHELPER_WEAKREF_HXX_
#include <cppuhelper/weakref.hxx>
#endif

class SwXTextView;
class SfxRequest;
class SwTransferable;

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

    void AddRemoveListener( BOOL bAdd );
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
    SfxObjectShellRef           xTmpSelDocSh;
    SfxObjectShellRef           aEmbeddedObjRef;

    SwView* pView;
    SwScannerEventListener*     pScanEvtLstnr;
    SwClipboardChangeListener*  pClipEvtLstnr;
    ShellModes                  eShellMode;

    SwMailMergeConfigItem*      pConfigItem;
    sal_uInt16                  nMailMergeRestartPage;
    sal_Bool                    bMailMergeSourceView;

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

    SfxObjectShellRef &             GetTmpSelectionDoc()    { return xTmpSelDocSh; }

    SfxObjectShellRef&              GetEmbeddedObjRef()     { return (SfxObjectShellRef&)(long&)aEmbeddedObjRef; }

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

};
#endif

