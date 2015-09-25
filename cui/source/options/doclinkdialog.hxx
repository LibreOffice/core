/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_CUI_SOURCE_OPTIONS_DOCLINKDIALOG_HXX
#define INCLUDED_CUI_SOURCE_OPTIONS_DOCLINKDIALOG_HXX

#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>
#include <vcl/button.hxx>
#include <svtools/inettbc.hxx>
#include <svtools/urlcontrol.hxx>


namespace svx
{

    /** dialog for editing document links associated with data sources
    */
    class ODocumentLinkDialog : public ModalDialog
    {
    protected:
        VclPtr< ::svt::OFileURLControl> m_pURL;
        VclPtr<PushButton>              m_pBrowseFile;
        VclPtr<Edit>                    m_pName;
        VclPtr<OKButton>                m_pOK;

        bool                            m_bCreatingNew;

        Link<const OUString&,bool>      m_aNameValidator;

    public:
        ODocumentLinkDialog( vcl::Window* _pParent, bool _bCreateNew );
        virtual ~ODocumentLinkDialog();
        virtual void dispose() SAL_OVERRIDE;

        // name validation has to be done by an external instance
        // the validator link gets a pointer to a String, and should return 0 if the string is not
        // acceptable
        void    setNameValidator( const Link<const OUString&,bool>& _rValidator ) { m_aNameValidator = _rValidator; }

        void    setLink( const  OUString& _rName, const   OUString& _rURL );
        void    getLink(        OUString& _rName,         OUString& _rURL ) const;

    protected:
        DECL_LINK(OnTextModified, void *);
        DECL_LINK_TYPED( OnBrowseFile, Button*, void );
        DECL_LINK_TYPED( OnOk, Button*, void );

        void validate( );
    };


}


#endif // INCLUDED_CUI_SOURCE_OPTIONS_DOCLINKDIALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
