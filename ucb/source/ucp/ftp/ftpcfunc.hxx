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
#ifndef _FTP_FTPCFUNC_HXX_
#define _FTP_FTPCFUNC_HXX_

#include <rtl/ustring.hxx>


namespace ftp {

    class FTPStreamContainer
    {
    public:

        virtual size_t write(void *buffer,size_t size,size_t nmemb) = 0;
    };

}


extern "C" {

    /** callback for curl_easy_perform(),
     *  forwarding the written content to the stream.
     *  stream has to be of type oslFileHandle.
     */


    int file_write(void *buffer,size_t size,size_t nmemb,void *stream);

}

#endif
