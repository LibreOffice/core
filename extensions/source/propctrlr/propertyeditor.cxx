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

#include "propertyeditor.hxx"
#include "browserpage.hxx"
#include "linedescriptor.hxx"

#include <tools/debug.hxx>
#include <osl/diagnose.h>


namespace pcr
{


    #define LAYOUT_BORDER_LEFT      3
    #define LAYOUT_BORDER_TOP       3
    #define LAYOUT_BORDER_RIGHT     3
    #define LAYOUT_BORDER_BOTTOM    3

    using ::com::sun::star::uno::Any;
    using ::com::sun::star::inspection::XPropertyControl;
    using ::com::sun::star::uno::Reference;


    // class OPropertyEditor


    OPropertyEditor::OPropertyEditor( vcl::Window* pParent)
            :Control(pParent, WB_DIALOGCONTROL)
            ,m_aTabControl( VclPtr<TabControl>::Create(this) )
            ,m_pListener(nullptr)
            ,m_pObserver(nullptr)
            ,m_nNextId(1)
            ,m_bHasHelpSection( false )
            ,m_nMinHelpLines( 0 )
            ,m_nMaxHelpLines( 0 )
    {

        m_aTabControl->Show();
        m_aTabControl->SetDeactivatePageHdl(LINK(this, OPropertyEditor, OnPageDeactivate));
        m_aTabControl->SetActivatePageHdl(LINK(this, OPropertyEditor, OnPageActivate));
        m_aTabControl->SetBackground(GetBackground());
        m_aTabControl->SetPaintTransparent(true);
    }


    OPropertyEditor::~OPropertyEditor()
    {
        disposeOnce();
    }

    void OPropertyEditor::dispose()
    {
        Hide();
        ClearAll();
        m_aTabControl.disposeAndClear();
        Control::dispose();
    }


    void OPropertyEditor::ClearAll()
    {
        m_nNextId=1;
        sal_uInt16 nCount = m_aTabControl->GetPageCount();
        for(long i = nCount-1; i >= 0; --i)
        {
            sal_uInt16 nID = m_aTabControl->GetPageId(static_cast<sal_uInt16>(i));
            VclPtr<OBrowserPage> pPage = static_cast<OBrowserPage*>(m_aTabControl->GetTabPage(nID));
            if (pPage)
            {
                pPage->EnableInput(false);
                m_aTabControl->RemovePage(nID);
                pPage.disposeAndClear();
            }
        }
        m_aTabControl->Clear();

        {
            MapStringToPageId aEmpty;
            m_aPropertyPageIds.swap( aEmpty );
        }

        for (auto& rEntry : m_aHiddenPages)
            rEntry.second.pPage.disposeAndClear();
        m_aHiddenPages.clear();
    }


    sal_Int32 OPropertyEditor::getMinimumHeight()
    {
        sal_Int32 nMinHeight( LAYOUT_BORDER_TOP + LAYOUT_BORDER_BOTTOM );

        if ( m_aTabControl->GetPageCount() > 0 )
        {
            sal_uInt16 nFirstID = m_aTabControl->GetPageId( 0 );

            // reserve space for the tabs themself
            tools::Rectangle aTabArea( m_aTabControl->GetTabBounds( nFirstID ) );
            nMinHeight += aTabArea.GetHeight();

            // ask the page how much it requires
            OBrowserPage* pPage = static_cast< OBrowserPage* >( m_aTabControl->GetTabPage( nFirstID ) );
            if ( pPage )
                nMinHeight += pPage->getMinimumHeight();
        }
        else
            nMinHeight += 250;  // arbitrary ...

        return nMinHeight;
    }


    sal_Int32 OPropertyEditor::getMinimumWidth()
    {
        sal_uInt16 nCount = m_aTabControl->GetPageCount();
        sal_Int32 nPageMinWidth = 0;
        for(long i = nCount-1; i >= 0; --i)
        {
            sal_uInt16 nID = m_aTabControl->GetPageId(static_cast<sal_uInt16>(i));
            OBrowserPage* pPage = static_cast<OBrowserPage*>(m_aTabControl->GetTabPage(nID));
            if (pPage)
            {
                sal_Int32 nCurPageMinWidth = pPage->getMinimumWidth();
                if( nCurPageMinWidth > nPageMinWidth )
                    nPageMinWidth = nCurPageMinWidth;
            }
        }
        return nPageMinWidth+6;
    }


    void OPropertyEditor::CommitModified()
    {
        // commit all of my pages, if necessary

        sal_uInt16 nCount = m_aTabControl->GetPageCount();
        for ( sal_uInt16 i=0; i<nCount; ++i )
        {
            sal_uInt16 nID = m_aTabControl->GetPageId( i );
            OBrowserPage* pPage = static_cast< OBrowserPage* >( m_aTabControl->GetTabPage( nID ) );

            if ( pPage && pPage->getListBox().IsModified() )
                pPage->getListBox().CommitModified();
        }
    }


    void OPropertyEditor::GetFocus()
    {
        if ( m_aTabControl )
            m_aTabControl->GrabFocus();
    }


    OBrowserPage* OPropertyEditor::getPage( const OUString& _rPropertyName )
    {
        OBrowserPage* pPage = nullptr;
        MapStringToPageId::const_iterator aPropertyPageIdPos = m_aPropertyPageIds.find( _rPropertyName );
        if ( aPropertyPageIdPos != m_aPropertyPageIds.end() )
            pPage = static_cast< OBrowserPage* >( m_aTabControl->GetTabPage( aPropertyPageIdPos->second ) );
        return pPage;
    }


    const OBrowserPage* OPropertyEditor::getPage( const OUString& _rPropertyName ) const
    {
        return const_cast< OPropertyEditor* >( this )->getPage( _rPropertyName );
    }


    OBrowserPage* OPropertyEditor::getPage( sal_uInt16 _rPageId )
    {
        return static_cast< OBrowserPage* >( m_aTabControl->GetTabPage( _rPageId ) );
    }


    const OBrowserPage* OPropertyEditor::getPage( sal_uInt16 _rPageId ) const
    {
        return const_cast< OPropertyEditor* >( this )->getPage( _rPageId );
    }


    void OPropertyEditor::Resize()
    {
        tools::Rectangle aPlayground(
            Point( LAYOUT_BORDER_LEFT, LAYOUT_BORDER_TOP ),
            Size(
                GetOutputSizePixel().Width() - LAYOUT_BORDER_LEFT - LAYOUT_BORDER_RIGHT,
                GetOutputSizePixel().Height() - LAYOUT_BORDER_TOP - LAYOUT_BORDER_BOTTOM
            )
        );

        tools::Rectangle aTabArea( aPlayground );
        m_aTabControl->SetPosSizePixel( aTabArea.TopLeft(), aTabArea.GetSize() );
    }


    sal_uInt16 OPropertyEditor::AppendPage( const OUString & _rText, const OString& _rHelpId )
    {
        // obtain a new id
        sal_uInt16 nId = m_nNextId++;
        // insert the id
        m_aTabControl->InsertPage(nId, _rText);

        // create a new page
        VclPtrInstance<OBrowserPage> pPage(m_aTabControl.get());
        pPage->SetText( _rText );
        // some knittings
        pPage->SetSizePixel(m_aTabControl->GetTabPageSizePixel());
        pPage->getListBox().SetListener(m_pListener);
        pPage->getListBox().SetObserver(m_pObserver);
        pPage->getListBox().EnableHelpSection( m_bHasHelpSection );
        pPage->getListBox().SetHelpLineLimites( m_nMinHelpLines, m_nMaxHelpLines );
        pPage->SetHelpId( _rHelpId );

        // immediately activate the page
        m_aTabControl->SetTabPage(nId, pPage);
        m_aTabControl->SetCurPageId(nId);

        return nId;
    }


    void OPropertyEditor::SetHelpId( const OString& rHelpId )
    {
        Control::SetHelpId("");
        m_aTabControl->SetHelpId(rHelpId);
    }


    void OPropertyEditor::RemovePage(sal_uInt16 nID)
    {
        VclPtr<OBrowserPage> pPage = static_cast<OBrowserPage*>(m_aTabControl->GetTabPage(nID));

        if (pPage)
            pPage->EnableInput(false);
        m_aTabControl->RemovePage(nID);
        pPage.disposeAndClear();
    }


    void OPropertyEditor::SetPage(sal_uInt16 nId)
    {
        m_aTabControl->SetCurPageId(nId);
    }


    sal_uInt16 OPropertyEditor::GetCurPage()
    {
        if(m_aTabControl->GetPageCount()>0)
            return m_aTabControl->GetCurPageId();
        else
            return 0;
    }


    void OPropertyEditor::Update(const std::function<void(OBrowserListBox *)>& _aUpdateFunction)
    {
        // forward this to all our pages
        sal_uInt16 nCount = m_aTabControl->GetPageCount();
        for (sal_uInt16 i=0;i<nCount;++i)
        {
            sal_uInt16 nID = m_aTabControl->GetPageId(i);
            OBrowserPage* pPage = static_cast<OBrowserPage*>(m_aTabControl->GetTabPage(nID));
            if (pPage)
                _aUpdateFunction(&pPage->getListBox());
        }
    }

    void OPropertyEditor::EnableUpdate()
    {
        Update(std::mem_fn(&OBrowserListBox::EnableUpdate));
    }

    void OPropertyEditor::DisableUpdate()
    {
        Update(std::mem_fn(&OBrowserListBox::DisableUpdate));
    }


    void OPropertyEditor::forEachPage( PageOperation _pOperation )
    {
        sal_uInt16 nCount = m_aTabControl->GetPageCount();
        for ( sal_uInt16 i=0; i<nCount; ++i )
        {
            sal_uInt16 nID = m_aTabControl->GetPageId(i);
            OBrowserPage* pPage = static_cast< OBrowserPage* >( m_aTabControl->GetTabPage( nID ) );
            if ( !pPage )
                continue;
            (this->*_pOperation)( *pPage, nullptr );
        }
    }


    void OPropertyEditor::setPageLineListener( OBrowserPage& _rPage, const void* )
    {
        _rPage.getListBox().SetListener( m_pListener );
    }


    void OPropertyEditor::SetLineListener(IPropertyLineListener* _pListener)
    {
        m_pListener = _pListener;
        forEachPage( &OPropertyEditor::setPageLineListener );
    }


    void OPropertyEditor::setPageControlObserver( OBrowserPage& _rPage, const void* )
    {
        _rPage.getListBox().SetObserver( m_pObserver );
    }


    void OPropertyEditor::SetControlObserver( IPropertyControlObserver* _pObserver )
    {
        m_pObserver = _pObserver;
        forEachPage( &OPropertyEditor::setPageControlObserver );
    }


    void OPropertyEditor::EnableHelpSection( bool _bEnable )
    {
        m_bHasHelpSection = _bEnable;
        forEachPage( &OPropertyEditor::enableHelpSection );
    }


    void OPropertyEditor::SetHelpText( const OUString& _rHelpText )
    {
        sal_uInt16 nCount = m_aTabControl->GetPageCount();
        for ( sal_uInt16 i=0; i<nCount; ++i )
        {
            sal_uInt16 nID = m_aTabControl->GetPageId(i);
            OBrowserPage* pPage = static_cast< OBrowserPage* >( m_aTabControl->GetTabPage( nID ) );
            if ( !pPage )
                continue;
            setHelpSectionText( *pPage, &_rHelpText );
        }
    }


    void OPropertyEditor::SetHelpLineLimites( sal_Int32 _nMinLines, sal_Int32 _nMaxLines )
    {
        m_nMinHelpLines = _nMinLines;
        m_nMaxHelpLines = _nMaxLines;
        forEachPage( &OPropertyEditor::setHelpLineLimits );
    }


    void OPropertyEditor::enableHelpSection( OBrowserPage& _rPage, const void* )
    {
        _rPage.getListBox().EnableHelpSection( m_bHasHelpSection );
    }


    void OPropertyEditor::setHelpSectionText( OBrowserPage& _rPage, const void* _pPointerToOUString )
    {
        OSL_ENSURE( _pPointerToOUString, "OPropertyEditor::setHelpSectionText: invalid argument!" );
        if ( !_pPointerToOUString )
            return;

        const OUString& rText( *static_cast<const OUString*>(_pPointerToOUString) );
        _rPage.getListBox().SetHelpText( rText );
    }


    void OPropertyEditor::setHelpLineLimits( OBrowserPage& _rPage, const void* )
    {
        _rPage.getListBox().SetHelpLineLimites( m_nMinHelpLines, m_nMaxHelpLines );
    }


    void OPropertyEditor::InsertEntry( const OLineDescriptor& rData, sal_uInt16 _nPageId, sal_uInt16 nPos )
    {
        // let the current page handle this
        OBrowserPage* pPage = getPage( _nPageId );
        DBG_ASSERT( pPage, "OPropertyEditor::InsertEntry: don't have such a page!" );
        if ( !pPage )
            return;

        pPage->getListBox().InsertEntry( rData, nPos );

        OSL_ENSURE( m_aPropertyPageIds.find( rData.sName ) == m_aPropertyPageIds.end(),
            "OPropertyEditor::InsertEntry: property already present in the map!" );
        m_aPropertyPageIds.emplace( rData.sName, _nPageId );
    }


    void OPropertyEditor::RemoveEntry( const OUString& _rName )
    {
        OBrowserPage* pPage = getPage( _rName );
        if ( pPage )
        {
            OSL_VERIFY( pPage->getListBox().RemoveEntry( _rName ) );

            OSL_ENSURE( m_aPropertyPageIds.find( _rName ) != m_aPropertyPageIds.end(),
                "OPropertyEditor::RemoveEntry: property not present in the map!" );
            m_aPropertyPageIds.erase( _rName );
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


    void OPropertyEditor::ShowPropertyPage( sal_uInt16 _nPageId, bool _bShow )
    {
        if ( !_bShow )
        {
            sal_uInt16 nPagePos = m_aTabControl->GetPagePos( _nPageId );
            if ( TAB_PAGE_NOTFOUND == nPagePos )
                return;
            DBG_ASSERT( m_aHiddenPages.find( _nPageId ) == m_aHiddenPages.end(), "OPropertyEditor::ShowPropertyPage: page already hidden!" );

            m_aHiddenPages[ _nPageId ] = HiddenPage( nPagePos, m_aTabControl->GetTabPage( _nPageId ) );
            m_aTabControl->RemovePage( _nPageId );
        }
        else
        {
            std::map< sal_uInt16, HiddenPage >::iterator aPagePos = m_aHiddenPages.find( _nPageId );
            if ( aPagePos == m_aHiddenPages.end() )
                return;

            aPagePos->second.pPage->SetSizePixel( m_aTabControl->GetTabPageSizePixel() );
            m_aTabControl->InsertPage( aPagePos->first, aPagePos->second.pPage->GetText(), aPagePos->second.nPos );
            m_aTabControl->SetTabPage( aPagePos->first, aPagePos->second.pPage );

            m_aHiddenPages.erase( aPagePos );
        }
    }


    void OPropertyEditor::EnablePropertyControls( const OUString& _rEntryName, sal_Int16 _nControls, bool _bEnable )
    {
        for ( sal_uInt16 i = 0; i < m_aTabControl->GetPageCount(); ++i )
        {
            OBrowserPage* pPage = static_cast< OBrowserPage* >( m_aTabControl->GetTabPage( m_aTabControl->GetPageId( i ) ) );
            if ( pPage )
                pPage->getListBox().EnablePropertyControls( _rEntryName, _nControls, _bEnable );
        }
    }


    void OPropertyEditor::EnablePropertyLine( const OUString& _rEntryName, bool _bEnable )
    {
        for ( sal_uInt16 i = 0; i < m_aTabControl->GetPageCount(); ++i )
        {
            OBrowserPage* pPage = static_cast< OBrowserPage* >( m_aTabControl->GetTabPage( m_aTabControl->GetPageId( i ) ) );
            if ( pPage )
                pPage->getListBox().EnablePropertyLine( _rEntryName, _bEnable );
        }
    }


    Reference< XPropertyControl > OPropertyEditor::GetPropertyControl(const OUString& rEntryName)
    {
        Reference< XPropertyControl > xControl;
        // let the current page handle this
        OBrowserPage* pPage = static_cast<OBrowserPage*>(m_aTabControl->GetTabPage(m_aTabControl->GetCurPageId()));
        if (pPage)
            xControl = pPage->getListBox().GetPropertyControl(rEntryName);
        return xControl;
    }


    IMPL_LINK_NOARG(OPropertyEditor, OnPageActivate, TabControl*, void)
    {
        m_aPageActivationHandler.Call(nullptr);
    }


    IMPL_LINK_NOARG(OPropertyEditor, OnPageDeactivate, TabControl *, bool)
    {
        // commit the data on the current (to-be-deactivated) tab page
        // (79404)
        sal_Int32 nCurrentId = m_aTabControl->GetCurPageId();
        OBrowserPage* pCurrentPage = static_cast<OBrowserPage*>(m_aTabControl->GetTabPage(static_cast<sal_uInt16>(nCurrentId)));
        if ( !pCurrentPage )
            return true;

        if ( pCurrentPage->getListBox().IsModified() )
            pCurrentPage->getListBox().CommitModified();

        return true;
    }


} // namespace pcr


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
