/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: zoomlist.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 17:54:15 $
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

#ifndef SD_ZOOM_LIST_HXX
#define SD_ZOOM_LIST_HXX


#ifndef _GEN_HXX //autogen
#include <tools/gen.hxx>
#endif

#ifndef _LIST_HXX //autogen
#include <tools/list.hxx>
#endif


namespace sd {

class ViewShell;

class ZoomList : public List
{
public:
    ZoomList(ViewShell* pViewShell);
    virtual ~ZoomList();

    void        InsertZoomRect(const Rectangle& rRect);
    Rectangle   GetCurrentZoomRect() const;
    Rectangle   GetNextZoomRect();
    Rectangle   GetPreviousZoomRect();
    BOOL        IsNextPossible() const;
    BOOL        IsPreviousPossible() const;

private:
    ViewShell*  mpViewShell;
    ULONG       mnCurPos;
};

} // end of namespace sd

#endif


