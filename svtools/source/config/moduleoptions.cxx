/*************************************************************************
 *
 *  $RCSfile: moduleoptions.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: as $ $Date: 2000-10-26 15:48:04 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#pragma hdrstop

//_________________________________________________________________________________________________________________
//  includes
//_________________________________________________________________________________________________________________

#include "moduleoptions.hxx"

#ifndef _UTL_CONFIGMGR_HXX_
#include <unotools/configmgr.hxx>
#endif

#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _RTL_USTRING_
#include <rtl/ustring>
#endif

//_________________________________________________________________________________________________________________
//  namespaces
//_________________________________________________________________________________________________________________

using namespace ::utl                   ;
using namespace ::rtl                   ;
using namespace ::osl                   ;
using namespace ::com::sun::star::uno   ;

//_________________________________________________________________________________________________________________
//  const
//_________________________________________________________________________________________________________________

#define ROOTNODE_MODULES                OUString(RTL_CONSTASCII_USTRINGPARAM("Setup/Office/Modules"))
#define INSTALLED                       sal_True
#define NOT_INSTALLED                   sal_False
#define DEFAULT_INSTALLSTATE            NOT_INSTALLED

#define PROPERTYNAME_MATH               OUString(RTL_CONSTASCII_USTRINGPARAM("Math/Install"     ))
#define PROPERTYNAME_CHART              OUString(RTL_CONSTASCII_USTRINGPARAM("Chart/Install"    ))
#define PROPERTYNAME_CALC               OUString(RTL_CONSTASCII_USTRINGPARAM("Calc/Install"     ))
#define PROPERTYNAME_DRAW               OUString(RTL_CONSTASCII_USTRINGPARAM("Draw/Install"     ))
#define PROPERTYNAME_WRITER             OUString(RTL_CONSTASCII_USTRINGPARAM("Writer/Install"   ))
#define PROPERTYNAME_IMPRESS            OUString(RTL_CONSTASCII_USTRINGPARAM("Impress/Install"  ))

#define PROPERTYHANDLE_MATH             0
#define PROPERTYHANDLE_CHART            1
#define PROPERTYHANDLE_CALC             2
#define PROPERTYHANDLE_DRAW             3
#define PROPERTYHANDLE_WRITER           4
#define PROPERTYHANDLE_IMPRESS          5

#define PROPERTYCOUNT                   6

#define FEATUREFLAG_MATH                0x00000100
#define FEATUREFLAG_CHART               0x00000200
#define FEATUREFLAG_CALC                0x00000800
#define FEATUREFLAG_DRAW                0x00001000
#define FEATUREFLAG_WRITER              0x00002000
#define FEATUREFLAG_IMPRESS             0x00008000

//_________________________________________________________________________________________________________________
//  private declarations!
//_________________________________________________________________________________________________________________

class SvtModuleOptions_Impl : public ConfigItem
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------

    public:

        //---------------------------------------------------------------------------------------------------------
        //  constructor / destructor
        //---------------------------------------------------------------------------------------------------------

         SvtModuleOptions_Impl();
        ~SvtModuleOptions_Impl();

        //---------------------------------------------------------------------------------------------------------
        //  overloaded methods of baseclass
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      called for notify of configmanager
            @descr      These method is called from the ConfigManager before application ends or from the
                         PropertyChangeListener if the sub tree broadcasts changes. You must update your
                        internal values.

            @ATTENTION  These method is'nt implemented yet - because we support readonly values only...
                        There is no reason to change ouer values at runtime!

            @seealso    baseclass ConfigItem

            @param      "seqPropertyNames" is the list of properties which should be updated.
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void Notify( const Sequence< OUString >& seqPropertyNames )
        {
            DBG_ERRORFILE( "SvtModuleOptions_Impl::Notify()\nNot implemented yet! I think we don't need it for readonly values! ...\n" );
        }

        /*-****************************************************************************************************//**
            @short      write changes to configuration
            @descr      These method writes the changed values into the sub tree
                        and should always called in our destructor to guarantee consistency of config data.

            @ATTENTION  These method is'nt implemented yet - because we support readonly values only...
                        There is no reason to change ouer values at runtime!

            @seealso    baseclass ConfigItem

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void Commit()
        {
            DBG_ERRORFILE( "SvtModuleOptions_Impl::Commit()\nNot implemented yet! I think we don't need it for readonly values! ...\n" );
        }

        //---------------------------------------------------------------------------------------------------------
        //  public interface
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      access method to get internal values
            @descr      These methods regulate a readonly access to ouer internal variables!

            @seealso    -

            @param      -
            @return     sal_True if module is installed or sal_Fals if not.

            @onerror    -
        *//*-*****************************************************************************************************/

        sal_Bool IsMath     () const { return m_bMathInstalled      ; }
        sal_Bool IsChart    () const { return m_bChartInstalled     ; }
        sal_Bool IsCalc     () const { return m_bCalcInstalled      ; }
        sal_Bool IsDraw     () const { return m_bDrawInstalled      ; }
        sal_Bool IsWriter   () const { return m_bWriterInstalled    ; }
        sal_Bool IsImpress  () const { return m_bImpressInstalled   ; }

    //-------------------------------------------------------------------------------------------------------------
    //  private methods
    //-------------------------------------------------------------------------------------------------------------

    private:

        /*-****************************************************************************************************//**
            @short      return list of key names of ouer configuration management which represent oue module tree
            @descr      These methods return a static const list of key names. We need it to get needed values from our
                        configuration management.

            @seealso    -

            @param      -
            @return     A list of needed configuration keys is returned.

            @onerror    -
        *//*-*****************************************************************************************************/

        static Sequence< OUString > GetPropertyNames();

    //-------------------------------------------------------------------------------------------------------------
    //  private member
    //-------------------------------------------------------------------------------------------------------------

    private:

        sal_Bool    m_bMathInstalled        ;
        sal_Bool    m_bChartInstalled       ;
        sal_Bool    m_bCalcInstalled        ;
        sal_Bool    m_bDrawInstalled        ;
        sal_Bool    m_bWriterInstalled      ;
        sal_Bool    m_bImpressInstalled     ;
};

//_________________________________________________________________________________________________________________
//  definitions
//_________________________________________________________________________________________________________________

//*****************************************************************************************************************
//  constructor
//*****************************************************************************************************************
SvtModuleOptions_Impl::SvtModuleOptions_Impl()
    // Init baseclasses first
    :   ConfigItem          ( ROOTNODE_MODULES      )
    // Init member then.
    ,   m_bMathInstalled    ( DEFAULT_INSTALLSTATE  )
    ,   m_bChartInstalled   ( DEFAULT_INSTALLSTATE  )
    ,   m_bCalcInstalled    ( DEFAULT_INSTALLSTATE  )
    ,   m_bDrawInstalled    ( DEFAULT_INSTALLSTATE  )
    ,   m_bWriterInstalled  ( DEFAULT_INSTALLSTATE  )
    ,   m_bImpressInstalled ( DEFAULT_INSTALLSTATE  )
{
    // Use our static list of configuration keys to get his values.
    const Sequence< OUString >  seqNames    = GetPropertyNames  (           );
    const Sequence< Any >       seqValues   = GetProperties     ( seqNames  );

    // Safe impossible cases.
    // We need values from ALL configuration keys.
    // Follow assignment use order of values in relation to our list of key names!
    DBG_ASSERT( !(seqNames.getLength()!=seqValues.getLength()), "SvtModuleOptions_Impl::SvtModuleOptions_Impl()\nI miss some values of configuration keys!\n" );

    // Copy values from list in right order to ouer internal member.
    sal_Int32 nPropertyCount = seqValues.getLength();
    for( sal_Int32 nProperty=0; nProperty<nPropertyCount; ++nProperty )
    {
        // Safe impossible cases.
        // Check any for valid value.
        DBG_ASSERT( !(seqValues[nProperty].hasValue()==sal_False||seqValues[nProperty].getValueTypeClass()!=TypeClass_BOOLEAN), "SvtModuleOptions_Impl::SvtModuleOptions_Impl()\nInvalid property value detected!\n" );
        switch( nProperty )
        {
            case PROPERTYHANDLE_MATH    :   {
                                                seqValues[nProperty] >>= m_bMathInstalled;
                                            }
                                            break;

            case PROPERTYHANDLE_CHART   :   {
                                                seqValues[nProperty] >>= m_bChartInstalled;
                                            }
                                            break;

            case PROPERTYHANDLE_CALC    :   {
                                                seqValues[nProperty] >>= m_bCalcInstalled;
                                            }
                                            break;

            case PROPERTYHANDLE_DRAW    :   {
                                                seqValues[nProperty] >>= m_bDrawInstalled;
                                            }
                                            break;

            case PROPERTYHANDLE_WRITER  :   {
                                                seqValues[nProperty] >>= m_bWriterInstalled;
                                            }
                                            break;

            case PROPERTYHANDLE_IMPRESS :   {
                                                seqValues[nProperty] >>= m_bImpressInstalled;
                                            }
                                            break;
        }
    }

    // I think we don't need any notifications ...
    // because we support readonly variables yet in the moment!
//  EnableNotification( seqNames );
}

//*****************************************************************************************************************
//  destructor
//*****************************************************************************************************************
SvtModuleOptions_Impl::~SvtModuleOptions_Impl()
{
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
Sequence< OUString > SvtModuleOptions_Impl::GetPropertyNames()
{
    // Build static list of configuration key names.
    static const OUString pProperties[] =
    {
        PROPERTYNAME_MATH   ,
        PROPERTYNAME_CHART  ,
        PROPERTYNAME_CALC   ,
        PROPERTYNAME_DRAW   ,
        PROPERTYNAME_WRITER ,
        PROPERTYNAME_IMPRESS,
    };
    // Initialize return sequence with these list ...
    static const Sequence< OUString > seqPropertyNames( pProperties, PROPERTYCOUNT );
    // ... and return it.
    return seqPropertyNames;
}

//*****************************************************************************************************************
//  initialize static member
//  DON'T DO IT IN YOUR HEADER!
//  see definition for further informations
//*****************************************************************************************************************
SvtModuleOptions_Impl*  SvtModuleOptions::m_pDataContainer  = NULL  ;
sal_Int32               SvtModuleOptions::m_nRefCount       = 0     ;

//*****************************************************************************************************************
//  constructor
//*****************************************************************************************************************
SvtModuleOptions::SvtModuleOptions()
{
    // Global access, must be guarded (multithreading!).
    MutexGuard aGuard( GetInitMutex() );
    // Increase ouer refcount ...
    ++m_nRefCount;
    // ... and initialize ouer data container only if it not already exist!
    if( m_pDataContainer == NULL )
    {
        m_pDataContainer = new SvtModuleOptions_Impl;
    }
}

//*****************************************************************************************************************
//  destructor
//*****************************************************************************************************************
SvtModuleOptions::~SvtModuleOptions()
{
    // Global access, must be guarded (multithreading!)
    MutexGuard aGuard( GetInitMutex() );
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
sal_Bool SvtModuleOptions::IsMath() const
{
    // These member are readonly ... and access to it is atomar ...
    // I think we don't need any mutex here!
    return m_pDataContainer->IsMath();
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
sal_Bool SvtModuleOptions::IsChart() const
{
    // These member are readonly ... and access to it is atomar ...
    // I think we don't need any mutex here!
    return m_pDataContainer->IsChart();
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
sal_Bool SvtModuleOptions::IsCalc() const
{
    // These member are readonly ... and access to it is atomar ...
    // I think we don't need any mutex here!
    return m_pDataContainer->IsCalc();
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
sal_Bool SvtModuleOptions::IsDraw() const
{
    // These member are readonly ... and access to it is atomar ...
    // I think we don't need any mutex here!
    return m_pDataContainer->IsDraw();
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
sal_Bool SvtModuleOptions::IsWriter() const
{
    // These member are readonly ... and access to it is atomar ...
    // I think we don't need any mutex here!
    return m_pDataContainer->IsWriter();
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
sal_Bool SvtModuleOptions::IsImpress() const
{
    // These member are readonly ... and access to it is atomar ...
    // I think we don't need any mutex here!
    return m_pDataContainer->IsImpress();
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
sal_uInt32 SvtModuleOptions::GetFeatures() const
{
    // Set default return value to "no features exist".
    sal_uInt32 nFeatures = 0;

    // Check every feature and collect only existing ones!
    if( m_pDataContainer->IsMath() == sal_True )
    {
        nFeatures |= FEATUREFLAG_MATH;
    }
    if( m_pDataContainer->IsChart() == sal_True )
    {
        nFeatures |= FEATUREFLAG_CHART;
    }
    if( m_pDataContainer->IsCalc() == sal_True )
    {
        nFeatures |= FEATUREFLAG_CALC;
    }
    if( m_pDataContainer->IsDraw() == sal_True )
    {
        nFeatures |= FEATUREFLAG_DRAW;
    }
    if( m_pDataContainer->IsWriter() == sal_True )
    {
        nFeatures |= FEATUREFLAG_WRITER;
    }
    if( m_pDataContainer->IsImpress() == sal_True )
    {
        nFeatures |= FEATUREFLAG_IMPRESS;
    }
    // Return result of operation.
    return nFeatures;
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
Mutex& SvtModuleOptions::GetInitMutex()
{
    // Initialize static mutex only for one time!
    static Mutex* pMutex = NULL;
    // If these method first called (Mutex not already exist!) ...
    if( pMutex == NULL )
    {
        // ... we must create a new one. Protect follow code with the global mutex -
        // It must be - we create a static variable!
        MutexGuard aGuard( Mutex::getGlobalMutex() );
        // We must check our pointer again - because it can be that another instance of ouer class will be fastr then these!
        if( pMutex == NULL )
        {
            // Create the new mutex and set it for return on static variable.
            static Mutex aMutex;
            pMutex = &aMutex;
        }
    }
    // Return new created or already existing mutex object.
    return *pMutex;
}
