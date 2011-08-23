/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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



#ifndef OOX_POWERPOINT_CUSTOMSHOWLISTCONTEXT_HXX
#define OOX_POWERPOINT_CUSTOMSHOWLISTCONTEXT_HXX

#include "oox/core/contexthandler.hxx"
#include <vector>

namespace oox { namespace ppt {


    struct CustomShow
    {
        ::rtl::OUString				maName;
        ::rtl::OUString				mnId;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
