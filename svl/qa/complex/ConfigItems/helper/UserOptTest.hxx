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



#ifndef  SVTOOLS_USEROPTTEST_HXX
#define  SVTOOLS_USEROPTTEST_HXX

#include <com/sun/star/container/XNameAccess.hpp>
#include <unotools/useroptions.hxx>

namespace css = ::com::sun::star;

class UserOptTest
{
public:
    UserOptTest();
    ~UserOptTest();

    void impl_checkUserData();

private:
    void impl_checkSetCompany( const ::rtl::OUString& sUserData );
    void impl_checkSetFirstName( const ::rtl::OUString& sUserData );
    void impl_checkSetLastName( const ::rtl::OUString& sUserData );
    void impl_checkSetID( const ::rtl::OUString& sUserData );
    void impl_checkSetStreet( const ::rtl::OUString& sUserData );
    void impl_checkSetCity( const ::rtl::OUString& sUserData );
    void impl_checkSetState( const ::rtl::OUString& sUserData );
    void impl_checkSetZip( const ::rtl::OUString& sUserData );
    void impl_checkSetCountry( const ::rtl::OUString& sUserData );
    void impl_checkSetPosition( const ::rtl::OUString& sUserData );
    void impl_checkSetTitle( const ::rtl::OUString& sUserData );
    void impl_checkSetTelephoneHome( const ::rtl::OUString& sUserData );
    void impl_checkSetTelephoneWork( const ::rtl::OUString& sUserData );
    void impl_checkSetFax( const ::rtl::OUString& sUserData );
    void impl_checkSetEmail( const ::rtl::OUString& sUserData );
    void impl_checkSetCustomerNumber( const ::rtl::OUString& sUserData );
    void impl_checkSetFathersName( const ::rtl::OUString& sUserData );
    void impl_checkSetApartment( const ::rtl::OUString& sUserData );

private:
    SvtUserOptions m_aConfigItem;

    css::uno::Reference< css::container::XNameAccess > m_xCfg;
};

#endif // #ifndef  SVTOOLS_USEROPTTEST_HXX
