/*************************************************************************
 *
 *  $RCSfile: ViewTabControl.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:05:36 $
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

#ifndef SD_TAB_CONTROL_HXX
#define SD_TAB_CONTROL_HXX

#ifndef _TABBAR_HXX //autogen wg. TabBar
#include <svtools/tabbar.hxx>
#endif
#ifndef _TRANSFER_HXX
#include <svtools/transfer.hxx>
#endif

namespace sd {

/*************************************************************************
|*
|* TabControl-Klasse fuer Seitenumschaltung
|*
\************************************************************************/

class DrawViewShell;

class TabControl
    : public TabBar,
      public DragSourceHelper,
      public DropTargetHelper
{
public:
    TabControl (DrawViewShell* pDrViewSh, ::Window* pParent);
    virtual ~TabControl (void);

    /** Inform all listeners of this control that the current page has been
        activated.  Call this method after switching the current page and is
        not done elsewhere (like when using page up/down keys).
    */
    void SendActivatePageEvent (void);

    /** Inform all listeners of this control that the current page has been
        deactivated.  Call this method before switching the current page and
        is not done elsewhere (like when using page up/down keys).
    */
    void SendDeactivatePageEvent (void);

protected:
    DrawViewShell*  pDrViewSh;
    BOOL                bInternalMove;

    // TabBar
    virtual void        Select();
    virtual void        DoubleClick();
    virtual void        MouseButtonDown(const MouseEvent& rMEvt);

    virtual void        Command(const CommandEvent& rCEvt);

    virtual long        StartRenaming();
    virtual long        AllowRenaming();
    virtual void        EndRenaming();

    virtual void        ActivatePage();
    virtual long        DeactivatePage();

    // DragSourceHelper
    virtual void        StartDrag( sal_Int8 nAction, const Point& rPosPixel );

    // DropTargetHelper
    virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt );
    virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& rEvt );

                        DECL_LINK(DeactivatePageHdl, void*);

private:
    // nested class to implement the TransferableHelper
    class TabControlTransferable : public TransferableHelper
    {
    public:
        TabControlTransferable( TabControl& rParent ) :
            mrParent( rParent ) {}
    private:

        TabControl&     mrParent;

        virtual             ~TabControlTransferable();

        virtual void        AddSupportedFormats();
        virtual sal_Bool    GetData( const ::com::sun::star::datatransfer::DataFlavor& rFlavor );
        virtual void        DragFinished( sal_Int8 nDropAction );

    };

    friend class TabControl::TabControlTransferable;

    void                DragFinished( sal_Int8 nDropAction );

};

} // end of namespace sd

#endif

