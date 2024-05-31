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

#undef SC_DLLIMPLEMENTATION

#include <delcodlg.hxx>

bool       ScDeleteContentsDlg::bPreviousAllCheck = false;
InsertDeleteFlags ScDeleteContentsDlg::nPreviousChecks   = InsertDeleteFlags::DATETIME | InsertDeleteFlags::STRING  |
                                                 InsertDeleteFlags::NOTE     | InsertDeleteFlags::FORMULA |
                                                 InsertDeleteFlags::VALUE;

ScDeleteContentsDlg::ScDeleteContentsDlg(weld::Window* pParent)
    : GenericDialogController(pParent, u"modules/scalc/ui/deletecontents.ui"_ustr, u"DeleteContentsDialog"_ustr)
    , m_bObjectsDisabled(false)
    , m_xBtnDelAll(m_xBuilder->weld_check_button(u"deleteall"_ustr))
    , m_xBtnDelStrings(m_xBuilder->weld_check_button(u"text"_ustr))
    , m_xBtnDelNumbers(m_xBuilder->weld_check_button(u"numbers"_ustr))
    , m_xBtnDelDateTime(m_xBuilder->weld_check_button(u"datetime"_ustr))
    , m_xBtnDelFormulas(m_xBuilder->weld_check_button(u"formulas"_ustr))
    , m_xBtnDelNotes(m_xBuilder->weld_check_button(u"comments"_ustr))
    , m_xBtnDelAttrs(m_xBuilder->weld_check_button(u"formats"_ustr))
    , m_xBtnDelObjects(m_xBuilder->weld_check_button(u"objects"_ustr))
{
    m_xBtnDelAll->set_active( ScDeleteContentsDlg::bPreviousAllCheck );
    m_xBtnDelStrings->set_active( bool(InsertDeleteFlags::STRING & ScDeleteContentsDlg::nPreviousChecks) );
    m_xBtnDelNumbers->set_active( bool(InsertDeleteFlags::VALUE & ScDeleteContentsDlg::nPreviousChecks) );
    m_xBtnDelDateTime->set_active( bool(InsertDeleteFlags::DATETIME & ScDeleteContentsDlg::nPreviousChecks) );
    m_xBtnDelFormulas->set_active( bool(InsertDeleteFlags::FORMULA & ScDeleteContentsDlg::nPreviousChecks) );
    m_xBtnDelNotes->set_active( bool(InsertDeleteFlags::NOTE & ScDeleteContentsDlg::nPreviousChecks) );
    m_xBtnDelAttrs->set_active( (InsertDeleteFlags::ATTRIB & ScDeleteContentsDlg::nPreviousChecks) == InsertDeleteFlags::ATTRIB );
    m_xBtnDelObjects->set_active( bool(InsertDeleteFlags::OBJECTS & ScDeleteContentsDlg::nPreviousChecks) );

    DisableChecks( m_xBtnDelAll->get_active() );

    m_xBtnDelAll->connect_toggled( LINK( this, ScDeleteContentsDlg, DelAllHdl ) );
}

ScDeleteContentsDlg::~ScDeleteContentsDlg()
{
}

InsertDeleteFlags ScDeleteContentsDlg::GetDelContentsCmdBits() const
{
    ScDeleteContentsDlg::nPreviousChecks = InsertDeleteFlags::NONE;

    if ( m_xBtnDelStrings->get_active() )
        ScDeleteContentsDlg::nPreviousChecks  = InsertDeleteFlags::STRING;
    if ( m_xBtnDelNumbers->get_active() )
        ScDeleteContentsDlg::nPreviousChecks |= InsertDeleteFlags::VALUE;
    if ( m_xBtnDelDateTime->get_active())
        ScDeleteContentsDlg::nPreviousChecks |= InsertDeleteFlags::DATETIME;
    if ( m_xBtnDelFormulas->get_active())
        ScDeleteContentsDlg::nPreviousChecks |= InsertDeleteFlags::FORMULA;
    if ( m_xBtnDelNotes->get_active()   )
        ScDeleteContentsDlg::nPreviousChecks |= InsertDeleteFlags::NOTE;
    if ( m_xBtnDelAttrs->get_active()   )
        ScDeleteContentsDlg::nPreviousChecks |= InsertDeleteFlags::ATTRIB;
    if ( m_xBtnDelObjects->get_active() )
        ScDeleteContentsDlg::nPreviousChecks |= InsertDeleteFlags::OBJECTS;

    ScDeleteContentsDlg::bPreviousAllCheck = m_xBtnDelAll->get_active();

    return ( ScDeleteContentsDlg::bPreviousAllCheck
                ? InsertDeleteFlags::ALL
                : ScDeleteContentsDlg::nPreviousChecks );
}

void ScDeleteContentsDlg::DisableChecks( bool bDelAllChecked )
{
    if ( bDelAllChecked )
    {
        m_xBtnDelStrings->set_sensitive(false);
        m_xBtnDelNumbers->set_sensitive(false);
        m_xBtnDelDateTime->set_sensitive(false);
        m_xBtnDelFormulas->set_sensitive(false);
        m_xBtnDelNotes->set_sensitive(false);
        m_xBtnDelAttrs->set_sensitive(false);
        m_xBtnDelObjects->set_sensitive(false);
    }
    else
    {
        m_xBtnDelStrings->set_sensitive(true);
        m_xBtnDelNumbers->set_sensitive(true);
        m_xBtnDelDateTime->set_sensitive(true);
        m_xBtnDelFormulas->set_sensitive(true);
        m_xBtnDelNotes->set_sensitive(true);
        m_xBtnDelAttrs->set_sensitive(true);
        if (m_bObjectsDisabled)
            m_xBtnDelObjects->set_sensitive(false);
        else
            m_xBtnDelObjects->set_sensitive(true);
    }
}

void ScDeleteContentsDlg::DisableObjects()
{
    m_bObjectsDisabled = true;
    m_xBtnDelObjects->set_active(false);
    m_xBtnDelObjects->set_sensitive(false);
}

IMPL_LINK_NOARG(ScDeleteContentsDlg, DelAllHdl, weld::Toggleable&, void)
{
    DisableChecks( m_xBtnDelAll->get_active() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
