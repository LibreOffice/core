/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2010 Novell, Inc.
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

#include <kdecommandthread.hxx>
#include <kdefilepicker.hxx>

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qgrid.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qobjectlist.h>
#include <qpushbutton.h>
#include <qvbox.h>

#ifdef QT_NO_EMIT
#define emit
#endif

#include <kdiroperator.h>
#include <kfiledialog.h>
#include <kfilefiltercombo.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ktempfile.h>

#include <algorithm>
#include <iostream>

//////////////////////////////////////////////////////////////////////////
// FileDialog
//////////////////////////////////////////////////////////////////////////

FileDialog::FileDialog( const QString &startDir, const QString &filter,
        QWidget *parent, const char *name )
    : KFileDialog( startDir, filter, parent, name, true, m_pCustomWidget = new QVBox() ),
      m_pCombosAndButtons( new QHBox( m_pCustomWidget ) ),
      m_pLabels( new QVBox( m_pCombosAndButtons ) ),
      m_pComboBoxes( new QVBox( m_pCombosAndButtons ) ),
      m_pPushButtons( new QVBox( m_pCombosAndButtons ) ),
      m_pCheckBoxes( new QGrid( 2, m_pCustomWidget ) ),
      m_bIsSave( false ),
      m_bIsExecuting( false ),
      m_bCanNotifySelection( true )
{
    connect( this, SIGNAL( fileHighlighted( const QString & ) ),
             this, SLOT( fileHighlightedCommand( const QString & ) ) );

    connect( this, SIGNAL( selectionChanged() ),
             this, SLOT( selectionChangedCommand() ) );

    m_pCustomWidget->setSpacing( KDialog::spacingHint() );
    m_pCombosAndButtons->setSpacing( KDialog::spacingHint() );

    updateCustomWidgetLayout();
}

FileDialog::~FileDialog()
{
}

void FileDialog::resizeEvent( QResizeEvent *pEvent )
{
    KFileDialog::resizeEvent( pEvent );

    updateCustomWidgetLayout();
}

void FileDialog::showEvent( QShowEvent *pEvent )
{
    KFileDialog::showEvent( pEvent );

    updateCustomWidgetLayout();
}

void FileDialog::updateCustomWidgetLayout()
{
    QPoint qReferencePoint = filterWidget->mapTo( this, QPoint( 0, 0 ) );
    QPoint qCustomPoint = m_pCustomWidget->mapTo( this, QPoint( 0, 0 ) );

    int nLeft = qReferencePoint.x() - qCustomPoint.x();
    int nRight = m_pCustomWidget->width() - filterWidget->width() - nLeft;

    nLeft -= KDialog::spacingHint();
    nRight -= KDialog::spacingHint();
    m_pLabels->setFixedWidth( ( nLeft > 0 )? nLeft: 80 );
    // FIXME The following call sets the width of m_pPushButtons all right,
    // but it also increases the width of m_pComboBoxes rapidly. Can we do
    // anything about it?
    m_pPushButtons->setFixedWidth( ( nRight > 0 )? nRight: 100 );
}

void FileDialog::customEvent( QCustomEvent *pEvent )
{
    if ( pEvent && pEvent->type() == CommandEvent::TypeId )
    {
        CommandEvent *pCommandEvent = static_cast< CommandEvent* >( pEvent );
        QStringList *pStringList = pCommandEvent->stringList();

        int nListSize = -1;
        if ( pStringList )
            nListSize = pStringList->size();

        switch ( pCommandEvent->command() )
        {
            case CommandEvent::AppendControl:
                if ( nListSize >= 3 )
                {
                    appendControl( (*pStringList)[0], (*pStringList)[1], (*pStringList)[2] );
                }
                break;
            case CommandEvent::EnableControl:
                if ( nListSize >= 2 )
                {
                    enableControl( (*pStringList)[0], (*pStringList)[1] );
                }
                break;
            case CommandEvent::GetValue:
                if ( nListSize >= 2 )
                {
                    getValue( (*pStringList)[0], (*pStringList)[1] );
                }
                break;
            case CommandEvent::SetValue:
                if ( nListSize >= 2 )
                {
                    QStringList qStringList = (*pStringList);
                    qStringList.pop_front();
                    qStringList.pop_front();

                    setValue( (*pStringList)[0], (*pStringList)[1], qStringList );
                }
                break;
            case CommandEvent::AppendFilter:
                if ( nListSize >= 2 )
                {
                    appendFilter( (*pStringList)[0], (*pStringList)[1] );

                    // update the filters widget
                    setFilter( filters() );
                }
                break;
            case CommandEvent::AppendFilterGroup:
                if ( nListSize >= 1 )
                {
                    QStringList::const_iterator it = pStringList->begin();
                    ++it; // We ignore the filter group name

                    while ( it != pStringList->end() )
                    {
                        QString qTitle = *it;
                        ++it;
                        if ( it != pStringList->end() )
                        {
                            appendFilter( qTitle, (*it) );
                            ++it;
                        }
                    }

                    // update the filters widget
                    setFilter( filters() );
                }
                break;
            case CommandEvent::GetCurrentFilter:
                {
                    QString qCurrentFilter = filterWidget->currentText();
                    sendCommand( "currentFilter " + escapeString( qCurrentFilter ) );
                }
                break;
            case CommandEvent::SetCurrentFilter:
                if ( nListSize >= 1 )
                {
                    static_cast< FileFilterComboHack* >( filterWidget )->setCurrentFilter( pStringList->front() );
                }
                break;
            case CommandEvent::GetDirectory:
                {
                    QString qDirectory = baseURL().url();
                    if ( qDirectory.startsWith( "file:/" ) && qDirectory.mid( 6, 1 ) != "/" )
                        qDirectory.replace( "file:/", "file:///" );
                    sendCommand( "currentDirectory " + escapeString( qDirectory ) );
                }
                break;
            case CommandEvent::SetDirectory:
                if ( nListSize >= 1 )
                {
                    setURL( pStringList->front() );
                }
                break;
            case CommandEvent::GetFiles:
                {
                    QString qString;
                    qString.reserve( 1024 );

                    qString.append( "files" );

                    if ( result() == QDialog::Accepted )
                    {
                        KURL::List qList( selectedURLs() );
                        for ( KURL::List::const_iterator it = qList.begin(); it != qList.end(); ++it )
                        {
                            qString.append( " " );
                            QString qUrlStr = addExtension( (*it).url() );

                            if ( !isExecuting() && !isSupportedProtocol( KURL( qUrlStr ).protocol() ) )
                                qUrlStr = localCopy( qUrlStr );

                            if ( qUrlStr.startsWith( "file:/" ) && qUrlStr.mid( 6, 1 ) != "/" )
                                qUrlStr.replace( "file:/", "file:///" );

                            if ( !qUrlStr.isEmpty() )
                                appendEscaped( qString, qUrlStr );
                        }
                    }
                    else
                    {
                        // we have to return the selected files anyway
                        const KFileItemList *pItems = ops->selectedItems();
                        for ( KFileItemListIterator it( *pItems ); it.current(); ++it )
                        {
                            qString.append( " " );
                            QString qUrlStr = addExtension( (*it)->url().url() );

                            if ( !isExecuting() && !isSupportedProtocol( KURL( qUrlStr ).protocol() ) )
                                qUrlStr = localCopy( qUrlStr );

                            if ( qUrlStr.startsWith( "file:/" ) && qUrlStr.mid( 6, 1 ) != "/" )
                                qUrlStr.replace( "file:/", "file:///" );

                            if ( !qUrlStr.isEmpty() )
                                appendEscaped( qString, qUrlStr );
                        }
                    }

                    sendCommand( qString );
                    setCanNotifySelection( true );
                }
                break;
            case CommandEvent::SetTitle:
                if ( nListSize >= 1 )
                {
                    setCaption( pStringList->front() );
                }
                break;
            case CommandEvent::SetType:
                if ( nListSize >= 1 )
                {
                    QString qType( pStringList->front() );
                    if ( qType == "open" )
                    {
                        setIsSave( false );
                        setCaption( i18n( "Open" ) );
                    }
                    else if ( qType == "save" )
                    {
                        setIsSave( true );
                        setCaption( i18n( "Save As" ) );
                    }
                }
                break;
            case CommandEvent::SetDefaultName:
                if ( nListSize >= 1 )
                {
                    setKeepLocation( true );
                    setSelection( pStringList->front() );
                }
                break;
            case CommandEvent::SetMultiSelection:
                if ( nListSize >= 1 )
                {
                    if ( pStringList->front() == "true" )
                        setMode( KFile::Files );
                    else
                        setMode( KFile::File );
                }
                break;
            case CommandEvent::Exec:
                {
                    filterWidget->setEditable( false );
                    QString qSelectedURL;
                    setIsExecuting( true );
                    bool bCanExit = false;
                    do {
                        setCanNotifySelection( true );
                        exec();

                        qSelectedURL = addExtension( selectedURL().url() );
                        QString qProtocol( selectedURL().protocol() );

                        if ( isSave() && result() == QDialog::Accepted )
                        {
                            if ( qSelectedURL.startsWith( "file:" ) )
                            {
                                bCanExit =
                                    !QFile::exists( qSelectedURL.mid( 5 ) ) ||
                                    ( KMessageBox::warningYesNo( 0,
                                                                 i18n( "A file named \"%1\" already exists. "
                                                                     "Are you sure you want to overwrite it?" ).arg( qSelectedURL ),
                                                                 i18n( "Overwrite File?" ),
                                                                 i18n( "Overwrite" ), KStdGuiItem::cancel() ) == KMessageBox::Yes );
                            }
                            else if ( !isSupportedProtocol( qProtocol ) )
                            {
                                KMessageBox::sorry( 0,
                                        i18n( "Saving using protocol \"%1\" is not supported." ).arg( qProtocol ) );
                                bCanExit = false;
                            }
                            else
                                bCanExit = true;
                        }
                        else if ( !isSave() && result() == QDialog::Accepted && !isSupportedProtocol( qProtocol ) )
                        {
                            KMessageBox::information( 0,
                                    i18n( "Protocol \"%1\" is supported only partially. "
                                        "Local copy of the file will be created." ).arg( qProtocol ) );
                            bCanExit = true;
                        }
                        else
                            bCanExit = true;
                    } while ( !bCanExit );
                    setIsExecuting( false );

                    if ( result() == QDialog::Accepted )
                        sendCommand( "accept" );
                    else
                        sendCommand( "reject" );
                }
                break;
            default:
                break;
        }

        // FIXME Some cleanup of pEvent? delete something, etc.?
    }
}

void FileDialog::appendControl( const QString &rId, const QString &rType, const QString &rTitle )
{
    QString qLabel( rTitle );
    qLabel.replace( '~', '&' );

    if ( rType == "checkbox" )
    {
        QCheckBox *pCheckBox = new QCheckBox( qLabel, m_pCheckBoxes, rId.utf8() );

        pCheckBox->setEnabled( true );
        pCheckBox->setChecked( false );
    }
    else if ( rType == "listbox" )
    {
        QLabel *pComboLabel = new QLabel( qLabel, m_pLabels );
        QComboBox *pComboBox = new QComboBox( m_pComboBoxes, rId.utf8() );

        pComboLabel->setBuddy( pComboBox );
        pComboBox->setEnabled( true );
    }
    else if ( rType == "pushbutton" )
    {
        QPushButton *pPushButton = new QPushButton( qLabel, m_pPushButtons, rId.utf8() );
        pPushButton->setEnabled( true );
    }
}

QWidget* FileDialog::findControl( const QString &rId ) const
{
    QObjectList *pList = m_pCustomWidget->queryList();
    QCString qName( rId.utf8() );
    QObjectList::const_iterator it = pList->begin();

    for ( ; it != pList->end() && qName != (*it)->name(); ++it )
        ;

    QWidget *pWidget = NULL;
    if ( it != pList->end() )
        pWidget = static_cast< QWidget* >( *it );

    delete pList;

    return pWidget;
}

void FileDialog::enableControl( const QString &rId, const QString &rValue )
{
    QWidget *pWidget = findControl( rId );

    if ( pWidget )
        pWidget->setEnabled( rValue.lower() == "true" );
}

void FileDialog::getValue( const QString &rId, const QString &rAction )
{
    QWidget *pWidget = findControl( rId );
    QString qString;
    qString.reserve( 1024 );
    qString.append( "value" );

    if ( pWidget )
    {
        QCString qClassName = pWidget->className();
        if ( qClassName == "QCheckBox" )
        {
            QCheckBox *pCheckBox = static_cast< QCheckBox* >( pWidget );

            if ( pCheckBox->isChecked() )
                qString.append( " bool true" );
            else
                qString.append( " bool false" );
        }
        else if ( qClassName == "QComboBox" )
        {
            QComboBox *pComboBox = static_cast< QComboBox* >( pWidget );
            if ( rAction == "getItems" )
            {
                qString.append( " stringList" );
                for ( int nIdx = 0; nIdx < pComboBox->count(); ++nIdx )
                {
                    qString.append( ' ' );
                    appendEscaped( qString, pComboBox->text( nIdx ) );
                }
            }
            else if ( rAction == "getSelectedItem" )
            {
                qString.append( " string " );
                appendEscaped( qString, pComboBox->currentText() );
            }
            else if ( rAction == "getSelectedItemIndex" )
            {
                qString.append( " int " );
                qString.append( QString().setNum( pComboBox->currentItem() ) );
            }
            // TODO getHelpURL
        }
        // TODO push button
    }

    sendCommand( qString );
}

void FileDialog::setValue( const QString &rId, const QString &rAction, const QStringList &rValue )
{
    QWidget *pWidget = findControl( rId );

    if ( pWidget )
    {
        QCString qClassName = pWidget->className();
        if ( qClassName == "QCheckBox" )
        {
            QCheckBox *pCheckBox = static_cast< QCheckBox* >( pWidget );

            bool bValue = ( !rValue.isEmpty() ) && ( rValue.front().lower() == "true" );
            pCheckBox->setChecked( bValue );
        }
        else if ( qClassName == "QComboBox" )
        {
            QComboBox *pComboBox = static_cast< QComboBox* >( pWidget );
            if ( rAction == "addItem" )
            {
                if ( !rValue.isEmpty() )
                    pComboBox->insertItem( rValue.front() );
            }
            else if ( rAction == "addItems" )
            {
                pComboBox->insertStringList( rValue );
            }
            else if ( rAction == "deleteItem" )
            {
                if ( !rValue.isEmpty() )
                    pComboBox->removeItem( rValue.front().toInt() );
            }
            else if ( rAction == "deleteItems" )
            {
                pComboBox->clear();
            }
            else if ( rAction == "setSelectedItem" )
            {
                if ( !rValue.isEmpty() )
                    pComboBox->setCurrentItem( rValue.front().toInt() );
            }
            // FIXME setHelpURL is ignored
        }
        // TODO push button
    }
}

void FileDialog::appendFilter( const QString &rTitle, const QString &rFilter )
{
    // Filters are separated by ';'
    QString qFilter( rFilter );
    qFilter.replace( QChar( ';' ), QChar( ' ' ) ).replace( "*.*", "*" );

    m_aFilters.push_back( qMakePair( rTitle, qFilter ) );
}

QString FileDialog::filters() const
{
    QString qString, qTmp;
    bool bFirstFilter = true;

    for ( FilterList::const_iterator it = m_aFilters.begin(); it != m_aFilters.end(); ++it )
    {
        if ( bFirstFilter )
            bFirstFilter = false;
        else
            qString.append( '\n' );

        qString.append( (*it).second );
        qString.append( '|' );

        qTmp = (*it).first;
        qString.append( qTmp.replace( '/', "\\/" ) );
    }

    return qString;
}

QString FileDialog::addExtension( const QString &rFileName ) const
{
    if ( !isSave() )
        return rFileName;

    QString qExtension;

    QWidget *pExtensionWidget = findControl( "100" ); // CHECKBOX_AUTOEXTENSION
    QCheckBox *pExtensionCB = pExtensionWidget? static_cast< QCheckBox* >( pExtensionWidget->qt_cast( "QCheckBox" ) ): NULL;
    if ( pExtensionCB && pExtensionCB->isChecked() )
    {
        // FIXME: qFilter can be a MIME; we ignore it now...
        QStringList qFilterList = QStringList::split( " ", currentFilter() );
        for ( QStringList::const_iterator it = qFilterList.begin();
              qExtension.isEmpty() && it != qFilterList.end();
              ++it )
        {
            int nUnwanted = (*it).findRev( '*' );
            if ( nUnwanted < 0 )
                nUnwanted = (*it).findRev( '?' );
            else
                nUnwanted = ::std::max( nUnwanted, (*it).find( '?', nUnwanted ) );

            int nIdx = (*it).find( '.', ::std::max( nUnwanted, 0 ) );
            if ( nIdx >= 0 )
                qExtension = (*it).mid( nIdx ).lower();
        }
    }

    if ( qExtension.isEmpty() || qExtension == "." || rFileName.endsWith( qExtension ) )
        return rFileName;
    else
        return rFileName + qExtension;
}

bool FileDialog::isSupportedProtocol( const QString &rProtocol ) const
{
    // TODO Get this information directly from OOo
    const char * pOOoProtocols[] = { "", "smb", "ftp", "http", "file", "mailto",
        "vnd.sun.star.webdav", "news", "private", "vnd.sun.star.help",
        "https", "slot", "macro", "javascript", "imap", "pop3", "data",
        "cid", "out", "vnd.sun.star.wfs", "vnd.sun.star.hier", "vim",
        ".uno", ".component", "vnd.sun.star.pkg", "ldap", "db",
        "vnd.sun.star.cmd", "vnd.sun.star.script", "vnd.sun.star.odma",
        "telnet",
        NULL };

    for ( const char **pIndex = pOOoProtocols; *pIndex != NULL; ++pIndex )
    {
        if ( rProtocol == *pIndex )
            return true;
    }

    // TODO gnome-vfs bits here

    return false;
}

QString FileDialog::localCopy( const QString &rFileName ) const
{
    int nExtensionPos = rFileName.findRev( '/' );
    if ( nExtensionPos >= 0 )
        nExtensionPos = rFileName.find( '.', nExtensionPos );
    else
        nExtensionPos = rFileName.find( '.' );

    KTempFile qTempFile( QString::null, ( nExtensionPos < 0 )? QString(): rFileName.mid( nExtensionPos ) );
    KURL qDestURL;
    qDestURL.setPath( qTempFile.name() );

    if ( !KIO::NetAccess::file_copy( rFileName, qDestURL, 0600, true, false, NULL ) )
    {
        KMessageBox::error( 0, KIO::NetAccess::lastErrorString() );
        return QString::null;
    }

    return qDestURL.url();
}

void FileDialog::fileHighlightedCommand( const QString & )
{
    if ( canNotifySelection() )
    {
    sendCommand( "fileSelectionChanged" );
    setCanNotifySelection( false );
    }
}

void FileDialog::selectionChangedCommand()
{
    if ( canNotifySelection() )
    {
    sendCommand( "fileSelectionChanged" );
    setCanNotifySelection( false );
    }
}

void FileDialog::sendCommand( const QString &rCommand )
{
#if OSL_DEBUG_LEVEL > 0
    ::std::cerr << "kdefilepicker sent: " << rCommand.latin1() << ::std::endl;
#endif

    //m_aOutputStream << rCommand << endl;
    ::std::cout << rCommand.utf8() << ::std::endl;
}

void FileDialog::appendEscaped( QString &rBuffer, const QString &rString )
{
    const QChar *pUnicode = rString.unicode();
    const QChar *pEnd     = pUnicode + rString.length();

    rBuffer.append( '"' );
    for ( ; pUnicode != pEnd; ++pUnicode )
    {
        if ( *pUnicode == '\\' )
            rBuffer.append( "\\\\" );
        else if ( *pUnicode == '"' )
            rBuffer.append( "\\\"" );
        else if ( *pUnicode == '\n' )
            rBuffer.append( "\\\n" );
        else
            rBuffer.append( *pUnicode );
    }
    rBuffer.append( '"' );
}

QString FileDialog::escapeString( const QString &rString )
{
    QString qString;
    qString.reserve( 2*rString.length() + 2 ); // every char escaped + quotes

    appendEscaped( qString, rString );

    return qString;
}


void FileFilterComboHack::setCurrentFilter( const QString& filter )
{
    setCurrentText( filter );
    filterChanged();
}
