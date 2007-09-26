/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ConnectionLineData.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-26 14:52:27 $
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
#ifndef DBAUI_CONNECTIONLINEDATA_HXX
#include "ConnectionLineData.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif


using namespace dbaui;
DBG_NAME(OConnectionLineData)
//==================================================================
//class OConnectionLineData
//==================================================================
//------------------------------------------------------------------------
OConnectionLineData::OConnectionLineData()
{
    DBG_CTOR(OConnectionLineData,NULL);
}

//------------------------------------------------------------------------
OConnectionLineData::OConnectionLineData( const ::rtl::OUString& rSourceFieldName, const ::rtl::OUString& rDestFieldName )
    :m_aSourceFieldName( rSourceFieldName )
    ,m_aDestFieldName( rDestFieldName )
{
    DBG_CTOR(OConnectionLineData,NULL);
}

//------------------------------------------------------------------------
OConnectionLineData::OConnectionLineData( const OConnectionLineData& rConnLineData )
    : ::vos::OReference()
{
    DBG_CTOR(OConnectionLineData,NULL);
    *this = rConnLineData;
}

//------------------------------------------------------------------------
OConnectionLineData::~OConnectionLineData()
{
    DBG_DTOR(OConnectionLineData,NULL);
}

//------------------------------------------------------------------------
void OConnectionLineData::CopyFrom(const OConnectionLineData& rSource)
{
    *this = rSource;
    // hier ziehe ich mich auf das (nicht-virtuelle) operator= zurueck, das nur meine Members kopiert
}

//------------------------------------------------------------------------
bool OConnectionLineData::IsValid() const
{
    return (GetSourceFieldName().getLength() && GetDestFieldName().getLength());
}

//------------------------------------------------------------------------
OConnectionLineData& OConnectionLineData::operator=( const OConnectionLineData& rConnLineData )
{
    if (&rConnLineData == this)
        return *this;

    m_aSourceFieldName = rConnLineData.GetSourceFieldName();
    m_aDestFieldName = rConnLineData.GetDestFieldName();

    return *this;
}

//------------------------------------------------------------------------
bool OConnectionLineData::Reset()
{
    m_aDestFieldName = m_aSourceFieldName = ::rtl::OUString();
    return true;
}
// -----------------------------------------------------------------------------
namespace dbaui
{
//-------------------------------------------------------------------------
bool operator==(const OConnectionLineData& lhs, const OConnectionLineData& rhs)
{
    return (lhs.m_aSourceFieldName == rhs.m_aSourceFieldName)
        && (lhs.m_aDestFieldName == rhs.m_aDestFieldName);
}
}

