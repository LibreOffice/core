/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TableWindowData.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 16:30:55 $
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

