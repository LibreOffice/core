/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sharedstringsbuffer.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-17 08:06:09 $
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

#include "oox/xls/sharedstringsbuffer.hxx"
#include "oox/xls/biffinputstream.hxx"

using ::com::sun::star::uno::Reference;
using ::com::sun::star::text::XText;

namespace oox {
namespace xls {

// ============================================================================

SharedStringsBuffer::SharedStringsBuffer( const WorkbookHelper& rHelper ) :
     WorkbookHelper( rHelper )
{
}

RichStringRef SharedStringsBuffer::createRichString()
{
    RichStringRef xString( new RichString( *this ) );
    maStrings.push_back( xString );
    return xString;
}

void SharedStringsBuffer::importSst( BiffInputStream& rStrm )
{
    sal_Int32 nStringCount = rStrm.skip( 4 ).readInt32();
    if( nStringCount > 0 )
    {
        maStrings.clear();
        maStrings.reserve( static_cast< size_t >( nStringCount ) );
        for( ; rStrm.isValid() && (nStringCount > 0); --nStringCount )
        {
            RichStringRef xString( new RichString( *this ) );
            maStrings.push_back( xString );
            xString->importUniString( rStrm );
        }
    }
}

void SharedStringsBuffer::finalizeImport()
{
    maStrings.forEachMem( &RichString::finalizeImport );
}

void SharedStringsBuffer::convertString( const Reference< XText >& rxText, sal_Int32 nStringId, sal_Int32 nXfId ) const
{
    if( rxText.is() )
        if( const RichString* pString = maStrings.get( nStringId ).get() )
            pString->convert( rxText, nXfId );
}

// ============================================================================

} // namespace xls
} // namespace oox

