/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: customshowlistcontext.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:47:10 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2007 by Sun Microsystems, Inc.
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



#ifndef OOX_POWERPOINT_CUSTOMSHOWLISTCONTEXT_HXX
#define OOX_POWERPOINT_CUSTOMSHOWLISTCONTEXT_HXX

#include "oox/core/contexthandler.hxx"
#include <vector>

namespace oox { namespace ppt {


    struct CustomShow
    {
        ::rtl::OUString             maName;
        ::rtl::OUString             mnId;
        std::vector< rtl::OUString >maSldLst;
    };

    /** CT_ */
    class CustomShowListContext : public ::oox::core::ContextHandler
    {
        std::vector< CustomShow >& mrCustomShowList;

    public:
        CustomShowListContext( ::oox::core::ContextHandler& rParent,
                std::vector< CustomShow >& rCustomShowList );

        ~CustomShowListContext( );
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL
            createFastChildContext( ::sal_Int32 aElementToken, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& /*xAttribs*/ )
                throw ( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );
    };

} }

#endif
