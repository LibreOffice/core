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


#ifndef INCLUDED_FORM_CONTROL_HELPER_HXX
#define INCLUDED_FORM_CONTROL_HELPER_HXX

#include <FFDataHandler.hxx>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include "FieldTypes.hxx"

namespace writerfilter {
namespace dmapper {

using namespace ::com::sun::star;

class FormControlHelper
{
public:
    typedef boost::shared_ptr<FormControlHelper> Pointer_t;
    FormControlHelper(FieldId eFieldId,
                      uno::Reference<text::XTextDocument> rTextDocument,
                      FFDataHandler::Pointer_t pFFData);
    ~FormControlHelper();

    bool insertControl(uno::Reference<text::XTextRange> xTextRange);

private:
    FFDataHandler::Pointer_t m_pFFData;
    struct FormControlHelper_Impl;
    typedef boost::shared_ptr<FormControlHelper_Impl> ImplPointer_t;
    ImplPointer_t m_pImpl;

    bool createCheckbox(uno::Reference<text::XTextRange> xTextRange,
                        const ::rtl::OUString & rControlName);
};

}
}

#endif // INCLUDED_FORM_CONTROL_HELPER_HXX
