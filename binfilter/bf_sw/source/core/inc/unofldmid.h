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
#ifndef SW_UNOFLDMID_H
#define SW_UNOFLDMID_H

class String;
namespace com { namespace sun { namespace star { namespace uno {
    class Any;
} } } }

/******************************************************************************
 *
 ******************************************************************************/
//Abbildung der Properties auf den Descriptor
#define FIELD_PROP_PAR1   			10
#define FIELD_PROP_PAR2             11
#define FIELD_PROP_PAR3             12
#define FIELD_PROP_FORMAT           13
#define FIELD_PROP_SUBTYPE          14
#define FIELD_PROP_BOOL1			15
#define FIELD_PROP_BOOL2			16
#define FIELD_PROP_DATE				17
#define FIELD_PROP_USHORT1          18
#define FIELD_PROP_USHORT2          19
#define FIELD_PROP_BYTE1			20
#define FIELD_PROP_DOUBLE           21
#define FIELD_PROP_BOOL3			22
#define FIELD_PROP_PAR4             23
#define FIELD_PROP_SHORT1           24
#define FIELD_PROP_DATE_TIME		25
#define FIELD_PROP_PROP_SEQ			26
#define FIELD_PROP_LOCALE			27
#define FIELD_PROP_BOOL4            28
#define FIELD_PROP_STRINGS          29

String& GetString( const com::sun::star::uno::Any&, String& rStr );

#endif
