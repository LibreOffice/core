/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: optHeaderTabListbox.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ihi $ $Date: 2006-12-20 14:11:57 $
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
#ifndef SVX_OPTHEADERTABLISTBOX_HXX
#define SVX_OPTHEADERTABLISTBOX_HXX

#ifndef _SVTABBX_HXX //autogen
#include <svtools/svtabbx.hxx>
#endif

namespace svx
{
    // class OptHeaderTabListBox ---------------------------------------------

    class OptHeaderTabListBox : public SvHeaderTabListBox
    {
    public:
        OptHeaderTabListBox( Window* pParent, WinBits nBits );

        virtual void    InitEntry( SvLBoxEntry*, const XubString&, const Image&, const Image&, SvLBoxButtonKind );
    };
} // svx
#endif // SVX_OPTHEADERTABLISTBOX_HXX

