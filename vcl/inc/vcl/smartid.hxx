/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: smartid.hxx,v $
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

#ifndef _SMARTID_HXX_
#define _SMARTID_HXX_

#include <tools/string.hxx>
#include <vcl/dllapi.h>

/// SMART_SET_SMART only sets the Ids that are defined in the SmartId
/// the other types set whatever is given. This can also be used to reset an Id
enum SmartIdUpdateMode { SMART_SET_SMART, SMART_SET_NUM, SMART_SET_STR, SMART_SET_ALL };

struct ImplSmartIdData;

/*

SmartId is a substitute for Numeric HelpIds. They can handle Numeric and String HelpIds and offer commonly needed operators.

Matching Ids:
if part of an Id is not set (HasNumeric HasString is False) then this part will never match to anything. Not even unset values

*/
class VCL_DLLPUBLIC SmartId
{
private:
    ImplSmartIdData* mpData;
    SAL_DLLPRIVATE ImplSmartIdData* GetSmartIdData();

public:
   explicit SmartId( const String& rId );
   explicit SmartId( ULONG nId );
            SmartId( const String& rId, ULONG nId );

            SmartId();

            SmartId( const SmartId& rId );
            SmartId& operator = ( const SmartId& rId );

            ~SmartId();

   void UpdateId( const SmartId& rId, SmartIdUpdateMode aMode = SMART_SET_SMART );

   BOOL HasNumeric() const;
   BOOL HasString() const;
   BOOL HasAny() const;
   ULONG GetNum() const;
   String GetStr() const;

   String GetText() const;   /// return String for UI usage

   BOOL Matches( const String &rId )const;
   BOOL Matches( const ULONG nId ) const;
/// In case both Ids have both values set only the StringId is used for Matching
   BOOL Matches( const SmartId &rId ) const;

   BOOL Equals( const SmartId &rId ) const;

   BOOL operator == ( const SmartId& rRight ) const;
   BOOL operator <  ( const SmartId& rRight ) const;
};

#endif
