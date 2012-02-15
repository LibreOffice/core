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



#ifndef TOOLKIT_SPINNINGPROGRESS_HXX
#define TOOLKIT_SPINNINGPROGRESS_HXX

#include "toolkit/controls/animatedimages.hxx"

/** === begin UNO includes === **/
/** === end UNO includes === **/

//......................................................................................................................
namespace toolkit
{
//......................................................................................................................

    //==================================================================================================================
    //= SpinningProgressControlModel
    //==================================================================================================================
    typedef AnimatedImagesControlModel SpinningProgressControlModel_Base;
    class SpinningProgressControlModel : public SpinningProgressControlModel_Base
    {
    public:
                                    SpinningProgressControlModel( ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > const & i_factory );
                                    SpinningProgressControlModel( const SpinningProgressControlModel& i_copySource );

        virtual UnoControlModel*    Clone() const;

        // XPropertySet
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

        // XPersistObject
        ::rtl::OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException);

        // XServiceInfo
        ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
        ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

    protected:
                                    ~SpinningProgressControlModel();
    };

//......................................................................................................................
} // namespace toolkit
//......................................................................................................................

#endif // TOOLKIT_SPINNINGPROGRESS_HXX
