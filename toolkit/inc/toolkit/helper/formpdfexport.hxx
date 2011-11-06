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



#ifndef _TOOLKIT_HELPER_FORM_FORMPDFEXPORT_HXX
#define _TOOLKIT_HELPER_FORM_FORMPDFEXPORT_HXX

#include <toolkit/dllapi.h>

/** === begin UNO includes === **/
#include <com/sun/star/awt/XControl.hpp>
/** === end UNO includes === **/

#include <vcl/pdfwriter.hxx>

#include <memory>

namespace vcl
{
    class PDFExtOutDevData;
}

//........................................................................
namespace toolkitform
{
//........................................................................

    /** creates a PDF compatible control descriptor for the given control
    */
    void TOOLKIT_DLLPUBLIC describePDFControl(
            const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& _rxControl,
            ::std::auto_ptr< ::vcl::PDFWriter::AnyWidget >& _rpDescriptor,
            ::vcl::PDFExtOutDevData& i_pdfExportData
        ) SAL_THROW(());

//........................................................................
} // namespace toolkitform
//........................................................................

#endif // _TOOLKIT_HELPER_FORM_FORMPDFEXPORT_HXX
