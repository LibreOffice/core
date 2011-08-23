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

#ifndef _EEITEM_HXX //autogen
#include <eeitem.hxx>
#endif


#ifndef _SVX_ITEMDATA_HXX
#include "itemdata.hxx"
#endif

#ifndef _SFXPOOLITEM_HXX
#include <bf_svtools/poolitem.hxx>
#endif

#ifndef _DATE_HXX
#include <tools/date.hxx>
#endif

#ifndef _TOOLS_TIME_HXX
#include <tools/time.hxx>
#endif

#include "svdfield.hxx"
#include "svdotext.hxx"
namespace binfilter {

// Do not remove this, it is still used in src536a!
/*N*/ void SdrRegisterFieldClasses()
/*N*/ {
/*N*/ 	SvxFieldItem::GetClassManager().SV_CLASS_REGISTER(SdrMeasureField);
/*N*/ }

///////////////////////////////////////////////////////////////////////////////////////////////// */

/*N*/ FASTBOOL SdrTextObj::CalcFieldValue(const SvxFieldItem& rField, USHORT nPara, USHORT nPos,
/*N*/ 	FASTBOOL bEdit,	Color*& rpTxtColor, Color*& rpFldColor, XubString& rRet) const
/*N*/ {
/*N*/ 	return FALSE;
/*N*/ }

}
