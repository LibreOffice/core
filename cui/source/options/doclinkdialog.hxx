/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _SVX_DOCLINKDIALOG_HXX_
#define _SVX_DOCLINKDIALOG_HXX_

#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>
#include <vcl/button.hxx>
#include <svtools/inettbc.hxx>
#include <svtools/urlcontrol.hxx>

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
