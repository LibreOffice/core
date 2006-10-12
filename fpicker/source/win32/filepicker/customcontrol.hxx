/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: customcontrol.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 10:51:16 $
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

#ifndef _CUSTOMCONTROL_HXX_
#define _CUSTOMCONTROL_HXX_

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

//-----------------------------------
//
//-----------------------------------

class CCustomControl
{
public:
    virtual ~CCustomControl();

    // align the control to a reference object/control
    virtual void SAL_CALL Align() = 0;

    virtual void SAL_CALL SetFont(HFONT hFont) = 0;

    virtual void SAL_CALL AddControl(CCustomControl* aCustomControl);
    virtual void SAL_CALL RemoveControl(CCustomControl* aCustomControl);
    virtual void SAL_CALL RemoveAllControls();
};

#endif
