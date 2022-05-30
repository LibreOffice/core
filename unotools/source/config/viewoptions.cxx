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

#include <unotools/viewoptions.hxx>
#include <com/sun/star/uno/Any.hxx>

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <unotools/configmgr.hxx>
#include <comphelper/configurationhelper.hxx>
#include <comphelper/processfactory.hxx>
#include <utility>
#include <tools/diagnose_ex.h>

constexpr OUStringLiteral PACKAGE_VIEWS = u"org.openoffice.Office.Views";
constexpr OUStringLiteral PROPERTY_WINDOWSTATE = u"WindowState";
constexpr OUStringLiteral PROPERTY_PAGEID = u"PageID";
constexpr OUStringLiteral PROPERTY_VISIBLE = u"Visible";
constexpr OUStringLiteral PROPERTY_USERDATA = u"UserData";


SvtViewOptions::SvtViewOptions( EViewType eType, OUString sViewName )
    :   m_eViewType ( eType     )
    ,   m_sViewName (std::move( sViewName ))
{
    (void)m_eViewType; // so the release build does not complain, since we only use it in assert
    // we must know, which view type we must support
    switch( eType )
    {
        case EViewType::Dialog: m_sListName = "Dialogs"; break;
        case EViewType::TabDialog: m_sListName = "TabDialogs"; break;
        case EViewType::TabPage: m_sListName = "TabPages"; break;
        case EViewType::Window: m_sListName = "Windows"; break;
        default: assert(false);
    }
    if (utl::ConfigManager::IsFuzzing())
        return;

    try
    {
        m_xRoot.set( ::comphelper::ConfigurationHelper::openConfig(
                            ::comphelper::getProcessComponentContext(),
                            PACKAGE_VIEWS,
                            ::comphelper::EConfigurationModes::Standard),
                     css::uno::UNO_QUERY);
        if (m_xRoot.is())
            m_xRoot->getByName(m_sListName) >>= m_xSet;
    }
    catch(const css::uno::Exception&)
        {
            TOOLS_WARN_EXCEPTION("unotools", "Unexpected exception");
            m_xRoot.clear();
            m_xSet.clear();
        }
}

//  public method

/*-************************************************************************************************************
    @short          checks for already existing entries
    @descr          If user don't know, if an entry already exist - he can get this information by calling this method.

    @seealso        member m_aList

    @param          "sName", name of entry to check exist state
    @return         true , if item exist
                    false, otherwise
*//*-*************************************************************************************************************/
bool SvtViewOptions::Exists() const
{
    bool bExists = false;

    try
    {
        if (m_xSet.is())
            bExists = m_xSet->hasByName(m_sViewName);
    }
    catch(const css::uno::Exception&)
        {
            TOOLS_WARN_EXCEPTION("unotools", "Unexpected exception");
            bExists = false;
        }

    return bExists;
}

//  public method

/*-************************************************************************************************************
    @short          delete entry
    @descr          Use it to delete set entry by given name.

    @seealso        member m_aList

    @param          "sName", name of entry to delete it
*//*-*************************************************************************************************************/
void SvtViewOptions::Delete()
{
    try
    {
        css::uno::Reference< css::container::XNameContainer > xSet(m_xSet, css::uno::UNO_QUERY_THROW);
        xSet->removeByName(m_sViewName);
        ::comphelper::ConfigurationHelper::flush(m_xRoot);
    }
    catch(const css::container::NoSuchElementException&)
        { }
    catch(const css::uno::Exception&)
        {
            TOOLS_WARN_EXCEPTION("unotools", "Unexpected exception");
        }
}

//  public method

/*-************************************************************************************************************
    @short          read/write access to cache view items and her properties
    @descr          Follow methods support read/write access to all cache view items.

    @seealso        member m_sList
*//*-*************************************************************************************************************/
OUString SvtViewOptions::GetWindowState() const
{
    OUString sWindowState;
    try
    {
        css::uno::Reference< css::beans::XPropertySet > xNode(
            impl_getSetNode(m_sViewName, false),
            css::uno::UNO_QUERY);
        if (xNode.is())
            xNode->getPropertyValue(PROPERTY_WINDOWSTATE) >>= sWindowState;
    }
    catch(const css::uno::Exception&)
        {
            TOOLS_WARN_EXCEPTION("unotools", "Unexpected exception");
            sWindowState.clear();
        }

    return sWindowState;
}


//  public method

/*-************************************************************************************************************
    @short          ctor
    @descr          We use it to open right configuration file and let configuration objects fill her caches.
                    Then we read all existing entries from right list and cached it inside our object too.
                    Normally we should enable notifications for changes on these values too ... but these feature
                    isn't full implemented in the moment.

    @seealso        baseclass ::utl::ConfigItem
    @seealso        method Notify()
*//*-*************************************************************************************************************/
void SvtViewOptions::SetWindowState( const OUString& sState )
{
    try
    {
        css::uno::Reference< css::beans::XPropertySet > xNode(
            impl_getSetNode(m_sViewName, true),
            css::uno::UNO_QUERY_THROW);
        xNode->setPropertyValue(PROPERTY_WINDOWSTATE, css::uno::Any(sState));
        ::comphelper::ConfigurationHelper::flush(m_xRoot);
    }
    catch(const css::uno::Exception&)
        {
            TOOLS_WARN_EXCEPTION("unotools", "Unexpected exception");
        }
}

//  public method

OString SvtViewOptions::GetPageID() const
{
    // Safe impossible cases.
    // These call isn't allowed for dialogs, tab-pages or windows!
    assert( m_eViewType == EViewType::TabDialog && "SvtViewOptions::GetPageID()\nCall not allowed for Dialogs, TabPages or Windows! I do nothing!" );

    OUString sID;
    try
    {
        css::uno::Reference< css::beans::XPropertySet > xNode(
            impl_getSetNode(m_sViewName, false),
            css::uno::UNO_QUERY);
        if (xNode.is())
            xNode->getPropertyValue(PROPERTY_PAGEID) >>= sID;
    }
    catch(const css::uno::Exception&)
        {
            TOOLS_WARN_EXCEPTION("unotools", "Unexpected exception");
        }

    return sID.toUtf8();
}


//  public method

void SvtViewOptions::SetPageID(std::string_view rID)
{
    // Safe impossible cases.
    // These call isn't allowed for dialogs, tab-pages or windows!
    assert( m_eViewType == EViewType::TabDialog && "SvtViewOptions::SetPageID()\nCall not allowed for Dialogs, TabPages or Windows! I do nothing!" );

    try
    {
        css::uno::Reference< css::beans::XPropertySet > xNode(
            impl_getSetNode(m_sViewName, true),
            css::uno::UNO_QUERY_THROW);
        xNode->setPropertyValue(PROPERTY_PAGEID, css::uno::Any(OUString::fromUtf8(rID)));
        ::comphelper::ConfigurationHelper::flush(m_xRoot);
    }
    catch(const css::uno::Exception&)
        {
            TOOLS_WARN_EXCEPTION("unotools", "Unexpected exception");
        }
}


//  public method

bool SvtViewOptions::IsVisible() const
{
    // Safe impossible cases.
    // These call isn't allowed for dialogs, tab-dialogs or tab-pages!
    assert( m_eViewType == EViewType::Window && "SvtViewOptions::IsVisible()\nCall not allowed for Dialogs, TabDialogs or TabPages! I do nothing!" );

    return GetVisible() == STATE_TRUE;
}

SvtViewOptions::State SvtViewOptions::GetVisible() const
{
    State eState = STATE_NONE;
    try
    {
        css::uno::Reference< css::beans::XPropertySet > xNode(
            impl_getSetNode(m_sViewName, false),
            css::uno::UNO_QUERY);
        if (xNode.is())
        {
            bool bVisible = false;
            if (xNode->getPropertyValue(PROPERTY_VISIBLE) >>= bVisible)
            {
                eState = bVisible ? STATE_TRUE : STATE_FALSE;
            }
        }
    }
    catch(const css::uno::Exception&)
        {
            TOOLS_WARN_EXCEPTION("unotools", "Unexpected exception");
        }
    return eState;
}

//  public method

void SvtViewOptions::SetVisible( bool bVisible )
{
    // Safe impossible cases.
    // These call isn't allowed for dialogs, tab-dialogs or tab-pages!
    assert(m_eViewType == EViewType::Window && "SvtViewOptions::SetVisible()\nCall not allowed for Dialogs, TabDialogs or TabPages! I do nothing!" );

    try
    {
        css::uno::Reference< css::beans::XPropertySet > xNode(
            impl_getSetNode(m_sViewName, true),
            css::uno::UNO_QUERY_THROW);
        xNode->setPropertyValue(PROPERTY_VISIBLE, css::uno::Any(bVisible));
        ::comphelper::ConfigurationHelper::flush(m_xRoot);
    }
    catch(const css::uno::Exception&)
        {
            TOOLS_WARN_EXCEPTION("unotools", "Unexpected exception");
        }
}

//  public method

bool SvtViewOptions::HasVisible() const
{
    // Safe impossible cases.
    // These call isn't allowed for dialogs, tab-dialogs or tab-pages!
    assert( m_eViewType == EViewType::Window && "SvtViewOptions::IsVisible()\nCall not allowed for Dialogs, TabDialogs or TabPages! I do nothing!" );

    return GetVisible() != STATE_NONE;
}

css::uno::Sequence< css::beans::NamedValue > SvtViewOptions::GetUserData() const
{
    try
    {
        css::uno::Reference< css::container::XNameAccess > xNode(
            impl_getSetNode(m_sViewName, false),
            css::uno::UNO_QUERY); // no _THROW ! because we don't create missing items here. So we have to live with zero references .-)
        css::uno::Reference< css::container::XNameAccess > xUserData;
        if (xNode.is())
            xNode->getByName(PROPERTY_USERDATA) >>= xUserData;
        if (xUserData.is())
        {
            const css::uno::Sequence<OUString> lNames = xUserData->getElementNames();
            sal_Int32 c = lNames.getLength();
            css::uno::Sequence< css::beans::NamedValue > lUserData(c);

            std::transform(lNames.begin(), lNames.end(), lUserData.getArray(),
                [&xUserData](const OUString& rName) -> css::beans::NamedValue {
                    return { rName, xUserData->getByName(rName) }; });

            return lUserData;
        }
    }
    catch(const css::uno::Exception&)
        {
            TOOLS_WARN_EXCEPTION("unotools", "Unexpected exception");
        }

    return css::uno::Sequence< css::beans::NamedValue >();
}

void SvtViewOptions::SetUserData( const css::uno::Sequence< css::beans::NamedValue >& lData )
{
    try
    {
        css::uno::Reference< css::container::XNameAccess > xNode(
            impl_getSetNode(m_sViewName, true),
            css::uno::UNO_QUERY_THROW);
        css::uno::Reference< css::container::XNameContainer > xUserData;
        xNode->getByName(PROPERTY_USERDATA) >>= xUserData;
        if (xUserData.is())
        {
            for (const css::beans::NamedValue& rData : lData)
            {
                if (xUserData->hasByName(rData.Name))
                    xUserData->replaceByName(rData.Name, rData.Value);
                else
                    xUserData->insertByName(rData.Name, rData.Value);
            }
        }
        ::comphelper::ConfigurationHelper::flush(m_xRoot);
    }
    catch(const css::uno::Exception&)
        {
            TOOLS_WARN_EXCEPTION("unotools", "Unexpected exception");
        }
}

css::uno::Any SvtViewOptions::GetUserItem( const OUString& sItemName ) const
{
    css::uno::Any aItem;
    try
    {
        css::uno::Reference< css::container::XNameAccess > xNode(
            impl_getSetNode(m_sViewName, false),
            css::uno::UNO_QUERY);
        css::uno::Reference< css::container::XNameAccess > xUserData;
        if (xNode.is())
            xNode->getByName(PROPERTY_USERDATA) >>= xUserData;
        if (xUserData.is())
            aItem = xUserData->getByName(sItemName);
    }
    catch(const css::container::NoSuchElementException&)
        { aItem.clear(); }
    catch(const css::uno::Exception&)
        {
            TOOLS_WARN_EXCEPTION("unotools", "Unexpected exception");
            aItem.clear();
        }

    return aItem;
}

void SvtViewOptions::SetUserItem( const OUString& sItemName  ,
                                  const css::uno::Any&   aValue )
{
    try
    {
        css::uno::Reference< css::container::XNameAccess > xNode(
            impl_getSetNode(m_sViewName, true),
            css::uno::UNO_QUERY_THROW);
        css::uno::Reference< css::container::XNameContainer > xUserData;
        xNode->getByName(PROPERTY_USERDATA) >>= xUserData;
        if (xUserData.is())
        {
            if (xUserData->hasByName(sItemName))
                xUserData->replaceByName(sItemName, aValue);
            else
                xUserData->insertByName(sItemName, aValue);
        }
        ::comphelper::ConfigurationHelper::flush(m_xRoot);
    }
    catch(const css::uno::Exception&)
        {
            TOOLS_WARN_EXCEPTION("unotools", "Unexpected exception");
        }
}



/*-************************************************************************************************************
    @short          create new set node with default values on disk
    @descr          To create a new UserData item - the super node of these property must already exist!
                    You can call this method to create these new entry with default values and change UserData then.

    @seealso        method impl_writeDirectProp()

    @param          "sNode", name of new entry
*//*-*************************************************************************************************************/
css::uno::Reference< css::uno::XInterface > SvtViewOptions::impl_getSetNode( const OUString& sNode           ,
                                                                                            bool         bCreateIfMissing) const
{
    css::uno::Reference< css::uno::XInterface > xNode;

    try
    {
        if (bCreateIfMissing)
            xNode = ::comphelper::ConfigurationHelper::makeSureSetNodeExists(m_xRoot, m_sListName, sNode);
        else
        {
            if (m_xSet.is() && m_xSet->hasByName(sNode) )
                m_xSet->getByName(sNode) >>= xNode;
        }
    }
    catch(const css::container::NoSuchElementException&)
        { xNode.clear(); }
    catch(const css::uno::Exception&)
        {
            TOOLS_WARN_EXCEPTION("unotools", "Unexpected exception");
            xNode.clear();
        }

    return xNode;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
