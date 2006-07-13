/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: radiobtnbox.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-13 11:58:38 $
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
#ifndef _SVX_RADIOBTNBOX_HXX
#define _SVX_RADIOBTNBOX_HXX

// include ---------------------------------------------------------------

#ifndef _SVX_SIMPTABL_HXX
#include "simptabl.hxx"
#endif
#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

namespace svx {

// class SvxRadioButtonListBox -------------------------------------------

class SVX_DLLPUBLIC SvxRadioButtonListBox : public SvxSimpleTable
{
private:
    Point               m_aCurMousePoint;

protected:
    virtual void        SetTabs();
    virtual void        MouseButtonUp( const MouseEvent& _rMEvt );
    virtual void        KeyInput( const KeyEvent& rKEvt );

public:
    SvxRadioButtonListBox( Window* _pParent, const ResId& _rId );
    ~SvxRadioButtonListBox();

    void                HandleEntryChecked( SvLBoxEntry* _pEntry );

    const Point&        GetCurMousePoint() const;
};

} // end of namespace ::svx

#endif // #ifndef _SVX_RADIOBTNBOX_HXX

