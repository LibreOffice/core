/*************************************************************************
 *
 *  $RCSfile: browserlistbox.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2003-10-21 09:03:53 $
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

#ifndef _EXTENSIONS_PROPCTRLR_BROWSERLISTBOX_HXX_
#include "browserlistbox.hxx"
#endif
#ifndef _EXTENSIONS_PROPCTRLR_PROPRESID_HRC_
#include "propresid.hrc"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _EXTENSIONS_PROPCTRLR_PROPLINELISTENER_HXX_
#include "proplinelistener.hxx"
#endif
#ifndef _EXTENSIONS_PROPCTRLR_LINEDESCRIPTOR_HXX_
#include "linedescriptor.hxx"
#endif
#ifndef _EXTENSIONS_PROPCTRLR_STANDARDCONTROL_HXX_
#include "standardcontrol.hxx"
#endif

//............................................................................
namespace pcr
{
//............................................................................

#define FRAME_OFFSET 4
    // TODO: find out what this is really for ... and check if it does make sense in the new
    // browser environment

    //==================================================================
    //= OBrowserListBox
    //==================================================================
    DBG_NAME(OBrowserListBox)
    //------------------------------------------------------------------
    OBrowserListBox::OBrowserListBox( Window* pParent, WinBits nWinStyle)
            :Control(pParent, nWinStyle| WB_CLIPCHILDREN)
            ,m_aVScroll(this,WB_VSCROLL|WB_REPEAT|WB_DRAG)
            ,m_aPlayGround(this,WB_DIALOGCONTROL | WB_CLIPCHILDREN)
            ,m_pLineListener(NULL)
            ,m_bUpdate(sal_True)
            ,m_aStandard(ModuleRes(RID_STR_STANDARD))
            ,m_bIsActive(sal_False)
            ,m_nYOffset(0)
            ,m_nSelectedLine(0)
            ,m_nTheNameSize(0)
    {
        DBG_CTOR(OBrowserListBox,NULL);

        ListBox aListBox(this,WB_DROPDOWN);
        aListBox.SetPosSizePixel(Point(0,0),Size(100,100));
        m_nRowHeight = (sal_uInt16)aListBox.GetSizePixel().Height()+2;
        SetBackground(m_aPlayGround.GetBackground());

        m_aPlayGround.SetPosPixel(Point(0,0));
        m_aPlayGround.SetPaintTransparent(sal_True);
        m_aPlayGround.Show();
        m_aVScroll.Hide();
        m_aVScroll.SetScrollHdl(LINK(this, OBrowserListBox, ScrollHdl));

    }

    //------------------------------------------------------------------
    OBrowserListBox::~OBrowserListBox()
    {
        OSL_ENSURE( !IsModified(), "OBrowserListBox::~OBrowserListBox: still modified - should have been committed before!" );
            // doing the commit here, while we, as well as our owner, as well as some other components,
            // are already "half dead" (means within their dtor) is potentially dangerous.
            // By definition, CommitModified has to be called (if necessary) before destruction
            // #105868# - 2002-12-13 - fs@openoffice.org

        Hide();
        Clear();

        DBG_DTOR(OBrowserListBox,NULL);
    }

    //------------------------------------------------------------------
    sal_Bool OBrowserListBox::IsModified( ) const
    {
        sal_Bool bModified = sal_False;

        if ( m_bIsActive && ( m_aLines.size() > m_nSelectedLine ) )
        {   // we have an active line at least
            IBrowserControl* pControl = m_aLines[ m_nSelectedLine ]->getControl();
            // and the control in this line is modified
            bModified = ( NULL != pControl ) && ( pControl->IsModified() );
        }
        return bModified;
    }

    //------------------------------------------------------------------
    void OBrowserListBox::CommitModified( )
    {
        if ( IsModified() )
        {
            if ( m_aLines.size() > m_nSelectedLine )
            {
                IBrowserControl* pControl = m_aLines[ m_nSelectedLine ]->getControl();
                pControl->CommitModified();
            }
        }
    }

    //------------------------------------------------------------------
    void OBrowserListBox::Activate(sal_Bool _bActive)
    {
        m_bIsActive = _bActive;
        if (m_bIsActive)
        {
            // TODO: what's the sense of this?
            m_aVScroll.SetThumbPos(100);
            MoveThumbTo(0);
            Resize();
        }
    }

    //------------------------------------------------------------------
    void OBrowserListBox::Resize()
    {
        Size aSize(GetOutputSizePixel());
        m_aPlayGround.SetSizePixel(aSize);

        UpdateVScroll();

        Size a2Size(aSize);
        Size aVScrollSize;

        sal_Int32 nThumbPos = m_aVScroll.GetThumbPos();
        sal_Int32 nLines = CalcVisibleLines();

        sal_Bool bNeedScrollbar = m_aLines.size() > (sal_uInt32)nLines;
        if (!bNeedScrollbar)
        {   // don't need a scrollbar
            if (m_aVScroll.IsVisible())
                m_aVScroll.Hide();
            nThumbPos=0;
            m_nYOffset=0;
            m_aPlayGround.SetSizePixel(aSize);
            m_aVScroll.SetThumbPos(nThumbPos);
        }
        else
        {   // need a scrollbar -> adjust the playground
            Point aPos(0,0);
            aVScrollSize = m_aVScroll.GetSizePixel();
            aVScrollSize.Height() = aSize.Height();
            a2Size.Width() -= aVScrollSize.Width();
            aPos.X() = a2Size.Width();
            m_aVScroll.SetPosPixel(aPos);
            m_aVScroll.SetSizePixel(aVScrollSize);
            m_aPlayGround.SetSizePixel(a2Size);
        }

        for (sal_uInt16 i=0; i<m_aLines.size(); ++i)
            m_aLines[i]->SetNeedsRepaint(sal_True);

        // repaint
        EnablePaint(sal_False);
        UpdatePlayGround();
        EnablePaint(sal_True);

        // show the scrollbar
        if (bNeedScrollbar)
            m_aVScroll.Show();
    }

    //------------------------------------------------------------------
    void OBrowserListBox::setListener(IPropertyLineListener* _pPLL)
    {
        m_pLineListener = _pPLL;
    }

    //------------------------------------------------------------------
    sal_uInt16 OBrowserListBox::CalcVisibleLines()
    {
        Size aSize(m_aPlayGround.GetOutputSizePixel());
        sal_uInt16 nResult = 0;
        if (0 != m_nRowHeight)
            nResult = (sal_uInt16) aSize.Height()/m_nRowHeight;

        return nResult;
    }

    //------------------------------------------------------------------
    void OBrowserListBox::UpdateVScroll()
    {
        sal_uInt16 nLines = CalcVisibleLines();
        m_aVScroll.SetPageSize(nLines-1);
        m_aVScroll.SetVisibleSize(nLines-1);

        sal_uInt16 nCount = m_aLines.size();
        sal_Int32 nThumbPos = m_aVScroll.GetThumbPos();
        if (nCount>0)
        {
            m_aVScroll.SetRange(Range(0,nCount-1));
            m_nYOffset = -m_aVScroll.GetThumbPos()*m_nRowHeight;
        }
        else
        {
            m_aVScroll.SetRange(Range(0,0));
            m_nYOffset = 0;
        }
    }

    //------------------------------------------------------------------
    void OBrowserListBox::ShowLine(sal_uInt16 i)
    {
        Size aSize(m_aPlayGround.GetOutputSizePixel());
        Point aPos(0, m_nYOffset);

        aSize.Height() = m_nRowHeight;

        aPos.Y() += i * m_nRowHeight;

        if (i<m_aLines.size())
        {
            // adjust the pos and - if needed - the size
            if (m_aLines[i]->GetSizePixel() != aSize)
                m_aLines[i]->SetPosSizePixel(aPos, aSize);
            else
                m_aLines[i]->SetPosPixel(aPos);

            m_aLines[i]->SetTitleWidth(m_nTheNameSize + 2 * FRAME_OFFSET);

            // show the line if necessary
            if (!m_aLines[i]->IsVisible())
                m_aLines[i]->Show();
        }

    }

    //------------------------------------------------------------------
    void OBrowserListBox::UpdatePosNSize()
    {
        sal_uInt16 i;

        for (i=0; i<m_aLines.size(); ++i)
        {
            if (m_aLines[i]->NeedsRepaint())
                ShowLine(i);
        }
        for (i=0; i<m_aLines.size(); ++i)
            m_aLines[i]->SetNeedsRepaint(sal_False);

    }

    //------------------------------------------------------------------
    void OBrowserListBox::UpdatePlayGround()
    {
        sal_Int32 nThumbPos = m_aVScroll.GetThumbPos();
        sal_Int32 nLines = CalcVisibleLines();

        sal_uInt16 nEnd = (sal_uInt16)(nThumbPos + nLines);
        if (nEnd >= m_aLines.size())
            nEnd = m_aLines.size()-1;

        if (m_aLines.size()>0)
        {
            for (sal_uInt16 i = (sal_uInt16)nThumbPos; i<=nEnd; ++i)
                m_aLines[i]->SetNeedsRepaint(sal_True);
            UpdatePosNSize();
        }
    }

    //------------------------------------------------------------------
    void OBrowserListBox::UpdateAll()
    {
        Resize();
    }

    //------------------------------------------------------------------
    void OBrowserListBox::DisableUpdate()
    {
        m_bUpdate = sal_False;
    }

    //------------------------------------------------------------------
    void OBrowserListBox::EnableUpdate()
    {
        m_bUpdate = sal_True;
        UpdateAll();
    }

    //------------------------------------------------------------------
    void OBrowserListBox::SetPropertyValue(const ::rtl::OUString& _rEntryName, const ::rtl::OUString& _rValue)
    {
        // TODO: why not an O(log n) search? would need a map for this ...
        sal_uInt16 i, nEnd = m_aLines.size();
        for (i = 0 ; i<nEnd ; ++i)
        {
            OBrowserLine* pLine = m_aLines[i];
            IBrowserControl* pControl = pLine->getControl();
            if (pControl && (pControl->GetMyName() == _rEntryName))
            {
                pControl->SetProperty(_rValue);
                break;
            }
        }
    }

    //------------------------------------------------------------------------
    ::rtl::OUString OBrowserListBox::GetPropertyValue(const ::rtl::OUString& _rEntryName)   const
    {
        // TODO: O(log n) search
        sal_uInt16 i, nEnd = m_aLines.size();
        for (i=0; i<nEnd; ++i)
        {
            OBrowserLine* pLine = m_aLines[i];
            IBrowserControl* pControl = pLine->getControl();
            if (pControl && (pControl->GetMyName() == _rEntryName))
                return pControl->GetProperty();
        }

        return ::rtl::OUString();
    }

    //------------------------------------------------------------------------
    sal_uInt16 OBrowserListBox::GetPropertyPos(const ::rtl::OUString& _rEntryName) const
    {
        // TODO: O(log n) search
        sal_uInt16 i, nEnd = m_aLines.size();
        sal_uInt16 nRet = LISTBOX_ENTRY_NOTFOUND;
        for (i=0; i<nEnd; ++i)
        {
            OBrowserLine* pLine = m_aLines[i];
            IBrowserControl* pControl = pLine->getControl();
            if (pControl && (pControl->GetMyName() == _rEntryName))
            {
                nRet = i;
                break;
            }
        }

        return nRet;
    }

    //------------------------------------------------------------------------
    void OBrowserListBox::EnablePropertyLine( const ::rtl::OUString& _rEntryName, bool _bEnable )
    {
        // TODO: O(log n) search
        sal_uInt16 i, nEnd = m_aLines.size();
        for ( i = 0 ; i<nEnd ; ++i )
        {
            OBrowserLine* pLine = m_aLines[i];
            IBrowserControl* pControl = pLine->getControl();
            if ( pControl && ( pControl->GetMyName() == _rEntryName ) )
            {
                pLine->Enable( _bEnable );
                break;
            }
        }
    }

    //------------------------------------------------------------------------
    IBrowserControl* OBrowserListBox::GetPropertyControl(const ::rtl::OUString& _rEntryName)
    {
        // TODO: O(log n) search
        sal_uInt16 i, nEnd = m_aLines.size();
        for (i = 0 ; i<nEnd ; ++i)
        {
            OBrowserLine* pLine = m_aLines[i];
            IBrowserControl* pControl = pLine->getControl();
            if (pControl && (pControl->GetMyName() == _rEntryName))
                return pControl;
        }
        return NULL;
    }

    //------------------------------------------------------------------------
    IBrowserControl* OBrowserListBox::GetCurrentPropertyControl()
    {
        sal_uInt16 i, nEnd = m_aLines.size();
        for (i=0; i<nEnd; ++i)
        {
            OBrowserLine* pLine = m_aLines[i];
            IBrowserControl* pControl = pLine ? pLine->getControl() : NULL;
            Window* pControlWindow = pControl ? pControl->GetMe() : NULL;
            if (pControlWindow && pControlWindow->HasChildPathFocus())
                return pControl;
        }
        return NULL;
    }

    //------------------------------------------------------------------------
    void OBrowserListBox::SetPropertyData(const ::rtl::OUString& _rEntryName, void* pData)
    {
        // TODO: O(log n) search
        sal_uInt16 i, nEnd = m_aLines.size();
        for (i = 0 ; i<nEnd ; ++i)
        {
            OBrowserLine* pLine = m_aLines[i];
            IBrowserControl* pControl = pLine->getControl();
            if (pControl && (pControl->GetMyName() == _rEntryName))
            {
                pControl->SetMyData(pData);
                break;
            }
        }
    }

    //------------------------------------------------------------------------
    sal_uInt16 OBrowserListBox::AppendEntry(const OLineDescriptor& _rPropData)
    {
        return InsertEntry(_rPropData);
    }

    //------------------------------------------------------------------
    sal_uInt16 OBrowserListBox::InsertEntry(const OLineDescriptor& _rPropertyData, sal_uInt16 _nPos)
    {
        // create a new line
        OBrowserLine* pBrowserLine = new OBrowserLine(&m_aPlayGround);

        sal_uInt16 nInsertPos = _nPos;

        if (EDITOR_LIST_APPEND == nInsertPos)
        {
            nInsertPos = m_aLines.size();
            m_aLines.push_back(pBrowserLine);
        }
        else
            m_aLines.insert(m_aLines.begin() + nInsertPos, pBrowserLine);

        pBrowserLine->SetTitleWidth(m_nTheNameSize);
        if (m_bUpdate)
        {
            UpdateVScroll();
            Invalidate();
        }

        // initialize the entry
        ChangeEntry(_rPropertyData, nInsertPos);
        return nInsertPos;
    }

    // #95343# ---------------------------------------------------------
    sal_Int32 OBrowserListBox::GetMinimumWidth()
    {
        return m_nTheNameSize + 2 * FRAME_OFFSET + (m_nRowHeight - 4) * 10;
    }

    //------------------------------------------------------------------
    sal_uInt16 OBrowserListBox::GetFirstVisibleEntry()
    {
        return (sal_uInt16)m_aVScroll.GetThumbPos();
    }

    //------------------------------------------------------------------
    void OBrowserListBox::SetFirstVisibleEntry(sal_uInt16 _nPos)
    {
        if (_nPos<m_aLines.size())
            MoveThumbTo(_nPos);
    }

    //------------------------------------------------------------------
    void OBrowserListBox::SetSelectedEntry(sal_uInt16 _nPos)
    {
        if (_nPos<m_aLines.size())
        {
            m_nSelectedLine=_nPos;
            ShowEntry(_nPos);
            m_aLines[_nPos]->GrabFocus();
        }
    }

    //------------------------------------------------------------------
    sal_uInt16 OBrowserListBox::GetSelectedEntry()
    {
        return m_nSelectedLine;
    }

    //------------------------------------------------------------------
    void OBrowserListBox::ShowEntry(sal_uInt16 _nPos)
    {
        if (_nPos<m_aLines.size())
        {
            sal_Int32 nThumbPos = m_aVScroll.GetThumbPos();

            if (_nPos < nThumbPos)
                MoveThumbTo(_nPos);
            else
            {
                sal_Int32 nLines = CalcVisibleLines();
                if (_nPos >= nThumbPos + nLines)
                    MoveThumbTo(_nPos - nLines + 1);
            }
        }

    }

    //------------------------------------------------------------------
    void OBrowserListBox::MoveThumbTo(sal_Int32 _nNewThumbPos)
    {
        // disable painting to prevent flicker
        m_aPlayGround.EnablePaint(sal_False);

        sal_Int32 nDelta = _nNewThumbPos - m_aVScroll.GetThumbPos();
        // adjust the scrollbar
        m_aVScroll.SetThumbPos(_nNewThumbPos);
        sal_Int32 nThumbPos = nThumbPos = _nNewThumbPos;

        m_nYOffset = -m_aVScroll.GetThumbPos() * m_nRowHeight;

        sal_Int32 nLines = CalcVisibleLines();
        sal_uInt16 nEnd = (sal_uInt16)(nThumbPos + nLines);

        m_aPlayGround.Scroll(0, -nDelta * m_nRowHeight, SCROLL_CHILDREN);

        if (1 == nDelta)
        {
            // TODO: what's the sense of this two ShowLines? Why not just one call?
            ShowLine(nEnd-1);
            ShowLine(nEnd);
        }
        else if (-1 == nDelta)
        {
            ShowLine((sal_uInt16)nThumbPos);
        }
        else if (0 != nDelta)
        {
            UpdatePlayGround();
        }

        m_aPlayGround.EnablePaint(sal_True);
        m_aPlayGround.Invalidate(INVALIDATE_CHILDREN);
    }

    //------------------------------------------------------------------
    IMPL_LINK(OBrowserListBox, ScrollHdl, ScrollBar*, _pScrollBar )
    {
        DBG_ASSERT(_pScrollBar == &m_aVScroll, "OBrowserListBox::ScrollHdl: where does this come from?");

        // disable painting to prevent flicker
        m_aPlayGround.EnablePaint(sal_False);

        sal_Int32 nThumbPos = m_aVScroll.GetThumbPos();

        sal_Int32 nDelta = m_aVScroll.GetDelta();
        m_nYOffset = -nThumbPos * m_nRowHeight;

        sal_uInt16 nEnd = (sal_uInt16)(nThumbPos + CalcVisibleLines());

        m_aPlayGround.Scroll(0, -nDelta * m_nRowHeight, SCROLL_CHILDREN);

        if (1 == nDelta)
        {
            ShowLine(nEnd-1);
            ShowLine(nEnd);
        }
        else if (nDelta==-1)
        {
            ShowLine((sal_uInt16)nThumbPos);
        }
        else if (nDelta!=0 || m_aVScroll.GetType() == SCROLL_DONTKNOW)
        {
            UpdatePlayGround();
        }

        m_aPlayGround.EnablePaint(sal_True);
        return 0;
    }

    //------------------------------------------------------------------
    IMPL_LINK( OBrowserListBox, ClickHdl, PushButton*,pPB)
    {
        DBG_ASSERT(pPB, "OBrowserListBox::ClickHdl: invalid button!");
        if (pPB)
        {
            // notify our LineListener of the ClickecEvent
            if (m_pLineListener)
            {
                OBrowserLine* pBrowserLine = static_cast<OBrowserLine*>(pPB->GetData());
                IBrowserControl* pControl = pBrowserLine->getControl();
                m_pLineListener->Clicked(pControl->GetMyName(), pControl->GetProperty(), pControl->GetMyData());
            }
            pPB->GrabFocus();
        }
        return 0;
    }

    //------------------------------------------------------------------
    void OBrowserListBox::Modified(IBrowserControl* _pControl)
    {
        DBG_ASSERT(_pControl, "OBrowserListBox::Modified: invalid event source!");
        // notify our line listener
        if (_pControl && m_pLineListener)
        {
            m_pLineListener->Modified(
                _pControl->GetMyName(),
                _pControl->GetProperty(),
                _pControl->GetMyData()
            );

        }
    }

    //------------------------------------------------------------------
    void OBrowserListBox::GetFocus(IBrowserControl* _pControl)
    {
        DBG_ASSERT(_pControl, "OBrowserListBox::GetFocus: invalid event source!");
        if (!_pControl)
            return;

        // notify our line listener
        if (m_pLineListener)
        {
            m_pLineListener->Select(
                _pControl->GetMyName(),
                _pControl->GetMyData()
            );
        }

        m_nSelectedLine = _pControl->GetLine();
        ShowEntry(m_nSelectedLine);
    }

    //------------------------------------------------------------------
    void OBrowserListBox::Commit(IBrowserControl* _pControl)
    {
        DBG_ASSERT(_pControl, "OBrowserListBox::Commit: invalid event source!");
        if (!_pControl)
            return;

        // upon losing the focus, we notify our listener so that it can commit the content
        if (m_pLineListener)
        {
            m_pLineListener->Commit(
                _pControl->GetMyName(),
                _pControl->GetProperty(),
                _pControl->GetMyData()
            );
        }
    }

    //------------------------------------------------------------------
    void OBrowserListBox::TravelLine(IBrowserControl* _pControl)
    {
        sal_Int32 nLine = _pControl->GetLine();

        if (_pControl->GetDirection())
        {
            // cycle forwards, 'til we've the next control which can grab the focus
            ++nLine;
            while ((sal_uInt32)nLine<m_aLines.size())
            {
                if (m_aLines[nLine]->GrabFocus())
                    break;
                ++nLine;
            }

            if  (   ((sal_uInt32)nLine >= m_aLines.size())
                &&  (m_aLines.size() > 0)
                )
                // wrap around
                m_aLines[0]->GrabFocus();
        }
        else
        {   // cycle backwards, 'til we've the next control which can grab the focus
            --nLine;
            while (nLine>=0)
            {
                if (m_aLines[nLine]->GrabFocus())
                    break;
                --nLine;
            }

            if  (   (nLine < 0)
                &&  (0 < m_aLines.size())
                )
                // wrap around
                m_aLines[m_aLines.size() - 1]->GrabFocus();
        }
    }

    //------------------------------------------------------------------
    void OBrowserListBox::KeyInput(IBrowserControl* _pControl, const KeyCode& _rKey)
    {
        DBG_ASSERT(_pControl, "OBrowserListBox::KeyInput: invalid event source!");
        if (!_pControl)
            return;

        // not really interested in ...
    }

    //------------------------------------------------------------------
    void OBrowserListBox::Clear()
    {
        for (   OBrowserLinesArrayIterator aLoop = m_aLines.begin();
                aLoop != m_aLines.end();
                ++aLoop
            )
        {
            // hide the line
            (*aLoop)->Hide();
            // reset the listener
            (*aLoop)->getControl()->setListener(NULL);
            // delete the control
            delete (*aLoop)->getControl();
            // delete the line
            delete *aLoop;
        }

        m_aLines.clear();
    }

    //------------------------------------------------------------------
    void OBrowserListBox::ChangeEntry(const OLineDescriptor& _rPropertyData, sal_uInt16 nPos)
    {
        sal_Bool bNew=sal_True;
        if (nPos < m_aLines.size())
        {
            Window* pRefWindow=NULL;
            if (nPos>0)
                pRefWindow = m_aLines[nPos-1]->GetRefWindow();

            // the current line and control
            OBrowserLine* pBrowserLine = m_aLines[nPos];
            IBrowserControl* pControl = pBrowserLine->getControl();

            // position of the control
            Point aControlPos;
            if (pControl)
                aControlPos=pControl->GetCtrPos();

            if (pBrowserLine->GetKindOfControl() != _rPropertyData.eControlType)
            {
                // need to create a new control

                bNew = sal_True;
                delete pBrowserLine->getControl();

                // default winbits: a border only
                WinBits nWinBits=WB_BORDER;
                if (_rPropertyData.bIsLocked)
                    nWinBits|=WB_READONLY;

                pBrowserLine->SetKindOfControl(_rPropertyData.eControlType);    // may be overwritten

                // create the new control
                IBrowserControl* pNewControl = NULL;
                // TODO: source this out (into a factory)
                switch(_rPropertyData.eControlType)
                {
                    case BCT_LEDIT:
                        pNewControl = new OMultilineEditControl(&m_aPlayGround,sal_False, nWinBits | WB_DROPDOWN | WB_TABSTOP);
                        pBrowserLine->SetKindOfControl(BCT_MEDIT);
                        break;

                    case BCT_MEDIT:
                        pNewControl = new OMultilineEditControl(&m_aPlayGround,sal_True, nWinBits | WB_DROPDOWN | WB_TABSTOP);
                        break;

                    case BCT_LISTBOX:
                        pNewControl = new OListboxControl(&m_aPlayGround, nWinBits | WB_TABSTOP | WB_DROPDOWN);
                        break;

                    case BCT_COMBOBOX:
                        pNewControl = new OComboboxControl(&m_aPlayGround, nWinBits | WB_TABSTOP | WB_SORT | WB_DROPDOWN);
                        break;

                    case BCT_EDIT:
                        pNewControl = new OEditControl(&m_aPlayGround,sal_False,nWinBits | WB_TABSTOP);
                        break;

                    case BCT_CHAREDIT:
                        pNewControl = new OEditControl(&m_aPlayGround,sal_True,nWinBits | WB_TABSTOP);
                        pBrowserLine->SetKindOfControl(BCT_EDIT);
                        break;

                    case BCT_NUMFIELD:
                        {
                            ONumericControl* pField = new ONumericControl(&m_aPlayGround, _rPropertyData.nDigits,
                                                            nWinBits | WB_TABSTOP | WB_SPIN);
                            if (_rPropertyData.bHaveMinMax)
                            {
                                pField->SetMin(_rPropertyData.nMinValue);
                                pField->SetMax(_rPropertyData.nMaxValue);
                            }
                            pNewControl = pField;
                        }
                        break;

                    case BCT_CURFIELD:
                        pNewControl = new OCurrencyControl(&m_aPlayGround,_rPropertyData.nDigits, nWinBits | WB_TABSTOP | WB_SPIN);
                        break;

                    case BCT_DATEFIELD:
                        pNewControl = new ODateControl(&m_aPlayGround,nWinBits | WB_TABSTOP | WB_SPIN);
                        break;

                    case BCT_TIMEFIELD:
                        pNewControl = new OTimeControl(&m_aPlayGround,nWinBits | WB_TABSTOP | WB_SPIN);
                        break;

                    case BCT_COLORBOX:
                        pNewControl = new OColorControl(&m_aPlayGround, nWinBits | WB_TABSTOP | WB_DROPDOWN);
                        break;

                    case BCT_USERDEFINED:
                        DBG_ASSERT(_rPropertyData.pControl, "OBrowserListBox::ChangeEntry: invalid user defined control!");
                        pNewControl = _rPropertyData.pControl;
                        pNewControl->SetCtrParent(&m_aPlayGround);
                        break;

                    default:
                        pNewControl = NULL;
                        pBrowserLine->SetKindOfControl(BCT_UNDEFINED);
                        break;
                }

                pBrowserLine->setControl(pNewControl);
                pNewControl->SetCtrPos(aControlPos);
                // forward the locking flag to the control
                pNewControl->SetLocked(_rPropertyData.bIsLocked);

                // remember this new control
                pControl = pNewControl;
            }
            else
            {
                bNew = sal_False;
                if (pControl && pControl->HasList())
                    pControl->ClearList();
            }

            if (pControl)
            {
                pControl->setListener(this);
                // forward the data from the descriptor
                pControl->SetMyData(_rPropertyData.pDataPtr);
                pControl->SetMyName(_rPropertyData.sName);
                pControl->SetLine(nPos);

                if (pControl->HasList())
                {
                    // the standard entry
                    if (_rPropertyData.bHasDefaultValue && (BCT_COLORBOX != _rPropertyData.eControlType))
                        pControl->InsertCtrEntry(m_aStandard);

                    // all other entries from the descriptor
                    for (sal_uInt32 i=0;i<_rPropertyData.aListValues.size(); ++i)
                        pControl->InsertCtrEntry(_rPropertyData.aListValues[i]);
                }

                // the initial property value
                pControl->SetProperty(_rPropertyData.sValue, _rPropertyData.bUnknownValue);
            }

            pBrowserLine->SetTitle(_rPropertyData.sTitle);

            sal_uInt16 nTextWidth = (sal_uInt16)m_aPlayGround.GetTextWidth(_rPropertyData.sTitle);
            if (m_nTheNameSize< nTextWidth)
                m_nTheNameSize = nTextWidth;

            if (_rPropertyData.bHasBrowseButton)
            {
                pBrowserLine->ShowXButton();
                pBrowserLine->SetClickHdl(LINK( this, OBrowserListBox, ClickHdl ) );
            }
            else
                pBrowserLine->HideXButton();

            pBrowserLine->Locked(_rPropertyData.bIsLocked);

            pBrowserLine->ShowAsHyperLink(_rPropertyData.bIsHyperlink);
            pBrowserLine->SetData(_rPropertyData.pDataPtr);

            if (bNew)
            {
                if (nPos>0)
                    pBrowserLine->SetTabOrder(pRefWindow, WINDOW_ZORDER_BEHIND);
                else
                    pBrowserLine->SetTabOrder(pRefWindow, WINDOW_ZORDER_FIRST );

                pBrowserLine->SetNeedsRepaint(sal_True);
                pBrowserLine->SetHelpId(_rPropertyData.nHelpId,_rPropertyData.nUniqueButtonId);
            }
        }
    }

    //------------------------------------------------------------------
    long OBrowserListBox::Notify( NotifyEvent& _rNEvt )
    {
        // interested in scroll events if we have a scrollbar
        if ( m_aVScroll.IsVisible() )
        {
            if ( EVENT_COMMAND == _rNEvt.GetType() )
            {
                const CommandEvent* pCommand = _rNEvt.GetCommandEvent();
                if  (   ( COMMAND_WHEEL == pCommand->GetCommand() )
                    ||  ( COMMAND_STARTAUTOSCROLL == pCommand->GetCommand() )
                    ||  ( COMMAND_AUTOSCROLL == pCommand->GetCommand() )
                    )
                {
                    HandleScrollCommand( *pCommand, NULL, &m_aVScroll );
                }
            }
        }
        return Control::Notify( _rNEvt );
    }

//............................................................................
} // namespace pcr
//............................................................................


