/*************************************************************************
 *
 *  $RCSfile: refltype.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2000-10-09 14:48:24 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

/*************************************************************************

    Source Code Control System - Update

    $Log: not supported by cvs2svn $
    Revision 1.2  2000/10/09 11:53:09  jsc
    change the binary typelibrary format, make it extendable

    Revision 1.1.1.1  2000/09/18 15:18:42  hr
    initial import

    Revision 1.19  2000/09/17 12:30:32  willem.vandorp
    OpenOffice header added.

    Revision 1.18  2000/08/04 15:31:39  jsc
    extend biniary type format

    Revision 1.17  2000/07/26 17:44:32  willem.vandorp
    Headers/footers replaced

    Revision 1.16  2000/03/24 15:00:36  jsc
    modified for unicode changes

    Revision 1.15  2000/02/08 16:01:09  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.14  1999/10/22 09:58:47  jsc
    TYPEREG_CALLTYPE = SAL_CALL

    Revision 1.13  1999/10/21 15:48:50  jsc
    optional flag now in registry

    Revision 1.12  1999/10/15 11:05:51  dbo
    short RTConstValue from sal_uInt16 to sal_Int16

    Revision 1.11  1999/07/02 09:25:40  jsc
    #67179# Binaerblop der CoreReflection ist erweitert worden

    Revision 1.10  1999/03/12 14:16:38  jsc
    #61011# Anpassung an osl-Typen

    Revision 1.9  1998/07/03 12:36:16  ts
    added SERVICE and OBJECT

    Revision 1.8  1998/07/03 11:20:07  ts
    prefixen von typen

    Revision 1.7  1998/07/03 10:55:41  ts
    added RT_TYPE_TYPEDEF

    Revision 1.6  1998/07/03 08:41:02  ts
    bugs und set/getUik()

    Revision 1.5  1998/07/02 11:58:47  jsc
    Typ Anpassung, Schnittstelle frei von solar.h

    Revision 1.4  1998/07/01 16:55:19  ts
    alignment unter soaris beachten

    Revision 1.3  1998/07/01 13:39:01  ts
    bugs, bugs, bugs

    Revision 1.2  1998/06/30 13:42:27  ts
    Anpassung an SOLARIS

    Revision 1.1.1.1  1998/06/30 11:17:45  jsc
    neu


*************************************************************************/

#ifndef __REGISTRY_REFLTYPE_HXX__
#define __REGISTRY_REFLTYPE_HXX__

#ifdef SOLARIS
#include <wchar.h>
#else
#include <stdlib.h>
#endif

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

enum RTTypeSource
{
    RT_UNO_IDL,
    RT_CORBA_IDL,
    RT_JAVA
};

enum RTTypeClass
{
    RT_TYPE_INVALID,
    RT_TYPE_INTERFACE,
    RT_TYPE_MODULE,
    RT_TYPE_STRUCT,
    RT_TYPE_ENUM,
    RT_TYPE_EXCEPTION,
    RT_TYPE_TYPEDEF,
    RT_TYPE_SERVICE,
    RT_TYPE_OBJECT,
    RT_TYPE_CONSTANTS
};

typedef sal_uInt16 RTFieldAccess;

#define RT_ACCESS_INVALID           0x0000
#define RT_ACCESS_READONLY          0x0001
#define RT_ACCESS_OPTIONAL          0x0002
#define RT_ACCESS_MAYBEVOID         0x0004
#define RT_ACCESS_BOUND             0x0008
#define RT_ACCESS_CONSTRAINED       0x0010
#define RT_ACCESS_TRANSIENT         0x0020
#define RT_ACCESS_MAYBEAMBIGUOUS    0x0040
#define RT_ACCESS_MAYBEDEFAULT      0x0080
#define RT_ACCESS_REMOVEABLE        0x0100
#define RT_ACCESS_ATTRIBUTE         0x0200
#define RT_ACCESS_PROPERTY          0x0400
#define RT_ACCESS_CONST             0x0800
#define RT_ACCESS_READWRITE         0x1000
/*
enum RTFieldAccess
{
    RT_ACCESS_INVALID,
    RT_ACCESS_CONST,
    RT_ACCESS_READONLY,
    RT_ACCESS_WRITEONLY,
    RT_ACCESS_READWRITE,
    RT_ACCESS_READONLY_OPTIONAL,
    RT_ACCESS_WRITEONLY_OPTIONAL,
    RT_ACCESS_READWRITE_OPTIONAL
};
*/
enum RTReferenceType
{
    RT_REF_INVALID,
    RT_REF_SUPPORTS,
    RT_REF_OBSERVES,
    RT_REF_EXPORTS,
    RT_REF_NEEDS
};

enum RTMethodMode
{
    RT_MODE_INVALID,
    RT_MODE_ONEWAY,
    RT_MODE_ONEWAY_CONST,
    RT_MODE_TWOWAY,
    RT_MODE_TWOWAY_CONST
};

enum RTParamMode
{
    RT_PARAM_INVALID,
    RT_PARAM_IN,
    RT_PARAM_OUT,
    RT_PARAM_INOUT
};

enum RTValueType
{
    RT_TYPE_NONE,
    RT_TYPE_BOOL,
    RT_TYPE_BYTE,
    RT_TYPE_INT16,
    RT_TYPE_UINT16,
    RT_TYPE_INT32,
    RT_TYPE_UINT32,
    RT_TYPE_INT64,
    RT_TYPE_UINT64,
    RT_TYPE_FLOAT,
    RT_TYPE_DOUBLE,
    RT_TYPE_STRING
};

union RTConstValueUnion
{
    sal_Bool        aBool;
    sal_uInt8       aByte;
    sal_Int16       aShort;
    sal_uInt16      aUShort;
    sal_Int32       aLong;
    sal_uInt32      aULong;
//  sal_Int64       aHyper;
//  sal_UInt64      aUHyper;
    float           aFloat;
    double          aDouble;
    const sal_Unicode*  aString;
};

class RTConstValue
{
public:
    RTValueType         m_type;
    RTConstValueUnion m_value;

    RTConstValue()
        : m_type(RT_TYPE_NONE)
    {
        m_value.aDouble = 0.0;
    }

    ~RTConstValue() {};
};

struct RTUik
{
    sal_uInt32 m_Data1;
    sal_uInt16 m_Data2;
    sal_uInt16 m_Data3;
    sal_uInt32 m_Data4;
    sal_uInt32 m_Data5;
};

#define TYPEREG_CALLTYPE    SAL_CALL

#endif
