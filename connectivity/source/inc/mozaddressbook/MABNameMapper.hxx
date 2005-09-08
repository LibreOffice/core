/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MABNameMapper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 07:29:33 $
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

#ifndef _CONNECTIVITY_MAB_NAMEMAPPER_HXX_
#define _CONNECTIVITY_MAB_NAMEMAPPER_HXX_ 1

#include <map>

// Mozilla includes
#include <MABNSInclude.hxx>

// StarOffice Includes
#include <rtl/ustring.hxx>

namespace connectivity
{
    namespace mozaddressbook
    {
        class OMozabNameMapper
        {
            private:

                struct ltstr
                {
                    bool operator()( const ::rtl::OUString &s1, const ::rtl::OUString &s2) const;
                };


                typedef ::std::multimap< ::rtl::OUString, nsIAbDirectory *, ltstr > dirMap;

                static OMozabNameMapper    *instance;
                dirMap                     *mDirMap;

            public:
                static OMozabNameMapper* getInstance();

                OMozabNameMapper();
                ~OMozabNameMapper();

                // May modify the name passed in so that it's unique
                void add( ::rtl::OUString& str, nsIAbDirectory* abook );

                // Will replace the given dir
                void replace( const ::rtl::OUString& str, nsIAbDirectory* abook );

                // Get the directory corresponding to str
                bool getDir( const ::rtl::OUString& str, nsIAbDirectory* *abook );

        };

    }
}

#endif //_CONNECTIVITY_MAB_NAMEMAPPER_HXX_
