/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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

#ifndef COMMANDIMAGEPROVIDER_HXX
#define COMMANDIMAGEPROVIDER_HXX

/** === begin UNO includes === **/
#include <com/sun/star/frame/XModel.hpp>
/** === end UNO includes === **/

#include <comphelper/componentcontext.hxx>
#include <vcl/image.hxx>

#include <boost/shared_ptr.hpp>

//........................................................................
namespace frm
{
//........................................................................

    //=====================================================================
    //= ICommandImageProvider
    //=====================================================================
    typedef ::rtl::OUString                                 CommandURL;
    typedef ::com::sun::star::uno::Sequence< CommandURL >   CommandURLs;
    typedef ::std::vector< Image >                          CommandImages;

    class SAL_NO_VTABLE ICommandImageProvider
    {
    public:
        virtual CommandImages   getCommandImages(
                                    const CommandURLs& _rCommandURLs,
                                    const bool _bLarge
                                ) const = 0;

        virtual ~ICommandImageProvider() { }
    };

    typedef ::boost::shared_ptr< const ICommandImageProvider >  PCommandImageProvider;

    //=====================================================================
    //= factory
    //=====================================================================
    PCommandImageProvider
        createDocumentCommandImageProvider(
            const ::comphelper::ComponentContext& _rContext,
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& _rxDocument
        );

//........................................................................
} // namespace frm
//........................................................................

#endif // COMMANDIMAGEPROVIDER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
