/*************************************************************************
 *
 *  $RCSfile: TableWindowData.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: oj $ $Date: 2001-02-28 10:18:26 $
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
#ifndef DBAUI_TABLEWINDOWDATA_HXX
#include "TableWindowData.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _COMPHELPER_STREAMSECTION_HXX_
#include <comphelper/streamsection.hxx>
#endif
#ifndef _COMPHELPER_BASIC_IO_HXX_
#include <comphelper/basicio.hxx>
#endif


using namespace dbaui;
using namespace comphelper;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::uno;

TYPEINIT0(OTableWindowData);

//==================================================================
// class OTableWindowData
//==================================================================
DBG_NAME(OTableWindowData);
//------------------------------------------------------------------------------
OTableWindowData::OTableWindowData()
    :m_aPosition( Point(-1,-1) )
    ,m_aSize( Size(-1,-1) )
    ,m_bShowAll( TRUE )
{
    DBG_CTOR(OTableWindowData,NULL);
}

//------------------------------------------------------------------------------
OTableWindowData::OTableWindowData( const ::rtl::OUString& _rComposedName, const ::rtl::OUString& rTableName, const ::rtl::OUString& rWinName )
    :m_sComposedName(_rComposedName)
    ,m_aTableName( rTableName )
    ,m_aWinName( rWinName )
    ,m_aPosition( Point(-1,-1) )
    ,m_aSize( Size(-1,-1) )
    ,m_bShowAll( TRUE )
{
    DBG_CTOR(OTableWindowData,NULL);
    if( !m_aWinName.getLength() )
        m_aWinName = m_aTableName;
}

//------------------------------------------------------------------------------
OTableWindowData::~OTableWindowData()
{
    DBG_DTOR(OTableWindowData,NULL);
}

//------------------------------------------------------------------------------
BOOL OTableWindowData::HasPosition() const
{
    return ( (m_aPosition.X() != -1) && (m_aPosition.Y() != -1) );
}

//------------------------------------------------------------------------------
BOOL OTableWindowData::HasSize() const
{
    return ( (m_aSize.Width() != -1) && (m_aSize.Height() !=-1) );
}

//------------------------------------------------------------------------------
void OTableWindowData::Load(const Reference<XObjectInputStream>& _rxIn)
{
    OStreamSection aSection(_rxIn.get());
    _rxIn >> m_sComposedName;
    _rxIn >> m_aTableName;
    _rxIn >> m_aWinName;
    _rxIn >> m_aPosition.X();
    _rxIn >> m_aPosition.Y();
    _rxIn >> m_aSize.Width();
    _rxIn >> m_aSize.Height();
    _rxIn >> m_bShowAll;
}
//------------------------------------------------------------------------------
void OTableWindowData::Save(const Reference<XObjectOutputStream>& _rxOut)
{
    OStreamSection aSection(_rxOut.get());
    _rxOut << m_sComposedName;
    _rxOut << m_aTableName;
    _rxOut << m_aWinName;
    _rxOut << m_aPosition.X();
    _rxOut << m_aPosition.Y();
    _rxOut << m_aSize.Width();
    _rxOut << m_aSize.Height();
    _rxOut << m_bShowAll;
}
// -----------------------------------------------------------------------------

