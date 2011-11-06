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


#ifndef INCLUDED_QNAME_TO_STRING_HXX
#define INCLUDED_QNAME_TO_STRING_HXX

#include <boost/shared_ptr.hpp>
#include <map>
#include <string>
#include <iostream>
#include <resourcemodel/WW8ResourceModel.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>

namespace writerfilter
{
using namespace ::std;

class WRITERFILTER_DLLPUBLIC QNameToString
{
    typedef boost::shared_ptr<QNameToString> Pointer_t;
    typedef map < Id, string > Map;

    static Pointer_t pInstance;

    void init_doctok();
    void init_ooxml();

    Map mMap;

protected:
    /**
       Generated.
     */
    QNameToString();

public:
    static Pointer_t Instance();

    string operator()(Id qName);
};

class WRITERFILTER_DLLPUBLIC SprmIdToString
{
    typedef boost::shared_ptr<SprmIdToString> Pointer_t;

    static Pointer_t pInstance;

    map<sal_uInt32, string> mMap;

protected:
    /**
       Generated automatically.
    */
    SprmIdToString();

public:
    static Pointer_t Instance();
    string operator()(sal_uInt32 nId);
};



}

#endif // INCLUDED_QNAME_TO_STRING_HXX
