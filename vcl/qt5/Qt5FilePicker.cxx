/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "Qt5FilePicker.hxx"
#include <Qt5FilePicker.moc>

#include "Qt5Frame.hxx"
#include "Qt5Tools.hxx"
#include "Qt5Widget.hxx"

#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ui/dialogs/CommonFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/ControlActions.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <cppuhelper/interfacecontainer.h>
#include <cppuhelper/supportsservice.hxx>

#include <QtCore/QDebug>
#include <QtCore/QRegularExpression>
#include <QtCore/QThread>
#include <QtCore/QUrl>
#include <QtGui/QClipboard>
#include <QtGui/QWindow>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

#include <unx/geninst.h>
#include <strings.hrc>


using namespace ::com::sun::star;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::ui::dialogs::TemplateDescription;
using namespace ::com::sun::star::ui::dialogs::ExtendedFilePickerElementIds;
using namespace ::com::sun::star::ui::dialogs::CommonFilePickerElementIds;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;

namespace
{
    uno::Sequence<OUString> SAL_CALL FilePicker_getSupportedServiceNames()
    {
        uno::Sequence<OUString> aRet(3);
        aRet[0] = "com.sun.star.ui.dialogs.FilePicker";
        aRet[1] = "com.sun.star.ui.dialogs.SystemFilePicker";
        aRet[2] = "com.sun.star.ui.dialogs.Qt5FilePicker";
        return aRet;
    }
}

Qt5FilePicker::Qt5FilePicker( QFileDialog::FileMode eMode )
    : Qt5FilePicker_Base( m_aHelperMutex )
{
SAL_DEBUG( "Qt5FilePicker::Qt5FilePicker" );
    m_pFileDialog = new QFileDialog();
    m_pOptionsDialog = new QDialog();

    m_pFileDialog->setFileMode( eMode );
//    m_pFileDialog->setWindowModality( Qt::WindowModal );
    m_pFileDialog->setWindowModality( Qt::ApplicationModal );
    m_pOptionsDialog->setWindowModality( Qt::WindowModal );

    setMultiSelectionMode( false );

    // XExecutableDialog functions
    connect( this, SIGNAL( setTitleSignal( const OUString & ) ),
             this, SLOT( setTitleSlot( const OUString & ) ), Qt::BlockingQueuedConnection );
    connect( this, SIGNAL( executeSignal() ),
             this, SLOT( executeSlot() ), Qt::BlockingQueuedConnection );

    // XFilePicker functions
    connect( this, SIGNAL( setMultiSelectionModeSignal( bool ) ),
             this, SLOT( setMultiSelectionModeSlot( bool ) ), Qt::BlockingQueuedConnection );
    connect( this, SIGNAL( setDefaultNameSignal( const OUString & ) ),
             this, SLOT( setDefaultNameSlot( const OUString & ) ), Qt::BlockingQueuedConnection );
    connect( this, SIGNAL( setDisplayDirectorySignal( const OUString & ) ),
             this, SLOT( setDisplayDirectorySlot( const OUString & ) ), Qt::BlockingQueuedConnection );
    connect( this, SIGNAL( getDisplayDirectorySignal() ),
             this, SLOT( getDisplayDirectorySlot() ), Qt::BlockingQueuedConnection );
    connect( this, SIGNAL( getFilesSignal() ),
             this, SLOT( getFilesSlot() ), Qt::BlockingQueuedConnection );

    // XFilterManager functions
    connect( this, SIGNAL( appendFilterSignal( const OUString &, const OUString & ) ),
             this, SLOT( appendFilterSlot( const OUString &, const OUString & ) ), Qt::BlockingQueuedConnection );
    connect( this, SIGNAL( setCurrentFilterSignal( const OUString & ) ),
             this, SLOT( setCurrentFilterSlot( const OUString & ) ), Qt::BlockingQueuedConnection );
    connect( this, SIGNAL( getCurrentFilterSignal() ),
             this, SLOT( getCurrentFilterSlot() ), Qt::BlockingQueuedConnection );

    // XFilterGroupManager functions
    connect( this, SIGNAL( appendFilterGroupSignal( const OUString &, const css::uno::Sequence< css::beans::StringPair > & ) ),
             this, SLOT( appendFilterGroupSlot( const OUString &, const css::uno::Sequence< css::beans::StringPair > & ) ), Qt::BlockingQueuedConnection );

    // XFilePickerControlAccess functions
    connect( this, SIGNAL( setValueSignal( sal_Int16, sal_Int16, const css::uno::Any & ) ),
             this, SLOT( setValueSlot( sal_Int16, sal_Int16, const css::uno::Any & ) ), Qt::BlockingQueuedConnection );
    connect( this, SIGNAL( getValueSignal( sal_Int16, sal_Int16 ) ),
             this, SLOT( getValueSlot( sal_Int16, sal_Int16 ) ), Qt::BlockingQueuedConnection );
    connect( this, SIGNAL( enableControlSignal( sal_Int16, bool ) ),
             this, SLOT( enableControlSlot( sal_Int16, bool ) ), Qt::BlockingQueuedConnection );
    connect( this, SIGNAL( setLabelSignal( sal_Int16, const OUString & ) ),
             this, SLOT( setLabelSlot( sal_Int16, const OUString & ) ), Qt::BlockingQueuedConnection );
    connect( this, SIGNAL( getLabelSignal( sal_Int16 ) ),
             this, SLOT( getLabelSlot( sal_Int16 ) ), Qt::BlockingQueuedConnection );

    // XFilePicker2 functions
    connect( this, SIGNAL( getSelectedFilesSignal() ),
             this, SLOT( getSelectedFilesSlot() ), Qt::BlockingQueuedConnection );

    // XInitialization
    connect( this, SIGNAL( initializeSignal( const css::uno::Sequence< css::uno::Any > & ) ),
             this, SLOT( initializeSlot( const css::uno::Sequence< css::uno::Any > & ) ), Qt::BlockingQueuedConnection );

    // Destructor proxy
    connect( this, SIGNAL( cleanupProxySignal() ), this, SLOT( cleanupProxy() ), Qt::BlockingQueuedConnection );

    // XFilePickerListener notifications
    connect( m_pFileDialog, SIGNAL( filterSelected(const QString&) ), this, SLOT( filterSelected(const QString&) ));
    connect( m_pFileDialog, SIGNAL( selectionChanged() ), this, SLOT( selectionChanged() ));
}

Qt5FilePicker::~Qt5FilePicker()
{
    cleanupProxy();
}

void Qt5FilePicker::cleanupProxy()
{
    if( qApp->thread() != QThread::currentThread() ) {
        SolarMutexReleaser aReleaser;
        return Q_EMIT cleanupProxySignal();
    }
    delete m_pOptionsDialog;
}

void SAL_CALL Qt5FilePicker::addFilePickerListener( const uno::Reference<XFilePickerListener>& xListener )
{
    SolarMutexGuard aGuard;
    m_xListener = xListener;
}

void SAL_CALL Qt5FilePicker::removeFilePickerListener( const uno::Reference<XFilePickerListener>& )
{
    SolarMutexGuard aGuard;
    m_xListener.clear();
}

void SAL_CALL Qt5FilePicker::setTitle( const OUString &title )
{
    if( qApp->thread() != QThread::currentThread() ) {
        SolarMutexReleaser aReleaser;
        return Q_EMIT setTitleSignal( title );
    }

    m_pOptionsDialog->setWindowTitle( toQString(title) );
}

sal_Int16 SAL_CALL Qt5FilePicker::execute()
{
    if( qApp->thread() != QThread::currentThread() ) {
        SolarMutexReleaser aReleaser;
        return Q_EMIT executeSignal();
    }

    vcl::Window *pWindow = ::Application::GetActiveTopWindow();
    assert( pWindow );
    Qt5Widget *pTransientParent = nullptr;
    QWindow *pTransientWindow = nullptr;
    if( pWindow )
    {
        Qt5Frame *pFrame = dynamic_cast<Qt5Frame *>( pWindow->ImplGetFrame() );
        assert( pFrame );
        if( pFrame )
        {
            pTransientParent = static_cast<Qt5Widget *>( pFrame->GetQWidget() );
            pTransientWindow = pTransientParent->window()->windowHandle();
        }
    }

    m_pFileDialog->setNameFilters( m_aNamedFilterList );
    if( !m_aCurrentFilterTitle.isNull() )
    {
        int i = 0;
        for( ; i < m_aFilterTitleList.size(); ++i )
            if( m_aCurrentFilterTitle == m_aFilterTitleList[ i ] )
            {
                m_pFileDialog->selectNameFilter( m_aNamedFilterList[ i ] );
                break;
            }
        assert( i < m_aFilterTitleList.size() );
    }

    if( pTransientParent )
    {
        m_pFileDialog->show();
        m_pFileDialog->window()->windowHandle()->setTransientParent( pTransientWindow );
        m_pFileDialog->setFocusProxy( pTransientParent );
//        pTransientParent->setMouseTracking( false );
    }
    int result = m_pFileDialog->exec();
    if( pTransientParent )
//        pTransientParent->setMouseTracking( true );
    if( QFileDialog::Rejected == result )
        return ExecutableDialogResults::CANCEL;

    if ( !m_aCustomWidgetsMap.empty() )
    {
        m_pFilenameLabel->setText( m_pFileDialog->selectedUrls()[ 0 ].url() );
        QString filter = m_pFileDialog->selectedNameFilter();
        int pos = filter.indexOf(" (");
        if( pos >= 0 )
            filter.truncate( pos );
        m_pFilterLabel->setText( filter );

        if( pTransientParent )
        {
            m_pOptionsDialog->show();
            m_pOptionsDialog->window()->windowHandle()->setTransientParent( pTransientWindow );
            m_pOptionsDialog->setFocusProxy( pTransientParent );
        }
        result = m_pOptionsDialog->exec();
        if( QFileDialog::Rejected == result )
            return ExecutableDialogResults::CANCEL;
    }

    return ExecutableDialogResults::OK;
}

void SAL_CALL Qt5FilePicker::setMultiSelectionMode( sal_Bool multiSelect )
{
    if( qApp->thread() != QThread::currentThread() ) {
        SolarMutexReleaser aReleaser;
        return Q_EMIT setMultiSelectionModeSignal( multiSelect );
    }

    if( multiSelect )
        m_pFileDialog->setFileMode( QFileDialog::ExistingFiles );
    else
        m_pFileDialog->setFileMode( QFileDialog::ExistingFile );
}

void SAL_CALL Qt5FilePicker::setDefaultName( const OUString &name )
{
    if( qApp->thread() != QThread::currentThread() ) {
        SolarMutexReleaser aReleaser;
        return Q_EMIT setDefaultNameSignal( name );
    }
    m_pFileDialog->selectFile( toQString( name ) );
}

void SAL_CALL Qt5FilePicker::setDisplayDirectory( const OUString &dir )
{
    if( qApp->thread() != QThread::currentThread() ) {
        SolarMutexReleaser aReleaser;
        return Q_EMIT setDisplayDirectorySignal( dir );
    }
    m_pFileDialog->setDirectory( toQString( dir ) );
}

OUString SAL_CALL Qt5FilePicker::getDisplayDirectory()
{
    if( qApp->thread() != QThread::currentThread() ) {
        SolarMutexReleaser aReleaser;
        return Q_EMIT getDisplayDirectorySignal();
    }
    return toOUString( m_pFileDialog->directoryUrl().toString() );
}

uno::Sequence< OUString > SAL_CALL Qt5FilePicker::getFiles()
{
    if( qApp->thread() != QThread::currentThread() ) {
        SolarMutexReleaser aReleaser;
        return Q_EMIT getFilesSignal();
    }
    uno::Sequence< OUString > seq = getSelectedFiles();
    if (seq.getLength() > 1)
        seq.realloc(1);
    return seq;
}

uno::Sequence< OUString > SAL_CALL Qt5FilePicker::getSelectedFiles()
{
    if( qApp->thread() != QThread::currentThread() ) {
        SolarMutexReleaser aReleaser;
        return Q_EMIT getSelectedFilesSignal();
    }
    QList<QUrl> urls = m_pFileDialog->selectedUrls();
    uno::Sequence< OUString > seq( urls.size() );
    int i = 0;
    foreach( const QUrl& url, urls )
        seq[ i++ ]= toOUString( url.toString() );
    return seq;
}

void SAL_CALL Qt5FilePicker::appendFilter( const OUString &title, const OUString &filter )
{
    if( qApp->thread() != QThread::currentThread() ) {
        SolarMutexReleaser aReleaser;
        return Q_EMIT appendFilterSignal( title, filter );
    }

    QString t = toQString(title);
    QString f = toQString(filter);

    // '/' need to be escaped else they are assumed to be mime types by kfiledialog
    //see the docs
    t.replace("/", "\\/");

    int pos = t.indexOf(" (");
    if( pos >= 0 )
        t.truncate( pos );

    // openoffice gives us filters separated by ';' qt dialogs just want space separated
    f.replace(";", " ");

    // make sure "*.*" is not used as "all files"
    f.replace("*.*", "*");

    m_aFilterTitleList.append( toQString( title ) );
    m_aNamedFilterList.append( QString("%1 (%2)").arg(t).arg(f) );
}

void SAL_CALL Qt5FilePicker::setCurrentFilter( const OUString &title )
{
    if( qApp->thread() != QThread::currentThread() ) {
        SolarMutexReleaser aReleaser;
        return Q_EMIT setCurrentFilterSignal( title );
    }

    m_aCurrentFilterTitle = toQString( title );
}

OUString SAL_CALL Qt5FilePicker::getCurrentFilter()
{
    if( qApp->thread() != QThread::currentThread() ) {
        SolarMutexReleaser aReleaser;
        return Q_EMIT getCurrentFilterSignal();
    }

    QString filter = m_pFileDialog->selectedNameFilter();
    for( int i = 0; i < m_aNamedFilterList.size(); ++i )
        if( filter == m_aNamedFilterList[ i ] )
            return toOUString( m_aFilterTitleList[ i ] );
    SAL_DEBUG( "vcl.qt5 " << m_aNamedFilterList.size() << " Unknown filter: " << toOUString( filter ) );
    assert( !"Selected filter not in filter list?! " );
    return OUString( "" );
}

void SAL_CALL Qt5FilePicker::appendFilterGroup( const OUString& rGroupTitle, const uno::Sequence<beans::StringPair>& filters)
{
    if( qApp->thread() != QThread::currentThread() ) {
        SolarMutexReleaser aReleaser;
        return Q_EMIT appendFilterGroupSignal( rGroupTitle, filters );
    }

    const sal_uInt16 length = filters.getLength();
    for (sal_uInt16 i = 0; i < length; ++i)
    {
        beans::StringPair aPair = filters[i];
        appendFilter( aPair.First, aPair.Second );
    }
}

void SAL_CALL Qt5FilePicker::setValue( sal_Int16 controlId, sal_Int16 nControlAction, const uno::Any &value )
{
    if( qApp->thread() != QThread::currentThread() ) {
        SolarMutexReleaser aReleaser;
        return Q_EMIT setValueSignal( controlId, nControlAction, value );
    }

    if (m_aCustomWidgetsMap.contains( controlId )) {
        QCheckBox* cb = dynamic_cast<QCheckBox*>( m_aCustomWidgetsMap.value( controlId ));
        if (cb)
            cb->setChecked(value.get<bool>());
    }
    else
        SAL_WARN( "vcl", "set label on unknown control " << controlId );
}

uno::Any SAL_CALL Qt5FilePicker::getValue( sal_Int16 controlId, sal_Int16 nControlAction )
{
    if (CHECKBOX_AUTOEXTENSION == controlId)
        // We ignore this one and rely on QFileDialog to provide the function.
        // Always return false, to pretend we do not support this, otherwise
        // LO core would try to be smart and cut the extension in some places,
        // interfering with QFileDialog's handling of it. QFileDialog also
        // saves the value of the setting, so LO core is not needed for that either.
        return uno::Any( false );

    if( qApp->thread() != QThread::currentThread() ) {
        SolarMutexReleaser aReleaser;
        return Q_EMIT getValueSignal( controlId, nControlAction );
    }

    uno::Any res(false);
    if (m_aCustomWidgetsMap.contains( controlId )) {
        QCheckBox* cb = dynamic_cast<QCheckBox*>( m_aCustomWidgetsMap.value( controlId ));
        if (cb)
            res <<= cb->isChecked();
    }
    else
        SAL_WARN( "vcl", "get value on unknown control " << controlId );

    return res;
}

void SAL_CALL Qt5FilePicker::enableControl( sal_Int16 controlId, sal_Bool enable )
{
    if( qApp->thread() != QThread::currentThread() ) {
        SolarMutexReleaser aReleaser;
        return Q_EMIT enableControlSignal( controlId, enable );
    }

    if (m_aCustomWidgetsMap.contains( controlId ))
        m_aCustomWidgetsMap.value( controlId )->setEnabled( enable );
    else
        SAL_WARN( "vcl", "enable unknown control " << controlId );
}

void SAL_CALL Qt5FilePicker::setLabel( sal_Int16 controlId, const OUString &label )
{
    if( qApp->thread() != QThread::currentThread() ) {
        SolarMutexReleaser aReleaser;
        return Q_EMIT setLabelSignal( controlId, label );
    }

    if (m_aCustomWidgetsMap.contains( controlId )) {
        QCheckBox* cb = dynamic_cast<QCheckBox*>( m_aCustomWidgetsMap.value( controlId ));
        if (cb)
            cb->setText( toQString(label) );
    }
    else
        SAL_WARN( "vcl", "set label on unknown control " << controlId );
}

OUString SAL_CALL Qt5FilePicker::getLabel(sal_Int16 controlId)
{
    if( qApp->thread() != QThread::currentThread() ) {
        SolarMutexReleaser aReleaser;
        return Q_EMIT getLabelSignal( controlId );
    }

    QString label;
    if (m_aCustomWidgetsMap.contains( controlId )) {
        QCheckBox* cb = dynamic_cast<QCheckBox*>( m_aCustomWidgetsMap.value( controlId ));
        if (cb)
            label = cb->text();
    }
    else
        SAL_WARN( "vcl", "get label on unknown control " << controlId );

    return toOUString(label);
}

QString Qt5FilePicker::getResString(const char *pResId)
{
    QString aResString;

    if (pResId == nullptr)
        return aResString;

    aResString = toQString(VclResId(pResId));

    return aResString.replace('~', '&');
}

void Qt5FilePicker::addCustomControl(QGridLayout *pLayout, sal_Int16 controlId)
{
    QWidget* widget = nullptr;
    QWidget* label = nullptr;
    const char* resId = nullptr;

    switch (controlId)
    {
        case CHECKBOX_AUTOEXTENSION:
            resId = STR_FPICKER_AUTO_EXTENSION;
            break;
        case CHECKBOX_PASSWORD:
            resId = STR_FPICKER_PASSWORD;
            break;
        case CHECKBOX_FILTEROPTIONS:
            resId = STR_FPICKER_FILTER_OPTIONS;
            break;
        case CHECKBOX_READONLY:
            resId = STR_FPICKER_READONLY;
            break;
        case CHECKBOX_LINK:
            resId = STR_FPICKER_INSERT_AS_LINK;
            break;
        case CHECKBOX_PREVIEW:
            resId = STR_FPICKER_SHOW_PREVIEW;
            break;
        case CHECKBOX_SELECTION:
            resId = STR_FPICKER_SELECTION;
            break;
        case PUSHBUTTON_PLAY:
            resId = STR_FPICKER_PLAY;
            break;
        case LISTBOX_VERSION:
            resId = STR_FPICKER_VERSION;
            break;
        case LISTBOX_TEMPLATE:
            resId = STR_FPICKER_TEMPLATES;
            break;
        case LISTBOX_IMAGE_TEMPLATE:
            resId = STR_FPICKER_IMAGE_TEMPLATE;
            break;
        case LISTBOX_VERSION_LABEL:
        case LISTBOX_TEMPLATE_LABEL:
        case LISTBOX_IMAGE_TEMPLATE_LABEL:
        case LISTBOX_FILTER_SELECTOR:
            break;
    }

    switch (controlId)
    {
        case CHECKBOX_AUTOEXTENSION:
        case CHECKBOX_PASSWORD:
        case CHECKBOX_FILTEROPTIONS:
        case CHECKBOX_READONLY:
        case CHECKBOX_LINK:
        case CHECKBOX_PREVIEW:
        case CHECKBOX_SELECTION:
            widget = new QCheckBox(getResString(resId), m_pOptionsDialog);
            break;
        case PUSHBUTTON_PLAY:
            break;
        case LISTBOX_VERSION:
        case LISTBOX_TEMPLATE:
        case LISTBOX_IMAGE_TEMPLATE:
        case LISTBOX_FILTER_SELECTOR:
            label = new QLabel( getResString(resId), m_pOptionsDialog );
            widget = new QComboBox( m_pOptionsDialog );
            break;
        case LISTBOX_VERSION_LABEL:
        case LISTBOX_TEMPLATE_LABEL:
        case LISTBOX_IMAGE_TEMPLATE_LABEL:
            break;
    }

    if( widget )
    {
        const int row = pLayout->rowCount();
        if( label )
            pLayout->addWidget( label, row, 0 );
        pLayout->addWidget( widget, row, 1 );
        m_aCustomWidgetsMap.insert( controlId, widget );
    }
}

void SAL_CALL Qt5FilePicker::initialize( const uno::Sequence<uno::Any> &args )
{
    if( qApp->thread() != QThread::currentThread() ) {
        SolarMutexReleaser aReleaser;
        return Q_EMIT initializeSignal( args );
    }

    m_aNamedFilterList.clear();
    m_aFilterTitleList.clear();
    m_aCurrentFilterTitle.clear();

    // parameter checking
    uno::Any arg;
    if (args.getLength() == 0)
    {
        throw lang::IllegalArgumentException(
                "no arguments",
                static_cast< XFilePicker2* >( this ), 1 );
    }

    arg = args[0];

    if (( arg.getValueType() != cppu::UnoType<sal_Int16>::get()) &&
        ( arg.getValueType() != cppu::UnoType<sal_Int8>::get()))
    {
        throw lang::IllegalArgumentException(
                "invalid argument type",
                static_cast< XFilePicker2* >( this ), 1 );
    }

    QGridLayout *pLayout = new QGridLayout();
    m_pOptionsDialog->setLayout( pLayout );
    pLayout->addWidget( new QLabel( "Filename:" ), 0, 0 );
    m_pFilenameLabel = new QLabel();
    pLayout->addWidget( m_pFilenameLabel, 0, 1 );
    pLayout->addWidget( new QLabel( "Type:" ), 1, 0 );
    m_pFilterLabel = new QLabel();
    pLayout->addWidget( m_pFilterLabel, 1, 1 );

    sal_Int16 templateId = -1;
    arg >>= templateId;

    QFileDialog::AcceptMode acceptMode = QFileDialog::AcceptOpen;
    switch ( templateId )
    {
        case FILEOPEN_SIMPLE:
            break;

        case FILESAVE_SIMPLE:
            acceptMode = QFileDialog::AcceptSave;
            break;

        case FILESAVE_AUTOEXTENSION:
            acceptMode = QFileDialog::AcceptSave;
            addCustomControl( pLayout, CHECKBOX_AUTOEXTENSION );
            break;

        case FILESAVE_AUTOEXTENSION_PASSWORD:
            acceptMode = QFileDialog::AcceptSave;
            addCustomControl( pLayout, CHECKBOX_AUTOEXTENSION );
            addCustomControl( pLayout, CHECKBOX_PASSWORD );
            break;

        case FILESAVE_AUTOEXTENSION_PASSWORD_FILTEROPTIONS:
            acceptMode = QFileDialog::AcceptSave;
            addCustomControl( pLayout, CHECKBOX_AUTOEXTENSION );
            addCustomControl( pLayout, CHECKBOX_PASSWORD );
            addCustomControl( pLayout, CHECKBOX_FILTEROPTIONS );
            break;

        case FILESAVE_AUTOEXTENSION_SELECTION:
            acceptMode = QFileDialog::AcceptSave;
            addCustomControl( pLayout, CHECKBOX_AUTOEXTENSION );
            addCustomControl( pLayout, CHECKBOX_SELECTION );
            break;

        case FILESAVE_AUTOEXTENSION_TEMPLATE:
            acceptMode = QFileDialog::AcceptSave;
            addCustomControl( pLayout, CHECKBOX_AUTOEXTENSION );
            addCustomControl( pLayout, LISTBOX_TEMPLATE );
            break;

        case FILEOPEN_LINK_PREVIEW_IMAGE_TEMPLATE:
            addCustomControl( pLayout, CHECKBOX_LINK );
            addCustomControl( pLayout, CHECKBOX_PREVIEW );
            addCustomControl( pLayout, LISTBOX_IMAGE_TEMPLATE );
            break;

        case FILEOPEN_PLAY:
            addCustomControl( pLayout, PUSHBUTTON_PLAY );
            break;

        case FILEOPEN_LINK_PLAY:
            addCustomControl( pLayout, CHECKBOX_LINK );
            addCustomControl( pLayout, PUSHBUTTON_PLAY );
            break;

        case FILEOPEN_READONLY_VERSION:
            addCustomControl( pLayout, CHECKBOX_READONLY );
            addCustomControl( pLayout, LISTBOX_VERSION );
            break;

        case FILEOPEN_LINK_PREVIEW:
            addCustomControl( pLayout, CHECKBOX_LINK );
            addCustomControl( pLayout, CHECKBOX_PREVIEW );
            break;

        case FILEOPEN_PREVIEW:
            addCustomControl( pLayout, CHECKBOX_PREVIEW );
            break;

        default:
            throw lang::IllegalArgumentException(
                    "Unknown template",
                    static_cast< XFilePicker2* >( this ),
                    1 );
    }

    if( !m_aCustomWidgetsMap.empty() )
    {
        QHBoxLayout *pHBoxLayout = new QHBoxLayout();
        pLayout->addLayout( pHBoxLayout, pLayout->rowCount(), 0, 1, 2 );
        pHBoxLayout->addStretch();
        QPushButton *pButton = new QPushButton( "Ok" );
        connect( pButton, SIGNAL(clicked()), m_pOptionsDialog, SLOT(accept()) );
        pHBoxLayout->addWidget( pButton );
        pButton = new QPushButton( "Cancel" );
        connect( pButton, SIGNAL(clicked()), m_pOptionsDialog, SLOT(reject()) );
        pHBoxLayout->addWidget( pButton );
    }

    const char *resId = nullptr;
    switch ( acceptMode )
    {
    case QFileDialog::AcceptOpen:
        resId = STR_FPICKER_OPEN;
        break;
    case QFileDialog::AcceptSave:
        resId = STR_FPICKER_SAVE;
        m_pFileDialog->setFileMode( QFileDialog::AnyFile );
        break;
    }

    m_pFileDialog->setAcceptMode( acceptMode );
    m_pFileDialog->setWindowTitle( getResString(resId) );
}

void SAL_CALL Qt5FilePicker::cancel()
{

}

void SAL_CALL Qt5FilePicker::disposing( const lang::EventObject &rEvent )
{
    uno::Reference<XFilePickerListener> xFilePickerListener( rEvent.Source, uno::UNO_QUERY );

    if ( xFilePickerListener.is() )
    {
        removeFilePickerListener( xFilePickerListener );
    }
}

OUString SAL_CALL Qt5FilePicker::getImplementationName()
{
    return OUString( "com.sun.star.ui.dialogs.Qt5FilePicker" );
}

sal_Bool SAL_CALL Qt5FilePicker::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

uno::Sequence< OUString > SAL_CALL Qt5FilePicker::getSupportedServiceNames()
{
    return FilePicker_getSupportedServiceNames();
}

void Qt5FilePicker::filterSelected(const QString &)
{
    FilePickerEvent aEvent;
    aEvent.ElementId = LISTBOX_FILTER;
    SAL_INFO( "vcl", "filter changed" );
    if (m_xListener.is())
        m_xListener->controlStateChanged( aEvent );
}

void Qt5FilePicker::selectionChanged()
{
    FilePickerEvent aEvent;
    SAL_INFO( "vcl", "file selection changed" );
    if (m_xListener.is())
        m_xListener->fileSelectionChanged( aEvent );
}

OUString Qt5FilePicker::getDirectory()
{
    uno::Sequence< OUString > seq = getSelectedFiles();
    if (seq.getLength() > 1)
        seq.realloc(1);
    return seq[ 0 ];
}

void Qt5FilePicker::setDescription( const OUString& )
{

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
