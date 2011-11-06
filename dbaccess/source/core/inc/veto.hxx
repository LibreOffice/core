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



#ifndef DBACCESS_VETO_HXX
#define DBACCESS_VETO_HXX

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_UTIL_XVETO_HPP_
#include <com/sun/star/util/XVeto.hpp>
#endif
/** === end UNO includes === **/

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

//........................................................................
namespace dbaccess
{
//........................................................................

    //====================================================================
    //= Veto
    //====================================================================
    typedef ::cppu::WeakImplHelper1 <   ::com::sun::star::util::XVeto
                                    >   Veto_Base;
    /** implements ::com::sun::star::util::XVeto
    */
    class Veto : public Veto_Base
    {
    private:
        const ::rtl::OUString               m_sReason;
        const ::com::sun::star::uno::Any    m_aDetails;

    public:
        Veto( const ::rtl::OUString& _rReason, const ::com::sun::star::uno::Any& _rDetails );

        virtual ::rtl::OUString SAL_CALL getReason() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any SAL_CALL getDetails() throw (::com::sun::star::uno::RuntimeException);

    protected:
        ~Veto();

    private:
        Veto();                         // never implemented
        Veto( const Veto& );            // never implemented
        Veto& operator=( const Veto& ); // never implemented
    };

//........................................................................
} // namespace dbaccess
//........................................................................

#endif // DBACCESS_VETO_HXX

