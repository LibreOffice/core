/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tooltiplbox.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 14:10:00 $
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

#ifndef SVTOOLS_TOOLTIPLBOX_HXX
#define SVTOOLS_TOOLTIPLBOX_HXX

#ifndef INCLUDED_SVTDLLAPI_H
#include "svtools/svtdllapi.h"
#endif

#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif

namespace svtools {

// ============================================================================

/** ListBox with tool tips for long entries. */
class ToolTipListBox : public ListBox
{
public:
                                ToolTipListBox( Window* pParent, WinBits nStyle = WB_BORDER );
                                ToolTipListBox( Window* pParent, const ResId& rResId );

protected:
    virtual void                RequestHelp( const HelpEvent& rHEvt );
};

// ----------------------------------------------------------------------------

/** MultiListBox with tool tips for long entries. */
class SVT_DLLPUBLIC ToolTipMultiListBox : public MultiListBox
{
public:
                                ToolTipMultiListBox( Window* pParent, WinBits nStyle = WB_BORDER );
                                ToolTipMultiListBox( Window* pParent, const ResId& rResId );

protected:
    virtual void                RequestHelp( const HelpEvent& rHEvt );
};

// ============================================================================

} // namespace svtools

#endif

