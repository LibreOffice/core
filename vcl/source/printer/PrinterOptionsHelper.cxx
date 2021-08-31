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

#include <sal/types.h>
#include <sal/log.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>
#include <tools/diagnose_ex.h>
#include <tools/debug.hxx>
#include <tools/urlobj.hxx>

#include <vcl/metaact.hxx>
#include <vcl/print.hxx>
#include <vcl/printer/PrinterOptions.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>

#include <configsettings.hxx>
#include <printdlg.hxx>
#include <salinst.hxx>
#include <salprn.hxx>
#include <strings.hrc>
#include <svdata.hxx>

#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/ui/dialogs/FilePicker.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/view/DuplexMode.hpp>

#include <unordered_map>
#include <unordered_set>

namespace vcl::print {

/*
 * PrinterOptionsHelper
**/
css::uno::Any PrinterOptionsHelper::getValue( const OUString& i_rPropertyName ) const
{
    css::uno::Any aRet;
    std::unordered_map< OUString, css::uno::Any >::const_iterator it =
        m_aPropertyMap.find( i_rPropertyName );
    if( it != m_aPropertyMap.end() )
        aRet = it->second;
    return aRet;
}

bool PrinterOptionsHelper::getBoolValue( const OUString& i_rPropertyName, bool i_bDefault ) const
{
    bool bRet = false;
    css::uno::Any aVal( getValue( i_rPropertyName ) );
    return (aVal >>= bRet) ? bRet : i_bDefault;
}

sal_Int64 PrinterOptionsHelper::getIntValue( const OUString& i_rPropertyName, sal_Int64 i_nDefault ) const
{
    sal_Int64 nRet = 0;
    css::uno::Any aVal( getValue( i_rPropertyName ) );
    return (aVal >>= nRet) ? nRet : i_nDefault;
}

OUString PrinterOptionsHelper::getStringValue( const OUString& i_rPropertyName ) const
{
    OUString aRet;
    css::uno::Any aVal( getValue( i_rPropertyName ) );
    return (aVal >>= aRet) ? aRet : OUString();
}

bool PrinterOptionsHelper::processProperties( const css::uno::Sequence< css::beans::PropertyValue >& i_rNewProp )
{
    bool bChanged = false;

    for( const auto& rVal : i_rNewProp )
    {
        std::unordered_map< OUString, css::uno::Any >::iterator it =
            m_aPropertyMap.find( rVal.Name );

        bool bElementChanged = (it == m_aPropertyMap.end()) || (it->second != rVal.Value);
        if( bElementChanged )
        {
            m_aPropertyMap[ rVal.Name ] = rVal.Value;
            bChanged = true;
        }
    }
    return bChanged;
}

void PrinterOptionsHelper::appendPrintUIOptions( css::uno::Sequence< css::beans::PropertyValue >& io_rProps ) const
{
    if( !m_aUIProperties.empty() )
    {
        sal_Int32 nIndex = io_rProps.getLength();
        io_rProps.realloc( nIndex+1 );
        css::beans::PropertyValue aVal;
        aVal.Name = "ExtraPrintUIOptions";
        aVal.Value <<= comphelper::containerToSequence(m_aUIProperties);
        io_rProps[ nIndex ] = aVal;
    }
}

css::uno::Any PrinterOptionsHelper::setUIControlOpt(const css::uno::Sequence< OUString >& i_rIDs,
                                          const OUString& i_rTitle,
                                          const css::uno::Sequence< OUString >& i_rHelpIds,
                                          const OUString& i_rType,
                                          const css::beans::PropertyValue* i_pVal,
                                          const PrinterOptionsHelper::UIControlOptions& i_rControlOptions)
{
    sal_Int32 nElements =
        2                                                             // ControlType + ID
        + (i_rTitle.isEmpty() ? 0 : 1)                                // Text
        + (i_rHelpIds.hasElements() ? 1 : 0)                          // HelpId
        + (i_pVal ? 1 : 0)                                            // Property
        + i_rControlOptions.maAddProps.size()                         // additional props
        + (i_rControlOptions.maGroupHint.isEmpty() ? 0 : 1)           // grouping
        + (i_rControlOptions.mbInternalOnly ? 1 : 0)                  // internal hint
        + (i_rControlOptions.mbEnabled ? 0 : 1)                       // enabled
        ;
    if( !i_rControlOptions.maDependsOnName.isEmpty() )
    {
        nElements += 1;
        if( i_rControlOptions.mnDependsOnEntry != -1 )
            nElements += 1;
        if( i_rControlOptions.mbAttachToDependency )
            nElements += 1;
    }

    css::uno::Sequence< css::beans::PropertyValue > aCtrl( nElements );
    sal_Int32 nUsed = 0;
    if( !i_rTitle.isEmpty() )
    {
        aCtrl[nUsed  ].Name  = "Text";
        aCtrl[nUsed++].Value <<= i_rTitle;
    }
    if( i_rHelpIds.hasElements() )
    {
        aCtrl[nUsed  ].Name = "HelpId";
        aCtrl[nUsed++].Value <<= i_rHelpIds;
    }
    aCtrl[nUsed  ].Name  = "ControlType";
    aCtrl[nUsed++].Value <<= i_rType;
    aCtrl[nUsed  ].Name  = "ID";
    aCtrl[nUsed++].Value <<= i_rIDs;
    if( i_pVal )
    {
        aCtrl[nUsed  ].Name  = "Property";
        aCtrl[nUsed++].Value <<= *i_pVal;
    }
    if( !i_rControlOptions.maDependsOnName.isEmpty() )
    {
        aCtrl[nUsed  ].Name  = "DependsOnName";
        aCtrl[nUsed++].Value <<= i_rControlOptions.maDependsOnName;
        if( i_rControlOptions.mnDependsOnEntry != -1 )
        {
            aCtrl[nUsed  ].Name  = "DependsOnEntry";
            aCtrl[nUsed++].Value <<= i_rControlOptions.mnDependsOnEntry;
        }
        if( i_rControlOptions.mbAttachToDependency )
        {
            aCtrl[nUsed  ].Name  = "AttachToDependency";
            aCtrl[nUsed++].Value <<= i_rControlOptions.mbAttachToDependency;
        }
    }
    if( !i_rControlOptions.maGroupHint.isEmpty() )
    {
        aCtrl[nUsed  ].Name    = "GroupingHint";
        aCtrl[nUsed++].Value <<= i_rControlOptions.maGroupHint;
    }
    if( i_rControlOptions.mbInternalOnly )
    {
        aCtrl[nUsed  ].Name    = "InternalUIOnly";
        aCtrl[nUsed++].Value <<= true;
    }
    if( ! i_rControlOptions.mbEnabled )
    {
        aCtrl[nUsed  ].Name    = "Enabled";
        aCtrl[nUsed++].Value <<= false;
    }

    sal_Int32 nAddProps = i_rControlOptions.maAddProps.size();
    for( sal_Int32 i = 0; i < nAddProps; i++ )
        aCtrl[ nUsed++ ] = i_rControlOptions.maAddProps[i];

    SAL_WARN_IF( nUsed != nElements, "vcl.gdi", "nUsed != nElements, probable heap corruption" );

    return css::uno::makeAny( aCtrl );
}

css::uno::Any PrinterOptionsHelper::setGroupControlOpt(const OUString& i_rID,
                                             const OUString& i_rTitle,
                                             const OUString& i_rHelpId)
{
    css::uno::Sequence< OUString > aHelpId;
    if( !i_rHelpId.isEmpty() )
    {
        aHelpId.realloc( 1 );
        *aHelpId.getArray() = i_rHelpId;
    }
    css::uno::Sequence< OUString > aIds { i_rID };
    return setUIControlOpt(aIds, i_rTitle, aHelpId, "Group");
}

css::uno::Any PrinterOptionsHelper::setSubgroupControlOpt(const OUString& i_rID,
                                                const OUString& i_rTitle,
                                                const OUString& i_rHelpId,
                                                const PrinterOptionsHelper::UIControlOptions& i_rControlOptions)
{
    css::uno::Sequence< OUString > aHelpId;
    if( !i_rHelpId.isEmpty() )
    {
        aHelpId.realloc( 1 );
        *aHelpId.getArray() = i_rHelpId;
    }
    css::uno::Sequence< OUString > aIds { i_rID };
    return setUIControlOpt(aIds, i_rTitle, aHelpId, "Subgroup", nullptr, i_rControlOptions);
}

css::uno::Any PrinterOptionsHelper::setBoolControlOpt(const OUString& i_rID,
                                            const OUString& i_rTitle,
                                            const OUString& i_rHelpId,
                                            const OUString& i_rProperty,
                                            bool i_bValue,
                                            const PrinterOptionsHelper::UIControlOptions& i_rControlOptions)
{
    css::uno::Sequence< OUString > aHelpId;
    if( !i_rHelpId.isEmpty() )
    {
        aHelpId.realloc( 1 );
        *aHelpId.getArray() = i_rHelpId;
    }
    css::beans::PropertyValue aVal;
    aVal.Name = i_rProperty;
    aVal.Value <<= i_bValue;
    css::uno::Sequence< OUString > aIds { i_rID };
    return setUIControlOpt(aIds, i_rTitle, aHelpId, "Bool", &aVal, i_rControlOptions);
}

css::uno::Any PrinterOptionsHelper::setChoiceRadiosControlOpt(const css::uno::Sequence< OUString >& i_rIDs,
                                              const OUString& i_rTitle,
                                              const css::uno::Sequence< OUString >& i_rHelpId,
                                              const OUString& i_rProperty,
                                              const css::uno::Sequence< OUString >& i_rChoices,
                                              sal_Int32 i_nValue,
                                              const css::uno::Sequence< sal_Bool >& i_rDisabledChoices,
                                              const PrinterOptionsHelper::UIControlOptions& i_rControlOptions)
{
    UIControlOptions aOpt( i_rControlOptions );
    sal_Int32 nUsed = aOpt.maAddProps.size();
    aOpt.maAddProps.resize( nUsed + 1 + (i_rDisabledChoices.hasElements() ? 1 : 0) );
    aOpt.maAddProps[nUsed].Name = "Choices";
    aOpt.maAddProps[nUsed].Value <<= i_rChoices;
    if( i_rDisabledChoices.hasElements() )
    {
        aOpt.maAddProps[nUsed+1].Name = "ChoicesDisabled";
        aOpt.maAddProps[nUsed+1].Value <<= i_rDisabledChoices;
    }

    css::beans::PropertyValue aVal;
    aVal.Name = i_rProperty;
    aVal.Value <<= i_nValue;
    return setUIControlOpt(i_rIDs, i_rTitle, i_rHelpId, "Radio", &aVal, aOpt);
}

css::uno::Any PrinterOptionsHelper::setChoiceListControlOpt(const OUString& i_rID,
                                              const OUString& i_rTitle,
                                              const css::uno::Sequence< OUString >& i_rHelpId,
                                              const OUString& i_rProperty,
                                              const css::uno::Sequence< OUString >& i_rChoices,
                                              sal_Int32 i_nValue,
                                              const css::uno::Sequence< sal_Bool >& i_rDisabledChoices,
                                              const PrinterOptionsHelper::UIControlOptions& i_rControlOptions)
{
    UIControlOptions aOpt( i_rControlOptions );
    sal_Int32 nUsed = aOpt.maAddProps.size();
    aOpt.maAddProps.resize( nUsed + 1 + (i_rDisabledChoices.hasElements() ? 1 : 0) );
    aOpt.maAddProps[nUsed].Name = "Choices";
    aOpt.maAddProps[nUsed].Value <<= i_rChoices;
    if( i_rDisabledChoices.hasElements() )
    {
        aOpt.maAddProps[nUsed+1].Name = "ChoicesDisabled";
        aOpt.maAddProps[nUsed+1].Value <<= i_rDisabledChoices;
    }

    css::beans::PropertyValue aVal;
    aVal.Name = i_rProperty;
    aVal.Value <<= i_nValue;
    css::uno::Sequence< OUString > aIds { i_rID };
    return setUIControlOpt(aIds, i_rTitle, i_rHelpId, "List", &aVal, aOpt);
}

css::uno::Any PrinterOptionsHelper::setRangeControlOpt(const OUString& i_rID,
                                             const OUString& i_rTitle,
                                             const OUString& i_rHelpId,
                                             const OUString& i_rProperty,
                                             sal_Int32 i_nValue,
                                             sal_Int32 i_nMinValue,
                                             sal_Int32 i_nMaxValue,
                                             const PrinterOptionsHelper::UIControlOptions& i_rControlOptions)
{
    UIControlOptions aOpt( i_rControlOptions );
    if( i_nMaxValue >= i_nMinValue )
    {
        sal_Int32 nUsed = aOpt.maAddProps.size();
        aOpt.maAddProps.resize( nUsed + 2 );
        aOpt.maAddProps[nUsed  ].Name  = "MinValue";
        aOpt.maAddProps[nUsed++].Value <<= i_nMinValue;
        aOpt.maAddProps[nUsed  ].Name  = "MaxValue";
        aOpt.maAddProps[nUsed++].Value <<= i_nMaxValue;
    }

    css::uno::Sequence< OUString > aHelpId;
    if( !i_rHelpId.isEmpty() )
    {
        aHelpId.realloc( 1 );
        *aHelpId.getArray() = i_rHelpId;
    }
    css::beans::PropertyValue aVal;
    aVal.Name = i_rProperty;
    aVal.Value <<= i_nValue;
    css::uno::Sequence< OUString > aIds { i_rID };
    return setUIControlOpt(aIds, i_rTitle, aHelpId, "Range", &aVal, aOpt);
}

css::uno::Any PrinterOptionsHelper::setEditControlOpt(const OUString& i_rID,
                                            const OUString& i_rTitle,
                                            const OUString& i_rHelpId,
                                            const OUString& i_rProperty,
                                            const OUString& i_rValue,
                                            const PrinterOptionsHelper::UIControlOptions& i_rControlOptions)
{
    css::uno::Sequence< OUString > aHelpId;
    if( !i_rHelpId.isEmpty() )
    {
        aHelpId.realloc( 1 );
        *aHelpId.getArray() = i_rHelpId;
    }
    css::beans::PropertyValue aVal;
    aVal.Name = i_rProperty;
    aVal.Value <<= i_rValue;
    css::uno::Sequence< OUString > aIds { i_rID };
    return setUIControlOpt(aIds, i_rTitle, aHelpId, "Edit", &aVal, i_rControlOptions);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
