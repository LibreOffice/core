/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _SXONITM_HXX
#define _SXONITM_HXX

#ifndef _SVDDEF_HXX //autogen
#include <bf_svx/svddef.hxx>
#endif

#ifndef _SFXSTRITEM_HXX //autogen
#include <bf_svtools/stritem.hxx>
#endif
namespace binfilter {

//------------------------------
// class SdrObjectNameItem
//------------------------------

class SdrObjectNameItem: public SfxStringItem {
public:
    SdrObjectNameItem()                   : SfxStringItem() { SetWhich(SDRATTR_OBJECTNAME); }
    SdrObjectNameItem(const String& rStr) : SfxStringItem(SDRATTR_OBJECTNAME,rStr) {}
    SdrObjectNameItem(SvStream& rIn)      : SfxStringItem(SDRATTR_OBJECTNAME,rIn)  {}
};

}//end of namespace binfilter
#endif
