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



#ifndef SVTOOLS_PICKERHISTORYACCESS_HXX
#define SVTOOLS_PICKERHISTORYACCESS_HXX

#include "svl/svldllapi.h"

#ifndef _COM_SUN_STAR_UNO_REFERENX_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif

//.........................................................................
namespace svt
{
//.........................................................................

    // --------------------------------------------------------------------
    SVL_DLLPUBLIC void addFolderPicker(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxPicker );

    SVL_DLLPUBLIC void addFilePicker(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxPicker );

//.........................................................................
}   // namespace svt
//.........................................................................

#endif // SVTOOLS_PICKERHISTORYACCESS_HXX

