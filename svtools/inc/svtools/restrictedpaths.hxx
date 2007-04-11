/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: restrictedpaths.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:32:53 $
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

#ifndef SVTOOLS_RESTRICTEDPATHS_HXX
#define SVTOOLS_RESTRICTEDPATHS_HXX

#ifndef SVTOOLS_URL_FILTER_HXX
#include <svtools/urlfilter.hxx>
#endif

#ifndef INCLUDED_SVTDLLAPI_H
#include <svtools/svtdllapi.h>
#endif

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#include <vector>

namespace svt
{
    class SVT_DLLPUBLIC RestrictedPaths : public IUrlFilter
    {
    private:
        ::std::vector< String >     m_aUnrestrictedURLs;
        bool                        m_bFilterIsEnabled;

    public:
        RestrictedPaths();
        virtual ~RestrictedPaths();

        inline bool hasFilter() const { return !m_aUnrestrictedURLs.empty(); }
        inline const ::std::vector< String >& getFilter() const { return m_aUnrestrictedURLs; }

        inline void enableFilter( bool _bEnable ) { m_bFilterIsEnabled = _bEnable; }
        inline bool isFilterEnabled() const { return m_bFilterIsEnabled; }

    public:
        /** checks URL access permissions

            <p>with the "restriction" feature we have in the file dialog, it's possible that
            only certain URLs can be browsed. This method checks whether a given URL belongs
            to this set of permitted URLs.</p>

            <p>If no "access restriction" is effective, this method always returns <TRUE/>.</p>
        */
        virtual bool                isUrlAllowed( const String& _rURL ) const;

        /** checks URL access permissions

            <p>with the "restriction" feature we have in the file dialog, it's possible that
            only certain URLs can be browsed. This method checks whether a given URL belongs
            to this set of permitted URLs.</p>

            <p>Default behavior allows access to parent folder of a restricted folder (but not to its siblings).
            If allowParents is set to <FALSE/> parent folders will be treated as forbidden.

            <p>If no "access restriction" is effective, this method always returns <TRUE/>.</p>
        */
        bool                        isUrlAllowed( const String& _rURL, bool allowParents ) const;
    };

}   // namespace svt

#endif  // SVTOOLS_RESTRICTEDPATHS_HXX
