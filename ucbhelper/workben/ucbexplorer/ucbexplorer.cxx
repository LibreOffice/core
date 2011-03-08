/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "precompiled_ucbhelper.hxx"
#include <cppuhelper/weak.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/ucb/XCommandInfo.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#include <com/sun/star/ucb/ContentInfo.hpp>
#include <com/sun/star/ucb/ContentInfoAttribute.hpp>

#include "rtl/ref.hxx"
#include <ucbhelper/content.hxx>
#include <ucbhelper/contentbroker.hxx>
#include <ucbhelper/commandenvironment.hxx>
#include <svtools/svtreebx.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/svapp.hxx>
#include <vcl/menu.hxx>
#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/group.hxx>
#include <vcl/button.hxx>
#include <vcl/msgbox.hxx>
#include <ucbhelper/macros.hxx>
#include <ucbhelper/configurationkeys.hxx>

#include "ucbexplorer.hrc"

using namespace com::sun::star;
using namespace com::sun::star::beans;
using namespace com::sun::star::io;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::task;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;

using ::rtl::OUString;

//=========================================================================
//
// class TestDataSink.
//
//=========================================================================

class TestDataSink : public cppu::OWeakObject, public XActiveDataSink
{
    uno::Reference< XInputStream > m_xStream;

public:
//    TestDataSink() {}
//    virtual ~TestDataSink();

    // XInterface methods
    virtual Any SAL_CALL queryInterface( const Type & rType )
        throw( RuntimeException );
    virtual void SAL_CALL acquire()
        throw ();
    virtual void SAL_CALL release()
        throw ();

    // XActiveDataSink methods.
    virtual void SAL_CALL setInputStream(
                                const uno::Reference< XInputStream >& aStream )
        throw( RuntimeException );
    virtual uno::Reference< XInputStream > SAL_CALL getInputStream()
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
    StringInputDialog( ResMgr& rResMgr,
                       const String& rTitle,
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

    ::ucbhelper::Content m_aContent;
    EntryType            m_eType;

public:
    UcbExplorerListBoxEntry();
    virtual ~UcbExplorerListBoxEntry();

    BOOL createNewContent( const ContentInfo& rInfo,
                           ::ucbhelper::Content& rNewContent );
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
    UcbExplorerTreeListBox( ResMgr & rResMgr, Window* pParent, WinBits nWinStyle = 0 );
    virtual ~UcbExplorerTreeListBox();

    virtual void Command( const CommandEvent& rCEvt );

    UcbExplorerListBoxEntry*
    InsertEntry( ::ucbhelper::Content& rContent, SvLBoxEntry* pParent );
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
    UcbExplorerWindow( ResMgr & rResMgr, Window *pParent, WinBits nWinStyle );
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
    throw()
{
    OWeakObject::acquire();
}

//=========================================================================
// virtual
void SAL_CALL TestDataSink::release()
    throw()
{
    OWeakObject::release();
}

//=========================================================================
// virtual
void SAL_CALL TestDataSink::setInputStream(
                                const uno::Reference< XInputStream >& aStream )
    throw( RuntimeException )
{
    m_xStream = aStream;
}

//=========================================================================
// virtual
uno::Reference< XInputStream > SAL_CALL TestDataSink::getInputStream()
    throw( RuntimeException )
{
    return m_xStream;
}

//=========================================================================
//
// StringInputDialog implementation.
//
//=========================================================================

StringInputDialog::StringInputDialog( ResMgr& rResMgr,
                                      const String& rTitle,
                                      const String& rDefaultText,
                                      String* pGroupName )
  : ModalDialog( 0, ResId( DLG_STRINGINPUT, rResMgr ) ),
  m_aNameText ( this, ResId( FT_STRINGINPUT_DLG_NAME, rResMgr ) ),
  m_aNameEdit ( this, ResId( ED_STRINGINPUT_DLG_NAME, rResMgr ) ),
  m_aNameGroup( this, ResId( GB_STRINGINPUT_DLG_NAME, rResMgr ) ),
  m_aOKBtn    ( this, ResId( BT_STRINGINPUT_DLG_OK, rResMgr ) ),
  m_aCancelBtn( this, ResId( BT_STRINGINPUT_DLG_CANCEL, rResMgr ) ),
  m_aHelpBtn  ( this, ResId( BT_STRINGINPUT_DLG_HELP, rResMgr ) )
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
/*
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
*/
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
                                                ::ucbhelper::Content& rNewContent )
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

            std::auto_ptr< ResMgr > xManager(
                ResMgr::CreateResMgr( CREATEVERSIONRESMGR_NAME( ucbexplorer ) ) );
            StringInputDialog* pDlg = new StringInputDialog( *xManager.get(), rName, rName );
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

    uno::Reference< XInputStream > xData;

    if ( rInfo.Attributes & ContentInfoAttribute::INSERT_WITH_INPUTSTREAM )
    {
        // Let the user specify the URL of a content containing the
        // data to supply to the new content.

        std::auto_ptr< ResMgr > xManager(
            ResMgr::CreateResMgr( CREATEVERSIONRESMGR_NAME( ucbexplorer ) ) );
        StringInputDialog* pDlg = new StringInputDialog(
                                            *xManager.get(),
                                            OUString(RTL_CONSTASCII_USTRINGPARAM(
                                                "Document Data Source URL" )),
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
                uno::Reference< XCommandEnvironment > xEnv;

                ::ucbhelper::ContentBroker* pBroker = ::ucbhelper::ContentBroker::get();
                if ( pBroker )
                {
                    uno::Reference< XInteractionHandler > xInteractionHandler(
                        pBroker->getServiceManager()->createInstance(
                                OUString(RTL_CONSTASCII_USTRINGPARAM(
                                    "com.sun.star.task.InteractionHandler" )) ),
                        UNO_QUERY );

                    uno::Reference< XProgressHandler > xProgressHandler
                                    /* = new ProgressHandler( *pBroker ) */ ;

                    xEnv = new ::ucbhelper::CommandEnvironment( xInteractionHandler,
                                                                xProgressHandler );
                }

                ::ucbhelper::Content aSourceContent( aSourceURL, xEnv );

                // Get source data.
                rtl::Reference< TestDataSink > xSourceData = new TestDataSink;
                aSourceContent.openStream( xSourceData.get() );
                xData = xSourceData->getInputStream();
            }
            catch ( ContentCreationException const & )
            {
                DBG_ERROR( "UcbExplorerListBoxEntry::createNewContent - "
                               "No content for document data!" );
                return FALSE;
            }
            catch ( CommandAbortedException const & )
            {
                return FALSE;
            }
            catch ( RuntimeException const & )
            {
                throw;
            }
            catch ( Exception const & )
            {
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
    catch ( CommandAbortedException const & )
    {
    }
    catch ( RuntimeException const & )
    {
        throw;
    }
    catch ( Exception const & )
    {
    }

    Application::AcquireSolarMutex( n );
    return bRet;
}

//=========================================================================
//
// UcbExplorerTreeListBox implementation.
//
//=========================================================================

UcbExplorerTreeListBox::UcbExplorerTreeListBox( ResMgr & rResMgr,
                                                Window* pParent,
                                                WinBits nWinStyle )
: SvTreeListBox( pParent, nWinStyle ),
  m_aFolderClosed( ResId( BMP_FOLDER_CLOSED, rResMgr ) ),
  m_aFolderOpened( ResId( BMP_FOLDER_OPENED, rResMgr ) ),
  m_aDocument( ResId( BMP_DOCUMENT, rResMgr ) ),
  m_aLink( ResId( BMP_LINK, rResMgr ) )
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
//                    OUString* pNames = aPropertyNames.getArray();
//                    pNames[ 0 ] = OUString(RTL_CONSTASCII_USTRINGPARAM("Title"));

                    uno::Reference< XResultSet > xResultSet
                        = pEntry->m_aContent.createCursor(
                                    aPropertyNames,
                                    ::ucbhelper::INCLUDE_FOLDERS_AND_DOCUMENTS );
                    uno::Reference< XContentAccess > xContentAccess(
                                                    xResultSet, UNO_QUERY );

                    if ( xResultSet.is() && xContentAccess.is() )
                    {
                        while ( xResultSet->next() )
                        {
                            Application::AcquireSolarMutex( n );

                            InsertEntry(
                                xContentAccess->queryContentIdentifierString(),
                                pParent );

                            n = Application::ReleaseSolarMutex();
                        }
                    }
                }
                catch ( CommandAbortedException const & )
                {
                }
                catch ( RuntimeException const & )
                {
                    throw;
                }
                catch ( Exception const & )
                {
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
           std::auto_ptr< ResMgr > xManager(
                ResMgr::CreateResMgr( CREATEVERSIONRESMGR_NAME( ucbexplorer ) ) );
           PopupMenu* pMenu = new PopupMenu( ResId( MENU_POPUP, *xManager.get() ) );
            PopupMenu* pNewMenu = 0;

//            pMenu->SetSelectHdl( LINK( this,
//                                       SfxCommonTemplateDialog_Impl,
//                                       MenuSelectHdl ) );

            //////////////////////////////////////////////////////////////
            // Configure "New"
            //////////////////////////////////////////////////////////////

            Sequence< ContentInfo > aInfo
                = pEntry->m_aContent.queryCreatableContentsInfo();
            const ContentInfo* pInfo = aInfo.getConstArray();
            sal_Int32 nCount = aInfo.getLength();
            BOOL bCanCreate =  ( nCount > 0 );

            pNewMenu = new PopupMenu;
            pMenu->SetPopupMenu( MENU_NEW, pNewMenu );

            for ( sal_Int32 n = 0; n < nCount; ++n )
            {
                const ContentInfo& rInfo = pInfo[ n ];
                pNewMenu->InsertItem( 20000 + n + 1, rInfo.Type );
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
                                        OUString(RTL_CONSTASCII_USTRINGPARAM("Title")) );
                bEnable = !( aProp.Attributes & PropertyAttribute::READONLY );
            }
            catch( UnknownPropertyException const & )
            {
                // getPropertyByName
                bEnable = sal_False;
            }
            catch ( CommandAbortedException const & )
            {
            }
            catch ( RuntimeException const & )
            {
                throw;
            }
            catch ( Exception const & )
            {
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
                                        OUString(RTL_CONSTASCII_USTRINGPARAM(
                                            "delete" )) ) );
            }
            catch ( CommandAbortedException const & )
            {
            }
            catch ( RuntimeException const & )
            {
                throw;
            }
            catch ( Exception const & )
            {
            }

            //////////////////////////////////////////////////////////////
            // Execute menu.
            //////////////////////////////////////////////////////////////

            USHORT nSelected = pMenu->Execute( this, rPos );
            switch ( nSelected )
            {
//                case MENU_NEW:
//                    break;

                case MENU_RENAME:
                {
                    OUString aNewTitle;

                    try
                    {
                        pEntry->m_aContent.getPropertyValue(
                                    OUString(RTL_CONSTASCII_USTRINGPARAM("Title")) )
                                    >>= aNewTitle;
                    }
                    catch ( CommandAbortedException const & )
                    {
                    }
                    catch ( RuntimeException const & )
                    {
                        throw;
                    }
                    catch ( Exception const & )
                    {
                    }

                    std::auto_ptr< ResMgr > xManager(
                        ResMgr::CreateResMgr( CREATEVERSIONRESMGR_NAME( ucbexplorer ) ) );
                    StringInputDialog* pDlg
                            = new StringInputDialog(
                                    *xManager.get(),
                                    OUString(RTL_CONSTASCII_USTRINGPARAM("Title")),
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
                                    OUString(RTL_CONSTASCII_USTRINGPARAM("Title")),
                                    aValue );
                        bOK = sal_True;
                    }
                    catch ( CommandAbortedException const & )
                    {
                    }
                    catch ( RuntimeException const & )
                    {
                        throw;
                    }
                    catch ( Exception const & )
                    {
                    }

                    Application::AcquireSolarMutex( n );

                    if ( bOK )
                    {
                        OUString aText( aNewTitle );
                        OUString aTargetURL;
                        try
                        {
                            pEntry->m_aContent.getPropertyValue(
                                OUString(RTL_CONSTASCII_USTRINGPARAM("TargetURL")) )
                                    >>= aTargetURL;
                        }
                        catch ( CommandAbortedException const & )
                        {
                        }
                        catch ( RuntimeException const & )
                        {
                            throw;
                        }
                        catch ( Exception const & )
                        {
                            // The property is optional!
                        }

                        if ( aTargetURL.getLength() > 0 )
                        {
                            // Insert link.
                            aText += OUString(RTL_CONSTASCII_USTRINGPARAM(" --> "));
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
                                    OUString(RTL_CONSTASCII_USTRINGPARAM("delete")),
                                    makeAny( sal_True ) );
                    }
                    catch ( CommandAbortedException const & )
                    {
                        bOK = sal_False;
                    }
                    catch ( RuntimeException const & )
                    {
                        throw;
                    }
                    catch ( Exception const & )
                    {
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

                        ::ucbhelper::Content aNewContent;
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

            delete pNewMenu;
            delete pMenu;
            return;
        }
    }

    SvTreeListBox::Command( rCEvt );
}

//=========================================================================
UcbExplorerListBoxEntry* UcbExplorerTreeListBox::InsertEntry(
                                                ::ucbhelper::Content& rContent,
                                                SvLBoxEntry* pParent )
{
    try
    {
        OUString aTitle;
        rContent.getPropertyValue(
                        OUString(RTL_CONSTASCII_USTRINGPARAM("Title")) ) >>= aTitle;
        if ( !aTitle.getLength() )
            aTitle = OUString(RTL_CONSTASCII_USTRINGPARAM("/"));

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
                    OUString(RTL_CONSTASCII_USTRINGPARAM("TargetURL")) ) >>= aTargetURL;
            }
            catch ( CommandAbortedException const & )
            {
            }
            catch ( RuntimeException const & )
            {
                throw;
            }
            catch ( Exception const & )
            {
                // The property is optional!
            }

            if ( aTargetURL.getLength() > 0 )
            {
                // Insert link.
                aTitle += OUString(RTL_CONSTASCII_USTRINGPARAM(" --> "));
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
    catch ( CommandAbortedException const & )
    {
    }
    catch ( Exception const & )
    {
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
        uno::Reference< XCommandEnvironment > xEnv;

        ::ucbhelper::ContentBroker* pBroker = ::ucbhelper::ContentBroker::get();
        if ( pBroker )
        {
            uno::Reference< XInteractionHandler > xInteractionHandler(
                pBroker->getServiceManager()->createInstance(
                        OUString(RTL_CONSTASCII_USTRINGPARAM(
                            "com.sun.star.task.InteractionHandler" )) ),
                UNO_QUERY );

            uno::Reference< XProgressHandler > xProgressHandler
                                /* = new ProgressHandler( *pBroker ) */ ;

            xEnv = new ::ucbhelper::CommandEnvironment( xInteractionHandler,
                                                        xProgressHandler );
        }

        ::ucbhelper::Content aContent( rURL, xEnv );
        return InsertEntry( aContent, pParent );
    }
    catch ( ContentCreationException const & )
    {
    }

    return 0;
}

//=========================================================================
//
// UcbExplorerWindow implementation.
//
//=========================================================================

UcbExplorerWindow::UcbExplorerWindow( ResMgr & rResMgr, Window *pParent, WinBits nWinStyle )
: WorkWindow( pParent, nWinStyle ),
  m_aTree( rResMgr, this, WB_HSCROLL )
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

    uno::Reference< XMultiServiceFactory > xFac;
    try
    {
        uno::Reference< XComponentContext > xCtx(
            cppu::defaultBootstrap_InitialComponentContext() );
        if ( !xCtx.is() )
        {
            OSL_FAIL( "Error creating initial component context!" );
            return;
        }

        xFac = uno::Reference< XMultiServiceFactory >(
            xCtx->getServiceManager(), UNO_QUERY );

        if ( !xFac.is() )
        {
            OSL_FAIL( "No service manager!" );
            return;
        }
    }
    catch ( com::sun::star::uno::Exception const & )
    {
        OSL_FAIL( "Exception during creation of initial component context!" );
        return;
    }

    comphelper::setProcessServiceFactory( xFac );

    uno::Reference< XComponent > xComponent( xFac, UNO_QUERY );

    //////////////////////////////////////////////////////////////////////
    // Create UCB.
    //////////////////////////////////////////////////////////////////////

#if 1
    // Init UCB (Read configuration from registry)
    Sequence< Any > aArgs( 2 );
    aArgs[ 0 ] <<= OUString(RTL_CONSTASCII_USTRINGPARAM( UCB_CONFIGURATION_KEY1_LOCAL ));
    aArgs[ 1 ] <<= OUString(RTL_CONSTASCII_USTRINGPARAM( UCB_CONFIGURATION_KEY2_OFFICE ));
    sal_Bool bSuccess = ::ucbhelper::ContentBroker::initialize( xFac, aArgs );
#else
    // Init UCB (Use provided configuration data)
    ::ucbhelper::ContentProviderDataList aProviders;
    aProviders.push_back(
        ::ucbhelper::ContentProviderData(
            OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.ucb.FileContentProvider")),
            OUString(RTL_CONSTASCII_USTRINGPARAM("file")),
            OUString() ) );
    sal_Bool bSuccess = ::ucbhelper::ContentBroker::initialize( xFac, aProviders );
#endif

    if ( !bSuccess )
    {
        OSL_FAIL( "Error creating UCB!" );
        return;
    }

    //////////////////////////////////////////////////////////////////////
    // Create/init/show app window.
    //////////////////////////////////////////////////////////////////////

    std::auto_ptr< ResMgr > xManager(
        ResMgr::CreateResMgr( CREATEVERSIONRESMGR_NAME( ucbexplorer ) ) );

    UcbExplorerWindow aAppWin( *xManager.get(), 0, WB_APP | WB_STDWORK );

    MenuBar aMBMain( ResId( MENU_MAIN, *xManager.get() ) );

    // Check for command line params

     String aRootURL = GetCommandLineParam( 0 );
     if ( aRootURL.Len() == 0 )
        aRootURL = UniString::CreateFromAscii(
                        RTL_CONSTASCII_STRINGPARAM( "vnd.sun.star.hier:/" ) );

     String aTitle( ResId( TEXT_TITLEBAR, *xManager.get() ) );
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

    ::ucbhelper::ContentBroker::deinitialize();

    if ( xComponent.is() )
        xComponent->dispose();
}

//=========================================================================
//
// The Application.
//
//=========================================================================

MyApp aMyApp;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
