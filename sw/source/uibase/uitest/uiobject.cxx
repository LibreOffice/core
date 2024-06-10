/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <uiobject.hxx>
#include <edtwin.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <ndtxt.hxx>
#include <viewopt.hxx>
#include <sfx2/sidebar/Sidebar.hxx>
#include <sfx2/viewfrm.hxx>

#include <AnnotationWin.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editview.hxx>

SwEditWinUIObject::SwEditWinUIObject(const VclPtr<SwEditWin>& xEditWin):
    WindowUIObject(xEditWin),
    mxEditWin(xEditWin)
{
}

namespace {

SwWrtShell& getWrtShell(VclPtr<SwEditWin> const & xEditWin)
{
    return xEditWin->GetView().GetWrtShell();
}

}

StringMap SwEditWinUIObject::get_state()
{
    StringMap aMap = WindowUIObject::get_state();

    aMap[u"SelectedText"_ustr] = mxEditWin->GetView().GetSelectionText();

    sal_uInt16 nPageNum = 0;
    sal_uInt16 nVirtPageNum = 0;
    SwWrtShell& rWrtShell = getWrtShell(mxEditWin);
    rWrtShell.GetPageNum(nPageNum, nVirtPageNum);
    aMap[u"CurrentPage"_ustr] = OUString::number(nPageNum);
    rWrtShell.GetPageNum(nPageNum, nVirtPageNum, false);
    aMap[u"TopVisiblePage"_ustr] = OUString::number(nPageNum);
    aMap[u"Zoom"_ustr] = OUString::number(rWrtShell.GetViewOptions()->GetZoom());

    sal_uInt16 nPages = rWrtShell.GetPageCnt();
    aMap[u"Pages"_ustr] = OUString::number(nPages);

    aMap[u"StartWord"_ustr] = OUString::boolean(rWrtShell.IsStartWord());
    aMap[u"EndWord"_ustr] = OUString::boolean(rWrtShell.IsEndWord());
    aMap[u"StartSentence"_ustr] = OUString::boolean(rWrtShell.IsStartSentence());
    aMap[u"EndSentence"_ustr] = OUString::boolean(rWrtShell.IsEndSentence());
    aMap[u"StartPara"_ustr] = OUString::boolean(rWrtShell.IsSttPara());
    aMap[u"EndPara"_ustr] = OUString::boolean(rWrtShell.IsEndPara());
    aMap[u"StartDoc"_ustr] = OUString::boolean(rWrtShell.IsStartOfDoc());
    aMap[u"EndDoc"_ustr] = OUString::boolean(rWrtShell.IsEndOfDoc());

    return aMap;
}

void SwEditWinUIObject::execute(const OUString& rAction,
        const StringMap& rParameters)
{
    if (rAction == "SET")
    {
        if (rParameters.find(u"ZOOM"_ustr) != rParameters.end())
        {
            auto itr = rParameters.find(u"ZOOM"_ustr);
            OUString aVal = itr->second;
            sal_Int32 nVal = aVal.toInt32();
            mxEditWin->GetView().SetZoom(SvxZoomType::PERCENT, nVal);
        }
    }
    else if (rAction == "GOTO")
    {
        if (rParameters.find(u"PAGE"_ustr) != rParameters.end())
        {
            auto itr = rParameters.find(u"PAGE"_ustr);
            OUString aVal = itr->second;
            sal_Int32 nVal = aVal.toInt32();
            getWrtShell(mxEditWin).GotoPage(nVal, false);
        }
    }
    else if (rAction == "SELECT")
    {
        if (rParameters.find(u"START_POS"_ustr) != rParameters.end())
        {
            auto itr = rParameters.find(u"START_POS"_ustr);
            OUString aStartPos = itr->second;
            TextFrameIndex const nStartPos(aStartPos.toInt32());

            itr = rParameters.find(u"END_POS"_ustr);
            assert(itr != rParameters.end());
            OUString aEndPos = itr->second;
            TextFrameIndex const nEndPos(aEndPos.toInt32());

            auto & shell = getWrtShell(mxEditWin);
            if (shell.GetCursor_()->GetPoint()->GetNode().GetTextNode())
            {
                shell.Push();
                shell.MovePara(GoCurrPara, fnParaEnd);
                TextFrameIndex const len(shell.GetCursorPointAsViewIndex());
                shell.Pop(SwCursorShell::PopMode::DeleteCurrent);
                SAL_WARN_IF(
                    sal_Int32(nStartPos) < 0 || nStartPos > len || sal_Int32(nEndPos) < 0 || nEndPos > len, "sw.ui",
                    "SELECT START/END_POS " << sal_Int32(nStartPos) << ".." << sal_Int32(nEndPos) << " outside 0.." << sal_Int32(len));
                shell.SelectTextView(
                    std::clamp(nStartPos, TextFrameIndex(0), len), std::clamp(nEndPos, TextFrameIndex(0), len));
            }
            else
            {
                SAL_WARN("sw.ui", "SELECT without SwTextNode");
            }
        }
    }
    else if (rAction == "SIDEBAR")
    {
        SfxViewFrame* pViewFrm = SfxViewFrame::Current();
        assert(pViewFrm && "SwEditWinUIObject::execute: no viewframe");
        pViewFrm->ShowChildWindow(SID_SIDEBAR);

        if (rParameters.find(u"PANEL"_ustr) != rParameters.end())
        {
            auto itr = rParameters.find(u"PANEL"_ustr);
            OUString aVal = itr->second;
            ::sfx2::sidebar::Sidebar::ShowPanel(aVal, pViewFrm->GetFrame().GetFrameInterface());
        }
    }
    else
        WindowUIObject::execute(rAction, rParameters);
}

OUString SwEditWinUIObject::get_name() const
{
    return u"SwEditWinUIObject"_ustr;
}

std::unique_ptr<UIObject> SwEditWinUIObject::create(vcl::Window* pWindow)
{
    SwEditWin* pEditWin = dynamic_cast<SwEditWin*>(pWindow);
    assert(pEditWin);
    return std::unique_ptr<UIObject>(new SwEditWinUIObject(pEditWin));
}

CommentUIObject::CommentUIObject(const VclPtr<sw::annotation::SwAnnotationWin>& xCommentUIObject):
    WindowUIObject(xCommentUIObject),
    mxCommentUIObject(xCommentUIObject)
{
}

StringMap CommentUIObject::get_state()
{
    StringMap aMap = WindowUIObject::get_state();
    aMap[u"Author"_ustr] = mxCommentUIObject->GetAuthor();
    aMap[u"ReadOnly"_ustr] = OUString::boolean(mxCommentUIObject->IsReadOnly());
    aMap[u"Resolved"_ustr] = OUString::boolean(mxCommentUIObject->IsResolved());
    aMap[u"Visible"_ustr] = OUString::boolean(mxCommentUIObject->IsVisible());

    aMap[u"Text"_ustr] = mxCommentUIObject->GetOutliner()->GetEditEngine().GetText();
    aMap[u"SelectedText"_ustr] = mxCommentUIObject->GetOutlinerView()->GetEditView().GetSelected();
    return aMap;
}

void CommentUIObject::execute(const OUString& rAction,
        const StringMap& rParameters)
{
    if (rAction == "SELECT")
    {
        if (rParameters.find(u"FROM"_ustr) != rParameters.end() &&
                    rParameters.find(u"TO"_ustr) != rParameters.end())
        {
                tools::Long nMin = rParameters.find(u"FROM"_ustr)->second.toInt32();
                tools::Long nMax = rParameters.find(u"TO"_ustr)->second.toInt32();
                ESelection aNewSelection( 0 , nMin, mxCommentUIObject->GetOutliner()->GetParagraphCount()-1, nMax );
                mxCommentUIObject->GetOutlinerView()->SetSelection( aNewSelection );
        }
    }
    else if (rAction == "LEAVE")
    {
        mxCommentUIObject->SwitchToFieldPos();
    }
    else if (rAction == "HIDE")
    {
        mxCommentUIObject->HideNote();
    }
    else if (rAction == "SHOW")
    {
        mxCommentUIObject->ShowNote();
    }
    else if (rAction == "DELETE")
    {
        mxCommentUIObject->Delete();
    }
    else if (rAction == "RESOLVE")
    {
        mxCommentUIObject->SetResolved(true);
    }
    else
        WindowUIObject::execute(rAction, rParameters);
}

std::unique_ptr<UIObject> CommentUIObject::create(vcl::Window* pWindow)
{
    sw::annotation::SwAnnotationWin* pCommentUIObject = dynamic_cast<sw::annotation::SwAnnotationWin*>(pWindow);
    assert(pCommentUIObject);
    return std::unique_ptr<UIObject>(new CommentUIObject(pCommentUIObject));
}

OUString CommentUIObject::get_name() const
{
    return u"CommentUIObject"_ustr;
}

PageBreakUIObject::PageBreakUIObject(const VclPtr<SwBreakDashedLine>& xPageBreakUIObject):
    WindowUIObject(xPageBreakUIObject),
    mxPageBreakUIObject(xPageBreakUIObject)
{
}

void PageBreakUIObject::execute(const OUString& rAction,
        const StringMap& rParameters)
{
    if (rAction == "DELETE" || rAction == "EDIT")
        mxPageBreakUIObject->execute(rAction.toAsciiLowerCase());
    else
        WindowUIObject::execute(rAction, rParameters);
}

std::unique_ptr<UIObject> PageBreakUIObject::create(vcl::Window* pWindow)
{
    SwBreakDashedLine* pPageBreakWin = dynamic_cast<SwBreakDashedLine*>(pWindow);
    assert(pPageBreakWin);
    return std::unique_ptr<UIObject>(new PageBreakUIObject(pPageBreakWin));
}

OUString PageBreakUIObject::get_name() const
{
    return u"PageBreakUIObject"_ustr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
