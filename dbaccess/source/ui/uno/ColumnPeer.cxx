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
#include "precompiled_dbaccess.hxx"
#include "ColumnPeer.hxx"
#include "ColumnControlWindow.hxx"
#include <vcl/svapp.hxx>
#include "dbustrings.hrc"
#include "FieldDescriptions.hxx"

//.........................................................................
namespace dbaui
{
//.........................................................................
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;

OColumnPeer::OColumnPeer(Window* _pParent,const Reference<XMultiServiceFactory>& _rxFactory)
    :m_xORB(_rxFactory)
    ,m_pActFieldDescr(NULL)
{
    osl_incrementInterlockedCount( &m_refCount );
    {
        OColumnControlWindow* pFieldControl = new OColumnControlWindow(_pParent,m_xORB);
        pFieldControl->SetComponentInterface(this);
        pFieldControl->Show();
    }
    osl_decrementInterlockedCount( &m_refCount );
}
// -----------------------------------------------------------------------------
void OColumnPeer::setEditWidth(sal_Int32 _nWidth)
{
    SolarMutexGuard aGuard;

    OColumnControlWindow* pFieldControl = static_cast<OColumnControlWindow*>( GetWindow() );
    if ( pFieldControl )
    {
        pFieldControl->setEditWidth(_nWidth);
    }
}
// -----------------------------------------------------------------------------
void OColumnPeer::setColumn(const Reference< XPropertySet>& _xColumn)
{
    SolarMutexGuard aGuard;

    OColumnControlWindow* pFieldControl = static_cast<OColumnControlWindow*>( GetWindow() );
    if ( pFieldControl )
    {
        if ( m_pActFieldDescr )
        {
            delete m_pActFieldDescr;
            m_pActFieldDescr = NULL;
        }
        if ( _xColumn.is() )
        {
            sal_Int32 nType         = 0;
            sal_Int32 nScale        = 0;
            sal_Int32 nPrecision    = 0;
            sal_Bool bAutoIncrement = sal_False;
            ::rtl::OUString sTypeName;

            try
            {
                // get the properties from the column
                _xColumn->getPropertyValue(PROPERTY_TYPENAME)       >>= sTypeName;
                _xColumn->getPropertyValue(PROPERTY_TYPE)           >>= nType;
                _xColumn->getPropertyValue(PROPERTY_SCALE)          >>= nScale;
                _xColumn->getPropertyValue(PROPERTY_PRECISION)      >>= nPrecision;
                _xColumn->getPropertyValue(PROPERTY_ISAUTOINCREMENT)    >>= bAutoIncrement;
            }
            catch(Exception)
            {
            }

            m_pActFieldDescr = new OFieldDescription(_xColumn,sal_True);
            // search for type
            ::rtl::OUString sCreateParam(RTL_CONSTASCII_USTRINGPARAM("x"));
            sal_Bool bForce;
            TOTypeInfoSP pTypeInfo = ::dbaui::getTypeInfoFromType(*pFieldControl->getTypeInfo(),nType,sTypeName,sCreateParam,nPrecision,nScale,bAutoIncrement,bForce);
            if ( !pTypeInfo.get() )
                pTypeInfo = pFieldControl->getDefaultTyp();

            m_pActFieldDescr->FillFromTypeInfo(pTypeInfo,sal_True,sal_False);
            m_xColumn = _xColumn;
        }
        pFieldControl->DisplayData(m_pActFieldDescr);
    }
}
// -----------------------------------------------------------------------------
void OColumnPeer::setConnection(const Reference< XConnection>& _xCon)
{
    SolarMutexGuard aGuard;
    OColumnControlWindow* pFieldControl = static_cast<OColumnControlWindow*>( GetWindow() );
    if ( pFieldControl )
        pFieldControl->setConnection(_xCon);
}
//------------------------------------------------------------------------------
void OColumnPeer::setProperty( const ::rtl::OUString& _rPropertyName, const Any& Value) throw( RuntimeException )
{
    SolarMutexGuard aGuard;

    if ( 0 == _rPropertyName.compareToAscii( PROPERTY_COLUMN ) )
    {
        Reference<XPropertySet> xProp(Value,UNO_QUERY);
        setColumn(xProp);
    }
    else if ( 0 == _rPropertyName.compareToAscii( PROPERTY_ACTIVE_CONNECTION ) )
    {
        Reference<XConnection> xCon(Value,UNO_QUERY);
        setConnection(xCon);
    }
    else
        VCLXWindow::setProperty(_rPropertyName,Value);
}
// -----------------------------------------------------------------------------
Any OColumnPeer::getProperty( const ::rtl::OUString& _rPropertyName ) throw( RuntimeException )
{
    Any aProp;
    OFieldDescControl* pFieldControl = static_cast<OFieldDescControl*>( GetWindow() );
    if ( pFieldControl && 0 == _rPropertyName.compareToAscii( PROPERTY_COLUMN ) )
    {
        aProp <<= m_xColumn;
    }
    else if ( pFieldControl && 0 == _rPropertyName.compareToAscii( PROPERTY_ACTIVE_CONNECTION ) )
    {
        aProp <<= pFieldControl->getConnection();
    }
    else
        aProp = VCLXWindow::getProperty(_rPropertyName);
    return aProp;
}
//.........................................................................
}   // namespace dbaui
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
