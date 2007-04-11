/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: smartid.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 18:10:36 $
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

#ifndef _SMARTID_HXX_
#define _SMARTID_HXX_

#include <tools/string.hxx>

#ifndef _VCL_DLLAPI_H
#include <vcl/dllapi.h>
#endif

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
