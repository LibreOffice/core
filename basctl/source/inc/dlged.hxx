/*************************************************************************
 *
 *  $RCSfile: dlged.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: vg $ $Date: 2003-03-26 12:51:00 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _BASCTL_DLGED_HXX
#define _BASCTL_DLGED_HXX

#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_DATATRANSFER_DATAFLAVOR_HPP_
#include <com/sun/star/datatransfer/DataFlavor.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATSSUPPLIER_HPP_
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#endif

#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif
#ifndef _SV_GEN_HXX
#include <vcl/gen.hxx>
#endif
#ifndef _SV_TIMER_HXX
#include <vcl/timer.hxx>
#endif
#ifndef _SFXHINT_HXX
#include <svtools/hint.hxx>
#endif
#ifndef _SFXBRDCST_HXX
#include <svtools/brdcst.hxx>
#endif


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
    TYPEINFO();
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

protected:
    ScrollBar*          pHScroll;
    ScrollBar*          pVScroll;
    DlgEdModel*         pDlgEdModel;
    DlgEdPage*          pDlgEdPage;
    DlgEdView*          pDlgEdView;
    DlgEdForm*          pDlgEdForm;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >     m_xUnoControlDialogModel;
    ::com::sun::star::uno::Sequence< ::com::sun::star::datatransfer::DataFlavor >       m_ClipboardDataFlavors;
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >  m_xSupplier;
    DlgEdFactory*       pObjFac;
    Window*             pWindow;
    DlgEdFunc*          pFunc;
    DlgEdMode           eMode;
    USHORT              eActObj;
    BOOL                bFirstDraw;
    Size                aGridSize;
    BOOL                bGridVisible;
    BOOL                bGridSnap;
    BOOL                bCreateOK;
    Timer               aPaintTimer;
    Rectangle           aPaintRect;
    BOOL                bDialogModelChanged;

public:
    DlgEditor();
    ~DlgEditor();

    void            SetWindow( Window* pWindow );
    Window*         GetWindow() const { return pWindow; }

    void            SetDlgEdForm( DlgEdForm* pForm ) { pDlgEdForm = pForm; }
    DlgEdForm*      GetDlgEdForm() const { return pDlgEdForm; }

    void            SetScrollBars( ScrollBar* pHScroll, ScrollBar* pVScroll );
    ScrollBar*      GetHScroll() const { return pHScroll; }
    ScrollBar*      GetVScroll() const { return pVScroll; }
    void            DoScroll( ScrollBar* pActScroll );
    void            UpdateScrollBars();

    void            SetDialog( ::com::sun::star::uno::Reference<
                        ::com::sun::star::container::XNameContainer > xUnoControlDialogModel );
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > GetDialog() const
                        {return m_xUnoControlDialogModel;}

    ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier > const & GetNumberFormatsSupplier();

    DlgEdModel*     GetModel()      const { return pDlgEdModel; }
    DlgEdView*      GetView()       const { return pDlgEdView; }
    DlgEdPage*      GetPage()       const { return pDlgEdPage; }

    void            ShowDialog();

    BOOL            UnmarkDialog();
    BOOL            RemarkDialog();

    void            SetDialogModelChanged( BOOL bChanged = TRUE ) { bDialogModelChanged = bChanged; }
    BOOL            IsDialogModelChanged() const { return bDialogModelChanged; }

    BOOL            IsModified() const;
    void            ClearModifyFlag();

    void            MouseButtonDown( const MouseEvent& rMEvt );
    void            MouseButtonUp( const MouseEvent& rMEvt );
    void            MouseMove( const MouseEvent& rMEvt );
    void            Paint( const Rectangle& rRect );
    BOOL            KeyInput( const KeyEvent& rKEvt );

    void            SetMode( DlgEdMode eMode );
    void            SetInsertObj( USHORT eObj );
    USHORT          GetInsertObj() const;
    void            CreateDefaultObject();
    DlgEdMode       GetMode() const { return eMode; }
    BOOL            IsCreateOK() const { return bCreateOK; }

    void            Cut();
    void            Copy();
    void            Paste();
    void            Delete();
    BOOL            IsPasteAllowed();

    void            ShowProperties();

    void            PrintData( Printer*, const String& rTitle );    // not working yet
};

#endif //_BASCTL_DLGED_HXX
