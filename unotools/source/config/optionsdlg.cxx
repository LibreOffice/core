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
#include <unotools/configmgr.hxx>
#include <unotools/configitem.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <osl/mutex.hxx>

#include "itemholder1.hxx"

#include <unordered_map>

using namespace utl;
using namespace com::sun::star::beans;
using namespace com::sun::star::uno;

#define CFG_FILENAME            OUString( "Office.OptionsDialog" )
#define ROOT_NODE               "OptionsDialogGroups"
#define PAGES_NODE              "Pages"
#define OPTIONS_NODE            "Options"

static SvtOptionsDlgOptions_Impl*   pOptions = nullptr;
static sal_Int32                    nRefCount = 0;

class SvtOptionsDlgOptions_Impl : public utl::ConfigItem
{
private:
    typedef std::unordered_map< OUString, sal_Bool, OUStringHash, std::equal_to< OUString > > OptionNodeList;

    OUString        m_sPathDelimiter;
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
    : ConfigItem( OUString( CFG_FILENAME ) ),

    m_sPathDelimiter( "/" ),
    m_aOptionNodeList( OptionNodeList() )

{
    OUString sRootNode( ROOT_NODE );
    Sequence< OUString > aNodeSeq = GetNodeNames( sRootNode );
    OUString sNode( sRootNode + m_sPathDelimiter );
    sal_uInt32 nCount = aNodeSeq.getLength();
    for ( sal_uInt32 n = 0; n < nCount; n++ )
    {
        OUString sSubNode( sNode + aNodeSeq[n] );
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
    OUString sNode( _rNode + m_sPathDelimiter );
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

    Sequence< Any > aValues;
    aValues = GetProperties( lResult );
    bool bHide = false;
    if ( aValues[0] >>= bHide )
        m_aOptionNodeList.insert( OptionNodeList::value_type( sNode, bHide ) );

    if ( _eType != NT_Option )
    {
        OUString sNodes( sNode + sSet );
        Sequence< OUString > aNodes = GetNodeNames( sNodes );
        if ( aNodes.getLength() > 0 )
        {
            for ( sal_uInt32 n = 0; n < (sal_uInt32)aNodes.getLength(); ++n )
            {
                OUString sSubNodeName( sNodes + m_sPathDelimiter + aNodes[n] );
                ReadNode( sSubNodeName, _eType == NT_Group ? NT_Page : NT_Option );
            }
        }
    }
}

OUString getGroupPath( const OUString& _rGroup )
{
    return OUString( ROOT_NODE "/" + _rGroup + "/" );
}
OUString getPagePath( const OUString& _rPage )
{
    return OUString( PAGES_NODE "/" + _rPage + "/" );
}
OUString getOptionPath( const OUString& _rOption )
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

        ItemHolder1::holdConfigItem( E_OPTIONSDLGOPTIONS );
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
