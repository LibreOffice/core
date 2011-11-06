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


#ifndef DBAUI_SQLNAMEEDIT_HXX
#define DBAUI_SQLNAMEEDIT_HXX

#ifndef _SV_EDIT_HXX
#include <vcl/edit.hxx>
#endif
#ifndef _SV_COMBOBOX_HXX
#include <vcl/combobox.hxx>
#endif

namespace dbaui
{
    class OSQLNameChecker
    {
        ::rtl::OUString m_sAllowedChars;
        sal_Bool        m_bOnlyUpperCase;
        sal_Bool        m_bCheck;           // true when we should check for invalid chars
    public:
        OSQLNameChecker(const ::rtl::OUString& _rAllowedChars)
            :m_sAllowedChars(_rAllowedChars)
            ,m_bOnlyUpperCase(sal_False)
            ,m_bCheck(sal_True)
        {
        }

        void setUpperCase(sal_Bool _bUpper=sal_True)
        {
            m_bOnlyUpperCase = _bUpper;
        }
        void setAllowedChars(const ::rtl::OUString& _rAllowedChars)
        {
            m_sAllowedChars = _rAllowedChars;
        }
        // default is false because it is initialized with true
        void setCheck(sal_Bool _bCheck = sal_False)
        {
            m_bCheck = _bCheck;
        }
        sal_Bool checkString(const ::rtl::OUString& _sOldValue,const ::rtl::OUString& _sToCheck,::rtl::OUString& _rsCorrected);
    };
    //==================================================================
    class OSQLNameEdit : public Edit
                        ,public OSQLNameChecker
    {
    public:
        OSQLNameEdit(Window* _pParent,const ::rtl::OUString& _rAllowedChars, WinBits nStyle = WB_BORDER)
            : Edit(_pParent,nStyle)
            ,OSQLNameChecker(_rAllowedChars)
        {
        }
        OSQLNameEdit(Window* _pParent,const ResId& _rRes,const ::rtl::OUString& _rAllowedChars = ::rtl::OUString())
            : Edit(_pParent,_rRes)
            ,OSQLNameChecker(_rAllowedChars)
        {
        }

        // Window overload
        //  virtual long PreNotify( NotifyEvent& rNEvt );
        // Edit
        virtual void Modify();
    };

    class OSQLNameComboBox : public ComboBox
                            ,public OSQLNameChecker
    {
    public:
        OSQLNameComboBox(Window* _pParent,const ::rtl::OUString& _rAllowedChars, WinBits nStyle = WB_BORDER)
            : ComboBox(_pParent,nStyle)
            ,OSQLNameChecker(_rAllowedChars)
        {
        }
        OSQLNameComboBox(Window* _pParent,const ResId& _rRes,const ::rtl::OUString& _rAllowedChars = ::rtl::OUString())
            : ComboBox(_pParent,_rRes)
            ,OSQLNameChecker(_rAllowedChars)
        {
        }

        // Window overload
        // Edit
        virtual void Modify();
    };

}
#endif // DBAUI_SQLNAMEEDIT_HXX


