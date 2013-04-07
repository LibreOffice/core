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


#include <cstdio>
#include <unotools/configitem.hxx>

#include "X11_selection.hxx"

#define SETTINGS_CONFIGNODE "VCL/Settings/Transfer"
#define SELECTION_PROPERTY "SelectionTimeout"

namespace x11
{

class DtransX11ConfigItem : public ::utl::ConfigItem
{
    sal_Int32           m_nSelectionTimeout;

    virtual void Notify( const ::com::sun::star::uno::Sequence< OUString >& rPropertyNames );
    virtual void Commit();
public:
    DtransX11ConfigItem();
    virtual ~DtransX11ConfigItem();

    sal_Int32 getSelectionTimeout() const { return m_nSelectionTimeout; }
};

}

using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace x11;


sal_Int32 SelectionManager::getSelectionTimeout()
{
    if( m_nSelectionTimeout < 1 )
    {
        DtransX11ConfigItem aCfg;
        m_nSelectionTimeout = aCfg.getSelectionTimeout();
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "initialized selection timeout to %" SAL_PRIdINT32 " seconds\n", m_nSelectionTimeout );
#endif
    }
    return m_nSelectionTimeout;
}

/*
 *  DtransX11ConfigItem constructor
 */

DtransX11ConfigItem::DtransX11ConfigItem() :
    ConfigItem( OUString( RTL_CONSTASCII_USTRINGPARAM( SETTINGS_CONFIGNODE ) ),
                CONFIG_MODE_DELAYED_UPDATE ),
    m_nSelectionTimeout( 3 )
{
    if( IsValidConfigMgr() )
    {
        Sequence< OUString > aKeys( 1 );
        aKeys.getArray()[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( SELECTION_PROPERTY ) );
        Sequence< Any > aValues = GetProperties( aKeys );
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "found %" SAL_PRIdINT32 " properties for %s\n", aValues.getLength(), SELECTION_PROPERTY );
#endif
        Any* pValue = aValues.getArray();
        for( int i = 0; i < aValues.getLength(); i++, pValue++ )
        {
            if( pValue->getValueTypeClass() == TypeClass_STRING )
            {
                const OUString* pLine = (const OUString*)pValue->getValue();
                if( !pLine->isEmpty() )
                {
                    m_nSelectionTimeout = pLine->toInt32();
                    if( m_nSelectionTimeout < 1 )
                        m_nSelectionTimeout = 1;
                }
#if OSL_DEBUG_LEVEL > 1
                fprintf( stderr, "found SelectionTimeout \"%s\"\n",
                OUStringToOString( *pLine, osl_getThreadTextEncoding() ).getStr() );
#endif
            }
#if OSL_DEBUG_LEVEL > 1
            else
                fprintf( stderr, "found SelectionTimeout of type \"%s\"\n",
                OUStringToOString( pValue->getValueType().getTypeName(), osl_getThreadTextEncoding() ).getStr() );
#endif
        }
    }
#if OSL_DEBUG_LEVEL > 1
    else
        fprintf( stderr, "no valid configmanager, could not read timeout setting\n" );
#endif
}

/*
 *  DtransX11ConfigItem destructor
 */

DtransX11ConfigItem::~DtransX11ConfigItem()
{
}

/*
 *  DtransX11ConfigItem::Commit
 */

void DtransX11ConfigItem::Commit()
{
    // for the clipboard service this is readonly, so
    // there is nothing to commit
}

/*
 *  DtransX11ConfigItem::Notify
 */

void DtransX11ConfigItem::Notify( const Sequence< OUString >& /*rPropertyNames*/ )
{
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
