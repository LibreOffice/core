/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: urlfilter.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:42:18 $
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

#ifndef SVTOOLS_URL_FILTER_HXX
#define SVTOOLS_URL_FILTER_HXX

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _WLDCRD_HXX
#include <tools/wldcrd.hxx>
#endif
#include <functional>
#include <vector>
/** filters allowed URLs
*/
class IUrlFilter
{
public:
    virtual bool isUrlAllowed( const String& _rURL ) const = 0;

protected:
    virtual inline ~IUrlFilter() = 0;
};

inline IUrlFilter::~IUrlFilter() {}

struct FilterMatch : public ::std::unary_function< bool, WildCard >
{
private:
    const String&   m_rCompareString;
public:
    FilterMatch( const String& _rCompareString ) : m_rCompareString( _rCompareString ) { }

    bool operator()( const WildCard& _rMatcher )
    {
        return _rMatcher.Matches( m_rCompareString ) ? true : false;
    }

    static void createWildCardFilterList(const String& _rFilterList,::std::vector< WildCard >& _rFilters);
};

#endif // SVTOOLS_URL_FILTER_HXX
