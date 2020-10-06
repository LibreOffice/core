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

#include "handlerhelper.hxx"
#include "propertyeditor.hxx"
#include "browserpage.hxx"
#include "linedescriptor.hxx"

#include <tools/debug.hxx>
#include <osl/diagnose.h>

namespace pcr
{
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::inspection::XPropertyControl;
    using ::com::sun::star::uno::Reference;

    OPropertyEditor::OPropertyEditor(const css::uno::Reference<css::uno::XComponentContext>& rContext, weld::Builder& rBuilder)
        : m_xContainer(rBuilder.weld_container("box"))
        , m_xTabControl(rBuilder.weld_notebook("tabcontrol"))
        , m_xControlHoldingParent(rBuilder.weld_container("controlparent")) // controls initially have this parent before they are moved
        , m_xContext(rContext)
        , m_pListener(nullptr)
        , m_pObserver(nullptr)
        , m_nNextId(1)
        , m_bHasHelpSection(false)
    {
        PropertyHandlerHelper::setBuilderParent(rContext, m_xControlHoldingParent.get());

        m_xTabControl->connect_leave_page(LINK(this, OPropertyEditor, OnPageDeactivate));
        m_xTabControl->connect_enter_page(LINK(this, OPropertyEditor, OnPageActivate));
    }

    OPropertyEditor::~OPropertyEditor()
    {
        PropertyHandlerHelper::clearBuilderParent(m_xContext);
        ClearAll();
    }

    void OPropertyEditor::ClearAll()
    {
        m_nNextId=1;

        m_aPropertyPageIds.clear();
        m_aShownPages.clear();
        m_aHiddenPages.clear();

        int nCount = m_xTabControl->get_n_pages();
        for (int i = nCount - 1; i >= 0; --i)
        {
            OString sID = m_xTabControl->get_page_ident(i);
            m_xTabControl->remove_page(sID);
        }

        assert(m_xTabControl->get_n_pages() == 0);
    }

    Size OPropertyEditor::get_preferred_size() const
    {
        return m_xTabControl->get_preferred_size();
    }

    void OPropertyEditor::CommitModified()
    {
        // commit all of my pages, if necessary
        for (auto& page : m_aShownPages)
        {
            OBrowserPage* pPage = page.second.xPage.get();
            if (pPage && pPage->getListBox().IsModified() )
                pPage->getListBox().CommitModified();
        }
    }

    OBrowserPage* OPropertyEditor::getPage(const OUString& rPropertyName)
    {
        OBrowserPage* pPage = nullptr;
        MapStringToPageId::const_iterator aPropertyPageIdPos = m_aPropertyPageIds.find(rPropertyName);
        if (aPropertyPageIdPos != m_aPropertyPageIds.end())
            pPage = getPage(aPropertyPageIdPos->second);
        return pPage;
    }

    const OBrowserPage* OPropertyEditor::getPage( const OUString& _rPropertyName ) const
    {
        return const_cast< OPropertyEditor* >( this )->getPage( _rPropertyName );
    }

    OBrowserPage* OPropertyEditor::getPage(sal_uInt16 rPageId)
    {
        OBrowserPage* pPage = nullptr;
        auto aPagePos = m_aShownPages.find(rPageId);
        if (aPagePos != m_aShownPages.end())
            pPage = aPagePos->second.xPage.get();
        return pPage;
    }

    const OBrowserPage* OPropertyEditor::getPage(sal_uInt16 rPageId) const
    {
        return const_cast<OPropertyEditor*>(this)->getPage(rPageId);
    }

    sal_uInt16 OPropertyEditor::AppendPage(const OUString& rText, const OString& rHelpId)
    {
        // obtain a new id
        sal_uInt16 nId = m_nNextId++;
        // insert the id
        OString sIdent = OString::number(nId);
        m_xTabControl->append_page(sIdent, rText);

        // create a new page
        auto xPage = std::make_unique<OBrowserPage>(m_xTabControl->get_page(sIdent), m_xControlHoldingParent.get());
        // some knittings
        xPage->getListBox().SetListener(m_pListener);
        xPage->getListBox().SetObserver(m_pObserver);
        xPage->getListBox().EnableHelpSection(m_bHasHelpSection);
        xPage->SetHelpId(rHelpId);

        m_aShownPages[nId] = PropertyPage(m_xTabControl->get_n_pages() - 1, rText, std::move(xPage));

        // immediately activate the page
        m_xTabControl->set_current_page(sIdent);

        return nId;
    }

    void OPropertyEditor::SetHelpId( const OString& rHelpId )
    {
        m_xTabControl->set_help_id(rHelpId);
    }

    void OPropertyEditor::RemovePage(sal_uInt16 nID)
    {
        auto aPagePos = m_aShownPages.find(nID);
        if (aPagePos == m_aShownPages.end())
            return;

        m_aShownPages.erase(aPagePos);
        OString sIdent(OString::number(nID));
        m_xTabControl->remove_page(sIdent);
    }

    void OPropertyEditor::SetPage(sal_uInt16 nId)
    {
        m_xTabControl->set_current_page(OString::number(nId));
    }

    sal_uInt16 OPropertyEditor::GetCurPage() const
    {
        return m_xTabControl->get_current_page_ident().toUInt32();
    }

    void OPropertyEditor::forEachPage( PageOperation _pOperation )
    {
        int nCount = m_xTabControl->get_n_pages();
        for (int i = 0; i < nCount; ++i)
        {
            sal_uInt16 nID = m_xTabControl->get_page_ident(i).toUInt32();
            OBrowserPage* pPage = getPage(nID);
            if (!pPage)
                continue;
            (this->*_pOperation)( *pPage, nullptr );
        }
    }

    void OPropertyEditor::setPageLineListener( OBrowserPage& rPage, const void* )
    {
        rPage.getListBox().SetListener( m_pListener );
    }

    void OPropertyEditor::SetLineListener(IPropertyLineListener* pListener)
    {
        m_pListener = pListener;
        forEachPage( &OPropertyEditor::setPageLineListener );
    }

    void OPropertyEditor::setPageControlObserver( OBrowserPage& rPage, const void* )
    {
        rPage.getListBox().SetObserver( m_pObserver );
    }

    void OPropertyEditor::SetControlObserver( IPropertyControlObserver* _pObserver )
    {
        m_pObserver = _pObserver;
        forEachPage( &OPropertyEditor::setPageControlObserver );
    }

    void OPropertyEditor::EnableHelpSection( bool bEnable )
    {
        m_bHasHelpSection = bEnable;
        forEachPage( &OPropertyEditor::enableHelpSection );
    }

    void OPropertyEditor::SetHelpText( const OUString& rHelpText )
    {
        int nCount = m_xTabControl->get_n_pages();
        for (int i = 0; i < nCount; ++i)
        {
            sal_uInt16 nID = m_xTabControl->get_page_ident(i).toUInt32();
            OBrowserPage* pPage = getPage(nID);
            if (!pPage)
                continue;
            setHelpSectionText( *pPage, &rHelpText );
        }
    }

    void OPropertyEditor::enableHelpSection( OBrowserPage& rPage, const void* )
    {
        rPage.getListBox().EnableHelpSection( m_bHasHelpSection );
    }

    void OPropertyEditor::setHelpSectionText( OBrowserPage& rPage, const void* pPointerToOUString )
    {
        OSL_ENSURE( pPointerToOUString, "OPropertyEditor::setHelpSectionText: invalid argument!" );
        if ( !pPointerToOUString )
            return;

        const OUString& rText( *static_cast<const OUString*>(pPointerToOUString) );
        rPage.getListBox().SetHelpText( rText );
    }

    void OPropertyEditor::InsertEntry( const OLineDescriptor& rData, sal_uInt16 nPageId, sal_uInt16 nPos )
    {
        // let the current page handle this
        OBrowserPage* pPage = getPage(nPageId);
        DBG_ASSERT( pPage, "OPropertyEditor::InsertEntry: don't have such a page!" );
        if ( !pPage )
            return;

        pPage->getListBox().InsertEntry( rData, nPos );

        OSL_ENSURE( m_aPropertyPageIds.find( rData.sName ) == m_aPropertyPageIds.end(),
            "OPropertyEditor::InsertEntry: property already present in the map!" );
        m_aPropertyPageIds.emplace( rData.sName, nPageId );
    }

    void OPropertyEditor::RemoveEntry( const OUString& rName )
    {
        OBrowserPage* pPage = getPage( rName );
        if ( pPage )
        {
            OSL_VERIFY( pPage->getListBox().RemoveEntry( rName ) );

            OSL_ENSURE( m_aPropertyPageIds.find( rName ) != m_aPropertyPageIds.end(),
                "OPropertyEditor::RemoveEntry: property not present in the map!" );
            m_aPropertyPageIds.erase( rName );
        }
    }

    void OPropertyEditor::ChangeEntry( const OLineDescriptor& rData )
    {
        OBrowserPage* pPage = getPage( rData.sName );
        if ( pPage )
            pPage->getListBox().ChangeEntry( rData, EDITOR_LIST_REPLACE_EXISTING );
    }

    void OPropertyEditor::SetPropertyValue( const OUString& rEntryName, const Any& _rValue, bool _bUnknownValue )
    {
        OBrowserPage* pPage = getPage( rEntryName );
        if ( pPage )
            pPage->getListBox().SetPropertyValue( rEntryName, _rValue, _bUnknownValue );
    }

    sal_uInt16 OPropertyEditor::GetPropertyPos( const OUString& rEntryName ) const
    {
        sal_uInt16 nVal=EDITOR_LIST_ENTRY_NOTFOUND;
        const OBrowserPage* pPage = getPage( rEntryName );
        if ( pPage )
            nVal = pPage->getListBox().GetPropertyPos( rEntryName );
        return nVal;
    }

    void OPropertyEditor::ShowPropertyPage(sal_uInt16 nPageId, bool bShow)
    {
        assert((m_aHiddenPages.find(nPageId) != m_aHiddenPages.end() ||
                m_aShownPages.find(nPageId) != m_aShownPages.end()) && "page doesn't exist");
        OString sIdent(OString::number(nPageId));
        if (!bShow)
        {
            auto aPagePos = m_aShownPages.find(nPageId);
            if (aPagePos != m_aShownPages.end())
            {
                aPagePos->second.xPage->detach();
                m_xTabControl->remove_page(sIdent);

                m_aHiddenPages[nPageId] = std::move(aPagePos->second);
                m_aShownPages.erase(aPagePos);
            }
        }
        else
        {
            auto aPagePos = m_aHiddenPages.find(nPageId);
            if (aPagePos != m_aHiddenPages.end())
            {
                m_xTabControl->insert_page(sIdent, aPagePos->second.sLabel, aPagePos->second.nPos);
                aPagePos->second.xPage->reattach(m_xTabControl->get_page(sIdent));

                m_aShownPages[nPageId] = std::move(aPagePos->second);
                m_aHiddenPages.erase(aPagePos);
            }
        }
    }

    void OPropertyEditor::EnablePropertyControls( const OUString& rEntryName, sal_Int16 nControls, bool bEnable )
    {
        for (auto& rPage : m_aShownPages)
        {
            OBrowserPage* pPage = rPage.second.xPage.get();
            if (pPage)
                pPage->getListBox().EnablePropertyControls( rEntryName, nControls, bEnable );
        }
    }

    void OPropertyEditor::EnablePropertyLine( const OUString& rEntryName, bool bEnable )
    {
        for (auto& rPage : m_aShownPages)
        {
            OBrowserPage* pPage = rPage.second.xPage.get();
            if (pPage)
                pPage->getListBox().EnablePropertyLine( rEntryName, bEnable );
        }
    }

    Reference< XPropertyControl > OPropertyEditor::GetPropertyControl(const OUString& rEntryName)
    {
        Reference< XPropertyControl > xControl;
        // let the current page handle this
        OBrowserPage* pPage = getPage(m_xTabControl->get_current_page_ident().toUInt32());
        if (pPage)
            xControl = pPage->getListBox().GetPropertyControl(rEntryName);
        return xControl;
    }

    IMPL_LINK_NOARG(OPropertyEditor, OnPageActivate, const OString&, void)
    {
        m_aPageActivationHandler.Call(nullptr);
    }

    IMPL_LINK(OPropertyEditor, OnPageDeactivate, const OString&, rIdent, bool)
    {
        // commit the data on the current (to-be-deactivated) tab page
        // (79404)
        OBrowserPage* pCurrentPage = getPage(rIdent.toUInt32());
        if (!pCurrentPage)
            return true;

        if (pCurrentPage->getListBox().IsModified())
            pCurrentPage->getListBox().CommitModified();

        return true;
    }

    OPropertyEditor::PropertyPage::PropertyPage()
        : nPos(0)
    {
    }

    OPropertyEditor::PropertyPage::PropertyPage(sal_uInt16 nPagePos, const OUString& rLabel, std::unique_ptr<OBrowserPage> pPage)
        : nPos(nPagePos), sLabel(rLabel), xPage(std::move(pPage))
    {
    }

} // namespace pcr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
