/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright IBM Corporation 2009, 2010.
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

#include "vbafiledialog.hxx"
#include "comphelper/processfactory.hxx"
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker2.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include "tools/urlobj.hxx"


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;


ScVbaFileDialog::ScVbaFileDialog( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > &xContext, const css::uno::Reference< css::frame::XModel >& xModel )
:   ScVbaFileDialog_BASE( xParent, xContext, xModel )
{
    m_pFileDialogSelectedItems = new VbaFileDialogSelectedItems(xParent, xContext, (com::sun::star::container::XIndexAccess *)&m_FileDialogSelectedObj);
}

ScVbaFileDialog::~ScVbaFileDialog()
{
    if (m_pFileDialogSelectedItems != NULL)
    {
        delete m_pFileDialogSelectedItems;
    }
}

rtl::OUString&
ScVbaFileDialog::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaFileDialog") );
    return sImplName;
}

css::uno::Sequence<rtl::OUString>
ScVbaFileDialog::getServiceNames()
{
    static Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.excel.FileDialog" ) );
    }
    return aServiceNames;

}

css::uno::Reference< ov::XFileDialogSelectedItems > SAL_CALL
ScVbaFileDialog::getSelectedItems() throw (css::uno::RuntimeException)
{
    css::uno::Reference< ov::XFileDialogSelectedItems > xFileDlgSlc = (ov::XFileDialogSelectedItems *)m_pFileDialogSelectedItems;
    return xFileDlgSlc;
}

::sal_Int32 SAL_CALL
ScVbaFileDialog::Show( ) throw (::com::sun::star::uno::RuntimeException)
{
    // Returns an Integer indicating if user pressed "Open" button(-1) or "Cancel" button(0).
    sal_Int32 nResult = -1;
    try
    {
        m_sSelectedItems.realloc(0);

        const ::rtl::OUString sServiceName(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.ui.dialogs.FilePicker" ));

        Reference< lang::XMultiServiceFactory > xMSF( comphelper::getProcessServiceFactory(), uno::UNO_QUERY );
        // Set the type of File Picker Dialog: TemplateDescription::FILEOPEN_SIMPLE.
        Sequence< uno::Any > aDialogType( 1 );
        aDialogType[0] <<= ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE;
        Reference < ui::dialogs::XFilePicker > xFilePicker( xMSF->createInstanceWithArguments( sServiceName, aDialogType ), UNO_QUERY );
        Reference < ui::dialogs::XFilePicker2 > xFilePicker2( xFilePicker, UNO_QUERY );
        if ( xFilePicker.is() )
        {
            xFilePicker->setMultiSelectionMode(sal_True);
            if ( xFilePicker->execute() )
            {
                sal_Bool bUseXFilePicker2 = false;
                Reference< lang::XServiceInfo > xServiceInfo( xFilePicker, UNO_QUERY );
                if (xServiceInfo.is())
                {
                    rtl::OUString sImplName = xServiceInfo->getImplementationName();
                    if (sImplName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("com.sun.star.comp.fpicker.VistaFileDialog")) ||
                        sImplName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("com.sun.star.ui.dialogs.SalGtkFilePicker")))
                    {
                        bUseXFilePicker2 = sal_True;
                    }
                }
                if ( bUseXFilePicker2 && xFilePicker2.is() )
                {
                    // On Linux, XFilePicker->getFiles() always return one selected file although we select
                    // more than one file, also on Vista XFilePicker->getFiles() does not work well too,
                    // so we call XFilePicker2->getSelectedFiles() to get selected files.
                    m_sSelectedItems = xFilePicker2->getSelectedFiles();
                }
                else
                {
                    // If only one file is selected, the first entry of the sequence contains the complete path/filename in
                    // URL format. If multiple files are selected, the first entry of the sequence contains the path in URL
                    // format, and the other entries contains the names of the selected files without path information.
                    Sequence< rtl::OUString > aSelectedFiles = xFilePicker->getFiles();
                    sal_Int32 iFileCount = aSelectedFiles.getLength();
                    if ( iFileCount > 1 )
                    {
                        m_sSelectedItems.realloc( iFileCount - 1 );
                        INetURLObject aPath( aSelectedFiles[0] );
                        aPath.setFinalSlash();
                        for ( sal_Int32 i = 1; i < iFileCount; i++ )
                        {
                            if ( aSelectedFiles[i].indexOf ('/') > 0 || aSelectedFiles[i].indexOf ('\\') > 0 )
                            {
                                m_sSelectedItems[i - 1] = aSelectedFiles[i];
                            }
                            else
                            {
                                if ( i == 1 )
                                    aPath.Append( aSelectedFiles[i] );
                                else
                                    aPath.setName( aSelectedFiles[i] );
                                m_sSelectedItems[i - 1] = aPath.GetMainURL(INetURLObject::NO_DECODE);
                            }
                        }
                    }
                    else if ( iFileCount == 1 )
                    {
                        m_sSelectedItems = aSelectedFiles;
                    }
                }

                sal_Int32 iFileCount = m_sSelectedItems.getLength();
                rtl::OUString aTemp;
                for ( sal_Int32 i = 0; i < iFileCount; i++ )
                {
                    INetURLObject aObj( m_sSelectedItems[i] );
                    if ( aObj.GetProtocol() == INET_PROT_FILE )
                    {
                        aTemp = aObj.PathToFileName();
                        m_sSelectedItems[i] = aTemp.getLength() > 0 ? aTemp : m_sSelectedItems[i];
                    }
                }
            }
            else
            {
                nResult = 0;
            }
        }

        m_FileDialogSelectedObj.SetSelectedFile(m_sSelectedItems);
    }
    catch( const uno::Exception& )
    {
        return 0;
    }

    return nResult;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
