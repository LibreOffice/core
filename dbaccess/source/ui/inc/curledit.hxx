/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: curledit.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2007-05-10 10:31:27 $
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
    ODsnTypeCollection  m_aTypeCollection;
    FixedText*          m_pForcedPrefix;
    String              m_sSaveValueNoPrefix;
    BOOL                m_bShowPrefix; // when <TRUE> the prefix will be visible, otherwise not

public:
    OConnectionURLEdit(Window* pParent, const ResId& rResId,BOOL _bShowPrefix = FALSE);
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
    void ShowPrefix(BOOL _bShowPrefix);
    /// get the currently set text, excluding the prefix indicating the type
    virtual String  GetTextNoPrefix() const;
    /// set a new text, leave the current prefix unchanged
    virtual void    SetTextNoPrefix(const String& _rText);

    inline void     SaveValueNoPrefix()             { m_sSaveValueNoPrefix = GetTextNoPrefix(); }
    inline String   GetSavedValueNoPrefix() const   { return m_sSaveValueNoPrefix; }
    inline void     initializeTypeCollection(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB) { m_aTypeCollection.initUserDriverTypes(_rxORB); }
};

//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // _DBAUI_CURLEDIT_HXX_

