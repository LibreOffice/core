/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: langtab.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 16:49:15 $
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
#ifndef _SVTOOLS_LANGTAB_HXX_
#define _SVTOOLS_LANGTAB_HXX_


#include <tools/resary.hxx>

#include "svtdllapi.h"

//========================================================================
//  class SvtLanguageTable
//========================================================================

class SVT_DLLPUBLIC SvtLanguageTable : public ResStringArray
{
public:
    SvtLanguageTable();
    ~SvtLanguageTable();

    const String&   GetString( const LanguageType eType ) const;
    LanguageType    GetType( const String& rStr ) const;

    sal_uInt32      GetEntryCount() const;
    LanguageType    GetTypeAtIndex( sal_uInt32 nIndex ) const;
};


#endif

