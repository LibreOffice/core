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

#ifndef _PAD_HELPER_HXX_
#define _PAD_HELPER_HXX_

#include <list>
#include <rtl/ustring.hxx>
#include <tools/resid.hxx>
#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/combobox.hxx>

#if defined SPA_DLLIMPLEMENTATION
#define SPA_DLLPUBLIC SAL_DLLPUBLIC_EXPORT
#else
#define SPA_DLLPUBLIC SAL_DLLPUBLIC_IMPORT
#endif


class Config;

namespace padmin
{
class DelMultiListBox : public MultiListBox
{
    Link            m_aDelPressedLink;
public:
    DelMultiListBox( Window* pParent, const ResId& rResId ) :
            MultiListBox( pParent, rResId ) {}
    ~DelMultiListBox() {}

    virtual bool Notify( NotifyEvent& rEvent );

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

    virtual bool Notify( NotifyEvent& rEvent );

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
    OKButton*    m_pOKButton;
    FixedText*   m_pFixedText;
    Edit*        m_pEdit;
    OUString&    m_rReturnValue;

    DECL_LINK( ClickBtnHdl, Button* );

public:
    QueryString(Window*, OUString &, OUString &);
    // parent window, Query text, initial value
    ~QueryString();
};

sal_Bool AreYouSure( Window*, int nRid = -1 );

ResId PaResId( sal_uInt32 nId );

void FindFiles( const OUString& rDirectory, ::std::list< OUString >& rResult, const OUString& rSuffixes, bool bRecursive = false );
Config& getPadminRC();
void freePadminRC();

bool chooseDirectory( OUString& rInOutPath );

} // namespace padmin

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
