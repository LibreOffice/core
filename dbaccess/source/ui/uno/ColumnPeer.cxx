/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbui.hxx"
#ifndef DBAUI_COLUMNPEER_HXX
#include "ColumnPeer.hxx"
#endif
#ifndef DBAUI_COLUMNCONTROLWINDOW_HXX
#include "ColumnControlWindow.hxx"
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef DBAUI_FIELDDESCRIPTIONS_HXX
#include "FieldDescriptions.hxx"
#endif

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
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    OColumnControlWindow* pFieldControl = static_cast<OColumnControlWindow*>( GetWindow() );
    if ( pFieldControl )
    {
        pFieldControl->setEditWidth(_nWidth);
    }
}
// -----------------------------------------------------------------------------
void OColumnPeer::setColumn(const Reference< XPropertySet>& _xColumn)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

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
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    OColumnControlWindow* pFieldControl = static_cast<OColumnControlWindow*>( GetWindow() );
    if ( pFieldControl )
        pFieldControl->setConnection(_xCon);
}
//------------------------------------------------------------------------------
void OColumnPeer::setProperty( const ::rtl::OUString& _rPropertyName, const Any& Value) throw( RuntimeException )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

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
