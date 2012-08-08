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

#include <deque>

#include <unotools/internaloptions.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/configitem.hxx>
#include <tools/debug.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>

#include <rtl/logfile.hxx>
#include "itemholder1.hxx"

//_________________________________________________________________________________________________________________
//  namespaces
//_________________________________________________________________________________________________________________

using namespace ::utl                   ;
using namespace ::rtl                   ;
using namespace ::osl                   ;
using namespace ::std                   ;
using namespace ::com::sun::star::uno   ;
using namespace ::com::sun::star::beans ;

#define ROOTNODE_INTERNAL                   OUString(RTL_CONSTASCII_USTRINGPARAM("Office.Common/Internal"   ))
#define DEFAULT_SLOTCFG                     sal_False
#define DEFAULT_SENDCRASHMAIL               sal_False
#define DEFAULT_USEMAILUI                   sal_True
#define DEFAULT_CURRENTTEMPURL              OUString(RTL_CONSTASCII_USTRINGPARAM(""))

#define FIXPROPERTYNAME_SLOTCFG             OUString(RTL_CONSTASCII_USTRINGPARAM("Slot"                     ))
#define FIXPROPERTYNAME_SENDCRASHMAIL       OUString(RTL_CONSTASCII_USTRINGPARAM("SendCrashMail"            ))
#define FIXPROPERTYNAME_USEMAILUI           OUString(RTL_CONSTASCII_USTRINGPARAM("UseMailUI"                ))
#define FIXPROPERTYNAME_CURRENTTEMPURL      OUString(RTL_CONSTASCII_USTRINGPARAM("CurrentTempURL"           ))

#define FIXPROPERTYHANDLE_SLOTCFG           0
#define FIXPROPERTYHANDLE_SENDCRASHMAIL     1
#define FIXPROPERTYHANDLE_USEMAILUI         2
#define FIXPROPERTYHANDLE_CURRENTTEMPURL    3

class SvtInternalOptions_Impl : public ConfigItem
{
    //-------------------------------------------------------------------------------------------------------------
    //  private member
    //-------------------------------------------------------------------------------------------------------------

    private:

        sal_Bool                m_bRemoveMenuEntryClose;
        sal_Bool                m_bRemoveMenuEntryBackToWebtop;
        sal_Bool                m_bRemoveMenuEntryNewWebtop;
        sal_Bool                m_bRemoveMenuEntryLogout;
        sal_Bool                m_bSlotCFG          ;   /// cache "Slot" of Internal section
        sal_Bool                m_bSendCrashMail    ;   /// cache "SendCrashMail" of Internal section
        sal_Bool                m_bUseMailUI;
        OUString                m_aCurrentTempURL   ;
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------

    public:

        //---------------------------------------------------------------------------------------------------------
        //  constructor / destructor
        //---------------------------------------------------------------------------------------------------------

         SvtInternalOptions_Impl();
        ~SvtInternalOptions_Impl();

        //---------------------------------------------------------------------------------------------------------
        //  overloaded methods of baseclass
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      called for notify of configmanager
            @descr      These method is called from the ConfigManager before application ends or from the
                         PropertyChangeListener if the sub tree broadcasts changes. You must update your
                        internal values.

            @seealso    baseclass ConfigItem

            @param      "seqPropertyNames" is the list of properties which should be updated.
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void Notify( const Sequence< OUString >& )
        {
            DBG_ASSERT( sal_False, "SvtInternalOptions::Notify()\nNot used yet ... but called!?\n" );
        }

        /*-****************************************************************************************************//**
            @short      write changes to configuration
            @descr      These method writes the changed values into the sub tree
                        and should always called in our destructor to guarantee consistency of config data.

            @seealso    baseclass ConfigItem

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void Commit();

        //---------------------------------------------------------------------------------------------------------
        //  public interface
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      access method to get internal values
            @descr      These method give us a chance to regulate acces to ouer internal values.
                        It's not used in the moment - but it's possible for the feature!

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        sal_Bool    IsRemoveMenuEntryClose() const { return m_bRemoveMenuEntryClose; }
        sal_Bool    IsRemoveMenuEntryBackToWebtop() const { return m_bRemoveMenuEntryBackToWebtop; }
        sal_Bool    IsRemoveMenuEntryNewWebtop() const { return m_bRemoveMenuEntryNewWebtop; }
        sal_Bool    IsRemoveMenuEntryLogout() const { return m_bRemoveMenuEntryLogout; }
        sal_Bool    SlotCFGEnabled      () const { return m_bSlotCFG; }
        sal_Bool    CrashMailEnabled    () const { return m_bSendCrashMail; }
        sal_Bool    MailUIEnabled       () const { return m_bUseMailUI; }

        OUString    GetCurrentTempURL() const { return m_aCurrentTempURL; }
        void        SetCurrentTempURL( const OUString& aNewCurrentTempURL );
    //-------------------------------------------------------------------------------------------------------------
    //  private methods
    //-------------------------------------------------------------------------------------------------------------

    private:

        /*-****************************************************************************************************//**
            @short      return list of fix key names of ouer configuration management which represent oue module tree
            @descr      These methods return a static const list of key names. We need it to get needed values from our
                        configuration management. We return well known key names only - because the "UserData" node
                        is handled in a special way!

            @seealso    -

            @param      -
            @return     A list of needed configuration keys is returned.

            @onerror    -
        *//*-*****************************************************************************************************/

        Sequence< OUString > impl_GetPropertyNames();
};

//*****************************************************************************************************************
//  constructor
//*****************************************************************************************************************
SvtInternalOptions_Impl::SvtInternalOptions_Impl()
    // Init baseclasses first
    :   ConfigItem          ( ROOTNODE_INTERNAL, CONFIG_MODE_IMMEDIATE_UPDATE )
    // Init member then.
    , m_bRemoveMenuEntryClose(sal_False)
    , m_bRemoveMenuEntryBackToWebtop(sal_False)
    , m_bRemoveMenuEntryNewWebtop(sal_False)
    , m_bRemoveMenuEntryLogout(sal_False)
    , m_bSlotCFG(DEFAULT_SLOTCFG)
    , m_bSendCrashMail(DEFAULT_SENDCRASHMAIL)
    , m_bUseMailUI(DEFAULT_USEMAILUI)
    , m_aCurrentTempURL(DEFAULT_CURRENTTEMPURL)
{
    // Use our list of configuration keys to get his values.
    // structure of internal section: (first 2 entries are fixed - all other are member of a set!)
    //      "Slot"
    //      "SendCrashMail"
    //      "RecoveryList/r1/URL"
    //      "RecoveryList/r1/Filter"
    //      "RecoveryList/r1/TempName"
    //      "RecoveryList/r2/URL"
    //      "RecoveryList/r2/Filter"
    //      "RecoveryList/r2/TempName"
    //      "RecoveryList/.."
    Sequence< OUString >    seqNames    = impl_GetPropertyNames()   ;
    Sequence< Any >         seqValues   = GetProperties( seqNames ) ;

    // Safe impossible cases.
    // We need values from ALL configuration keys.
    // Follow assignment use order of values in relation to our list of key names!
    DBG_ASSERT( !(seqNames.getLength()!=seqValues.getLength()), "SvtInternalOptions_Impl::SvtInternalOptions_Impl()\nI miss some values of configuration keys!\n" );

    // Read fixed values first!
    DBG_ASSERT(!seqValues[FIXPROPERTYHANDLE_SLOTCFG].hasValue() || (seqValues[FIXPROPERTYHANDLE_SLOTCFG].getValueTypeClass() == TypeClass_BOOLEAN), "SvtInternalOptions_Impl::SvtInternalOptions_Impl()\nWho has changed the value type of \"Office.Common\\Internal\\Slot\"?");
    DBG_ASSERT(!seqValues[FIXPROPERTYHANDLE_SENDCRASHMAIL].hasValue() || !(seqValues[FIXPROPERTYHANDLE_SENDCRASHMAIL].getValueTypeClass()!=TypeClass_BOOLEAN), "SvtInternalOptions_Impl::SvtInternalOptions_Impl()\nWho has changed the value type of \"Office.Common\\Internal\\SendCrashMail\"?");
    seqValues[FIXPROPERTYHANDLE_SLOTCFG] >>= m_bSlotCFG;
    seqValues[FIXPROPERTYHANDLE_SENDCRASHMAIL] >>= m_bSendCrashMail;
    seqValues[FIXPROPERTYHANDLE_USEMAILUI] >>= m_bUseMailUI;
    seqValues[FIXPROPERTYHANDLE_CURRENTTEMPURL] >>= m_aCurrentTempURL;
}

//*****************************************************************************************************************
//  destructor
//*****************************************************************************************************************
SvtInternalOptions_Impl::~SvtInternalOptions_Impl()
{
    if( IsModified() == sal_True )
    {
        Commit();
    }
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtInternalOptions_Impl::Commit()
{
    // We have to write our current temp URL
    Sequence< OUString > aNames( 1 );
    OUString* pNames = aNames.getArray();
    Sequence< Any > aValues( 1 );
    Any* pValues = aValues.getArray();

    pNames[0] = FIXPROPERTYNAME_CURRENTTEMPURL;
    pValues[0] <<= m_aCurrentTempURL;

    PutProperties( aNames, aValues );
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtInternalOptions_Impl::SetCurrentTempURL( const OUString& aNewCurrentTempURL )
{
    m_aCurrentTempURL = aNewCurrentTempURL;
    SetModified();
    Commit();
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
Sequence< OUString > SvtInternalOptions_Impl::impl_GetPropertyNames()
{
    Sequence< OUString > seqProperties(4);

    // Add names of fix properties to list.
    seqProperties[FIXPROPERTYHANDLE_SLOTCFG         ]   =   FIXPROPERTYNAME_SLOTCFG         ;
    seqProperties[FIXPROPERTYHANDLE_SENDCRASHMAIL   ]   =   FIXPROPERTYNAME_SENDCRASHMAIL   ;
    seqProperties[FIXPROPERTYHANDLE_USEMAILUI       ]   =   FIXPROPERTYNAME_USEMAILUI       ;
    seqProperties[FIXPROPERTYHANDLE_CURRENTTEMPURL  ]   =   FIXPROPERTYNAME_CURRENTTEMPURL  ;
    // Return result.
    return seqProperties;
}

//*****************************************************************************************************************
//  initialize static member
//  DON'T DO IT IN YOUR HEADER!
//  see definition for further informations
//*****************************************************************************************************************
SvtInternalOptions_Impl*    SvtInternalOptions::m_pDataContainer    = NULL  ;
sal_Int32                   SvtInternalOptions::m_nRefCount         = 0     ;

//*****************************************************************************************************************
//  constructor
//*****************************************************************************************************************
SvtInternalOptions::SvtInternalOptions()
{
    // Global access, must be guarded (multithreading!).
    MutexGuard aGuard( GetOwnStaticMutex() );
    // Increase ouer refcount ...
    ++m_nRefCount;
    // ... and initialize ouer data container only if it not already!
    if( m_pDataContainer == NULL )
    {
        RTL_LOGFILE_CONTEXT(aLog, "unotools ( ??? ) ::SvtInternalOptions_Impl::ctor()");
        m_pDataContainer = new SvtInternalOptions_Impl();

        ItemHolder1::holdConfigItem(E_INTERNALOPTIONS);
     }
}

//*****************************************************************************************************************
//  destructor
//*****************************************************************************************************************
SvtInternalOptions::~SvtInternalOptions()
{
    // Global access, must be guarded (multithreading!)
    MutexGuard aGuard( GetOwnStaticMutex() );
    // Decrease ouer refcount.
    --m_nRefCount;
    // If last instance was deleted ...
    // we must destroy ouer static data container!
    if( m_nRefCount <= 0 )
    {
        delete m_pDataContainer;
        m_pDataContainer = NULL;
    }
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
sal_Bool SvtInternalOptions::SlotCFGEnabled() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->SlotCFGEnabled();
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
sal_Bool SvtInternalOptions::MailUIEnabled() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->MailUIEnabled();
}

OUString SvtInternalOptions::GetCurrentTempURL() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->GetCurrentTempURL();
}

void SvtInternalOptions::SetCurrentTempURL( const OUString& aNewCurrentTempURL )
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    m_pDataContainer->SetCurrentTempURL( aNewCurrentTempURL );
}

namespace
{
    class theInternalOptionsMutex : public rtl::Static<osl::Mutex, theInternalOptionsMutex>{};
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
Mutex& SvtInternalOptions::GetOwnStaticMutex()
{
    return theInternalOptionsMutex::get();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
