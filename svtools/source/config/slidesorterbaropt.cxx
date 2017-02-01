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

#include <svtools/slidesorterbaropt.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/configitem.hxx>
#include <tools/debug.hxx>
#include <osl/diagnose.h>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <comphelper/lok.hxx>
#include <rtl/instance.hxx>

using namespace ::utl;
using namespace ::osl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;

#define ROOTNODE_SLIDESORTERBAR                "Office.Impress/MultiPaneGUI/SlideSorterBar/Visible"

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
#define PROPERTYNAME_VISIBLE_DRAWVIEW          OUString("DrawView")
#define PROPERTYHANDLE_VISIBLE_DRAWVIEW        5

class SvtSlideSorterBarOptions_Impl : public ConfigItem
{
    Sequence< OUString > m_seqPropertyNames;

    public:

         SvtSlideSorterBarOptions_Impl();
        ~SvtSlideSorterBarOptions_Impl() override;

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
        bool m_bVisibleDrawView;

    private:
        virtual void ImplCommit() final override;

        /** return list of key names of our configuration management which represent our module tree

            This method returns a static const list of key names. We need it to get needed values from
            configuration management.

            \return A list of needed configuration keys is returned.
        */
        static Sequence< OUString > GetPropertyNames();

        void SetVisibleViewImpl( bool& bVisibleView, bool bVisible );

    public:
        void SetVisibleImpressView( bool bVisible)
             { SetVisibleViewImpl( m_bVisibleImpressView, bVisible ); }

        void SetVisibleOutlineView( bool bVisible)
             { SetVisibleViewImpl( m_bVisibleOutlineView, bVisible ); }

        void SetVisibleNotesView( bool bVisible)
             { SetVisibleViewImpl( m_bVisibleNotesView, bVisible ); }

        void SetVisibleHandoutView( bool bVisible)
             { SetVisibleViewImpl( m_bVisibleHandoutView, bVisible ); }

        void SetVisibleSlideSorterView( bool bVisible)
             { SetVisibleViewImpl( m_bVisibleSlideSorterView, bVisible ); }

        void SetVisibleDrawView( bool bVisible)
             { SetVisibleViewImpl( m_bVisibleDrawView, bVisible ); }

};

SvtSlideSorterBarOptions_Impl::SvtSlideSorterBarOptions_Impl()
    // Init baseclasses first
    : ConfigItem( ROOTNODE_SLIDESORTERBAR )

    , m_bVisibleImpressView( false )
    , m_bVisibleOutlineView( false )
    , m_bVisibleNotesView( false )
    , m_bVisibleHandoutView( false )
    , m_bVisibleSlideSorterView( false )
    , m_bVisibleDrawView( false )

{
    m_seqPropertyNames = GetPropertyNames( );

    // Use our static list of configuration keys to get his values.
    Sequence< Any > seqValues = GetProperties( m_seqPropertyNames  );

    // Safe impossible cases.
    // We need values from ALL configuration keys.
    // Follow assignment use order of values in relation to our list of key names!
    DBG_ASSERT( !(m_seqPropertyNames.getLength()!=seqValues.getLength()),
                "SvtSlideSorterBarOptions_Impl::SvtSlideSorterBarOptions_Impl()\nI miss some values of configuration keys!\n" );

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
                    OSL_FAIL("Wrong type of \"SlideSorterBar\\VisibleImpressView\"!" );
                break;
            }
            case PROPERTYHANDLE_VISIBLE_OUTLINEVIEW :
            {
                if( !(seqValues[nProperty] >>= m_bVisibleOutlineView) )
                    OSL_FAIL("Wrong type of \"SlideSorterBar\\VisibleOutlineView\"!" );
                break;
            }
            case PROPERTYHANDLE_VISIBLE_NOTESVIEW :
            {
                if( !(seqValues[nProperty] >>= m_bVisibleNotesView) )
                    OSL_FAIL("Wrong type of \"SlideSorterBar\\VisibleNotesView\"!" );
                break;
            }
            case PROPERTYHANDLE_VISIBLE_HANDOUTVIEW :
            {
                if( !(seqValues[nProperty] >>= m_bVisibleHandoutView) )
                    OSL_FAIL("Wrong type of \"SlideSorterBar\\VisibleHandoutView\"!" );
                break;
            }
            case PROPERTYHANDLE_VISIBLE_SLIDESORTERVIEW :
            {
                if( !(seqValues[nProperty] >>= m_bVisibleSlideSorterView) )
                    OSL_FAIL("Wrong type of \"SlideSorterBar\\VisibleSlideSorterView\"!" );
                break;
            }
            case PROPERTYHANDLE_VISIBLE_DRAWVIEW :
            {
                if( !(seqValues[nProperty] >>= m_bVisibleDrawView) )
                    OSL_FAIL("Wrong type of \"SlideSorterBar\\VisibleDrawView\"!" );
                break;
            }
        }
    }

    // Enable notification mechanism of our baseclass.
    // We need it to get information about changes outside these class on our used configuration keys!
    EnableNotification( m_seqPropertyNames );
}

SvtSlideSorterBarOptions_Impl::~SvtSlideSorterBarOptions_Impl()
{
    if (IsModified())
        Commit();
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

void SvtSlideSorterBarOptions_Impl::Load( const Sequence< OUString >& rPropertyNames )
{
    const uno::Sequence< OUString> aInternalPropertyNames( GetPropertyNames());
    Sequence< Any > seqValues = GetProperties( rPropertyNames  );

    // Safe impossible cases.
    // We need values from ALL configuration keys.
    // Follow assignment use order of values in relation to our list of key names!
    DBG_ASSERT( !(rPropertyNames.getLength()!=seqValues.getLength()),
                "SvtSlideSorterBarOptions_Impl::SvtSlideSorterBarOptions_Impl()\nI miss some values of configuration keys!\n" );

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
                    OSL_FAIL("Wrong type of \"SlideSorterBar\\VisibleImpressView\"!" );
            }
            break;
            case PROPERTYHANDLE_VISIBLE_OUTLINEVIEW :
            {
                if( !(seqValues[nProperty] >>= m_bVisibleOutlineView) )
                    OSL_FAIL("Wrong type of \"SlideSorterBar\\VisibleOutlineView\"!" );
            }
            break;
            case PROPERTYHANDLE_VISIBLE_NOTESVIEW :
            {
                if( !(seqValues[nProperty] >>= m_bVisibleNotesView) )
                    OSL_FAIL("Wrong type of \"SlideSorterBar\\VisibleNotesView\"!" );
            }
            break;
            case PROPERTYHANDLE_VISIBLE_HANDOUTVIEW :
            {
                if( !(seqValues[nProperty] >>= m_bVisibleHandoutView) )
                    OSL_FAIL("Wrong type of \"SlideSorterBar\\VisibleHandoutView\"!" );
            }
            break;
            case PROPERTYHANDLE_VISIBLE_SLIDESORTERVIEW :
            {
                if( !(seqValues[nProperty] >>= m_bVisibleSlideSorterView) )
                    OSL_FAIL("Wrong type of \"SlideSorterBar\\VisibleSlideSorterView\"!" );
            }
            break;

            case PROPERTYHANDLE_VISIBLE_DRAWVIEW :
            {
                if( !(seqValues[nProperty] >>= m_bVisibleDrawView) )
                    OSL_FAIL("Wrong type of \"SlideSorterBar\\VisibleDrawView\"!" );
            }
            break;
       }
    }
}

void SvtSlideSorterBarOptions_Impl::Notify( const Sequence< OUString >& rPropertyNames )
{
    Load( rPropertyNames );
}

void SvtSlideSorterBarOptions_Impl::ImplCommit()
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
            case PROPERTYHANDLE_VISIBLE_DRAWVIEW:
            {
                seqValues[nProperty] <<= m_bVisibleDrawView;
                break;
            }

        }
    }
    // Set properties in configuration.
    PutProperties( m_seqPropertyNames, seqValues );
}

Sequence< OUString > SvtSlideSorterBarOptions_Impl::GetPropertyNames()
{
    // Build list of configuration key names.
    OUString pProperties[] =
    {
        PROPERTYNAME_VISIBLE_IMPRESSVIEW,
        PROPERTYNAME_VISIBLE_OUTLINEVIEW,
        PROPERTYNAME_VISIBLE_NOTESVIEW,
        PROPERTYNAME_VISIBLE_HANDOUTVIEW,
        PROPERTYNAME_VISIBLE_SLIDESORTERVIEW,
        PROPERTYNAME_VISIBLE_DRAWVIEW,
    };

    // Initialize return sequence with these list and run
    return Sequence< OUString >( pProperties, SAL_N_ELEMENTS( pProperties ) );
}

void SvtSlideSorterBarOptions_Impl::SetVisibleViewImpl( bool& bVisibleView, bool bVisible )
{
    if( bVisibleView != bVisible )
    {
        bVisibleView = bVisible;
        SetModified();
    }
}

namespace {
    std::weak_ptr<SvtSlideSorterBarOptions_Impl> g_pSlideSorterBarOptions;
}

SvtSlideSorterBarOptions::SvtSlideSorterBarOptions()
{
    // Global access, must be guarded (multithreading!).
    MutexGuard aGuard( GetInitMutex() );

    m_pImpl = g_pSlideSorterBarOptions.lock();
    if( !m_pImpl )
    {
        m_pImpl = std::make_shared<SvtSlideSorterBarOptions_Impl>();
        g_pSlideSorterBarOptions = m_pImpl;
    }
}

SvtSlideSorterBarOptions::~SvtSlideSorterBarOptions()
{
    // Global access, must be guarded (multithreading!)
    MutexGuard aGuard( GetInitMutex() );

    m_pImpl.reset();
}

bool SvtSlideSorterBarOptions::GetVisibleImpressView() const
{
    return m_pImpl->m_bVisibleImpressView && !comphelper::LibreOfficeKit::isActive();
}

void SvtSlideSorterBarOptions::SetVisibleImpressView(bool bVisible)
{
    m_pImpl->SetVisibleImpressView( bVisible );
}

bool SvtSlideSorterBarOptions::GetVisibleOutlineView() const
{
    return m_pImpl->m_bVisibleOutlineView;
}

void SvtSlideSorterBarOptions::SetVisibleOutlineView(bool bVisible)
{
    m_pImpl->SetVisibleOutlineView( bVisible );
}

bool SvtSlideSorterBarOptions::GetVisibleNotesView() const
{
    return m_pImpl->m_bVisibleNotesView;
}

void SvtSlideSorterBarOptions::SetVisibleNotesView(bool bVisible)
{
    m_pImpl->SetVisibleNotesView( bVisible );
}

bool SvtSlideSorterBarOptions::GetVisibleHandoutView() const
{
    return m_pImpl->m_bVisibleHandoutView;
}

void SvtSlideSorterBarOptions::SetVisibleHandoutView(bool bVisible)
{
    m_pImpl->SetVisibleHandoutView( bVisible );
}

bool SvtSlideSorterBarOptions::GetVisibleSlideSorterView() const
{
    return m_pImpl->m_bVisibleSlideSorterView && !comphelper::LibreOfficeKit::isActive();
}

void SvtSlideSorterBarOptions::SetVisibleSlideSorterView(bool bVisible)
{
    m_pImpl->SetVisibleSlideSorterView( bVisible );
}

bool SvtSlideSorterBarOptions::GetVisibleDrawView() const
{
    return m_pImpl->m_bVisibleDrawView;
}

void SvtSlideSorterBarOptions::SetVisibleDrawView(bool bVisible)
{
    m_pImpl->SetVisibleDrawView( bVisible );
}

namespace
{
    class theSvtSlideSorterBarOptionsMutex :
        public rtl::Static< osl::Mutex, theSvtSlideSorterBarOptionsMutex > {};
}

Mutex & SvtSlideSorterBarOptions::GetInitMutex()
{
    return theSvtSlideSorterBarOptionsMutex::get();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
