/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
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
