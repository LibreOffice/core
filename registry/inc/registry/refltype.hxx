/*************************************************************************
 *
 *  $RCSfile: refltype.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: jsc $ $Date: 2002-11-11 15:20:20 $
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
    Revision 1.6  2001/11/15 18:01:32  jsc
    #88337# review docu

    Revision 1.5  2001/05/18 15:30:58  jsc
    #87233# insert enum value

    Revision 1.4  2001/03/14 09:37:08  jsc
    remove vos dependencies

    Revision 1.3  2000/10/09 14:48:24  hr
    #65293#: syntax

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

#ifndef _REGISTRY_REFLTYPE_HXX_
#define _REGISTRY_REFLTYPE_HXX_

#ifdef SOLARIS
#include <wchar.h>
#else
#include <stdlib.h>
#endif

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

/** specifies the typeclass of a binary type blob.

    The general structure of a binary type blob is always the same. It depends
    on the typeclass which parts of the blob are filled with data or not.
 */
enum RTTypeClass
{
    /** specifies that the structure of the given blob is unknown and can't be read.
     */
    RT_TYPE_INVALID,
    /** specifies that the blob represents an interface type. An interface blob can contain
        a base interface, attributes and methods.
     */
    RT_TYPE_INTERFACE,
    /** specifies that the blob represents a module type. A module blob can contain
        a base module and constant members (fields).
     */
    RT_TYPE_MODULE,
    /** specifies that the blob represents a struct type. A struct blob can contain
        a base struct and members (fields).
     */
    RT_TYPE_STRUCT,
    /** specifies that the blob represents an enum type. An enum blob can contain
        enum values which are accessible as fields.
     */
    RT_TYPE_ENUM,
    /** specifies that the blob represents an exception type. An exception blob can contain
        a base exception and members (fields).
     */
    RT_TYPE_EXCEPTION,
    /** specifies that the blob represents a typedef type. A typedef blob can contain
        a base type.
     */
    RT_TYPE_TYPEDEF,
    /** specifies that the blob represents a service type. A service blob can contain
        a base service, properties (fields), references to services or interfaces.
     */
    RT_TYPE_SERVICE,
    /** specifies that the blob represents a singleton type (a named object) which refers exactly
        one existing service.
     */
    RT_TYPE_SINGLETON,
    /// deprecated, not used.
    RT_TYPE_OBJECT,
    /** specifies that the blob represents a constants type. A constants blob can contain
        constant types as fields.
     */
    RT_TYPE_CONSTANTS,
    /** @deprecated
        a union type was evaluated but currently not supported.
    */
    RT_TYPE_UNION
};

/** specifies the type for the field access.

    Fields in a type blob are used for different types. Among others they were used
    for properties of services and these poperties can have several flags.
    @see RT_ACCESS_INVALID
    @see RT_ACCESS_READONLY
    @see RT_ACCESS_OPTIONAL
    @see RT_ACCESS_MAYBEVOID
    @see RT_ACCESS_BOUND
    @see RT_ACCESS_CONSTRAINED
    @see RT_ACCESS_TRANSIENT
    @see RT_ACCESS_MAYBEAMBIGUOUS
    @see RT_ACCESS_MAYBEDEFAULT
    @see RT_ACCESS_REMOVEABLE
    @see RT_ACCESS_ATTRIBUTE
    @see RT_ACCESS_PROPERTY
    @see RT_ACCESS_CONST
    @see RT_ACCESS_READWRITE
    @see RT_ACCESS_DEFAULT
 */
typedef sal_uInt16 RTFieldAccess;

/// specifies a unknown flag
#define RT_ACCESS_INVALID           0x0000
/// specifies a readonly property/attribute
#define RT_ACCESS_READONLY          0x0001
/// specifies a property as optional that means that it must not be implemented.
#define RT_ACCESS_OPTIONAL          0x0002
/// @see com::sun::star::beans::PropertyAttribute
#define RT_ACCESS_MAYBEVOID         0x0004
/// @see com::sun::star::beans::PropertyAttribute
#define RT_ACCESS_BOUND             0x0008
/// @see com::sun::star::beans::PropertyAttribute
#define RT_ACCESS_CONSTRAINED       0x0010
/// @see com::sun::star::beans::PropertyAttribute
#define RT_ACCESS_TRANSIENT         0x0020
/// @see com::sun::star::beans::PropertyAttribute
#define RT_ACCESS_MAYBEAMBIGUOUS    0x0040
/// @see com::sun::star::beans::PropertyAttribute
#define RT_ACCESS_MAYBEDEFAULT      0x0080
/// @see com::sun::star::beans::PropertyAttribute
#define RT_ACCESS_REMOVEABLE        0x0100
/// @see com::sun::star::beans::PropertyAttribute
#define RT_ACCESS_ATTRIBUTE         0x0200
/// specifies that the field is a property
#define RT_ACCESS_PROPERTY          0x0400
/// specifies that the field is a constant or enum value
#define RT_ACCESS_CONST             0x0800
/// specifies that the property/attribute has read/write access
#define RT_ACCESS_READWRITE         0x1000
/// only to describe a union default label
#define RT_ACCESS_DEFAULT           0x2000

/** specifies the type of a reference used in a service description.
 */
enum RTReferenceType
{
    /// the reference type is unknown
    RT_REF_INVALID,
    /** the service support the interface that means a implementation of this service
        must implement this interface.
     */
    RT_REF_SUPPORTS,
    /** @deprecated
        the service observes the interface.
     */
    RT_REF_OBSERVES,
    /** the service exports the specified service that means this service provides also
        the specified service.
     */
    RT_REF_EXPORTS,
    /** @deprecated
        the service needs the specified service that means in the context of this service
        the specified service will be used or must be available.
     */
    RT_REF_NEEDS
};

/** specifies the mode of a method.

    A method can be synchron or asynchron (oneway). The const attribute
    for methods was removed so that the const values are deprecated.
 */
enum RTMethodMode
{
    /// indicates an invalid mode
    RT_MODE_INVALID,
    /// indicates the asynchronous mode of a method
    RT_MODE_ONEWAY,
    /// @deprecated
    RT_MODE_ONEWAY_CONST,
    /// indicated the synchronous mode of a method
    RT_MODE_TWOWAY,
    /// @deprecated
    RT_MODE_TWOWAY_CONST
};

/** specifies the mode of a parameter.

    There are three paramter modes which have impact of the
    handling of the paramter in the UNO bridges and the UNO code
    generation.
 */
enum RTParamMode
{
    /// indicates an invalid parameter mode
    RT_PARAM_INVALID,
    /// indicates a pure in parameter which is used by value
    RT_PARAM_IN,
    /// indicates a pure out parameter which is used by reference
    RT_PARAM_OUT,
    /// indicates a in and out parameter which is used also by reference
    RT_PARAM_INOUT
};

/** specifies the type of a field value.

    A field can have a value if it repsresents a constant or an enum value.
 */
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

/** specifies a variable container for field values.
 */
union RTConstValueUnion
{
    sal_Bool        aBool;
    sal_uInt8       aByte;
    sal_Int16       aShort;
    sal_uInt16      aUShort;
    sal_Int32       aLong;
    sal_uInt32      aULong;
    sal_Int64       aHyper;
    sal_uInt64      aUHyper;
    float           aFloat;
    double          aDouble;
    const sal_Unicode*  aString;
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
