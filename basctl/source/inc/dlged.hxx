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

#ifndef BASCTL_DLGED_HXX
#define BASCTL_DLGED_HXX

#include <com/sun/star/awt/XControlContainer.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/datatransfer/DataFlavor.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <svl/brdcst.hxx>
#include <svl/hint.hxx>
#include <tools/gen.hxx>
#include <vcl/timer.hxx>

class ScrollBar;
class Printer;
class KeyEvent;
class MouseEvent;
class Timer;
class Window;

namespace basctl
{

#define DLGED_PAGE_WIDTH_MIN    1280
#define DLGED_PAGE_HEIGHT_MIN   1024

//============================================================================
// DlgEdHint
//============================================================================

class DlgEdObj;

class DlgEdHint: public SfxHint
{
public:
    enum Kind {
        UNKNOWN,
        WINDOWSCROLLED,
        LAYERCHANGED,
        OBJORDERCHANGED,
        SELECTIONCHANGED,
    };

private:
    Kind       eKind;
    DlgEdObj*  pDlgEdObj;

public:
    TYPEINFO();
    DlgEdHint (Kind);
    DlgEdHint (Kind, DlgEdObj* pObj);
    virtual ~DlgEdHint();

    Kind       GetKind() const { return eKind; }
    DlgEdObj*  GetObject() const { return pDlgEdObj; }
};


//============================================================================
// DlgEditor
//============================================================================

class DlgEdModel;
class DlgEdPage;
class DlgEdView;
class DlgEdForm;
class DlgEdFactory;
class DlgEdFunc;

class DlgEditor: public SfxBroadcaster
{
public:
    enum Mode {
        INSERT,
        SELECT,
        TEST,
        READONLY,
    };

private:
    DECL_LINK(PaintTimeout, void *);
    DECL_LINK(MarkTimeout, void *);

    void Print( Printer* pPrinter, const ::rtl::OUString& rTitle );

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
    Mode                eMode;
    sal_uInt16          eActObj;
    bool                bFirstDraw;
    Size                aGridSize;
    bool                bGridVisible;
    bool                bGridSnap;
    bool                bCreateOK;
    Timer               aPaintTimer;
    Rectangle           aPaintRect;
    bool                bDialogModelChanged;
    Timer               aMarkTimer;
    long                mnPaintGuard;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > m_xDocument;

    DlgEditor(); // not implemented
public:
    DlgEditor( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xModel );
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

    bool            UnmarkDialog();
    bool            RemarkDialog();

    void            SetDialogModelChanged (bool bChanged = true) { bDialogModelChanged = bChanged; }
    bool            IsDialogModelChanged () const { return bDialogModelChanged; }

    bool            IsModified () const;
    void            ClearModifyFlag();

    void            MouseButtonDown( const MouseEvent& rMEvt );
    void            MouseButtonUp( const MouseEvent& rMEvt );
    void            MouseMove( const MouseEvent& rMEvt );
    void            Paint( const Rectangle& rRect );
    bool            KeyInput( const KeyEvent& rKEvt );

    void            SetMode (Mode eMode);
    void            SetInsertObj( sal_uInt16 eObj );
    sal_uInt16      GetInsertObj() const;
    void            CreateDefaultObject();
    Mode            GetMode() const { return eMode; }
    bool            IsCreateOK() const { return bCreateOK; }

    void            Cut();
    void            Copy();
    void            Paste();
    void            Delete();
    bool            IsPasteAllowed();

    void            ShowProperties();
    void            UpdatePropertyBrowserDelayed();

    sal_Int32       countPages( Printer* pPrinter );
    void            printPage( sal_Int32 nPage, Printer* pPrinter, const ::rtl::OUString& );

    bool            AdjustPageSize();

    bool            isInPaint() const { return mnPaintGuard > 0; }
};

} // namespace basctl

#endif // BASCTL_DLGED_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
