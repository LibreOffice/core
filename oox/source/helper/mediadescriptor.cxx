/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: mediadescriptor.cxx,v $
 * $Revision: 1.1 $
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

#include "oox/helper/mediadescriptor.hxx"

using ::rtl::OUString;
using ::com::sun::star::beans::NamedValue;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Sequence;

namespace oox {

// ============================================================================

MediaDescriptor::MediaDescriptor()
{
}

MediaDescriptor::MediaDescriptor( const Sequence< PropertyValue >& rMediaDescSeq ) :
    ::comphelper::MediaDescriptor( rMediaDescSeq )
{
}

Any MediaDescriptor::getComponentDataEntry( const OUString& rName ) const
{
    Sequence< NamedValue > aCompData = getUnpackedValueOrDefault( PROP_COMPONENTDATA(), Sequence< NamedValue >() );
    ::comphelper::SequenceAsHashMap aCompDataMap( aCompData );
    ::comphelper::SequenceAsHashMap::iterator aIt = aCompDataMap.find( rName );
    return (aIt == aCompDataMap.end()) ? Any() : aIt->second;
}

void MediaDescriptor::setComponentDataEntry( const OUString& rName, const Any& rValue )
{
    Any& rCompDataAny = operator[]( PROP_COMPONENTDATA() );
    OSL_ENSURE( !rCompDataAny.hasValue() || rCompDataAny.has< Sequence< NamedValue > >(),
        "MediaDescriptor::setComponentDataEntry - incompatible 'ComponentData' property in media descriptor" );
    if( !rCompDataAny.hasValue() || rCompDataAny.has< Sequence< NamedValue > >() )
    {
        // extract existing sequence
        Sequence< NamedValue > aCompData;
        rCompDataAny >>= aCompData;
        // insert or overwrite the passed value
        SequenceAsHashMap aCompDataMap( aCompData );
        aCompDataMap[ rName ] = rValue;
        // write back the sequence (sal_False = use NamedValue instead of PropertyValue)
        rCompDataAny = aCompDataMap.getAsConstAny( sal_False );
    }
}

// ============================================================================

} // namespace oox

