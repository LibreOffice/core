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

#include <svtools/toolpanelopt.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/configitem.hxx>
#include <tools/debug.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <tools/link.hxx>

#include <rtl/instance.hxx>
#include "itemholder2.hxx"

#include <svtools/imgdef.hxx>
#include <vcl/svapp.hxx>

#include <list>

using namespace ::utl;
using namespace ::osl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;

#define ROOTNODE_TOOLPANEL                     "Office.Impress/MultiPaneGUI/ToolPanel/Visible"

#define PROPERTYNAME_VISIBLE_IMPRESSVIEW       OUString("ImpressView")
#define PROPERTYHANDLE_VISIBLE_IMPRESSVIEW     0
#define PROPERTYNAME_VISIBLE_OUTLINEVIEW       OUString("OutlineView")
#define PROPERTYHANDLE_VISIBLE_OUTLINEVIEW     1
#define PROPERTYNAME_VISIBLE_NOTESVIEW         OUString("NotesView")
#define PROPERTYHANDLE_VISIBLE_NOTESVIEW       2
#define PROPERTYNAME_VISIBLE_HANDOUTVIEW       OUString("HandoutView")
#define PROPERTYHANDLE_VISIBLE_HANDOUTVIEW     3
#define PROPERTYNAME_VISIBLE_SLIDESORTERVIEW   OUString("SlideSorterView")
#define PROPERTYHANDLE_VISIBLE_SLIDESORTERVIEW 4

class SvtToolPanelOptions_Impl : public ConfigItem
{
    private:
    Sequence< OUString > m_seqPropertyNames;

    public:

         SvtToolPanelOptions_Impl();
        virtual ~SvtToolPanelOptions_Impl() override;

        /** called for notify of configmanager

            This method is called from the ConfigManager before the application ends or from the
            PropertyChangeListener if the sub tree broadcasts changes. You must update your
            internal values.

            \sa baseclass ConfigItem
            \param[in,out] seqPropertyNames is the list of properties which should be updated.
        */
        virtual void Notify( const Sequence< OUString >& seqPropertyNames ) override;

        /**
         loads required data from the configuration. It's called in the constructor to
         read all entries and form ::Notify to re-read changed setting
        */
        void Load( const Sequence< OUString >& rPropertyNames );

        //  public interface
        bool m_bVisibleImpressView;
        bool m_bVisibleOutlineView;
        bool m_bVisibleNotesView;
        bool m_bVisibleHandoutView;
        bool m_bVisibleSlideSorterView;

    private:

        virtual void ImplCommit() override;

        /** return list of key names of our configuration management which represent our module tree

            This methods returns a static const list of key names. We need it to get needed values from our
            configuration management.

            \return A list of needed configuration keys is returned.
        */
        static Sequence< OUString > GetPropertyNames();

    protected:
};

SvtToolPanelOptions_Impl::SvtToolPanelOptions_Impl()
    // Init baseclasses first
    : ConfigItem( ROOTNODE_TOOLPANEL )

    , m_bVisibleImpressView( false )
    , m_bVisibleOutlineView( false )
    , m_bVisibleNotesView( false )
    , m_bVisibleHandoutView( false )
    , m_bVisibleSlideSorterView( false )

{
    m_seqPropertyNames = GetPropertyNames( );

    // Use our static list of configuration keys to get his values.
    Sequence< Any > seqValues = GetProperties( m_seqPropertyNames  );

    // Safe impossible cases.
    // We need values from ALL configuration keys.
    // Follow assignment use order of values in relation to our list of key names!
    DBG_ASSERT( !(m_seqPropertyNames.getLength()!=seqValues.getLength()),
                "SvtToolPanelOptions_Impl::SvtToolPanelOptions_Impl()\nI miss some values of configuration keys!\n" );

    // Copy values from list in right order to our internal member.
    for( sal_Int32 nProperty=0; nProperty<seqValues.getLength(); ++nProperty )
    {
        if (!seqValues[nProperty].hasValue())
            continue;
        switch( nProperty )
        {
            case PROPERTYHANDLE_VISIBLE_IMPRESSVIEW :
            {
                if( !(seqValues[nProperty] >>= m_bVisibleImpressView) )
                    OSL_FAIL("Wrong type of \"ToolPanel\\VisibleImpressView\"!" );
                break;
            }
            case PROPERTYHANDLE_VISIBLE_OUTLINEVIEW :
            {
                if( !(seqValues[nProperty] >>= m_bVisibleOutlineView) )
                    OSL_FAIL("Wrong type of \"ToolPanel\\VisibleOutlineView\"!" );
                break;
            }
            case PROPERTYHANDLE_VISIBLE_NOTESVIEW :
            {
                if( !(seqValues[nProperty] >>= m_bVisibleNotesView) )
                    OSL_FAIL("Wrong type of \"ToolPanel\\VisibleNotesView\"!" );
                break;
            }
            case PROPERTYHANDLE_VISIBLE_HANDOUTVIEW :
            {
                if( !(seqValues[nProperty] >>= m_bVisibleHandoutView) )
                    OSL_FAIL("Wrong type of \"ToolPanel\\VisibleHandoutView\"!" );
                break;
            }
            case PROPERTYHANDLE_VISIBLE_SLIDESORTERVIEW :
            {
                if( !(seqValues[nProperty] >>= m_bVisibleSlideSorterView) )
                    OSL_FAIL("Wrong type of \"ToolPanel\\VisibleSlideSorterView\"!" );
                break;
            }
        }
    }

    // Enable notification mechanism of our baseclass.
    // We need it to get information about changes outside these class on our used configuration keys!
    EnableNotification( m_seqPropertyNames );
}

SvtToolPanelOptions_Impl::~SvtToolPanelOptions_Impl()
{
    assert(!IsModified()); // should have been committed
}

static int lcl_MapPropertyName( const OUString& rCompare,
                const uno::Sequence< OUString>& aInternalPropertyNames)
{
    for(int nProp = 0; nProp < aInternalPropertyNames.getLength(); ++nProp)
    {
        if( aInternalPropertyNames[nProp] == rCompare )
            return nProp;
    }
    return -1;
}

void SvtToolPanelOptions_Impl::Load( const Sequence< OUString >& rPropertyNames )
{
    const uno::Sequence< OUString> aInternalPropertyNames( GetPropertyNames());
    Sequence< Any > seqValues = GetProperties( rPropertyNames  );

    // Safe impossible cases.
    // We need values from ALL configuration keys.
    // Follow assignment use order of values in relation to our list of key names!
    DBG_ASSERT( !(rPropertyNames.getLength()!=seqValues.getLength()),
                "SvtToolPanelOptions_Impl::SvtToolPanelOptions_Impl()\nI miss some values of configuration keys!\n" );

    // Copy values from list in right order to our internal member.
    for( sal_Int32 nProperty=0; nProperty<seqValues.getLength(); ++nProperty )
    {
        if (!seqValues[nProperty].hasValue())
            continue;
        switch( lcl_MapPropertyName(rPropertyNames[nProperty], aInternalPropertyNames) )
        {
            case PROPERTYHANDLE_VISIBLE_IMPRESSVIEW:
            {
                if( !(seqValues[nProperty] >>= m_bVisibleImpressView) )
                    OSL_FAIL("Wrong type of \"ToolPanel\\VisibleImpressView\"!" );
            }
            break;
            case PROPERTYHANDLE_VISIBLE_OUTLINEVIEW :
            {
                if( !(seqValues[nProperty] >>= m_bVisibleOutlineView) )
                    OSL_FAIL("Wrong type of \"ToolPanel\\VisibleOutlineView\"!" );
            }
            break;
            case PROPERTYHANDLE_VISIBLE_NOTESVIEW :
            {
                if( !(seqValues[nProperty] >>= m_bVisibleNotesView) )
                    OSL_FAIL("Wrong type of \"ToolPanel\\VisibleNotesView\"!" );
            }
            break;
            case PROPERTYHANDLE_VISIBLE_HANDOUTVIEW :
            {
                if( !(seqValues[nProperty] >>= m_bVisibleHandoutView) )
                    OSL_FAIL("Wrong type of \"ToolPanel\\VisibleHandoutView\"!" );
            }
            break;
            case PROPERTYHANDLE_VISIBLE_SLIDESORTERVIEW :
            {
                if( !(seqValues[nProperty] >>= m_bVisibleSlideSorterView) )
                    OSL_FAIL("Wrong type of \"ToolPanel\\VisibleSlideSorterView\"!" );
            }
            break;
       }
    }
}

void SvtToolPanelOptions_Impl::Notify( const Sequence< OUString >& rPropertyNames )
{
    Load( rPropertyNames );
}

void SvtToolPanelOptions_Impl::ImplCommit()
{
    // Get names of supported properties, create a list for values and copy current values to it.
    sal_Int32               nCount      = m_seqPropertyNames.getLength();
    Sequence< Any >         seqValues   ( nCount );
    for( sal_Int32 nProperty=0; nProperty<nCount; ++nProperty )
    {
        switch( nProperty )
        {
            case PROPERTYHANDLE_VISIBLE_IMPRESSVIEW:
            {
                seqValues[nProperty] <<= m_bVisibleImpressView;
                break;
            }
            case PROPERTYHANDLE_VISIBLE_OUTLINEVIEW:
            {
                seqValues[nProperty] <<= m_bVisibleOutlineView;
                break;
            }
            case PROPERTYHANDLE_VISIBLE_NOTESVIEW:
            {
                seqValues[nProperty] <<= m_bVisibleNotesView;
                break;
            }
            case PROPERTYHANDLE_VISIBLE_HANDOUTVIEW:
            {
                seqValues[nProperty] <<= m_bVisibleHandoutView;
                break;
            }
            case PROPERTYHANDLE_VISIBLE_SLIDESORTERVIEW:
            {
                seqValues[nProperty] <<= m_bVisibleSlideSorterView;
                break;
            }
        }
    }
    // Set properties in configuration.
    PutProperties( m_seqPropertyNames, seqValues );
}

Sequence< OUString > SvtToolPanelOptions_Impl::GetPropertyNames()
{
    // Build list of configuration key names.
    const OUString pProperties[] =
    {
        PROPERTYNAME_VISIBLE_IMPRESSVIEW,
        PROPERTYNAME_VISIBLE_OUTLINEVIEW,
        PROPERTYNAME_VISIBLE_NOTESVIEW,
        PROPERTYNAME_VISIBLE_HANDOUTVIEW,
        PROPERTYNAME_VISIBLE_SLIDESORTERVIEW,
    };

    // Initialize return sequence with these list and run
    return Sequence< OUString >( pProperties, SAL_N_ELEMENTS( pProperties ) );
}

namespace {

std::weak_ptr<SvtToolPanelOptions_Impl> theOptions;

}

SvtToolPanelOptions::SvtToolPanelOptions()
{
    // Global access, must be guarded (multithreading!).
    MutexGuard aGuard( GetInitMutex() );

    m_pImpl = theOptions.lock();
    if( !m_pImpl )
    {
       m_pImpl = std::make_shared<SvtToolPanelOptions_Impl>();
       theOptions = m_pImpl;
    }
}

SvtToolPanelOptions::~SvtToolPanelOptions()
{
    // Global access, must be guarded (multithreading!)
    MutexGuard aGuard( GetInitMutex() );

    m_pImpl.reset();
}

bool SvtToolPanelOptions::GetVisibleImpressView() const
{
    return m_pImpl->m_bVisibleImpressView;
}

void SvtToolPanelOptions::SetVisibleImpressView(bool bVisible)
{
    m_pImpl->m_bVisibleImpressView = bVisible;
}

bool SvtToolPanelOptions::GetVisibleOutlineView() const
{
    return m_pImpl->m_bVisibleOutlineView;
}

void SvtToolPanelOptions::SetVisibleOutlineView(bool bVisible)
{
    m_pImpl->m_bVisibleOutlineView = bVisible;
}

bool SvtToolPanelOptions::GetVisibleNotesView() const
{
    return m_pImpl->m_bVisibleNotesView;
}

void SvtToolPanelOptions::SetVisibleNotesView(bool bVisible)
{
    m_pImpl->m_bVisibleNotesView = bVisible;
}

bool SvtToolPanelOptions::GetVisibleHandoutView() const
{
    return m_pImpl->m_bVisibleHandoutView;
}

void SvtToolPanelOptions::SetVisibleHandoutView(bool bVisible)
{
    m_pImpl->m_bVisibleHandoutView = bVisible;
}

bool SvtToolPanelOptions::GetVisibleSlideSorterView() const
{
    return m_pImpl->m_bVisibleSlideSorterView;
}

void SvtToolPanelOptions::SetVisibleSlideSorterView(bool bVisible)
{
    m_pImpl->m_bVisibleSlideSorterView = bVisible;
}

namespace
{
    class theSvtToolPanelOptionsMutex :
        public rtl::Static< osl::Mutex, theSvtToolPanelOptionsMutex > {};
}

Mutex & SvtToolPanelOptions::GetInitMutex()
{
    return theSvtToolPanelOptionsMutex::get();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
