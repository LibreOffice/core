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

#include "OfficeFolderPicker.hxx"

#include "iodlg.hxx"

#include <list>
#include <tools/urlobj.hxx>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/container/XSet.hpp>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <unotools/pathoptions.hxx>

using namespace     ::com::sun::star::container;
using namespace     ::com::sun::star::lang;
using namespace     ::com::sun::star::uno;
using namespace     ::com::sun::star::beans;

SvtFolderPicker::SvtFolderPicker()
{
}

SvtFolderPicker::~SvtFolderPicker()
{
}

void SAL_CALL SvtFolderPicker::setTitle( const OUString& _rTitle )
{
    OCommonPicker::setTitle( _rTitle );
}

sal_Int16 SAL_CALL SvtFolderPicker::execute(  )
{
    return OCommonPicker::execute();
}

void SAL_CALL SvtFolderPicker::setDialogTitle( const OUString& _rTitle)
{
    setTitle( _rTitle );
}

void SAL_CALL SvtFolderPicker::startExecuteModal( const Reference< css::ui::dialogs::XDialogClosedListener >& xListener )
{
    m_xListener = xListener;
    prepareDialog();
    prepareExecute();
    SvtFileDialog_Base* pDialog = getDialog();
    pDialog->EnableAutocompletion();
    pDialog->StartExecuteAsync([this](sal_Int32 nResult){
        DialogClosedHdl(nResult);
    });
}

VclPtr<SvtFileDialog_Base> SvtFolderPicker::implCreateDialog( vcl::Window* _pParent )
{
    return VclPtr<SvtFileDialog>::Create( _pParent, PickerFlags::PathDialog );
}

sal_Int16 SvtFolderPicker::implExecutePicker( )
{
    prepareExecute();

    // now we are ready to execute the dialog
    getDialog()->EnableAutocompletion( false );
    sal_Int16 nRet = getDialog()->Execute();

    return nRet;
}

void SvtFolderPicker::prepareExecute()
{
    // set the default directory
    if ( !m_aDisplayDirectory.isEmpty() )
        getDialog()->SetPath( m_aDisplayDirectory );
    else
    {
        // set the default standard dir
        INetURLObject aStdDirObj( SvtPathOptions().GetWorkPath() );
        getDialog()->SetPath( aStdDirObj.GetMainURL( INetURLObject::DecodeMechanism::NONE) );
    }
}

void SvtFolderPicker::DialogClosedHdl(sal_Int32 nResult)
{
    if ( m_xListener.is() )
    {
        sal_Int16 nRet = static_cast<sal_Int16>(nResult);
        css::ui::dialogs::DialogClosedEvent aEvent( *this, nRet );
        m_xListener->dialogClosed( aEvent );
        m_xListener.clear();
    }
}

void SAL_CALL SvtFolderPicker::setDisplayDirectory( const OUString& aDirectory )
{
    m_aDisplayDirectory = aDirectory;
}

OUString SAL_CALL SvtFolderPicker::getDisplayDirectory()
{
    if ( ! getDialog() )
        return m_aDisplayDirectory;

    std::vector<OUString> aPathList(getDialog()->GetPathList());

    if(!aPathList.empty())
        return aPathList[0];

    return OUString();
}

OUString SAL_CALL SvtFolderPicker::getDirectory()
{
    if ( ! getDialog() )
        return m_aDisplayDirectory;

    std::vector<OUString> aPathList(getDialog()->GetPathList());

    if(!aPathList.empty())
        return aPathList[0];

    return OUString();
}

void SAL_CALL SvtFolderPicker::setDescription( const OUString& )
{
}

void SvtFolderPicker::cancel()
{
    OCommonPicker::cancel();
}

/* XServiceInfo */
OUString SAL_CALL SvtFolderPicker::getImplementationName()
{
    return impl_getStaticImplementationName();
}

/* XServiceInfo */
sal_Bool SAL_CALL SvtFolderPicker::supportsService( const OUString& sServiceName )
{
    return cppu::supportsService(this, sServiceName);
}

/* XServiceInfo */
Sequence< OUString > SAL_CALL SvtFolderPicker::getSupportedServiceNames()
{
    return impl_getStaticSupportedServiceNames();
}

/* Helper for XServiceInfo */
Sequence< OUString > SvtFolderPicker::impl_getStaticSupportedServiceNames()
{
    Sequence< OUString > seqServiceNames { "com.sun.star.ui.dialogs.OfficeFolderPicker" };
    return seqServiceNames ;
}

/* Helper for XServiceInfo */
OUString SvtFolderPicker::impl_getStaticImplementationName()
{
    return OUString( "com.sun.star.svtools.OfficeFolderPicker" );
}

/* Helper for registry */
Reference< XInterface > SvtFolderPicker::impl_createInstance( const Reference< XComponentContext >&  )
{
    return Reference< XInterface >( *new SvtFolderPicker );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
