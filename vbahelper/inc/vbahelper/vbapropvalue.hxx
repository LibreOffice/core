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


#ifndef SC_VBA_PROPVALULE_HXX
#define SC_VBA_PROPVALULE_HXX
#include <ooo/vba/XPropValue.hpp>
#include <cppuhelper/implbase1.hxx>

#include <vbahelper/vbahelper.hxx>

typedef ::cppu::WeakImplHelper1< ov::XPropValue > PropValueImpl_BASE;

class VBAHELPER_DLLPUBLIC PropListener
{
public:
    virtual void setValueEvent( const css::uno::Any& value ) = 0;
    virtual css::uno::Any getValueEvent() = 0;
};


class VBAHELPER_DLLPUBLIC ScVbaPropValue : public PropValueImpl_BASE
{
    PropListener* m_pListener;
public:
    ScVbaPropValue( PropListener* pListener );

    // Attributes
    virtual css::uno::Any SAL_CALL getValue() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setValue( const css::uno::Any& _value ) throw (css::uno::RuntimeException);

    rtl::OUString SAL_CALL getDefaultPropertyName() throw (css::uno::RuntimeException) { return ::rtl::OUString::createFromAscii("Value"); }

};
#endif //SC_VBA_PROPVALULE_HXX
