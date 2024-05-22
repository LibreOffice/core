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
#include <ooo/vba/word/XDialog.hpp>
#include "vbadialogs.hxx"
#include "vbadialog.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;

uno::Any
SwVbaDialogs::Item( const uno::Any &aItem )
{
    sal_Int32 nIndex = 0;
    aItem >>= nIndex;
    uno::Reference< word::XDialog > aDialog( new SwVbaDialog( uno::Reference< XHelperInterface >( Application(),uno::UNO_QUERY_THROW ), mxContext, m_xModel, nIndex ) );
    return uno::Any( aDialog );
}

OUString
SwVbaDialogs::getServiceImplName()
{
    return u"SwVbaDialogs"_ustr;
}

uno::Sequence< OUString >
SwVbaDialogs::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        u"ooo.vba.word.Dialogs"_ustr
    };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
