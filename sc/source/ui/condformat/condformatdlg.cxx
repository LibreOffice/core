/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "condformatdlg.hxx"
#include "condformatdlg.hrc"

#include <vcl/vclevent.hxx>
#include <svl/style.hxx>
#include <sfx2/dispatch.hxx>
#include <svl/stritem.hxx>
#include <svl/intitem.hxx>
#include <svx/xtable.hxx>
#include <svx/drawitem.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/settings.hxx>
#include <vcl/builderfactory.hxx>
#include <libxml/tree.h>

#include "anyrefdg.hxx"
#include "document.hxx"
#include "conditio.hxx"
#include "stlpool.hxx"
#include "tabvwsh.hxx"
#include "colorscale.hxx"
#include "colorformat.hxx"
#include "reffact.hxx"
#include "docsh.hxx"
#include "docfunc.hxx"
#include "condformatdlgentry.hxx"

#include "globstr.hrc"

ScCondFormatList::ScCondFormatList(vcl::Window* pParent, WinBits nStyle)
    : Control(pParent, nStyle | WB_DIALOGCONTROL)
    , mbHasScrollBar(false)
    , mpScrollBar(VclPtr<ScrollBar>::Create(this, WB_VERT ))
    , mpDoc(NULL)
    , mpDialogParent(NULL)
{
    mpScrollBar->SetScrollHdl( LINK( this, ScCondFormatList, ScrollHdl ) );
    mpScrollBar->EnableDrag();
    SetControlBackground( GetSettings().GetStyleSettings().GetWindowColor() );
    SetBackground(GetControlBackground());
}

ScCondFormatList::~ScCondFormatList()
{
    disposeOnce();
}

void ScCondFormatList::dispose()
{
    mpDialogParent.clear();
    mpScrollBar.disposeAndClear();
    for (auto it = maEntries.begin(); it != maEntries.end(); ++it)
        it->disposeAndClear();
    maEntries.clear();
    Control::dispose();
}

void ScCondFormatList::init(ScDocument* pDoc, ScCondFormatDlg* pDialogParent,
        const ScConditionalFormat* pFormat, const ScRangeList& rRanges,
        const ScAddress& rPos, condformat::dialog::ScCondFormatDialogType eType)
{
    mpDialogParent = pDialogParent;
    mpDoc = pDoc;
    maPos = rPos;
    maRanges = rRanges;

    if(pFormat)
    {
        size_t nCount = pFormat->size();
        for (size_t nIndex = 0; nIndex < nCount; ++nIndex)
        {
            const ScFormatEntry* pEntry = pFormat->GetEntry(nIndex);
            switch(pEntry->GetType())
            {
                case condformat::CONDITION:
                    {
                        const ScCondFormatEntry* pConditionEntry = static_cast<const ScCondFormatEntry*>( pEntry );
                        if(pConditionEntry->GetOperation() != SC_COND_DIRECT)
                            maEntries.push_back(VclPtr<ScConditionFrmtEntry>::Create( this, mpDoc, pDialogParent, maPos, pConditionEntry ) );
                        else
                            maEntries.push_back(VclPtr<ScFormulaFrmtEntry>::Create( this, mpDoc, pDialogParent, maPos, pConditionEntry ) );

                    }
                    break;
                case condformat::COLORSCALE:
                    {
                        const ScColorScaleFormat* pColorScale = static_cast<const ScColorScaleFormat*>( pEntry );
                        if( pColorScale->size() == 2 )
                            maEntries.push_back(VclPtr<ScColorScale2FrmtEntry>::Create( this, mpDoc, maPos, pColorScale ) );
                        else
                            maEntries.push_back(VclPtr<ScColorScale3FrmtEntry>::Create( this, mpDoc, maPos, pColorScale ) );
                    }
                    break;
                case condformat::DATABAR:
                    maEntries.push_back(VclPtr<ScDataBarFrmtEntry>::Create( this, mpDoc, maPos, static_cast<const ScDataBarFormat*>( pEntry ) ) );
                    break;
                case condformat::ICONSET:
                    maEntries.push_back(VclPtr<ScIconSetFrmtEntry>::Create( this, mpDoc, maPos, static_cast<const ScIconSetFormat*>( pEntry ) ) );
                    break;
                case condformat::DATE:
                    maEntries.push_back(VclPtr<ScDateFrmtEntry>::Create( this, mpDoc, static_cast<const ScCondDateFormatEntry*>( pEntry ) ) );
                    break;
            }
        }
        if(nCount)
            EntrySelectHdl(maEntries[0].get());
    }
    else
    {
        switch(eType)
        {
            case condformat::dialog::CONDITION:
                maEntries.push_back(VclPtr<ScConditionFrmtEntry>::Create( this, mpDoc, pDialogParent, maPos ));
                break;
            case condformat::dialog::COLORSCALE:
                maEntries.push_back(VclPtr<ScColorScale3FrmtEntry>::Create( this, mpDoc, maPos ));
                break;
            case condformat::dialog::DATABAR:
                maEntries.push_back(VclPtr<ScDataBarFrmtEntry>::Create( this, mpDoc, maPos ));
                break;
            case condformat::dialog::ICONSET:
                maEntries.push_back(VclPtr<ScIconSetFrmtEntry>::Create( this, mpDoc, maPos ));
                break;
            case condformat::dialog::DATE:
                maEntries.push_back(VclPtr<ScDateFrmtEntry>::Create( this, mpDoc ));
                break;
            case condformat::dialog::NONE:
                break;
        }
    }
    RecalcAll();
    if (!maEntries.empty())
        (*maEntries.begin())->SetActive();

    RecalcAll();
}

VCL_BUILDER_DECL_FACTORY(ScCondFormatList)
{
    WinBits nWinBits = 0;

    OString sBorder = VclBuilder::extractCustomProperty(rMap);
    if (!sBorder.isEmpty())
       nWinBits |= WB_BORDER;

    rRet = VclPtr<ScCondFormatList>::Create(pParent, nWinBits);
}

Size ScCondFormatList::GetOptimalSize() const
{
    return LogicToPixel(Size(290, 185), MAP_APPFONT);
}

void ScCondFormatList::Resize()
{
    Control::Resize();
    RecalcAll();
}

ScConditionalFormat* ScCondFormatList::GetConditionalFormat() const
{
    if(maEntries.empty())
        return NULL;

    ScConditionalFormat* pFormat = new ScConditionalFormat(0, mpDoc);
    for(EntryContainer::const_iterator itr = maEntries.begin(); itr != maEntries.end(); ++itr)
    {
        ScFormatEntry* pEntry = (*itr)->GetEntry();
        if(pEntry)
            pFormat->AddEntry(pEntry);
    }

    pFormat->SetRange(maRanges);

    return pFormat;
}

void ScCondFormatList::RecalcAll()
{
    sal_Int32 nTotalHeight = 0;
    sal_Int32 nIndex = 1;
    for(EntryContainer::iterator itr = maEntries.begin(); itr != maEntries.end(); ++itr)
    {
        nTotalHeight += (*itr)->GetSizePixel().Height();
        (*itr)->SetIndex( nIndex );
        ++nIndex;
    }

    Size aCtrlSize = GetOutputSize();
    long nSrcBarSize = GetSettings().GetStyleSettings().GetScrollBarSize();
    if(nTotalHeight > GetSizePixel().Height())
    {
        mbHasScrollBar = true;
        mpScrollBar->SetPosSizePixel(Point(aCtrlSize.Width() -nSrcBarSize, 0),
                Size(nSrcBarSize, aCtrlSize.Height()) );
        mpScrollBar->SetRangeMax(nTotalHeight);
        mpScrollBar->SetVisibleSize(aCtrlSize.Height());
        mpScrollBar->Show();
    }
    else
    {
        mbHasScrollBar = false;
        mpScrollBar->Hide();
    }

    Point aPoint(0,-1*mpScrollBar->GetThumbPos());
    for(EntryContainer::iterator itr = maEntries.begin(); itr != maEntries.end(); ++itr)
    {
        (*itr)->SetPosPixel(aPoint);
        Size aSize = (*itr)->GetSizePixel();
        if(mbHasScrollBar)
            aSize.Width() = aCtrlSize.Width() - nSrcBarSize;
        else
            aSize.Width() = aCtrlSize.Width();
        (*itr)->SetSizePixel(aSize);

        aPoint.Y() += (*itr)->GetSizePixel().Height();
    }
}

void ScCondFormatList::DoScroll(long nDelta)
{
    Point aNewPoint = mpScrollBar->GetPosPixel();
    Rectangle aRect(Point(), GetOutputSize());
    aRect.Right() -= mpScrollBar->GetSizePixel().Width();
    Scroll( 0, -nDelta, aRect );
    mpScrollBar->SetPosPixel(aNewPoint);
}

IMPL_LINK(ScCondFormatList, ColFormatTypeHdl, ListBox*, pBox)
{
    EntryContainer::iterator itr = maEntries.begin();
    for(; itr != maEntries.end(); ++itr)
    {
        if((*itr)->IsSelected())
            break;
    }
    if(itr == maEntries.end())
        return 0;

    sal_Int32 nPos = pBox->GetSelectEntryPos();
    switch(nPos)
    {
        case 0:
            if((*itr)->GetType() == condformat::entry::COLORSCALE2)
                return 0;

            itr->disposeAndClear();
            *itr = VclPtr<ScColorScale2FrmtEntry>::Create( this, mpDoc, maPos );
            break;
        case 1:
            if((*itr)->GetType() == condformat::entry::COLORSCALE3)
                return 0;

            itr->disposeAndClear();
            *itr = VclPtr<ScColorScale3FrmtEntry>::Create( this, mpDoc, maPos );
            break;
        case 2:
            if((*itr)->GetType() == condformat::entry::DATABAR)
                return 0;

            itr->disposeAndClear();
            *itr = VclPtr<ScDataBarFrmtEntry>::Create( this, mpDoc, maPos );
            break;
        case 3:
            if((*itr)->GetType() == condformat::entry::ICONSET)
                return 0;

            itr->disposeAndClear();
            *itr = VclPtr<ScIconSetFrmtEntry>::Create( this, mpDoc, maPos );
            break;
        default:
            break;
    }
    mpDialogParent->InvalidateRefData();
    (*itr)->SetActive();
    RecalcAll();
    return 0;
}

IMPL_LINK(ScCondFormatList, TypeListHdl, ListBox*, pBox)
{
    //Resolves: fdo#79021 At this point we are still inside the ListBox Select.
    //If we call maEntries.replace here then the pBox will be deleted before it
    //has finished Select and will crash on accessing its deleted this. So Post
    //to do the real work after the Select has completed
    Application::PostUserEvent(LINK(this, ScCondFormatList, AfterTypeListHdl), pBox, true);
    return 0;
}

IMPL_LINK_TYPED(ScCondFormatList, AfterTypeListHdl, void*, p, void)
{
    ListBox* pBox = static_cast<ListBox*>(p);
    EntryContainer::iterator itr = maEntries.begin();
    for(; itr != maEntries.end(); ++itr)
    {
        if((*itr)->IsSelected())
            break;
    }
    if(itr == maEntries.end())
        return;

    sal_Int32 nPos = pBox->GetSelectEntryPos();
    switch(nPos)
    {
        case 0:
            switch((*itr)->GetType())
            {
                case condformat::entry::FORMULA:
                case condformat::entry::CONDITION:
                case condformat::entry::DATE:
                    break;
                case condformat::entry::COLORSCALE2:
                case condformat::entry::COLORSCALE3:
                case condformat::entry::DATABAR:
                case condformat::entry::ICONSET:
                    return;
            }
            itr->disposeAndClear();
            *itr = VclPtr<ScColorScale3FrmtEntry>::Create(this, mpDoc, maPos);
            mpDialogParent->InvalidateRefData();
            (*itr)->SetActive();
            break;
        case 1:
            if((*itr)->GetType() == condformat::entry::CONDITION)
                return;

            itr->disposeAndClear();
            *itr = VclPtr<ScConditionFrmtEntry>::Create(this, mpDoc, mpDialogParent, maPos);
            mpDialogParent->InvalidateRefData();
            (*itr)->SetActive();
            break;
        case 2:
            if((*itr)->GetType() == condformat::entry::FORMULA)
                return;

            itr->disposeAndClear();
            *itr = VclPtr<ScFormulaFrmtEntry>::Create(this, mpDoc, mpDialogParent, maPos);
            mpDialogParent->InvalidateRefData();
            (*itr)->SetActive();
            break;
        case 3:
            if((*itr)->GetType() == condformat::entry::DATE)
                return;

            itr->disposeAndClear();
            *itr = VclPtr<ScDateFrmtEntry>::Create( this, mpDoc );
            mpDialogParent->InvalidateRefData();
            (*itr)->SetActive();
            break;

    }
    RecalcAll();
}

IMPL_LINK_NOARG_TYPED( ScCondFormatList, AddBtnHdl, Button*, void )
{
    VclPtr<ScCondFrmtEntry> pNewEntry = VclPtr<ScConditionFrmtEntry>::Create(this, mpDoc, mpDialogParent, maPos);
    maEntries.push_back( pNewEntry );
    for(EntryContainer::iterator itr = maEntries.begin(); itr != maEntries.end(); ++itr)
    {
        (*itr)->SetInactive();
    }
    mpDialogParent->InvalidateRefData();
    pNewEntry->SetActive();
    RecalcAll();
}

IMPL_LINK_NOARG_TYPED( ScCondFormatList, RemoveBtnHdl, Button*, void )
{
    for(EntryContainer::iterator itr = maEntries.begin(); itr != maEntries.end(); ++itr)
    {
        if((*itr)->IsSelected())
        {
            itr->disposeAndClear();
            maEntries.erase(itr);
            break;
        }
    }
    mpDialogParent->InvalidateRefData();
    RecalcAll();
}

IMPL_LINK( ScCondFormatList, EntrySelectHdl, ScCondFrmtEntry*, pEntry )
{
    if(pEntry->IsSelected())
        return 0;

    //A child has focus, but we will hide that, so regrab to whatever new thing gets
    //shown instead of leaving it stuck in the inaccessible hidden element
    bool bReGrabFocus = HasChildPathFocus();
    for(EntryContainer::iterator itr = maEntries.begin(); itr != maEntries.end(); ++itr)
    {
        (*itr)->SetInactive();
    }
    mpDialogParent->InvalidateRefData();
    pEntry->SetActive();
    RecalcAll();
    if (bReGrabFocus)
        GrabFocus();
    return 0;
}

IMPL_LINK_NOARG_TYPED( ScCondFormatList, ScrollHdl, ScrollBar*, void )
{
    DoScroll(mpScrollBar->GetDelta());
}

// -------------------------------------------------------------------
// Conditional Format Dialog
//
ScCondFormatDlg::ScCondFormatDlg(SfxBindings* pB, SfxChildWindow* pCW,
    vcl::Window* pParent, ScViewData* pViewData,
    const ScConditionalFormat* pFormat, const ScRangeList& rRange,
    const ScAddress& rPos, condformat::dialog::ScCondFormatDialogType eType,
    bool bManaged)
        : ScAnyRefDlg(pB, pCW, pParent, "ConditionalFormatDialog",
                        "modules/scalc/ui/conditionalformatdialog.ui")
    , mbManaged(bManaged)
    , maPos(rPos)
    , mpViewData(pViewData)
    , mpLastEdit(NULL)
{
    get(mpBtnOk, "ok");
    get(mpBtnAdd, "add");
    get(mpBtnRemove, "delete");
    get(mpBtnCancel, "cancel");

    get(mpFtRange, "ftassign");
    get(mpEdRange, "edassign");
    mpEdRange->SetReferences(this, mpFtRange);

    get(mpRbRange, "rbassign");
    mpRbRange->SetReferences(this, mpEdRange);

    maKey = pFormat ? pFormat->GetKey() : 0;

    get(mpCondFormList, "list");
    mpCondFormList->init(mpViewData->GetDocument(), this, pFormat, rRange, rPos, eType);

    mpBtnOk->SetClickHdl(LINK(this, ScCondFormatDlg, BtnPressedHdl ) );
    mpBtnAdd->SetClickHdl( LINK( mpCondFormList, ScCondFormatList, AddBtnHdl ) );
    mpBtnRemove->SetClickHdl( LINK( mpCondFormList, ScCondFormatList, RemoveBtnHdl ) );
    mpBtnCancel->SetClickHdl( LINK(this, ScCondFormatDlg, BtnPressedHdl ) );
    mpEdRange->SetModifyHdl( LINK( this, ScCondFormatDlg, EdRangeModifyHdl ) );
    mpEdRange->SetGetFocusHdl( LINK( this, ScCondFormatDlg, RangeGetFocusHdl ) );

    OUString aRangeString;
    rRange.Format(aRangeString, SCA_VALID, pViewData->GetDocument(),
                    pViewData->GetDocument()->GetAddressConvention());
    mpEdRange->SetText(aRangeString);

    msBaseTitle = GetText();
    updateTitle();
}

void ScCondFormatDlg::updateTitle()
{
    OUString aTitle = msBaseTitle + " " + mpEdRange->GetText();

    SetText(aTitle);
}

ScCondFormatDlg::~ScCondFormatDlg()
{
    disposeOnce();
}

void ScCondFormatDlg::dispose()
{
    mpBtnOk.clear();
    mpBtnAdd.clear();
    mpBtnRemove.clear();
    mpBtnCancel.clear();
    mpFtRange.clear();
    mpEdRange.clear();
    mpRbRange.clear();
    mpCondFormList.clear();
    mpLastEdit.clear();

    ScAnyRefDlg::dispose();
}

void ScCondFormatDlg::SetActive()
{
    if(mpLastEdit)
        mpLastEdit->GrabFocus();
    else
        mpEdRange->GrabFocus();

    RefInputDone();
}

void ScCondFormatDlg::RefInputDone( bool bForced )
{
    ScAnyRefDlg::RefInputDone(bForced);

    // ScAnyRefModalDlg::RefInputDone resets the title back
    // to it's original state.
    // I.e. if we open the dialog normally, and then click into the sheet
    // to modify the selection, the title is updated such that the range
    // is only a single cell (e.g. $A$1), after which the dialog switches
    // into the RefInput mode. During the RefInput mode the title is updated
    // as expected, however at the end RefInputDone overwrites the title
    // with the initial (now incorrect) single cell range. Hence we correct
    // it here.
    updateTitle();
}

bool ScCondFormatDlg::IsTableLocked() const
{
    if (mpLastEdit && mpLastEdit != mpEdRange)
        return false;

    return true;
}

bool ScCondFormatDlg::IsRefInputMode() const
{
    return mpEdRange->IsEnabled();
}

#define ABS_SREF          SCA_VALID \
    | SCA_COL_ABSOLUTE | SCA_ROW_ABSOLUTE | SCA_TAB_ABSOLUTE
#define ABS_DREF          ABS_SREF \
    | SCA_COL2_ABSOLUTE | SCA_ROW2_ABSOLUTE | SCA_TAB2_ABSOLUTE
#define ABS_DREF3D      ABS_DREF | SCA_TAB_3D

void ScCondFormatDlg::SetReference(const ScRange& rRef, ScDocument*)
{
    formula::RefEdit* pEdit = mpLastEdit;
    if (!mpLastEdit)
        pEdit = mpEdRange;

    if( pEdit->IsEnabled() )
    {
        if(rRef.aStart != rRef.aEnd)
            RefInputStart(pEdit);

        sal_uInt16 n = 0;
        if (mpLastEdit && mpLastEdit != mpEdRange)
            n = ABS_DREF3D;
        else
            n = ABS_DREF;

        OUString aRefStr(rRef.Format(n, mpViewData->GetDocument(),
            ScAddress::Details(mpViewData->GetDocument()->GetAddressConvention(), 0, 0)));
        pEdit->SetRefString( aRefStr );
        updateTitle();
    }
}

ScConditionalFormat* ScCondFormatDlg::GetConditionalFormat() const
{
    OUString aRangeStr = mpEdRange->GetText();
    if(aRangeStr.isEmpty())
        return NULL;

    ScRangeList aRange;
    sal_uInt16 nFlags = aRange.Parse(aRangeStr, mpViewData->GetDocument(),
        SCA_VALID, mpViewData->GetDocument()->GetAddressConvention(), maPos.Tab());
    ScConditionalFormat* pFormat = mpCondFormList->GetConditionalFormat();

    if(nFlags & SCA_VALID && !aRange.empty() && pFormat)
        pFormat->SetRange(aRange);
    else
    {
        delete pFormat;
        pFormat = NULL;
    }

    return pFormat;
}

void ScCondFormatDlg::InvalidateRefData()
{
    mpLastEdit = NULL;
}

// -------------------------------------------------------------
// Close the Conditional Format Dialog
//
bool ScCondFormatDlg::Close()
{
    return DoClose( ScCondFormatDlgWrapper::GetChildWindowId() );
}

// ------------------------------------------------------------------------
// Occurs when the Conditional Format Dialog the OK button is pressed.
//
void ScCondFormatDlg::OkPressed()
{
    ScConditionalFormat* pFormat = GetConditionalFormat();

    if(pFormat)
        mpViewData->GetDocShell()->GetDocFunc().ReplaceConditionalFormat(maKey,
            pFormat, maPos.Tab(), pFormat->GetRange());
    else
        mpViewData->GetDocShell()->GetDocFunc().ReplaceConditionalFormat(maKey,
            NULL, maPos.Tab(), ScRangeList());

    if ( mbManaged )
    {
        SetDispatcherLock( false );
        // Queue message to open Conditional Format Manager Dialog
        GetBindings().GetDispatcher()->Execute( SID_OPENDLG_CONDFRMT_MANAGER,
                                            SfxCallMode::ASYNCHRON );
    }
    Close();
}

// ------------------------------------------------------------------------
// Occurs when the Conditional Format Dialog is cancelled.
//
void ScCondFormatDlg::CancelPressed()
{
    if ( mbManaged )
    {
        SetDispatcherLock( false );
        // Queue message to open Conditional Format Manager Dialog
        GetBindings().GetDispatcher()->Execute( SID_OPENDLG_CONDFRMT_MANAGER,
                                            SfxCallMode::ASYNCHRON );
    }
    Close();
}

// ------------------------------------------------------------------------------
// Parse xml string parameters used to initialize the Conditional Format Dialog
// when it is created.
//
bool ScCondFormatDlg::ParseXmlString(const OUString&    sXMLString,
                                     sal_uInt32&        nIndex,
                                     sal_uInt8&         nType,
                                     bool&              bManaged)
{
    bool bRetVal = false;
    OString sTagName;
    OUString sTagValue;

    xmlNodePtr      pXmlRoot  = NULL;
    xmlNodePtr      pXmlNode  = NULL;

    OString sOString = OUStringToOString( sXMLString, RTL_TEXTENCODING_UTF8 );
    xmlDocPtr pXmlDoc = xmlParseMemory(sOString.getStr(), sOString.getLength());

    if( pXmlDoc )
    {
        bRetVal = true;
        pXmlRoot = xmlDocGetRootElement( pXmlDoc );
        pXmlNode = pXmlRoot->children;

        while (pXmlNode != NULL && bRetVal)
        {
            sTagName  = OUStringToOString(OUString("Index"), RTL_TEXTENCODING_UTF8);
            if (xmlStrcmp(pXmlNode->name, reinterpret_cast<xmlChar const *>(sTagName.getStr())) == 0)
            {
                if (pXmlNode->children != NULL && pXmlNode->children->type == XML_TEXT_NODE)
                {
                    sTagValue = OUString(reinterpret_cast<char*>(pXmlNode->children->content),
                                     strlen(reinterpret_cast<char*>(pXmlNode->children->content)),
                                     RTL_TEXTENCODING_UTF8);
                    nIndex   = sTagValue.toUInt32();
                    pXmlNode = pXmlNode->next;
                    continue;
                }
            }

            sTagName  = OUStringToOString(OUString("Type"), RTL_TEXTENCODING_UTF8);
            if (xmlStrcmp(pXmlNode->name, reinterpret_cast<xmlChar const *>(sTagName.getStr())) == 0)
            {
                if (pXmlNode->children != NULL && pXmlNode->children->type == XML_TEXT_NODE)
                {
                    sTagValue = OUString(reinterpret_cast<char*>(pXmlNode->children->content),
                                     strlen(reinterpret_cast<char*>(pXmlNode->children->content)),
                                     RTL_TEXTENCODING_UTF8);
                    nType    = sTagValue.toUInt32();
                    pXmlNode = pXmlNode->next;
                    continue;
                }
            }

            sTagName  = OUStringToOString(OUString("Managed"), RTL_TEXTENCODING_UTF8);
            if (xmlStrcmp(pXmlNode->name, reinterpret_cast<xmlChar const *>(sTagName.getStr())) == 0)
            {
                if (pXmlNode->children != NULL && pXmlNode->children->type == XML_TEXT_NODE)
                {
                    sTagValue = OUString(reinterpret_cast<char*>(pXmlNode->children->content),
                                     strlen(reinterpret_cast<char*>(pXmlNode->children->content)),
                                     RTL_TEXTENCODING_UTF8);
                    bManaged = sTagValue.toBoolean();
                    pXmlNode = pXmlNode->next;
                    continue;
                }
            }
            bRetVal = false;
        }
    }

    xmlFreeDoc(pXmlDoc);
    return bRetVal;
}

// ---------------------------------------------------------------------------------------
// Generate xml string parameters used to initialize the Conditional Format Dialog
// when it is created.
//
OUString ScCondFormatDlg::GenerateXmlString(sal_uInt32 nIndex, sal_uInt8 nType, bool bManaged)
{
    OUString sReturn;

    OString sTagName;
    OString sTagValue;

    xmlNodePtr      pXmlRoot  = NULL;
    xmlNodePtr      pXmlNode  = NULL;

    xmlChar*        pBuffer   = NULL;
    const xmlChar*  pTagName  = NULL;
    const xmlChar*  pTagValue = NULL;

    xmlDocPtr pXmlDoc = xmlNewDoc(reinterpret_cast<const xmlChar*>("1.0"));

    sTagName = OUStringToOString(OUString("ScCondFormatDlg"), RTL_TEXTENCODING_UTF8);
    pTagName = reinterpret_cast<const xmlChar*>(sTagName.getStr());
    pXmlRoot = xmlNewDocNode(pXmlDoc, NULL, pTagName, NULL);

    xmlDocSetRootElement(pXmlDoc, pXmlRoot);

    sTagName  = OUStringToOString(OUString("Index"), RTL_TEXTENCODING_UTF8);
    sTagValue = OUStringToOString(OUString::number(nIndex), RTL_TEXTENCODING_UTF8);
    pTagName  = reinterpret_cast<const xmlChar*>(sTagName.getStr());
    pTagValue = reinterpret_cast<const xmlChar*>(sTagValue.getStr());
    pXmlNode  = xmlNewDocNode(pXmlDoc, NULL, pTagName, pTagValue);

    xmlAddChild(pXmlRoot, pXmlNode);

    sTagName  = OUStringToOString(OUString("Type"), RTL_TEXTENCODING_UTF8);
    sTagValue = OUStringToOString(OUString::number(nType), RTL_TEXTENCODING_UTF8);
    pTagName  = reinterpret_cast<const xmlChar*>(sTagName.getStr());
    pTagValue = reinterpret_cast<const xmlChar*>(sTagValue.getStr());
    pXmlNode  = xmlNewDocNode(pXmlDoc, NULL, pTagName, pTagValue);

    xmlAddChild(pXmlRoot, pXmlNode);

    sTagName  = OUStringToOString(OUString("Managed"), RTL_TEXTENCODING_UTF8);
    sTagValue = OUStringToOString(OUString::boolean(bManaged), RTL_TEXTENCODING_UTF8);
    pTagName  = reinterpret_cast<const xmlChar*>(sTagName.getStr());
    pTagValue = reinterpret_cast<const xmlChar*>(sTagValue.getStr());
    pXmlNode  = xmlNewDocNode(pXmlDoc, NULL, pTagName, pTagValue);

    xmlAddChild(pXmlRoot, pXmlNode);

    int nSize = 0;
    xmlDocDumpMemory(pXmlDoc, &pBuffer, &nSize);

    sReturn = OUString(reinterpret_cast<char const *>(pBuffer), nSize, RTL_TEXTENCODING_UTF8);

    xmlFree(pBuffer);
    xmlFreeDoc(pXmlDoc);

    return sReturn;
}


IMPL_LINK( ScCondFormatDlg, EdRangeModifyHdl, Edit*, pEdit )
{
    OUString aRangeStr = pEdit->GetText();
    ScRangeList aRange;
    sal_uInt16 nFlags = aRange.Parse(aRangeStr, mpViewData->GetDocument(),
        SCA_VALID, mpViewData->GetDocument()->GetAddressConvention());
    if(nFlags & SCA_VALID)
        pEdit->SetControlBackground(GetSettings().GetStyleSettings().GetWindowColor());
    else
        pEdit->SetControlBackground(COL_LIGHTRED);

    updateTitle();
    return 0;
}

IMPL_LINK_TYPED( ScCondFormatDlg, RangeGetFocusHdl, Control&, rControl, void )
{
    mpLastEdit = static_cast<formula::RefEdit*>(&rControl);
}

// ------------------------------------------------------
// Conditional Format Dialog button click event handler.
//
IMPL_LINK_TYPED( ScCondFormatDlg, BtnPressedHdl, Button*, pBtn, void)
{
    if (pBtn == mpBtnOk)
        OkPressed();
    else if (pBtn == mpBtnCancel)
        CancelPressed();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
