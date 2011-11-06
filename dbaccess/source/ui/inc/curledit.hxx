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

