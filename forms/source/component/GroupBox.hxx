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



#ifndef _FORMS_GROUPBOX_HXX_
#define _FORMS_GROUPBOX_HXX_

#include "FormComponent.hxx"

//.........................................................................
namespace frm
{

//==================================================================
// OGroupBoxModel
//==================================================================
class OGroupBoxModel
        :public OControlModel
{
public:
    DECLARE_DEFAULT_LEAF_XTOR( OGroupBoxModel );

    // XServiceInfo
    IMPLEMENTATION_NAME(OGroupBoxModel);
    virtual StringSequence SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

    // XPersistObject
    virtual ::rtl::OUString SAL_CALL    getServiceName() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL
        write(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream>& _rxOutStream) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL
        read(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream>& _rxInStream) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

    // OControlModel's property handling
    virtual void describeAggregateProperties(
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rAggregateProps
    ) const;

protected:
    DECLARE_XCLONEABLE();
};

//==================================================================
// OGroupBoxControl (nur aus Kompatibilitaet zur 5.0)
//==================================================================
class OGroupBoxControl : public OControl
{
public:
    OGroupBoxControl(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory);

    // XServiceInfo
    IMPLEMENTATION_NAME(OGroupBoxControl);
    virtual StringSequence SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);
};

//.........................................................................
}
//.........................................................................

#endif // _FORMS_GROUPBOX_HXX_

