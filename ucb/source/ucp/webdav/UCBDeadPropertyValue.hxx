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



#ifndef _UCBDEADPROPERTYVALUE_HXX_
#define _UCBDEADPROPERTYVALUE_HXX_

#include <rtl/string.hxx>
#include <com/sun/star/uno/Any.hxx>

namespace webdav_ucp
{

class UCBDeadPropertyValue
{
private:
    static const rtl::OUString aTypeString;
    static const rtl::OUString aTypeLong;
    static const rtl::OUString aTypeShort;
    static const rtl::OUString aTypeBoolean;
    static const rtl::OUString aTypeChar;
    static const rtl::OUString aTypeByte;
    static const rtl::OUString aTypeHyper;
    static const rtl::OUString aTypeFloat;
    static const rtl::OUString aTypeDouble;

    static const rtl::OUString aXMLPre;
    static const rtl::OUString aXMLMid;
    static const rtl::OUString aXMLEnd;

public:
    static bool supportsType( const com::sun::star::uno::Type & rType );

    static bool createFromXML( const rtl::OString & rInData,
                               com::sun::star::uno::Any & rOutData );
    static bool toXML( const com::sun::star::uno::Any & rInData,
                       rtl::OUString & rOutData );
};

}

#endif /* _UCBDEADPROPERTYVALUE_HXX_ */
