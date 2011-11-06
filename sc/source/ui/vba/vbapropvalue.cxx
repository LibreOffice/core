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


#include "vbapropvalue.hxx"

using namespace com::sun::star;

ScVbaPropValue::ScVbaPropValue( PropListener* pListener ) : m_pListener( pListener )
{
}

css::uno::Any SAL_CALL
ScVbaPropValue::getValue() throw (css::uno::RuntimeException)
{
    return m_pListener->getValueEvent();
}

void SAL_CALL
ScVbaPropValue::setValue( const css::uno::Any& _value ) throw (css::uno::RuntimeException)
{
    m_pListener->setValueEvent( _value );
}
