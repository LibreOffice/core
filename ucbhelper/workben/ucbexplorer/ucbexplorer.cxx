/*************************************************************************
 *
 *  $RCSfile: ucbexplorer.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:03:38 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _CPPUHELPER_SERVICEFACTORY_HXX_
#include <cppuhelper/servicefactory.hxx>
#endif
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX_
#include <unotools/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSETINFO_HPP_
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASINK_HPP_
#include <com/sun/star/io/XActiveDataSink.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMMANDINFO_HPP_
#include <com/sun/star/ucb/XCommandInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTACCESS_HPP_
#include <com/sun/star/ucb/XContentAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTCREATOR_HPP_
#include <com/sun/star/ucb/XContentCreator.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_COMMANDABORTEDEXCEPTION_HPP_
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_CONTENTINFOATTRIBUTE_HPP_
#include <com/sun/star/ucb/ContentInfoAttribute.hpp>
#endif

#ifndef _VOS_PROCESS_HXX_
#include <vos/process.hxx>
#endif
#ifndef _UCBHELPER_CONTENT_HXX
#include <ucbhelper/content.hxx>
#endif
#ifndef _UCBHELPER_CONTENTBROKER_HXX
#include <ucbhelper/contentbroker.hxx>
#endif
#ifndef _UCBHELPER_COMMANDENVIRONMENT_HXX
#include <ucbhelper/commandenvironment.hxx>
#endif
#ifndef _SVTREEBOX_HXX
#include <svtools/svtreebx.hxx>
#endif
#ifndef _SV_WRKWIN_HXX
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_MENU_HXX
#include <vcl/menu.hxx>
#endif
#ifndef _SV_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_GROUP_HXX
#include <vcl/group.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif

#ifndef _UCBHELPER_MACROS_HXX
#include <ucbhelper/macros.hxx>
#endif

#ifndef _UCBEXPLORER_HRC
#include "ucbexplorer.hrc"
#endif

using namespace com::sun::star::beans;
using namespace com::sun::star::io;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::task;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;
using namespace rtl;
using namespace vos;

//=========================================================================
//
// class TestDataSink.
//
//=========================================================================

class TestDataSink : public cppu::OWeakObject, public XActiveDataSink
{
    Reference< XInputStream > m_xStream;

public:
//  TestDataSink() {}
//  virtual ~TestDataSink();

    // XInterface methods
    virtual Any SAL_CALL queryInterface( const Type & rType )
        throw( RuntimeException );
    virtual void SAL_CALL acquire()
        throw ( RuntimeException );
    virtual void SAL_CALL release()
        throw ( RuntimeException );

    // XActiveDataSink methods.
    virtual void SAL_CALL setInputStream(
                                const Reference< XInputStream >& aStream )
        throw( RuntimeException );
    virtual Reference< XInputStream > SAL_CALL getInputStream()
        throw( RuntimeException );
};

//=========================================================================
//
// class StringInputDialog.
//
//=========================================================================

class StringInputDialog : public ModalDialog
{
private:
    FixedText    m_aNameText;
    Edit         m_aNameEdit;
    GroupBox     m_aNameGroup;
    OKButton     m_aOKBtn;
    CancelButton m_aCancelBtn;
    HelpButton   m_aHelpBtn;

    DECL_LINK( OKHdl, Button * );
    DECL_LINK( NameHdl, Edit * );

public:
    StringInputDialog( const String& rTitle,
                       const String& rDefaultText,
                       String* pGroupName = 0 );
    String GetValue() const { return m_aNameEdit.GetText(); }
    void SetValue( const String& rNewName ) { m_aNameEdit.SetText( rNewName ); }
};

//=========================================================================
//
// class UcbExplorerListBoxEntry.
//
//=========================================================================

class UcbExplorerListBoxEntry : public SvLBoxEntry
{
    friend class UcbExplorerTreeListBox;

    enum EntryType { FOLDER, DOCUMENT, LINK };

    ::ucb::Content m_aContent;
    EntryType      m_eType;

public:
    UcbExplorerListBoxEntry();
    virtual ~UcbExplorerListBoxEntry();

    BOOL createNewContent( const ContentInfo& rInfo,
                           ::ucb::Content& rNewContent );
};

//=========================================================================
//
// class UcbExplorerTreeListBox.
//
//=========================================================================

class UcbExplorerTreeListBox : public SvTreeListBox
{
    Bitmap  m_aFolderClosed;
    Bitmap  m_aFolderOpened;
    Bitmap  m_aDocument;
    Bitmap  m_aLink;

private:
    virtual SvLBoxEntry* CreateEntry() const;
    virtual void         RequestingChilds( SvLBoxEntry* pParent );

public:
    UcbExplorerTreeListBox( Window* pParent, WinBits nWinStyle = 0 );
    virtual ~UcbExplorerTreeListBox();

    virtual void Command( const CommandEvent& rCEvt );

    UcbExplorerListBoxEntry*
    InsertEntry( ::ucb::Content& rContent, SvLBoxEntry* pParent );
    UcbExplorerListBoxEntry*
    InsertEntry( const String& rURL, SvLBoxEntry* pParent = 0 );
};

//=========================================================================
//
// class UcbExplorerWindow.
//
//=========================================================================

class UcbExplorerWindow : public WorkWindow
{
    friend class MyApp;

    UcbExplorerTreeListBox m_aTree;

public:
    UcbExplorerWindow( Window *pParent, WinBits nWinStyle );
    virtual ~UcbExplorerWindow();

    virtual void Resize();
};

//=========================================================================
//
// class MyApp.
//
//=========================================================================

class MyApp : public Application
{
public:
    virtual void Main();
};

//=========================================================================
//
// TestDataSink implementation.
//
//=========================================================================

// virtual
Any SAL_CALL TestDataSink::queryInterface( const Type & rType )
    throw( RuntimeException )
{
    Any aRet = cppu::queryInterface(
                        rType,
                           static_cast< XActiveDataSink * >( this ) );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}

//=========================================================================
// virtual
void SAL_CALL TestDataSink::acquire()
    throw( RuntimeException )
{
    OWeakObject::acquire();
}

//=========================================================================
// virtual
void SAL_CALL TestDataSink::release()
    throw( RuntimeException )
{
    OWeakObject::release();
}

//=========================================================================
// virtual
void SAL_CALL TestDataSink::setInputStream(
                                const Reference< XInputStream >& aStream )
    throw( RuntimeException )
{
    m_xStream = aStream;
}

//=========================================================================
// virtual
Reference< XInputStream > SAL_CALL TestDataSink::getInputStream()
    throw( RuntimeException )
{
    return m_xStream;
}

//=========================================================================
//
// StringInputDialog implementation.
//
//=========================================================================

StringInputDialog::StringInputDialog( const String& rTitle,
                                      const String& rDefaultText,
                                      String* pGroupName )
: ModalDialog( GetpApp()->GetDefModalDialogParent(), ResId( DLG_STRINGINPUT ) ),
  m_aNameText ( this, ResId( FT_STRINGINPUT_DLG_NAME ) ),
  m_aNameEdit ( this, ResId( ED_STRINGINPUT_DLG_NAME ) ),
  m_aNameGroup( this, ResId( GB_STRINGINPUT_DLG_NAME ) ),
  m_aOKBtn    ( this, ResId( BT_STRINGINPUT_DLG_OK ) ),
  m_aCancelBtn( this, ResId( BT_STRINGINPUT_DLG_CANCEL ) ),
  m_aHelpBtn  ( this, ResId( BT_STRINGINPUT_DLG_HELP ) )
{
    FreeResource();
    SetText( rTitle );
    m_aNameEdit.SetText( rDefaultText );
    m_aOKBtn.SetClickHdl( LINK( this, StringInputDialog, OKHdl ) );
    m_aNameEdit.SetModifyHdl( LINK( this, StringInputDialog, NameHdl ) );

    if ( pGroupName )
        m_aNameGroup.SetText( *pGroupName );
};

//=========================================================================
IMPL_LINK( StringInputDialog, OKHdl, Button *, EMPTYARG )
{
    // trim the strings
    m_aNameEdit.SetText(
        m_aNameEdit.GetText().EraseLeadingChars().EraseTrailingChars() );
    EndDialog( RET_OK );
    return 1;
}

//=========================================================================
IMPL_LINK( StringInputDialog, NameHdl, Edit *, EMPTYARG )
{
    // trim the strings
    String aName = m_aNameEdit.GetText();
    aName.EraseLeadingChars().EraseTrailingChars();
    if ( aName.Len() )
    {
        if ( !m_aOKBtn.IsEnabled() )
            m_aOKBtn.Enable( TRUE );
    }
    else
    {
        if ( m_aOKBtn.IsEnabled() )
            m_aOKBtn.Enable( FALSE );
    }
    return 0;
}

//=========================================================================
//
// UcbExplorerListBoxEntry implementation.
//
//=========================================================================

UcbExplorerListBoxEntry::UcbExplorerListBoxEntry()
{
}

//=========================================================================
// virtual
UcbExplorerListBoxEntry::~UcbExplorerListBoxEntry()
{
}

//=========================================================================
BOOL UcbExplorerListBoxEntry::createNewContent( const ContentInfo& rInfo,
                                                ::ucb::Content& rNewContent )
{
    sal_Int32 nCount = rInfo.Properties.getLength();
    Sequence< Any > aPropValues( nCount );
    Sequence< OUString > aPropNames( nCount );

    if ( nCount > 0 )
    {
        // Collect property values.

        Any* pValues = aPropValues.getArray();
        const Property* pProps = rInfo.Properties.getConstArray();
        for ( sal_Int32 n = 0; n < nCount; ++n )
        {
            const OUString& rName = pProps[ n ].Name;
            StringInputDialog* pDlg = new StringInputDialog( rName, rName );
            USHORT nRet = pDlg->Execute();
            if ( nRet == RET_OK )
            {
                OUString aValue( pDlg->GetValue() );

                // Convert value according to supplied type info...

                const Type& rType = pProps[ n ].Type;

                if ( rType == getCppuType(
                                    static_cast< const OUString * >( 0 ) ) )
                {
                    // string
                    pValues[ n ] <<= aValue;
                }
                else if ( rType == getCppuType(
                                    static_cast< const sal_Int32 * >( 0 ) ) )
                {
                    // long
                    pValues[ n ] <<= aValue.toInt32();
                }
                else if ( rType == getCppuType(
                                    static_cast< const sal_Int16 * >( 0 ) ) )
                {
                    // short
                    pValues[ n ] <<= sal_Int16( aValue.toInt32() ) ;
                }
                else if ( rType == getCppuBooleanType() )
                {
                    // boolean
                    pValues[ n ] <<= sal_Bool( aValue.toChar() ) ;
                }
                else if ( rType == getCppuCharType() )
                {
                    // char
                    pValues[ n ] <<= aValue.toChar();
                }
                else if ( rType == getCppuType(
                                    static_cast< const sal_Int8 * >( 0 ) ) )
                {
                    // byte
                    pValues[ n ] <<= sal_Int8( aValue.toChar() ) ;
                }
                else if ( rType == getCppuType(
                                    static_cast< const sal_Int64 * >( 0 ) ) )
                {
                    // hyper
                    pValues[ n ] <<= aValue.toInt64();
                }
                else if ( rType == getCppuType(
                                    static_cast< const float * >( 0 ) ) )
                {
                    // float
                    pValues[ n ] <<= aValue.toFloat();
                }
                else if ( rType == getCppuType(
                                    static_cast< const double * >( 0 ) ) )
                {
                    // double
                    pValues[ n ] <<= aValue.toDouble();
                }
                else
                {
                    // See com/sun/star/ucb/ContentInfo.idl
                    DBG_ERROR( "UcbExplorerListBoxEntry::createNewContent - "
                               "Invlid type for bootstrap property!" );
                }
            }

            delete pDlg;

            if ( nRet != RET_OK )
                return FALSE;

            aPropNames[ n ] = rName;
        }
    }

    Reference< XInputStream > xData;

    if ( rInfo.Attributes & ContentInfoAttribute::INSERT_WITH_INPUTSTREAM )
    {
        // Let the user specify the URL of a content containing the
        // data to supply to the new content.

        StringInputDialog* pDlg = new StringInputDialog(
                                            OUString::createFromAscii(
                                                "Document Data Source URL" ),
                                            OUString() );
        USHORT nRet = pDlg->Execute();
        if ( nRet == RET_OK )
        {
            // Create a content object for the given URL.

            OUString aSourceURL( pDlg->GetValue() );
            if ( aSourceURL.getLength() == 0 )
            {
                DBG_ERROR( "UcbExplorerListBoxEntry::createNewContent - "
                              "No document data URL!" );
                return FALSE;
            }

            try
            {
                Reference< XCommandEnvironment > xEnv;

                ::ucb::ContentBroker* pBroker = ::ucb::ContentBroker::get();
                if ( pBroker )
                {
                    Reference< XInteractionHandler > xInteractionHandler(
                         pBroker->getServiceManager()->createInstance(
                                OUString::createFromAscii(
                                    "com.sun.star.uui.InteractionHandler" ) ),
                        UNO_QUERY );

                    Reference< XProgressHandler > xProgressHandler
                                    /* = new ProgressHandler( *pBroker ) */ ;

                    xEnv = new ::ucb::CommandEnvironment( xInteractionHandler,
                                                               xProgressHandler );
                }

                ::ucb::Content aSourceContent( aSourceURL, xEnv );

                // Get source data.
                vos::ORef< TestDataSink > xSourceData = new TestDataSink;
                aSourceContent.openStream( xSourceData.getBodyPtr() );
                xData = xSourceData->getInputStream();
            }
            catch ( ::ucb::ContentCreationException& )
            {
                DBG_ERROR( "UcbExplorerListBoxEntry::createNewContent - "
                              "No content for document data!" );
                return FALSE;
            }
            catch ( CommandAbortedException& )
            {
                DBG_ERROR( "UcbExplorerListBoxEntry::createNewContent - "
                              "CommandAbortedException!" );
                return FALSE;
            }
            catch ( RuntimeException& )
            {
                throw;
            }
            catch ( Exception& )
            {
                DBG_ERROR( "UcbExplorerListBoxEntry::createNewContent - "
                              "Exception!" );
                return FALSE;
            }
        }

        delete pDlg;

        if ( nRet != RET_OK )
            return FALSE;
    }

    // Create new content.

       ULONG n = Application::ReleaseSolarMutex();
    BOOL bRet = sal_False;

    try
    {
        bRet = m_aContent.insertNewContent(
                    rInfo.Type, aPropNames, aPropValues, xData, rNewContent );
    }
    catch ( CommandAbortedException& )
    {
        DBG_ERROR( "UcbExplorerListBoxEntry::createNewContent - "
                   "CommandAbortedException!" );
    }
    catch ( RuntimeException& )
    {
        throw;
    }
    catch ( Exception& )
    {
        DBG_ERROR( "UcbExplorerListBoxEntry::createNewContent - "
                   "Exception!" );
    }

    Application::AcquireSolarMutex( n );
    return bRet;
}

//=========================================================================
//
// UcbExplorerTreeListBox implementation.
//
//=========================================================================

UcbExplorerTreeListBox::UcbExplorerTreeListBox(
                                    Window* pParent, WinBits nWinStyle )
: SvTreeListBox( pParent, nWinStyle ),
  m_aFolderClosed( ResId( BMP_FOLDER_CLOSED ) ),
  m_aFolderOpened( ResId( BMP_FOLDER_OPENED ) ),
  m_aDocument( ResId( BMP_DOCUMENT ) ),
  m_aLink( ResId( BMP_LINK ) )
{
}

//=========================================================================
// virtual
UcbExplorerTreeListBox::~UcbExplorerTreeListBox()
{
}

//=========================================================================
// virtual
SvLBoxEntry* UcbExplorerTreeListBox::CreateEntry() const
{
    return new UcbExplorerListBoxEntry();
}

//=========================================================================
// virtual
void UcbExplorerTreeListBox::RequestingChilds( SvLBoxEntry* pParent )
{
    UcbExplorerListBoxEntry* pEntry
        = static_cast< UcbExplorerListBoxEntry * >( pParent );
    if ( !pEntry->HasChilds() )
    {
        switch ( pEntry->m_eType )
        {
            case UcbExplorerListBoxEntry::FOLDER:
            {
                   ULONG n = Application::ReleaseSolarMutex();

                try
                {
                    Sequence< OUString > aPropertyNames( 0 );
//                  OUString* pNames = aPropertyNames.getArray();
//                  pNames[ 0 ] = OUString::createFromAscii( "Title" );

                    Reference< XResultSet > xResultSet
                        = pEntry->m_aContent.createCursor(
                                    aPropertyNames,
                                    ::ucb::INCLUDE_FOLDERS_AND_DOCUMENTS );
                    Reference< XContentAccess > xContentAccess(
                                                    xResultSet, UNO_QUERY );

                    if ( xResultSet.is() && xContentAccess.is() )
                    {
                        while ( xResultSet->next() )
                        {
                            Application::AcquireSolarMutex( n );

                            InsertEntry(
                                xContentAccess->queryContentIdentfierString(),
                                pParent );

                            n = Application::ReleaseSolarMutex();
                        }
                    }
                }
                catch ( CommandAbortedException& )
                {
                    DBG_ERROR(
                        "UcbExplorerTreeListBox::RequestingChilds - "
                        "CommandAbortedException!" );
                }
                catch ( RuntimeException& )
                {
                    throw;
                }
                catch ( Exception& )
                {
                    DBG_ERROR(
                        "UcbExplorerTreeListBox::RequestingChilds - "
                        "Exception!" );
                }
                Application::AcquireSolarMutex( n );
                break;
            }

            case UcbExplorerListBoxEntry::DOCUMENT:
                break;

            case UcbExplorerListBoxEntry::LINK:
                break;

            default:
                break;
        }
    }
}

//=========================================================================
// virtual
void UcbExplorerTreeListBox::Command( const CommandEvent& rCEvt )
{
    if ( rCEvt.GetCommand() == COMMAND_CONTEXTMENU )
    {
        const Point& rPos = rCEvt.GetMousePosPixel();
        UcbExplorerListBoxEntry* pEntry
            = static_cast< UcbExplorerListBoxEntry * >(
                GetEntry( rPos, TRUE ) );
        if ( pEntry )
        {
            PopupMenu* pMenu = new PopupMenu( ResId( MENU_POPUP ) );
            PopupMenu* pNewMenu = 0;

//          pMenu->SetSelectHdl( LINK( this,
//                                     SfxCommonTemplateDialog_Impl,
//                                     MenuSelectHdl ) );

            //////////////////////////////////////////////////////////////
            // Configure "New"
            //////////////////////////////////////////////////////////////

            Reference< XContentCreator > xCreator(
                                    pEntry->m_aContent.get(), UNO_QUERY );
            Sequence< ContentInfo > aInfo;
            BOOL bCanCreate = xCreator.is();
            if ( bCanCreate )
            {
                aInfo = xCreator->queryCreatableContentsInfo();
                const ContentInfo* pInfo = aInfo.getConstArray();
                sal_Int32 nCount = aInfo.getLength();
                bCanCreate =  ( nCount > 0 );

                pNewMenu = new PopupMenu;
                pMenu->SetPopupMenu( MENU_NEW, pNewMenu );

                for ( sal_Int32 n = 0; n < nCount; ++n )
                {
                    const ContentInfo& rInfo = pInfo[ n ];
                    pNewMenu->InsertItem( 20000 + n + 1, rInfo.Type );
                }
            }

            pMenu->EnableItem( MENU_NEW, bCanCreate );

            //////////////////////////////////////////////////////////////
            // Configure "Rename"
            //////////////////////////////////////////////////////////////

            sal_Bool bEnable = sal_False;

            try
            {
                Property aProp =
                    pEntry->m_aContent.getProperties()->getPropertyByName(
                                        OUString::createFromAscii( "Title" ) );
                bEnable = !( aProp.Attributes & PropertyAttribute::READONLY );
            }
            catch( UnknownPropertyException& )
            {
                // getPropertyByName
                bEnable = sal_False;
            }
            catch ( CommandAbortedException& )
            {
                DBG_ERROR( "UcbExplorerTreeListBox::Command - "
                           "CommandAbortedException!" );
            }
            catch ( RuntimeException& )
            {
                throw;
            }
            catch ( Exception& )
            {
                DBG_ERROR( "UcbExplorerTreeListBox::Command - "
                           "Exception!" );
            }

            pMenu->EnableItem( MENU_RENAME, bEnable );

            //////////////////////////////////////////////////////////////
            // Configure "Delete"
            //////////////////////////////////////////////////////////////

            try
            {
                pMenu->EnableItem( MENU_DELETE,
                                   pEntry->m_aContent
                                       .getCommands()->hasCommandByName(
                                        OUString::createFromAscii(
                                            "delete" ) ) );
            }
            catch ( CommandAbortedException& )
            {
                DBG_ERROR( "UcbExplorerTreeListBox::Command - "
                           "CommandAbortedException!" );
            }
            catch ( RuntimeException& )
            {
                throw;
            }
            catch ( Exception& )
            {
                DBG_ERROR( "UcbExplorerTreeListBox::Command - "
                           "Exception!" );
            }

            //////////////////////////////////////////////////////////////
            // Execute menu.
            //////////////////////////////////////////////////////////////

            USHORT nSelected = pMenu->Execute( this, rPos );
            switch ( nSelected )
            {
//              case MENU_NEW:
//                  break;

                case MENU_RENAME:
                {
                    OUString aNewTitle;

                    try
                    {
                        pEntry->m_aContent.getPropertyValue(
                                    OUString::createFromAscii( "Title" ) )
                                    >>= aNewTitle;
                    }
                    catch ( CommandAbortedException& )
                    {
                        DBG_ERROR( "UcbExplorerTreeListBox::Command - "
                                      "CommandAbortedException!" );
                    }
                    catch ( RuntimeException& )
                    {
                        throw;
                    }
                    catch ( Exception& )
                    {
                        DBG_ERROR( "UcbExplorerTreeListBox::Command - "
                                      "Exception!" );
                    }

                    StringInputDialog* pDlg
                            = new StringInputDialog(
                                    OUString::createFromAscii( "Title" ),
                                    aNewTitle );

                    USHORT nRet = pDlg->Execute();

                    Any aValue;

                    if ( nRet == RET_OK )
                    {
                        aNewTitle = pDlg->GetValue();
                        aValue <<= aNewTitle;
                    }

                    delete pDlg;

                    if ( nRet != RET_OK )
                        break;

                    sal_Bool bOK = sal_False;

                    ULONG n = Application::ReleaseSolarMutex();

                    try
                    {
                        pEntry->m_aContent.setPropertyValue(
                                    OUString::createFromAscii( "Title" ),
                                    aValue );
                        bOK = sal_True;
                    }
                    catch ( CommandAbortedException& )
                    {
                        DBG_ERROR( "UcbExplorerTreeListBox::Command - "
                                      "CommandAbortedException!" );
                    }
                    catch ( RuntimeException& )
                    {
                        throw;
                    }
                    catch ( Exception& )
                    {
                        DBG_ERROR( "UcbExplorerTreeListBox::Command - "
                                      "Exception!" );
                    }

                    Application::AcquireSolarMutex( n );

                    if ( bOK )
                    {
                        OUString aText( aNewTitle );
                        OUString aTargetURL;
                        try
                        {
                            pEntry->m_aContent.getPropertyValue(
                                OUString::createFromAscii( "TargetURL" ) )
                                    >>= aTargetURL;
                        }
                        catch ( CommandAbortedException& )
                        {
                            // The property is optional!
                        }

                        if ( aTargetURL.getLength() > 0 )
                        {
                            // Insert link.
                            aText += OUString::createFromAscii( " --> " );
                            aText += aTargetURL;
                        }

                        SetEntryText( pEntry, aText );
                    }
                    break;
                }

                case MENU_DELETE:
                {
                    ULONG n = Application::ReleaseSolarMutex();
                    sal_Bool bOK = sal_True;

                    try
                    {
                        pEntry->m_aContent.executeCommand(
                                    OUString::createFromAscii( "delete" ),
                                    makeAny( sal_True ) );
                    }
                    catch ( CommandAbortedException& )
                    {
                        DBG_ERROR(
                            "UcbExplorerTreeListBox::Command - "
                            "CommandAbortedException!" );
                        bOK = sal_False;
                    }
                    catch ( RuntimeException& )
                    {
                        throw;
                    }
                    catch ( Exception& )
                    {
                        DBG_ERROR(
                            "UcbExplorerTreeListBox::Command - "
                            "Exception!" );
                        bOK = sal_False;
                    }

                    Application::AcquireSolarMutex( n );

                    if ( bOK )
                        RemoveSelection();

                    break;
                }

                default:
                {
                    if ( ( nSelected > 20000 ) &&
                         ( ( nSelected - 20000 ) <= aInfo.getLength() ) )
                    {
                        // New-menu entry selected.

                           ::ucb::Content aNewContent;
                        if ( pEntry->createNewContent(
                                    aInfo.getConstArray()[ nSelected - 20001 ],
                                    aNewContent ) )
                        {
                            if ( !IsExpanded( pEntry ) )
                                Expand( pEntry );
                            else
                                InsertEntry( aNewContent, pEntry );
                        }
                    }
                    break;
                }
            }

            delete pMenu;
            delete pNewMenu;
            return;
        }
    }

    SvTreeListBox::Command( rCEvt );
}

//=========================================================================
UcbExplorerListBoxEntry* UcbExplorerTreeListBox::InsertEntry(
                                              ::ucb::Content& rContent,
                                                 SvLBoxEntry* pParent )
{
    try
    {
        OUString aTitle;
        rContent.getPropertyValue(
                        OUString::createFromAscii( "Title" ) ) >>= aTitle;
        if ( !aTitle.getLength() )
            aTitle = OUString::createFromAscii( "/" );

           UcbExplorerListBoxEntry* pEntry = 0;

        if ( rContent.isFolder() )
        {
            // Insert folder.
            pEntry = static_cast< UcbExplorerListBoxEntry * >(
                            SvTreeListBox::InsertEntry( aTitle,
                                                    m_aFolderOpened,
                                                    m_aFolderClosed,
                                                    pParent,
                                                    TRUE ) );
            pEntry->m_eType = UcbExplorerListBoxEntry::FOLDER;
        }
        else
        {
            OUString aTargetURL;
            try
            {
                rContent.getPropertyValue(
                    OUString::createFromAscii( "TargetURL" ) ) >>= aTargetURL;
            }
            catch ( CommandAbortedException& )
            {
                // The property is optional!
            }

            if ( aTargetURL.getLength() > 0 )
            {
                // Insert link.
                aTitle += OUString::createFromAscii( " --> " );
                aTitle += aTargetURL;
                pEntry = static_cast< UcbExplorerListBoxEntry * >(
                                SvTreeListBox::InsertEntry( aTitle,
                                                        m_aLink,
                                                        m_aLink,
                                                        pParent,
                                                        TRUE ) );
                pEntry->m_eType = UcbExplorerListBoxEntry::LINK;
            }
            else
            {
                // Insert Document
                pEntry = static_cast< UcbExplorerListBoxEntry * >(
                                SvTreeListBox::InsertEntry( aTitle,
                                                        m_aDocument,
                                                        m_aDocument,
                                                        pParent,
                                                        TRUE ) );
                pEntry->m_eType = UcbExplorerListBoxEntry::DOCUMENT;
            }
        }

        pEntry->m_aContent = rContent;
        return pEntry;
    }
    catch ( ::com::sun::star::ucb::CommandAbortedException& )
    {
        DBG_ERROR(
            "UcbExplorerTreeListBox::InsertEntry - CommandAbortedException!" );
    }
    catch ( Exception& )
    {
        DBG_ERROR(
            "UcbExplorerTreeListBox::InsertEntry - Exception!" );
    }

    return 0;
}

//=========================================================================
UcbExplorerListBoxEntry* UcbExplorerTreeListBox::InsertEntry(
                                                  const String& rURL,
                                                     SvLBoxEntry* pParent )
{
    try
    {
        Reference< XCommandEnvironment > xEnv;

        ::ucb::ContentBroker* pBroker = ::ucb::ContentBroker::get();
        if ( pBroker )
        {
            Reference< XInteractionHandler > xInteractionHandler(
                 pBroker->getServiceManager()->createInstance(
                        OUString::createFromAscii(
                            "com.sun.star.uui.InteractionHandler" ) ),
                UNO_QUERY );

            Reference< XProgressHandler > xProgressHandler
                                /* = new ProgressHandler( *pBroker ) */ ;

            xEnv = new ::ucb::CommandEnvironment( xInteractionHandler,
                                                     xProgressHandler );
        }

        ::ucb::Content aContent( rURL, xEnv );
        return InsertEntry( aContent, pParent );
    }
    catch ( ::ucb::ContentCreationException& )
    {
        DBG_ERROR(
            "UcbExplorerTreeListBox::InsertEntry - ContentCreationException!" );
    }

    return 0;
}

//=========================================================================
//
// UcbExplorerWindow implementation.
//
//=========================================================================

UcbExplorerWindow::UcbExplorerWindow( Window *pParent, WinBits nWinStyle )
: WorkWindow( pParent, nWinStyle ),
  m_aTree( this, WB_HSCROLL )
{
    Font aTreeFont( m_aTree.GetFont() );
    aTreeFont.SetName( String( RTL_CONSTASCII_USTRINGPARAM("Courier") ) );
    aTreeFont.SetFamily( FAMILY_MODERN );
    aTreeFont.SetPitch( PITCH_FIXED );
    aTreeFont.SetSize( Size( 0, 12 ) );

    m_aTree.SetFont( aTreeFont );
    m_aTree.SetIndent( 20 );
    m_aTree.SetPosPixel( Point( 0, 0 ) );

    m_aTree.Show();
}

//-------------------------------------------------------------------------
// virtual
UcbExplorerWindow::~UcbExplorerWindow()
{
}

//-------------------------------------------------------------------------
// virtual
void UcbExplorerWindow::Resize()
{
    m_aTree.SetSizePixel( GetOutputSizePixel() );
}

//=========================================================================
//
// MyApp implementation.
//
//=========================================================================

// virtual
void MyApp::Main()
{
    //////////////////////////////////////////////////////////////////////
    // Initialize local Service Manager and basic services.
    //////////////////////////////////////////////////////////////////////

    OStartupInfo aInfo;
    OUString aExeName;
    if ( aInfo.getExecutableFile( aExeName )
                            != OStartupInfo::E_None )
    {
        DBG_ERROR( "Error getting Executable file name!" );
        return;
    }

    OUString aReadOnlyRegFile
        = aExeName.copy( 0, aExeName.lastIndexOf( '/' ) + 1 );
    OUString aWritableRegFile
        = aReadOnlyRegFile;
    aReadOnlyRegFile += OUString::createFromAscii( "applicat.rdb" );
    aWritableRegFile += OUString::createFromAscii( "ucbexplorer.rdb" );

    Reference< XMultiServiceFactory > xFac;
    try
    {
        xFac = cppu::createRegistryServiceFactory(
                                    aWritableRegFile, aReadOnlyRegFile );
    }
    catch ( com::sun::star::uno::Exception )
    {
        DBG_ERROR( "Error creating RegistryServiceFactory!" );
        return;
    }

    utl::setProcessServiceFactory( xFac );

    Reference< XComponent > xComponent( xFac, UNO_QUERY );

    //////////////////////////////////////////////////////////////////////
    // Create UCB.
    //////////////////////////////////////////////////////////////////////

    Sequence< Any > aArgs(1);
    aArgs[0] <<= sal_True;
    sal_Bool bSuccess = ::ucb::ContentBroker::initialize( xFac, aArgs );

    if ( !bSuccess )
    {
        DBG_ERROR( "Error creating UCB!" );
        return;
    }

    //////////////////////////////////////////////////////////////////////
    // Create/init/show app window.
    //////////////////////////////////////////////////////////////////////

    ResMgr* pMgr = ResMgr::CreateResMgr(
                                CREATEVERSIONRESMGR_NAME( ucbexplorer ) );
    Resource::SetResManager( pMgr );

    UcbExplorerWindow aAppWin( 0, WB_APP | WB_STDWORK );

    MenuBar aMBMain( ResId( MENU_MAIN ) );

    // Check for command line params
#if 0
    for ( int i = 0; i < GetCommandLineParamCount(); ++i )
    {
        String aPara = GetCommandLineParam( i );
    }
#endif

     String aRootURL = GetCommandLineParam( 0 );
     if ( aRootURL.Len() == 0 )
        aRootURL = UniString::CreateFromAscii(
                        RTL_CONSTASCII_STRINGPARAM( "vnd.sun.star.hier:/" ) );

    String aTitle( ResId( TEXT_TITLEBAR ) );
    aTitle.AppendAscii( RTL_CONSTASCII_STRINGPARAM( " - " ) );
    aTitle += aRootURL;

    aAppWin.SetText( aTitle );

    aAppWin.SetPosSizePixel( 0, 0, 500, 750 );

    aAppWin.Show();

    aAppWin.m_aTree.InsertEntry( aRootURL );

    //////////////////////////////////////////////////////////////////////
    // Execute app.
    //////////////////////////////////////////////////////////////////////

    Execute();

    //////////////////////////////////////////////////////////////////////
    // Cleanup.
    //////////////////////////////////////////////////////////////////////

    // m_aTree holds UCB contents!
    aAppWin.m_aTree.Clear();

    ::ucb::ContentBroker::deinitialize();

    if ( xComponent.is() )
        xComponent->dispose();
}

//=========================================================================
//
// The Application.
//
//=========================================================================

MyApp aMyApp;

