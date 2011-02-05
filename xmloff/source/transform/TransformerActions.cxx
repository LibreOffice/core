/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"
#include "TransformerActions.hxx"


using ::rtl::OUString;

using namespace ::osl;
using namespace ::xmloff::token;
using namespace ::com::sun::star::uno;

XMLTransformerActions::XMLTransformerActions( XMLTransformerActionInit *pInit )
{
    if( pInit )
    {
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
}

XMLTransformerActions::~XMLTransformerActions()
{
}

void XMLTransformerActions::Add( XMLTransformerActionInit *pInit )
{
    if( pInit )
    {
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
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
