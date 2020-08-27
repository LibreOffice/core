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

#include <unotools/optionsdlg.hxx>
#include <unotools/configitem.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <osl/mutex.hxx>

#include "itemholder1.hxx"

#include <unordered_map>

using namespace utl;
using namespace com::sun::star::beans;
using namespace com::sun::star::uno;

#define CFG_FILENAME            "Office.OptionsDialog"
#define ROOT_NODE               "OptionsDialogGroups"
#define PAGES_NODE              "Pages"
#define OPTIONS_NODE            "Options"

static SvtOptionsDlgOptions_Impl*   pOptions = nullptr;
static sal_Int32                    nRefCount = 0;

class SvtOptionsDlgOptions_Impl : public utl::ConfigItem
{
private:
    typedef std::unordered_map< OUString, bool > OptionNodeList;

    static constexpr OUStringLiteral g_sPathDelimiter = u"/";
    OptionNodeList  m_aOptionNodeList;

    enum NodeType{ NT_Group, NT_Page, NT_Option };
    void            ReadNode( const OUString& _rNode, NodeType _eType );
    bool        IsHidden( const OUString& _rPath ) const;

    virtual void    ImplCommit() override;

public:
                    SvtOptionsDlgOptions_Impl();

    virtual void    Notify( const css::uno::Sequence< OUString >& aPropertyNames ) override;

    static ::osl::Mutex & getInitMutex();

    bool        IsGroupHidden   (   const OUString& _rGroup ) const;
    bool        IsPageHidden    (   const OUString& _rPage,
                                        const OUString& _rGroup ) const;
    bool        IsOptionHidden  (   const OUString& _rOption,
                                        const OUString& _rPage,
                                        const OUString& _rGroup ) const;
};

namespace
{
    class theOptionsDlgOptions_ImplMutex : public rtl::Static<osl::Mutex, theOptionsDlgOptions_ImplMutex>{};
}

::osl::Mutex & SvtOptionsDlgOptions_Impl::getInitMutex()
{
    return theOptionsDlgOptions_ImplMutex::get();
}

SvtOptionsDlgOptions_Impl::SvtOptionsDlgOptions_Impl()
    : ConfigItem( CFG_FILENAME ),
    m_aOptionNodeList( OptionNodeList() )
{
    OUString sRootNode( ROOT_NODE );
    const Sequence< OUString > aNodeSeq = GetNodeNames( sRootNode );
    OUString sNode( sRootNode + g_sPathDelimiter );
    for ( const auto& rNode : aNodeSeq )
    {
        OUString sSubNode( sNode + rNode );
        ReadNode( sSubNode, NT_Group );
    }
}

void SvtOptionsDlgOptions_Impl::ImplCommit()
{
    // nothing to commit
}

void SvtOptionsDlgOptions_Impl::Notify( const Sequence< OUString >& )
{
    // nothing to notify
}

void SvtOptionsDlgOptions_Impl::ReadNode( const OUString& _rNode, NodeType _eType )
{
    OUString sNode( _rNode + g_sPathDelimiter );
    OUString sSet;
    sal_Int32 nLen = 0;
    switch ( _eType )
    {
        case NT_Group :
        {
            sSet = PAGES_NODE;
            nLen = 2;
            break;
        }

        case NT_Page :
        {
            sSet = OPTIONS_NODE;
            nLen = 2;
            break;
        }

        case NT_Option :
        {
            nLen = 1;
            break;
        }
    }

    Sequence< OUString > lResult( nLen );
    lResult[0] = sNode + "Hide";
    if ( _eType != NT_Option )
        lResult[1] = sNode + sSet;

    Sequence< Any > aValues = GetProperties( lResult );
    bool bHide = false;
    if ( aValues[0] >>= bHide )
        m_aOptionNodeList.emplace( sNode, bHide );

    if ( _eType != NT_Option )
    {
        OUString sNodes( sNode + sSet );
        const Sequence< OUString > aNodes = GetNodeNames( sNodes );
        for ( const auto& rNode : aNodes )
        {
            OUString sSubNodeName( sNodes + g_sPathDelimiter + rNode );
            ReadNode( sSubNodeName, _eType == NT_Group ? NT_Page : NT_Option );
        }
    }
}

static OUString getGroupPath( const OUString& _rGroup )
{
    return OUString( ROOT_NODE "/" + _rGroup + "/" );
}
static OUString getPagePath( const OUString& _rPage )
{
    return OUString( PAGES_NODE "/" + _rPage + "/" );
}
static OUString getOptionPath( const OUString& _rOption )
{
    return OUString( OPTIONS_NODE "/" + _rOption + "/" );
}

bool SvtOptionsDlgOptions_Impl::IsHidden( const OUString& _rPath ) const
{
    bool bRet = false;
    OptionNodeList::const_iterator pIter = m_aOptionNodeList.find( _rPath );
    if ( pIter != m_aOptionNodeList.end() )
        bRet = pIter->second;
    return bRet;
}

bool SvtOptionsDlgOptions_Impl::IsGroupHidden( const OUString& _rGroup ) const
{
    return IsHidden( getGroupPath( _rGroup ) );
}

bool SvtOptionsDlgOptions_Impl::IsPageHidden( const OUString& _rPage, const OUString& _rGroup ) const
{
    return IsHidden( getGroupPath( _rGroup  ) + getPagePath( _rPage ) );
}

bool SvtOptionsDlgOptions_Impl::IsOptionHidden(
    const OUString& _rOption, const OUString& _rPage, const OUString& _rGroup ) const
{
    return IsHidden( getGroupPath( _rGroup  ) + getPagePath( _rPage ) + getOptionPath( _rOption ) );
}

SvtOptionsDialogOptions::SvtOptionsDialogOptions()
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( SvtOptionsDlgOptions_Impl::getInitMutex() );
    ++nRefCount;
    if ( !pOptions )
    {
        pOptions = new SvtOptionsDlgOptions_Impl;

        ItemHolder1::holdConfigItem( EItem::OptionsDialogOptions );
    }
    m_pImp = pOptions;
}

SvtOptionsDialogOptions::~SvtOptionsDialogOptions()
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( SvtOptionsDlgOptions_Impl::getInitMutex() );
    if ( !--nRefCount )
    {
        if ( pOptions->IsModified() )
            pOptions->Commit();
        delete pOptions;
        pOptions = nullptr;
    }
}

bool SvtOptionsDialogOptions::IsGroupHidden( const OUString& _rGroup ) const
{
    return m_pImp->IsGroupHidden( _rGroup );
}

bool SvtOptionsDialogOptions::IsPageHidden( const OUString& _rPage, const OUString& _rGroup ) const
{
    return m_pImp->IsPageHidden( _rPage, _rGroup );
}

bool SvtOptionsDialogOptions::IsOptionHidden(
    const OUString& _rOption, const OUString& _rPage, const OUString& _rGroup ) const
{
    return m_pImp->IsOptionHidden( _rOption, _rPage, _rGroup );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
