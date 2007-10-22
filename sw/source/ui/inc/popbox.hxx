/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: popbox.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: vg $ $Date: 2007-10-22 15:22:15 $
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
#ifndef _POPBOX_HXX
#define _POPBOX_HXX


#ifndef _TOOLBOX_HXX
#include <vcl/toolbox.hxx>
#endif
#ifndef _TRANSFER_HXX
#include <svtools/transfer.hxx>
#endif

class SwNavigationPI;

class SwHelpToolBox: public ToolBox, public DropTargetHelper
{
    Link aDoubleClickLink;
    Link aRightClickLink;       // Link bekommt MouseEvent als Parameter !!!

    using ToolBox::DoubleClick;

protected:
    virtual void MouseButtonDown(const MouseEvent &rEvt);
    virtual long DoubleClick(ToolBox *);
    virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt );
    virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& rEvt );

public:
    SwHelpToolBox(SwNavigationPI* pParent, const ResId &);
    ~SwHelpToolBox();
    void SetDoubleClickLink(const Link &);      // inline
    void SetRightClickLink(const Link &);       // inline
};

inline void SwHelpToolBox::SetDoubleClickLink(const Link &rLink) {
    aDoubleClickLink = rLink;
}

inline void SwHelpToolBox::SetRightClickLink(const Link &rLink) {
    aRightClickLink = rLink;
}


#endif
