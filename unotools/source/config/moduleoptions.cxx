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


#include <unotools/moduleoptions.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/configitem.hxx>
#include <comphelper/processfactory.hxx>
#include <osl/diagnose.h>
#include <rtl/ustrbuf.hxx>

#include <rtl/instance.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/document/XTypeDetection.hpp>
#include <com/sun/star/util/PathSubstitution.hpp>
#include <com/sun/star/util/XStringSubstitution.hpp>

#include "itemholder1.hxx"

/*-************************************************************************************************************//**
    @descr          These values are used to define neccessary keys from our configuration management to support
                    all functionality of these implementation.
                    It's a fast way to make changes if some keys change his name or location!

                    Property handle are neccessary to specify right position in return list of configuration
                    for asked values. We ask it with a list of properties to get his values. The returned list
                    has the same order like our given name list!
                    e.g.:
                            NAMELIST[ PROPERTYHANDLE_xxx ] => VALUELIST[ PROPERTYHANDLE_xxx ]
*//*-*************************************************************************************************************/
#define ROOTNODE_FACTORIES                  OUString("Setup/Office/Factories"        )
#define PATHSEPARATOR                       OUString("/"                             )

// Attention: The property "ooSetupFactoryEmptyDocumentURL" is read from configuration but not used! There is
//            special code that uses hard coded strings to return them.
#define PROPERTYNAME_SHORTNAME              OUString("ooSetupFactoryShortName"       )
#define PROPERTYNAME_TEMPLATEFILE           OUString("ooSetupFactoryTemplateFile"    )
#define PROPERTYNAME_WINDOWATTRIBUTES       OUString("ooSetupFactoryWindowAttributes")
#define PROPERTYNAME_EMPTYDOCUMENTURL       OUString("ooSetupFactoryEmptyDocumentURL")
#define PROPERTYNAME_DEFAULTFILTER          OUString("ooSetupFactoryDefaultFilter"   )
#define PROPERTYNAME_ICON                   OUString("ooSetupFactoryIcon"            )

#define PROPERTYHANDLE_SHORTNAME            0
#define PROPERTYHANDLE_TEMPLATEFILE         1
#define PROPERTYHANDLE_WINDOWATTRIBUTES     2
#define PROPERTYHANDLE_EMPTYDOCUMENTURL     3
#define PROPERTYHANDLE_DEFAULTFILTER        4
#define PROPERTYHANDLE_ICON                 5

#define PROPERTYCOUNT                       6

#define FACTORYNAME_WRITER                  OUString("com.sun.star.text.TextDocument"                )
#define FACTORYNAME_WRITERWEB               OUString("com.sun.star.text.WebDocument"                 )
#define FACTORYNAME_WRITERGLOBAL            OUString("com.sun.star.text.GlobalDocument"              )
#define FACTORYNAME_CALC                    OUString("com.sun.star.sheet.SpreadsheetDocument"        )
#define FACTORYNAME_DRAW                    OUString("com.sun.star.drawing.DrawingDocument"          )
#define FACTORYNAME_IMPRESS                 OUString("com.sun.star.presentation.PresentationDocument")
#define FACTORYNAME_MATH                    OUString("com.sun.star.formula.FormulaProperties"        )
#define FACTORYNAME_CHART                   OUString("com.sun.star.chart2.ChartDocument"             )
#define FACTORYNAME_DATABASE                OUString("com.sun.star.sdb.OfficeDatabaseDocument"       )
#define FACTORYNAME_STARTMODULE             OUString("com.sun.star.frame.StartModule"                )

#define FACTORYCOUNT                        10

/*-************************************************************************************************************//**
    @descr  This struct hold information about one factory. We declare a complete array which can hold infos
            for all well known factories. Values of enum "EFactory" (see header!) are directly used as index!
            So we can support a fast access on these information.
*//*-*************************************************************************************************************/
struct FactoryInfo
{
    public:
        //---------------------------------------------------------------------------------------------------------
        // initialize empty struct
        FactoryInfo()
        {
            free();
        }

        //---------------------------------------------------------------------------------------------------------
        // easy way to reset struct member!
        void free()
        {
            bInstalled                  = sal_False;
            sFactory                    = "";
            sShortName                  = "";
            sTemplateFile               = "";
            sWindowAttributes           = "";
            sEmptyDocumentURL           = "";
            sDefaultFilter              = "";
            nIcon                       = 0;
            bChangedTemplateFile        = sal_False;
            bChangedWindowAttributes    = sal_False;
            bChangedEmptyDocumentURL    = sal_False;
            bChangedDefaultFilter       = sal_False;
            bChangedIcon                = sal_False;
            bDefaultFilterReadonly      = sal_False;
        }

        //---------------------------------------------------------------------------------------------------------
        // returns list of properties, which has changed only!
        // We use given value of sNodeBase to build full qualified paths ...
        // Last sign of it must be "/". because we use it directly, without any additional things!
        css::uno::Sequence< css::beans::PropertyValue > getChangedProperties( const OUString& sNodeBase )
        {
            // a) reserve memory for max. count of changed properties
            // b) add names and values of changed ones only and count it
            // c) resize return list by using count
            css::uno::Sequence< css::beans::PropertyValue > lProperties   ( 4 );
            sal_Int8                                        nRealyChanged = 0  ;

            if( bChangedTemplateFile == sal_True )
            {
                lProperties[nRealyChanged].Name = sNodeBase + PROPERTYNAME_TEMPLATEFILE;

                if ( !sTemplateFile.isEmpty() )
                {
                    lProperties[nRealyChanged].Value
                        <<= getStringSubstitution()
                            ->reSubstituteVariables( sTemplateFile );
                }
                else
                {
                    lProperties[nRealyChanged].Value <<= sTemplateFile;
                }

                ++nRealyChanged;
            }
            if( bChangedWindowAttributes == sal_True )
            {
                lProperties[nRealyChanged].Name    = sNodeBase + PROPERTYNAME_WINDOWATTRIBUTES;
                lProperties[nRealyChanged].Value <<= sWindowAttributes;
                ++nRealyChanged;
            }
            if( bChangedEmptyDocumentURL == sal_True )
            {
                lProperties[nRealyChanged].Name    = sNodeBase + PROPERTYNAME_EMPTYDOCUMENTURL;
                lProperties[nRealyChanged].Value <<= sEmptyDocumentURL;
                ++nRealyChanged;
            }
            if( bChangedDefaultFilter == sal_True )
            {
                lProperties[nRealyChanged].Name    = sNodeBase + PROPERTYNAME_DEFAULTFILTER;
                lProperties[nRealyChanged].Value <<= sDefaultFilter;
                ++nRealyChanged;
            }
            if( bChangedIcon == sal_True )
            {
                lProperties[nRealyChanged].Name    = sNodeBase + PROPERTYNAME_ICON;
                lProperties[nRealyChanged].Value <<= nIcon;
                ++nRealyChanged;
            }

            // Don't forget to reset changed flags! Otherwise we save it again and again and ...
            bChangedTemplateFile        = sal_False         ;
            bChangedWindowAttributes    = sal_False         ;
            bChangedEmptyDocumentURL    = sal_False         ;
            bChangedDefaultFilter       = sal_False         ;
            bChangedIcon                = sal_False         ;

            lProperties.realloc( nRealyChanged );
            return lProperties;
        }

        //---------------------------------------------------------------------------------------------------------
        // We must support setting AND marking of changed values.
        // That's why we can't make our member public. We must use get/set/init methods
        // to control access on it!
        sal_Bool            getInstalled        () const { return bInstalled;         };
        OUString     getFactory          () const { return sFactory;           };
        OUString     getShortName        () const { return sShortName;         };
        OUString     getTemplateFile     () const { return sTemplateFile;      };
        OUString     getWindowAttributes () const { return sWindowAttributes;  };
        OUString     getDefaultFilter    () const { return sDefaultFilter;     };
        sal_Bool            isDefaultFilterReadonly() const { return bDefaultFilterReadonly; }
        sal_Int32           getIcon             () const { return nIcon;              };

        //---------------------------------------------------------------------------------------------------------
        // If you call set-methods - we check for changes of valkues and mark it.
        // But if you whish to set it without that ... you must initialize it!
        void initInstalled        ( sal_Bool               bNewInstalled        ) { bInstalled        = bNewInstalled        ; }
        void initFactory          ( const OUString& sNewFactory          ) { sFactory          = sNewFactory          ; }
        void initShortName        ( const OUString& sNewShortName        ) { sShortName        = sNewShortName        ; }
        void initWindowAttributes ( const OUString& sNewWindowAttributes ) { sWindowAttributes = sNewWindowAttributes ; }
        void initEmptyDocumentURL ( const OUString& sNewEmptyDocumentURL ) { sEmptyDocumentURL = sNewEmptyDocumentURL ; }
        void initDefaultFilter    ( const OUString& sNewDefaultFilter    ) { sDefaultFilter    = sNewDefaultFilter    ; }
        void setDefaultFilterReadonly( const sal_Bool bVal){bDefaultFilterReadonly = bVal;}
        void initIcon             ( sal_Int32              nNewIcon             ) { nIcon             = nNewIcon             ; }

        //---------------------------------------------------------------------------------------------------------
        void initTemplateFile( const OUString& sNewTemplateFile )
        {
            if ( !sNewTemplateFile.isEmpty() )
            {
                sTemplateFile
                    = getStringSubstitution()
                        ->substituteVariables( sNewTemplateFile, sal_False );
            }
            else
            {
                sTemplateFile = sNewTemplateFile;
            }
        }

        //---------------------------------------------------------------------------------------------------------
        void setTemplateFile( const OUString& sNewTemplateFile )
        {
            if( sTemplateFile != sNewTemplateFile )
            {
                sTemplateFile        = sNewTemplateFile;
                bChangedTemplateFile = sal_True        ;
            }
        };

        //---------------------------------------------------------------------------------------------------------
        void setWindowAttributes( const OUString& sNewWindowAttributes )
        {
            if( sWindowAttributes != sNewWindowAttributes )
            {
                sWindowAttributes        = sNewWindowAttributes;
                bChangedWindowAttributes = sal_True            ;
            }
        };

        //---------------------------------------------------------------------------------------------------------
        void setDefaultFilter( const OUString& sNewDefaultFilter )
        {
            if( sDefaultFilter != sNewDefaultFilter )
            {
                sDefaultFilter       = sNewDefaultFilter;
                bChangedDefaultFilter = sal_True         ;
            }
        };

    private:
        css::uno::Reference< css::util::XStringSubstitution > getStringSubstitution()
        {
            if ( !xSubstVars.is() )
            {
                xSubstVars.set( css::util::PathSubstitution::create(::comphelper::getProcessComponentContext()) );
            }
            return xSubstVars;
        }

        sal_Bool            bInstalled                      ;
        OUString     sFactory                        ;
        OUString     sShortName                      ;
        OUString     sTemplateFile                   ;
        OUString     sWindowAttributes               ;
        OUString     sEmptyDocumentURL               ;
        OUString     sDefaultFilter                  ;
        sal_Int32           nIcon                           ;

        sal_Bool            bChangedTemplateFile        :1  ;
        sal_Bool            bChangedWindowAttributes    :1  ;
        sal_Bool            bChangedEmptyDocumentURL    :1  ;
        sal_Bool            bChangedDefaultFilter       :1  ;
        sal_Bool            bChangedIcon                :1  ;
        sal_Bool            bDefaultFilterReadonly      :1  ;

        css::uno::Reference< css::util::XStringSubstitution >  xSubstVars;
};

typedef FactoryInfo   FactoryInfoList[FACTORYCOUNT];

/*-************************************************************************************************************//**
    @short          IMPL data container for wrapper class SvtModulOptions!
    @descr          These class is used as a static data container of class SvtModuleOptions. The hold it by using
                    a refcount and make it threadsafe by using an osl mutex. So we don't must do anything for that.
                    We can implement pure functionality to read/write configuration data only.

    @implements     -
    @base           ConfigItem

    @devstatus      ready to use
    @threadsafe     no
*//*-*************************************************************************************************************/
class SvtModuleOptions_Impl : public ::utl::ConfigItem
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
        virtual void Notify( const css::uno::Sequence< OUString >& lPropertyNames );
        virtual void Commit(                                                             );

        //---------------------------------------------------------------------------------------------------------
        //  public interface
        //---------------------------------------------------------------------------------------------------------
        sal_Bool        IsModuleInstalled         (       SvtModuleOptions::EModule     eModule    ) const;
        ::com::sun::star::uno::Sequence < OUString > GetAllServiceNames();
        OUString GetFactoryName            (       SvtModuleOptions::EFactory    eFactory   ) const;
        OUString GetFactoryStandardTemplate(       SvtModuleOptions::EFactory    eFactory   ) const;
        OUString GetFactoryEmptyDocumentURL(       SvtModuleOptions::EFactory    eFactory   ) const;
        OUString GetFactoryDefaultFilter   (       SvtModuleOptions::EFactory    eFactory   ) const;
        sal_Bool        IsDefaultFilterReadonly(          SvtModuleOptions::EFactory eFactory      ) const;
        sal_Int32       GetFactoryIcon            (       SvtModuleOptions::EFactory    eFactory   ) const;
        static sal_Bool ClassifyFactoryByName     ( const OUString&              sName      ,
                                                          SvtModuleOptions::EFactory&   eFactory   );
        void            SetFactoryStandardTemplate(       SvtModuleOptions::EFactory    eFactory   ,
                                                    const OUString&              sTemplate  );
        void            SetFactoryDefaultFilter   (       SvtModuleOptions::EFactory    eFactory   ,
                                                    const OUString&              sFilter    );
        void            MakeReadonlyStatesAvailable();

    //-------------------------------------------------------------------------------------------------------------
    //  private methods
    //-------------------------------------------------------------------------------------------------------------
    private:
        static css::uno::Sequence< OUString > impl_ExpandSetNames ( const css::uno::Sequence< OUString >& lSetNames   );
               void                                  impl_Read           ( const css::uno::Sequence< OUString >& lSetNames   );

    //-------------------------------------------------------------------------------------------------------------
    //  private types
    //-------------------------------------------------------------------------------------------------------------
    private:

    //-------------------------------------------------------------------------------------------------------------
    //  private member
    //-------------------------------------------------------------------------------------------------------------
    private:
        FactoryInfoList     m_lFactories;
        sal_Bool            m_bReadOnlyStatesWellKnown;
};

/*-************************************************************************************************************//**
    @short      default ctor
    @descr      We open our configuration here and read all neccessary values from it.
                These values are cached till everyone call Commit(). Then we write changed ones back to cfg.

    @seealso    baseclass ConfigItem
    @seealso    method impl_Read()

    @param      -
    @return     -

    @onerror    -
    @threadsafe no
*//*-*************************************************************************************************************/
SvtModuleOptions_Impl::SvtModuleOptions_Impl()
    :   ::utl::ConfigItem( ROOTNODE_FACTORIES )
    ,   m_bReadOnlyStatesWellKnown( sal_False )
{
    // First initialize list of factory infos! Otherwise we couldnt gurantee right working of these class.
    for( sal_Int32 nFactory=0; nFactory<FACTORYCOUNT; ++nFactory )
        m_lFactories[nFactory].free();

    // Get name list of all existing set node names in configuration to read her properties in impl_Read().
    // These list is a list of long names of our factories.
    const css::uno::Sequence< OUString > lFactories = GetNodeNames( OUString() );
    impl_Read( lFactories );

    // Enable notification for changes by using configuration directly.
    // So we can update our internal values immediately.
    EnableNotification( lFactories );
}

/*-************************************************************************************************************//**
    @short      default dtor
    @descr      If any values of our cache was modified we should write it back to configuration.

    @attention  Don't forget to call "SetModified()" method of base class ConfigItem if any interface method
                of this class modify internal member list m_lFactories! Otherwise Commit() will never be called!!!

    @seealso    baseclass ConfigItem

    @param      -
    @return     -

    @onerror    -
    @threadsafe no
*//*-*************************************************************************************************************/
SvtModuleOptions_Impl::~SvtModuleOptions_Impl()
{
    if( IsModified() == sal_True )
    {
        Commit();
    }
}

/*-************************************************************************************************************//**
    @short      called for notify of configmanager
    @descr      These method is called from the ConfigManager before application ends or from the
                PropertyChangeListener if the sub tree broadcasts changes. You must update our
                internal values.

    @attention  We are registered for pure set node names only. So we can use our internal method "impl_Read()" to
                update our info list. Because - these method expand given name list to full qualified property list
                and use it to read the values. These values are filled into our internal member list m_lFactories
                at right position.

    @seealso    method impl_Read()

    @param      "lNames" is the list of set node entries which should be updated.
    @return     -

    @onerror    -
    @threadsafe no
*//*-*************************************************************************************************************/
void SvtModuleOptions_Impl::Notify( const css::uno::Sequence< OUString >& )
{
    OSL_FAIL( "SvtModuleOptions_Impl::Notify()\nNot implemented yet!\n" );
}

/*-****************************************************************************************************//**
    @short      write changes to configuration
    @descr      These method writes the changed values into the sub tree
                and should always called in our destructor to guarantee consistency of config data.

    @attention  We clear complete set in configuration first and write it completely new! So we don't must
                distinguish between existing, added or removed elements. Our internal cached values
                are the only and right ones.

    @seealso    baseclass ConfigItem

    @param      -
    @return     -

    @onerror    -
    @threadsafe no
*//*-*****************************************************************************************************/
void SvtModuleOptions_Impl::Commit()
{
    // Reserve memory for ALL possible factory properties!
    // Step over all factories and get her realy changed values only.
    // Build list of these ones and use it for commit.
    css::uno::Sequence< css::beans::PropertyValue > lCommitProperties( FACTORYCOUNT*PROPERTYCOUNT );
    FactoryInfo*                                    pInfo            = NULL                        ;
    sal_Int32                                       nRealCount       = 0                           ;
    OUString                                 sBasePath                                      ;
    for( sal_Int32 nFactory=0; nFactory<FACTORYCOUNT; ++nFactory )
    {
        pInfo = &(m_lFactories[nFactory]);

        // These path is used to build full qualified property names ....
        // See pInfo->getChangedProperties() for further information
        sBasePath  = PATHSEPARATOR + pInfo->getFactory() + PATHSEPARATOR;

        const css::uno::Sequence< css::beans::PropertyValue > lChangedProperties = pInfo->getChangedProperties ( sBasePath );
        const css::beans::PropertyValue*                      pChangedProperties = lChangedProperties.getConstArray();
        sal_Int32                                             nPropertyCount     = lChangedProperties.getLength();
        for( sal_Int32 nProperty=0; nProperty<nPropertyCount; ++nProperty )
        {
            lCommitProperties[nRealCount] = pChangedProperties[nProperty];
            ++nRealCount;
        }
    }
    // Resize commit list to real size.
    // If nothing to do - suppress calling of configuration ...
    // It could be to expensive :-)
    if( nRealCount > 0 )
    {
        lCommitProperties.realloc( nRealCount );
        SetSetProperties( OUString(), lCommitProperties );
    }
}

/*-****************************************************************************************************//**
    @short      access method to get internal values
    @descr      These methods implement easy access to our internal values.
                You give us right enum value to specify which module interest you ... we return right information.

    @attention  Some people use any value as enum ... but we support in header specified values only!
                We use it directly as index in our internal list. If enum value isn't right - we crash with an
                "index out of range"!!! Please use me right - otherwise there is no guarantee.

    @seealso    -

    @param      "eModule"  , index in list - specify module
    @param      "eFactory" , index in list - specify factory
    @param      "sTemplate", set new standard template for these factory
    @return     Queried information.

    @onerror    We return default values. (mostly "not installed"!)
    @threadsafe no
*//*-*****************************************************************************************************/
sal_Bool SvtModuleOptions_Impl::IsModuleInstalled( SvtModuleOptions::EModule eModule ) const
{
    sal_Bool bInstalled = sal_False;
    switch( eModule )
    {
        case SvtModuleOptions::E_SWRITER    :   bInstalled = m_lFactories[SvtModuleOptions::E_WRITER].getInstalled();
                                                break;
        case SvtModuleOptions::E_SWEB       :   bInstalled = m_lFactories[SvtModuleOptions::E_WRITERWEB].getInstalled();
                                                break;
        case SvtModuleOptions::E_SGLOBAL    :   bInstalled = m_lFactories[SvtModuleOptions::E_WRITERGLOBAL].getInstalled();
                                                break;
        case SvtModuleOptions::E_SCALC      :   bInstalled = m_lFactories[SvtModuleOptions::E_CALC].getInstalled();
                                                break;
        case SvtModuleOptions::E_SDRAW      :   bInstalled = m_lFactories[SvtModuleOptions::E_DRAW].getInstalled();
                                                break;
        case SvtModuleOptions::E_SIMPRESS   :   bInstalled = m_lFactories[SvtModuleOptions::E_IMPRESS].getInstalled();
                                                break;
        case SvtModuleOptions::E_SMATH      :   bInstalled = m_lFactories[SvtModuleOptions::E_MATH].getInstalled();
                                                break;
        case SvtModuleOptions::E_SCHART     :   bInstalled = m_lFactories[SvtModuleOptions::E_CHART].getInstalled();
                                                break;
        case SvtModuleOptions::E_SSTARTMODULE :   bInstalled = m_lFactories[SvtModuleOptions::E_STARTMODULE].getInstalled();
                                                break;
        case SvtModuleOptions::E_SBASIC     :   bInstalled = sal_True; // Couldn't be deselected by setup yet!
                                                break;
        case SvtModuleOptions::E_SDATABASE  :   bInstalled = m_lFactories[SvtModuleOptions::E_DATABASE].getInstalled();
                                                break;
    }

    return bInstalled;
}

::com::sun::star::uno::Sequence < OUString > SvtModuleOptions_Impl::GetAllServiceNames()
{
    sal_uInt32 nCount=0;
    if( m_lFactories[SvtModuleOptions::E_WRITER].getInstalled() )
        nCount++;
    if ( m_lFactories[SvtModuleOptions::E_WRITERWEB].getInstalled() )
        nCount++;
    if ( m_lFactories[SvtModuleOptions::E_WRITERGLOBAL].getInstalled() )
        nCount++;
    if( m_lFactories[SvtModuleOptions::E_SCALC].getInstalled() )
        nCount++;
    if( m_lFactories[SvtModuleOptions::E_SDRAW].getInstalled() )
        nCount++;
    if( m_lFactories[SvtModuleOptions::E_SIMPRESS].getInstalled() )
        nCount++;
    if( m_lFactories[SvtModuleOptions::E_SCHART].getInstalled() )
        nCount++;
    if( m_lFactories[SvtModuleOptions::E_SMATH].getInstalled() )
        nCount++;
    if( m_lFactories[SvtModuleOptions::E_SBASIC].getInstalled() )
        nCount++;
    if( m_lFactories[SvtModuleOptions::E_SDATABASE].getInstalled() )
        nCount++;

    css::uno::Sequence < OUString > aRet( nCount );
    sal_Int32 n=0;
    if( m_lFactories[SvtModuleOptions::E_WRITER].getInstalled() )
        aRet[n++] = m_lFactories[SvtModuleOptions::E_WRITER].getFactory();
    if ( m_lFactories[SvtModuleOptions::E_WRITERWEB].getInstalled() )
        aRet[n++] = m_lFactories[SvtModuleOptions::E_WRITERWEB].getFactory();
    if ( m_lFactories[SvtModuleOptions::E_WRITERGLOBAL].getInstalled() )
        aRet[n++] = m_lFactories[SvtModuleOptions::E_WRITERGLOBAL].getFactory();
    if( m_lFactories[SvtModuleOptions::E_SCALC].getInstalled() )
        aRet[n++] = m_lFactories[SvtModuleOptions::E_SCALC].getFactory();
    if( m_lFactories[SvtModuleOptions::E_SDRAW].getInstalled() )
        aRet[n++] = m_lFactories[SvtModuleOptions::E_SDRAW].getFactory();
    if( m_lFactories[SvtModuleOptions::E_SIMPRESS].getInstalled() )
        aRet[n++] = m_lFactories[SvtModuleOptions::E_SIMPRESS].getFactory();
    if( m_lFactories[SvtModuleOptions::E_SCHART].getInstalled() )
        aRet[n++] = m_lFactories[SvtModuleOptions::E_SCHART].getFactory();
    if( m_lFactories[SvtModuleOptions::E_SMATH].getInstalled() )
        aRet[n++] = m_lFactories[SvtModuleOptions::E_SMATH].getFactory();
    if( m_lFactories[SvtModuleOptions::E_SBASIC].getInstalled() )
        aRet[n++] = m_lFactories[SvtModuleOptions::E_SBASIC].getFactory();
    if( m_lFactories[SvtModuleOptions::E_SDATABASE].getInstalled() )
        aRet[n++] = m_lFactories[SvtModuleOptions::E_SDATABASE].getFactory();

    return aRet;
}

//*****************************************************************************************************************
OUString SvtModuleOptions_Impl::GetFactoryName( SvtModuleOptions::EFactory eFactory ) const
{
    OUString sName;

    if( eFactory>=0 && eFactory<FACTORYCOUNT )
    {
        sName = m_lFactories[eFactory].getFactory();
    }

    return sName;
}

//*****************************************************************************************************************
OUString SvtModuleOptions::GetFactoryShortName(SvtModuleOptions::EFactory eFactory)
{
    // Attention: Hard configured yet ... because it's not fine to make changes possible by xml file yet.
    //            But it's good to plan further possibilities!

    //return m_lFactories[eFactory].sShortName;

    OUString sShortName;
    switch( eFactory )
    {
        case SvtModuleOptions::E_WRITER        :  sShortName = "swriter";
                                                  break;
        case SvtModuleOptions::E_WRITERWEB     :  sShortName = "swriter/web";
                                                  break;
        case SvtModuleOptions::E_WRITERGLOBAL  :  sShortName = "swriter/GlobalDocument";
                                                  break;
        case SvtModuleOptions::E_CALC          :  sShortName = "scalc";
                                                  break;
        case SvtModuleOptions::E_DRAW          :  sShortName = "sdraw";
                                                  break;
        case SvtModuleOptions::E_IMPRESS       :  sShortName = "simpress";
                                                  break;
        case SvtModuleOptions::E_MATH          :  sShortName = "smath";
                                                  break;
        case SvtModuleOptions::E_CHART         :  sShortName = "schart";
                                                  break;
        case SvtModuleOptions::E_BASIC         :  sShortName = "sbasic";
                                                  break;
        case SvtModuleOptions::E_DATABASE     :  sShortName = "sdatabase";
                                                  break;
        default:
            OSL_FAIL( "unknown factory" );
            break;
    }

    return sShortName;
}

//*****************************************************************************************************************
OUString SvtModuleOptions_Impl::GetFactoryStandardTemplate( SvtModuleOptions::EFactory eFactory ) const
{
    OUString sFile;

    if( eFactory>=0 && eFactory<FACTORYCOUNT )
    {
        sFile = m_lFactories[eFactory].getTemplateFile();
    }

    return sFile;
}

//*****************************************************************************************************************
OUString SvtModuleOptions_Impl::GetFactoryEmptyDocumentURL( SvtModuleOptions::EFactory eFactory ) const
{
    // Attention: Hard configured yet ... because it's not fine to make changes possible by xml file yet.
    //            But it's good to plan further possibilities!

    //return m_lFactories[eFactory].getEmptyDocumentURL();

    OUString sURL;
    switch( eFactory )
    {
        case SvtModuleOptions::E_WRITER        :  sURL = "private:factory/swriter";
                                                  break;
        case SvtModuleOptions::E_WRITERWEB     :  sURL = "private:factory/swriter/web";
                                                  break;
        case SvtModuleOptions::E_WRITERGLOBAL  :  sURL = "private:factory/swriter/GlobalDocument";
                                                  break;
        case SvtModuleOptions::E_CALC          :  sURL = "private:factory/scalc";
                                                  break;
        case SvtModuleOptions::E_DRAW          :  sURL = "private:factory/sdraw";
                                                  break;
        case SvtModuleOptions::E_IMPRESS       :  sURL = "private:factory/simpress?slot=6686";
                                                  break;
        case SvtModuleOptions::E_MATH          :  sURL = "private:factory/smath";
                                                  break;
        case SvtModuleOptions::E_CHART         :  sURL = "private:factory/schart";
                                                  break;
        case SvtModuleOptions::E_BASIC         :  sURL = "private:factory/sbasic";
                                                  break;
        case SvtModuleOptions::E_DATABASE     :  sURL = "private:factory/sdatabase?Interactive";
                                                  break;
        default:
            OSL_FAIL( "unknown factory" );
            break;
    }
    return sURL;
}

//*****************************************************************************************************************
OUString SvtModuleOptions_Impl::GetFactoryDefaultFilter( SvtModuleOptions::EFactory eFactory ) const
{
    OUString sDefaultFilter;

    if( eFactory>=0 && eFactory<FACTORYCOUNT )
    {
        sDefaultFilter = m_lFactories[eFactory].getDefaultFilter();
    }
    return sDefaultFilter;
}
//*****************************************************************************************************************
sal_Bool SvtModuleOptions_Impl::IsDefaultFilterReadonly( SvtModuleOptions::EFactory eFactory   ) const
{
    sal_Bool bRet = sal_False;
    if( eFactory>=0 && eFactory<FACTORYCOUNT )
    {
        bRet = m_lFactories[eFactory].isDefaultFilterReadonly();
    }
    return bRet;
}

//*****************************************************************************************************************
sal_Int32 SvtModuleOptions_Impl::GetFactoryIcon( SvtModuleOptions::EFactory eFactory ) const
{
    sal_Int32 nIcon = 0;

    if( eFactory>=0 && eFactory<FACTORYCOUNT )
    {
        nIcon = m_lFactories[eFactory].getIcon();
    }

    return nIcon;
}

//*****************************************************************************************************************
void SvtModuleOptions_Impl::SetFactoryStandardTemplate(       SvtModuleOptions::EFactory eFactory   ,
                                                        const OUString&           sTemplate  )
{
    if( eFactory>=0 && eFactory<FACTORYCOUNT )
    {
        m_lFactories[eFactory].setTemplateFile( sTemplate );
        SetModified();
    }
}

//*****************************************************************************************************************
void SvtModuleOptions_Impl::SetFactoryDefaultFilter(       SvtModuleOptions::EFactory eFactory,
                                                     const OUString&           sFilter )
{
    if( eFactory>=0 && eFactory<FACTORYCOUNT )
    {
        m_lFactories[eFactory].setDefaultFilter( sFilter );
        SetModified();
    }
}

/*-************************************************************************************************************//**
    @short      return list of key names of ouer configuration management which represent our module tree
    @descr      You give use a list of current existing set node names .. and we expand it for all
                well known properties which are neccessary for this implementation.
                These full expanded list should be used to get values of this properties.

    @seealso    ctor

    @param      -
    @return     List of all relative addressed properties of given set entry names.

    @onerror    List will be empty.
    @threadsafe no
*//*-*************************************************************************************************************/
css::uno::Sequence< OUString > SvtModuleOptions_Impl::impl_ExpandSetNames( const css::uno::Sequence< OUString >& lSetNames )
{
    sal_Int32                             nCount     = lSetNames.getLength() ;
    css::uno::Sequence< OUString > lPropNames ( nCount*PROPERTYCOUNT );
    OUString*                      pPropNames = lPropNames.getArray() ;
    sal_Int32                             nPropStart = 0                     ;

    for( sal_Int32 nName=0; nName<nCount; ++nName )
    {
        pPropNames[nPropStart+PROPERTYHANDLE_SHORTNAME       ] = lSetNames[nName] + PATHSEPARATOR + PROPERTYNAME_SHORTNAME       ;
        pPropNames[nPropStart+PROPERTYHANDLE_TEMPLATEFILE    ] = lSetNames[nName] + PATHSEPARATOR + PROPERTYNAME_TEMPLATEFILE    ;
        pPropNames[nPropStart+PROPERTYHANDLE_WINDOWATTRIBUTES] = lSetNames[nName] + PATHSEPARATOR + PROPERTYNAME_WINDOWATTRIBUTES;
        pPropNames[nPropStart+PROPERTYHANDLE_EMPTYDOCUMENTURL] = lSetNames[nName] + PATHSEPARATOR + PROPERTYNAME_EMPTYDOCUMENTURL;
        pPropNames[nPropStart+PROPERTYHANDLE_DEFAULTFILTER   ] = lSetNames[nName] + PATHSEPARATOR + PROPERTYNAME_DEFAULTFILTER   ;
        pPropNames[nPropStart+PROPERTYHANDLE_ICON            ] = lSetNames[nName] + PATHSEPARATOR + PROPERTYNAME_ICON            ;
        nPropStart += PROPERTYCOUNT;
    }

    return lPropNames;
}

/*-************************************************************************************************************//**
    @short      helper to classify given factory by name
    @descr      Every factory has his own long and short name. So we can match right enum value for internal using.

    @attention  We change in/out parameter "eFactory" in every case! But you should use it only, if return value is sal_True!
                Algorithm:  Set out-parameter to propably value ... and check the longname.
                            If it match with these factory - break operation and return true AND right set parameter.
                            Otherwise try next one and so on. If no factory was found return false. Out parameter eFactory
                            is set to last tried value but shouldn't be used! Because our return value is false!

    @seealso    -

    @param      "sLongName" , long name of factory, which should be classified
    @return     "eFactory"  , right enum value, which match given long name
                and true for successfully classification, false otherwise

    @onerror    We return false.
    @threadsafe no
*//*-*************************************************************************************************************/
sal_Bool SvtModuleOptions_Impl::ClassifyFactoryByName( const OUString& sName, SvtModuleOptions::EFactory& eFactory )
{
    sal_Bool bState;

    eFactory = SvtModuleOptions::E_WRITER     ;
    bState   = ( sName == FACTORYNAME_WRITER );

    if( bState == sal_False )
    {
        eFactory = SvtModuleOptions::E_WRITERWEB     ;
        bState   = ( sName == FACTORYNAME_WRITERWEB );
    }
    // no else!
    if( bState == sal_False )
    {
        eFactory = SvtModuleOptions::E_WRITERGLOBAL     ;
        bState   = ( sName == FACTORYNAME_WRITERGLOBAL );
    }
    // no else!
    if( bState == sal_False )
    {
        eFactory = SvtModuleOptions::E_CALC     ;
        bState   = ( sName == FACTORYNAME_CALC );
    }
    // no else!
    if( bState == sal_False )
    {
        eFactory = SvtModuleOptions::E_DRAW     ;
        bState   = ( sName == FACTORYNAME_DRAW );
    }
    // no else!
    if( bState == sal_False )
    {
        eFactory = SvtModuleOptions::E_IMPRESS     ;
        bState   = ( sName == FACTORYNAME_IMPRESS );
    }
    // no else!
    if( bState == sal_False )
    {
        eFactory = SvtModuleOptions::E_MATH     ;
        bState   = ( sName == FACTORYNAME_MATH );
    }
    // no else!
    if( bState == sal_False )
    {
        eFactory = SvtModuleOptions::E_CHART     ;
        bState   = ( sName == FACTORYNAME_CHART );
    }
    // no else!
    if( bState == sal_False )
    {
        eFactory = SvtModuleOptions::E_DATABASE     ;
        bState   = ( sName == FACTORYNAME_DATABASE );
    }
    // no else!
    if( bState == sal_False )
    {
        eFactory = SvtModuleOptions::E_STARTMODULE    ;
        bState   = ( sName == FACTORYNAME_STARTMODULE);
    }

    return bState;
}

/*-************************************************************************************************************//**
    @short      read factory configuration
    @descr      Give us a list of pure factory names (long names!) which can be used as
                direct set node names ... and we read her property values and fill internal list.
                These method can be used by initial reading at ctor and later updating by "Notify()".

    @seealso    ctor
    @seealso    method Notify()

    @param      "lFactories" is the list of set node entries which should be readed.
    @return     -

    @onerror    We do nothing.
    @threadsafe no
*//*-*************************************************************************************************************/
void SvtModuleOptions_Impl::impl_Read( const css::uno::Sequence< OUString >& lFactories )
{
    // Expand every set node name in lFactories to full qualified paths to his properties
    // and get right values from configuration.
    const css::uno::Sequence< OUString > lProperties = impl_ExpandSetNames( lFactories  );
    const css::uno::Sequence< css::uno::Any >   lValues     = GetProperties( lProperties );

    // Safe impossible cases.
    // We need values from ALL configuration keys.
    // Follow assignment use order of values in relation to our list of key names!
    OSL_ENSURE( !(lProperties.getLength()!=lValues.getLength()), "SvtModuleOptions_Impl::impl_Read()\nI miss some values of configuration keys!\n" );

    // Algorithm:   We step over all given factory names and classify it. These enum value can be used as direct index
    //              in our member list m_lFactories! VAriable nPropertyStart marks start position of every factory
    //              and her properties in expanded property/value list. The defines PROPERTHANDLE_xxx are used as offset values
    //              added to nPropertyStart. So we can address every property relative in these lists.
    //              If we found any valid values ... we reset all existing information for corresponding m_lFactories-entry and
    //              use a pointer to these struct in memory directly to set new values.
    //              But we set it only, if bInstalled is true. Otherwise all other values of a factory can be undeclared .. They
    //              shouldn't be used then.
    // Attention:   If a propertyset of a factory will be ignored we must step to next start position of next factory infos!
    //              see "nPropertyStart += PROPERTYCOUNT" ...

    sal_Int32                   nPropertyStart  = 0                     ;
    sal_Int32                   nNodeCount      = lFactories.getLength();
    FactoryInfo*                pInfo           = NULL                  ;
    SvtModuleOptions::EFactory  eFactory                                ;

    for( sal_Int32 nSetNode=0; nSetNode<nNodeCount; ++nSetNode )
    {
        const OUString& sFactoryName = lFactories[nSetNode];
        if( ClassifyFactoryByName( sFactoryName, eFactory ) == sal_True )
        {
            OUString sTemp;
            sal_Int32       nTemp = 0;

            pInfo = &(m_lFactories[eFactory]);
            pInfo->free();

            pInfo->initInstalled( sal_True     );
            pInfo->initFactory  ( sFactoryName );

            if (lValues[nPropertyStart+PROPERTYHANDLE_SHORTNAME] >>= sTemp)
                pInfo->initShortName( sTemp );
            if (lValues[nPropertyStart+PROPERTYHANDLE_TEMPLATEFILE] >>= sTemp)
                pInfo->initTemplateFile( sTemp );
            if (lValues[nPropertyStart+PROPERTYHANDLE_WINDOWATTRIBUTES] >>= sTemp)
                pInfo->initWindowAttributes( sTemp );
            if (lValues[nPropertyStart+PROPERTYHANDLE_EMPTYDOCUMENTURL] >>= sTemp)
                pInfo->initEmptyDocumentURL( sTemp );
            if (lValues[nPropertyStart+PROPERTYHANDLE_DEFAULTFILTER   ] >>= sTemp)
                pInfo->initDefaultFilter( sTemp );
            if (lValues[nPropertyStart+PROPERTYHANDLE_ICON] >>= nTemp)
                pInfo->initIcon( nTemp );
        }
        nPropertyStart += PROPERTYCOUNT;
    }
}

//*****************************************************************************************************************
void SvtModuleOptions_Impl::MakeReadonlyStatesAvailable()
{
    if (m_bReadOnlyStatesWellKnown)
        return;

    css::uno::Sequence< OUString > lFactories = GetNodeNames(OUString());
    sal_Int32 c = lFactories.getLength();
    sal_Int32 i = 0;
    for (i=0; i<c; ++i)
    {
        OUStringBuffer sPath(256);
        sPath.append(lFactories[i]             );
        sPath.append(PATHSEPARATOR             );
        sPath.append(PROPERTYNAME_DEFAULTFILTER);

        lFactories[i] = sPath.makeStringAndClear();
    }

    css::uno::Sequence< sal_Bool > lReadonlyStates = GetReadOnlyStates(lFactories);
    for (i=0; i<c; ++i)
    {
        OUString&            rFactoryName = lFactories[i];
        SvtModuleOptions::EFactory  eFactory                    ;

        if (!ClassifyFactoryByName(rFactoryName, eFactory))
            continue;

        FactoryInfo& rInfo = m_lFactories[eFactory];
        rInfo.setDefaultFilterReadonly(lReadonlyStates[i]);
    }

    m_bReadOnlyStatesWellKnown = sal_True;
}

//*****************************************************************************************************************
//  initialize static member
//  DON'T DO IT IN YOUR HEADER!
//  see definition for further information
//*****************************************************************************************************************
SvtModuleOptions_Impl*  SvtModuleOptions::m_pDataContainer  = NULL  ;
sal_Int32               SvtModuleOptions::m_nRefCount       = 0     ;

/*-************************************************************************************************************//**
    @short      standard constructor and destructor
    @descr      This will initialize an instance with default values. We initialize/deinitialize our static data
                container and create a static mutex, which is used for threadsafe code in further time of this object.

    @seealso    method impl_GetOwnStaticMutex()

    @param      -
    @return     -

    @onerror    -
    @threadsafe yes
*//*-*************************************************************************************************************/
SvtModuleOptions::SvtModuleOptions()
{
    ::osl::MutexGuard aGuard( impl_GetOwnStaticMutex() );
    ++m_nRefCount;
    if( m_nRefCount == 1 )
    {
        m_pDataContainer = new SvtModuleOptions_Impl();

        ItemHolder1::holdConfigItem(E_MODULEOPTIONS);
    }
}

//*****************************************************************************************************************
SvtModuleOptions::~SvtModuleOptions()
{
    ::osl::MutexGuard aGuard( impl_GetOwnStaticMutex() );
    --m_nRefCount;
    if( m_nRefCount == 0 )
    {
        delete m_pDataContainer;
        m_pDataContainer = NULL;
    }
}

/*-************************************************************************************************************//**
    @short      access to configuration data
    @descr      This methods allow read/write access to configuration values.
                They are threadsafe. All calls are forwarded to impl-data-container. See there for further information!

    @seealso    method impl_GetOwnStaticMutex()

    @param      -
    @return     -

    @onerror    -
    @threadsafe yes
*//*-*************************************************************************************************************/
sal_Bool SvtModuleOptions::IsModuleInstalled( EModule eModule ) const
{
    ::osl::MutexGuard aGuard( impl_GetOwnStaticMutex() );
    return m_pDataContainer->IsModuleInstalled( eModule );
}

//*****************************************************************************************************************
OUString SvtModuleOptions::GetFactoryName( EFactory eFactory ) const
{
    ::osl::MutexGuard aGuard( impl_GetOwnStaticMutex() );
    return m_pDataContainer->GetFactoryName( eFactory );
}

//*****************************************************************************************************************
OUString SvtModuleOptions::GetFactoryStandardTemplate( EFactory eFactory ) const
{
    ::osl::MutexGuard aGuard( impl_GetOwnStaticMutex() );
    return m_pDataContainer->GetFactoryStandardTemplate( eFactory );
}

//*****************************************************************************************************************
OUString SvtModuleOptions::GetFactoryEmptyDocumentURL( EFactory eFactory ) const
{
    ::osl::MutexGuard aGuard( impl_GetOwnStaticMutex() );
    return m_pDataContainer->GetFactoryEmptyDocumentURL( eFactory );
}

//*****************************************************************************************************************
OUString SvtModuleOptions::GetFactoryDefaultFilter( EFactory eFactory ) const
{
    ::osl::MutexGuard aGuard( impl_GetOwnStaticMutex() );
    return m_pDataContainer->GetFactoryDefaultFilter( eFactory );
}
//*****************************************************************************************************************
sal_Bool SvtModuleOptions::IsDefaultFilterReadonly( EFactory eFactory   ) const
{
    ::osl::MutexGuard aGuard( impl_GetOwnStaticMutex() );
    m_pDataContainer->MakeReadonlyStatesAvailable();
    return m_pDataContainer->IsDefaultFilterReadonly( eFactory );
}
//*****************************************************************************************************************
sal_Int32 SvtModuleOptions::GetFactoryIcon( EFactory eFactory ) const
{
    ::osl::MutexGuard aGuard( impl_GetOwnStaticMutex() );
    return m_pDataContainer->GetFactoryIcon( eFactory );
}

//*****************************************************************************************************************
sal_Bool SvtModuleOptions::ClassifyFactoryByName( const OUString& sName    ,
                                                        EFactory&        eFactory )
{
    // We don't need any mutex here ... because we don't use any member here!
    return SvtModuleOptions_Impl::ClassifyFactoryByName( sName, eFactory );
}

//*****************************************************************************************************************
void SvtModuleOptions::SetFactoryStandardTemplate(       EFactory         eFactory   ,
                                                   const OUString& sTemplate  )
{
    ::osl::MutexGuard aGuard( impl_GetOwnStaticMutex() );
    m_pDataContainer->SetFactoryStandardTemplate( eFactory, sTemplate );
}

//*****************************************************************************************************************
void SvtModuleOptions::SetFactoryDefaultFilter(       EFactory         eFactory,
                                                const OUString& sFilter )
{
    ::osl::MutexGuard aGuard( impl_GetOwnStaticMutex() );
    m_pDataContainer->SetFactoryDefaultFilter( eFactory, sFilter );
}

//*****************************************************************************************************************
sal_Bool SvtModuleOptions::IsMath() const
{
    ::osl::MutexGuard aGuard( impl_GetOwnStaticMutex() );
    return m_pDataContainer->IsModuleInstalled( E_SMATH );
}

//*****************************************************************************************************************
sal_Bool SvtModuleOptions::IsChart() const
{
    ::osl::MutexGuard aGuard( impl_GetOwnStaticMutex() );
    return m_pDataContainer->IsModuleInstalled( E_SCHART );
}

//*****************************************************************************************************************
sal_Bool SvtModuleOptions::IsCalc() const
{
    ::osl::MutexGuard aGuard( impl_GetOwnStaticMutex() );
    return m_pDataContainer->IsModuleInstalled( E_SCALC );
}

//*****************************************************************************************************************
sal_Bool SvtModuleOptions::IsDraw() const
{
    ::osl::MutexGuard aGuard( impl_GetOwnStaticMutex() );
    return m_pDataContainer->IsModuleInstalled( E_SDRAW );
}

//*****************************************************************************************************************
sal_Bool SvtModuleOptions::IsWriter() const
{
    ::osl::MutexGuard aGuard( impl_GetOwnStaticMutex() );
    return m_pDataContainer->IsModuleInstalled( E_SWRITER );
}

//*****************************************************************************************************************
sal_Bool SvtModuleOptions::IsImpress() const
{
    ::osl::MutexGuard aGuard( impl_GetOwnStaticMutex() );
    return m_pDataContainer->IsModuleInstalled( E_SIMPRESS );
}

//*****************************************************************************************************************
sal_Bool SvtModuleOptions::IsBasicIDE() const
{
    return sal_True;
}
//*****************************************************************************************************************
sal_Bool SvtModuleOptions::IsDataBase() const
{
    ::osl::MutexGuard aGuard( impl_GetOwnStaticMutex() );
    return m_pDataContainer->IsModuleInstalled( E_SDATABASE );
}

namespace
{
    class theModuleOptionsMutex : public rtl::Static<osl::Mutex, theModuleOptionsMutex> {};
}
/*-****************************************************************************************************//**
    @short      return a reference to a static mutex
    @descr      These class is threadsafe.
                We create a static mutex only for one time and use it to protect our refcount and container
                member!

    @seealso    -

    @param      -
    @return     A reference to a static mutex member.

    @onerror    -
    @threadsafe yes
*//*-*****************************************************************************************************/
::osl::Mutex& SvtModuleOptions::impl_GetOwnStaticMutex()
{
    return theModuleOptionsMutex::get();
}

OUString SvtModuleOptions::GetModuleName( EModule eModule ) const
{
    switch( eModule )
    {
        case SvtModuleOptions::E_SWRITER    :   { return OUString("Writer"); }
        case SvtModuleOptions::E_SWEB       :   { return OUString("Web"); }
        case SvtModuleOptions::E_SGLOBAL    :   { return OUString("Global"); }
        case SvtModuleOptions::E_SCALC      :   { return OUString("Calc"); }
        case SvtModuleOptions::E_SDRAW      :   { return OUString("Draw"); }
        case SvtModuleOptions::E_SIMPRESS   :   { return OUString("Impress"); }
        case SvtModuleOptions::E_SMATH      :   { return OUString("Math"); }
        case SvtModuleOptions::E_SCHART     :   { return OUString("Chart"); }
        case SvtModuleOptions::E_SBASIC     :   { return OUString("Basic"); }
        case SvtModuleOptions::E_SDATABASE  :   { return OUString("Database"); }
        default:
            OSL_FAIL( "unknown module" );
            break;
    }

    return OUString();
}

SvtModuleOptions::EFactory SvtModuleOptions::ClassifyFactoryByShortName(const OUString& sName)
{
    if ( sName == "swriter" )
        return E_WRITER;
    if (sName.equalsIgnoreAsciiCase("swriter/Web")) // sometimes they are registerd for swriter/web :-(
        return E_WRITERWEB;
    if (sName.equalsIgnoreAsciiCase("swriter/GlobalDocument")) // sometimes they are registerd for swriter/globaldocument :-(
        return E_WRITERGLOBAL;
    if ( sName == "scalc" )
        return E_CALC;
    if ( sName == "sdraw" )
        return E_DRAW;
    if ( sName == "simpress" )
        return E_IMPRESS;
    if ( sName == "schart" )
        return E_CHART;
    if ( sName == "smath" )
        return E_MATH;
    if ( sName == "sbasic" )
        return E_BASIC;
    if ( sName == "sdatabase" )
        return E_DATABASE;

    return E_UNKNOWN_FACTORY;
}

SvtModuleOptions::EFactory SvtModuleOptions::ClassifyFactoryByServiceName(const OUString& sName)
{
    if (sName.equals(FACTORYNAME_WRITERGLOBAL))
        return E_WRITERGLOBAL;
    if (sName.equals(FACTORYNAME_WRITERWEB))
        return E_WRITERWEB;
    if (sName.equals(FACTORYNAME_WRITER))
        return E_WRITER;
    if (sName.equals(FACTORYNAME_CALC))
        return E_CALC;
    if (sName.equals(FACTORYNAME_DRAW))
        return E_DRAW;
    if (sName.equals(FACTORYNAME_IMPRESS))
        return E_IMPRESS;
    if (sName.equals(FACTORYNAME_MATH))
        return E_MATH;
    if (sName.equals(FACTORYNAME_CHART))
        return E_CHART;
    if (sName.equals(FACTORYNAME_DATABASE))
        return E_DATABASE;

    return E_UNKNOWN_FACTORY;
}

SvtModuleOptions::EFactory SvtModuleOptions::ClassifyFactoryByURL(const OUString&                                 sURL            ,
                                                                  const css::uno::Sequence< css::beans::PropertyValue >& lMediaDescriptor)
{
    css::uno::Reference< css::uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();

    css::uno::Reference< css::container::XNameAccess > xFilterCfg;
    css::uno::Reference< css::container::XNameAccess > xTypeCfg ;
    try
    {
        xFilterCfg = css::uno::Reference< css::container::XNameAccess >(
            xContext->getServiceManager()->createInstanceWithContext("com.sun.star.document.FilterFactory", xContext), css::uno::UNO_QUERY);
        xTypeCfg = css::uno::Reference< css::container::XNameAccess >(
            xContext->getServiceManager()->createInstanceWithContext("com.sun.star.document.TypeDetection", xContext), css::uno::UNO_QUERY);
    }
    catch(const css::uno::RuntimeException&)
        { throw; }
    catch(const css::uno::Exception&)
        { return E_UNKNOWN_FACTORY; }

    ::comphelper::SequenceAsHashMap stlDesc(lMediaDescriptor);

    // is there already a filter inside the descriptor?
    OUString sFilterName = stlDesc.getUnpackedValueOrDefault("FilterName", OUString());
    if (!sFilterName.isEmpty())
    {
        try
        {
            ::comphelper::SequenceAsHashMap stlFilterProps   (xFilterCfg->getByName(sFilterName));
            OUString                 sDocumentService = stlFilterProps.getUnpackedValueOrDefault("DocumentService", OUString());
            SvtModuleOptions::EFactory      eApp             = SvtModuleOptions::ClassifyFactoryByServiceName(sDocumentService);

            if (eApp != E_UNKNOWN_FACTORY)
                return eApp;
        }
        catch(const css::uno::RuntimeException&)
            { throw; }
        catch(const css::uno::Exception&)
            { /* do nothing here ... may the following code can help!*/ }
    }

    // is there already a type inside the descriptor?
    OUString sTypeName = stlDesc.getUnpackedValueOrDefault("TypeName", OUString());
    if (sTypeName.isEmpty())
    {
        // no :-(
        // start flat detection of URL
        css::uno::Reference< css::document::XTypeDetection > xDetect(xTypeCfg, css::uno::UNO_QUERY);
        sTypeName = xDetect->queryTypeByURL(sURL);
    }

    if (sTypeName.isEmpty())
        return E_UNKNOWN_FACTORY;

    // yes - there is a type info
    // Try to find the preferred filter.
    try
    {
        ::comphelper::SequenceAsHashMap stlTypeProps     (xTypeCfg->getByName(sTypeName));
        OUString                 sPreferredFilter = stlTypeProps.getUnpackedValueOrDefault("PreferredFilter", OUString());
        ::comphelper::SequenceAsHashMap stlFilterProps   (xFilterCfg->getByName(sPreferredFilter));
        OUString                 sDocumentService = stlFilterProps.getUnpackedValueOrDefault("DocumentService", OUString());
        SvtModuleOptions::EFactory      eApp             = SvtModuleOptions::ClassifyFactoryByServiceName(sDocumentService);

        if (eApp != E_UNKNOWN_FACTORY)
            return eApp;
    }
    catch(const css::uno::RuntimeException&)
        { throw; }
    catch(const css::uno::Exception&)
        { /* do nothing here ... may the following code can help!*/ }

    // no filter/no type/no detection result => no fun :-)
    return E_UNKNOWN_FACTORY;
}

SvtModuleOptions::EFactory SvtModuleOptions::ClassifyFactoryByModel(const css::uno::Reference< css::frame::XModel >& xModel)
{
    css::uno::Reference< css::lang::XServiceInfo > xInfo(xModel, css::uno::UNO_QUERY);
    if (!xInfo.is())
        return E_UNKNOWN_FACTORY;

    const css::uno::Sequence< OUString > lServices = xInfo->getSupportedServiceNames();
    const OUString*                      pServices = lServices.getConstArray();

    for (sal_Int32 i=0; i<lServices.getLength() ; ++i)
    {
        SvtModuleOptions::EFactory eApp = SvtModuleOptions::ClassifyFactoryByServiceName(pServices[i]);
        if (eApp != E_UNKNOWN_FACTORY)
            return eApp;
    }

    return E_UNKNOWN_FACTORY;
}

::com::sun::star::uno::Sequence < OUString > SvtModuleOptions::GetAllServiceNames()
{
    ::osl::MutexGuard aGuard( impl_GetOwnStaticMutex() );
    return m_pDataContainer->GetAllServiceNames();
}

OUString SvtModuleOptions::GetDefaultModuleName()
{
    OUString aModule;
    if (m_pDataContainer->IsModuleInstalled(SvtModuleOptions::E_SWRITER))
        aModule = GetFactoryShortName(SvtModuleOptions::E_WRITER);
    else if (m_pDataContainer->IsModuleInstalled(SvtModuleOptions::E_SCALC))
        aModule = GetFactoryShortName(SvtModuleOptions::E_CALC);
    else if (m_pDataContainer->IsModuleInstalled(SvtModuleOptions::E_SIMPRESS))
        aModule = GetFactoryShortName(SvtModuleOptions::E_IMPRESS);
    else if (m_pDataContainer->IsModuleInstalled(SvtModuleOptions::E_SDATABASE))
        aModule = GetFactoryShortName(SvtModuleOptions::E_DATABASE);
    else if (m_pDataContainer->IsModuleInstalled(SvtModuleOptions::E_SDRAW))
        aModule = GetFactoryShortName(SvtModuleOptions::E_DRAW);
    else if (m_pDataContainer->IsModuleInstalled(SvtModuleOptions::E_SWEB))
        aModule = GetFactoryShortName(SvtModuleOptions::E_WRITERWEB);
    else if (m_pDataContainer->IsModuleInstalled(SvtModuleOptions::E_SGLOBAL))
        aModule = GetFactoryShortName(SvtModuleOptions::E_WRITERGLOBAL);
    else if (m_pDataContainer->IsModuleInstalled(SvtModuleOptions::E_SMATH))
        aModule = GetFactoryShortName(SvtModuleOptions::E_MATH);
    return aModule;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
