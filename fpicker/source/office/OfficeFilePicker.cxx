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


#include "OfficeFilePicker.hxx"
#include "iodlg.hxx"
#include "RemoteFilesDialog.hxx"

#include <list>
#include <algorithm>
#include <sal/log.hxx>
#include <tools/urlobj.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/ui/dialogs/FilePickerEvent.hpp>
#include <com/sun/star/ui/dialogs/FilePreviewImageFormats.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/beans/StringPair.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/NamedValue.hpp>
#include <unotools/ucbhelper.hxx>
#include <unotools/pathoptions.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <vcl/svapp.hxx>

using namespace     ::com::sun::star::container;
using namespace     ::com::sun::star::lang;
using namespace     ::com::sun::star::ui::dialogs;
using namespace     ::com::sun::star::uno;
using namespace     ::com::sun::star::beans;
using namespace     ::com::sun::star::awt;
using namespace     ::utl;


struct FilterEntry
{
protected:
    OUString     m_sTitle;
    OUString     m_sFilter;

    UnoFilterList       m_aSubFilters;

public:
    FilterEntry( const OUString& _rTitle, const OUString& _rFilter )
        :m_sTitle( _rTitle )
        ,m_sFilter( _rFilter )
    {
    }

    FilterEntry( const OUString& _rTitle, const UnoFilterList& _rSubFilters );

    const OUString& getTitle() const { return m_sTitle; }
    const OUString& getFilter() const { return m_sFilter; }

    /// determines if the filter has sub filter (i.e., the filter is a filter group in real)
    bool            hasSubFilters( ) const;

    /** retrieves the filters belonging to the entry
    */
    void            getSubFilters( UnoFilterList& _rSubFilterList );

    // helpers for iterating the sub filters
    const UnoFilterEntry*   beginSubFilters() const { return m_aSubFilters.getConstArray(); }
    const UnoFilterEntry*   endSubFilters() const { return m_aSubFilters.getConstArray() + m_aSubFilters.getLength(); }
};


FilterEntry::FilterEntry( const OUString& _rTitle, const UnoFilterList& _rSubFilters )
    :m_sTitle( _rTitle )
    ,m_aSubFilters( _rSubFilters )
{
}


bool FilterEntry::hasSubFilters( ) const
{
    return ( 0 < m_aSubFilters.getLength() );
}


void FilterEntry::getSubFilters( UnoFilterList& _rSubFilterList )
{
    _rSubFilterList = m_aSubFilters;
}

struct ElementEntry_Impl
{
    sal_Int16       m_nElementID;
    sal_Int16       m_nControlAction;
    Any         m_aValue;
    OUString       m_aLabel;
    bool        m_bEnabled      : 1;

    bool        m_bHasValue     : 1;
    bool        m_bHasLabel     : 1;
    bool        m_bHasEnabled   : 1;

    explicit        ElementEntry_Impl( sal_Int16 nId );

    void            setValue( const Any& rVal ) { m_aValue = rVal; m_bHasValue = true; }
    void            setAction( sal_Int16 nAction ) { m_nControlAction = nAction; }
    void            setLabel( const OUString& rVal ) { m_aLabel = rVal; m_bHasLabel = true; }
    void            setEnabled( bool bEnabled ) { m_bEnabled = bEnabled; m_bHasEnabled = true; }
};

ElementEntry_Impl::ElementEntry_Impl( sal_Int16 nId )
    : m_nElementID( nId )
    , m_nControlAction( 0 )
    , m_bEnabled( false )
    , m_bHasValue( false )
    , m_bHasLabel( false )
    , m_bHasEnabled( false )
{}


void SvtFilePicker::prepareExecute()
{
    // set the default directory
    // --**-- doesn't match the spec yet
    if ( !m_aDisplayDirectory.isEmpty() || !m_aDefaultName.isEmpty() )
    {
        bool isFileSet = false;
        if ( !m_aDisplayDirectory.isEmpty() )
        {

            INetURLObject aPath;
            INetURLObject givenPath( m_aDisplayDirectory );
            if (!givenPath.HasError())
                aPath = givenPath;
            else
            {
                INetURLObject aStdDirObj( SvtPathOptions().GetWorkPath() );
                aPath = aStdDirObj;
            }
            if ( !m_aDefaultName.isEmpty() )
            {
                aPath.insertName( m_aDefaultName );
                getDialog()->SetHasFilename( true );
            }
            getDialog()->SetPath( aPath.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );
            isFileSet = true;
        }
        if ( !isFileSet && !m_aDefaultName.isEmpty() )
        {
            getDialog()->SetPath( m_aDefaultName );
            getDialog()->SetHasFilename( true );
        }
    }
    else
    {
        // set the default standard dir
        INetURLObject aStdDirObj( SvtPathOptions().GetWorkPath() );
        getDialog()->SetPath( aStdDirObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );
    }

    // set the control values and set the control labels, too
    if ( m_pElemList && !m_pElemList->empty() )
    {
        ::svt::OControlAccess aAccess( getDialog(), getDialog()->GetView() );

        for (auto const& elem : *m_pElemList)
        {
            if ( elem.m_bHasValue )
                aAccess.setValue( elem.m_nElementID, elem.m_nControlAction, elem.m_aValue );
            if ( elem.m_bHasLabel )
                aAccess.setLabel( elem.m_nElementID, elem.m_aLabel );
            if ( elem.m_bHasEnabled )
                aAccess.enableControl( elem.m_nElementID, elem.m_bEnabled );
        }

    }

    if ( m_pFilterList )
    {
        for (auto & elem : *m_pFilterList)
        {
            if ( elem.hasSubFilters() )
            {   // it's a filter group
                UnoFilterList aSubFilters;
                elem.getSubFilters( aSubFilters );

                getDialog()->AddFilterGroup( elem.getTitle(), aSubFilters );
             }
            else
                // it's a single filter
                getDialog()->AddFilter( elem.getTitle(), elem.getFilter() );
        }
    }

    // set the default filter
    if ( !m_aCurrentFilter.isEmpty() )
        getDialog()->SetCurFilter( m_aCurrentFilter );

}

void SvtFilePicker::DialogClosedHdl(sal_Int32 nResult)
{
    if ( m_xDlgClosedListener.is() )
    {
        sal_Int16 nRet = static_cast< sal_Int16 >(nResult);
        css::ui::dialogs::DialogClosedEvent aEvent( *this, nRet );
        m_xDlgClosedListener->dialogClosed( aEvent );
        m_xDlgClosedListener.clear();
    }
}

// SvtFilePicker
PickerFlags SvtFilePicker::getPickerFlags()
{
    // set the winbits for creating the filedialog
    PickerFlags nBits = PickerFlags::NONE;

    // set the standard bits according to the service name
    if ( m_nServiceType == TemplateDescription::FILEOPEN_SIMPLE )
    {
        nBits = PickerFlags::Open;
    }
    else if ( m_nServiceType == TemplateDescription::FILESAVE_SIMPLE )
    {
        nBits = PickerFlags::SaveAs;
    }
    else if ( m_nServiceType == TemplateDescription::FILESAVE_AUTOEXTENSION )
    {
        nBits = PickerFlags::SaveAs | PickerFlags::AutoExtension;
    }
    else if ( m_nServiceType == TemplateDescription::FILESAVE_AUTOEXTENSION_PASSWORD )
    {
        nBits = PickerFlags::SaveAs | PickerFlags::Password | PickerFlags::AutoExtension;
    }
    else if ( m_nServiceType == TemplateDescription::FILESAVE_AUTOEXTENSION_PASSWORD_FILTEROPTIONS )
    {
        nBits = PickerFlags::SaveAs | PickerFlags::Password | PickerFlags::AutoExtension | PickerFlags::FilterOptions;
    }
    else if ( m_nServiceType == TemplateDescription::FILESAVE_AUTOEXTENSION_TEMPLATE )
    {
        nBits = PickerFlags::SaveAs | PickerFlags::AutoExtension | PickerFlags::Templates;
    }
    else if ( m_nServiceType == TemplateDescription::FILESAVE_AUTOEXTENSION_SELECTION )
    {
        nBits = PickerFlags::SaveAs | PickerFlags::AutoExtension | PickerFlags::Selection;
    }

    else if ( m_nServiceType == TemplateDescription::FILEOPEN_LINK_PREVIEW_IMAGE_TEMPLATE )
    {
        nBits = PickerFlags::Open | PickerFlags::InsertAsLink | PickerFlags::ShowPreview | PickerFlags::ImageTemplate;
    }
    else if ( m_nServiceType == TemplateDescription::FILEOPEN_PLAY )
    {
        nBits = PickerFlags::Open | PickerFlags::PlayButton;
    }
    else if ( m_nServiceType == TemplateDescription::FILEOPEN_LINK_PLAY )
    {
        nBits = PickerFlags::Open | PickerFlags::InsertAsLink | PickerFlags::PlayButton;
    }
    else if ( m_nServiceType == TemplateDescription::FILEOPEN_READONLY_VERSION )
    {
        nBits = PickerFlags::Open | PickerFlags::ReadOnly | PickerFlags::ShowVersions;
    }
    else if ( m_nServiceType == TemplateDescription::FILEOPEN_LINK_PREVIEW )
    {
        nBits = PickerFlags::Open | PickerFlags::InsertAsLink | PickerFlags::ShowPreview;
    }
    else if ( m_nServiceType == TemplateDescription::FILEOPEN_LINK_PREVIEW_IMAGE_ANCHOR )
    {
        nBits = PickerFlags::Open | PickerFlags::InsertAsLink | PickerFlags::ShowPreview | PickerFlags::ImageAnchor;
    }
    else if ( m_nServiceType == TemplateDescription::FILEOPEN_PREVIEW )
    {
        nBits = PickerFlags::Open | PickerFlags::ShowPreview;
    }
    if ( m_bMultiSelection && ( nBits & PickerFlags::Open ) )
        nBits |= PickerFlags::MultiSelection;

    return nBits;
}


void SvtFilePicker::notify( sal_Int16 _nEventId, sal_Int16 _nControlId )
{
    if ( !m_xListener.is() )
        return;

    FilePickerEvent aEvent( *this, _nControlId );

    switch ( _nEventId )
    {
        case FILE_SELECTION_CHANGED:
            m_xListener->fileSelectionChanged( aEvent );
            break;
        case DIRECTORY_CHANGED:
            m_xListener->directoryChanged( aEvent );
            break;
        case CTRL_STATE_CHANGED:
            m_xListener->controlStateChanged( aEvent );
            break;
        case DIALOG_SIZE_CHANGED:
            m_xListener->dialogSizeChanged();
            break;
        default:
            SAL_WARN( "fpicker.office", "SvtFilePicker::notify(): Unknown event id!" );
            break;
    }
}


namespace {

    struct FilterTitleMatch
    {
    protected:
        const OUString& rTitle;

    public:
        explicit FilterTitleMatch( const OUString& _rTitle ) : rTitle( _rTitle ) { }


        bool operator () ( const FilterEntry& _rEntry )
        {
            bool bMatch;
            if ( !_rEntry.hasSubFilters() )
                // a real filter
                bMatch = ( _rEntry.getTitle() == rTitle );
            else
                // a filter group -> search the sub filters
                bMatch =
                    ::std::any_of(
                        _rEntry.beginSubFilters(),
                        _rEntry.endSubFilters(),
                        *this
                    );

            return bMatch;
        }
        bool operator () ( const UnoFilterEntry& _rEntry )
        {
            return _rEntry.First == rTitle;
        }
    };
}


bool SvtFilePicker::FilterNameExists( const OUString& rTitle )
{
    bool bRet = false;

    if ( m_pFilterList )
        bRet =
            ::std::any_of(
                m_pFilterList->begin(),
                m_pFilterList->end(),
                FilterTitleMatch( rTitle )
            );

    return bRet;
}


bool SvtFilePicker::FilterNameExists( const UnoFilterList& _rGroupedFilters )
{
    bool bRet = false;

    if ( m_pFilterList )
    {
        const UnoFilterEntry* pStart = _rGroupedFilters.getConstArray();
        const UnoFilterEntry* pEnd = pStart + _rGroupedFilters.getLength();
        for ( ; pStart != pEnd; ++pStart )
            if ( ::std::any_of( m_pFilterList->begin(), m_pFilterList->end(), FilterTitleMatch( pStart->First ) ) )
                break;

        bRet = pStart != pEnd;
    }

    return bRet;
}


void SvtFilePicker::ensureFilterList( const OUString& _rInitialCurrentFilter )
{
    if ( !m_pFilterList )
    {
        m_pFilterList.reset( new FilterList );

        // set the first filter to the current filter
        if ( m_aCurrentFilter.isEmpty() )
            m_aCurrentFilter = _rInitialCurrentFilter;
    }
}


// class SvtFilePicker

SvtFilePicker::SvtFilePicker()
    :m_bMultiSelection  ( false )
    ,m_nServiceType     ( TemplateDescription::FILEOPEN_SIMPLE )
{
}

SvtFilePicker::~SvtFilePicker()
{
}


sal_Int16 SvtFilePicker::implExecutePicker( )
{
    getDialog()->SetFileCallback( this );

    prepareExecute();

    getDialog()->EnableAutocompletion();
    // now we are ready to execute the dialog
    sal_Int16 nRet = getDialog()->Execute();

    // the execution of the dialog yields, so it is possible the at this point the window or the dialog is closed
    if ( getDialog() )
        getDialog()->SetFileCallback( nullptr );

    return nRet;
}


VclPtr<SvtFileDialog_Base> SvtFilePicker::implCreateDialog( vcl::Window* _pParent )
{
    PickerFlags nBits = getPickerFlags();

    VclPtrInstance<SvtFileDialog> dialog( _pParent, nBits );

    // Set StandardDir if present
    if ( !m_aStandardDir.isEmpty())
    {
        OUString sStandardDir = m_aStandardDir;
        dialog->SetStandardDir( sStandardDir );
        dialog->SetBlackList( m_aBlackList );
    }

    return dialog;
}


// disambiguate XInterface

IMPLEMENT_FORWARD_XINTERFACE2( SvtFilePicker, OCommonPicker, SvtFilePicker_Base )


// disambiguate XTypeProvider

IMPLEMENT_FORWARD_XTYPEPROVIDER2( SvtFilePicker, OCommonPicker, SvtFilePicker_Base )

IMPLEMENT_FORWARD_XINTERFACE3( SvtRemoteFilePicker, SvtFilePicker, OCommonPicker, SvtFilePicker_Base )


// disambiguate XTypeProvider

IMPLEMENT_FORWARD_XTYPEPROVIDER3( SvtRemoteFilePicker, SvtFilePicker, OCommonPicker, SvtFilePicker_Base )


// XExecutableDialog functions


void SAL_CALL SvtFilePicker::setTitle( const OUString& _rTitle )
{
    OCommonPicker::setTitle( _rTitle );
}


sal_Int16 SAL_CALL SvtFilePicker::execute(  )
{
    return OCommonPicker::execute();
}


// XAsynchronousExecutableDialog functions


void SAL_CALL SvtFilePicker::setDialogTitle( const OUString& _rTitle )
{
    setTitle( _rTitle );
}


void SAL_CALL SvtFilePicker::startExecuteModal( const Reference< css::ui::dialogs::XDialogClosedListener >& xListener )
{
    m_xDlgClosedListener = xListener;
    prepareDialog();
    prepareExecute();
    SvtFileDialog_Base* pDialog = getDialog();
    pDialog->EnableAutocompletion();
    pDialog->StartExecuteAsync([this](sal_Int32 nResult){
        DialogClosedHdl(nResult);
    });
}


// XFilePicker functions


void SAL_CALL SvtFilePicker::setMultiSelectionMode( sal_Bool bMode )
{
    checkAlive();

    SolarMutexGuard aGuard;
    m_bMultiSelection = bMode;
}

void SAL_CALL SvtFilePicker::setDefaultName( const OUString& aName )
{
    checkAlive();

    SolarMutexGuard aGuard;
    m_aDefaultName = aName;
}

void SAL_CALL SvtFilePicker::setDisplayDirectory( const OUString& aDirectory )
{
    checkAlive();

    SolarMutexGuard aGuard;
    m_aDisplayDirectory = aDirectory;
}

OUString SAL_CALL SvtFilePicker::getDisplayDirectory()
{
    checkAlive();

    SolarMutexGuard aGuard;
    if ( getDialog() )
    {
        OUString aPath = getDialog()->GetPath();

        if( m_aOldHideDirectory == aPath )
            return m_aOldDisplayDirectory;
        m_aOldHideDirectory = aPath;

        if( !getDialog()->ContentIsFolder( aPath ) )
        {
            INetURLObject aFolder( aPath );
            aFolder.CutLastName();
            aPath = aFolder.GetMainURL( INetURLObject::DecodeMechanism::NONE );
        }
        m_aOldDisplayDirectory = aPath;
        return aPath;
    }
    else
        return m_aDisplayDirectory;
}

Sequence< OUString > SAL_CALL SvtFilePicker::getSelectedFiles()
{
    checkAlive();

    SolarMutexGuard aGuard;
    if ( ! getDialog() )
    {
        Sequence< OUString > aEmpty;
        return aEmpty;
    }

    return comphelper::containerToSequence(getDialog()->GetPathList());
}

Sequence< OUString > SAL_CALL SvtFilePicker::getFiles()
{
    Sequence< OUString > aFiles = getSelectedFiles();
    if (aFiles.getLength() > 1)
        aFiles.realloc(1);
    return aFiles;
}


// XFilePickerControlAccess functions


void SAL_CALL SvtFilePicker::setValue( sal_Int16 nElementID,
                                       sal_Int16 nControlAction,
                                       const Any& rValue )
{
    checkAlive();

    SolarMutexGuard aGuard;
    if ( getDialog() )
    {
        ::svt::OControlAccess aAccess( getDialog(), getDialog()->GetView() );
        aAccess.setValue( nElementID, nControlAction, rValue );
    }
    else
    {
        if ( !m_pElemList )
            m_pElemList.reset( new ElementList );

        bool bFound = false;

        for (auto & elem : *m_pElemList)
        {
            if ( ( elem.m_nElementID == nElementID ) &&
                 ( !elem.m_bHasValue || ( elem.m_nControlAction == nControlAction ) ) )
            {
                elem.setAction( nControlAction );
                elem.setValue( rValue );
                bFound = true;
            }
        }

        if ( !bFound )
        {
            ElementEntry_Impl aNew( nElementID );
            aNew.setAction( nControlAction );
            aNew.setValue( rValue );
            m_pElemList->insert( m_pElemList->end(), aNew );
        }
    }
}


Any SAL_CALL SvtFilePicker::getValue( sal_Int16 nElementID, sal_Int16 nControlAction )
{
    checkAlive();

    SolarMutexGuard aGuard;
    Any      aAny;

    // execute() called?
    if ( getDialog() )
    {
        ::svt::OControlAccess aAccess( getDialog(), getDialog()->GetView() );
        aAny = aAccess.getValue( nElementID, nControlAction );
    }
    else if ( m_pElemList )
    {
        for (auto const& elem : *m_pElemList)
        {
            if ( ( elem.m_nElementID == nElementID ) &&
                 ( elem.m_bHasValue ) &&
                 ( elem.m_nControlAction == nControlAction ) )
            {
                aAny = elem.m_aValue;
                break;
            }
        }
    }

    return aAny;
}


void SAL_CALL SvtFilePicker::setLabel( sal_Int16 nLabelID, const OUString& rValue )
{
    checkAlive();

    SolarMutexGuard aGuard;
    if ( getDialog() )
    {
        ::svt::OControlAccess aAccess( getDialog(), getDialog()->GetView() );
        aAccess.setLabel( nLabelID, rValue );
    }
    else
    {
        if ( !m_pElemList )
            m_pElemList.reset( new ElementList );

        bool bFound = false;

        for (auto & elem : *m_pElemList)
        {
            if ( elem.m_nElementID == nLabelID )
            {
                elem.setLabel( rValue );
                bFound = true;
            }
        }

        if ( !bFound )
        {
            ElementEntry_Impl aNew( nLabelID );
            aNew.setLabel( rValue );
            m_pElemList->insert( m_pElemList->end(), aNew );
        }
    }
}


OUString SAL_CALL SvtFilePicker::getLabel( sal_Int16 nLabelID )
{
    checkAlive();

    SolarMutexGuard aGuard;
    OUString aLabel;

    if ( getDialog() )
    {
        ::svt::OControlAccess aAccess( getDialog(), getDialog()->GetView() );
        aLabel = aAccess.getLabel( nLabelID );
    }
    else if ( m_pElemList )
    {
        for (auto const& elem : *m_pElemList)
        {
            if ( elem.m_nElementID == nLabelID )
            {
                if ( elem.m_bHasLabel )
                    aLabel = elem.m_aLabel;
                break;
            }
        }
    }

    return aLabel;
}


void SAL_CALL SvtFilePicker::enableControl( sal_Int16 nElementID, sal_Bool bEnable )
{
    checkAlive();

    SolarMutexGuard aGuard;
    if ( getDialog() )
    {
        ::svt::OControlAccess aAccess( getDialog(), getDialog()->GetView() );
        aAccess.enableControl( nElementID, bEnable );
    }
    else
    {
        if ( !m_pElemList )
            m_pElemList.reset( new ElementList );

        bool bFound = false;

        for (auto & elem : *m_pElemList)
        {
            if ( elem.m_nElementID == nElementID )
            {
                elem.setEnabled( bEnable );
                bFound = true;
            }
        }

        if ( !bFound )
        {
            ElementEntry_Impl aNew( nElementID );
            aNew.setEnabled( bEnable );
            m_pElemList->insert( m_pElemList->end(), aNew );
        }
    }
}


// XFilePickerNotifier functions


void SAL_CALL SvtFilePicker::addFilePickerListener( const Reference< XFilePickerListener >& xListener )
{
    checkAlive();

    SolarMutexGuard aGuard;
    m_xListener = xListener;
}


void SAL_CALL SvtFilePicker::removeFilePickerListener( const Reference< XFilePickerListener >& )
{
    checkAlive();

    SolarMutexGuard aGuard;
    m_xListener.clear();
}


// XFilePreview functions


Sequence< sal_Int16 > SAL_CALL SvtFilePicker::getSupportedImageFormats()
{
    checkAlive();

    SolarMutexGuard aGuard;
    Sequence< sal_Int16 > aFormats( 1 );

    aFormats[0] = FilePreviewImageFormats::BITMAP;

    return aFormats;
}


sal_Int32 SAL_CALL SvtFilePicker::getTargetColorDepth()
{
    checkAlive();

    SolarMutexGuard aGuard;
    sal_Int32 nDepth = 0;

    if ( getDialog() )
        nDepth = getDialog()->getTargetColorDepth();

    return nDepth;
}


sal_Int32 SAL_CALL SvtFilePicker::getAvailableWidth()
{
    checkAlive();

    SolarMutexGuard aGuard;
    sal_Int32 nWidth = 0;

    if ( getDialog() )
        nWidth = getDialog()->getAvailableWidth();

    return nWidth;
}


sal_Int32 SAL_CALL SvtFilePicker::getAvailableHeight()
{
    checkAlive();

    SolarMutexGuard aGuard;
    sal_Int32 nHeight = 0;

    if ( getDialog() )
        nHeight = getDialog()->getAvailableHeight();

    return nHeight;
}


void SAL_CALL SvtFilePicker::setImage( sal_Int16 aImageFormat, const Any& rImage )
{
    checkAlive();

    SolarMutexGuard aGuard;
    if ( getDialog() )
        getDialog()->setImage( aImageFormat, rImage );
}


sal_Bool SAL_CALL SvtFilePicker::setShowState( sal_Bool )
{
    checkAlive();

    SolarMutexGuard aGuard;
    bool bRet = false;

    if ( getDialog() )
    {
    // #97633 for the system filedialog it's
    // useful to make the preview switchable
    // because the preview occupies
    // half of the size of the file listbox
    // which is not the case here,
    // so we (TRA/FS) decided not to make
    // the preview window switchable because
    // else we would have to change the layout
    // of the file dialog dynamically
    // support for set/getShowState is optionally
    // see css::ui::dialogs::XFilePreview

        bRet = false;
    }

    return bRet;
}


sal_Bool SAL_CALL SvtFilePicker::getShowState()
{
    checkAlive();

    SolarMutexGuard aGuard;
    bool bRet = false;

    if ( getDialog() )
        bRet = getDialog()->getShowState();

    return bRet;
}


// XFilterGroupManager functions


void SAL_CALL SvtFilePicker::appendFilterGroup( const OUString& sGroupTitle,
                                                const Sequence< StringPair >& aFilters )
{
    checkAlive();

    SolarMutexGuard aGuard;

    // check the names
    if ( FilterNameExists( aFilters ) )
        throw IllegalArgumentException(
            "filter name exists",
            static_cast< OWeakObject * >(this), 1);

    // ensure that we have a filter list
    OUString sInitialCurrentFilter;
    if ( aFilters.getLength() )
        sInitialCurrentFilter = aFilters[0].First;
    ensureFilterList( sInitialCurrentFilter );

    // append the filter
    m_pFilterList->insert( m_pFilterList->end(), FilterEntry( sGroupTitle, aFilters ) );
}


// XFilterManager functions


void SAL_CALL SvtFilePicker::appendFilter( const OUString& aTitle,
                                           const OUString& aFilter )
{
    checkAlive();

    SolarMutexGuard aGuard;
    // check the name
    if ( FilterNameExists( aTitle ) )
        // TODO: a more precise exception message
        throw IllegalArgumentException();

    // ensure that we have a filter list
    ensureFilterList( aTitle );

    // append the filter
    m_pFilterList->insert( m_pFilterList->end(), FilterEntry( aTitle, aFilter ) );
}


void SAL_CALL SvtFilePicker::setCurrentFilter( const OUString& aTitle )
{
    checkAlive();

    SolarMutexGuard aGuard;
    if ( ! FilterNameExists( aTitle ) )
        throw IllegalArgumentException();

    m_aCurrentFilter = aTitle;

    if ( getDialog() )
        getDialog()->SetCurFilter( aTitle );
}


OUString SAL_CALL SvtFilePicker::getCurrentFilter()
{
    checkAlive();

    SolarMutexGuard aGuard;
    OUString aFilter = getDialog() ? getDialog()->GetCurFilter() :
                                     m_aCurrentFilter;
    return aFilter;
}


// XInitialization functions


void SAL_CALL SvtFilePicker::initialize( const Sequence< Any >& _rArguments )
{
    checkAlive();

    Sequence< Any > aArguments( _rArguments.getLength());

    m_nServiceType = TemplateDescription::FILEOPEN_SIMPLE;

    if ( _rArguments.getLength() >= 1 )
    {
        // compatibility: one argument, type sal_Int16 , specifies the service type
        int index = 0;

        if (_rArguments[0] >>= m_nServiceType)
        {
            // skip the first entry if it was the ServiceType, because it's not needed in OCommonPicker::initialize and it's not a NamedValue
            NamedValue emptyNamedValue;
            aArguments[0] <<= emptyNamedValue;
            index = 1;

        }
        for ( int i = index; i < _rArguments.getLength(); i++)
        {
            NamedValue namedValue;
            aArguments[i] = _rArguments[i];

            if (aArguments[i] >>= namedValue )
            {

                if ( namedValue.Name == "StandardDir" )
                {
                    OUString sStandardDir;

                    namedValue.Value >>= sStandardDir;

                    // Set the directory for the "back to the default dir" button
                    if ( !sStandardDir.isEmpty() )
                    {
                        m_aStandardDir = sStandardDir;
                    }
                }
                else if ( namedValue.Name == "BlackList" )
                {
                    namedValue.Value >>= m_aBlackList;
                }
            }
        }
    }

    // let the base class analyze the sequence (will call into implHandleInitializationArgument)
    OCommonPicker::initialize( aArguments );
}


bool SvtFilePicker::implHandleInitializationArgument( const OUString& _rName, const Any& _rValue )
{
    if ( _rName == "TemplateDescription" )
    {
        m_nServiceType = TemplateDescription::FILEOPEN_SIMPLE;
        OSL_VERIFY( _rValue >>= m_nServiceType );
        return true;
    }
    if ( _rName == "StandardDir" )
    {
        OSL_VERIFY( _rValue >>= m_aStandardDir );
        return true;
    }

    if ( _rName == "BlackList" )
    {
        OSL_VERIFY( _rValue >>= m_aBlackList );
        return true;
    }


    return OCommonPicker::implHandleInitializationArgument( _rName, _rValue );
}


// XServiceInfo


/* XServiceInfo */
OUString SAL_CALL SvtFilePicker::getImplementationName()
{
    return impl_getStaticImplementationName();
}

/* XServiceInfo */
sal_Bool SAL_CALL SvtFilePicker::supportsService( const OUString& sServiceName )
{
    return cppu::supportsService(this, sServiceName);
}

/* XServiceInfo */
Sequence< OUString > SAL_CALL SvtFilePicker::getSupportedServiceNames()
{
    return impl_getStaticSupportedServiceNames();
}

/* Helper for XServiceInfo */
Sequence< OUString > SvtFilePicker::impl_getStaticSupportedServiceNames()
{
    Sequence<OUString> seqServiceNames { "com.sun.star.ui.dialogs.OfficeFilePicker" };
    return seqServiceNames ;
}

/* Helper for XServiceInfo */
OUString SvtFilePicker::impl_getStaticImplementationName()
{
    return OUString( "com.sun.star.svtools.OfficeFilePicker" );
}

/* Helper for registry */
Reference< XInterface > SvtFilePicker::impl_createInstance(
    const Reference< XComponentContext >& )
{
    return Reference< XInterface >( *new SvtFilePicker );
}

// SvtRemoteFilePicker

SvtRemoteFilePicker::SvtRemoteFilePicker()
{
}

VclPtr<SvtFileDialog_Base> SvtRemoteFilePicker::implCreateDialog( vcl::Window* _pParent )
{
    PickerFlags nBits = getPickerFlags();

    VclPtrInstance<RemoteFilesDialog> dialog( _pParent, nBits);

    // Set StandardDir if present
    if ( !m_aStandardDir.isEmpty())
    {
        OUString sStandardDir = m_aStandardDir;
        dialog->SetStandardDir( sStandardDir );
        dialog->SetBlackList( m_aBlackList );
    }

    return dialog;
}

// XServiceInfo


/* XServiceInfo */
OUString SAL_CALL SvtRemoteFilePicker::getImplementationName()
{
    return impl_getStaticImplementationName();
}

/* XServiceInfo */
sal_Bool SAL_CALL SvtRemoteFilePicker::supportsService( const OUString& sServiceName )
{
    return cppu::supportsService(this, sServiceName);
}

/* XServiceInfo */
Sequence< OUString > SAL_CALL SvtRemoteFilePicker::getSupportedServiceNames()
{
    return impl_getStaticSupportedServiceNames();
}

/* Helper for XServiceInfo */
Sequence< OUString > SvtRemoteFilePicker::impl_getStaticSupportedServiceNames()
{
    Sequence<OUString> seqServiceNames { "com.sun.star.ui.dialogs.RemoteFilePicker" };
    return seqServiceNames ;
}

/* Helper for XServiceInfo */
OUString SvtRemoteFilePicker::impl_getStaticImplementationName()
{
    return OUString( "com.sun.star.svtools.RemoteFilePicker" );
}

/* Helper for registry */
Reference< XInterface > SvtRemoteFilePicker::impl_createInstance(
    const Reference< XComponentContext >& )
{
    return Reference< XInterface >( *new SvtRemoteFilePicker );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
