/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: stdctrl.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-26 10:31:32 $
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

#ifndef _STDCTRL_HXX
#define _STDCTRL_HXX

#ifndef INCLUDED_SVTDLLAPI_H
#include "svtools/svtdllapi.h"
#endif

#ifndef _EDIT_HXX
#include <vcl/edit.hxx>
#endif
#ifndef _FIXED_HXX
#include <vcl/fixed.hxx>
#endif

// -------------
// - FixedInfo -
// -------------

class SVT_DLLPUBLIC FixedInfo : public FixedText
{
public:
    FixedInfo( Window* pParent, WinBits nWinStyle = WB_LEFT );
    FixedInfo( Window* pParent, const ResId& rResId );
};

namespace svt
{
    // ----------------------------
    // - svt::SelectableFixedText -
    // ----------------------------

    class SVT_DLLPUBLIC SelectableFixedText : public Edit
    {
    private:
        void    Init();

    public:
                SelectableFixedText( Window* pParent, WinBits nWinStyle );
                SelectableFixedText( Window* pParent, const ResId& rResId );
        virtual ~SelectableFixedText();

        virtual void    LoseFocus();
    };

} // namespace svt

#endif  // _STDCTRL_HXX

