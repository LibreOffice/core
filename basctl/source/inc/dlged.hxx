/*************************************************************************
 *
 *  $RCSfile: dlged.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ab $ $Date: 2001-03-03 14:36:55 $
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

#ifndef _BASCTL_DLGEDPAGE_HXX
#include <dlgedpage.hxx>
#endif

#ifndef _BASCTL_DLGEDMOD_HXX //autogen
#include <dlgedmod.hxx>
#endif

#ifndef _BASCTL_DLGEDOBJ_HXX
#include <dlgedobj.hxx>
#endif

#ifndef _CLIP_HXX //autogen
#include <vcl/clip.hxx>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif

enum VCDlgMode { VCDLGED_INSERT, VCDLGED_SELECT, VCDLGED_TEST, VCDLGED_BLOCK_PAINT };

//============================================================================
// VCDlgEditor
//============================================================================

class DlgEdForm;
class DlgEdFunc;
class VCDlgEditFactory;
class SdrModel;
class SdrPage;
class SdrView;
class StarBASIC;
//class VCDlgEditor;
class ScrollBar;
class Printer;

class VCDlgEditor /* : public LinkHdl */
{
//#if __PRIVATE
    DECL_LINK( ClipboardCleared, Clipboard * );
    DECL_LINK( PaintTimeout, Timer * );
//#endif
protected:
    ScrollBar*          pHScroll;
    ScrollBar*          pVScroll;
    StarBASIC*          pBasic;
    DlgEdModel*         pSdrModel;
    DlgEdPage*          pSdrPage;
    SdrView*            pSdrView;
    DlgEdForm*          pDlgEdForm;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > m_xUnoControlDialogModel;
    VCDlgEditFactory*   pObjFac;
    Window*             pWindow;
    DlgEdFunc*          pFunc;
    VCDlgMode           eMode;
    USHORT              eActObj;
    BOOL                bFirstDraw;
    Size                aGridSize;
    BOOL                bGridVisible;
    BOOL                bGridSnap;
    Clipboard           aClip;
    BOOL                bClipPrivate;
    BOOL                bCreateOK;
    Timer               aPaintTimer;
    Rectangle           aPaintRect;

    // Data for new library container mechanism

public:
    VCDlgEditor( StarBASIC* pBasic );
    ~VCDlgEditor();

    void            SetWindow( Window* pWindow );
    Window*         GetWindow() const { return pWindow; }

    void            SetDlgEdForm( DlgEdForm* pForm ) { pDlgEdForm = pForm; }
    DlgEdForm*      GetDlgEdForm() const { return pDlgEdForm; }

    void            SetScrollBars( ScrollBar* pHScroll, ScrollBar* pVScroll );
    ScrollBar*      GetHScroll() const { return pHScroll; }
    ScrollBar*      GetVScroll() const { return pVScroll; }
    void            DoScroll( ScrollBar* pActScroll );

    void            SetDialog( ::com::sun::star::uno::Reference<
                        ::com::sun::star::container::XNameContainer > xUnoControlDialogModel );
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > GetDialog() const
                        {return m_xUnoControlDialogModel;}

    StarBASIC*      GetBasic() const;

    SdrModel*       GetModel()      const { return pSdrModel; }
    SdrView*        GetView()       const { return pSdrView; }
    SdrPage*        GetPage()       const { return pSdrPage; }

    void            ShowDialog();

    BOOL            UnmarkDialog();
    void            RemarkDialog();

    BOOL            IsModified() const;
    void            ClearModifyFlag();

    void            MouseButtonDown( const MouseEvent& rMEvt );
    void            MouseButtonUp( const MouseEvent& rMEvt );
    void            MouseMove( const MouseEvent& rMEvt );
    void            Paint( const Rectangle& rRect );
    BOOL            KeyInput( const KeyEvent& rKEvt );

    void            SetMode( VCDlgMode eMode );
    void            SetInsertObj( USHORT eObj );
    USHORT          GetInsertObj() const;
    VCDlgMode       GetMode() const { return eMode; }
    BOOL            IsCreateOK() const { return bCreateOK; }

    void            Cut();
    void            Copy();
    void            Paste();
    void            Delete();

    void            PrintData( Printer*, const String& rTitle );
};

#endif //_BASCTL_DLGED_HXX
