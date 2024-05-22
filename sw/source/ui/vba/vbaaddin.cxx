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
#include "vbaaddin.hxx"
#include <tools/urlobj.hxx>
#include <osl/file.hxx>
#include <utility>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

SwVbaAddin::SwVbaAddin( const uno::Reference< ooo::vba::XHelperInterface >& rParent, const uno::Reference< uno::XComponentContext >& rContext, OUString aFileURL ) :
    SwVbaAddin_BASE( rParent, rContext ), msFileURL(std::move( aFileURL )), mbInstalled( true )
{
}

SwVbaAddin::~SwVbaAddin()
{
}

OUString SAL_CALL SwVbaAddin::getName()
{
    OUString sName;
    INetURLObject aURL( msFileURL );
    ::osl::File::getSystemPathFromFileURL( aURL.GetLastName(), sName );
    return sName;
}

void SAL_CALL
SwVbaAddin::setName( const OUString& )
{
    throw uno::RuntimeException(u" Fail to set name"_ustr );
}

OUString SAL_CALL SwVbaAddin::getPath()
{
    INetURLObject aURL( msFileURL );
    aURL.CutLastName();
    return aURL.GetURLPath();
}

sal_Bool SAL_CALL SwVbaAddin::getAutoload()
{
    return true;
}

sal_Bool SAL_CALL SwVbaAddin::getInstalled()
{
    return mbInstalled;
}

void SAL_CALL SwVbaAddin::setInstalled( sal_Bool _installed )
{
    if( bool(_installed) != mbInstalled )
    {
        mbInstalled = _installed;
        // TODO: should call AutoExec and AutoExit etc.
    }
}

OUString
SwVbaAddin::getServiceImplName()
{
    return u"SwVbaAddin"_ustr;
}

uno::Sequence< OUString >
SwVbaAddin::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        u"ooo.vba.word.Addin"_ustr
    };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
