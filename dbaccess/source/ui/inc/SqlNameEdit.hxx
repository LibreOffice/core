/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef DBAUI_SQLNAMEEDIT_HXX
#define DBAUI_SQLNAMEEDIT_HXX

#include <vcl/edit.hxx>
#include <vcl/combobox.hxx>

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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
