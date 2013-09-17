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
#include <comphelper/stl_types.hxx>

#include <boost/unordered_map.hpp>
#include "itemholder1.hxx"

using namespace utl;
using namespace com::sun::star::beans ;
using namespace com::sun::star::uno;


#define CFG_FILENAME            OUString( "Office.OptionsDialog" )
#define ROOT_NODE               OUString( "OptionsDialogGroups" )
#define PAGES_NODE              OUString( "Pages" )
#define OPTIONS_NODE            OUString( "Options" )
#define PROPERTY_HIDE           OUString( "Hide" )

static SvtOptionsDlgOptions_Impl*   pOptions = NULL;
static sal_Int32                    nRefCount = 0;

class SvtOptionsDlgOptions_Impl : public utl::ConfigItem
{
private:
    typedef boost::unordered_map< OUString, sal_Bool, OUStringHash, ::std::equal_to< OUString > > OptionNodeList;

    OUString        m_sPathDelimiter;
    OptionNodeList  m_aOptionNodeList;

    enum NodeType{ NT_Group, NT_Page, NT_Option };
    void            ReadNode( const OUString& _rNode, NodeType _eType );
    sal_Bool        IsHidden( const OUString& _rPath ) const;

public:
                    SvtOptionsDlgOptions_Impl();

    virtual void    Notify( const com::sun::star::uno::Sequence< OUString >& aPropertyNames );
    virtual void    Commit();

    static ::osl::Mutex & getInitMutex();

    sal_Bool        IsGroupHidden   (   const OUString& _rGroup ) const;
    sal_Bool        IsPageHidden    (   const OUString& _rPage,
                                        const OUString& _rGroup ) const;
    sal_Bool        IsOptionHidden  (   const OUString& _rOption,
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

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

void SvtOptionsDlgOptions_Impl::Commit()
{
    // nothing to commit
}

// -----------------------------------------------------------------------

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
    lResult[0] = OUString( sNode + PROPERTY_HIDE );
    if ( _eType != NT_Option )
        lResult[1] = OUString( sNode + sSet );

    Sequence< Any > aValues;
    aValues = GetProperties( lResult );
    sal_Bool bHide = sal_False;
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

// -----------------------------------------------------------------------

OUString getGroupPath( const OUString& _rGroup )
{
    return OUString( ROOT_NODE + OUString('/') + _rGroup + OUString('/') );
}
OUString getPagePath( const OUString& _rPage )
{
    return OUString( PAGES_NODE + OUString('/') + _rPage + OUString('/') );
}
OUString getOptionPath( const OUString& _rOption )
{
    return OUString( OPTIONS_NODE + OUString('/') + _rOption + OUString('/') );
}

// -----------------------------------------------------------------------

sal_Bool SvtOptionsDlgOptions_Impl::IsHidden( const OUString& _rPath ) const
{
    sal_Bool bRet = sal_False;
    OptionNodeList::const_iterator pIter = m_aOptionNodeList.find( _rPath );
    if ( pIter != m_aOptionNodeList.end() )
        bRet = pIter->second;
    return bRet;
}

// -----------------------------------------------------------------------

sal_Bool SvtOptionsDlgOptions_Impl::IsGroupHidden( const OUString& _rGroup ) const
{
    return IsHidden( getGroupPath( _rGroup ) );
}

// -----------------------------------------------------------------------

sal_Bool SvtOptionsDlgOptions_Impl::IsPageHidden( const OUString& _rPage, const OUString& _rGroup ) const
{
    return IsHidden( getGroupPath( _rGroup  ) + getPagePath( _rPage ) );
}

// -----------------------------------------------------------------------

sal_Bool SvtOptionsDlgOptions_Impl::IsOptionHidden(
    const OUString& _rOption, const OUString& _rPage, const OUString& _rGroup ) const
{
    return IsHidden( getGroupPath( _rGroup  ) + getPagePath( _rPage ) + getOptionPath( _rOption ) );
}

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

SvtOptionsDialogOptions::~SvtOptionsDialogOptions()
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( SvtOptionsDlgOptions_Impl::getInitMutex() );
    if ( !--nRefCount )
    {
        if ( pOptions->IsModified() )
            pOptions->Commit();
        delete pOptions;
        pOptions = NULL;
    }
}

sal_Bool SvtOptionsDialogOptions::IsGroupHidden( const OUString& _rGroup ) const
{
    return m_pImp->IsGroupHidden( _rGroup );
}

sal_Bool SvtOptionsDialogOptions::IsPageHidden( const OUString& _rPage, const OUString& _rGroup ) const
{
    return m_pImp->IsPageHidden( _rPage, _rGroup );
}

sal_Bool SvtOptionsDialogOptions::IsOptionHidden(
    const OUString& _rOption, const OUString& _rPage, const OUString& _rGroup ) const
{
    return m_pImp->IsOptionHidden( _rOption, _rPage, _rGroup );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
