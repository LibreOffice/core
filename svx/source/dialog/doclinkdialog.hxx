/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: doclinkdialog.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 21:01:28 $
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

