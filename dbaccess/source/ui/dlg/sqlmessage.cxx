/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sqlmessage.cxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: hr $ $Date: 2005-09-23 12:32:10 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _DBAUI_SQLMESSAGE_HXX_
#include "sqlmessage.hxx"
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef _DBU_DLG_HRC_
#include "dbu_dlg.hrc"
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
#include <vcl/fixed.hxx>
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
#ifndef _UTL_CONFIGMGR_HXX_
#include <unotools/configmgr.hxx>
#endif
#ifndef _SFX_SFXUNO_HXX
#include <sfx2/sfxuno.hxx>
#endif
#ifndef _DBA_DBACCESS_HELPID_HRC_
#include "dbaccess_helpid.hrc"
#endif
#ifndef DBAUI_TOOLS_HXX
#include "UITools.hxx"
#endif

#define BUTTONID_MORE   BUTTONID_RETRY + 1

#define DLG_LIMIT       320 // max dialog size
#define BTN_HEIGHT      14
#define BTN_WIDTH       50
#define BORDER_HEIGHT   6   // default distance control - dialog
#define BORDER_WIDTH    6   // default distance control - dialog

using namespace dbtools;
using namespace com::sun::star::uno;
using namespace com::sun::star::sdb;
using namespace com::sun::star::sdbc;

//.........................................................................
namespace dbaui
{
//.........................................................................

//==============================================================================
class OExceptionChainDialog : public ModalDialog
{
    FixedLine       m_aFrame;
    FixedText       m_aListLabel;
    SvTreeListBox   m_aExceptionList;
    FixedText       m_aDescLabel;
    MultiLineEdit   m_aExceptionText;
    OKButton        m_aOK;

    String          m_sStatusLabel;
    String          m_sErrorCodeLabel;

public:
    OExceptionChainDialog(Window* pParent, const Any& _rStart);
    ~OExceptionChainDialog();

protected:
    DECL_LINK(OnExceptionSelected, void*);
};

DBG_NAME(OExceptionChainDialog)
//------------------------------------------------------------------------------
OExceptionChainDialog::OExceptionChainDialog(Window* pParent, const Any& _rStart)
    :ModalDialog(pParent, ModuleRes(DLG_SQLEXCEPTIONCHAIN))
    ,m_aFrame           (this, ResId(FL_DETAILS))
    ,m_aListLabel       (this, ResId(FT_ERRORLIST))
    ,m_aExceptionList   (this, ResId(CTL_ERRORLIST))
    ,m_aDescLabel       (this, ResId(FT_DESCRIPTION))
    ,m_aExceptionText   (this, ResId(ME_DESCRIPTION))
    ,m_aOK              (this, ResId(PB_OK))
{
    DBG_CTOR(OExceptionChainDialog,NULL);

    String sErrorLabel(ResId(STR_EXCEPTION_ERROR));
    String sWarningLabel(ResId(STR_EXCEPTION_WARNING));
    String sInfoLabel(ResId(STR_EXCEPTION_INFO));
    String sDetailsLabel(ResId(STR_EXCEPTION_DETAILS));
    m_sStatusLabel = String( ResId( STR_EXCEPTION_STATUS ) );
    m_sErrorCodeLabel = String( ResId( STR_EXCEPTION_ERRORCODE ) );

    FreeResource();

    m_aExceptionList.SetSelectionMode(SINGLE_SELECTION);
    m_aExceptionList.SetDragDropMode(0);
    m_aExceptionList.EnableInplaceEditing(sal_False);
    m_aExceptionList.SetWindowBits(WB_HASLINES | WB_HASBUTTONS | WB_HASBUTTONSATROOT | WB_HSCROLL);

    m_aExceptionList.SetSelectHdl(LINK(this, OExceptionChainDialog, OnExceptionSelected));
    sal_Bool bHiContrast = isHiContrast(this);

    m_aExceptionList.SetNodeDefaultImages( );

    m_aExceptionText.SetReadOnly(sal_True);

    SQLExceptionInfo aInfo(_rStart);
    DBG_ASSERT(aInfo.isValid(), "OExceptionChainDialog::OExceptionChainDialog : invalid chain start !");
    SQLExceptionIteratorHelper aIter(aInfo);

    Image   aErrorImage(    ModuleRes( bHiContrast ? BMP_EXCEPTION_ERROR_SCH    : BMP_EXCEPTION_ERROR)),
            aWarningImage(  ModuleRes( bHiContrast ? BMP_EXCEPTION_WARNING_SCH  : BMP_EXCEPTION_WARNING)),
            m_aInfoImage(   ModuleRes( bHiContrast ? BMP_EXCEPTION_INFO_SCH     : BMP_EXCEPTION_INFO));

    bool bHave22018 = false;

    SQLExceptionInfo aCurrent;
    while (aIter.hasMoreElements())
    {
        aIter.next(aCurrent);
        if (aCurrent.isValid())
        {
            const SQLException* pCurrentException = (const SQLException*)aCurrent;
            bHave22018 = pCurrentException->SQLState.equalsAscii( "22018" );

            SvLBoxEntry* pListEntry = NULL;
            SQLExceptionInfo* pUserData = new SQLExceptionInfo(aCurrent);

            switch (aCurrent.getType())
            {
                case SQLExceptionInfo::SQL_EXCEPTION:
                {
                    pListEntry = m_aExceptionList.InsertEntry(sErrorLabel, aErrorImage, aErrorImage);
                    const SQLException* pException = (const SQLException*)aCurrent;
                }
                break;

                case SQLExceptionInfo::SQL_WARNING:
                    pListEntry = m_aExceptionList.InsertEntry(sWarningLabel, aWarningImage, aWarningImage);
                    break;

                case SQLExceptionInfo::SQL_CONTEXT:
                {
                    pListEntry = m_aExceptionList.InsertEntry(sInfoLabel, m_aInfoImage, m_aInfoImage);
                    const SQLContext* pContext = (const SQLContext*)aCurrent;
                    if (pContext->Details.getLength())
                    {
                        SvLBoxEntry* pDetailsEntry = m_aExceptionList.InsertEntry(sDetailsLabel, m_aInfoImage, m_aInfoImage, pListEntry);
                        pDetailsEntry->SetUserData(pUserData);
                        m_aExceptionList.Expand(pListEntry);
                    }
                }
                break;

                default:
                    DBG_ERROR("OExceptionChainDialog::OExceptionChainDialog : valid SQLException but unknown type !");
                    break;
            }
            if ( pListEntry )
                pListEntry->SetUserData( pUserData );
            else
                delete (SQLExceptionInfo*)pUserData;
        }
    }

    // if the error has the code 22018, then add an additional explanation
    // #i24021# / 2004-10-14 / frank.schoenheit@sun.com
    if ( bHave22018 )
    {
        SvLBoxEntry* pExplanation = m_aExceptionList.InsertEntry( sInfoLabel, m_aInfoImage, m_aInfoImage );

        SQLContext aExplanation;
        aExplanation.Message = String( ModuleRes( STR_EXPLAN_STRINGCONVERSION_ERROR ) );

        pExplanation->SetUserData( new SQLExceptionInfo( aExplanation ) );
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
            delete static_cast<SQLExceptionInfo*>(pLoop->GetUserData());
        pLoop = m_aExceptionList.Next(pLoop);
    }

    DBG_DTOR(OExceptionChainDialog,NULL);
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
        SQLExceptionInfo aInfo(*(const SQLExceptionInfo*)pSelected->GetUserData());
        const SQLException* pException = (const SQLException*)aInfo;

        String sText;
        if (   ( aInfo.getType() == SQLExceptionInfo::SQL_CONTEXT )
            &&  ( m_aExceptionList.GetParent( pSelected ) != NULL )
            )
        {
            sText = static_cast< const SQLContext* >( pException )->Details;
        }
        else
        {
            if ( pException->SQLState.getLength() )
            {
                sText += m_sStatusLabel;
                sText.AppendAscii(": ");
                sText += pException->SQLState.getStr();
                sText.AppendAscii("\n");
            }
            if ( pException->ErrorCode )
            {
                sText += m_sErrorCodeLabel;
                sText.AppendAscii(": ");
                sText += String::CreateFromInt32( pException->ErrorCode );
                sText.AppendAscii("\n");
            }
            if ( sText.Len() )
                sText.AppendAscii( "\n" );
            sText += String( pException->Message );
        }
        m_aExceptionText.SetText(sText);
    }

    return 0L;
}

//------------------------------------------------------------------------------
void OSQLMessageBox::Construct(const UniString& rTitle,
                          const UniString& rMessage,
                          WinBits nStyle,
                          MessageType eImage)
{
    // Changed as per BugID 79541 Branding/Configuration
    ::utl::ConfigManager* pMgr = ::utl::ConfigManager::GetConfigManager();
    Any aProductName = pMgr->GetDirectConfigProperty(::utl::ConfigManager::PRODUCTNAME);
    ::rtl::OUString sProductName;
    aProductName >>= sProductName;

    String aTitle = sProductName;
    aTitle.AppendAscii(" Base");
    SetText(aTitle);
    SetSizePixel(LogicToPixel(Size(220, 30),MAP_APPFONT));

    m_aInfoImage.SetPosSizePixel(LogicToPixel(Point(6, 6),MAP_APPFONT),
                               LogicToPixel(Size(20, 20),MAP_APPFONT));
    m_aInfoImage.Show();

    m_aTitle.SetPosSizePixel(LogicToPixel(Point(45, 6),MAP_APPFONT),
                             LogicToPixel(Size(169, 20),MAP_APPFONT));

    m_aTitle.Show();

    m_aMessage.SetStyle( m_aMessage.GetStyle() | WB_NOLABEL );
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
    long   nBtnCount = 0;
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

    sal_Bool bAtLeastTwo = m_aNextChainElement.hasValue() && ((SQLException*)m_aNextChainElement.getValue())->NextException.getValue();
    if (bAtLeastTwo)
    {
        m_pInfoButton = new PushButton(this);
        m_pInfoButton->SetText(Button::GetStandardText(BUTTON_MORE));
        m_pInfoButton->SetClickHdl(LINK(this,OSQLMessageBox,ButtonClickHdl));
        m_pInfoButton->SetUniqueId(UID_SQLERROR_BUTTONMORE);
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
            // skip the first one
            if(aIter.hasMoreElements())
                aIter.next();
                // note that this leaves aIter in a state where it's current exception is only an SQLException,
                // even if _rException was more than this. But this is irrelevant here, as we always handled
                // this first chain element
            while (aIter.hasMoreElements() && !sMessage.Len())
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

DBG_NAME(OSQLMessageBox)
//------------------------------------------------------------------------------
OSQLMessageBox::OSQLMessageBox(Window* _pParent, const UniString& _rTitle, const SQLException& _rError, WinBits _nStyle,
                        MessageType _eImage)
    :ButtonDialog(_pParent,WB_HORZ | WB_STDDIALOG)
    ,m_aInfoImage(this)
    ,m_aTitle(this,WB_WORDBREAK | WB_LEFT)
    ,m_aMessage(this,WB_WORDBREAK | WB_LEFT)
    ,m_pInfoButton(NULL)
    ,m_aNextChainElement(SQLExceptionInfo(_rError).get())
{
    DBG_CTOR(OSQLMessageBox,NULL);

    Construct(_rTitle, _rError.Message, _nStyle, _eImage);
}

//------------------------------------------------------------------------------
OSQLMessageBox::OSQLMessageBox(Window* _pParent, const SQLException& _rError, WinBits _nStyle, MessageType _eImage)
    :ButtonDialog(_pParent,WB_HORZ | WB_STDDIALOG)
    ,m_aInfoImage(this)
    ,m_aTitle(this,WB_WORDBREAK | WB_LEFT)
    ,m_aMessage(this,WB_WORDBREAK | WB_LEFT)
    ,m_pInfoButton(NULL)
    ,m_aNextChainElement(SQLExceptionInfo(_rError).get())
{
    DBG_CTOR(OSQLMessageBox,NULL);

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
    DBG_CTOR(OSQLMessageBox,NULL);

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
    DBG_CTOR(OSQLMessageBox,NULL);

    Construct(rTitle, rMessage, nStyle, eImage);
}

//------------------------------------------------------------------------------
OSQLMessageBox::~OSQLMessageBox()
{
    delete m_pInfoButton;

    DBG_DTOR(OSQLMessageBox,NULL);
}

//--------------------------------------------------------------------------
IMPL_LINK( OSQLMessageBox, ButtonClickHdl, Button *, pButton )
{
    OExceptionChainDialog aDlg( this, m_aNextChainElement );
    aDlg.Execute();
    return 0;
}

//.........................................................................
}   // namespace dbaui
//.........................................................................

