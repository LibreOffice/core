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



#ifndef _CONNECTIVITY_MAB_NAMEMAPPER_HXX_
#define _CONNECTIVITY_MAB_NAMEMAPPER_HXX_ 1

#include <map>

// Mozilla includes
#include <MNSInclude.hxx>

// Star Includes
#include <rtl/ustring.hxx>

namespace connectivity
{
    namespace mozab
    {
        class MNameMapper
        {
            private:

                struct ltstr
                {
                    bool operator()( const ::rtl::OUString &s1, const ::rtl::OUString &s2) const;
                };


                typedef ::std::multimap< ::rtl::OUString, nsIAbDirectory *, ltstr > dirMap;
                typedef ::std::multimap< ::rtl::OUString, nsIAbDirectory *, ltstr > uriMap;

                static MNameMapper    *instance;
                dirMap                     *mDirMap;
                uriMap                     *mUriMap;

                //clear dirs
                void clear();

            public:
                static MNameMapper* getInstance();

                MNameMapper();
                ~MNameMapper();

                // May modify the name passed in so that it's unique
                nsresult add( ::rtl::OUString& str, nsIAbDirectory* abook );

                //reset dirs
                void reset();

                // Get the directory corresponding to str
                bool getDir( const ::rtl::OUString& str, nsIAbDirectory* *abook );

        };

    }
}

#endif //_CONNECTIVITY_MAB_NAMEMAPPER_HXX_
