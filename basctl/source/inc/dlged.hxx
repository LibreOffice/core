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



#ifndef _BASCTL_DLGED_HXX
#define _BASCTL_DLGED_HXX

#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/awt/XControlContainer.hpp>
#include <com/sun/star/datatransfer/DataFlavor.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <tools/link.hxx>
#include <tools/gen.hxx>
#include <vcl/timer.hxx>
#include <svl/hint.hxx>
#include <svl/brdcst.hxx>

#define DLGED_PAGE_WIDTH_MIN    1280
#define DLGED_PAGE_HEIGHT_MIN   1024


//============================================================================
// DlgEdHint
//============================================================================

enum DlgEdHintKind
{
    DLGED_HINT_UNKNOWN,
    DLGED_HINT_WINDOWSCROLLED,
    DLGED_HINT_LAYERCHANGED,
    DLGED_HINT_OBJORDERCHANGED,
    DLGED_HINT_SELECTIONCHANGED
};

class DlgEdObj;

class DlgEdHint: public SfxHint
{
private:
    DlgEdHintKind   eHintKind;
    DlgEdObj*       pDlgEdObj;

public:
    DlgEdHint( DlgEdHintKind eHint );
    DlgEdHint( DlgEdHintKind eHint, DlgEdObj* pObj );
    virtual ~DlgEdHint();

    DlgEdHintKind   GetKind() const { return eHintKind; }
    DlgEdObj*       GetObject() const { return pDlgEdObj; }
};


//============================================================================
// DlgEditor
//============================================================================

enum DlgEdMode { DLGED_INSERT, DLGED_SELECT, DLGED_TEST, DLGED_READONLY };

class ScrollBar;
class DlgEdModel;
class DlgEdPage;
class DlgEdView;
class DlgEdForm;
class DlgEdFactory;
class DlgEdFunc;
class Printer;
class KeyEvent;
class MouseEvent;
class Timer;
class Window;

class DlgEditor: public SfxBroadcaster
{
private:
    DECL_LINK( PaintTimeout, Timer * );
    DECL_LINK( MarkTimeout, Timer * );

    void Print( Printer* pPrinter, const String& rTitle );

protected:
    ScrollBar*          pHScroll;
    ScrollBar*          pVScroll;
    DlgEdModel*         pDlgEdModel;
    DlgEdPage*          pDlgEdPage;
    DlgEdView*          pDlgEdView;
    DlgEdForm*          pDlgEdForm;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >     m_xUnoControlDialogModel;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer >        m_xControlContainer;
    ::com::sun::star::uno::Sequence< ::com::sun::star::datatransfer::DataFlavor >       m_ClipboardDataFlavors;
    ::com::sun::star::uno::Sequence< ::com::sun::star::datatransfer::DataFlavor >       m_ClipboardDataFlavorsResource;
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >  m_xSupplier;
    DlgEdFactory*       pObjFac;
    Window*             pWindow;
    DlgEdFunc*          pFunc;
    DlgEdMode           eMode;
    sal_uInt16              eActObj;
    sal_Bool                bFirstDraw;
    Size                aGridSize;
    sal_Bool                bGridVisible;
    sal_Bool                bGridSnap;
    sal_Bool                bCreateOK;
    Timer               aPaintTimer;
    Rectangle           aPaintRect;
    sal_Bool                bDialogModelChanged;
    Timer               aMarkTimer;
    long                mnPaintGuard;

public:
    DlgEditor();
    ~DlgEditor();

    void            SetWindow( Window* pWindow );
    Window*         GetWindow() const { return pWindow; }

    /** returns the control container associated with our window
        @see GetWindow
        @see SetWindow
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer >
                    GetWindowControlContainer();

    void            SetDlgEdForm( DlgEdForm* pForm ) { pDlgEdForm = pForm; }
    DlgEdForm*      GetDlgEdForm() const { return pDlgEdForm; }

    void            SetScrollBars( ScrollBar* pHScroll, ScrollBar* pVScroll );
    void            InitScrollBars();
    ScrollBar*      GetHScroll() const { return pHScroll; }
    ScrollBar*      GetVScroll() const { return pVScroll; }
    void            DoScroll( ScrollBar* pActScroll );
    void            UpdateScrollBars();

    void            SetDialog( ::com::sun::star::uno::Reference<
                        ::com::sun::star::container::XNameContainer > xUnoControlDialogModel );
    void            ResetDialog( void );
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > GetDialog() const
                        {return m_xUnoControlDialogModel;}

    ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier > const & GetNumberFormatsSupplier();

    DlgEdModel*     GetModel()      const { return pDlgEdModel; }
    DlgEdView*      GetView()       const { return pDlgEdView; }
    DlgEdPage*      GetPage()       const { return pDlgEdPage; }

    void            ShowDialog();

    sal_Bool            UnmarkDialog();
    sal_Bool            RemarkDialog();

    void            SetDialogModelChanged( sal_Bool bChanged = sal_True ) { bDialogModelChanged = bChanged; }
    sal_Bool            IsDialogModelChanged() const { return bDialogModelChanged; }

    sal_Bool            IsModified() const;
    void            ClearModifyFlag();

    void            MouseButtonDown( const MouseEvent& rMEvt );
    void            MouseButtonUp( const MouseEvent& rMEvt );
    void            MouseMove( const MouseEvent& rMEvt );
    void            Paint( const Rectangle& rRect );
    sal_Bool            KeyInput( const KeyEvent& rKEvt );

    void            SetMode( DlgEdMode eMode );
    void            SetInsertObj( sal_uInt16 eObj );
    sal_uInt16          GetInsertObj() const;
    void            CreateDefaultObject();
    DlgEdMode       GetMode() const { return eMode; }
    sal_Bool            IsCreateOK() const { return bCreateOK; }

    void            Cut();
    void            Copy();
    void            Paste();
    void            Delete();
    sal_Bool            IsPasteAllowed();

    void            ShowProperties();
    void            UpdatePropertyBrowserDelayed();

    sal_Int32       countPages( Printer* pPrinter );
    void            printPage( sal_Int32 nPage, Printer* pPrinter, const String& );

    bool            AdjustPageSize();

    bool            isInPaint() const { return mnPaintGuard > 0; }
};

#endif //_BASCTL_DLGED_HXX
