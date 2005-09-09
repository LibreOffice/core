/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: refltype.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 05:12:21 $
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

#ifndef _REGISTRY_REFLTYPE_HXX_
#define _REGISTRY_REFLTYPE_HXX_

#include "registry/types.h"

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

/** specifies the type source of a binary type blob.

    Currently only RT_UNO_IDL type is used.
 */
enum RTTypeSource
{
    RT_UNO_IDL,
    RT_CORBA_IDL,
    RT_JAVA
};

/** specifies a helper class for const values.

    This class is used for easy handling of constants or enum values
    as fields in binary type blob.
 */
class RTConstValue
{
public:
    /// stores the type of the constant value.
    RTValueType         m_type;
    /// stores the value of the constant.
    RTConstValueUnion m_value;

    /// Default constructor.
    RTConstValue()
        : m_type(RT_TYPE_NONE)
    {
        m_value.aDouble = 0.0;
    }

    /// Destructor
    ~RTConstValue() {}
};

/** deprecated.

    An earlier version of UNO used an unique identifier for interfaces. In the
    current version of UNO this uik was eliminated and this type is not longer used.
 */
struct RTUik
{
    sal_uInt32 m_Data1;
    sal_uInt16 m_Data2;
    sal_uInt16 m_Data3;
    sal_uInt32 m_Data4;
    sal_uInt32 m_Data5;
};

/// specifies the calling onvention for type reader/wrter api
#define TYPEREG_CALLTYPE    SAL_CALL

#endif
