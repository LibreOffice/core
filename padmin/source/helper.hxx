/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: helper.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 12:15:58 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _PAD_HELPER_HXX_
#define _PAD_HELPER_HXX_

#ifndef __SGI_STL_LIST
#include <list>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _RESID_HXX
#include <tools/resid.hxx>
#endif
#ifndef _SV_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SV_EDIT_HXX
#include <vcl/edit.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_LISTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _SV_COMBOBOX_HXX
#include <vcl/combobox.hxx>
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

BOOL AreYouSure( Window*, int nRid = -1 );

ResId PaResId( sal_uInt32 nId );

void FindFiles( const String& rDirectory, ::std::list< String >& rResult, const String& rSuffixes, bool bRecursive = false );

Config& getPadminRC();
void freePadminRC();

bool chooseDirectory( String& rInOutPath );

} // namespace padmin

#endif
