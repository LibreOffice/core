/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: invmerge.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 15:54:11 $
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

#ifndef SC_INVMERGE_HXX
#define SC_INVMERGE_HXX

#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif

#include <vector>

class Window;

class ScInvertMerger
{
private:
    Window*     pWin;
    ::std::vector< Rectangle >* pRects;
    Rectangle   aTotalRect;
    Rectangle   aLineRect;

    void        FlushLine();
    void        FlushTotal();

public:
                ScInvertMerger( Window* pWindow );
                ScInvertMerger( ::std::vector< Rectangle >* pRectangles );
                ~ScInvertMerger();

    void        AddRect( const Rectangle& rRect );
    void        Flush();
};



#endif


