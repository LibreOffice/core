/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fupage.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 05:37:08 $
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

#ifndef SD_FU_PAGE_HXX
#define SD_FU_PAGE_HXX

#ifndef SD_FU_POOR_HXX
#include "fupoor.hxx"
#endif

class SfxItemSet;
class SdBackgroundObjUndoAction;
class SdPage;

namespace sd {
class DrawViewShell;

class FuPage
    : public FuPoor
{
 public:
    TYPEINFO();

    FuPage (
        ViewShell* pViewSh,
        ::sd::Window* pWin,
        ::sd::View* pView,
        SdDrawDocument* pDoc,
        SfxRequest& rReq );
    virtual ~FuPage (void);

    virtual void Activate();           // Function aktivieren
    virtual void Deactivate();         // Function deaktivieren

    const SfxItemSet* ExecuteDialog( Window* pParent );
    void ApplyItemSet( const SfxItemSet* pArgs );

private:
    SfxRequest&                 mrReq;
    const SfxItemSet*           mpArgs;
    SdBackgroundObjUndoAction*  mpBackgroundObjUndoAction;
    Size                        maSize;
    bool                        mbPageBckgrdDeleted;
    bool                        mbMasterPage;
    bool                        mbDisplayBackgroundTabPage;
    SdPage*                     mpPage;
    DrawViewShell*              mpDrawViewShell;
};

} // end of namespace sd

#endif

