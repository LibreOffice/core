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
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <tools/link.hxx>

#include <rtl/logfile.hxx>
#include <rtl/instance.hxx>
#include "itemholder2.hxx"

#include <svtools/imgdef.hxx>
#include <vcl/svapp.hxx>

#include <list>

using namespace ::utl                   ;
using namespace ::rtl                   ;
using namespace ::osl                   ;
using namespace ::com::sun::star::uno   ;
using namespace ::com::sun::star;

#define ASCII_STR(s)                        OUString( RTL_CONSTASCII_USTRINGPARAM(s) )
#define ROOTNODE_SLIDESORTERBAR             ASCII_STR("Office.Impress/MultiPaneGUI/SlideSorterBar/Visible")

#define PROPERTYNAME_VISIBLE_IMPRESSVIEW       ASCII_STR("ImpressView")
#define PROPERTYHANDLE_VISIBLE_IMPRESSVIEW     0
#define PROPERTYNAME_VISIBLE_OUTLINEVIEW       ASCII_STR("OutlineView")
#define PROPERTYHANDLE_VISIBLE_OUTLINEVIEW     1
#define PROPERTYNAME_VISIBLE_NOTESVIEW         ASCII_STR("NotesView")
#define PROPERTYHANDLE_VISIBLE_NOTESVIEW       2
#define PROPERTYNAME_VISIBLE_HANDOUTVIEW       ASCII_STR("HandoutView")
#define PROPERTYHANDLE_VISIBLE_HANDOUTVIEW     3
#define PROPERTYNAME_VISIBLE_SLIDESORTERVIEW   ASCII_STR("SlideSorterView")
#define PROPERTYHANDLE_VISIBLE_SLIDESORTERVIEW 4
#define PROPERTYNAME_VISIBLE_DRAWVIEW          ASCII_STR("DrawView")
#define PROPERTYHANDLE_VISIBLE_DRAWVIEW        5

#define PROPERTYCOUNT                          6

class SvtSlideSorterBarOptions_Impl : public ConfigItem
{
    private:
    ::std::list<Link> aList;
    sal_Bool    m_bVisibleImpressView;
    sal_Bool    m_bVisibleOutlineView;
    sal_Bool    m_bVisibleNotesView;
    sal_Bool    m_bVisibleHandoutView;
    sal_Bool    m_bVisibleSlideSorterView;
    sal_Bool    m_bVisibleDrawView;

    public:

         SvtSlideSorterBarOptions_Impl();
        ~SvtSlideSorterBarOptions_Impl();

          /*@short      called for notify of configmanager
            @descr      These method is called from the ConfigManager before application ends or from the
                         PropertyChangeListener if the sub tree broadcasts changes. You must update your
                        internal values.
            @seealso    baseclass ConfigItem
            @param      "seqPropertyNames" is the list of properties which should be updated.*/

        virtual void Notify( const Sequence< OUString >& seqPropertyNames );

        /** loads required data from the configuration. It's called in the constructor to
         read all entries and form ::Notify to re-read changed setting */
        void Load( const Sequence< OUString >& rPropertyNames );

          /*@short      write changes to configuration
            @descr      These method writes the changed values into the sub tree
                        and should always called in our destructor to guarantee consistency of config data.
            @seealso    baseclass ConfigItem*/
        virtual void Commit();

        //  public interface
        void SetVisibleImpressView( sal_Bool bSet )
        {  m_bVisibleImpressView = bSet; SetModified(); }

        sal_Bool GetVisibleImpressView() const
        { return m_bVisibleImpressView; }

        void SetVisibleOutlineView( sal_Bool bSet )
        {  m_bVisibleOutlineView = bSet; SetModified(); }

        sal_Bool GetVisibleOutlineView() const
        { return m_bVisibleOutlineView; }

        void SetVisibleNotesView( sal_Bool bSet )
        {  m_bVisibleNotesView = bSet; SetModified(); }

        sal_Bool GetVisibleNotesView() const
        { return m_bVisibleNotesView; }

        void SetVisibleHandoutView( sal_Bool bSet )
        {  m_bVisibleHandoutView = bSet; SetModified(); }

        sal_Bool GetVisibleHandoutView() const
        { return m_bVisibleHandoutView; }

        void SetVisibleSlideSorterView( sal_Bool bSet )
        {  m_bVisibleSlideSorterView = bSet; SetModified(); }

        sal_Bool GetVisibleSlideSorterView() const
        { return m_bVisibleSlideSorterView; }

        void SetVisibleDrawView( sal_Bool bSet )
        {  m_bVisibleDrawView = bSet; SetModified(); }

        sal_Bool GetVisibleDrawView() const
        { return m_bVisibleDrawView; }

        void AddListenerLink( const Link& rLink );
        void RemoveListenerLink( const Link& rLink );
        void CallListeners();

    private:
          /*@short      return list of key names of our configuration management which represent oue module tree
            @descr      These methods return a static const list of key names. We need it to get needed values from our
                        configuration management.
            @return     A list of needed configuration keys is returned.*/
        static Sequence< OUString > GetPropertyNames();

    protected:
};

SvtSlideSorterBarOptions_Impl::SvtSlideSorterBarOptions_Impl()
    // Init baseclasses first
    : ConfigItem( ROOTNODE_SLIDESORTERBAR )

    , m_bVisibleImpressView( sal_False )
    , m_bVisibleOutlineView( sal_False )
    , m_bVisibleNotesView( sal_False )
    , m_bVisibleHandoutView( sal_False )
    , m_bVisibleSlideSorterView( sal_False )
    , m_bVisibleDrawView( sal_False )

{
    // Use our static list of configuration keys to get his values.
    Sequence< OUString >    seqNames    = GetPropertyNames  (           );
    Load( seqNames );
    Sequence< Any >         seqValues   = GetProperties     ( seqNames  );
    Sequence< sal_Bool >    seqRO       = GetReadOnlyStates ( seqNames  );

    // Safe impossible cases.
    // We need values from ALL configuration keys.
    // Follow assignment use order of values in relation to our list of key names!
    DBG_ASSERT( !(seqNames.getLength()!=seqValues.getLength()), "SvtSlideSorterBarOptions_Impl::SvtSlideSorterBarOptions_Impl()\nI miss some values of configuration keys!\n" );

    // Copy values from list in right order to our internal member.
    sal_Int32 nPropertyCount = seqValues.getLength();
    for( sal_Int32 nProperty=0; nProperty<nPropertyCount; ++nProperty )
    {
        if (seqValues[nProperty].hasValue()==sal_False)
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
    EnableNotification( seqNames );
}

SvtSlideSorterBarOptions_Impl::~SvtSlideSorterBarOptions_Impl()
{
    // We must save our current values .. if user forgets it!
    if( IsModified() == sal_True )
    {
        Commit();
    }
}

static int lcl_MapPropertyName( const ::rtl::OUString rCompare,
                const uno::Sequence< ::rtl::OUString>& aInternalPropertyNames)
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
    const uno::Sequence< ::rtl::OUString> aInternalPropertyNames( GetPropertyNames());
    Sequence< Any > seqValues = GetProperties( rPropertyNames  );

    // Safe impossible cases.
    // We need values from ALL configuration keys.
    // Follow assignment use order of values in relation to our list of key names!
    DBG_ASSERT( !(rPropertyNames.getLength()!=seqValues.getLength()), "SvtSlideSorterBarOptions_Impl::SvtSlideSorterBarOptions_Impl()\nI miss some values of configuration keys!\n" );

    // Copy values from list in right order to our internal member.
    sal_Int32 nPropertyCount = seqValues.getLength();
    for( sal_Int32 nProperty=0; nProperty<nPropertyCount; ++nProperty )
    {
        if (seqValues[nProperty].hasValue()==sal_False)
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

void SvtSlideSorterBarOptions_Impl::AddListenerLink( const Link& rLink )
{
    aList.push_back( rLink );
}

void SvtSlideSorterBarOptions_Impl::RemoveListenerLink( const Link& rLink )
{
    for ( ::std::list<Link>::iterator iter = aList.begin(); iter != aList.end(); ++iter )
    {
        if ( *iter == rLink )
        {
            aList.erase(iter);
            break;
        }
    }
}

void SvtSlideSorterBarOptions_Impl::CallListeners()
{
    for ( ::std::list<Link>::const_iterator iter = aList.begin(); iter != aList.end(); ++iter )
        iter->Call( this );
}

void SvtSlideSorterBarOptions_Impl::Notify( const Sequence< OUString >& rPropertyNames )
{
    Load( rPropertyNames );
    CallListeners();
}

void SvtSlideSorterBarOptions_Impl::Commit()
{
    // Get names of supported properties, create a list for values and copy current values to it.
    Sequence< OUString >    seqNames    = GetPropertyNames  ();
    sal_Int32               nCount      = seqNames.getLength();
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
    PutProperties( seqNames, seqValues );
}

Sequence< OUString > SvtSlideSorterBarOptions_Impl::GetPropertyNames()
{
    // Build list of configuration key names.
    const OUString pProperties[] =
    {
        PROPERTYNAME_VISIBLE_IMPRESSVIEW,
        PROPERTYNAME_VISIBLE_OUTLINEVIEW,
        PROPERTYNAME_VISIBLE_NOTESVIEW,
        PROPERTYNAME_VISIBLE_HANDOUTVIEW,
        PROPERTYNAME_VISIBLE_SLIDESORTERVIEW,
        PROPERTYNAME_VISIBLE_DRAWVIEW,
    };

    // Initialize return sequence with these list and run
    const Sequence< OUString > seqPropertyNames( pProperties, SAL_N_ELEMENTS( pProperties ) );
    return seqPropertyNames;
}

//  initialize static member, see definition for further information
//  DON'T DO IT IN YOUR HEADER!
SvtSlideSorterBarOptions_Impl* SvtSlideSorterBarOptions::m_pDataContainer    = NULL  ;
sal_Int32                      SvtSlideSorterBarOptions::m_nRefCount = 0     ;

SvtSlideSorterBarOptions::SvtSlideSorterBarOptions()
{
    // Global access, must be guarded (multithreading!).
    MutexGuard aGuard( GetInitMutex() );
    ++m_nRefCount;
    // ... and initialize our data container only if it not already exist!
    if( m_pDataContainer == NULL )
    {
       RTL_LOGFILE_CONTEXT(aLog, "svtools ( ??? ) ::SvtSlideSorterBarOptions_Impl::ctor()");
       m_pDataContainer = new SvtSlideSorterBarOptions_Impl;
    }
}

SvtSlideSorterBarOptions::~SvtSlideSorterBarOptions()
{
    // Global access, must be guarded (multithreading!)
    MutexGuard aGuard( GetInitMutex() );
    --m_nRefCount;
    // If last instance was deleted we must destroy our static data container!
    if( m_nRefCount <= 0 )
    {
        delete m_pDataContainer;
        m_pDataContainer = NULL;
    }
}

sal_Bool SvtSlideSorterBarOptions::GetVisibleImpressView() const
{
    return m_pDataContainer->GetVisibleImpressView();
}

void SvtSlideSorterBarOptions::SetVisibleImpressView(sal_Bool bVisible)
{
    return m_pDataContainer->SetVisibleImpressView(bVisible);
}

sal_Bool SvtSlideSorterBarOptions::GetVisibleOutlineView() const
{
    return m_pDataContainer->GetVisibleOutlineView();
}

void SvtSlideSorterBarOptions::SetVisibleOutlineView(sal_Bool bVisible)
{
    return m_pDataContainer->SetVisibleOutlineView(bVisible);
}

sal_Bool SvtSlideSorterBarOptions::GetVisibleNotesView() const
{
    return m_pDataContainer->GetVisibleNotesView();
}

void SvtSlideSorterBarOptions::SetVisibleNotesView(sal_Bool bVisible)
{
    return m_pDataContainer->SetVisibleNotesView(bVisible);
}

sal_Bool SvtSlideSorterBarOptions::GetVisibleHandoutView() const
{
    return m_pDataContainer->GetVisibleHandoutView();
}

void SvtSlideSorterBarOptions::SetVisibleHandoutView(sal_Bool bVisible)
{
    return m_pDataContainer->SetVisibleHandoutView(bVisible);
}

sal_Bool SvtSlideSorterBarOptions::GetVisibleSlideSorterView() const
{
    return m_pDataContainer->GetVisibleSlideSorterView();
}

void SvtSlideSorterBarOptions::SetVisibleSlideSorterView(sal_Bool bVisible)
{
    return m_pDataContainer->SetVisibleSlideSorterView(bVisible);
}

sal_Bool SvtSlideSorterBarOptions::GetVisibleDrawView() const
{
    return m_pDataContainer->GetVisibleDrawView();
}

void SvtSlideSorterBarOptions::SetVisibleDrawView(sal_Bool bVisible)
{
    return m_pDataContainer->SetVisibleDrawView(bVisible);
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

void SvtSlideSorterBarOptions::AddListenerLink( const Link& rLink )
{
    m_pDataContainer->AddListenerLink( rLink );
}

void SvtSlideSorterBarOptions::RemoveListenerLink( const Link& rLink )
{
    m_pDataContainer->RemoveListenerLink( rLink );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
