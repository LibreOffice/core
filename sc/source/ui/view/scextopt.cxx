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
#include "precompiled_sc.hxx"
#include "scextopt.hxx"

#include <vector>
#include <map>
#include <boost/shared_ptr.hpp>

// ============================================================================

ScExtDocSettings::ScExtDocSettings() :
    mfTabBarWidth( -1.0 ),
    mnLinkCnt( 0 ),
    mnDisplTab( 0 )
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

SCTAB ScExtDocOptions::GetCodeNameCount() const
{
    return static_cast< SCTAB >( mxImpl->maCodeNames.size() );
}

const String& ScExtDocOptions::GetCodeName( SCTAB nTab ) const
{
    DBG_ASSERT( (0 <= nTab) && (nTab < GetCodeNameCount()), "ScExtDocOptions::GetCodeName - invalid sheet index" );
    return ((0 <= nTab) && (nTab < GetCodeNameCount())) ? mxImpl->maCodeNames[ static_cast< size_t >( nTab ) ] : EMPTY_STRING;
}

void ScExtDocOptions::SetCodeName( SCTAB nTab, const String& rCodeName )
{
    DBG_ASSERT( nTab >= 0, "ScExtDocOptions::SetCodeName - invalid sheet index" );
    if( nTab >= 0 )
    {
        size_t nIndex = static_cast< size_t >( nTab );
        if( nIndex >= mxImpl->maCodeNames.size() )
            mxImpl->maCodeNames.resize( nIndex + 1 );
        mxImpl->maCodeNames[ nIndex ] = rCodeName;
    }
}

// ============================================================================

