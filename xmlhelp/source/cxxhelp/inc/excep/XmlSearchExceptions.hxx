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


#ifndef _XMLSEARCH_EXCEP_XMLSEARCHEXCEPTIONS_HXX_
#define _XMLSEARCH_EXCEP_XMLSEARCHEXCEPTIONS_HXX_

#include  <rtl/ustring.hxx>


namespace xmlsearch {

    namespace excep {


        class XmlSearchException
        {
        public:

            XmlSearchException( const rtl::OUString& message )
                : _message( message )
            {
            }

            rtl::OUString getMessage() const
            {
                return _message;
            }


        private:

            rtl::OUString _message;
        };


        class IOException
            : public virtual XmlSearchException
        {
        public:

            IOException( const rtl::OUString& message )
                : XmlSearchException( message )
            {
            }
        };


        class NoFactoryException
            : public virtual XmlSearchException
        {
        public:
            NoFactoryException( const rtl::OUString& message )
                : XmlSearchException( message )
            {
            }
        };


        class NoSuchBlock
            : public virtual XmlSearchException
        {
        public:
            NoSuchBlock( const rtl::OUString& message )
                : XmlSearchException( message )
            {
            }
        };


        class IllegalIndexException
            : public virtual XmlSearchException
        {
        public:
            IllegalIndexException( const rtl::OUString& message )
                : XmlSearchException( message )
            {
            }
        };

    }
}


#endif
