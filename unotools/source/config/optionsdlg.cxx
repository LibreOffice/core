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
#include <unotools/configmgr.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <cassert>

using namespace com::sun::star::beans;
using namespace com::sun::star::uno;

constexpr OUStringLiteral ROOT_NODE = u"OptionsDialogGroups";
constexpr OUStringLiteral PAGES_NODE = u"Pages";
constexpr OUStringLiteral OPTIONS_NODE = u"Options";

namespace {
    enum NodeType{ NT_Group, NT_Page, NT_Option };
}
constexpr OUStringLiteral g_sPathDelimiter = u"/";
static void ReadNode(
        const Reference<css::container::XHierarchicalNameAccess>& xHierarchyAccess,
        SvtOptionsDialogOptions::OptionNodeList & aOptionNodeList,
        std::u16string_view _rNode, NodeType _eType );


SvtOptionsDialogOptions::SvtOptionsDialogOptions()
{
    Reference<css::container::XHierarchicalNameAccess> xHierarchyAccess = utl::ConfigManager::acquireTree(u"Office.OptionsDialog");
    const Sequence< OUString > aNodeSeq = utl::ConfigItem::GetNodeNames( xHierarchyAccess, ROOT_NODE, utl::ConfigNameFormat::LocalNode);
    OUString sNode( ROOT_NODE + g_sPathDelimiter );
    for ( const auto& rNode : aNodeSeq )
    {
        OUString sSubNode( sNode + rNode );
        ReadNode( xHierarchyAccess, m_aOptionNodeList, sSubNode, NT_Group );
    }
}


static void ReadNode(
        const Reference<css::container::XHierarchicalNameAccess>& xHierarchyAccess,
        SvtOptionsDialogOptions::OptionNodeList & aOptionNodeList,
        std::u16string_view _rNode, NodeType _eType )
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

    assert(nLen > 0);

    Sequence< OUString > lResult( nLen );
    auto plResult = lResult.getArray();
    plResult[0] = sNode + "Hide";
    if ( _eType != NT_Option )
        plResult[1] = sNode + sSet;

    Sequence< Any > aValues = utl::ConfigItem::GetProperties( xHierarchyAccess, lResult, /*bAllLocales*/false );
    bool bHide = false;
    if ( aValues[0] >>= bHide )
        aOptionNodeList.emplace( sNode, bHide );

    if ( _eType != NT_Option )
    {
        OUString sNodes( sNode + sSet );
        const Sequence< OUString > aNodes = utl::ConfigItem::GetNodeNames( xHierarchyAccess, sNodes, utl::ConfigNameFormat::LocalNode );
        for ( const auto& rNode : aNodes )
        {
            OUString sSubNodeName( sNodes + g_sPathDelimiter + rNode );
            ReadNode( xHierarchyAccess, aOptionNodeList, sSubNodeName, _eType == NT_Group ? NT_Page : NT_Option );
        }
    }
}

static OUString getGroupPath( std::u16string_view _rGroup )
{
    return OUString( OUString::Concat(ROOT_NODE) + "/" + _rGroup + "/" );
}
static OUString getPagePath( std::u16string_view _rPage )
{
    return OUString( OUString::Concat(PAGES_NODE) + "/" + _rPage + "/" );
}
static OUString getOptionPath( std::u16string_view _rOption )
{
    return OUString( OUString::Concat(OPTIONS_NODE) + "/" + _rOption + "/" );
}

bool SvtOptionsDialogOptions::IsHidden( const OUString& _rPath ) const
{
    bool bRet = false;
    OptionNodeList::const_iterator pIter = m_aOptionNodeList.find( _rPath );
    if ( pIter != m_aOptionNodeList.end() )
        bRet = pIter->second;
    return bRet;
}

bool SvtOptionsDialogOptions::IsGroupHidden( std::u16string_view _rGroup ) const
{
    return IsHidden( getGroupPath( _rGroup ) );
}

bool SvtOptionsDialogOptions::IsPageHidden( std::u16string_view _rPage, std::u16string_view _rGroup ) const
{
    return IsHidden( getGroupPath( _rGroup  ) + getPagePath( _rPage ) );
}

bool SvtOptionsDialogOptions::IsOptionHidden(
    std::u16string_view _rOption, std::u16string_view _rPage, std::u16string_view _rGroup ) const
{
    return IsHidden( getGroupPath( _rGroup  ) + getPagePath( _rPage ) + getOptionPath( _rOption ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
