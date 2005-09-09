/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textcontrolcombo.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 10:09:33 $
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

#ifndef _TEXTCONTROLCOMBO_HXX
#define _TEXTCONTROLCOMBO_HXX

#ifndef _FIXED_HXX
#include <vcl/fixed.hxx>
#endif

#ifndef INCLUDED_SWDLLAPI_H
#include "swdllapi.h"
#endif

class SW_DLLPUBLIC TextControlCombo : public Window
{
private:
protected:
    Control&    mrCtrl;
    FixedText&  mrFTbefore;
    FixedText&  mrFTafter;
public:
                TextControlCombo( Window* _pParent, const ResId& _rResId,
                            Control& _rCtrl, FixedText& _rFTbefore, FixedText& _rFTafter );
    virtual     ~TextControlCombo();

    void        Arrange( FixedText& _rOrg, BOOL bShow = true );

    // identical to window functionality
    void        Show( BOOL bVisible = TRUE, USHORT nFlags = 0 );
    void        Hide( USHORT nFlags = 0 ) { Show( FALSE, nFlags ); }
    void        Enable( BOOL bEnable = TRUE, BOOL bChild = TRUE );
    void        Disable( BOOL bChild = TRUE ) { Enable( FALSE, bChild ); }
};


#endif
