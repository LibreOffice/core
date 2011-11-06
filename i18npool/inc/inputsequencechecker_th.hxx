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


#ifndef _I18N_INPUTSEQUENCECHECKER_TH_HXX_
#define _I18N_INPUTSEQUENCECHECKER_TH_HXX_

#include <inputsequencechecker.hxx>

namespace com { namespace sun { namespace star { namespace i18n {

//  ----------------------------------------------------
//  class InputSequenceChecker_th
//  ----------------------------------------------------
class InputSequenceChecker_th : public InputSequenceCheckerImpl
{
public:
    InputSequenceChecker_th();
    ~InputSequenceChecker_th();

    sal_Bool SAL_CALL checkInputSequence(const rtl::OUString& Text, sal_Int32 nStartPos,
        sal_Unicode inputChar, sal_Int16 inputCheckMode) throw(com::sun::star::uno::RuntimeException);

    sal_Int32 SAL_CALL correctInputSequence(rtl::OUString& Text, sal_Int32 nStartPos,
        sal_Unicode inputChar, sal_Int16 inputCheckMode) throw(com::sun::star::uno::RuntimeException);
};

} } } }

#endif // _I18N_BREAKITERATOR_HXX_
