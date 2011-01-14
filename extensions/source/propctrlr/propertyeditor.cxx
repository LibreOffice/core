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
#include "precompiled_extensions.hxx"
#include "propertyeditor.hxx"
#include "browserpage.hxx"
#include "linedescriptor.hxx"

/** === begin UNO includes === **/
/** === end UNO includes === **/
#include <tools/debug.hxx>

//............................................................................
namespace pcr
{
//............................................................................

    #define LAYOUT_BORDER_LEFT      3
    #define LAYOUT_BORDER_TOP       3
    #define LAYOUT_BORDER_RIGHT     3
    #define LAYOUT_BORDER_BOTTOM    3

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::inspection::XPropertyControl;
    using ::com::sun::star::uno::Reference;
    /** === end UNO using === **/

    //==================================================================
    // class OPropertyEditor
    //==================================================================
    DBG_NAME(OPropertyEditor)
    //------------------------------------------------------------------
    OPropertyEditor::OPropertyEditor( Window* pParent, WinBits nWinStyle)
            :Control(pParent, nWinStyle)
            ,m_aTabControl( this )
            ,m_nNextId(1)
            ,m_bHasHelpSection( false )
            ,m_nMinHelpLines( 0 )
            ,m_nMaxHelpLines( 0 )
    {
        DBG_CTOR(OPropertyEditor,NULL);

        m_aTabControl.Show();
        m_aTabControl.SetDeactivatePageHdl(LINK(this, OPropertyEditor, OnPageDeactivate));
        m_aTabControl.SetActivatePageHdl(LINK(this, OPropertyEditor, OnPageActivate));
        m_aTabControl.SetBackground(GetBackground());
        m_aTabControl.SetPaintTransparent(sal_True);
    }

    //------------------------------------------------------------------
    OPropertyEditor::~OPropertyEditor()
    {
        Hide();
        ClearAll();
        DBG_DTOR(OPropertyEditor,NULL);
    }

    //------------------------------------------------------------------
    void OPropertyEditor::ClearAll()
    {
        m_nNextId=1;
        sal_uInt16 nCount = m_aTabControl.GetPageCount();
        for(long i = nCount-1; i >= 0; --i)
        {
            sal_uInt16 nID = m_aTabControl.GetPageId((sal_uInt16)i);
            OBrowserPage* pPage = static_cast<OBrowserPage*>(m_aTabControl.GetTabPage(nID));
            if (pPage)
            {
                pPage->EnableInput(sal_False);
                m_aTabControl.RemovePage(nID);
                delete pPage;
            }
        }
        m_aTabControl.Clear();

        {
            MapStringToPageId aEmpty;
            m_aPropertyPageIds.swap( aEmpty );
        }

        while ( !m_aHiddenPages.empty() )
        {
            delete m_aHiddenPages.begin()->second.pPage;
            m_aHiddenPages.erase( m_aHiddenPages.begin() );
        }
    }

    //------------------------------------------------------------------
    sal_Int32 OPropertyEditor::getMinimumHeight()
    {
        sal_Int32 nMinHeight( LAYOUT_BORDER_TOP + LAYOUT_BORDER_BOTTOM );

        if ( m_aTabControl.GetPageCount() > 0 )
        {
            sal_uInt16 nFirstID = m_aTabControl.GetPageId( 0 );

            // reserve space for the tabs themself
            Rectangle aTabArea( m_aTabControl.GetTabBounds( nFirstID ) );
            nMinHeight += aTabArea.GetHeight();

            // ask the page how much it requires
            OBrowserPage* pPage = static_cast< OBrowserPage* >( m_aTabControl.GetTabPage( nFirstID ) );
            if ( pPage )
                nMinHeight += pPage->getMinimumHeight();
        }
        else
            nMinHeight += 250;  // arbitrary ...

        return nMinHeight;
    }

    //------------------------------------------------------------------
    sal_Int32 OPropertyEditor::getMinimumWidth()
    {
        sal_uInt16 nCount = m_aTabControl.GetPageCount();
        sal_Int32 nPageMinWidth = 0;
        for(long i = nCount-1; i >= 0; --i)
        {
            sal_uInt16 nID = m_aTabControl.GetPageId((sal_uInt16)i);
            OBrowserPage* pPage = static_cast<OBrowserPage*>(m_aTabControl.GetTabPage(nID));
            if (pPage)
            {
                sal_Int32 nCurPageMinWidth = pPage->getMinimumWidth();
                if( nCurPageMinWidth > nPageMinWidth )
                    nPageMinWidth = nCurPageMinWidth;
            }
        }
        return nPageMinWidth+6;
    }

    //------------------------------------------------------------------
    void OPropertyEditor::CommitModified()
    {
        // commit all of my pages, if necessary

        sal_uInt16 nCount = m_aTabControl.GetPageCount();
        for ( sal_uInt16 i=0; i<nCount; ++i )
        {
            sal_uInt16 nID = m_aTabControl.GetPageId( i );
            OBrowserPage* pPage = static_cast< OBrowserPage* >( m_aTabControl.GetTabPage( nID ) );

            if ( pPage && pPage->getListBox().IsModified() )
                pPage->getListBox().CommitModified();
        }
    }

    //------------------------------------------------------------------
    void OPropertyEditor::GetFocus()
    {
        m_aTabControl.GrabFocus();
    }

    //------------------------------------------------------------------
    OBrowserPage* OPropertyEditor::getPage( const ::rtl::OUString& _rPropertyName )
    {
        OBrowserPage* pPage = NULL;
        MapStringToPageId::const_iterator aPropertyPageIdPos = m_aPropertyPageIds.find( _rPropertyName );
        if ( aPropertyPageIdPos != m_aPropertyPageIds.end() )
            pPage = static_cast< OBrowserPage* >( m_aTabControl.GetTabPage( aPropertyPageIdPos->second ) );
        return pPage;
    }

    //------------------------------------------------------------------
    const OBrowserPage* OPropertyEditor::getPage( const ::rtl::OUString& _rPropertyName ) const
    {
        return const_cast< OPropertyEditor* >( this )->getPage( _rPropertyName );
    }

    //------------------------------------------------------------------
    OBrowserPage* OPropertyEditor::getPage( sal_uInt16& _rPageId )
    {
        return static_cast< OBrowserPage* >( m_aTabControl.GetTabPage( _rPageId ) );
    }

    //------------------------------------------------------------------
    const OBrowserPage* OPropertyEditor::getPage( sal_uInt16& _rPageId ) const
    {
        return const_cast< OPropertyEditor* >( this )->getPage( _rPageId );
    }

    //------------------------------------------------------------------
    void OPropertyEditor::Resize()
    {
        Rectangle aPlayground(
            Point( LAYOUT_BORDER_LEFT, LAYOUT_BORDER_TOP ),
            Size(
                GetOutputSizePixel().Width() - LAYOUT_BORDER_LEFT - LAYOUT_BORDER_RIGHT,
                GetOutputSizePixel().Height() - LAYOUT_BORDER_TOP - LAYOUT_BORDER_BOTTOM
            )
        );

        Rectangle aTabArea( aPlayground );
        m_aTabControl.SetPosSizePixel( aTabArea.TopLeft(), aTabArea.GetSize() );
    }

    //------------------------------------------------------------------
    sal_uInt16 OPropertyEditor::AppendPage( const String & _rText, const rtl::OString& _rHelpId )
    {
        // obtain a new id
        sal_uInt16 nId = m_nNextId++;
        // insert the id
        m_aTabControl.InsertPage(nId, _rText);

        // create a new page
        OBrowserPage* pPage = new OBrowserPage(&m_aTabControl);
        pPage->SetText( _rText );
        // some knittings
        pPage->SetSizePixel(m_aTabControl.GetTabPageSizePixel());
        pPage->getListBox().SetListener(m_pListener);
        pPage->getListBox().SetObserver(m_pObserver);
        pPage->getListBox().EnableHelpSection( m_bHasHelpSection );
        pPage->getListBox().SetHelpLineLimites( m_nMinHelpLines, m_nMaxHelpLines );
        pPage->SetHelpId( _rHelpId );

        // immediately activate the page
        m_aTabControl.SetTabPage(nId, pPage);
        m_aTabControl.SetCurPageId(nId);

        return nId;
    }

    //------------------------------------------------------------------
    void OPropertyEditor::SetHelpId( const rtl::OString& rHelpId )
    {
        Control::SetHelpId("");
        m_aTabControl.SetHelpId(rHelpId);
    }

    //------------------------------------------------------------------
    void OPropertyEditor::RemovePage(sal_uInt16 nID)
    {
        OBrowserPage* pPage = static_cast<OBrowserPage*>(m_aTabControl.GetTabPage(nID));

        if (pPage)
            pPage->EnableInput(sal_False);
        m_aTabControl.RemovePage(nID);
        if (pPage)
            delete pPage;
    }

    //------------------------------------------------------------------
    void OPropertyEditor::SetPage(sal_uInt16 nId)
    {
        m_aTabControl.SetCurPageId(nId);
    }

    //------------------------------------------------------------------
    sal_uInt16 OPropertyEditor::GetCurPage()
    {
        if(m_aTabControl.GetPageCount()>0)
            return m_aTabControl.GetCurPageId();
        else
            return 0;
    }

    //------------------------------------------------------------------
    void OPropertyEditor::Update(const ::std::mem_fun_t<void,OBrowserListBox>& _aUpdateFunction)
    {
        // forward this to all our pages
        sal_uInt16 nCount = m_aTabControl.GetPageCount();
        for (sal_uInt16 i=0;i<nCount;++i)
        {
            sal_uInt16 nID = m_aTabControl.GetPageId(i);
            OBrowserPage* pPage = static_cast<OBrowserPage*>(m_aTabControl.GetTabPage(nID));
            if (pPage)
                _aUpdateFunction(&pPage->getListBox());
        }
    }
    //------------------------------------------------------------------
    void OPropertyEditor::EnableUpdate()
    {
        Update(::std::mem_fun(&OBrowserListBox::EnableUpdate));
    }
    //------------------------------------------------------------------
    void OPropertyEditor::DisableUpdate()
    {
        Update(::std::mem_fun(&OBrowserListBox::DisableUpdate));
    }

    //------------------------------------------------------------------
    void OPropertyEditor::forEachPage( PageOperation _pOperation, const void* _pArgument )
    {
        sal_uInt16 nCount = m_aTabControl.GetPageCount();
        for ( sal_uInt16 i=0; i<nCount; ++i )
        {
            sal_uInt16 nID = m_aTabControl.GetPageId(i);
            OBrowserPage* pPage = static_cast< OBrowserPage* >( m_aTabControl.GetTabPage( nID ) );
            if ( !pPage )
                continue;
            (this->*_pOperation)( *pPage, _pArgument );
        }
    }

    //------------------------------------------------------------------
    void OPropertyEditor::setPageLineListener( OBrowserPage& _rPage, const void* )
    {
        _rPage.getListBox().SetListener( m_pListener );
    }

    //------------------------------------------------------------------
    void OPropertyEditor::SetLineListener(IPropertyLineListener* _pListener)
    {
        m_pListener = _pListener;
        forEachPage( &OPropertyEditor::setPageLineListener );
    }

    //------------------------------------------------------------------
    void OPropertyEditor::setPageControlObserver( OBrowserPage& _rPage, const void* )
    {
        _rPage.getListBox().SetObserver( m_pObserver );
    }

    //------------------------------------------------------------------
    void OPropertyEditor::SetControlObserver( IPropertyControlObserver* _pObserver )
    {
        m_pObserver = _pObserver;
        forEachPage( &OPropertyEditor::setPageControlObserver );
    }

    //------------------------------------------------------------------
    void OPropertyEditor::EnableHelpSection( bool _bEnable )
    {
        m_bHasHelpSection = _bEnable;
        forEachPage( &OPropertyEditor::enableHelpSection );
    }

    //------------------------------------------------------------------
    bool OPropertyEditor::HasHelpSection() const
    {
        return m_bHasHelpSection;
    }

    //------------------------------------------------------------------
    void OPropertyEditor::SetHelpText( const ::rtl::OUString& _rHelpText )
    {
        forEachPage( &OPropertyEditor::setHelpSectionText, &_rHelpText );
    }

    //------------------------------------------------------------------
    void OPropertyEditor::SetHelpLineLimites( sal_Int32 _nMinLines, sal_Int32 _nMaxLines )
    {
        m_nMinHelpLines = _nMinLines;
        m_nMaxHelpLines = _nMaxLines;
        forEachPage( &OPropertyEditor::setHelpLineLimits );
    }

    //------------------------------------------------------------------
    void OPropertyEditor::enableHelpSection( OBrowserPage& _rPage, const void* )
    {
        _rPage.getListBox().EnableHelpSection( m_bHasHelpSection );
    }

    //------------------------------------------------------------------
    void OPropertyEditor::setHelpSectionText( OBrowserPage& _rPage, const void* _pPointerToOUString )
    {
        OSL_ENSURE( _pPointerToOUString, "OPropertyEditor::setHelpSectionText: invalid argument!" );
        if ( !_pPointerToOUString )
            return;

        const ::rtl::OUString& rText( *(const ::rtl::OUString*)_pPointerToOUString );
        _rPage.getListBox().SetHelpText( rText );
    }

    //------------------------------------------------------------------
    void OPropertyEditor::setHelpLineLimits( OBrowserPage& _rPage, const void* )
    {
        _rPage.getListBox().SetHelpLineLimites( m_nMinHelpLines, m_nMaxHelpLines );
    }

    //------------------------------------------------------------------
    sal_uInt16 OPropertyEditor::InsertEntry( const OLineDescriptor& rData, sal_uInt16 _nPageId, sal_uInt16 nPos )
    {
        // let the current page handle this
        OBrowserPage* pPage = getPage( _nPageId );
        DBG_ASSERT( pPage, "OPropertyEditor::InsertEntry: don't have such a page!" );
        if ( !pPage )
            return LISTBOX_ENTRY_NOTFOUND;

        sal_uInt16 nEntry = pPage->getListBox().InsertEntry( rData, nPos );

        OSL_ENSURE( m_aPropertyPageIds.find( rData.sName ) == m_aPropertyPageIds.end(),
            "OPropertyEditor::InsertEntry: property already present in the map!" );
        m_aPropertyPageIds.insert( MapStringToPageId::value_type( rData.sName, _nPageId ) );

        return nEntry;
    }

    //------------------------------------------------------------------
    void OPropertyEditor::RemoveEntry( const ::rtl::OUString& _rName )
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

    //------------------------------------------------------------------
    void OPropertyEditor::ChangeEntry( const OLineDescriptor& rData )
    {
        OBrowserPage* pPage = getPage( rData.sName );
        if ( pPage )
            pPage->getListBox().ChangeEntry( rData, EDITOR_LIST_REPLACE_EXISTING );
    }

    //------------------------------------------------------------------
    void OPropertyEditor::SetPropertyValue( const ::rtl::OUString& rEntryName, const Any& _rValue, bool _bUnknownValue )
    {
        OBrowserPage* pPage = getPage( rEntryName );
        if ( pPage )
            pPage->getListBox().SetPropertyValue( rEntryName, _rValue, _bUnknownValue );
    }

    //------------------------------------------------------------------
    sal_uInt16 OPropertyEditor::GetPropertyPos( const ::rtl::OUString& rEntryName ) const
    {
        sal_uInt16 nVal=LISTBOX_ENTRY_NOTFOUND;
        const OBrowserPage* pPage = getPage( rEntryName );
        if ( pPage )
            nVal = pPage->getListBox().GetPropertyPos( rEntryName );
        return nVal;
    }

    //------------------------------------------------------------------
    void OPropertyEditor::ShowPropertyPage( sal_uInt16 _nPageId, bool _bShow )
    {
        if ( !_bShow )
        {
            sal_uInt16 nPagePos = m_aTabControl.GetPagePos( _nPageId );
            if ( TAB_PAGE_NOTFOUND == nPagePos )
                return;
            DBG_ASSERT( m_aHiddenPages.find( _nPageId ) == m_aHiddenPages.end(), "OPropertyEditor::ShowPropertyPage: page already hidden!" );

            m_aHiddenPages[ _nPageId ] = HiddenPage( nPagePos, m_aTabControl.GetTabPage( _nPageId ) );
            m_aTabControl.RemovePage( _nPageId );
        }
        else
        {
            ::std::map< sal_uInt16, HiddenPage >::iterator aPagePos = m_aHiddenPages.find( _nPageId );
            if ( aPagePos == m_aHiddenPages.end() )
                return;

            aPagePos->second.pPage->SetSizePixel( m_aTabControl.GetTabPageSizePixel() );
            m_aTabControl.InsertPage( aPagePos->first, aPagePos->second.pPage->GetText(), aPagePos->second.nPos );
            m_aTabControl.SetTabPage( aPagePos->first, aPagePos->second.pPage );

            m_aHiddenPages.erase( aPagePos );
        }
    }

    //------------------------------------------------------------------
    void OPropertyEditor::EnablePropertyControls( const ::rtl::OUString& _rEntryName, sal_Int16 _nControls, bool _bEnable )
    {
        for ( sal_uInt16 i = 0; i < m_aTabControl.GetPageCount(); ++i )
        {
            OBrowserPage* pPage = static_cast< OBrowserPage* >( m_aTabControl.GetTabPage( m_aTabControl.GetPageId( i ) ) );
            if ( pPage )
                pPage->getListBox().EnablePropertyControls( _rEntryName, _nControls, _bEnable );
        }
    }

    //------------------------------------------------------------------
    void OPropertyEditor::EnablePropertyLine( const ::rtl::OUString& _rEntryName, bool _bEnable )
    {
        for ( sal_uInt16 i = 0; i < m_aTabControl.GetPageCount(); ++i )
        {
            OBrowserPage* pPage = static_cast< OBrowserPage* >( m_aTabControl.GetTabPage( m_aTabControl.GetPageId( i ) ) );
            if ( pPage )
                pPage->getListBox().EnablePropertyLine( _rEntryName, _bEnable );
        }
    }

    //------------------------------------------------------------------
    Reference< XPropertyControl > OPropertyEditor::GetPropertyControl(const ::rtl::OUString& rEntryName)
    {
        Reference< XPropertyControl > xControl;
        // let the current page handle this
        OBrowserPage* pPage = static_cast<OBrowserPage*>(m_aTabControl.GetTabPage(m_aTabControl.GetCurPageId()));
        if (pPage)
            xControl = pPage->getListBox().GetPropertyControl(rEntryName);
        return xControl;
    }

    //------------------------------------------------------------------
    IMPL_LINK(OPropertyEditor, OnPageActivate, TabControl*, EMPTYARG)
    {
        if (m_aPageActivationHandler.IsSet())
            m_aPageActivationHandler.Call(NULL);
        return 0L;
    }

    //------------------------------------------------------------------
    IMPL_LINK(OPropertyEditor, OnPageDeactivate, TabControl*, EMPTYARG)
    {
        // commit the data on the current (to-be-decativated) tab page
        // (79404)
        sal_Int32 nCurrentId = m_aTabControl.GetCurPageId();
        OBrowserPage* pCurrentPage = static_cast<OBrowserPage*>(m_aTabControl.GetTabPage((sal_uInt16)nCurrentId));
        if ( !pCurrentPage )
            return 1L;

        if ( pCurrentPage->getListBox().IsModified() )
            pCurrentPage->getListBox().CommitModified();

        return 1L;
    }

//............................................................................
} // namespace pcr
//............................................................................


