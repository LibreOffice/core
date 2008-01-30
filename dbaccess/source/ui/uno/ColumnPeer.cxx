/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ColumnPeer.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-30 08:56:09 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"
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
