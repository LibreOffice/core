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



/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/
#ifndef _FTP_STRCONT_HXX_
#define _FTP_STRCONT_HXX_

#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include "ftpcfunc.hxx"


namespace ftp {


    class FTPInputStream;


    class FTPOutputStreamContainer
        : public FTPStreamContainer
    {
    public:

        FTPOutputStreamContainer(const com::sun::star::uno::Reference<
                                 com::sun::star::io::XOutputStream>& out);
                                 virtual ~FTPOutputStreamContainer() {}

        virtual size_t write(void *buffer,size_t size,size_t nmemb);


    private:

        com::sun::star::uno::Reference<
        com::sun::star::io::XOutputStream> m_out;
    };


    class FTPInputStreamContainer
        : public FTPStreamContainer
    {
    public:

        FTPInputStreamContainer(FTPInputStream* out);

        virtual size_t write(void *buffer,size_t size,size_t nmemb);

        com::sun::star::uno::Reference<
        com::sun::star::io::XInputStream> operator()();

    private:

        FTPInputStream* m_out;
    };


}


#endif
