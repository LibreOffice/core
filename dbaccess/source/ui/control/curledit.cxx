/*************************************************************************
 *
 *  $RCSfile: curledit.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: fs $ $Date: 2000-10-24 12:08:19 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _DBAUI_CURLEDIT_HXX_
#include "curledit.hxx"
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

//.........................................................................
namespace dbaui
{
//.........................................................................

//=========================================================================
//= OConnectionURLEdit
//=========================================================================
//-------------------------------------------------------------------------
OConnectionURLEdit::OConnectionURLEdit(Window* _pParent, WinBits _nStyle)
    :Edit(_pParent, _nStyle)
    ,m_pForcedPrefix(NULL)
{
}

//-------------------------------------------------------------------------
OConnectionURLEdit::OConnectionURLEdit(Window* _pParent, const ResId& _rResId)
    :Edit(_pParent, _rResId)
    ,m_pForcedPrefix(NULL)
{
}

//-------------------------------------------------------------------------
OConnectionURLEdit::~OConnectionURLEdit()
{
    // delete my sub controls
    Edit* pSubEdit = GetSubEdit();
    SetSubEdit(NULL);
    delete pSubEdit;
    delete m_pForcedPrefix;
}

//-------------------------------------------------------------------------
void OConnectionURLEdit::SetTextNoPrefix(const String& _rText)
{
    DBG_ASSERT(GetSubEdit(), "OConnectionURLEdit::SetTextNoPrefix: have no current type, not changing the text!");
    if (GetSubEdit())
        GetSubEdit()->SetText(_rText);
}

//-------------------------------------------------------------------------
String OConnectionURLEdit::GetTextNoPrefix() const
{
    if (GetSubEdit())
        return GetSubEdit()->GetText();
    return GetText();
}

//-------------------------------------------------------------------------
void OConnectionURLEdit::SetText(const String& _rStr)
{
    Selection aNoSelection(0,0);
    SetText(_rStr, aNoSelection);
}

//-------------------------------------------------------------------------
void OConnectionURLEdit::SetText(const String& _rStr, const Selection& _rNewSelection)
{
    // create new sub controls, if necessary
    if (!GetSubEdit())
        SetSubEdit(new Edit(this, 0));
    if (!m_pForcedPrefix)
    {
        m_pForcedPrefix = new FixedText(this, WB_VCENTER);

        // we use a gray background for the fixed text
        StyleSettings aSystemStyle = Application::GetSettings().GetStyleSettings();
        m_pForcedPrefix->SetBackground(Wallpaper(aSystemStyle.GetDialogColor()));
    }

    sal_Bool bIsEmpty = 0 == _rStr.Len();
    // calc the prefix
    String sPrefix;
    if (!bIsEmpty)
    {
        // determine the type of the new URL described by the new text
        DATASOURCE_TYPE eType = m_aTypeCollection.getType(_rStr);
        // and the prefix belonging to this type
        if (DST_UNKNOWN != eType)
            sPrefix = m_aTypeCollection.getDatasourcePrefix(eType);
        else
            DBG_ERROR("OConnectionURLEdit::SetText : the new text is no valid connection URL!");
    }

    // the fixed text gets the prefix
    m_pForcedPrefix->SetText(sPrefix);

    // both subs have to be resized according to the text len of the prefix
    Size aMySize = GetSizePixel();
    sal_Int32 nTextWidth = m_pForcedPrefix->GetTextWidth(sPrefix) + 2;
    m_pForcedPrefix->SetPosSizePixel(Point(0, -2), Size(nTextWidth, aMySize.Height()));
    GetSubEdit()->SetPosSizePixel(Point(nTextWidth, -2), Size(aMySize.Width() - nTextWidth - 4, aMySize.Height()));
        // -2 because the edit has a frame which is 2 pixel wide ... should not be necessary, but I don't fully understand this ....

    // show the sub controls (in case they were just created)
    m_pForcedPrefix->Show();
    GetSubEdit()->Show();

    // do the real SetTex
    Edit::SetText(bIsEmpty ? _rStr : m_aTypeCollection.cutPrefix(_rStr), _rNewSelection);
}

//-------------------------------------------------------------------------
String OConnectionURLEdit::GetText() const
{
    if (m_pForcedPrefix)
        return m_pForcedPrefix->GetText() += Edit::GetText();
    return Edit::GetText();
}

//.........................................................................
}   // namespace dbaui
//.........................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.2  2000/10/18 16:00:47  fs
 *  use the system's DialogColor as background for the fixed text - looks much better
 *
 *  Revision 1.1  2000/10/05 09:59:38  fs
 *  edit control for connection urls
 *
 *
 *  Revision 1.0 28.09.00 13:12:21  fs
 ************************************************************************/

