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

#ifndef _DBAUI_CURLEDIT_HXX_
#define _DBAUI_CURLEDIT_HXX_

#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>
#include "dsntypes.hxx"
#include <memory>

namespace dbaui
{

// OConnectionURLEdit
/** an edit control which may be used to edit connection URLs like
    "sdbc:odbc:" or "jdbc:". It prevents the user from editing this prefix,
    though it is normally shown.
*/
class OConnectionURLEdit : public Edit
{
    ::dbaccess::ODsnTypeCollection*
                        m_pTypeCollection;
    FixedText*          m_pForcedPrefix;
    String              m_sSaveValueNoPrefix;
    sal_Bool                m_bShowPrefix; // when <TRUE> the prefix will be visible, otherwise not

public:
    OConnectionURLEdit(Window* pParent, const ResId& rResId,sal_Bool _bShowPrefix = sal_False);
    ~OConnectionURLEdit();

public:
    // Edit overridables
    virtual void    SetText(const OUString& _rStr);
    virtual void    SetText(const OUString& _rStr, const Selection& _rNewSelection);
    virtual OUString  GetText() const;

    /** Showsthe Prefix
        @param  _bShowPrefix
            If <TRUE/> than the prefix will be visible, otherwise not.
    */
    void ShowPrefix(sal_Bool _bShowPrefix);
    /// get the currently set text, excluding the prefix indicating the type
    virtual String  GetTextNoPrefix() const;
    /// set a new text, leave the current prefix unchanged
    virtual void    SetTextNoPrefix(const String& _rText);

    inline void     SaveValueNoPrefix()             { m_sSaveValueNoPrefix = GetTextNoPrefix(); }
    inline String   GetSavedValueNoPrefix() const   { return m_sSaveValueNoPrefix; }
    inline void     SetTypeCollection(::dbaccess::ODsnTypeCollection* _pTypeCollection) { m_pTypeCollection = _pTypeCollection; }
};

}   // namespace dbaui

#endif // _DBAUI_CURLEDIT_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
