/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: stringlistresource.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-21 15:28:30 $
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

#ifndef SVX_STRINGLISTRESOURCE_HXX
#define SVX_STRINGLISTRESOURCE_HXX

/** === begin UNO includes === **/
/** === end UNO includes === **/

#include <tools/rc.hxx>

#include <memory>

//........................................................................
namespace svx
{
//........................................................................

    //====================================================================
    //= StringListResource
    //====================================================================
    /** loads a list of strings from a resource, where the resource is of type RSC_RESOURCE,
        and has sub resources of type string, numbered from 1 to n
    */
    class StringListResource : public Resource
    {
    public:
        StringListResource( const ResId& _rResId );
        ~StringListResource();

        inline void get( ::std::vector< String >& _rStrings )
        {
            _rStrings = m_aStrings;
        }


        /** returns the String with a given local resource id

            @param  _nResId
                The resource id. It will not be checked if this id exists.

            @return String
                The string.
        */
        String getString( USHORT _nResId )
        {
            return String( ResId( _nResId, *m_pResMgr ) );
        }

        size_t  size() const    { return m_aStrings.size(); }
        bool    empty() const   { return m_aStrings.empty(); }

        const String& operator[]( size_t _index ) const { return m_aStrings[ _index ]; }

    private:
        ::std::vector< String > m_aStrings;
    };

//........................................................................
} // namespace svx
//........................................................................

#endif // SVX_STRINGLISTRESOURCE_HXX
