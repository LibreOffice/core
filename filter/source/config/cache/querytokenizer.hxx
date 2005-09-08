/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: querytokenizer.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 21:31:40 $
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
