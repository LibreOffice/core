/*************************************************************************
 *
 *  $RCSfile: opendoccontrols.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2005-06-30 16:34:44 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef DBACCESS_SOURCE_UI_INC_OPENDOCCONTROLS_HXX
#define DBACCESS_SOURCE_UI_INC_OPENDOCCONTROLS_HXX

#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#include <map>

//........................................................................
namespace dbaui
{
//........................................................................

    //====================================================================
    //= OpenDocumentButton
    //====================================================================
    /** a button which can be used to open a document

        The text of the button is the same as for the "Open" command in the application
        UI. Additionally, the icon for this command is also displayed on the button.
    */
    class OpenDocumentButton : public PushButton
    {
    private:
        ::rtl::OUString     m_sModule;

    public:
        OpenDocumentButton( Window* _pParent, const sal_Char* _pAsciiModuleName, WinBits _nStyle = 0 );
        OpenDocumentButton( Window* _pParent, const sal_Char* _pAsciiModuleName, const ResId& _rResId );

    protected:
        void    impl_init( const sal_Char* _pAsciiModuleName );
    };

    //====================================================================
    //= OpenDocumentListBox
    //====================================================================
    class OpenDocumentListBox : public ListBox
    {
    private:
        typedef ::std::pair< String, String >       StringPair;
        typedef ::std::map< USHORT, StringPair >    MapIndexToStringPair;

        ::rtl::OUString         m_sModule;
        MapIndexToStringPair    m_aURLs;

    public:
        OpenDocumentListBox( Window* _pParent, const sal_Char* _pAsciiModuleName, WinBits _nStyle = WB_BORDER );
        OpenDocumentListBox( Window* _pParent, const sal_Char* _pAsciiModuleName, const ResId& _rResId );

        String  GetSelectedDocumentURL() const;
        String  GetSelectedDocumentFilter() const;

    protected:
        virtual void        RequestHelp( const HelpEvent& _rHEvt );

        StringPair  impl_getDocumentAtIndex( USHORT _nListIndex, bool _bSystemNotation = false ) const;

    private:
        void    impl_init( const sal_Char* _pAsciiModuleName );
    };

//........................................................................
} // namespace dbaui
//........................................................................

#endif // DBACCESS_SOURCE_UI_INC_OPENDOCCONTROLS_HXX

