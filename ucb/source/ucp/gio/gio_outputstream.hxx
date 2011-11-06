/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef GIO_OUTPUTSTREAM_HXX
#define GIO_OUTPUTSTREAM_HXX

#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <cppuhelper/weak.hxx>

#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XTruncate.hpp>
#include <com/sun/star/io/XSeekable.hpp>

#include "gio_seekable.hxx"

namespace gio
{

class OutputStream :
    public ::com::sun::star::io::XOutputStream,
    public Seekable
{
private:
    GFileOutputStream *mpStream;

public:
    OutputStream ( GFileOutputStream *pStream );
    virtual ~OutputStream();

    // XInterface
    virtual com::sun::star::uno::Any SAL_CALL queryInterface(const ::com::sun::star::uno::Type & type )
            throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL acquire( void ) throw () { OWeakObject::acquire(); }
    virtual void SAL_CALL release( void ) throw() { OWeakObject::release(); }

    // XOutputStream
   virtual void SAL_CALL writeBytes( const com::sun::star::uno::Sequence< sal_Int8 >& aData )
           throw( com::sun::star::io::NotConnectedException,
                  com::sun::star::io::BufferSizeExceededException,
                  com::sun::star::io::IOException,
                  com::sun::star::uno::RuntimeException);

   virtual void SAL_CALL flush( void )
           throw( com::sun::star::io::NotConnectedException,
                  com::sun::star::io::BufferSizeExceededException,
                  com::sun::star::io::IOException,
                  com::sun::star::uno::RuntimeException);


    virtual void SAL_CALL closeOutput( void )
           throw( com::sun::star::io::NotConnectedException,
                  com::sun::star::io::IOException,
                  com::sun::star::uno::RuntimeException );
};

} // namespace gio
#endif
