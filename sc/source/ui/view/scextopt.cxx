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

#include <scextopt.hxx>

#include <osl/diagnose.h>

#include <map>
#include <memory>
#include <vector>

ScExtDocSettings::ScExtDocSettings() :
    mfTabBarWidth( -1.0 ),
    mnLinkCnt( 0 ),
    mnDisplTab( -1 )
{
}

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
    mbPageMode( false ),
    mbShowGrid( true )
{
}

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

    SCTAB GetLastTab() const;

private:
    typedef std::shared_ptr< ScExtTabSettings >     ScExtTabSettingsRef;
    typedef ::std::map< SCTAB, ScExtTabSettingsRef >    ScExtTabSettingsMap;

    /** Makes a deep copy of all objects in the passed map. */
    void                CopyFromMap( const ScExtTabSettingsMap& rMap );

    ScExtTabSettingsMap maMap;
};

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
    return (aIt == maMap.end()) ? nullptr : aIt->second.get();
}

ScExtTabSettings& ScExtTabSettingsCont::GetOrCreateTabSettings( SCTAB nTab )
{
    ScExtTabSettingsRef& rxTabSett = maMap[ nTab ];
    if( !rxTabSett )
        rxTabSett.reset( new ScExtTabSettings );
    return *rxTabSett;
}

SCTAB ScExtTabSettingsCont::GetLastTab() const
{
    return maMap.empty() ? -1 : maMap.rbegin()->first;
}

void ScExtTabSettingsCont::CopyFromMap( const ScExtTabSettingsMap& rMap )
{
    maMap.clear();
    for( const auto& [rTab, rxSettings] : rMap )
        maMap[ rTab ].reset( new ScExtTabSettings( *rxSettings ) );
}

/** Implementation struct for ScExtDocOptions containing all members. */
struct ScExtDocOptionsImpl
{
    ScExtDocSettings    maDocSett;          /// Global document settings.
    ScExtTabSettingsCont maTabSett;         /// Settings for all sheets.
    std::vector< OUString > maCodeNames;        /// Codenames for all sheets (VBA module names).
    bool                mbChanged;          /// Use only if something has been changed.

    explicit            ScExtDocOptionsImpl();
};

ScExtDocOptionsImpl::ScExtDocOptionsImpl() :
    mbChanged( false )
{
}

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

SCTAB ScExtDocOptions::GetLastTab() const
{
    return mxImpl->maTabSett.GetLastTab();
}

ScExtTabSettings& ScExtDocOptions::GetOrCreateTabSettings( SCTAB nTab )
{
    return mxImpl->maTabSett.GetOrCreateTabSettings( nTab );
}

SCTAB ScExtDocOptions::GetCodeNameCount() const
{
    return static_cast< SCTAB >( mxImpl->maCodeNames.size() );
}

const OUString& ScExtDocOptions::GetCodeName( SCTAB nTab ) const
{
    OSL_ENSURE( (0 <= nTab) && (nTab < GetCodeNameCount()), "ScExtDocOptions::GetCodeName - invalid sheet index" );
    return ((0 <= nTab) && (nTab < GetCodeNameCount())) ? mxImpl->maCodeNames[ static_cast< size_t >( nTab ) ] : EMPTY_OUSTRING;
}

void ScExtDocOptions::SetCodeName( SCTAB nTab, const OUString& rCodeName )
{
    OSL_ENSURE( nTab >= 0, "ScExtDocOptions::SetCodeName - invalid sheet index" );
    if( nTab >= 0 )
    {
        size_t nIndex = static_cast< size_t >( nTab );
        if( nIndex >= mxImpl->maCodeNames.size() )
            mxImpl->maCodeNames.resize( nIndex + 1 );
        mxImpl->maCodeNames[ nIndex ] = rCodeName;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
