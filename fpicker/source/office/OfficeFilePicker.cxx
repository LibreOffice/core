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

#ifndef _LIST_
#include <list>
#endif
#ifndef _FUNCTIONAL_
#include <functional>
#endif
#ifndef _ALGORITHM_
#include <algorithm>
#endif
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
#include <comphelper/sequence.hxx>
#include <cppuhelper/typeprovider.hxx>
#include "osl/mutex.hxx"
#include "vcl/svapp.hxx"

// using ----------------------------------------------------------------

using namespace     ::com::sun::star::container;
using namespace     ::com::sun::star::lang;
using namespace     ::com::sun::star::ui::dialogs;
using namespace     ::com::sun::star::uno;
using namespace     ::com::sun::star::beans;
using namespace     ::com::sun::star::awt;
using namespace     ::utl;

//=====================================================================

//=====================================================================

struct FilterEntry
{
protected:
    ::rtl::OUString     m_sTitle;
    ::rtl::OUString     m_sFilter;

    UnoFilterList       m_aSubFilters;

public:
    FilterEntry( const ::rtl::OUString& _rTitle, const ::rtl::OUString& _rFilter )
        :m_sTitle( _rTitle )
        ,m_sFilter( _rFilter )
    {
    }

    FilterEntry( const ::rtl::OUString& _rTitle, const UnoFilterList& _rSubFilters );

    ::rtl::OUString     getTitle() const { return m_sTitle; }
    ::rtl::OUString     getFilter() const { return m_sFilter; }

    /// determines if the filter has sub filter (i.e., the filter is a filter group in real)
    sal_Bool            hasSubFilters( ) const;

    /** retrieves the filters belonging to the entry
    @return
        the number of sub filters
    */
    sal_Int32           getSubFilters( UnoFilterList& _rSubFilterList );

    // helpers for iterating the sub filters
    const UnoFilterEntry*   beginSubFilters() const { return m_aSubFilters.getConstArray(); }
    const UnoFilterEntry*   endSubFilters() const { return m_aSubFilters.getConstArray() + m_aSubFilters.getLength(); }
};

//=====================================================================

//---------------------------------------------------------------------
FilterEntry::FilterEntry( const ::rtl::OUString& _rTitle, const UnoFilterList& _rSubFilters )
    :m_sTitle( _rTitle )
    ,m_aSubFilters( _rSubFilters )
{
}

//---------------------------------------------------------------------
sal_Bool FilterEntry::hasSubFilters( ) const
{
    return ( 0 < m_aSubFilters.getLength() );
}

//---------------------------------------------------------------------
sal_Int32 FilterEntry::getSubFilters( UnoFilterList& _rSubFilterList )
{
    _rSubFilterList = m_aSubFilters;
    return m_aSubFilters.getLength();
}

// struct ElementEntry_Impl ----------------------------------------------

struct ElementEntry_Impl
{
    sal_Int16       m_nElementID;
    sal_Int16       m_nControlAction;
    Any         m_aValue;
    rtl::OUString       m_aLabel;
    sal_Bool        m_bEnabled      : 1;

    sal_Bool        m_bHasValue     : 1;
    sal_Bool        m_bHasLabel     : 1;
    sal_Bool        m_bHasEnabled   : 1;

                    ElementEntry_Impl( sal_Int16 nId );

    void            setValue( const Any& rVal ) { m_aValue = rVal; m_bHasValue = sal_True; }
    void            setAction( sal_Int16 nAction ) { m_nControlAction = nAction; }
    void            setLabel( const rtl::OUString& rVal ) { m_aLabel = rVal; m_bHasLabel = sal_True; }
    void            setEnabled( sal_Bool bEnabled ) { m_bEnabled = bEnabled; m_bHasEnabled = sal_True; }
};

ElementEntry_Impl::ElementEntry_Impl( sal_Int16 nId )
    : m_nElementID( nId )
    , m_nControlAction( 0 )
    , m_bEnabled( sal_False )
    , m_bHasValue( sal_False )
    , m_bHasLabel( sal_False )
    , m_bHasEnabled( sal_False )
{}

//------------------------------------------------------------------------------------
void SvtFilePicker::prepareExecute()
{
    // set the default directory
    // --**-- doesn't match the spec yet
    if ( !m_aDisplayDirectory.isEmpty() || !m_aDefaultName.isEmpty() )
    {
        if ( !m_aDisplayDirectory.isEmpty() )
        {

            INetURLObject aPath( m_aDisplayDirectory );
            if ( !m_aDefaultName.isEmpty() )
            {
                aPath.insertName( m_aDefaultName );
                getDialog()->SetHasFilename( true );
            }
            String sPath = aPath.GetMainURL( INetURLObject::NO_DECODE );
            getDialog()->SetPath( aPath.GetMainURL( INetURLObject::NO_DECODE ) );
        }
        else if ( !m_aDefaultName.isEmpty() )
        {
            getDialog()->SetPath( m_aDefaultName );
            getDialog()->SetHasFilename( true );
        }
    }
    else
    {
        // Default-Standard-Dir setzen
        INetURLObject aStdDirObj( SvtPathOptions().GetWorkPath() );
        getDialog()->SetPath( aStdDirObj.GetMainURL( INetURLObject::NO_DECODE ) );
    }

    // set the control values and set the control labels, too
    if ( m_pElemList && !m_pElemList->empty() )
    {
        ::svt::OControlAccess aAccess( getDialog(), getDialog()->GetView() );

        ElementList::iterator aListIter;
        for ( aListIter = m_pElemList->begin();
              aListIter != m_pElemList->end(); ++aListIter )
        {
            ElementEntry_Impl& rEntry = *aListIter;
            if ( rEntry.m_bHasValue )
                aAccess.setValue( rEntry.m_nElementID, rEntry.m_nControlAction, rEntry.m_aValue );
            if ( rEntry.m_bHasLabel )
                aAccess.setLabel( rEntry.m_nElementID, rEntry.m_aLabel );
            if ( rEntry.m_bHasEnabled )
                aAccess.enableControl( rEntry.m_nElementID, rEntry.m_bEnabled );
        }

        getDialog()->updateListboxLabelSizes();
    }

    if ( m_pFilterList && !m_pFilterList->empty() )
    {
        for (   FilterList::iterator aListIter = m_pFilterList->begin();
                aListIter != m_pFilterList->end();
                ++aListIter
            )
        {
            if ( aListIter->hasSubFilters() )
            {   // it's a filter group
                UnoFilterList aSubFilters;
                aListIter->getSubFilters( aSubFilters );

                getDialog()->AddFilterGroup( aListIter->getTitle(), aSubFilters );
             }
            else
                // it's a single filter
                getDialog()->AddFilter( aListIter->getTitle(), aListIter->getFilter() );
        }
    }

    // set the default filter
    if ( !m_aCurrentFilter.isEmpty() )
        getDialog()->SetCurFilter( m_aCurrentFilter );

}

//-----------------------------------------------------------------------------
IMPL_LINK( SvtFilePicker, DialogClosedHdl, Dialog*, pDlg )
{
    if ( m_xDlgClosedListener.is() )
    {
        sal_Int16 nRet = static_cast< sal_Int16 >( pDlg->GetResult() );
        ::com::sun::star::ui::dialogs::DialogClosedEvent aEvent( *this, nRet );
        m_xDlgClosedListener->dialogClosed( aEvent );
        m_xDlgClosedListener.clear();
    }
    return 0;
}

//------------------------------------------------------------------------------------
// SvtFilePicker
//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
WinBits SvtFilePicker::getWinBits( WinBits& rExtraBits )
{
    // set the winbits for creating the filedialog
    WinBits nBits = 0L;
    rExtraBits = 0L;

    // set the standard bits acording to the service name
    if ( m_nServiceType == TemplateDescription::FILEOPEN_SIMPLE )
    {
        nBits = WB_OPEN;
    }
    else if ( m_nServiceType == TemplateDescription::FILESAVE_SIMPLE )
    {
        nBits = WB_SAVEAS;
    }
    else if ( m_nServiceType == TemplateDescription::FILESAVE_AUTOEXTENSION )
    {
        nBits = WB_SAVEAS;
        rExtraBits = SFX_EXTRA_AUTOEXTENSION;
    }
    else if ( m_nServiceType == TemplateDescription::FILESAVE_AUTOEXTENSION_PASSWORD )
    {
        nBits = WB_SAVEAS | SFXWB_PASSWORD;
        rExtraBits = SFX_EXTRA_AUTOEXTENSION;
    }
    else if ( m_nServiceType == TemplateDescription::FILESAVE_AUTOEXTENSION_PASSWORD_FILTEROPTIONS )
    {
        nBits = WB_SAVEAS | SFXWB_PASSWORD;
        rExtraBits = SFX_EXTRA_AUTOEXTENSION | SFX_EXTRA_FILTEROPTIONS;
    }
    else if ( m_nServiceType == TemplateDescription::FILESAVE_AUTOEXTENSION_TEMPLATE )
    {
        nBits = WB_SAVEAS;
        rExtraBits = SFX_EXTRA_AUTOEXTENSION | SFX_EXTRA_TEMPLATES;
    }
    else if ( m_nServiceType == TemplateDescription::FILESAVE_AUTOEXTENSION_SELECTION )
    {
        nBits = WB_SAVEAS;
        rExtraBits = SFX_EXTRA_AUTOEXTENSION | SFX_EXTRA_SELECTION;
    }

    else if ( m_nServiceType == TemplateDescription::FILEOPEN_LINK_PREVIEW_IMAGE_TEMPLATE )
    {
        nBits = WB_OPEN;
        rExtraBits = SFX_EXTRA_INSERTASLINK | SFX_EXTRA_SHOWPREVIEW | SFX_EXTRA_IMAGE_TEMPLATE;
    }
    else if ( m_nServiceType == TemplateDescription::FILEOPEN_PLAY )
    {
        nBits = WB_OPEN;
        rExtraBits = SFX_EXTRA_PLAYBUTTON;
    }
    else if ( m_nServiceType == TemplateDescription::FILEOPEN_READONLY_VERSION )
    {
        nBits = WB_OPEN | SFXWB_READONLY;
        rExtraBits = SFX_EXTRA_SHOWVERSIONS;
    }
    else if ( m_nServiceType == TemplateDescription::FILEOPEN_LINK_PREVIEW )
    {
        nBits = WB_OPEN;
        rExtraBits = SFX_EXTRA_INSERTASLINK | SFX_EXTRA_SHOWPREVIEW;
    }
    if ( m_bMultiSelection && ( ( nBits & WB_OPEN ) == WB_OPEN ) )
        nBits |= SFXWB_MULTISELECTION;

    return nBits;
}

//------------------------------------------------------------------------------------
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
        case HELP_REQUESTED:
            m_xListener->helpRequested( aEvent );
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

//------------------------------------------------------------------------------------
namespace {
    //................................................................................
    struct FilterTitleMatch : public ::std::unary_function< FilterEntry, bool >
    {
    protected:
        const ::rtl::OUString& rTitle;

    public:
        FilterTitleMatch( const ::rtl::OUString& _rTitle ) : rTitle( _rTitle ) { }

        //............................................................................
        bool operator () ( const FilterEntry& _rEntry )
        {
            sal_Bool bMatch;
            if ( !_rEntry.hasSubFilters() )
                // a real filter
                bMatch = ( _rEntry.getTitle() == rTitle );
            else
                // a filter group -> search the sub filters
                bMatch =
                    _rEntry.endSubFilters() != ::std::find_if(
                        _rEntry.beginSubFilters(),
                        _rEntry.endSubFilters(),
                        *this
                    );

            return bMatch ? true : false;
        }
        bool operator () ( const UnoFilterEntry& _rEntry )
        {
            return _rEntry.First == rTitle ? true : false;
        }
    };
}

//------------------------------------------------------------------------------------
sal_Bool SvtFilePicker::FilterNameExists( const ::rtl::OUString& rTitle )
{
    sal_Bool bRet = sal_False;

    if ( m_pFilterList )
        bRet =
            m_pFilterList->end() != ::std::find_if(
                m_pFilterList->begin(),
                m_pFilterList->end(),
                FilterTitleMatch( rTitle )
            );

    return bRet;
}

//------------------------------------------------------------------------------------
sal_Bool SvtFilePicker::FilterNameExists( const UnoFilterList& _rGroupedFilters )
{
    sal_Bool bRet = sal_False;

    if ( m_pFilterList )
    {
        const UnoFilterEntry* pStart = _rGroupedFilters.getConstArray();
        const UnoFilterEntry* pEnd = pStart + _rGroupedFilters.getLength();
        for ( ; pStart != pEnd; ++pStart )
            if ( m_pFilterList->end() != ::std::find_if( m_pFilterList->begin(), m_pFilterList->end(), FilterTitleMatch( pStart->First ) ) )
                break;

        bRet = pStart != pEnd;
    }

    return bRet;
}

//------------------------------------------------------------------------------------
void SvtFilePicker::ensureFilterList( const ::rtl::OUString& _rInitialCurrentFilter )
{
    if ( !m_pFilterList )
    {
        m_pFilterList = new FilterList;

        // set the first filter to the current filter
        if ( m_aCurrentFilter.isEmpty() )
            m_aCurrentFilter = _rInitialCurrentFilter;
    }
}

//------------------------------------------------------------------------------------
// class SvtFilePicker
//------------------------------------------------------------------------------------
SvtFilePicker::SvtFilePicker( const Reference < XMultiServiceFactory >& xFactory )
    :OCommonPicker( xFactory )
    ,m_pFilterList      ( NULL )
    ,m_pElemList        ( NULL )
    ,m_bMultiSelection  ( sal_False )
    ,m_nServiceType     ( TemplateDescription::FILEOPEN_SIMPLE )
{
}

SvtFilePicker::~SvtFilePicker()
{
    if ( m_pFilterList && !m_pFilterList->empty() )
        m_pFilterList->erase( m_pFilterList->begin(), m_pFilterList->end() );
    delete m_pFilterList;

    if ( m_pElemList && !m_pElemList->empty() )
        m_pElemList->erase( m_pElemList->begin(), m_pElemList->end() );
    delete m_pElemList;
}

//------------------------------------------------------------------------------------
sal_Int16 SvtFilePicker::implExecutePicker( )
{
    getDialog()->SetFileCallback( this );

    prepareExecute();

    getDialog()->EnableAutocompletion( sal_True );
    // now we are ready to execute the dialog
    sal_Int16 nRet = getDialog()->Execute();

    // the execution of the dialog yields, so it is possible the at this point the window or the dialog is closed
    if ( getDialog() )
        getDialog()->SetFileCallback( NULL );

    return nRet;
}

//------------------------------------------------------------------------------------
SvtFileDialog* SvtFilePicker::implCreateDialog( Window* _pParent )
{
    WinBits nExtraBits;
    WinBits nBits = getWinBits( nExtraBits );

    SvtFileDialog* dialog = new SvtFileDialog( _pParent, nBits, nExtraBits );

    // Set StandardDir if present
    if ( !m_aStandardDir.isEmpty())
    {
        String sStandardDir = String( m_aStandardDir );
        dialog->SetStandardDir( sStandardDir );
        dialog->SetBlackList( m_aBlackList );
    }

    return dialog;
}

//------------------------------------------------------------------------------------
// disambiguate XInterface
//------------------------------------------------------------------------------------
IMPLEMENT_FORWARD_XINTERFACE2( SvtFilePicker, OCommonPicker, SvtFilePicker_Base )

//------------------------------------------------------------------------------------
// disambiguate XTypeProvider
//------------------------------------------------------------------------------------
IMPLEMENT_FORWARD_XTYPEPROVIDER2( SvtFilePicker, OCommonPicker, SvtFilePicker_Base )

//------------------------------------------------------------------------------------
// XExecutableDialog functions
//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
void SAL_CALL SvtFilePicker::setTitle( const ::rtl::OUString& _rTitle ) throw (RuntimeException)
{
    OCommonPicker::setTitle( _rTitle );
}

//------------------------------------------------------------------------------------
sal_Int16 SAL_CALL SvtFilePicker::execute(  ) throw (RuntimeException)
{
    return OCommonPicker::execute();
}

//------------------------------------------------------------------------------------
// XAsynchronousExecutableDialog functions
//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
void SAL_CALL SvtFilePicker::setDialogTitle( const ::rtl::OUString& _rTitle ) throw (RuntimeException)
{
    setTitle( _rTitle );
}

//------------------------------------------------------------------------------------
void SAL_CALL SvtFilePicker::startExecuteModal( const Reference< ::com::sun::star::ui::dialogs::XDialogClosedListener >& xListener ) throw (RuntimeException)
{
    m_xDlgClosedListener = xListener;
    prepareDialog();
    prepareExecute();
    getDialog()->EnableAutocompletion( sal_True );
    getDialog()->StartExecuteModal( LINK( this, SvtFilePicker, DialogClosedHdl ) );
}

//------------------------------------------------------------------------------------
// XFilePicker functions
//------------------------------------------------------------------------------------

void SAL_CALL SvtFilePicker::setMultiSelectionMode( sal_Bool bMode ) throw( RuntimeException )
{
    checkAlive();

    SolarMutexGuard aGuard;
    m_bMultiSelection = bMode;
}

void SAL_CALL SvtFilePicker::setDefaultName( const rtl::OUString& aName ) throw( RuntimeException )
{
    checkAlive();

    SolarMutexGuard aGuard;
    m_aDefaultName = aName;
}

void SAL_CALL SvtFilePicker::setDisplayDirectory( const rtl::OUString& aDirectory )
    throw( IllegalArgumentException, RuntimeException )
{
    checkAlive();

    SolarMutexGuard aGuard;
    m_aDisplayDirectory = aDirectory;
}

rtl::OUString SAL_CALL SvtFilePicker::getDisplayDirectory() throw( RuntimeException )
{
    checkAlive();

    SolarMutexGuard aGuard;
    if ( getDialog() )
    {
        rtl::OUString aPath = getDialog()->GetPath();

        // #97148# ----
        if( m_aOldHideDirectory == aPath )
            return m_aOldDisplayDirectory;
        m_aOldHideDirectory = aPath;

        // #102204# -----
        if( !getDialog()->ContentIsFolder( aPath ) )
        {
            INetURLObject aFolder( aPath );
            aFolder.CutLastName();
            aPath = aFolder.GetMainURL( INetURLObject::NO_DECODE );
        }
        m_aOldDisplayDirectory = aPath;
        return aPath;
    }
    else
        return m_aDisplayDirectory;
}

Sequence< rtl::OUString > SAL_CALL SvtFilePicker::getFiles() throw( RuntimeException )
{
    checkAlive();

    SolarMutexGuard aGuard;
    if ( ! getDialog() )
    {
        Sequence< rtl::OUString > aEmpty;
        return aEmpty;
    }

    // if there is more than one path we have to return the path to the
    // files first and then the list of the selected entries

    std::vector<rtl::OUString> aPathList(getDialog()->GetPathList());
    size_t nCount = aPathList.size();
    size_t nTotal = nCount > 1 ? nCount+1: nCount;

    Sequence< rtl::OUString > aPath( nTotal );

    if ( nCount == 1 )
        aPath[0] = rtl::OUString(aPathList[0]);
    else if ( nCount > 1 )
    {
        INetURLObject aObj(aPathList[0]);
        aObj.removeSegment();
        aPath[0] = aObj.GetMainURL( INetURLObject::NO_DECODE );

        for(size_t i = 0; i < aPathList.size(); ++i)
        {
            aObj.SetURL(aPathList[i]);
            aPath[i + 1] = aObj.getName();
        }
    }

    return aPath;
}

//------------------------------------------------------------------------------------
// XFilePickerControlAccess functions
//------------------------------------------------------------------------------------

void SAL_CALL SvtFilePicker::setValue( sal_Int16 nElementID,
                                       sal_Int16 nControlAction,
                                       const Any& rValue )
    throw( RuntimeException )
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
            m_pElemList = new ElementList;

        sal_Bool bFound = sal_False;
        ElementList::iterator aListIter;

        for ( aListIter = m_pElemList->begin();
              aListIter != m_pElemList->end(); ++aListIter )
        {
            ElementEntry_Impl& rEntry = *aListIter;
            if ( ( rEntry.m_nElementID == nElementID ) &&
                 ( !rEntry.m_bHasValue || ( rEntry.m_nControlAction == nControlAction ) ) )
            {
                rEntry.setAction( nControlAction );
                rEntry.setValue( rValue );
                bFound = sal_True;
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

//------------------------------------------------------------------------------------

Any SAL_CALL SvtFilePicker::getValue( sal_Int16 nElementID, sal_Int16 nControlAction )
    throw( RuntimeException )
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
    else if ( m_pElemList && !m_pElemList->empty() )
    {
        ElementList::iterator aListIter;
        for ( aListIter = m_pElemList->begin();
              aListIter != m_pElemList->end(); ++aListIter )
        {
            ElementEntry_Impl& rEntry = *aListIter;
            if ( ( rEntry.m_nElementID == nElementID ) &&
                 ( rEntry.m_bHasValue ) &&
                 ( rEntry.m_nControlAction == nControlAction ) )
            {
                aAny = rEntry.m_aValue;
                break;
            }
        }
    }

    return aAny;
}


//------------------------------------------------------------------------------------
void SAL_CALL SvtFilePicker::setLabel( sal_Int16 nLabelID, const rtl::OUString& rValue )
    throw ( RuntimeException )
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
            m_pElemList = new ElementList;

        sal_Bool bFound = sal_False;
        ElementList::iterator aListIter;

        for ( aListIter = m_pElemList->begin();
              aListIter != m_pElemList->end(); ++aListIter )
        {
            ElementEntry_Impl& rEntry = *aListIter;
            if ( rEntry.m_nElementID == nLabelID )
            {
                rEntry.setLabel( rValue );
                bFound = sal_True;
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

//------------------------------------------------------------------------------------
rtl::OUString SAL_CALL SvtFilePicker::getLabel( sal_Int16 nLabelID )
    throw ( RuntimeException )
{
    checkAlive();

    SolarMutexGuard aGuard;
    rtl::OUString aLabel;

    if ( getDialog() )
    {
        ::svt::OControlAccess aAccess( getDialog(), getDialog()->GetView() );
        aLabel = aAccess.getLabel( nLabelID );
    }
    else if ( m_pElemList && !m_pElemList->empty() )
    {
        ElementList::iterator aListIter;
        for ( aListIter = m_pElemList->begin();
              aListIter != m_pElemList->end(); ++aListIter )
        {
            ElementEntry_Impl& rEntry = *aListIter;
            if ( rEntry.m_nElementID == nLabelID )
            {
                if ( rEntry.m_bHasLabel )
                    aLabel = rEntry.m_aLabel;
                break;
            }
        }
    }

    return aLabel;
}

//------------------------------------------------------------------------------------
void SAL_CALL SvtFilePicker::enableControl( sal_Int16 nElementID, sal_Bool bEnable )
    throw( RuntimeException )
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
            m_pElemList = new ElementList;

        sal_Bool bFound = sal_False;
        ElementList::iterator aListIter;

        for ( aListIter = m_pElemList->begin();
              aListIter != m_pElemList->end(); ++aListIter )
        {
            ElementEntry_Impl& rEntry = *aListIter;
            if ( rEntry.m_nElementID == nElementID )
            {
                rEntry.setEnabled( bEnable );
                bFound = sal_True;
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

//------------------------------------------------------------------------------------
// XFilePickerNotifier functions
//------------------------------------------------------------------------------------

void SAL_CALL SvtFilePicker::addFilePickerListener( const Reference< XFilePickerListener >& xListener ) throw ( RuntimeException )
{
    checkAlive();

    SolarMutexGuard aGuard;
    m_xListener = xListener;
}

//------------------------------------------------------------------------------------
void SAL_CALL SvtFilePicker::removeFilePickerListener( const Reference< XFilePickerListener >& ) throw ( RuntimeException )
{
    checkAlive();

    SolarMutexGuard aGuard;
    m_xListener.clear();
}

//------------------------------------------------------------------------------------
// XFilePreview functions
//------------------------------------------------------------------------------------

Sequence< sal_Int16 > SAL_CALL SvtFilePicker::getSupportedImageFormats()
    throw ( RuntimeException )
{
    checkAlive();

    SolarMutexGuard aGuard;
    Sequence< sal_Int16 > aFormats( 1 );

    aFormats[0] = FilePreviewImageFormats::BITMAP;

    return aFormats;
}

//------------------------------------------------------------------------------------
sal_Int32 SAL_CALL SvtFilePicker::getTargetColorDepth() throw ( RuntimeException )
{
    checkAlive();

    SolarMutexGuard aGuard;
    sal_Int32 nDepth = 0;

    if ( getDialog() )
        nDepth = getDialog()->getTargetColorDepth();

    return nDepth;
}

//------------------------------------------------------------------------------------
sal_Int32 SAL_CALL SvtFilePicker::getAvailableWidth() throw ( RuntimeException )
{
    checkAlive();

    SolarMutexGuard aGuard;
    sal_Int32 nWidth = 0;

    if ( getDialog() )
        nWidth = getDialog()->getAvailableWidth();

    return nWidth;
}

//------------------------------------------------------------------------------------
sal_Int32 SAL_CALL SvtFilePicker::getAvailableHeight() throw ( RuntimeException )
{
    checkAlive();

    SolarMutexGuard aGuard;
    sal_Int32 nHeigth = 0;

    if ( getDialog() )
        nHeigth = getDialog()->getAvailableHeight();

    return nHeigth;
}

//------------------------------------------------------------------------------------
void SAL_CALL SvtFilePicker::setImage( sal_Int16 aImageFormat, const Any& rImage )
    throw ( IllegalArgumentException, RuntimeException )
{
    checkAlive();

    SolarMutexGuard aGuard;
    if ( getDialog() )
        getDialog()->setImage( aImageFormat, rImage );
}

//------------------------------------------------------------------------------------
sal_Bool SAL_CALL SvtFilePicker::setShowState( sal_Bool bShowState )
    throw ( RuntimeException )
{
    checkAlive();

    SolarMutexGuard aGuard;
    sal_Bool bRet = sal_False;

    if ( getDialog() )
        bRet = getDialog()->setShowState( bShowState );

    return bRet;
}

//------------------------------------------------------------------------------------
sal_Bool SAL_CALL SvtFilePicker::getShowState() throw ( RuntimeException )
{
    checkAlive();

    SolarMutexGuard aGuard;
    sal_Bool bRet = sal_False;

    if ( getDialog() )
        bRet = getDialog()->getShowState();

    return bRet;
}

//------------------------------------------------------------------------------------
// XFilterGroupManager functions
//------------------------------------------------------------------------------------

void SAL_CALL SvtFilePicker::appendFilterGroup( const ::rtl::OUString& sGroupTitle,
                                                const Sequence< StringPair >& aFilters )
    throw ( IllegalArgumentException, RuntimeException )
{
    checkAlive();

    SolarMutexGuard aGuard;

    // check the names
    if ( FilterNameExists( aFilters ) )
        throw IllegalArgumentException(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("filter name exists")),
            static_cast< OWeakObject * >(this), 1);

    // ensure that we have a filter list
    ::rtl::OUString sInitialCurrentFilter;
    if ( aFilters.getLength() )
        sInitialCurrentFilter = aFilters[0].First;
    ensureFilterList( sInitialCurrentFilter );

    // append the filter
    m_pFilterList->insert( m_pFilterList->end(), FilterEntry( sGroupTitle, aFilters ) );
}

//------------------------------------------------------------------------------------
// XFilterManager functions
//------------------------------------------------------------------------------------

void SAL_CALL SvtFilePicker::appendFilter( const rtl::OUString& aTitle,
                                           const rtl::OUString& aFilter )
    throw( IllegalArgumentException, RuntimeException )
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

//------------------------------------------------------------------------------------
void SAL_CALL SvtFilePicker::setCurrentFilter( const rtl::OUString& aTitle )
    throw( IllegalArgumentException, RuntimeException )
{
    checkAlive();

    SolarMutexGuard aGuard;
    if ( ! FilterNameExists( aTitle ) )
        throw IllegalArgumentException();

    m_aCurrentFilter = aTitle;

    if ( getDialog() )
        getDialog()->SetCurFilter( aTitle );
}

//------------------------------------------------------------------------------------
rtl::OUString SAL_CALL SvtFilePicker::getCurrentFilter()
    throw( RuntimeException )
{
    checkAlive();

    SolarMutexGuard aGuard;
    rtl::OUString aFilter = getDialog() ? rtl::OUString( getDialog()->GetCurFilter() ) :
                                            rtl::OUString( m_aCurrentFilter );
    return aFilter;
}


//------------------------------------------------------------------------------------
// XInitialization functions
//------------------------------------------------------------------------------------

void SAL_CALL SvtFilePicker::initialize( const Sequence< Any >& _rArguments )
    throw ( Exception, RuntimeException )
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
            aArguments[i] <<= _rArguments[i];

            if (aArguments[i] >>= namedValue )
            {

                if ( namedValue.Name == "StandardDir" )
                {
                    ::rtl::OUString sStandardDir;

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

//-------------------------------------------------------------------------
sal_Bool SvtFilePicker::implHandleInitializationArgument( const ::rtl::OUString& _rName, const Any& _rValue ) SAL_THROW( ( Exception, RuntimeException ) )
{
    if ( _rName == "TemplateDescription" )
    {
        m_nServiceType = TemplateDescription::FILEOPEN_SIMPLE;
        OSL_VERIFY( _rValue >>= m_nServiceType );
        return sal_True;
    }
    if ( _rName == "StandardDir" )
    {
        OSL_VERIFY( _rValue >>= m_aStandardDir );
        return sal_True;
    }

    if ( _rName == "BlackList" )
    {
        OSL_VERIFY( _rValue >>= m_aBlackList );
        return sal_True;
    }



    return OCommonPicker::implHandleInitializationArgument( _rName, _rValue );
}

//------------------------------------------------------------------------------------
// XServiceInfo
//------------------------------------------------------------------------------------

/* XServiceInfo */
rtl::OUString SAL_CALL SvtFilePicker::getImplementationName() throw( RuntimeException )
{
    return impl_getStaticImplementationName();
}

/* XServiceInfo */
sal_Bool SAL_CALL SvtFilePicker::supportsService( const rtl::OUString& sServiceName ) throw( RuntimeException )
{
    Sequence< rtl::OUString > seqServiceNames = getSupportedServiceNames();
    const rtl::OUString* pArray = seqServiceNames.getConstArray();
    for ( sal_Int32 i = 0; i < seqServiceNames.getLength(); i++ )
    {
        if ( sServiceName == pArray[i] )
        {
            return sal_True ;
        }
    }
    return sal_False ;
}

/* XServiceInfo */
Sequence< rtl::OUString > SAL_CALL SvtFilePicker::getSupportedServiceNames() throw( RuntimeException )
{
    return impl_getStaticSupportedServiceNames();
}

/* Helper for XServiceInfo */
Sequence< rtl::OUString > SvtFilePicker::impl_getStaticSupportedServiceNames()
{
    Sequence< rtl::OUString > seqServiceNames( 1 );
    rtl::OUString* pArray = seqServiceNames.getArray();
    pArray[0] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.ui.dialogs.OfficeFilePicker" ));
    return seqServiceNames ;
}

/* Helper for XServiceInfo */
rtl::OUString SvtFilePicker::impl_getStaticImplementationName()
{
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.svtools.OfficeFilePicker" ));
}

/* Helper for registry */
Reference< XInterface > SAL_CALL SvtFilePicker::impl_createInstance(
    const Reference< XComponentContext >& rxContext) throw( Exception )
{
    Reference< XMultiServiceFactory > xServiceManager (rxContext->getServiceManager(), UNO_QUERY_THROW);
    return Reference< XInterface >( *new SvtFilePicker( xServiceManager ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
