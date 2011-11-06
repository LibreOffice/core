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



#ifndef _PICKERHELPER_HXX
#define _PICKERHELPER_HXX

#include "svl/svldllapi.h"
#include "sal/types.h"
#include "com/sun/star/uno/Reference.hxx"

namespace com
{
    namespace sun
    {
        namespace star
        {
            namespace ui
            {
                namespace dialogs
                {
                    class XFilePicker;
                    class XFolderPicker;
                }
            }
        }
    }
}


namespace svt
{

    SVL_DLLPUBLIC void SetDialogHelpId(
        ::com::sun::star::uno::Reference < ::com::sun::star::ui::dialogs::XFilePicker > _mxFileDlg,
        sal_Int32 _nHelpId );

    SVL_DLLPUBLIC void SetDialogHelpId(
        ::com::sun::star::uno::Reference < ::com::sun::star::ui::dialogs::XFolderPicker > _mxFileDlg,
        sal_Int32 _nHelpId );

}

//-----------------------------------------------------------------------------

#endif
