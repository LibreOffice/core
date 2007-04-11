/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sxmspitm.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:36:39 $
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
#ifndef _SXMSPITM_HXX
#define _SXMSPITM_HXX

#ifndef _SVDDEF_HXX //autogen
#include <svx/svddef.hxx>
#endif

#ifndef _SDYNITM_HXX
#include <svx/sdynitm.hxx>
#endif

//------------------------------
// class SdrMoveProtectItem
//------------------------------
class SdrObjMoveProtectItem: public SdrYesNoItem {
public:
    SdrObjMoveProtectItem(bool bOn=false): SdrYesNoItem(SDRATTR_OBJMOVEPROTECT,bOn) {}
    SdrObjMoveProtectItem(SvStream& rIn): SdrYesNoItem(SDRATTR_OBJMOVEPROTECT,rIn) {}
};

//------------------------------
// class SdrSizeProtectItem
//------------------------------
class SdrObjSizeProtectItem: public SdrYesNoItem {
public:
    SdrObjSizeProtectItem(bool bOn=false): SdrYesNoItem(SDRATTR_OBJSIZEPROTECT,bOn) {}
    SdrObjSizeProtectItem(SvStream& rIn): SdrYesNoItem(SDRATTR_OBJSIZEPROTECT,rIn) {}
};

#endif
