/*************************************************************************
 *
 *  $RCSfile: moduleoptions.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: as $ $Date: 2000-11-03 09:45:51 $
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

#ifndef _VOS_PROFILE_HXX_
#include <vos/profile.hxx>
#endif

//_________________________________________________________________________________________________________________
//  namespaces
//_________________________________________________________________________________________________________________

using namespace ::utl                   ;
using namespace ::rtl                   ;
using namespace ::osl                   ;
using namespace ::vos                   ;
using namespace ::com::sun::star::uno   ;

//_________________________________________________________________________________________________________________
//  const
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @descr          These values are used to define needed keys from ouer configuration management to support
                    all neccessary functionality of these implementation.
                    It's a fast way to make changes if some keys change his name or location!

                    Follow property handle are neccessary to specify right position in return list of configuration
                    for asked values. We ask it with a list of properties to get his values. The returned list
                    has the same order like ouer given name list!
                        NAMELIST[ PROPERTYHANDLE_xxx ] has value VALUELIST[ PROPERTYHANDLE_xxx ]!
                    See impl_GetPropertyNames() and his using for furter informations!
*//*-*************************************************************************************************************/

#define ROOTNODE_MODULES                    OUString(RTL_CONSTASCII_USTRINGPARAM("Setup/Office/Modules" ))

#define PROPERTYNAME_PORTALMATH             OUString(RTL_CONSTASCII_USTRINGPARAM("Math/Install"         ))
#define PROPERTYNAME_PORTALCHART            OUString(RTL_CONSTASCII_USTRINGPARAM("Chart/Install"        ))
#define PROPERTYNAME_PORTALCALC             OUString(RTL_CONSTASCII_USTRINGPARAM("Calc/Install"         ))
#define PROPERTYNAME_PORTALDRAW             OUString(RTL_CONSTASCII_USTRINGPARAM("Draw/Install"         ))
#define PROPERTYNAME_PORTALWRITER           OUString(RTL_CONSTASCII_USTRINGPARAM("Writer/Install"       ))
#define PROPERTYNAME_PORTALIMPRESS          OUString(RTL_CONSTASCII_USTRINGPARAM("Impress/Install"      ))
#define PROPERTYNAME_PORTALBASICIDE         OUString(RTL_CONSTASCII_USTRINGPARAM("BasicIDE/Install"     ))

/*TODO:

    Enable it if new configuration keys exist!
    in the moment use HACK with sversion.ini!

#define PROPERTYNAME_PORTALMATH             OUString(RTL_CONSTASCII_USTRINGPARAM("Math/PortalInstalled"         ))
#define PROPERTYNAME_PORTALCHART            OUString(RTL_CONSTASCII_USTRINGPARAM("Chart/PortalInstalled"        ))
#define PROPERTYNAME_PORTALCALC             OUString(RTL_CONSTASCII_USTRINGPARAM("Calc/PortalInstalled"         ))
#define PROPERTYNAME_PORTALDRAW             OUString(RTL_CONSTASCII_USTRINGPARAM("Draw/PortalInstalled"         ))
#define PROPERTYNAME_PORTALWRITER           OUString(RTL_CONSTASCII_USTRINGPARAM("Writer/PortalInstalled"       ))
#define PROPERTYNAME_PORTALIMPRESS          OUString(RTL_CONSTASCII_USTRINGPARAM("Impress/PortalInstalled"      ))
#define PROPERTYNAME_CLIENTMATH             OUString(RTL_CONSTASCII_USTRINGPARAM("Math/ClientInstalled"         ))
#define PROPERTYNAME_CLIENTCHART            OUString(RTL_CONSTASCII_USTRINGPARAM("Chart/ClientInstalled"        ))
#define PROPERTYNAME_CLIENTCALC             OUString(RTL_CONSTASCII_USTRINGPARAM("Calc/ClientInstalled"         ))
#define PROPERTYNAME_CLIENTDRAW             OUString(RTL_CONSTASCII_USTRINGPARAM("Draw/ClientInstalled"         ))
#define PROPERTYNAME_CLIENTWRITER           OUString(RTL_CONSTASCII_USTRINGPARAM("Writer/ClientInstalled"       ))
#define PROPERTYNAME_CLIENTIMPRESS          OUString(RTL_CONSTASCII_USTRINGPARAM("Impress/ClientInstalled"      ))
*/

#define PROPERTYHANDLE_PORTALMATH            0
#define PROPERTYHANDLE_PORTALCHART           1
#define PROPERTYHANDLE_PORTALCALC            2
#define PROPERTYHANDLE_PORTALDRAW            3
#define PROPERTYHANDLE_PORTALWRITER          4
#define PROPERTYHANDLE_PORTALIMPRESS         5
#define PROPERTYHANDLE_PORTALBASICIDE        6

/*TODO:

    Enable it if new configuration keys exist!
    in the moment use HACK with sversion.ini!

#define PROPERTYHANDLE_CLIENTMATH            6
#define PROPERTYHANDLE_CLIENTCHART           7
#define PROPERTYHANDLE_CLIENTCALC            8
#define PROPERTYHANDLE_CLIENTDRAW            9
#define PROPERTYHANDLE_CLIENTWRITER         10
#define PROPERTYHANDLE_CLIENTIMPRESS        11
*/

//#define   PROPERTYCOUNT                   12
#define PROPERTYCOUNT                       7

/*-************************************************************************************************************//**
    @descr          The return value of GetFeature() is a combination of different bit-flags. Follow const definitions
                    can be used to seperate all included informations from these return value!
                    These values can be used to enable/disable a module flag on a returned integer too.
                    To enable a module information we use ENABLEFEATURE_xxx defines ... to disable it the DISABLEFEATURE_xxx!
*//*-*************************************************************************************************************/

#define ENABLEFEATURE_BASICIDE              FEATUREFLAG_BASICIDE        // ----------1-----
#define ENABLEFEATURE_MATH                  FEATUREFLAG_MATH            // -------1--------
#define ENABLEFEATURE_CHART                 FEATUREFLAG_CHART           // ------1---------
#define ENABLEFEATURE_CALC                  FEATUREFLAG_CALC            // ----1-----------
#define ENABLEFEATURE_DRAW                  FEATUREFLAG_DRAW            // ---1------------
#define ENABLEFEATURE_WRITER                FEATUREFLAG_WRITER          // --1-------------
#define ENABLEFEATURE_IMPRESS               FEATUREFLAG_IMPRESS         // 1---------------

#define DISABLEFEATURE_BASICIDE             0xFFFFFFDF                  // 1111111111-11111
#define DISABLEFEATURE_MATH                 0xFFFFFEFF                  // 1111111-11111111
#define DISABLEFEATURE_CHART                0xFFFFFDFF                  // 111111-111111111
#define DISABLEFEATURE_CALC                 0xFFFFF7FF                  // 1111-11111111111
#define DISABLEFEATURE_DRAW                 0xFFFFEFFF                  // 111-111111111111
#define DISABLEFEATURE_WRITER               0xFFFFDFFF                  // 11-1111111111111
#define DISABLEFEATURE_IMPRESS              0xFFFF7FFF                  // -111111111111111

/*-************************************************************************************************************//**
    @descr          These defines are used for better code reading!
                    They represent the only possible states of module installation.
*//*-*************************************************************************************************************/

#define INSTALLED                           sal_True
#define NOT_INSTALLED                       sal_False

/*-************************************************************************************************************//**
    @descr          These defines are used as default values for m_nClient/PortalModules!
                    So you can change the initialization state in an easy manner.
*//*-*************************************************************************************************************/

#define DEFAULT_CLIENTMODULES               0
#define DEFAULT_PORTALMODULES               0

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

        sal_Bool    IsMath      ( sal_Bool bClient = sal_False ) const;
        sal_Bool    IsChart     ( sal_Bool bClient = sal_False ) const;
        sal_Bool    IsCalc      ( sal_Bool bClient = sal_False ) const;
        sal_Bool    IsDraw      ( sal_Bool bClient = sal_False ) const;
        sal_Bool    IsWriter    ( sal_Bool bClient = sal_False ) const;
        sal_Bool    IsImpress   ( sal_Bool bClient = sal_False ) const;
        sal_Bool    IsBasicIDE  ( sal_Bool bClient = sal_False ) const;
        sal_uInt32  GetFeatures ( sal_Bool bClient = sal_False ) const;

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

        static Sequence< OUString > impl_GetPropertyNames();

    //-------------------------------------------------------------------------------------------------------------
    //  private member
    //-------------------------------------------------------------------------------------------------------------

    private:

        sal_uInt32      m_nClientModules        ;   // cache states of localy cached components as flag field
        sal_uInt32      m_nPortalModules        ;   // cache states of portal installed components as flag field
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
    ,   m_nClientModules    ( DEFAULT_CLIENTMODULES )
    ,   m_nPortalModules    ( DEFAULT_PORTALMODULES )
{
    // Use our static list of configuration keys to get his values.
    const Sequence< OUString >  seqNames    = impl_GetPropertyNames (           );
    const Sequence< Any >       seqValues   = GetProperties         ( seqNames  );

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
        sal_Bool bState;
        seqValues[nProperty] >>= bState;
        switch( nProperty )
        {
/*TODO: Hack with sversion.ini
            case PROPERTYHANDLE_CLIENTMATH      :   {
                                                        if( bState == INSTALLED )
                                                        {
                                                            m_nClientModules |= ENABLEFEATURE_MATH;
                                                        }
                                                        else
                                                        {
                                                            m_nClientModules &= DISABLEFEATURE_MATH;
                                                        }
                                                    }
                                                    break;

            case PROPERTYHANDLE_CLIENTCHART     :   {
                                                        if( bState == INSTALLED )
                                                        {
                                                            m_nClientModules |= ENABLEFEATURE_CHART;
                                                        }
                                                        else
                                                        {
                                                            m_nClientModules &= DISABLEFEATURE_CHART;
                                                        }
                                                    }
                                                    break;

            case PROPERTYHANDLE_CLIENTCALC      :   {
                                                        if( bState == INSTALLED )
                                                        {
                                                            m_nClientModules |= ENABLEFEATURE_CALC;
                                                        }
                                                        else
                                                        {
                                                            m_nClientModules &= DISABLEFEATURE_CALC;
                                                        }
                                                    }
                                                    break;

            case PROPERTYHANDLE_CLIENTDRAW      :   {
                                                        if( bState == INSTALLED )
                                                        {
                                                            m_nClientModules |= ENABLEFEATURE_DRAW;
                                                        }
                                                        else
                                                        {
                                                            m_nClientModules &= DISABLEFEATURE_DRAW;
                                                        }
                                                    }
                                                    break;

            case PROPERTYHANDLE_CLIENTWRITER    :   {
                                                        if( bState == INSTALLED )
                                                        {
                                                            m_nClientModules |= ENABLEFEATURE_WRITER;
                                                        }
                                                        else
                                                        {
                                                            m_nClientModules &= DISABLEFEATURE_WRITER;
                                                        }
                                                    }
                                                    break;

            case PROPERTYHANDLE_CLIENTIMPRESS   :   {
                                                        if( bState == INSTALLED )
                                                        {
                                                            m_nClientModules |= ENABLEFEATURE_IMPRESS;
                                                        }
                                                        else
                                                        {
                                                            m_nClientModules &= DISABLEFEATURE_IMPRESS;
                                                        }
                                                    }
                                                    break;
*/
            case PROPERTYHANDLE_PORTALMATH      :   {
                                                        if( bState == INSTALLED )
                                                        {
                                                            m_nPortalModules |= ENABLEFEATURE_MATH;
                                                        }
                                                        else
                                                        {
                                                            m_nPortalModules &= DISABLEFEATURE_MATH;
                                                        }
                                                    }
                                                    break;

            case PROPERTYHANDLE_PORTALCHART     :   {
                                                        if( bState == INSTALLED )
                                                        {
                                                            m_nPortalModules |= ENABLEFEATURE_CHART;
                                                        }
                                                        else
                                                        {
                                                            m_nPortalModules &= DISABLEFEATURE_CHART;
                                                        }
                                                    }
                                                    break;

            case PROPERTYHANDLE_PORTALCALC      :   {
                                                        if( bState == INSTALLED )
                                                        {
                                                            m_nPortalModules |= ENABLEFEATURE_CALC;
                                                        }
                                                        else
                                                        {
                                                            m_nPortalModules &= DISABLEFEATURE_CALC;
                                                        }
                                                    }
                                                    break;

            case PROPERTYHANDLE_PORTALDRAW      :   {
                                                        if( bState == INSTALLED )
                                                        {
                                                            m_nPortalModules |= ENABLEFEATURE_DRAW;
                                                        }
                                                        else
                                                        {
                                                            m_nPortalModules &= DISABLEFEATURE_DRAW;
                                                        }
                                                    }
                                                    break;

            case PROPERTYHANDLE_PORTALWRITER    :   {
                                                        if( bState == INSTALLED )
                                                        {
                                                            m_nPortalModules |= ENABLEFEATURE_WRITER;
                                                        }
                                                        else
                                                        {
                                                            m_nPortalModules &= DISABLEFEATURE_WRITER;
                                                        }
                                                    }
                                                    break;

            case PROPERTYHANDLE_PORTALIMPRESS   :   {
                                                        if( bState == INSTALLED )
                                                        {
                                                            m_nPortalModules |= ENABLEFEATURE_IMPRESS;
                                                        }
                                                        else
                                                        {
                                                            m_nPortalModules &= DISABLEFEATURE_IMPRESS;
                                                        }
                                                    }
                                                    break;

            case PROPERTYHANDLE_PORTALBASICIDE  :   {
                                                        if( bState == INSTALLED )
                                                        {
                                                            m_nPortalModules |= ENABLEFEATURE_BASICIDE;
                                                        }
                                                        else
                                                        {
                                                            m_nPortalModules &= DISABLEFEATURE_BASICIDE;
                                                        }
                                                    }
                                                    break;

            default                             :   DBG_ERRORFILE( "SvtModuleOptions_Impl::SvtModuleOptions_Impl()\nWho has changed my property order mechanism?\n" );
        }
    }

/* Hack! -------------------------------------------------------------------------------------
    Read values for localy cached components from sversion.ini!
 */
    OUString sSVersionINI;
    OProfile::getProfileName( sSVersionINI, OUString::createFromAscii( "sversion" ), OUString::createFromAscii( "?^" ) );

    OProfile aSVersion;
    aSVersion.open( sSVersionINI );

    if( aSVersion.readBool( "StarWebTop 6.0 Plugin Priority", "application/vnd.stardivision.math", sal_False ) == sal_True )
    {
        m_nClientModules |= ENABLEFEATURE_MATH;
    }
    else
    {
        m_nClientModules &= DISABLEFEATURE_MATH;
    }
    if( aSVersion.readBool( "StarWebTop 6.0 Plugin Priority", "application/vnd.stardivision.chart", sal_False ) == sal_True )
    {
        m_nClientModules |= ENABLEFEATURE_CHART;
    }
    else
    {
        m_nClientModules &= DISABLEFEATURE_CHART;
    }
    if( aSVersion.readBool( "StarWebTop 6.0 Plugin Priority", "application/vnd.stardivision.calc", sal_False ) == sal_True )
    {
        m_nClientModules |= ENABLEFEATURE_CALC;
    }
    else
    {
        m_nClientModules &= DISABLEFEATURE_CALC;
    }
    if( aSVersion.readBool( "StarWebTop 6.0 Plugin Priority", "application/vnd.stardivision.draw", sal_False ) == sal_True )
    {
        m_nClientModules |= ENABLEFEATURE_DRAW;
    }
    else
    {
        m_nClientModules &= DISABLEFEATURE_DRAW;
    }
    if( aSVersion.readBool( "StarWebTop 6.0 Plugin Priority", "application/vnd.stardivision.writer", sal_False ) == sal_True )
    {
        m_nClientModules |= ENABLEFEATURE_WRITER;
    }
    else
    {
        m_nClientModules &= DISABLEFEATURE_WRITER;
    }
    if( aSVersion.readBool( "StarWebTop 6.0 Plugin Priority", "application/vnd.stardivision.impress", sal_False ) == sal_True )
    {
        m_nClientModules |= ENABLEFEATURE_IMPRESS;
    }
    else
    {
        m_nClientModules &= DISABLEFEATURE_IMPRESS;
    }

    // To read the sversion.ini is a hack ... and these new key isn't supported by a sversion entry!
    // Enable basic ide by default!!!
    m_nClientModules |= ENABLEFEATURE_BASICIDE;

    aSVersion.close();

/* Hack! -------------------------------------------------------------------------------------*/

    // I think we don't need any notifications ...
    // because we support readonly variables yet in the moment!
//  EnableNotification( seqNames );
}

//*****************************************************************************************************************
//  destructor
//*****************************************************************************************************************
SvtModuleOptions_Impl::~SvtModuleOptions_Impl()
{
    // We support readonly variables in the moment only.
    // There is no reason to commit ouer data set ...
    /*

    if( IsModified() == sal_True )
    {
        Commit();
    }

    */
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
sal_Bool SvtModuleOptions_Impl::IsMath( sal_Bool bClient ) const
{
    // Set default return state to "non installed".
    sal_Bool bState = sal_False;
    // Try to specify right install state for given search parameter!
    if( bClient == sal_True )
    {
        bState = ( m_nClientModules & FEATUREFLAG_MATH );
    }
    else
    {
        bState = ( m_nPortalModules & FEATUREFLAG_MATH );
    }
    // Return install state.
    return bState;
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
sal_Bool SvtModuleOptions_Impl::IsChart( sal_Bool bClient ) const
{
    // Set default return state to "non installed".
    sal_Bool bState = sal_False;
    // Try to specify right install state for given search parameter!
    if( bClient == sal_True )
    {
        bState = ( m_nClientModules & FEATUREFLAG_CHART );
    }
    else
    {
        bState = ( m_nPortalModules & FEATUREFLAG_CHART );
    }
    // Return install state.
    return bState;
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
sal_Bool SvtModuleOptions_Impl::IsCalc( sal_Bool bClient ) const
{
    // Set default return state to "non installed".
    sal_Bool bState = sal_False;
    // Try to specify right install state for given search parameter!
    if( bClient == sal_True )
    {
        bState = ( m_nClientModules & FEATUREFLAG_CALC );
    }
    else
    {
        bState = ( m_nPortalModules & FEATUREFLAG_CALC );
    }
    // Return install state.
    return bState;
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
sal_Bool SvtModuleOptions_Impl::IsDraw( sal_Bool bClient ) const
{
    // Set default return state to "non installed".
    sal_Bool bState = sal_False;
    // Try to specify right install state for given search parameter!
    if( bClient == sal_True )
    {
        bState = ( m_nClientModules & FEATUREFLAG_DRAW );
    }
    else
    {
        bState = ( m_nPortalModules & FEATUREFLAG_DRAW );
    }
    // Return install state.
    return bState;
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
sal_Bool SvtModuleOptions_Impl::IsWriter( sal_Bool bClient ) const
{
    // Set default return state to "non installed".
    sal_Bool bState = sal_False;
    // Try to specify right install state for given search parameter!
    if( bClient == sal_True )
    {
        bState = ( m_nClientModules & FEATUREFLAG_WRITER );
    }
    else
    {
        bState = ( m_nPortalModules & FEATUREFLAG_WRITER );
    }
    // Return install state.
    return bState;
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
sal_Bool SvtModuleOptions_Impl::IsImpress( sal_Bool bClient ) const
{
    // Set default return state to "non installed".
    sal_Bool bState = sal_False;
    // Try to specify right install state for given search parameter!
    if( bClient == sal_True )
    {
        bState = ( m_nClientModules & FEATUREFLAG_IMPRESS );
    }
    else
    {
        bState = ( m_nPortalModules & FEATUREFLAG_IMPRESS );
    }
    // Return install state.
    return bState;
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
sal_Bool SvtModuleOptions_Impl::IsBasicIDE( sal_Bool bClient ) const
{
    // Set default return state to "non installed".
    sal_Bool bState = sal_False;
    // Try to specify right install state for given search parameter!
    if( bClient == sal_True )
    {
        bState = ( m_nClientModules & FEATUREFLAG_BASICIDE );
    }
    else
    {
        bState = ( m_nPortalModules & FEATUREFLAG_BASICIDE );
    }
    // Return install state.
    return bState;
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
sal_uInt32 SvtModuleOptions_Impl::GetFeatures( sal_Bool bClient ) const
{
    // Set default return state to "non installed".
    sal_uInt32 nFeatures = 0;
    // Try to specify right install state for given search parameter!
    if( bClient == sal_True )
    {
        nFeatures = m_nClientModules;
    }
    else
    {
        nFeatures = m_nPortalModules;
    }
    // Return install state.
    return nFeatures;
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
Sequence< OUString > SvtModuleOptions_Impl::impl_GetPropertyNames()
{
    // Build static list of configuration key names.
    static const OUString pProperties[] =
    {
/*TODO: Hack width sversion.ini
        PROPERTYNAME_CLIENTMATH     ,
        PROPERTYNAME_CLIENTCHART    ,
        PROPERTYNAME_CLIENTCALC     ,
        PROPERTYNAME_CLIENTDRAW     ,
        PROPERTYNAME_CLIENTWRITER   ,
        PROPERTYNAME_CLIENTIMPRESS  ,*/
         PROPERTYNAME_PORTALMATH        ,
        PROPERTYNAME_PORTALCHART    ,
        PROPERTYNAME_PORTALCALC     ,
        PROPERTYNAME_PORTALDRAW     ,
        PROPERTYNAME_PORTALWRITER   ,
        PROPERTYNAME_PORTALIMPRESS  ,
        PROPERTYNAME_PORTALBASICIDE ,
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
    MutexGuard aGuard( GetOwnStaticMutex() );
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
sal_Bool SvtModuleOptions::IsMath( sal_Bool bClient ) const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->IsMath( bClient );
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
sal_Bool SvtModuleOptions::IsChart( sal_Bool bClient ) const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->IsChart( bClient );
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
sal_Bool SvtModuleOptions::IsCalc( sal_Bool bClient ) const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->IsCalc( bClient );
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
sal_Bool SvtModuleOptions::IsDraw( sal_Bool bClient ) const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->IsDraw( bClient );
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
sal_Bool SvtModuleOptions::IsWriter( sal_Bool bClient ) const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->IsWriter( bClient );
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
sal_Bool SvtModuleOptions::IsImpress( sal_Bool bClient ) const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->IsImpress( bClient );
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
sal_Bool SvtModuleOptions::IsBasicIDE( sal_Bool bClient ) const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->IsBasicIDE( bClient );
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
sal_uInt32 SvtModuleOptions::GetFeatures( sal_Bool bClient ) const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->GetFeatures( bClient );
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
Mutex& SvtModuleOptions::GetOwnStaticMutex()
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
