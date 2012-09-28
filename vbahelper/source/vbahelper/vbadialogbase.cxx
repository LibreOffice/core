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
#include <vbahelper/vbadialogbase.hxx>
#include <vbahelper/vbahelper.hxx>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

void SAL_CALL VbaDialogBase::Show() throw ( uno::RuntimeException )
{
    OUString aURL;
    if ( m_xModel.is() )
    {
        aURL = mapIndexToName( mnIndex );
        if( aURL.isEmpty() )
            throw uno::RuntimeException(
                " Unable to open the specified dialog ",
                uno::Reference< XInterface > () );

        uno::Sequence< beans::PropertyValue > dispatchProps(0);
        dispatchRequests( m_xModel, aURL, dispatchProps );
    }
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
