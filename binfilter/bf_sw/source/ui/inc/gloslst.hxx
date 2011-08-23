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

#ifndef _GLOSLST_HXX
#define _GLOSLST_HXX


#ifndef _DATETIME_HXX
#include <tools/datetime.hxx>
#endif
#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef _TIMER_HXX //autogen
#include <vcl/timer.hxx>
#endif
#ifndef _SVARRAY_HXX //autogen
#include <bf_svtools/svarray.hxx>
#endif
class SvStringsISortDtor; 
namespace binfilter {

class SwGlossaries;


struct AutoTextGroup
{
    USHORT 		nCount;
    String 		sName;
    String  	sTitle;
    String		sLongNames;   // durch 0x0A getrennte Langnamen
    String 		sShortNames;  // durch 0x0A getrennte Kurznamen
    DateTime	aDateModified;
};


typedef AutoTextGroup* AutoTextGroupPtr;
SV_DECL_PTRARR(AutoTextGroups, AutoTextGroupPtr, 4, 4)//STRIP008 ;

class SwGlossaryList : public AutoTimer
{
    AutoTextGroups	aGroupArr;
    String			sPath;
    BOOL 			bFilled;
};

} //namespace binfilter
#endif






