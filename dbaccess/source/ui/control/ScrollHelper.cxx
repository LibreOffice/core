/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ScrollHelper.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2005-09-23 12:23:36 $
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

#ifndef DBAUI_SCROLLHELPER_HXX
#include "ScrollHelper.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#define LISTBOX_SCROLLING_AREA  12
namespace dbaui
{
DBG_NAME(OScrollHelper)

// -----------------------------------------------------------------------------


    OScrollHelper::OScrollHelper()
    {
        DBG_CTOR(OScrollHelper,NULL);
    }
    // -----------------------------------------------------------------------------
    OScrollHelper::~OScrollHelper()
    {

        DBG_DTOR(OScrollHelper,NULL);
    }
    // -----------------------------------------------------------------------------
    void OScrollHelper::scroll(const Point& _rPoint, const Size& _rOutputSize)
    {
        // Scrolling Areas
        Rectangle aScrollArea( Point(0, _rOutputSize.Height() - LISTBOX_SCROLLING_AREA),
                                     Size(_rOutputSize.Width(), LISTBOX_SCROLLING_AREA) );

        Link aToCall;
        // if pointer in bottom area begin scroll
        if( aScrollArea.IsInside(_rPoint) )
            aToCall = m_aUpScroll;
        else
        {
            aScrollArea.SetPos(Point(0,0));
            // if pointer in top area begin scroll
            if( aScrollArea.IsInside(_rPoint) )
                aToCall = m_aDownScroll;
        }
        if ( aToCall.IsSet() )
            aToCall.Call( NULL );
    }
    // -----------------------------------------------------------------------------
}
// -----------------------------------------------------------------------------


