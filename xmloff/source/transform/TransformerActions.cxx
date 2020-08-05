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

#include "TransformerActions.hxx"

#include <osl/diagnose.h>


using namespace ::osl;
using namespace ::xmloff::token;
using namespace ::com::sun::star::uno;

XMLTransformerActions::XMLTransformerActions( XMLTransformerActionInit const *pInit )
{
    if( !pInit )
        return;

    XMLTransformerActions::key_type aKey;
    XMLTransformerActions::mapped_type aData;
    while( pInit->m_nActionType != XML_TACTION_EOT )
    {
        aKey.m_nPrefix = pInit->m_nPrefix;
        aKey.SetLocalName( pInit->m_eLocalName );

        OSL_ENSURE( find( aKey ) == end(), "duplicate action map entry" );

        aData.m_nActionType  = pInit->m_nActionType;
        aData.m_nParam1 = pInit->m_nParam1;
        aData.m_nParam2 = pInit->m_nParam2;
        aData.m_nParam3 = pInit->m_nParam3;
        XMLTransformerActions::value_type aVal( aKey, aData );

        insert( aVal );
        ++pInit;
    }
}

XMLTransformerActions::~XMLTransformerActions()
{
}

void XMLTransformerActions::Add( XMLTransformerActionInit const *pInit )
{
    if( !pInit )
        return;

    XMLTransformerActions::key_type aKey;
    XMLTransformerActions::mapped_type aData;
    while( pInit->m_nActionType != XML_TACTION_EOT )
    {
        aKey.m_nPrefix = pInit->m_nPrefix;
        aKey.SetLocalName( pInit->m_eLocalName );
        XMLTransformerActions::iterator aIter = find( aKey );
        if( aIter == end() )
        {
            aData.m_nActionType  = pInit->m_nActionType;
            aData.m_nParam1 = pInit->m_nParam1;
            aData.m_nParam2 = pInit->m_nParam2;
            aData.m_nParam3 = pInit->m_nParam3;
            XMLTransformerActions::value_type aVal( aKey, aData );
            insert( aVal );
        }

        ++pInit;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
