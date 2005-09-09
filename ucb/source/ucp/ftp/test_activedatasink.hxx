/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: test_activedatasink.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 15:41:20 $
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
