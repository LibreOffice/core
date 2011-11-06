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



#ifndef ADABASUI_SQLNAMEEDIT_HXX
#define ADABASUI_SQLNAMEEDIT_HXX

#ifndef _SV_EDIT_HXX
#include <vcl/edit.hxx>
#endif
#ifndef _SV_COMBOBOX_HXX
#include <vcl/combobox.hxx>
#endif

namespace adabasui
{
    class OSQLNameChecker
    {
    public:
        sal_Bool checkString(const ::rtl::OUString& _sOldValue,const ::rtl::OUString& _sToCheck,::rtl::OUString& _rsCorrected);
    };
    //==================================================================
    class OSQLNameEdit : public Edit
                        ,public OSQLNameChecker
    {
    public:
        OSQLNameEdit(Window* _pParent,const ResId& _rRes)
            : Edit(_pParent,_rRes)
        {
        }

        // Edit
        virtual void Modify();
    };
}
#endif // DBAUI_SQLNAMEEDIT_HXX


