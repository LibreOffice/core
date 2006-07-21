/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: scextopt.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 15:07:47 $
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
#include "precompiled_sc.hxx"
#ifndef SC_SCEXTOPT_HXX
#include "scextopt.hxx"
#endif

#include <vector>
#include <map>
#include <boost/shared_ptr.hpp>

// ============================================================================

ScExtDocSettings::ScExtDocSettings() :
    maOleSize( ScAddress::INITIALIZE_INVALID ),
    mfTabBarWidth( -1.0 ),
    mnLinkCnt( 0 ),
    mnDisplTab( 0 ),
    mbWinProtected( false ),
    mbEncrypted( false )
{
}

// ============================================================================

ScExtTabSettings::ScExtTabSettings() :
    maUsedArea( ScAddress::INITIALIZE_INVALID ),
    maCursor( ScAddress::INITIALIZE_INVALID ),
    maFirstVis( ScAddress::INITIALIZE_INVALID ),
    maSecondVis( ScAddress::INITIALIZE_INVALID ),
    maFreezePos( 0, 0, 0 ),
    maSplitPos( 0, 0 ),
    meActivePane( SCEXT_PANE_TOPLEFT ),
    maGridColor( COL_AUTO ),
    mnNormalZoom( 0 ),
    mnPageZoom( 0 ),
    mbSelected( false ),
    mbFrozenPanes( false ),
    mbPageMode( false )
{
}

// ============================================================================

/** A container for ScExtTabSettings objects.
    @descr  Internally, a std::map with shared pointers to ScExtTabSettings is
    used. The copy constructor and assignment operator make deep copies of the
    objects. */
class ScExtTabSettingsCont
{
public:
    explicit            ScExtTabSettingsCont();
                        ScExtTabSettingsCont( const ScExtTabSettingsCont& rSrc );
    ScExtTabSettingsCont& operator=( const ScExtTabSettingsCont& rSrc );

    const ScExtTabSettings* GetTabSettings( SCTAB nTab ) const;
    ScExtTabSettings&   GetOrCreateTabSettings( SCTAB nTab );

private:
    typedef ::boost::shared_ptr< ScExtTabSettings >     ScExtTabSettingsRef;
    typedef ::std::map< SCTAB, ScExtTabSettingsRef >    ScExtTabSettingsMap;

    /** Makes a deep copy of all objects in the passed map. */
    void                CopyFromMap( const ScExtTabSettingsMap& rMap );

    ScExtTabSettingsMap maMap;
};

// ----------------------------------------------------------------------------

ScExtTabSettingsCont::ScExtTabSettingsCont()
{
}

ScExtTabSettingsCont::ScExtTabSettingsCont( const ScExtTabSettingsCont& rSrc )
{
    CopyFromMap( rSrc.maMap );
}

ScExtTabSettingsCont& ScExtTabSettingsCont::operator=( const ScExtTabSettingsCont& rSrc )
{
    CopyFromMap( rSrc.maMap );
    return *this;
}

const ScExtTabSettings* ScExtTabSettingsCont::GetTabSettings( SCTAB nTab ) const
{
    ScExtTabSettingsMap::const_iterator aIt = maMap.find( nTab );
    return (aIt == maMap.end()) ? 0 : aIt->second.get();
}

ScExtTabSettings& ScExtTabSettingsCont::GetOrCreateTabSettings( SCTAB nTab )
{
    ScExtTabSettingsRef& rxTabSett = maMap[ nTab ];
    if( !rxTabSett )
        rxTabSett.reset( new ScExtTabSettings );
    return *rxTabSett;
}

void ScExtTabSettingsCont::CopyFromMap( const ScExtTabSettingsMap& rMap )
{
    maMap.clear();
    for( ScExtTabSettingsMap::const_iterator aIt = rMap.begin(), aEnd = rMap.end(); aIt != aEnd; ++aIt )
        maMap[ aIt->first ].reset( new ScExtTabSettings( *aIt->second ) );
}

// ============================================================================

/** Implementation struct for ScExtDocOptions containing all members. */
struct ScExtDocOptionsImpl
{
    typedef ::std::vector< String > StringVec;

    ScExtDocSettings    maDocSett;          /// Global document settings.
    ScExtTabSettingsCont maTabSett;         /// Settings for all sheets.
    StringVec           maCodeNames;        /// Codenames for all sheets (VBA module names).
    bool                mbChanged;          /// Use only if something has been changed.

    explicit            ScExtDocOptionsImpl();
};

ScExtDocOptionsImpl::ScExtDocOptionsImpl() :
    mbChanged( false )
{
}

// ----------------------------------------------------------------------------

ScExtDocOptions::ScExtDocOptions() :
    mxImpl( new ScExtDocOptionsImpl )
{
}

ScExtDocOptions::ScExtDocOptions( const ScExtDocOptions& rSrc ) :
    mxImpl( new ScExtDocOptionsImpl( *rSrc.mxImpl ) )
{
}

ScExtDocOptions::~ScExtDocOptions()
{
}

ScExtDocOptions& ScExtDocOptions::operator=( const ScExtDocOptions& rSrc )
{
    *mxImpl = *rSrc.mxImpl;
    return *this;
}

bool ScExtDocOptions::IsChanged() const
{
    return mxImpl->mbChanged;
}

void ScExtDocOptions::SetChanged( bool bChanged )
{
    mxImpl->mbChanged = bChanged;
}

const ScExtDocSettings& ScExtDocOptions::GetDocSettings() const
{
    return mxImpl->maDocSett;
}

ScExtDocSettings& ScExtDocOptions::GetDocSettings()
{
    return mxImpl->maDocSett;
}

const ScExtTabSettings* ScExtDocOptions::GetTabSettings( SCTAB nTab ) const
{
    return mxImpl->maTabSett.GetTabSettings( nTab );
}

ScExtTabSettings& ScExtDocOptions::GetOrCreateTabSettings( SCTAB nTab )
{
    return mxImpl->maTabSett.GetOrCreateTabSettings( nTab );
}

size_t ScExtDocOptions::GetCodeNameCount() const
{
    return mxImpl->maCodeNames.size();
}

const String& ScExtDocOptions::GetCodeName( size_t nIdx ) const
{
    DBG_ASSERT( nIdx < GetCodeNameCount(), "ScExtDocOptions::GetCodeName - invalid index" );
    return (nIdx < GetCodeNameCount()) ? mxImpl->maCodeNames[ nIdx ] : EMPTY_STRING;
}

void ScExtDocOptions::AppendCodeName( const String& rCodeName )
{
    mxImpl->maCodeNames.push_back( rCodeName );
}

// ============================================================================

