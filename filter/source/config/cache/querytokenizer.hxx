/*************************************************************************
 *
 *  $RCSfile: querytokenizer.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2004-01-28 15:18:20 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _FILTER_CONFIG_QUERYTOKENIZER_HXX_
#define _FILTER_CONFIG_QUERYTOKENIZER_HXX_

//_______________________________________________
// includes

#include <hash_map>

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

//_______________________________________________
// namespace

namespace filter{
    namespace config{

//_______________________________________________
// definitions

/** @short      It can be used to split any query string (which can be used at the
                related interface <type scope="com::sun::star::container">XContainerQuery</type>)
                into its different tokens using a fix schema.

    @descr      All queries implemented of the services
                <ul>
                    <li><type scope="com::sun::star::document">TypeDetection</type></li>
                    <li><type scope="com::sun::star::document">FilterFactory</type></li>
                    <li><type scope="com::sun::star::document">ExtendedTypeDetectionFactory</type></li>
                    <li><type scope="com::sun::star::frame">FrameLoaderFactory</type></li>
                    <li><type scope="com::sun::star::frame">ContentHandlerFactory</type></li>
                </ul>
                uses this schema.

    @attention  This class is not threadsafe implemented. Because its not neccessary.
                But you have to make shure, that ist not used as such :-)
 */
class QueryTokenizer : public ::std::hash_map< ::rtl::OUString                    ,
                                               ::rtl::OUString                    ,
                                               ::rtl::OUStringHash                ,
                                               ::std::equal_to< ::rtl::OUString > >
{
    //-------------------------------------------
    // member

    private:

        /** @short  Because the given query can contain errors,
                    it should be checked outside.

            TODO    May its a good idea to describe the real problem
                    more detailed ...
         */
        sal_Bool m_bValid;

    //-------------------------------------------
    // interface

    public:

        /** @short  create a new tokenizer instance with a
                    a new query.

            @descr  The given query is immidiatly analyzed
                    and seperated into its token, which can
                    be access by some specialized method later.

            @param  sQuery
                    the query string.
         */
        QueryTokenizer(const ::rtl::OUString& sQuery);

        //---------------------------------------

        /** @short  destruct an instance of this class.
         */
        virtual ~QueryTokenizer();

        //---------------------------------------

        /** @short  can be used to check if analyzing of given query
                    was successfully or not.
         */
        virtual sal_Bool valid() const;
};

    } // namespace config
} // namespace filter

#endif // _FILTER_CONFIG_QUERYTOKENIZER_HXX_
