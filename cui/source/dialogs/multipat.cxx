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

#include <sal/config.h>

#include <osl/file.hxx>
#include <sfx2/filedlghelper.hxx>
#include <tools/urlobj.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>

#include <multipat.hxx>
#include <dialmgr.hxx>

#include <strings.hrc>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/ui/dialogs/XFolderPicker2.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>

#include <unotools/pathoptions.hxx>
#include <o3tl/string_view.hxx>

using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::uno;

IMPL_LINK_NOARG(SvxMultiPathDialog, SelectHdl_Impl, weld::TreeView&, void)
{
    auto nCount = m_xRadioLB->n_children();
    bool bIsSelected = m_xRadioLB->get_selected_index() != -1;
    bool bEnable = nCount > 1;
    m_xDelBtn->set_sensitive(bEnable && bIsSelected);
}

IMPL_LINK_NOARG(SvxPathSelectDialog, SelectHdl_Impl, weld::TreeView&, void)
{
    auto nCount = m_xPathLB->n_children();
    bool bIsSelected = m_xPathLB->get_selected_index() != -1;
    bool bEnable = nCount > 1;
    m_xDelBtn->set_sensitive(bEnable && bIsSelected);
}

void SvxMultiPathDialog::HandleEntryChecked(int nRow)
{
    m_xRadioLB->select(nRow);
    bool bChecked = m_xRadioLB->get_toggle(nRow) == TRISTATE_TRUE;
    if (bChecked)
    {
        // we have radio button behavior -> so uncheck the other entries
        int nCount = m_xRadioLB->n_children();
        for (int i = 0; i < nCount; ++i)
        {
            if (i != nRow)
                m_xRadioLB->set_toggle(i, TRISTATE_FALSE);
        }
    }
}

IMPL_LINK(SvxMultiPathDialog, CheckHdl_Impl, const weld::TreeView::iter_col&, rRowCol, void)
{
    HandleEntryChecked(m_xRadioLB->get_iter_index_in_parent(rRowCol.first));
}

void SvxMultiPathDialog::AppendEntry(const OUString& rText, const OUString& rId)
{
    m_xRadioLB->append();
    const int nRow = m_xRadioLB->n_children() - 1;
    m_xRadioLB->set_toggle(nRow, TRISTATE_FALSE);
    m_xRadioLB->set_text(nRow, rText, 0);
    m_xRadioLB->set_id(nRow, rId);
}

IMPL_LINK_NOARG(SvxMultiPathDialog, AddHdl_Impl, weld::Button&, void)
{
    Reference < XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
    Reference < XFolderPicker2 >  xFolderPicker = sfx2::createFolderPicker(xContext, m_xDialog.get());

    if ( xFolderPicker->execute() != ExecutableDialogResults::OK )
        return;

    INetURLObject aPath( xFolderPicker->getDirectory() );
    aPath.removeFinalSlash();
    OUString aURL = aPath.GetMainURL( INetURLObject::DecodeMechanism::NONE );
    OUString sInsPath;
    osl::FileBase::getSystemPathFromFileURL(aURL, sInsPath);

    if (m_xRadioLB->find_text(sInsPath) != -1)
    {
        OUString sMsg( CuiResId( RID_MULTIPATH_DBL_ERR ) );
        sMsg = sMsg.replaceFirst( "%1", sInsPath );
        std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                      VclMessageType::Info, VclButtonsType::Ok, sMsg));
        xInfoBox->run();
    }
    else
    {
        AppendEntry(sInsPath, aURL);
    }

    SelectHdl_Impl(*m_xRadioLB);
}

IMPL_LINK_NOARG(SvxPathSelectDialog, AddHdl_Impl, weld::Button&, void)
{
    Reference < XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
    Reference < XFolderPicker2 >  xFolderPicker = sfx2::createFolderPicker(xContext, m_xDialog.get());

    if ( xFolderPicker->execute() != ExecutableDialogResults::OK )
        return;

    INetURLObject aPath( xFolderPicker->getDirectory() );
    aPath.removeFinalSlash();
    OUString aURL = aPath.GetMainURL( INetURLObject::DecodeMechanism::NONE );
    OUString sInsPath;
    osl::FileBase::getSystemPathFromFileURL(aURL, sInsPath);

    if (m_xPathLB->find_text(sInsPath) != -1)
    {
        OUString sMsg( CuiResId( RID_MULTIPATH_DBL_ERR ) );
        sMsg = sMsg.replaceFirst( "%1", sInsPath );
        std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                      VclMessageType::Info, VclButtonsType::Ok, sMsg));
        xInfoBox->run();
    }
    else
    {
        m_xPathLB->append(aURL, sInsPath);
    }

    SelectHdl_Impl(*m_xPathLB);
}

IMPL_LINK_NOARG(SvxMultiPathDialog, DelHdl_Impl, weld::Button&, void)
{
    int nPos = m_xRadioLB->get_selected_index();
    bool bChecked = m_xRadioLB->get_toggle(nPos) == TRISTATE_TRUE;
    m_xRadioLB->remove(nPos);
    int nCnt = m_xRadioLB->n_children();
    if (nCnt)
    {
        --nCnt;

        if ( nPos > nCnt )
            nPos = nCnt;
        if (bChecked)
        {
            m_xRadioLB->set_toggle(nPos, TRISTATE_TRUE);
            HandleEntryChecked(nPos);
        }
        m_xRadioLB->select(nPos);
    }

    SelectHdl_Impl(*m_xRadioLB);
}

IMPL_LINK_NOARG(SvxPathSelectDialog, DelHdl_Impl, weld::Button&, void)
{
    int nPos = m_xPathLB->get_selected_index();
    m_xPathLB->remove(nPos);
    int nCnt = m_xPathLB->n_children();

    if (nCnt)
    {
        --nCnt;

        if ( nPos > nCnt )
            nPos = nCnt;
        m_xPathLB->select(nPos);
    }

    SelectHdl_Impl(*m_xPathLB);
}

SvxMultiPathDialog::SvxMultiPathDialog(weld::Window* pParent)
    : GenericDialogController(pParent, u"cui/ui/multipathdialog.ui"_ustr, u"MultiPathDialog"_ustr)
    , m_xRadioLB(m_xBuilder->weld_tree_view(u"paths"_ustr))
    , m_xAddBtn(m_xBuilder->weld_button(u"add"_ustr))
    , m_xDelBtn(m_xBuilder->weld_button(u"delete"_ustr))
{
    m_xRadioLB->set_size_request(m_xRadioLB->get_approximate_digit_width() * 60,
                                 m_xRadioLB->get_text_height() * 10);
    m_xRadioLB->enable_toggle_buttons(weld::ColumnToggleType::Radio);
    m_xRadioLB->connect_toggled(LINK(this, SvxMultiPathDialog, CheckHdl_Impl));
    m_xRadioLB->connect_changed(LINK(this, SvxMultiPathDialog, SelectHdl_Impl));

    m_xAddBtn->connect_clicked(LINK(this, SvxMultiPathDialog, AddHdl_Impl));
    m_xDelBtn->connect_clicked(LINK(this, SvxMultiPathDialog, DelHdl_Impl));

    SelectHdl_Impl(*m_xRadioLB);
}

SvxPathSelectDialog::SvxPathSelectDialog(weld::Window* pParent)
    : GenericDialogController(pParent, u"cui/ui/selectpathdialog.ui"_ustr, u"SelectPathDialog"_ustr)
    , m_xPathLB(m_xBuilder->weld_tree_view(u"paths"_ustr))
    , m_xAddBtn(m_xBuilder->weld_button(u"add"_ustr))
    , m_xDelBtn(m_xBuilder->weld_button(u"delete"_ustr))
{
    m_xPathLB->set_size_request(m_xPathLB->get_approximate_digit_width() * 60,
                                m_xPathLB->get_text_height() * 10);

    m_xPathLB->connect_changed(LINK(this, SvxPathSelectDialog, SelectHdl_Impl));
    m_xAddBtn->connect_clicked(LINK(this, SvxPathSelectDialog, AddHdl_Impl));
    m_xDelBtn->connect_clicked(LINK(this, SvxPathSelectDialog, DelHdl_Impl));

    SelectHdl_Impl(*m_xPathLB);
}

SvxMultiPathDialog::~SvxMultiPathDialog()
{
}

OUString SvxMultiPathDialog::GetPath() const
{
    OUStringBuffer sNewPath;
    sal_Unicode cDelim = SVT_SEARCHPATH_DELIMITER;

    OUString sWritable;
    for (int i = 0, nCount = m_xRadioLB->n_children(); i < nCount; ++i)
    {
        if (m_xRadioLB->get_toggle(i) == TRISTATE_TRUE)
            sWritable = m_xRadioLB->get_id(i);
        else
        {
            if (!sNewPath.isEmpty())
                sNewPath.append(cDelim);
            sNewPath.append(m_xRadioLB->get_id(i));
        }
    }
    if (!sNewPath.isEmpty())
        sNewPath.append(cDelim);
    sNewPath.append(sWritable);

    return sNewPath.makeStringAndClear();
}

OUString SvxPathSelectDialog::GetPath() const
{
    OUStringBuffer sNewPath;

    for (int i = 0; i < m_xPathLB->n_children(); ++i)
    {
        if ( !sNewPath.isEmpty() )
            sNewPath.append(SVT_SEARCHPATH_DELIMITER);
        sNewPath.append( m_xPathLB->get_id(i));
    }

    return sNewPath.makeStringAndClear();
}

void SvxMultiPathDialog::SetPath( std::u16string_view rPath )
{
    if ( !rPath.empty() )
    {
        const sal_Unicode cDelim = SVT_SEARCHPATH_DELIMITER;
        int nCount = 0;
        sal_Int32 nIndex = 0;
        do
        {
            const OUString sPath( o3tl::getToken(rPath, 0, cDelim, nIndex ) );
            OUString sSystemPath;
            bool bIsSystemPath =
                osl::FileBase::getSystemPathFromFileURL(sPath, sSystemPath) == osl::FileBase::E_None;

            const OUString sEntry((bIsSystemPath ? sSystemPath : sPath));
            AppendEntry(sEntry, sPath);
            ++nCount;
        }
        while (nIndex >= 0);

        if (nCount)
        {
            m_xRadioLB->set_toggle(nCount - 1, TRISTATE_TRUE);
            HandleEntryChecked(nCount - 1);
        }
    }

    SelectHdl_Impl(*m_xRadioLB);
}

void SvxPathSelectDialog::SetPath(std::u16string_view rPath)
{
    if ( !rPath.empty() )
    {
        sal_Int32 nIndex = 0;
        do
        {
            const OUString sPath( o3tl::getToken(rPath, 0, SVT_SEARCHPATH_DELIMITER, nIndex ) );
            OUString sSystemPath;
            bool bIsSystemPath =
                osl::FileBase::getSystemPathFromFileURL(sPath, sSystemPath) == osl::FileBase::E_None;

            m_xPathLB->append(sPath, bIsSystemPath ? sSystemPath : sPath);
        }
        while (nIndex >= 0);
    }

    SelectHdl_Impl(*m_xPathLB);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
