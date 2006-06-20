/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MNameMapper.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 01:51:50 $
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
#include <MNSInclude.hxx>

// Star Includes
#include <rtl/ustring.hxx>

namespace connectivity
{
    namespace mozab
    {
        class MNameMapper
        {
            private:

                struct ltstr
                {
                    bool operator()( const ::rtl::OUString &s1, const ::rtl::OUString &s2) const;
                };


                typedef ::std::multimap< ::rtl::OUString, nsIAbDirectory *, ltstr > dirMap;
                typedef ::std::multimap< ::rtl::OUString, nsIAbDirectory *, ltstr > uriMap;

                static MNameMapper    *instance;
                dirMap                     *mDirMap;
                uriMap                     *mUriMap;

                //clear dirs
                void clear();

            public:
                static MNameMapper* getInstance();

                MNameMapper();
                ~MNameMapper();

                // May modify the name passed in so that it's unique
                nsresult add( ::rtl::OUString& str, nsIAbDirectory* abook );

                //reset dirs
                void reset();

                // Get the directory corresponding to str
                bool getDir( const ::rtl::OUString& str, nsIAbDirectory* *abook );

        };

    }
}

#endif //_CONNECTIVITY_MAB_NAMEMAPPER_HXX_
