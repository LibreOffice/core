/*************************************************************************
 *
 *  $RCSfile: doclinkdialog.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 16:39:02 $
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

#ifndef _SVX_DOCLINKDIALOG_HXX_
#define _SVX_DOCLINKDIALOG_HXX_

#ifndef _SV_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_EDIT_HXX
#include <vcl/edit.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SVTOOLS_INETTBC_HXX
#include <svtools/inettbc.hxx>
#endif
#ifndef SVTOOLS_URLCONTROL_HXX
#include <svtools/urlcontrol.hxx>
#endif

//......................................................................
namespace svx
{
//......................................................................

    //==================================================================
    //= ODocumentLinkDialog
    //==================================================================
    /** dialog for editing document links associated with data sources
    */
    class ODocumentLinkDialog : public ModalDialog
    {
    protected:
        FixedText               m_aURLLabel;
        ::svt::OFileURLControl  m_aURL;
        PushButton              m_aBrowseFile;
        FixedText               m_aNameLabel;
        Edit                    m_aName;

        FixedLine               m_aBottomLine;
        OKButton                m_aOK;
        CancelButton            m_aCancel;
        HelpButton              m_aHelp;

        sal_Bool                m_bCreatingNew;

        Link                    m_aNameValidator;

    public:
        ODocumentLinkDialog( Window* _pParent, sal_Bool _bCreateNew );

        // name validation has to be done by an external instance
        // the validator link gets a pointer to a String, and should return 0 if the string is not
        // acceptable
        void    setNameValidator( const Link& _rValidator ) { m_aNameValidator = _rValidator; }
        Link    getNameValidator( ) const { return m_aNameValidator; }

        void    set( const  String& _rName, const   String& _rURL );
        void    get(        String& _rName,         String& _rURL ) const;

    protected:
        DECL_LINK( OnTextModified, Control* );
        DECL_LINK( OnBrowseFile, void* );
        DECL_LINK( OnOk, void* );

        void validate( );
    };

//......................................................................
}   // namespace svx
//......................................................................

#endif // _SVX_DOCLINKDIALOG_HXX_

