/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef DBAUI_SQLNAMEEDIT_HXX
#define DBAUI_SQLNAMEEDIT_HXX

#include <vcl/edit.hxx>
#include <vcl/combobox.hxx>

namespace dbaui
{
    class OSQLNameChecker
    {
        OUString m_sAllowedChars;
        sal_Bool        m_bOnlyUpperCase;
        sal_Bool        m_bCheck;           // true when we should check for invalid chars
    public:
        OSQLNameChecker(const OUString& _rAllowedChars)
            :m_sAllowedChars(_rAllowedChars)
            ,m_bOnlyUpperCase(sal_False)
            ,m_bCheck(sal_True)
        {
        }

        void setUpperCase(sal_Bool _bUpper=sal_True)
        {
            m_bOnlyUpperCase = _bUpper;
        }
        void setAllowedChars(const OUString& _rAllowedChars)
        {
            m_sAllowedChars = _rAllowedChars;
        }
        // default is false because it is initialized with true
        void setCheck(sal_Bool _bCheck = sal_False)
        {
            m_bCheck = _bCheck;
        }
        sal_Bool checkString(const OUString& _sToCheck,OUString& _rsCorrected);
    };
    class OSQLNameEdit : public Edit
                        ,public OSQLNameChecker
    {
    public:
        OSQLNameEdit(Window* _pParent,const OUString& _rAllowedChars, WinBits nStyle = WB_BORDER)
            : Edit(_pParent,nStyle)
            ,OSQLNameChecker(_rAllowedChars)
        {
        }
        OSQLNameEdit(Window* _pParent,const ResId& _rRes,const OUString& _rAllowedChars = OUString())
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
        OSQLNameComboBox(Window* _pParent,const OUString& _rAllowedChars, WinBits nStyle = WB_BORDER)
            : ComboBox(_pParent,nStyle)
            ,OSQLNameChecker(_rAllowedChars)
        {
        }
        OSQLNameComboBox(Window* _pParent,const ResId& _rRes,const OUString& _rAllowedChars = OUString())
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
