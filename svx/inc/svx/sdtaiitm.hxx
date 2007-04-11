/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdtaiitm.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:13:37 $
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
#ifndef _SDTAIITM_HXX
#define _SDTAIITM_HXX

#ifndef _SDYNITM_HXX
#include <svx/sdynitm.hxx>
#endif

#ifndef _SVDDEF_HXX //autogen
#include <svx/svddef.hxx>
#endif

class SdrTextAniStartInsideItem: public SdrYesNoItem {
public:
    SdrTextAniStartInsideItem(BOOL bOn=FALSE): SdrYesNoItem(SDRATTR_TEXT_ANISTARTINSIDE,bOn) {}
    SdrTextAniStartInsideItem(SvStream& rIn) : SdrYesNoItem(SDRATTR_TEXT_ANISTARTINSIDE,rIn) {}
};

class SdrTextAniStopInsideItem: public SdrYesNoItem {
public:
    SdrTextAniStopInsideItem(BOOL bOn=FALSE): SdrYesNoItem(SDRATTR_TEXT_ANISTOPINSIDE,bOn) {}
    SdrTextAniStopInsideItem(SvStream& rIn) : SdrYesNoItem(SDRATTR_TEXT_ANISTOPINSIDE,rIn) {}
};

#endif
