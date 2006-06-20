/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: config.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 05:58:37 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include <cstdio>

#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif

#include "X11_selection.hxx"

#define SETTINGS_CONFIGNODE "VCL/Settings/Transfer"
#define SELECTION_PROPERTY "SelectionTimeout"

namespace x11
{

class DtransX11ConfigItem : public ::utl::ConfigItem
{
    sal_Int32           m_nSelectionTimeout;

    virtual void Notify( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& rPropertyNames );
    virtual void Commit();
public:
    DtransX11ConfigItem();
    virtual ~DtransX11ConfigItem();

    sal_Int32 getSelectionTimeout() const { return m_nSelectionTimeout; }
};

}

using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace rtl;
using namespace x11;

sal_Int32 SelectionManager::getSelectionTimeout()
{
    if( m_nSelectionTimeout < 1 )
    {
        DtransX11ConfigItem aCfg;
        m_nSelectionTimeout = aCfg.getSelectionTimeout();
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "initialized selection timeout to %ld seconds\n", m_nSelectionTimeout );
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
        fprintf( stderr, "found %ld properties for %s\n", aValues.getLength(), SELECTION_PROPERTY );
#endif
        Any* pValue = aValues.getArray();
        for( int i = 0; i < aValues.getLength(); i++, pValue++ )
        {
            if( pValue->getValueTypeClass() == TypeClass_STRING )
            {
                const OUString* pLine = (const OUString*)pValue->getValue();
                if( pLine->getLength() )
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


