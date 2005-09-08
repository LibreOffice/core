/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: iniadrtk.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 09:45:06 $
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

