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



#ifndef COMPHELPER_DOCUMENTINFO_HXX
#define COMPHELPER_DOCUMENTINFO_HXX

#include "comphelper/comphelperdllapi.h"

/** === begin UNO includes === **/
#include <com/sun/star/frame/XModel.hpp>
/** === end UNO includes === **/

//........................................................................
namespace comphelper {
//........................................................................

    //====================================================================
    //= DocumentInfo
    //====================================================================
    class COMPHELPER_DLLPUBLIC DocumentInfo
    {
    public:
        /** retrieves the UI title of the given document
        */
        static ::rtl::OUString  getDocumentTitle( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& _rxDocument );

    private:
        DocumentInfo(); // never implemented
    };

//........................................................................
}   // namespace comphelper
//........................................................................

#endif // COMPHELPER_DOCUMENTINFO_HXX
