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

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/
#ifndef _TEST_ACTIVEDATASINK_HXX_
#define _TEST_ACTIVEDATASINK_HXX_

#include <cppuhelper/weak.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/io/XInputStream.hpp>

namespace test_ftp {


    class Test_ActiveDataSink
        : public cppu::OWeakObject,
          public com::sun::star::io::XActiveDataSink
    {
    public:

        // XInterface

        virtual com::sun::star::uno::Any SAL_CALL
        queryInterface( const com::sun::star::uno::Type& rType )
            throw( com::sun::star::uno::RuntimeException );


        virtual void SAL_CALL acquire( void ) throw();

        virtual void SAL_CALL release( void ) throw();


        // XActiveDataSink

        virtual void SAL_CALL
        setInputStream(const com::sun::star::uno::Reference<com::sun::star::io::XInputStream>& aStream )
            throw(com::sun::star::uno::RuntimeException)
        {
            m_xInputStream = aStream;
        }


        virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL
        getInputStream(  )
            throw(::com::sun::star::uno::RuntimeException)
        {
            return m_xInputStream;
        }


    private:

        com::sun::star::uno::Reference<com::sun::star::io::XInputStream> m_xInputStream;

    };

}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
