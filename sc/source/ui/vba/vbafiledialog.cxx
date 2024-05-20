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

#include "vbafiledialog.hxx"
#include "vbafiledialogitems.hxx"

#include <osl/file.hxx>

#include <ooo/vba/office/MsoFileDialogType.hpp>

#include <com/sun/star/ui/dialogs/FilePicker.hpp>
#include <com/sun/star/ui/dialogs/FolderPicker.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>

using namespace ::com::sun::star;
using namespace ::ooo::vba;

ScVbaFileDialog::ScVbaFileDialog( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const sal_Int32 nType )
    : ScVbaFileDialog_BASE( xParent, xContext)
    , m_nType(nType)
    , m_sTitle(u"FileDialog"_ustr)
    , m_bMultiSelectMode(false)
{}

uno::Any
ScVbaFileDialog::getInitialFileName() { return uno::Any( m_sInitialFileName ); }

void ScVbaFileDialog::setInitialFileName( const css::uno::Any& rName )
{
    OUString sDefaultPath;

    if( rName >>= sDefaultPath )
    {
        OUString sDefaultURL;
        sal_Int32 eSuccess = osl::FileBase::getFileURLFromSystemPath(
                sDefaultPath, sDefaultURL ) ;
        if( eSuccess == osl::FileBase::RC::E_INVAL )
            m_sInitialFileName = sDefaultPath; // the user may gave it in URL form
        else
            m_sInitialFileName = sDefaultURL;
    }
}

css::uno::Any ScVbaFileDialog::getTitle() { return uno::Any( m_sTitle ); }

void ScVbaFileDialog::setTitle( const css::uno::Any& rTitle )
{
    rTitle >>= m_sTitle;
}

uno::Any ScVbaFileDialog::getAllowMultiSelect()
{
    return uno::Any(m_bMultiSelectMode);
}

void ScVbaFileDialog::setAllowMultiSelect(const uno::Any& rAllowMultiSelect)
{
    rAllowMultiSelect >>= m_bMultiSelectMode;
}

uno::Reference< excel::XFileDialogSelectedItems > SAL_CALL ScVbaFileDialog::getSelectedItems()
{
    // TODO use InitialFileName when m_xItems is empty
    return m_xItems;
}

sal_Int32 ScVbaFileDialog::Show()
{
    std::vector<OUString> sSelectedPaths;
    sal_Int32 nRet = -1;

    switch (m_nType)
    {
        case office::MsoFileDialogType::msoFileDialogOpen:
            // TODO implement
            break;
        case office::MsoFileDialogType::msoFileDialogSaveAs:
            // TODO implement
            break;
        case office::MsoFileDialogType::msoFileDialogFilePicker:
            {
                uno::Reference<ui::dialogs::XFilePicker3> xFilePicker =
                        ui::dialogs::FilePicker::createWithMode(
                                mxContext, ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE );

                if( !m_sInitialFileName.isEmpty() )
                    xFilePicker->setDisplayDirectory( m_sInitialFileName );

                if( xFilePicker->execute() != ui::dialogs::ExecutableDialogResults::OK )
                {
                    nRet = 0; // cancel pressed
                    break;
                }

                const uno::Sequence<OUString> aSelectedFiles = xFilePicker->getSelectedFiles();
                for( const auto& sURL : aSelectedFiles )
                {
                    OUString sPath;
                    osl::FileBase::getSystemPathFromFileURL(sURL, sPath);

                    sSelectedPaths.push_back(sPath);
                }
            }
            break;
        case office::MsoFileDialogType::msoFileDialogFolderPicker:
            {
                uno::Reference< ui::dialogs::XFolderPicker2 > xFolderPicker =
                        ui::dialogs::FolderPicker::create(mxContext);

                if( !m_sInitialFileName.isEmpty() )
                    xFolderPicker->setDisplayDirectory( m_sInitialFileName );

                if( xFolderPicker->execute() != ui::dialogs::ExecutableDialogResults::OK )
                {
                    nRet = 0; // cancel pressed
                    break;
                }

                OUString sURL = xFolderPicker->getDirectory();

                if(!sURL.isEmpty())
                {
                    OUString sPath;
                    osl::FileBase::getSystemPathFromFileURL(sURL, sPath);

                    sSelectedPaths.push_back(sPath);
                }

            }
            break;
        default:
            throw uno::RuntimeException();
    }

    m_xItems = css::uno::Reference< ov::excel::XFileDialogSelectedItems >(
            new ScVbaFileDialogSelectedItems(this, mxContext, std::move(sSelectedPaths)) );
    return nRet;
}

// XHelperInterface
OUString
ScVbaFileDialog::getServiceImplName()
{
    return u"ScVbaFileDialog"_ustr;
}

uno::Sequence<OUString>
ScVbaFileDialog::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        u"ooo.vba.FileDialog"_ustr
    };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
