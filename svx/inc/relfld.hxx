/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: relfld.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 18:23:26 $
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
#ifndef _SVX_RELFLD_HXX
#define _SVX_RELFLD_HXX

// include ---------------------------------------------------------------

#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif


// class SvxRelativeField ------------------------------------------------
/*
    [Beschreibung]

    "Ahnlich der Klasse FontSizeBox. Abgeleitet von der Klasse MetricField.
    Zus"atzliche Funktionalit"at: relative Angaben.
*/

class SVX_DLLPUBLIC SvxRelativeField : public MetricField
{
private:
    USHORT          nRelMin;
    USHORT          nRelMax;
    USHORT          nRelStep;
    BOOL            bRelativeMode;
    BOOL            bRelative;
    BOOL            bNegativeEnabled;

protected:
    void            Modify();

public:
    SvxRelativeField( Window* pParent, WinBits nWinStyle = 0 );
    SvxRelativeField( Window* pParent, const ResId& rResId );

    void            EnableRelativeMode( USHORT nMin = 50, USHORT nMax = 150,
                                        USHORT nStep = 5 );
    BOOL            IsRelativeMode() const { return bRelativeMode; }
    void            SetRelative( BOOL bRelative = FALSE );
    BOOL            IsRelative() const { return bRelative; }
    void            EnableNegativeMode() {bNegativeEnabled = TRUE;}
};


#endif

