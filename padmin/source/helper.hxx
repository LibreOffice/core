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



#ifndef _PAD_HELPER_HXX_
#define _PAD_HELPER_HXX_

#ifndef __SGI_STL_LIST
#include <list>
#endif
#include <tools/string.hxx>
#ifndef _RESID_HXX
#include <tools/resid.hxx>
#endif
#include <vcl/dialog.hxx>
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>
#ifndef _SV_LISTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#include <vcl/combobox.hxx>

#if defined SPA_DLLIMPLEMENTATION
#define SPA_DLLPUBLIC SAL_DLLPUBLIC_EXPORT
#else
#define SPA_DLLPUBLIC SAL_DLLPUBLIC_IMPORT
#endif


class Config;

#define PSPRINT_PPDDIR "driver"

namespace padmin
{
class DelMultiListBox : public MultiListBox
{
    Link            m_aDelPressedLink;
public:
    DelMultiListBox( Window* pParent, const ResId& rResId ) :
            MultiListBox( pParent, rResId ) {}
    ~DelMultiListBox() {}

    virtual long Notify( NotifyEvent& rEvent );

    Link setDelPressedLink( const Link& rLink )
    {
        Link aOldLink( m_aDelPressedLink );
                m_aDelPressedLink = rLink;
                return aOldLink;
    }
    const Link& getDelPressedLink() const { return m_aDelPressedLink; }
};

class DelListBox : public ListBox
{
    Link            m_aDelPressedLink;
public:
    DelListBox( Window* pParent, const ResId& rResId ) :
                ListBox( pParent, rResId ) {}
    ~DelListBox() {}

    virtual long Notify( NotifyEvent& rEvent );

    Link setDelPressedLink( const Link& rLink )
    {
        Link aOldLink( m_aDelPressedLink );
        m_aDelPressedLink = rLink;
        return aOldLink;
    }
    const Link& getDelPressedLink() const { return m_aDelPressedLink; }
};

class QueryString : public ModalDialog
{
private:
    OKButton     m_aOKButton;
    CancelButton m_aCancelButton;
    FixedText    m_aFixedText;
    Edit         m_aEdit;
    ComboBox     m_aComboBox;

    String&      m_rReturnValue;
    bool         m_bUseEdit;

    DECL_LINK( ClickBtnHdl, Button* );

public:
    QueryString( Window*, String &, String &, const ::std::list< String >& rChoices = ::std::list<String>() );
    // parent window, Query text, initial value
    ~QueryString();
};

sal_Bool AreYouSure( Window*, int nRid = -1 );

ResId PaResId( sal_uInt32 nId );

void FindFiles( const String& rDirectory, ::std::list< String >& rResult, const String& rSuffixes, bool bRecursive = false );

Config& getPadminRC();
void freePadminRC();

bool chooseDirectory( String& rInOutPath );

} // namespace padmin

#endif
