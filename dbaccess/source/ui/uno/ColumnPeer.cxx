/*************************************************************************
 *
 *  $RCSfile: ColumnPeer.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 16:20:54 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
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
: m_pActFieldDescr(NULL)
, m_xORB(_rxFactory)
{
    osl_incrementInterlockedCount( &m_refCount );
    {
        OColumnControlWindow* pFieldControl = new OColumnControlWindow(_pParent,m_xORB);
        pFieldControl->Show();
        pFieldControl->SetComponentInterface(this);
    }
    osl_decrementInterlockedCount( &m_refCount );
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
    else if ( 0 == _rPropertyName.compareToAscii( PROPERTY_ACTIVECONNECTION ) )
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
    else if ( pFieldControl && 0 == _rPropertyName.compareToAscii( PROPERTY_ACTIVECONNECTION ) )
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