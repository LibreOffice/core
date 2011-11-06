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



#ifndef _FORMS_NUMERIC_HXX_
#define _FORMS_NUMERIC_HXX_

#include "EditBase.hxx"

//.........................................................................
namespace frm
{
//.........................................................................

//==================================================================
//= ONumericModel
//==================================================================
class ONumericModel
                :public OEditBaseModel
{
private:
    ::com::sun::star::uno::Any          m_aSaveValue;
protected:
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> _getTypes();

public:
    DECLARE_DEFAULT_LEAF_XTOR( ONumericModel );

    // ::com::sun::star::lang::XServiceInfo
    IMPLEMENTATION_NAME(ONumericModel);
    virtual StringSequence SAL_CALL getSupportedServiceNames() throw();

    // ::com::sun::star::io::XPersistObject
    virtual ::rtl::OUString SAL_CALL getServiceName() throw ( ::com::sun::star::uno::RuntimeException);

    // OControlModel's property handling
    virtual void describeFixedProperties(
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rProps
    ) const;

protected:
    // OBoundControlModel overridables
    virtual ::com::sun::star::uno::Any
                            translateDbColumnToControlValue( );
    virtual sal_Bool        commitControlValueToDbColumn( bool _bPostReset );

    virtual ::com::sun::star::uno::Any
                            getDefaultForReset() const;
    virtual void            resetNoBroadcast();

protected:
    DECLARE_XCLONEABLE();
};

//==================================================================
//= ONumericControl
//==================================================================
class ONumericControl: public OBoundControl
{
protected:
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> _getTypes();

public:
    ONumericControl(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory);

    // ::com::sun::star::lang::XServiceInfo
    IMPLEMENTATION_NAME(ONumericControl);
    virtual StringSequence SAL_CALL getSupportedServiceNames() throw();
};

//.........................................................................
}   // namespace frm
//.........................................................................

#endif // _FORMS_NUMERIC_HXX_

