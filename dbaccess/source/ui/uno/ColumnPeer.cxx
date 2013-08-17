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

#include "ColumnPeer.hxx"
#include "ColumnControlWindow.hxx"
#include <vcl/svapp.hxx>
#include "dbustrings.hrc"
#include "FieldDescriptions.hxx"

namespace dbaui
{
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;

OColumnPeer::OColumnPeer(Window* _pParent,const Reference<XComponentContext>& _rxContext)
    :m_pActFieldDescr(NULL)
{
    osl_atomic_increment( &m_refCount );
    {
        OColumnControlWindow* pFieldControl = new OColumnControlWindow(_pParent, _rxContext);
        pFieldControl->SetComponentInterface(this);
        pFieldControl->Show();
    }
    osl_atomic_decrement( &m_refCount );
}

void OColumnPeer::setEditWidth(sal_Int32 _nWidth)
{
    SolarMutexGuard aGuard;

    OColumnControlWindow* pFieldControl = static_cast<OColumnControlWindow*>( GetWindow() );
    if ( pFieldControl )
    {
        pFieldControl->setEditWidth(_nWidth);
    }
}

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
            OUString sTypeName;

            try
            {
                // get the properties from the column
                _xColumn->getPropertyValue(PROPERTY_TYPENAME)       >>= sTypeName;
                _xColumn->getPropertyValue(PROPERTY_TYPE)           >>= nType;
                _xColumn->getPropertyValue(PROPERTY_SCALE)          >>= nScale;
                _xColumn->getPropertyValue(PROPERTY_PRECISION)      >>= nPrecision;
                _xColumn->getPropertyValue(PROPERTY_ISAUTOINCREMENT)    >>= bAutoIncrement;
            }
            catch(const Exception&)
            {
            }

            m_pActFieldDescr = new OFieldDescription(_xColumn,sal_True);
            // search for type
            OUString sCreateParam("x");
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

void OColumnPeer::setConnection(const Reference< XConnection>& _xCon)
{
    SolarMutexGuard aGuard;
    OColumnControlWindow* pFieldControl = static_cast<OColumnControlWindow*>( GetWindow() );
    if ( pFieldControl )
        pFieldControl->setConnection(_xCon);
}

void OColumnPeer::setProperty( const OUString& _rPropertyName, const Any& Value) throw( RuntimeException )
{
    SolarMutexGuard aGuard;

    if (_rPropertyName == PROPERTY_COLUMN)
    {
        Reference<XPropertySet> xProp(Value,UNO_QUERY);
        setColumn(xProp);
    }
    else if (_rPropertyName == PROPERTY_ACTIVE_CONNECTION)
    {
        Reference<XConnection> xCon(Value,UNO_QUERY);
        setConnection(xCon);
    }
    else
        VCLXWindow::setProperty(_rPropertyName,Value);
}

Any OColumnPeer::getProperty( const OUString& _rPropertyName ) throw( RuntimeException )
{
    Any aProp;
    OFieldDescControl* pFieldControl = static_cast<OFieldDescControl*>( GetWindow() );
    if (pFieldControl && _rPropertyName == PROPERTY_COLUMN)
    {
        aProp <<= m_xColumn;
    }
    else if (pFieldControl && _rPropertyName == PROPERTY_ACTIVE_CONNECTION)
    {
        aProp <<= pFieldControl->getConnection();
    }
    else
        aProp = VCLXWindow::getProperty(_rPropertyName);
    return aProp;
}

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
