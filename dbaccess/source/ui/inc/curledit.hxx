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

#ifndef _DBAUI_CURLEDIT_HXX_
#define _DBAUI_CURLEDIT_HXX_

#ifndef _SV_EDIT_HXX
#include <vcl/edit.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _DBAUI_DSNTYPES_HXX_
#include "dsntypes.hxx"
#endif
#include <memory>

//.........................................................................
namespace dbaui
{
//.........................................................................

//=========================================================================
//= OConnectionURLEdit
//=========================================================================
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
    virtual void    SetText(const String& _rStr);
    virtual void    SetText(const String& _rStr, const Selection& _rNewSelection);
    virtual String  GetText() const;


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

//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // _DBAUI_CURLEDIT_HXX_

