/*************************************************************************
 *
 *  $RCSfile: propertyeditor.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: fs $ $Date: 2001-02-19 14:08:31 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _EXTENSIONS_PROPCTRLR_PROPERTYEDITOR_HXX_
#include "propertyeditor.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _EXTENSIONS_PROPCTRLR_BROWSERPAGE_HXX_
#include "browserpage.hxx"
#endif

//............................................................................
namespace pcr
{
//............................................................................

    //==================================================================
    // class OPropertyEditor
    //==================================================================
    DBG_NAME(OPropertyEditor)
    //------------------------------------------------------------------
    OPropertyEditor::OPropertyEditor( Window* pParent, WinBits nWinStyle)
            :Control(pParent, nWinStyle)
            ,m_aTabControl(this)
            ,m_nNextId(1)
    {
        DBG_CTOR(OPropertyEditor,NULL);

        m_aTabControl.Show();
        m_aTabControl.SetDeactivatePageHdl(LINK(this, OPropertyEditor, OnPageDeactivate));
        m_aTabControl.SetActivatePageHdl(LINK(this, OPropertyEditor, OnPageActivate));
        m_aTabControl.SetBackground(GetBackground());
    }

    //------------------------------------------------------------------
    OPropertyEditor::OPropertyEditor( Window* pParent, const ResId& rResId )
                : Control(pParent,rResId)
                  ,m_aTabControl(this)
                  ,m_nNextId(1)
    {
        DBG_CTOR(OPropertyEditor,NULL);

        m_aTabControl.Show();
        m_aTabControl.SetDeactivatePageHdl(LINK(this, OPropertyEditor, OnPageDeactivate));
        m_aTabControl.SetActivatePageHdl(LINK(this, OPropertyEditor, OnPageActivate));

        SetHelpId(GetHelpId());
        m_aTabControl.SetBackground(GetBackground());
        m_aTabControl.SetPaintTransparent(sal_True);

        Resize();
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
    }

    //------------------------------------------------------------------
    void OPropertyEditor::Resize()
    {
        Point aPos(3,3);
        Size aSize(GetOutputSizePixel());
        aSize.Width()-=6;
        aSize.Height()-=6;

        m_aTabControl.SetPosSizePixel(aPos, aSize);

    }

    //------------------------------------------------------------------
    sal_uInt16 OPropertyEditor::AppendPage(const String & _rText,sal_uInt32 _nHelpId)
    {
        // obtain a new id
        sal_uInt16 nId = m_nNextId++;
        // insert the id
        m_aTabControl.InsertPage(nId, _rText);

        // create a new page
        OBrowserPage* pPage = new OBrowserPage(&m_aTabControl);
        // some knittings
        pPage->SetSizePixel(m_aTabControl.GetOutputSizePixel());
        pPage->getListBox()->setListener(m_pListener);
        pPage->SetHelpId(_nHelpId);

        // immediately activate the page
        m_aTabControl.SetTabPage(nId, pPage);
        m_aTabControl.SetCurPageId(nId);

        return nId;
    }

    //------------------------------------------------------------------
    void OPropertyEditor::SetHelpId( sal_uInt32 nHelpId )
    {
        Control::SetHelpId(0);
        m_aTabControl.SetHelpId(nHelpId);
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
    sal_uInt16 OPropertyEditor::CalcVisibleLines()
    {
        OBrowserPage* pPage = static_cast<OBrowserPage*>(m_aTabControl.GetTabPage(m_aTabControl.GetCurPageId()));
        if (pPage)
            return pPage->getListBox()->CalcVisibleLines();
        else return 0;
    }

    //------------------------------------------------------------------
    void OPropertyEditor::EnableUpdate()
    {
        // forward this to all our pages
        sal_uInt16 nCount = m_aTabControl.GetPageCount();
        for (sal_uInt16 i=0;i<nCount;++i)
        {
            sal_uInt16 nID = m_aTabControl.GetPageId(i);
            OBrowserPage* pPage = static_cast<OBrowserPage*>(m_aTabControl.GetTabPage(nID));
            if (pPage)
                pPage->getListBox()->EnableUpdate();
        }
    }

    //------------------------------------------------------------------
    void OPropertyEditor::DisableUpdate()
    {
        // forward this to all our pages
        sal_uInt16 nCount = m_aTabControl.GetPageCount();
        for (sal_uInt16 i=0;i<nCount;++i)
        {
            sal_uInt16 nID = m_aTabControl.GetPageId(i);
            OBrowserPage* pPage = static_cast<OBrowserPage*>(m_aTabControl.GetTabPage(nID));
            if (pPage)
                pPage->getListBox()->DisableUpdate();
        }
    }

    //------------------------------------------------------------------
    void OPropertyEditor::SetLineListener(IPropertyLineListener* _pListener)
    {
        m_pListener = _pListener;

        // forward the new listener to our pages
        sal_uInt16 nCount = m_aTabControl.GetPageCount();
        for (sal_uInt16 i=0;i<nCount;++i)
        {
            sal_uInt16 nID = m_aTabControl.GetPageId(i);
            OBrowserPage* pPage = static_cast<OBrowserPage*>(m_aTabControl.GetTabPage(nID));
            if (pPage)
                pPage->getListBox()->setListener(m_pListener);
        }

    }

    //------------------------------------------------------------------
    sal_uInt16 OPropertyEditor::InsertEntry( const OLineDescriptor& rData, sal_uInt16 nPos)
    {
        // let the current page handle this
        sal_uInt16 nEntry = LISTBOX_ENTRY_NOTFOUND;
        OBrowserPage* pPage = static_cast<OBrowserPage*>(m_aTabControl.GetTabPage(m_aTabControl.GetCurPageId()));
        if (pPage)
            nEntry=pPage->getListBox()->InsertEntry(rData,nPos);

        return nEntry;
    }

    //------------------------------------------------------------------
    void OPropertyEditor::ChangeEntry( const OLineDescriptor& rData, sal_uInt16 nPos)
    {
        // let the current page handle this
        OBrowserPage* pPage = static_cast<OBrowserPage*>(m_aTabControl.GetTabPage(m_aTabControl.GetCurPageId()));
        if (pPage)
            pPage->getListBox()->ChangeEntry(rData,nPos);
    }

    //------------------------------------------------------------------
    sal_uInt16 OPropertyEditor::AppendEntry( const OLineDescriptor& rData)
    {
        // let the current page handle this
        sal_uInt16 nEntry = LISTBOX_ENTRY_NOTFOUND;
        OBrowserPage* pPage = static_cast<OBrowserPage*>(m_aTabControl.GetTabPage(m_aTabControl.GetCurPageId()));
        if (pPage)
            nEntry = pPage->getListBox()->AppendEntry(rData);

        return nEntry;
    }

    //------------------------------------------------------------------
    void OPropertyEditor::SetPropertyValue( const ::rtl::OUString & rEntryName, const ::rtl::OUString & rValue )
    {
        // let the current page handle this
        OBrowserPage* pPage = static_cast<OBrowserPage*>(m_aTabControl.GetTabPage(m_aTabControl.GetCurPageId()));
        if (pPage)
            pPage->getListBox()->SetPropertyValue( rEntryName, rValue );
    }

    //------------------------------------------------------------------
    ::rtl::OUString OPropertyEditor::GetPropertyValue( const ::rtl::OUString& rEntryName ) const
    {
        // let the current page handle this
        ::rtl::OUString aString;
        OBrowserPage* pPage = static_cast<OBrowserPage*>(m_aTabControl.GetTabPage(m_aTabControl.GetCurPageId()));
        if(pPage)
            aString=pPage->getListBox()->GetPropertyValue( rEntryName );
        return aString;
    }

    //------------------------------------------------------------------
    sal_uInt16 OPropertyEditor::GetPropertyPos( const ::rtl::OUString& rEntryName ) const
    {
        // let the current page handle this
        sal_uInt16 nVal=LISTBOX_ENTRY_NOTFOUND;
        OBrowserPage* pPage = static_cast<OBrowserPage*>(m_aTabControl.GetTabPage(m_aTabControl.GetCurPageId()));
        if(pPage)
            nVal=pPage->getListBox()->GetPropertyPos( rEntryName );
        return nVal;
    }

    //------------------------------------------------------------------
    IBrowserControl* OPropertyEditor::GetPropertyControl(const ::rtl::OUString& rEntryName)
    {
        // let the current page handle this
        OBrowserPage* pPage = static_cast<OBrowserPage*>(m_aTabControl.GetTabPage(m_aTabControl.GetCurPageId()));
        if (pPage)
            return pPage->getListBox()->GetPropertyControl(rEntryName);
        return NULL;
    }

    //------------------------------------------------------------------
    void OPropertyEditor::SetPropertyData( const ::rtl::OUString& rEntryName, void* pData )
    {
        // let the current page handle this
        OBrowserPage* pPage = static_cast<OBrowserPage*>(m_aTabControl.GetTabPage(m_aTabControl.GetCurPageId()));
        if (pPage)
            pPage->getListBox()->SetPropertyData(rEntryName, pData);

    }

    //------------------------------------------------------------------
    void OPropertyEditor::SetFirstVisibleEntry(sal_uInt16 nPos)
    {
        // let the current page handle this
        OBrowserPage* pPage = static_cast<OBrowserPage*>(m_aTabControl.GetTabPage(m_aTabControl.GetCurPageId()));
        if(pPage)
            pPage->getListBox()->SetFirstVisibleEntry(nPos);
    }

    //------------------------------------------------------------------
    sal_uInt16 OPropertyEditor::GetFirstVisibleEntry()
    {
        // let the current page handle this
        sal_uInt16 nEntry=LISTBOX_ENTRY_NOTFOUND;
        OBrowserPage* pPage = static_cast<OBrowserPage*>(m_aTabControl.GetTabPage(m_aTabControl.GetCurPageId()));
        if(pPage)
            nEntry=pPage->getListBox()->GetFirstVisibleEntry();
        return nEntry;
    }

    //------------------------------------------------------------------
    void OPropertyEditor::SetSelectedEntry(sal_uInt16 nPos)
    {
        // let the current page handle this
        OBrowserPage* pPage = static_cast<OBrowserPage*>(m_aTabControl.GetTabPage(m_aTabControl.GetCurPageId()));
        if(pPage)
            pPage->getListBox()->SetSelectedEntry(nPos);
    }

    //------------------------------------------------------------------
    sal_uInt16 OPropertyEditor::GetSelectedEntry()
    {
        sal_uInt16 nEntry=LISTBOX_ENTRY_NOTFOUND;
        // let the current page handle this
        OBrowserPage* pPage = static_cast<OBrowserPage*>(m_aTabControl.GetTabPage(m_aTabControl.GetCurPageId()));
        if(pPage)
            nEntry=pPage->getListBox()->GetSelectedEntry();
        return nEntry;
    }

    //------------------------------------------------------------------
    void OPropertyEditor::ClearTable()
    {
        // let the current page handle this
        OBrowserPage* pPage = static_cast<OBrowserPage*>(m_aTabControl.GetTabPage(m_aTabControl.GetCurPageId()));
        if(pPage)
            pPage->getListBox()->Clear();
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
        OBrowserListBox* pListBox = pCurrentPage ? pCurrentPage->getListBox() : NULL;
        IBrowserControl* pControl = pListBox ? pListBox->GetCurrentPropertyControl() : NULL;
        if (pControl && pControl->IsModified())
            pControl->CommitModified();

        return 1L;
    }

//............................................................................
} // namespace pcr
//............................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.2  2001/01/18 14:45:10  rt
 *  #65293# semicolon removed
 *
 *  Revision 1.1  2001/01/12 11:31:24  fs
 *  initial checkin - outsourced the form property browser
 *
 *
 *  Revision 1.0 08.01.01 15:38:46  fs
 ************************************************************************/

