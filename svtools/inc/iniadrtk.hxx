/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: iniadrtk.hxx,v $
 * $Revision: 1.3 $
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

#ifndef SVTOOLS_INIADRTK_HXX
#define SVTOOLS_INIADRTK_HXX

class String;

//============================================================================
enum AddressToken
{
    ADDRESS_COMPANY,
    ADDRESS_STREET,
    ADDRESS_COUNTRY,
    ADDRESS_PLZ,
    ADDRESS_CITY,
    ADDRESS_TITLE,
    ADDRESS_POSITION,
    ADDRESS_TEL_PRIVATE,
    ADDRESS_TEL_COMPANY,
    ADDRESS_FAX,
    ADDRESS_EMAIL,
    ADDRESS_STATE,
    ADDRESS_FATHERSNAME,
    ADDRESS_APARTMENT
};

//============================================================================
class SfxIniManagerAddressEntry
{
public:
    static String get(const String & rAddress, AddressToken eToken);
};

#endif // SVTOOLS_INIADRTK_HXX

