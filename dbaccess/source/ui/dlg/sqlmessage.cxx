/*************************************************************************
 *
 *  $RCSfile: sqlmessage.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2000-10-18 09:11:18 $
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

#ifndef _DBAUI_SQLMESSAGE_HXX_
#include "sqlmessage.hxx"
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef _DBU_RESOURCE_HRC_
#include "dbu_resource.hrc"
#endif
#ifndef _DBAUI_SQLMESSAGE_HRC_
#include "sqlmessage.hrc"
#endif
#ifndef _COM_SUN_STAR_SDBC_SQLEXCEPTION_HPP_
#include <com/sun/star/sdbc/SQLException.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_SQLCONTEXT_HPP_
#include <com/sun/star/sdb/SQLContext.hpp>
#endif
#ifndef _SV_GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif
#ifndef _SVTREEBOX_HXX
#include <svtools/svtreebx.hxx>
#endif
#ifndef _SVEDIT_HXX //autogen
#include <svtools/svmedit.hxx>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif

#define BUTTONID_MORE   BUTTONID_RETRY + 1

#define DLG_LIMIT       320 // max dialog size
#define BTN_HEIGHT      14
#define BTN_WIDTH       50
#define BORDER_HEIGHT   6   // default distance control - dialog
#define BORDER_WIDTH    6   // default distance control - dialog

using namespace dbtools;
using namespace com::sun::star::sdb;
using namespace com::sun::star::sdbc;

//.........................................................................
namespace dbaui
{
//.........................................................................

//==============================================================================
class OExceptionChainDialog : public ModalDialog
{
    GroupBox        m_aFrame;
    FixedText       m_aListLabel;
    SvTreeListBox   m_aExceptionList;
    FixedText       m_aDescLabel;
    MultiLineEdit   m_aExceptionText;
    OKButton        m_aOK;

public:
    OExceptionChainDialog(Window* pParent, const ::com::sun::star::uno::Any& _rStart);
    ~OExceptionChainDialog();

protected:
    DECL_LINK(OnExceptionSelected, void*);
};

//------------------------------------------------------------------------------
OExceptionChainDialog::OExceptionChainDialog(Window* pParent, const ::com::sun::star::uno::Any& _rStart)
    :ModalDialog(pParent, ModuleRes(DLG_SQLEXCEPTIONCHAIN))
    ,m_aFrame           (this, ResId(GB_DETAILS))
    ,m_aListLabel       (this, ResId(FT_ERRORLIST))
    ,m_aExceptionList   (this, ResId(CTL_ERRORLIST))
    ,m_aDescLabel       (this, ResId(FT_DESCRIPTION))
    ,m_aExceptionText   (this, ResId(ME_DESCRIPTION))
    ,m_aOK              (this, ResId(PB_OK))
{
    String sErrorLabel(ResId(STR_EXCEPTION_ERROR));
    String sWarningLabel(ResId(STR_EXCEPTION_WARNING));
    String sInfoLabel(ResId(STR_EXCEPTION_INFO));
    String sDetailsLabel(ResId(STR_EXCEPTION_DETAILS));
    String sStatusLabel(ResId(STR_EXCEPTION_STATUS));
    String sErrorCodeLabel(ResId(STR_EXCEPTION_ERRORCODE));

    FreeResource();

    m_aExceptionList.SetSelectionMode(SINGLE_SELECTION);
    m_aExceptionList.SetDragDropMode(0);
    m_aExceptionList.EnableInplaceEditing(sal_False);
    m_aExceptionList.SetWindowBits(WB_HASLINES | WB_HASLINESATROOT | WB_HASBUTTONS | WB_HASBUTTONSATROOT | WB_HSCROLL);

    m_aExceptionList.SetSelectHdl(LINK(this, OExceptionChainDialog, OnExceptionSelected));
    ModuleRes aPlusButton(BMP_PLUSBUTTON);
    ModuleRes aMinusButton(BMP_MINUSBUTTON);
    Bitmap  aPlusButtonBitmap(aPlusButton);
    Bitmap  aMinusButtonBitmap(aMinusButton);
    Image aCollapsedImage(aPlusButtonBitmap);
    Image aExpandedImage(aMinusButtonBitmap);
//  Image aCollapsedImage(Bitmap(ModuleRes(BMP_PLUSBUTTON)));
//  Image aExpandedImage(Bitmap(ModuleRes(BMP_MINUSBUTTON)));
    m_aExceptionList.SetNodeBitmaps( aCollapsedImage, aExpandedImage);

    m_aExceptionText.SetReadOnly(sal_True);

    SQLExceptionInfo aInfo(_rStart);
    DBG_ASSERT(aInfo.isValid(), "OExceptionChainDialog::OExceptionChainDialog : invalid chain start !");
    SQLExceptionIteratorHelper aIter(( const ::com::sun::star::sdbc::SQLException*)aInfo);

    Image   aErrorImage(ModuleRes(BMP_EXCEPTION_ERROR)),
            aWarningImage(ModuleRes(BMP_EXCEPTION_WARNING)),
            m_aInfoImage(ModuleRes(BMP_EXCEPTION_INFO));

    while (aIter.hasMoreElements())
    {
        SQLExceptionInfo aCurrent(*aIter.next());
        if (aCurrent.isValid())
        {
            const ::com::sun::star::sdbc::SQLException* pCurrentException = (const ::com::sun::star::sdbc::SQLException*)aCurrent;
            SvLBoxEntry* pListEntry = NULL;
            ::com::sun::star::sdbc::SQLException* pUserData;
            switch (aCurrent.getType())
            {
                case SQLExceptionInfo::SQL_EXCEPTION:
                    pListEntry = m_aExceptionList.InsertEntry(sErrorLabel, aErrorImage, aErrorImage);
                    pUserData = new ::com::sun::star::sdbc::SQLException(*(const ::com::sun::star::sdbc::SQLException*)aCurrent);
                    if (pUserData->SQLState.getLength())
                    {
                        UniString sTitle(sStatusLabel);
                        sTitle.AppendAscii(" : ");
                        sTitle += pUserData->SQLState.getStr();
                        SvLBoxEntry* pSQLstateEntry = m_aExceptionList.InsertEntry(sTitle, aErrorImage, aErrorImage, pListEntry);
                        pSQLstateEntry->SetUserData(pUserData);
                        m_aExceptionList.Expand(pListEntry);
                    }
                    if (pUserData->ErrorCode != 0)
                    {
                        UniString sTitle(sErrorCodeLabel);
                        sTitle.AppendAscii(" : ");
                        sTitle += String::CreateFromInt32(pUserData->ErrorCode);
                        SvLBoxEntry* pErrorCodeEntry = m_aExceptionList.InsertEntry(sTitle, aErrorImage, aErrorImage, pListEntry);
                        pErrorCodeEntry->SetUserData(pUserData);
                        m_aExceptionList.Expand(pListEntry);
                    }
                    break;
                case SQLExceptionInfo::SQL_WARNING:
                    pListEntry = m_aExceptionList.InsertEntry(sWarningLabel, aWarningImage, aWarningImage);
                    pUserData = new ::com::sun::star::sdbc::SQLWarning(*(const ::com::sun::star::sdbc::SQLWarning*)aCurrent);
                    break;
                case SQLExceptionInfo::SQL_CONTEXT:
                {
                    pListEntry = m_aExceptionList.InsertEntry(sInfoLabel, m_aInfoImage, m_aInfoImage);
                    pUserData = new SQLContext(*(const SQLContext*)aCurrent);
                    if (((const SQLContext*)aCurrent)->Details.getLength())
                    {
                        SvLBoxEntry* pDetailsEntry = m_aExceptionList.InsertEntry(sDetailsLabel, m_aInfoImage, m_aInfoImage, pListEntry);
                        pDetailsEntry->SetUserData(pUserData);
                        m_aExceptionList.Expand(pListEntry);
                    }
                }
                break;
                default:
                    DBG_ERROR("OExceptionChainDialog::OExceptionChainDialog : valid ::com::sun::star::sdbc::SQLException but unknown type !");
                    break;
            }
            if (pListEntry)
                pListEntry->SetUserData(pUserData);
        }
    }
}

//------------------------------------------------------------------------------
OExceptionChainDialog::~OExceptionChainDialog()
{
    SvLBoxEntry* pLoop = m_aExceptionList.First();
    while (pLoop)
    {
        if (!m_aExceptionList.GetParent(pLoop))
            // it's not the "details" entry for an SQLContext object
            delete (::com::sun::star::sdbc::SQLException*)pLoop->GetUserData();
        pLoop = m_aExceptionList.Next(pLoop);
    }
}

//------------------------------------------------------------------------------
IMPL_LINK(OExceptionChainDialog, OnExceptionSelected, void*, EMPTYARG)
{
    SvLBoxEntry* pSelected = m_aExceptionList.FirstSelected();
    DBG_ASSERT(!pSelected || !m_aExceptionList.NextSelected(pSelected), "OExceptionChainDialog::OnExceptionSelected : multi selection ?");

    if (!pSelected)
        m_aExceptionText.SetText(UniString());
    else
    {
        SQLExceptionInfo aInfo(*(const ::com::sun::star::sdbc::SQLException*)pSelected->GetUserData());
        ::rtl::OUString aText = ((const ::com::sun::star::sdbc::SQLException*)aInfo)->Message;

        if (m_aExceptionList.GetParent(pSelected))
            if (aInfo.isKindOf(SQLExceptionInfo::SQL_CONTEXT))
                // Context-Details
                aText = ((const SQLContext*)aInfo)->Details;
            else
                // all other children get the text of the parent
                aText = ((const ::com::sun::star::sdbc::SQLException*)aInfo)->Message;

        m_aExceptionText.SetText(aText);
    }

    return 0L;
}

//------------------------------------------------------------------------------
void OSQLMessageBox::Construct(const UniString& rTitle,
                          const UniString& rMessage,
                          WinBits nStyle,
                          MessageType eImage)
{
    SetText(String::CreateFromAscii("StarBase"));
    SetSizePixel(LogicToPixel(Size(220, 30),MAP_APPFONT));

    m_aInfoImage.SetPosSizePixel(LogicToPixel(Point(6, 6),MAP_APPFONT),
                               LogicToPixel(Size(20, 20),MAP_APPFONT));
    m_aInfoImage.Show();

    m_aTitle.SetPosSizePixel(LogicToPixel(Point(45, 6),MAP_APPFONT),
                             LogicToPixel(Size(169, 20),MAP_APPFONT));

    Font  aFont = m_aTitle.GetFont();
    aFont.SetWeight(WEIGHT_SEMIBOLD);
    m_aTitle.SetFont(aFont);
    m_aTitle.Show();

    m_aMessage.SetPosSizePixel(LogicToPixel(Point(45, 29),MAP_APPFONT),
                               LogicToPixel(Size(169, 1),MAP_APPFONT));
    m_aMessage.Show();

    m_pInfoButton = NULL;

    // Image festlegen
    switch (eImage)
    {
        case Info:
            m_aInfoImage.SetImage(InfoBox::GetStandardImage());
            break;
        case Warning:
            m_aInfoImage.SetImage(WarningBox::GetStandardImage());
            break;
        case Error:
            m_aInfoImage.SetImage(ErrorBox::GetStandardImage());
            break;
        case Query:
            m_aInfoImage.SetImage(QueryBox::GetStandardImage());
            break;
    }

    // Title setzen
    m_aTitle.SetText(rTitle);

    // Ermitteln der Hoehe des Textfeldes und des Dialogs
    Size aBorderSize = LogicToPixel(Size(BORDER_WIDTH, BORDER_HEIGHT),MAP_APPFONT);
    Rectangle aDlgRect(GetPosPixel(),GetSizePixel());
    Rectangle aMessageRect(m_aMessage.GetPosPixel(),m_aMessage.GetSizePixel());
    Rectangle aTextRect  =
        GetTextRect(aMessageRect,rMessage, TEXT_DRAW_WORDBREAK |
                                           TEXT_DRAW_MULTILINE | TEXT_DRAW_LEFT);

    long nHText = aTextRect.Bottom() > aMessageRect.Bottom() ? aTextRect.Bottom() - aMessageRect.Bottom() : 0;

    aDlgRect.Bottom() += nHText + 2 * aBorderSize.Height();
    aMessageRect.Bottom() += nHText;

    // Dialog anpassen
    SetSizePixel(aDlgRect.GetSize());
    SetPageSizePixel(aDlgRect.GetSize());

    // Message Text anpassen und setzen
    m_aMessage.SetSizePixel(aMessageRect.GetSize());
    m_aMessage.SetText(rMessage);

    // Buttons anlegen
    long   nBtnCount = (m_sInfo.Len() != 0) ? 1 : 0;
    sal_Bool   bHelp = sal_False; //aHelpBtn.IsVisible();

    sal_uInt16 nDefId = 0;

    if (nStyle & WB_DEF_YES)
        nDefId = BUTTONID_YES;
    else if (nStyle & WB_DEF_NO)
        nDefId = BUTTONID_NO;
    else if (nStyle & WB_DEF_CANCEL)
        nDefId = BUTTONID_CANCEL;
    else if (nStyle & WB_DEF_RETRY)
        nDefId = BUTTONID_RETRY;
    else
        nDefId = BUTTONID_OK;

    if (nStyle & WB_YES_NO_CANCEL)
    {
        if (nStyle & WB_DEF_YES)
            AddButton(BUTTON_YES,BUTTONID_YES,BUTTONDIALOG_DEFBUTTON|BUTTONDIALOG_FOCUSBUTTON);
        else
            AddButton(BUTTON_YES,BUTTONID_YES,0);

        if (nStyle & WB_DEF_NO)
            AddButton(BUTTON_NO,BUTTONID_NO,BUTTONDIALOG_DEFBUTTON|BUTTONDIALOG_FOCUSBUTTON);
        else
            AddButton(BUTTON_NO,BUTTONID_NO, 0);

        if (nStyle & WB_DEF_CANCEL)
            AddButton(BUTTON_CANCEL,BUTTONID_CANCEL,BUTTONDIALOG_DEFBUTTON|BUTTONDIALOG_FOCUSBUTTON);
        else
            AddButton(BUTTON_CANCEL,BUTTONID_CANCEL, 0);
    }
    else if (nStyle & WB_OK_CANCEL)
    {
        if (nStyle & WB_DEF_CANCEL)
        {
            AddButton(BUTTON_OK,BUTTONID_OK,0);
            AddButton(BUTTON_CANCEL,BUTTONID_CANCEL,BUTTONDIALOG_DEFBUTTON|BUTTONDIALOG_FOCUSBUTTON);
        }
        else
        {
            AddButton(BUTTON_OK,BUTTONID_OK,BUTTONDIALOG_DEFBUTTON|BUTTONDIALOG_FOCUSBUTTON);
            AddButton(BUTTON_CANCEL,BUTTONID_CANCEL,0);
        }
    }
    else if (nStyle & WB_YES_NO)
    {
        if (nStyle & WB_DEF_YES)
            AddButton(BUTTON_YES,BUTTONID_YES,BUTTONDIALOG_DEFBUTTON|BUTTONDIALOG_FOCUSBUTTON);
        else
            AddButton(BUTTON_YES,BUTTONID_YES,0);

        if (nStyle & WB_DEF_NO)
            AddButton(BUTTON_NO,BUTTONID_NO,BUTTONDIALOG_DEFBUTTON|BUTTONDIALOG_FOCUSBUTTON);
        else
            AddButton(BUTTON_NO,BUTTONID_NO, 0);
    }
    else if (nStyle & WB_RETRY_CANCEL)
    {
        if (nStyle & WB_DEF_RETRY)
            AddButton(BUTTON_RETRY,BUTTONID_RETRY,BUTTONDIALOG_DEFBUTTON|BUTTONDIALOG_FOCUSBUTTON);
        else
            AddButton(BUTTON_YES,BUTTONID_YES,0);

        if (nStyle & WB_DEF_CANCEL)
            AddButton(BUTTON_CANCEL,BUTTONID_CANCEL,BUTTONDIALOG_DEFBUTTON|BUTTONDIALOG_FOCUSBUTTON);
        else
            AddButton(BUTTON_CANCEL,BUTTONID_CANCEL, 0);
    }
    else
    {
        DBG_ASSERT(WB_OK & nStyle, "kein Button gesetzt");
        AddButton(BUTTON_OK,BUTTONID_OK,BUTTONDIALOG_DEFBUTTON|BUTTONDIALOG_FOCUSBUTTON);
    }

    sal_Bool bAtLeastTwo = m_aNextChainElement.hasValue() && ((::com::sun::star::sdbc::SQLException*)m_aNextChainElement.getValue())->NextException.getValue();
    if (bAtLeastTwo)
    {
        m_pInfoButton = new PushButton(this);
        m_pInfoButton->SetText(Button::GetStandardText(BUTTON_MORE));
        m_pInfoButton->SetClickHdl(LINK(this,OSQLMessageBox,ButtonClickHdl));
        m_pInfoButton->Show();
        AddButton(m_pInfoButton, BUTTONID_MORE, 0);
    }
}

//------------------------------------------------------------------------------
void OSQLMessageBox::Construct(const SQLExceptionInfo& _rException, WinBits _nStyle, MessageType _eImage)
{
    const SQLException* pFirst = NULL;
    if (_rException.isKindOf(SQLExceptionInfo::SQL_EXCEPTION))
        pFirst = (const SQLException*)_rException;

    // get the first two strings in the chain
    String sTitle, sMessage;
    if (pFirst)
    {
        sTitle = pFirst->Message;
            // we assume this to be not empty, so in reall we're searching the only te second string, the first
            // one is always the Message of the first exception)

        if (_rException.isKindOf(SQLExceptionInfo::SQL_CONTEXT))
        {   // take the detailed message
            const SQLContext* pContext = (const SQLContext*)_rException;
            sMessage = pContext->Details.getStr();
        }

        if (!sMessage.Len())
        {
            // loop through all the remaining exceptions
            SQLExceptionIteratorHelper aIter((const SQLException*)pFirst);
                // note that this leaves aIter in a state where it's current exception is only an SQLException,
                // even if _rException was more than this. But this is irrelevant here, as we always handled
                // this first chain element
            while (aIter.hasMoreElements())
            {
                SQLExceptionInfo aInfo(*aIter.next());
                if (aInfo.isValid())
                {   // first take the normal message of the exception
                    const SQLException* pException = (const SQLException*)aInfo;
                    sMessage = pException->Message.getStr();
                    // the, if necessary and possible, the details
                    if (aInfo.isKindOf(SQLExceptionInfo::SQL_CONTEXT))
                    {   // check if we have a detailed message
                        const SQLContext* pContext = (const SQLContext*)aInfo;
                        sMessage = pContext->Details.getStr();
                    }
                }
            }
        }
    }

    if (!sMessage.Len())
    {   // use the only string we have as message and an default title
        sMessage = sTitle;
        sTitle = ModuleRes(STR_GENERAL_SDB_ERROR);
    }

    Construct(sTitle, sMessage, _nStyle, _eImage);
}

//------------------------------------------------------------------------------
OSQLMessageBox::OSQLMessageBox(Window* _pParent, const UniString& _rTitle, const ::com::sun::star::sdbc::SQLException& _rError, WinBits _nStyle,
                        MessageType _eImage)
    :ButtonDialog(_pParent,WB_HORZ | WB_STDDIALOG)
    ,m_aInfoImage(this)
    ,m_aTitle(this,WB_WORDBREAK | WB_LEFT)
    ,m_aMessage(this,WB_WORDBREAK | WB_LEFT)
    ,m_pInfoButton(NULL)
    ,m_aNextChainElement(SQLExceptionInfo(_rError).get())
{
    Construct(_rTitle, _rError.Message, _nStyle, _eImage);
}

//------------------------------------------------------------------------------
OSQLMessageBox::OSQLMessageBox(Window* _pParent, const ::com::sun::star::sdbc::SQLException& _rError, WinBits _nStyle, MessageType _eImage)
    :ButtonDialog(_pParent,WB_HORZ | WB_STDDIALOG)
    ,m_aInfoImage(this)
    ,m_aTitle(this,WB_WORDBREAK | WB_LEFT)
    ,m_aMessage(this,WB_WORDBREAK | WB_LEFT)
    ,m_pInfoButton(NULL)
    ,m_aNextChainElement(SQLExceptionInfo(_rError).get())
{
    Construct(SQLExceptionInfo(_rError), _nStyle, _eImage);
}

//------------------------------------------------------------------------------
OSQLMessageBox::OSQLMessageBox(Window* _pParent, const SQLExceptionInfo& _rException, WinBits _nStyle, MessageType _eImage)
      :ButtonDialog(_pParent,WB_HORZ | WB_STDDIALOG)
      ,m_aInfoImage(this)
      ,m_aTitle(this,WB_WORDBREAK | WB_LEFT)
      ,m_aMessage(this,WB_WORDBREAK | WB_LEFT)
      ,m_pInfoButton(NULL)
      ,m_aNextChainElement(_rException.get())
{
    Construct(_rException, _nStyle, _eImage);
}

//------------------------------------------------------------------------------
OSQLMessageBox::OSQLMessageBox(Window* pParent, const UniString& rTitle, const UniString& rMessage, WinBits nStyle, MessageType eImage)
      :ButtonDialog(pParent,WB_HORZ | WB_STDDIALOG)
      ,m_aInfoImage(this)
      ,m_aTitle(this,WB_WORDBREAK | WB_LEFT)
      ,m_aMessage(this,WB_WORDBREAK | WB_LEFT)
      ,m_pInfoButton(NULL)
{
    Construct(rTitle, rMessage, nStyle, eImage);
}

//------------------------------------------------------------------------------
OSQLMessageBox::~OSQLMessageBox()
{
    delete m_pInfoButton;
}

//--------------------------------------------------------------------------
IMPL_LINK( OSQLMessageBox, ButtonClickHdl, Button *, pButton )
{
    OExceptionChainDialog aDlg(GetParent(), m_aNextChainElement);
    Hide();
    aDlg.Execute();
    Show();
    return 0;
}

//.........................................................................
}   // namespace dbaui
//.........................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.2  2000/10/09 12:39:29  fs
 *  some (a lot of) new imlpementations - still under development
 *
 *  Revision 1.1  2000/10/05 10:06:44  fs
 *  initial checkin
 *
 *
 *  Revision 1.0 02.10.00 12:37:20  fs
 ************************************************************************/

