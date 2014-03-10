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

#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <cppuhelper/interfacecontainer.h>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/CommonFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/ControlActions.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>

#include <osl/mutex.hxx>

#include <vcl/fpicker.hrc>
#include <vcl/svapp.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/syswin.hxx>

#include "osl/file.h"

#include "KDE4FilePicker.hxx"
#include "FPServiceInfo.hxx"

/* ********* Hack, but needed because of conflicting types... */
#define Region QtXRegion

#include <kfiledialog.h>
#include <kwindowsystem.h>
#include <kapplication.h>
#include <kfilefiltercombo.h>
#include <kfilewidget.h>
#include <kdiroperator.h>
#include <kservicetypetrader.h>
#include <kmessagebox.h>

#include <qclipboard.h>
#include <QWidget>
#include <QCheckBox>
#include <QGridLayout>

#undef Region

#include "generic/geninst.h"

using namespace ::com::sun::star;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::ui::dialogs::TemplateDescription;
using namespace ::com::sun::star::ui::dialogs::ExtendedFilePickerElementIds;
using namespace ::com::sun::star::ui::dialogs::CommonFilePickerElementIds;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;

// The dialog should check whether LO also supports the protocol
// provided by KIO, and KFileWidget::dirOperator() is only 4.3+ .
// Moreover it's only in this somewhat internal KFileWidget class,
// which may not necessarily be what KFileDialog::fileWidget() returns,
// but that's hopefully not a problem in practice.
#if KDE_VERSION_MAJOR == 4 && KDE_VERSION_MINOR >= 2
#define ALLOW_REMOTE_URLS 1
#else
#define ALLOW_REMOTE_URLS 0
#endif

//////////////////////////////////////////////////////////////////////////
// helper functions
//////////////////////////////////////////////////////////////////////////

#include <QDebug>

namespace
{
    uno::Sequence<OUString> SAL_CALL FilePicker_getSupportedServiceNames()
    {
        uno::Sequence<OUString> aRet(3);
        aRet[0] = "com.sun.star.ui.dialogs.FilePicker";
        aRet[1] = "com.sun.star.ui.dialogs.SystemFilePicker";
        aRet[2] = "com.sun.star.ui.dialogs.KDE4FilePicker";
        return aRet;
    }
}

OUString toOUString(const QString& s)
{
    // QString stores UTF16, just like OUString
    return OUString(reinterpret_cast<const sal_Unicode*>(s.data()), s.length());
}

QString toQString(const OUString& s)
{
    return QString::fromUtf16(s.getStr(), s.getLength());
}

//////////////////////////////////////////////////////////////////////////
// KDE4FilePicker
//////////////////////////////////////////////////////////////////////////

KDE4FilePicker::KDE4FilePicker( const uno::Reference<uno::XComponentContext>& )
    : KDE4FilePicker_Base(_helperMutex)
    , _resMgr( ResMgr::CreateResMgr("fps_office") )
    , allowRemoteUrls( false )
{
    _extraControls = new QWidget();
    _layout = new QGridLayout(_extraControls);

    _dialog = new KFileDialog(KUrl("~"), QString(""), 0, _extraControls);
#if ALLOW_REMOTE_URLS
    if( KFileWidget* fileWidget = dynamic_cast< KFileWidget* >( _dialog->fileWidget()))
    {
        allowRemoteUrls = true;
        // Use finishedLoading signal rather than e.g. urlEntered, because if there's a problem
        // such as the URL being mistyped, there's no way to prevent two message boxes about it,
        // one from us and one from KDE code.
        connect( fileWidget->dirOperator(), SIGNAL( finishedLoading()), SLOT( checkProtocol()));
    }
#endif

    setMultiSelectionMode( false );
    //default mode
    _dialog->setOperationMode(KFileDialog::Opening);

    // XExecutableDialog functions
    connect( this, SIGNAL( setTitleSignal( const OUString & ) ),
             this, SLOT( setTitleSlot( const OUString & ) ), Qt::BlockingQueuedConnection );
    connect( this, SIGNAL( executeSignal() ),
             this, SLOT( executeSlot() ), Qt::BlockingQueuedConnection );

    // XFilePicker functions
    connect( this, SIGNAL( setMultiSelectionModeSignal( sal_Bool ) ),
             this, SLOT( setMultiSelectionModeSlot( sal_Bool ) ), Qt::BlockingQueuedConnection );
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
    connect( this, SIGNAL( appendFilterGroupSignal( const OUString &, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::StringPair > & ) ),
             this, SLOT( appendFilterGroupSlot( const OUString &, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::StringPair > & ) ), Qt::BlockingQueuedConnection );

    // XFilePickerControlAccess functions
    connect( this, SIGNAL( setValueSignal( sal_Int16, sal_Int16, const ::com::sun::star::uno::Any & ) ),
             this, SLOT( setValueSlot( sal_Int16, sal_Int16, const ::com::sun::star::uno::Any & ) ), Qt::BlockingQueuedConnection );
    connect( this, SIGNAL( getValueSignal( sal_Int16, sal_Int16 ) ),
             this, SLOT( getValueSlot( sal_Int16, sal_Int16 ) ), Qt::BlockingQueuedConnection );
    connect( this, SIGNAL( enableControlSignal( sal_Int16, sal_Bool ) ),
             this, SLOT( enableControlSlot( sal_Int16, sal_Bool ) ), Qt::BlockingQueuedConnection );
    connect( this, SIGNAL( setLabelSignal( sal_Int16, const OUString & ) ),
             this, SLOT( setLabelSlot( sal_Int16, const OUString & ) ), Qt::BlockingQueuedConnection );
    connect( this, SIGNAL( getLabelSignal( sal_Int16 ) ),
             this, SLOT( getLabelSlot( sal_Int16 ) ), Qt::BlockingQueuedConnection );

    // XFilePicker2 functions
    connect( this, SIGNAL( getSelectedFilesSignal() ),
             this, SLOT( getSelectedFilesSlot() ), Qt::BlockingQueuedConnection );

    // XInitialization
    connect( this, SIGNAL( initializeSignal( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > & ) ),
             this, SLOT( initializeSlot( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > & ) ), Qt::BlockingQueuedConnection );

    // Destructor proxy
    connect( this, SIGNAL( cleanupProxySignal() ), this, SLOT( cleanupProxy() ), Qt::BlockingQueuedConnection );

    connect( this, SIGNAL( checkProtocolSignal() ), this, SLOT( checkProtocol() ), Qt::BlockingQueuedConnection );

    // XFilePickerListener notifications
    connect( _dialog, SIGNAL( filterChanged(const QString&) ), this, SLOT( filterChanged(const QString&) ));
    connect( _dialog, SIGNAL( selectionChanged() ), this, SLOT( selectionChanged() ));
}

KDE4FilePicker::~KDE4FilePicker()
{
    cleanupProxy();
}

void KDE4FilePicker::cleanupProxy()
{
    if( qApp->thread() != QThread::currentThread() ) {
        SalYieldMutexReleaser aReleaser;
        return Q_EMIT cleanupProxySignal();
    }
    delete _resMgr;
    delete _dialog;
}

void SAL_CALL KDE4FilePicker::addFilePickerListener( const uno::Reference<XFilePickerListener>& xListener )
    throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    m_xListener = xListener;
}

void SAL_CALL KDE4FilePicker::removeFilePickerListener( const uno::Reference<XFilePickerListener>& )
    throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    m_xListener.clear();
}

void SAL_CALL KDE4FilePicker::setTitle( const OUString &title )
    throw( uno::RuntimeException )
{
    if( qApp->thread() != QThread::currentThread() ) {
        SalYieldMutexReleaser aReleaser;
        return Q_EMIT setTitleSignal( title );
    }

    _dialog->setCaption(toQString(title));
}

sal_Int16 SAL_CALL KDE4FilePicker::execute()
    throw( uno::RuntimeException )
{
    if( qApp->thread() != QThread::currentThread() ) {
        SalYieldMutexReleaser aReleaser;
        return Q_EMIT executeSignal();
    }

    //get the window id of the main OO window to set it for the dialog as a parent
    Window *pParentWin = Application::GetDefDialogParent();
    if ( pParentWin )
    {
        const SystemEnvData* pSysData = ((SystemWindow *)pParentWin)->GetSystemData();
        if ( pSysData )
        {
            KWindowSystem::setMainWindow( _dialog, pSysData->aWindow); // unx only
        }
    }

    _dialog->clearFilter();
    _dialog->setFilter(_filter);
    _dialog->filterWidget()->setEditable(false);

    // We're entering a nested loop.
    // Release the yield mutex to prevent deadlocks.
    int result = _dialog->exec();

    // HACK: KFileDialog uses KConfig("kdeglobals") for saving some settings
    // (such as the auto-extension flag), but that doesn't update KGlobal::config()
    // (which is probably a KDE bug), so force reading the new configuration,
    // otherwise the next opening of the dialog would use the old settings.
    KGlobal::config()->reparseConfiguration();

    if( result == KFileDialog::Accepted)
        return ExecutableDialogResults::OK;

    return ExecutableDialogResults::CANCEL;
}

void SAL_CALL KDE4FilePicker::setMultiSelectionMode( sal_Bool multiSelect )
    throw( uno::RuntimeException )
{
    if( qApp->thread() != QThread::currentThread() ) {
        SalYieldMutexReleaser release;
        return Q_EMIT setMultiSelectionModeSignal( multiSelect );
    }

    if( allowRemoteUrls )
    {
        if (multiSelect)
            _dialog->setMode(KFile::Files);
        else
            _dialog->setMode(KFile::File);
    }
    else
    {
        if (multiSelect)
            _dialog->setMode(KFile::Files | KFile::LocalOnly);
        else
            _dialog->setMode(KFile::File | KFile::LocalOnly);
    }
}

void SAL_CALL KDE4FilePicker::setDefaultName( const OUString &name )
    throw( uno::RuntimeException )
{
    if( qApp->thread() != QThread::currentThread() ) {
        SalYieldMutexReleaser release;
        return Q_EMIT setDefaultNameSignal( name );
    }

    const QString url = toQString(name);
    _dialog->setSelection(url);
}

void SAL_CALL KDE4FilePicker::setDisplayDirectory( const OUString &dir )
    throw( uno::RuntimeException )
{
    if( qApp->thread() != QThread::currentThread() ) {
        SalYieldMutexReleaser release;
        return Q_EMIT setDisplayDirectorySignal( dir );
    }

    const QString url = toQString(dir);
    _dialog->setUrl(KUrl(url));
}

OUString SAL_CALL KDE4FilePicker::getDisplayDirectory()
    throw( uno::RuntimeException )
{
    if( qApp->thread() != QThread::currentThread() ) {
        SalYieldMutexReleaser release;
        return Q_EMIT getDisplayDirectorySignal();
    }

    QString dir = _dialog->baseUrl().url();
    return toOUString(dir);
}

uno::Sequence< OUString > SAL_CALL KDE4FilePicker::getFiles()
    throw( uno::RuntimeException )
{
    if( qApp->thread() != QThread::currentThread() ) {
        SalYieldMutexReleaser release;
        return Q_EMIT getFilesSignal();
    }

    KUrl::List urls = _dialog->selectedUrls();
    uno::Sequence< OUString > seq( urls.size());
    int i = 0;
    foreach( const KUrl& url, urls )
        seq[ i++ ]= toOUString( url.url());
    return seq;
}

uno::Sequence< OUString > SAL_CALL KDE4FilePicker::getSelectedFiles()
    throw( uno::RuntimeException )
{
    if( qApp->thread() != QThread::currentThread() ) {
        SalYieldMutexReleaser release;
        return Q_EMIT getSelectedFilesSignal();
    }

    return getFiles();
}

void SAL_CALL KDE4FilePicker::appendFilter( const OUString &title, const OUString &filter )
    throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    if( qApp->thread() != QThread::currentThread() ) {
        SalYieldMutexReleaser release;
        return Q_EMIT appendFilterSignal( title, filter );
    }

    QString t = toQString(title);
    QString f = toQString(filter);

    if (!_filter.isNull())
        _filter.append("\n");

    // '/' need to be escaped else they are assumed to be mime types by kfiledialog
    //see the docs
    t.replace("/", "\\/");

    // openoffice gives us filters separated by ';' qt dialogs just want space separated
    f.replace(";", " ");

    // make sure "*.*" is not used as "all files"
    f.replace("*.*", "*");

    _filter.append(QString("%1|%2").arg(f).arg(t));
}

void SAL_CALL KDE4FilePicker::setCurrentFilter( const OUString &title )
    throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    if( qApp->thread() != QThread::currentThread() ) {
        SalYieldMutexReleaser release;
        return Q_EMIT setCurrentFilterSignal( title );
    }

    QString t = toQString(title);
    t.replace("/", "\\/");
    _dialog->filterWidget()->setCurrentFilter(t);
}

OUString SAL_CALL KDE4FilePicker::getCurrentFilter()
    throw( uno::RuntimeException )
{
    if( qApp->thread() != QThread::currentThread() ) {
        SalYieldMutexReleaser release;
        return Q_EMIT getCurrentFilterSignal();
    }

    // _dialog->currentFilter() wouldn't quite work, because it returns only e.g. "*.doc",
    // without the description, and there may be several filters with the same pattern
    QString filter = _dialog->filterWidget()->currentText();
    filter = filter.mid( filter.indexOf( '|' ) + 1 ); // convert from the pattern|description format if needed
    filter.replace( "\\/", "/" );

    //default if not found
    if (filter.isNull())
        filter = "ODF Text Document (.odt)";

    return toOUString(filter);
}

void SAL_CALL KDE4FilePicker::appendFilterGroup( const OUString& rGroupTitle, const uno::Sequence<beans::StringPair>& filters)
    throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    if( qApp->thread() != QThread::currentThread() ) {
        SalYieldMutexReleaser release;
        return Q_EMIT appendFilterGroupSignal( rGroupTitle, filters );
    }

    const sal_uInt16 length = filters.getLength();
    for (sal_uInt16 i = 0; i < length; ++i)
    {
        beans::StringPair aPair = filters[i];
        appendFilter( aPair.First, aPair.Second );
    }
}

void SAL_CALL KDE4FilePicker::setValue( sal_Int16 controlId, sal_Int16 nControlAction, const uno::Any &value )
    throw( uno::RuntimeException )
{
    if( qApp->thread() != QThread::currentThread() ) {
        SalYieldMutexReleaser release;
        return Q_EMIT setValueSignal( controlId, nControlAction, value );
    }

    QWidget* widget = _customWidgets[controlId];

    if (widget)
    {
        switch (controlId)
        {
            case ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION:
            // we actually rely on KFileDialog and ignore CHECKBOX_AUTOEXTENSION completely,
            // otherwise the checkbox would be duplicated
                break;
            case ExtendedFilePickerElementIds::CHECKBOX_PASSWORD:
            case ExtendedFilePickerElementIds::CHECKBOX_FILTEROPTIONS:
            case ExtendedFilePickerElementIds::CHECKBOX_READONLY:
            case ExtendedFilePickerElementIds::CHECKBOX_LINK:
            case ExtendedFilePickerElementIds::CHECKBOX_PREVIEW:
            case ExtendedFilePickerElementIds::CHECKBOX_SELECTION:
            {
                QCheckBox* cb = dynamic_cast<QCheckBox*>(widget);
                cb->setChecked(value.get<bool>());
                break;
            }
            case ExtendedFilePickerElementIds::PUSHBUTTON_PLAY:
            case ExtendedFilePickerElementIds::LISTBOX_VERSION:
            case ExtendedFilePickerElementIds::LISTBOX_TEMPLATE:
            case ExtendedFilePickerElementIds::LISTBOX_IMAGE_TEMPLATE:
            case ExtendedFilePickerElementIds::LISTBOX_VERSION_LABEL:
            case ExtendedFilePickerElementIds::LISTBOX_TEMPLATE_LABEL:
            case ExtendedFilePickerElementIds::LISTBOX_IMAGE_TEMPLATE_LABEL:
            case ExtendedFilePickerElementIds::LISTBOX_FILTER_SELECTOR:
                break;
        }
    }
}

uno::Any SAL_CALL KDE4FilePicker::getValue( sal_Int16 controlId, sal_Int16 nControlAction )
    throw( uno::RuntimeException )
{
    if( qApp->thread() != QThread::currentThread() ) {
        SalYieldMutexReleaser release;
        return Q_EMIT getValueSignal( controlId, nControlAction );
    }

    uno::Any res(false);

    QWidget* widget = _customWidgets[controlId];

    if (widget)
    {
        switch (controlId)
        {
            case ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION:
            // We ignore this one and rely on KFileDialog to provide the function.
            // Always return false, to pretend we do not support this, otherwise
            // LO core would try to be smart and cut the extension in some places,
            // interfering with KFileDialog's handling of it. KFileDialog also
            // saves the value of the setting, so LO core is not needed for that either.
                res = uno::Any( false );
                break;
            case ExtendedFilePickerElementIds::CHECKBOX_PASSWORD:
            case ExtendedFilePickerElementIds::CHECKBOX_FILTEROPTIONS:
            case ExtendedFilePickerElementIds::CHECKBOX_READONLY:
            case ExtendedFilePickerElementIds::CHECKBOX_LINK:
            case ExtendedFilePickerElementIds::CHECKBOX_PREVIEW:
            case ExtendedFilePickerElementIds::CHECKBOX_SELECTION:
            {
                QCheckBox* cb = dynamic_cast<QCheckBox*>(widget);
                res = uno::Any(cb->isChecked());
                break;
            }
            case ExtendedFilePickerElementIds::PUSHBUTTON_PLAY:
            case ExtendedFilePickerElementIds::LISTBOX_VERSION:
            case ExtendedFilePickerElementIds::LISTBOX_TEMPLATE:
            case ExtendedFilePickerElementIds::LISTBOX_IMAGE_TEMPLATE:
            case ExtendedFilePickerElementIds::LISTBOX_VERSION_LABEL:
            case ExtendedFilePickerElementIds::LISTBOX_TEMPLATE_LABEL:
            case ExtendedFilePickerElementIds::LISTBOX_IMAGE_TEMPLATE_LABEL:
            case ExtendedFilePickerElementIds::LISTBOX_FILTER_SELECTOR:
                break;
        }
    }

    return res;
}

void SAL_CALL KDE4FilePicker::enableControl( sal_Int16 controlId, sal_Bool enable )
    throw( uno::RuntimeException )
{
    if( qApp->thread() != QThread::currentThread() ) {
        SalYieldMutexReleaser release;
        return Q_EMIT enableControlSignal( controlId, enable );
    }

    QWidget* widget = _customWidgets[controlId];

    if (widget)
    {
        widget->setEnabled(enable);
    }
}

void SAL_CALL KDE4FilePicker::setLabel( sal_Int16 controlId, const OUString &label )
    throw( uno::RuntimeException )
{
    if( qApp->thread() != QThread::currentThread() ) {
        SalYieldMutexReleaser release;
        return Q_EMIT setLabelSignal( controlId, label );
    }

    QWidget* widget = _customWidgets[controlId];

    if (widget)
    {
        switch (controlId)
        {
            case ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION: // ignored
            case ExtendedFilePickerElementIds::CHECKBOX_PASSWORD:
            case ExtendedFilePickerElementIds::CHECKBOX_FILTEROPTIONS:
            case ExtendedFilePickerElementIds::CHECKBOX_READONLY:
            case ExtendedFilePickerElementIds::CHECKBOX_LINK:
            case ExtendedFilePickerElementIds::CHECKBOX_PREVIEW:
            case ExtendedFilePickerElementIds::CHECKBOX_SELECTION:
            {
                QCheckBox* cb = dynamic_cast<QCheckBox*>(widget);
                cb->setText(toQString(label));
                break;
            }
            case ExtendedFilePickerElementIds::PUSHBUTTON_PLAY:
            case ExtendedFilePickerElementIds::LISTBOX_VERSION:
            case ExtendedFilePickerElementIds::LISTBOX_TEMPLATE:
            case ExtendedFilePickerElementIds::LISTBOX_IMAGE_TEMPLATE:
            case ExtendedFilePickerElementIds::LISTBOX_VERSION_LABEL:
            case ExtendedFilePickerElementIds::LISTBOX_TEMPLATE_LABEL:
            case ExtendedFilePickerElementIds::LISTBOX_IMAGE_TEMPLATE_LABEL:
            case ExtendedFilePickerElementIds::LISTBOX_FILTER_SELECTOR:
                break;
        }
    }
}

OUString SAL_CALL KDE4FilePicker::getLabel(sal_Int16 controlId)
    throw ( uno::RuntimeException )
{
    if( qApp->thread() != QThread::currentThread() ) {
        SalYieldMutexReleaser release;
        return Q_EMIT getLabelSignal( controlId );
    }

    QWidget* widget = _customWidgets[controlId];
    QString label;

    if (widget)
    {
        switch (controlId)
        {
            case ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION: // ignored
            case ExtendedFilePickerElementIds::CHECKBOX_PASSWORD:
            case ExtendedFilePickerElementIds::CHECKBOX_FILTEROPTIONS:
            case ExtendedFilePickerElementIds::CHECKBOX_READONLY:
            case ExtendedFilePickerElementIds::CHECKBOX_LINK:
            case ExtendedFilePickerElementIds::CHECKBOX_PREVIEW:
            case ExtendedFilePickerElementIds::CHECKBOX_SELECTION:
            {
                QCheckBox* cb = dynamic_cast<QCheckBox*>(widget);
                label = cb->text();
                break;
            }
            case ExtendedFilePickerElementIds::PUSHBUTTON_PLAY:
            case ExtendedFilePickerElementIds::LISTBOX_VERSION:
            case ExtendedFilePickerElementIds::LISTBOX_TEMPLATE:
            case ExtendedFilePickerElementIds::LISTBOX_IMAGE_TEMPLATE:
            case ExtendedFilePickerElementIds::LISTBOX_VERSION_LABEL:
            case ExtendedFilePickerElementIds::LISTBOX_TEMPLATE_LABEL:
            case ExtendedFilePickerElementIds::LISTBOX_IMAGE_TEMPLATE_LABEL:
            case ExtendedFilePickerElementIds::LISTBOX_FILTER_SELECTOR:
                break;
        }
    }
    return toOUString(label);
}

void KDE4FilePicker::addCustomControl(sal_Int16 controlId)
{
    QWidget* widget = 0;
    sal_Int32 resId = -1;

    switch (controlId)
    {
        case ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION:
            resId = STR_SVT_FILEPICKER_AUTO_EXTENSION;
            break;
        case ExtendedFilePickerElementIds::CHECKBOX_PASSWORD:
            resId = STR_SVT_FILEPICKER_PASSWORD;
            break;
        case ExtendedFilePickerElementIds::CHECKBOX_FILTEROPTIONS:
            resId = STR_SVT_FILEPICKER_FILTER_OPTIONS;
            break;
        case ExtendedFilePickerElementIds::CHECKBOX_READONLY:
            resId = STR_SVT_FILEPICKER_READONLY;
            break;
        case ExtendedFilePickerElementIds::CHECKBOX_LINK:
            resId = STR_SVT_FILEPICKER_INSERT_AS_LINK;
            break;
        case ExtendedFilePickerElementIds::CHECKBOX_PREVIEW:
            resId = STR_SVT_FILEPICKER_SHOW_PREVIEW;
            break;
        case ExtendedFilePickerElementIds::CHECKBOX_SELECTION:
            resId = STR_SVT_FILEPICKER_SELECTION;
            break;
        case ExtendedFilePickerElementIds::PUSHBUTTON_PLAY:
            resId = STR_SVT_FILEPICKER_PLAY;
            break;
        case ExtendedFilePickerElementIds::LISTBOX_VERSION:
            resId = STR_SVT_FILEPICKER_VERSION;
            break;
        case ExtendedFilePickerElementIds::LISTBOX_TEMPLATE:
            resId = STR_SVT_FILEPICKER_TEMPLATES;
            break;
        case ExtendedFilePickerElementIds::LISTBOX_IMAGE_TEMPLATE:
            resId = STR_SVT_FILEPICKER_IMAGE_TEMPLATE;
            break;
        case ExtendedFilePickerElementIds::LISTBOX_VERSION_LABEL:
        case ExtendedFilePickerElementIds::LISTBOX_TEMPLATE_LABEL:
        case ExtendedFilePickerElementIds::LISTBOX_IMAGE_TEMPLATE_LABEL:
        case ExtendedFilePickerElementIds::LISTBOX_FILTER_SELECTOR:
            break;
    }

    switch (controlId)
    {
        case ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION:
        case ExtendedFilePickerElementIds::CHECKBOX_PASSWORD:
        case ExtendedFilePickerElementIds::CHECKBOX_FILTEROPTIONS:
        case ExtendedFilePickerElementIds::CHECKBOX_READONLY:
        case ExtendedFilePickerElementIds::CHECKBOX_LINK:
        case ExtendedFilePickerElementIds::CHECKBOX_PREVIEW:
        case ExtendedFilePickerElementIds::CHECKBOX_SELECTION:
        {
            QString label;

            if (_resMgr && resId != -1)
            {
                OUString s(ResId(resId, *_resMgr).toString());
                label = toQString(s);
                label.replace("~", "&");
            }

            widget = new QCheckBox(label, _extraControls);
            // the checkbox is created even for CHECKBOX_AUTOEXTENSION to simplify
            // code, but the checkbox is hidden and ignored
            if( controlId == ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION )
                widget->hide();

            break;
        }
        case ExtendedFilePickerElementIds::PUSHBUTTON_PLAY:
        case ExtendedFilePickerElementIds::LISTBOX_VERSION:
        case ExtendedFilePickerElementIds::LISTBOX_TEMPLATE:
        case ExtendedFilePickerElementIds::LISTBOX_IMAGE_TEMPLATE:
        case ExtendedFilePickerElementIds::LISTBOX_VERSION_LABEL:
        case ExtendedFilePickerElementIds::LISTBOX_TEMPLATE_LABEL:
        case ExtendedFilePickerElementIds::LISTBOX_IMAGE_TEMPLATE_LABEL:
        case ExtendedFilePickerElementIds::LISTBOX_FILTER_SELECTOR:
            break;
    }

    if (widget)
    {
        _layout->addWidget(widget);
        _customWidgets.insert(controlId, widget);
    }
}

void SAL_CALL KDE4FilePicker::initialize( const uno::Sequence<uno::Any> &args )
    throw( uno::Exception, uno::RuntimeException )
{
    if( qApp->thread() != QThread::currentThread() ) {
        SalYieldMutexReleaser release;
        return Q_EMIT initializeSignal( args );
    }

    _filter.clear();

    // parameter checking
    uno::Any arg;
    if (args.getLength() == 0)
    {
        throw lang::IllegalArgumentException(
                OUString( "no arguments" ),
                static_cast< XFilePicker2* >( this ), 1 );
    }

    arg = args[0];

    if (( arg.getValueType() != ::getCppuType((sal_Int16*)0)) &&
        ( arg.getValueType() != ::getCppuType((sal_Int8*)0)))
    {
        throw lang::IllegalArgumentException(
                OUString( "invalid argument type" ),
                static_cast< XFilePicker2* >( this ), 1 );
    }

    sal_Int16 templateId = -1;
    arg >>= templateId;

    //default is opening
    KFileDialog::OperationMode operationMode = KFileDialog::Opening;

    switch ( templateId )
    {
        case FILEOPEN_SIMPLE:
            break;

        case FILESAVE_SIMPLE:
            operationMode = KFileDialog::Saving;
            break;

        case FILESAVE_AUTOEXTENSION:
            operationMode = KFileDialog::Saving;
            addCustomControl( ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION );
            break;

        case FILESAVE_AUTOEXTENSION_PASSWORD:
        {
            operationMode = KFileDialog::Saving;
            addCustomControl( ExtendedFilePickerElementIds::CHECKBOX_PASSWORD );
            break;
        }
        case FILESAVE_AUTOEXTENSION_PASSWORD_FILTEROPTIONS:
        {
            operationMode = KFileDialog::Saving;
            addCustomControl( ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION );
            addCustomControl( ExtendedFilePickerElementIds::CHECKBOX_PASSWORD );
            addCustomControl( ExtendedFilePickerElementIds::CHECKBOX_FILTEROPTIONS );
            break;
        }
        case FILESAVE_AUTOEXTENSION_SELECTION:
            operationMode = KFileDialog::Saving;
            addCustomControl( ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION );
            addCustomControl( ExtendedFilePickerElementIds::CHECKBOX_SELECTION );
            break;

        case FILESAVE_AUTOEXTENSION_TEMPLATE:
            operationMode = KFileDialog::Saving;
            addCustomControl( ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION );
            addCustomControl( ExtendedFilePickerElementIds::LISTBOX_TEMPLATE );
            break;

        case FILEOPEN_LINK_PREVIEW_IMAGE_TEMPLATE:
            addCustomControl( ExtendedFilePickerElementIds::CHECKBOX_LINK );
            addCustomControl( ExtendedFilePickerElementIds::CHECKBOX_PREVIEW );
            addCustomControl( ExtendedFilePickerElementIds::LISTBOX_IMAGE_TEMPLATE );
            break;

        case FILEOPEN_PLAY:
            addCustomControl( ExtendedFilePickerElementIds::PUSHBUTTON_PLAY );
            break;

        case FILEOPEN_READONLY_VERSION:
            addCustomControl( ExtendedFilePickerElementIds::CHECKBOX_READONLY );
            addCustomControl( ExtendedFilePickerElementIds::LISTBOX_VERSION );
            break;

        case FILEOPEN_LINK_PREVIEW:
            addCustomControl( ExtendedFilePickerElementIds::CHECKBOX_LINK );
            addCustomControl( ExtendedFilePickerElementIds::CHECKBOX_PREVIEW );
            break;

        default:
            throw lang::IllegalArgumentException(
                    OUString( "Unknown template" ),
                    static_cast< XFilePicker2* >( this ),
                    1 );
    }

    _dialog->setOperationMode(operationMode);
    _dialog->setConfirmOverwrite(true);
}

void SAL_CALL KDE4FilePicker::cancel()
    throw ( uno::RuntimeException )
{

}

void SAL_CALL KDE4FilePicker::disposing( const lang::EventObject &rEvent )
    throw( uno::RuntimeException )
{
    uno::Reference<XFilePickerListener> xFilePickerListener( rEvent.Source, uno::UNO_QUERY );

    if ( xFilePickerListener.is() )
    {
        removeFilePickerListener( xFilePickerListener );
    }
}

OUString SAL_CALL KDE4FilePicker::getImplementationName()
    throw( uno::RuntimeException )
{
    return OUString( FILE_PICKER_IMPL_NAME );
}

sal_Bool SAL_CALL KDE4FilePicker::supportsService( const OUString& ServiceName )
    throw( uno::RuntimeException )
{
    return cppu::supportsService(this, ServiceName);
}

uno::Sequence< OUString > SAL_CALL KDE4FilePicker::getSupportedServiceNames()
    throw( uno::RuntimeException )
{
    return FilePicker_getSupportedServiceNames();
}

void KDE4FilePicker::checkProtocol()
{
    if( qApp->thread() != QThread::currentThread() ) {
        SalYieldMutexReleaser release;
        return Q_EMIT checkProtocolSignal();
    }

    // There's no libreoffice.desktop :(, so find a matching one.
    KService::List services = KServiceTypeTrader::self()->query( "Application", "Exec =~ 'libreoffice %U'" );
    QStringList protocols;
    if( !services.isEmpty())
        protocols = services[ 0 ]->property( "X-KDE-Protocols" ).toStringList();
    if( protocols.isEmpty()) // incorrect (developer?) installation ?
        protocols << "file" << "http";
    if( !protocols.contains( _dialog->baseUrl().protocol()) && !protocols.contains( "KIO" ))
        KMessageBox::error( _dialog, KIO::buildErrorString( KIO::ERR_UNSUPPORTED_PROTOCOL, _dialog->baseUrl().protocol()));
}

void KDE4FilePicker::filterChanged(const QString &)
{
    FilePickerEvent aEvent;
    aEvent.ElementId = LISTBOX_FILTER;
    OSL_TRACE( "filter changed" );
    if (m_xListener.is())
        m_xListener->controlStateChanged( aEvent );
}

void KDE4FilePicker::selectionChanged()
{
    FilePickerEvent aEvent;
    OSL_TRACE( "file selection changed" );
    if (m_xListener.is())
        m_xListener->fileSelectionChanged( aEvent );
}

#include "KDE4FilePicker.moc"

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
