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
#include <comphelper/sequence.hxx>
#include <osl/diagnose.h>
#include <o3tl/enumarray.hxx>
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

/*-************************************************************************************************************
    @descr          These values are used to define necessary keys from our configuration management to support
                    all functionality of these implementation.
                    It's a fast way to make changes if some keys change his name or location!

                    Property handle are necessary to specify right position in return list of configuration
                    for asked values. We ask it with a list of properties to get his values. The returned list
                    has the same order like our given name list!
                    e.g.:
                            NAMELIST[ PROPERTYHANDLE_xxx ] => VALUELIST[ PROPERTYHANDLE_xxx ]
*//*-*************************************************************************************************************/
#define ROOTNODE_FACTORIES                  "Setup/Office/Factories"
#define PATHSEPARATOR                       "/"

// Attention: The property "ooSetupFactoryEmptyDocumentURL" is read from configuration but not used! There is
//            special code that uses hard coded strings to return them.
#define PROPERTYNAME_SHORTNAME              "ooSetupFactoryShortName"
#define PROPERTYNAME_TEMPLATEFILE           "ooSetupFactoryTemplateFile"
#define PROPERTYNAME_WINDOWATTRIBUTES       "ooSetupFactoryWindowAttributes"
#define PROPERTYNAME_EMPTYDOCUMENTURL       "ooSetupFactoryEmptyDocumentURL"
#define PROPERTYNAME_DEFAULTFILTER          "ooSetupFactoryDefaultFilter"
#define PROPERTYNAME_ICON                   "ooSetupFactoryIcon"

#define PROPERTYHANDLE_SHORTNAME            0
#define PROPERTYHANDLE_TEMPLATEFILE         1
#define PROPERTYHANDLE_WINDOWATTRIBUTES     2
#define PROPERTYHANDLE_EMPTYDOCUMENTURL     3
#define PROPERTYHANDLE_DEFAULTFILTER        4
#define PROPERTYHANDLE_ICON                 5

#define PROPERTYCOUNT                       6

#define FACTORYNAME_WRITER                  "com.sun.star.text.TextDocument"
#define FACTORYNAME_WRITERWEB               "com.sun.star.text.WebDocument"
#define FACTORYNAME_WRITERGLOBAL            "com.sun.star.text.GlobalDocument"
#define FACTORYNAME_CALC                    "com.sun.star.sheet.SpreadsheetDocument"
#define FACTORYNAME_DRAW                    "com.sun.star.drawing.DrawingDocument"
#define FACTORYNAME_IMPRESS                 "com.sun.star.presentation.PresentationDocument"
#define FACTORYNAME_MATH                    "com.sun.star.formula.FormulaProperties"
#define FACTORYNAME_CHART                   "com.sun.star.chart2.ChartDocument"
#define FACTORYNAME_DATABASE                "com.sun.star.sdb.OfficeDatabaseDocument"
#define FACTORYNAME_STARTMODULE             "com.sun.star.frame.StartModule"
#define FACTORYNAME_BASIC                   "com.sun.star.script.BasicIDE"

#define FACTORYCOUNT                        11

/*-************************************************************************************************************
    @descr  This struct hold information about one factory. We declare a complete array which can hold infos
            for all well known factories. Values of enum "EFactory" (see header!) are directly used as index!
            So we can support a fast access on these information.
*//*-*************************************************************************************************************/
struct FactoryInfo
{
    public:

        // initialize empty struct
        FactoryInfo()
        {
            free();
        }

        // easy way to reset struct member!
        void free()
        {
            bInstalled                  = false;
            sFactory.clear();
            sShortName.clear();
            sTemplateFile.clear();
            sWindowAttributes.clear();
            sEmptyDocumentURL.clear();
            sDefaultFilter.clear();
            nIcon                       = 0;
            bChangedTemplateFile        = false;
            bChangedDefaultFilter       = false;
            bDefaultFilterReadonly      = false;
        }

        // returns list of properties, which has changed only!
        // We use given value of sNodeBase to build full qualified paths ...
        // Last sign of it must be "/". because we use it directly, without any additional things!
        css::uno::Sequence< css::beans::PropertyValue > getChangedProperties( const OUString& sNodeBase )
        {
            // a) reserve memory for max. count of changed properties
            // b) add names and values of changed ones only and count it
            // c) resize return list by using count
            css::uno::Sequence< css::beans::PropertyValue > lProperties   ( 4 );
            sal_Int8                                        nRealyChanged = 0;

            if( bChangedTemplateFile )
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
            if( bChangedDefaultFilter )
            {
                lProperties[nRealyChanged].Name    = sNodeBase + PROPERTYNAME_DEFAULTFILTER;
                lProperties[nRealyChanged].Value <<= sDefaultFilter;
                ++nRealyChanged;
            }

            // Don't forget to reset changed flags! Otherwise we save it again and again and ...
            bChangedTemplateFile        = false;
            bChangedDefaultFilter       = false;

            lProperties.realloc( nRealyChanged );
            return lProperties;
        }

        // We must support setting AND marking of changed values.
        // That's why we can't make our member public. We must use get/set/init methods
        // to control access on it!
        bool            getInstalled        () const { return bInstalled;         };
        const OUString& getFactory          () const { return sFactory;           };
        const OUString& getTemplateFile     () const { return sTemplateFile;      };
        const OUString& getDefaultFilter    () const { return sDefaultFilter;     };
        bool            isDefaultFilterReadonly() const { return bDefaultFilterReadonly; }
        sal_Int32       getIcon             () const { return nIcon;              };

        // If you call set-methods - we check for changes of values and mark it.
        // But if you wish to set it without that... you must initialize it!
        void initInstalled        ()                                       { bInstalled        = true; }
        void initFactory          ( const OUString& sNewFactory          ) { sFactory          = sNewFactory; }
        void initShortName        ( const OUString& sNewShortName        ) { sShortName        = sNewShortName; }
        void initWindowAttributes ( const OUString& sNewWindowAttributes ) { sWindowAttributes = sNewWindowAttributes; }
        void initEmptyDocumentURL ( const OUString& sNewEmptyDocumentURL ) { sEmptyDocumentURL = sNewEmptyDocumentURL; }
        void initDefaultFilter    ( const OUString& sNewDefaultFilter    ) { sDefaultFilter    = sNewDefaultFilter; }
        void setDefaultFilterReadonly( const bool bVal){bDefaultFilterReadonly = bVal;}
        void initIcon             ( sal_Int32              nNewIcon             ) { nIcon             = nNewIcon; }

        void initTemplateFile( const OUString& sNewTemplateFile )
        {
            if ( !sNewTemplateFile.isEmpty() )
            {
                sTemplateFile= getStringSubstitution()->substituteVariables( sNewTemplateFile, false );
            }
            else
            {
                sTemplateFile = sNewTemplateFile;
            }
        }

        void setTemplateFile( const OUString& sNewTemplateFile )
        {
            if( sTemplateFile != sNewTemplateFile )
            {
                sTemplateFile        = sNewTemplateFile;
                bChangedTemplateFile = true;
            }
        };

        void setDefaultFilter( const OUString& sNewDefaultFilter )
        {
            if( sDefaultFilter != sNewDefaultFilter )
            {
                sDefaultFilter       = sNewDefaultFilter;
                bChangedDefaultFilter = true;
            }
        };

    private:
        css::uno::Reference< css::util::XStringSubstitution > const & getStringSubstitution()
        {
            if ( !xSubstVars.is() )
            {
                xSubstVars.set( css::util::PathSubstitution::create(::comphelper::getProcessComponentContext()) );
            }
            return xSubstVars;
        }

        bool         bInstalled;
        OUString     sFactory;
        OUString     sShortName;
        OUString     sTemplateFile;
        OUString     sWindowAttributes;
        OUString     sEmptyDocumentURL;
        OUString     sDefaultFilter;
        sal_Int32    nIcon;

        bool            bChangedTemplateFile        :1;
        bool            bChangedDefaultFilter       :1;
        bool            bDefaultFilterReadonly      :1;

        css::uno::Reference< css::util::XStringSubstitution >  xSubstVars;
};

class SvtModuleOptions_Impl : public ::utl::ConfigItem
{

    //  public methods

    public:

        //  constructor / destructor

         SvtModuleOptions_Impl();
        virtual ~SvtModuleOptions_Impl() override;

        //  override methods of baseclass

        virtual void Notify( const css::uno::Sequence< OUString >& lPropertyNames ) override;

        //  public interface

        bool            IsModuleInstalled         (       SvtModuleOptions::EModule     eModule    ) const;
        css::uno::Sequence < OUString > GetAllServiceNames();
        OUString        GetFactoryName            (       SvtModuleOptions::EFactory    eFactory   ) const;
        OUString        GetFactoryStandardTemplate(       SvtModuleOptions::EFactory    eFactory   ) const;
        static OUString GetFactoryEmptyDocumentURL(       SvtModuleOptions::EFactory    eFactory   );
        OUString        GetFactoryDefaultFilter   (       SvtModuleOptions::EFactory    eFactory   ) const;
        bool            IsDefaultFilterReadonly(          SvtModuleOptions::EFactory eFactory      ) const;
        sal_Int32       GetFactoryIcon            (       SvtModuleOptions::EFactory    eFactory   ) const;
        static bool     ClassifyFactoryByName     ( const OUString&              sName      ,
                                                          SvtModuleOptions::EFactory&   eFactory   );
        void            SetFactoryStandardTemplate(       SvtModuleOptions::EFactory    eFactory   ,
                                                    const OUString&              sTemplate  );
        void            SetFactoryDefaultFilter   (       SvtModuleOptions::EFactory    eFactory   ,
                                                    const OUString&              sFilter    );
        void            MakeReadonlyStatesAvailable();

    //  private methods

    private:
        static css::uno::Sequence< OUString > impl_ExpandSetNames ( const css::uno::Sequence< OUString >& lSetNames );
        void impl_Read ( const css::uno::Sequence< OUString >& lSetNames );

        virtual void ImplCommit() override;

    //  private member

    private:
        o3tl::enumarray<SvtModuleOptions::EFactory, FactoryInfo> m_lFactories;
        bool            m_bReadOnlyStatesWellKnown;
};

/*-************************************************************************************************************
    @short      default ctor
    @descr      We open our configuration here and read all necessary values from it.
                These values are cached till everyone call Commit(). Then we write changed ones back to cfg.

    @seealso    baseclass ConfigItem
    @seealso    method impl_Read()
    @threadsafe no
*//*-*************************************************************************************************************/
SvtModuleOptions_Impl::SvtModuleOptions_Impl()
    :   ::utl::ConfigItem( ROOTNODE_FACTORIES )
    ,   m_bReadOnlyStatesWellKnown( false )
{
    // First initialize list of factory infos! Otherwise we couldnt guarantee right working of these class.
    for( auto & rFactory : m_lFactories )
        rFactory.free();

    // Get name list of all existing set node names in configuration to read her properties in impl_Read().
    // These list is a list of long names of our factories.
    const css::uno::Sequence< OUString > lFactories = GetNodeNames( OUString() );
    impl_Read( lFactories );

    // Enable notification for changes by using configuration directly.
    // So we can update our internal values immediately.
    EnableNotification( lFactories );
}

SvtModuleOptions_Impl::~SvtModuleOptions_Impl()
{
    assert(!IsModified()); // should have been committed
}

/*-************************************************************************************************************
    @short      called for notify of configmanager
    @descr      This method is called from the ConfigManager before application ends or from the
                PropertyChangeListener if the sub tree broadcasts changes. You must update our
                internal values.

    @attention  We are registered for pure set node names only. So we can use our internal method "impl_Read()" to
                update our info list. Because - this method expand given name list to full qualified property list
                and use it to read the values. These values are filled into our internal member list m_lFactories
                at right position.

    @seealso    method impl_Read()

    @param      "lNames" is the list of set node entries which should be updated.
    @threadsafe no
*//*-*************************************************************************************************************/
void SvtModuleOptions_Impl::Notify( const css::uno::Sequence< OUString >& )
{
    OSL_FAIL( "SvtModuleOptions_Impl::Notify()\nNot implemented yet!\n" );
}

/*-****************************************************************************************************
    @short      write changes to configuration
    @descr      This method writes the changed values into the sub tree
                and should always called in our destructor to guarantee consistency of config data.

    @attention  We clear complete set in configuration first and write it completely new! So we don't must
                distinguish between existing, added or removed elements. Our internal cached values
                are the only and right ones.

    @seealso    baseclass ConfigItem
    @threadsafe no
*//*-*****************************************************************************************************/
void SvtModuleOptions_Impl::ImplCommit()
{
    // Reserve memory for ALL possible factory properties!
    // Step over all factories and get her really changed values only.
    // Build list of these ones and use it for commit.
    css::uno::Sequence< css::beans::PropertyValue > lCommitProperties( FACTORYCOUNT*PROPERTYCOUNT );
    sal_Int32                                       nRealCount       = 0;
    OUString                                 sBasePath;
    for( FactoryInfo & rInfo : m_lFactories )
    {
        // These path is used to build full qualified property names ....
        // See pInfo->getChangedProperties() for further information
        sBasePath  = PATHSEPARATOR + rInfo.getFactory() + PATHSEPARATOR;

        const css::uno::Sequence< css::beans::PropertyValue > lChangedProperties = rInfo.getChangedProperties ( sBasePath );
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

/*-****************************************************************************************************
    @short      access method to get internal values
    @descr      These methods implement easy access to our internal values.
                You give us right enum value to specify which module interest you ... we return right information.

    @attention  Some people use any value as enum ... but we support in header specified values only!
                We use it directly as index in our internal list. If enum value isn't right - we crash with an
                "index out of range"!!! Please use me right - otherwise there is no guarantee.
    @param      "eModule"  , index in list - specify module
    @return     Queried information.

    @onerror    We return default values. (mostly "not installed"!)
    @threadsafe no
*//*-*****************************************************************************************************/
bool SvtModuleOptions_Impl::IsModuleInstalled( SvtModuleOptions::EModule eModule ) const
{
    switch( eModule )
    {
        case SvtModuleOptions::EModule::WRITER:
            return m_lFactories[SvtModuleOptions::EFactory::WRITER].getInstalled();
        case SvtModuleOptions::EModule::WEB:
            return m_lFactories[SvtModuleOptions::EFactory::WRITERWEB].getInstalled();
        case SvtModuleOptions::EModule::GLOBAL:
            return m_lFactories[SvtModuleOptions::EFactory::WRITERGLOBAL].getInstalled();
        case SvtModuleOptions::EModule::CALC:
            return m_lFactories[SvtModuleOptions::EFactory::CALC].getInstalled();
        case SvtModuleOptions::EModule::DRAW:
            return m_lFactories[SvtModuleOptions::EFactory::DRAW].getInstalled();
        case SvtModuleOptions::EModule::IMPRESS:
            return m_lFactories[SvtModuleOptions::EFactory::IMPRESS].getInstalled();
        case SvtModuleOptions::EModule::MATH:
            return m_lFactories[SvtModuleOptions::EFactory::MATH].getInstalled();
        case SvtModuleOptions::EModule::CHART:
            return m_lFactories[SvtModuleOptions::EFactory::CHART].getInstalled();
        case SvtModuleOptions::EModule::STARTMODULE:
            return m_lFactories[SvtModuleOptions::EFactory::STARTMODULE].getInstalled();
        case SvtModuleOptions::EModule::BASIC:
            return true; // Couldn't be deselected by setup yet!
        case SvtModuleOptions::EModule::DATABASE:
            return m_lFactories[SvtModuleOptions::EFactory::DATABASE].getInstalled();
    }

    return false;
}

css::uno::Sequence < OUString > SvtModuleOptions_Impl::GetAllServiceNames()
{
    std::vector<OUString> aVec;

    for( auto & rFactory : m_lFactories )
        if( rFactory.getInstalled() )
            aVec.push_back( rFactory.getFactory() );

    return comphelper::containerToSequence(aVec);
}

OUString SvtModuleOptions_Impl::GetFactoryName( SvtModuleOptions::EFactory eFactory ) const
{
    return m_lFactories[eFactory].getFactory();
}

OUString SvtModuleOptions::GetFactoryShortName(SvtModuleOptions::EFactory eFactory)
{
    // Attention: Hard configured yet ... because it's not fine to make changes possible by xml file yet.
    //            But it's good to plan further possibilities!

    //return m_lFactories[eFactory].sShortName;

    OUString sShortName;
    switch( eFactory )
    {
        case SvtModuleOptions::EFactory::WRITER   :  sShortName = "swriter";
                                                       break;
        case SvtModuleOptions::EFactory::WRITERWEB:  sShortName = "swriter/web";
                                                       break;
        case SvtModuleOptions::EFactory::WRITERGLOBAL:  sShortName = "swriter/GlobalDocument";
                                                       break;
        case SvtModuleOptions::EFactory::CALC     :  sShortName = "scalc";
                                                       break;
        case SvtModuleOptions::EFactory::DRAW     :  sShortName = "sdraw";
                                                       break;
        case SvtModuleOptions::EFactory::IMPRESS  :  sShortName = "simpress";
                                                       break;
        case SvtModuleOptions::EFactory::MATH     :  sShortName = "smath";
                                                       break;
        case SvtModuleOptions::EFactory::CHART    :  sShortName = "schart";
                                                       break;
        case SvtModuleOptions::EFactory::BASIC    :  sShortName = "sbasic";
                                                       break;
        case SvtModuleOptions::EFactory::DATABASE :  sShortName = "sdatabase";
                                                       break;
        default:
            OSL_FAIL( "unknown factory" );
            break;
    }

    return sShortName;
}

OUString SvtModuleOptions_Impl::GetFactoryStandardTemplate( SvtModuleOptions::EFactory eFactory ) const
{
    return m_lFactories[eFactory].getTemplateFile();
}

OUString SvtModuleOptions_Impl::GetFactoryEmptyDocumentURL( SvtModuleOptions::EFactory eFactory )
{
    // Attention: Hard configured yet ... because it's not fine to make changes possible by xml file yet.
    //            But it's good to plan further possibilities!

    //return m_lFactories[eFactory].getEmptyDocumentURL();

    OUString sURL;
    switch( eFactory )
    {
        case SvtModuleOptions::EFactory::WRITER        :  sURL = "private:factory/swriter";
                                                  break;
        case SvtModuleOptions::EFactory::WRITERWEB     :  sURL = "private:factory/swriter/web";
                                                  break;
        case SvtModuleOptions::EFactory::WRITERGLOBAL  :  sURL = "private:factory/swriter/GlobalDocument";
                                                  break;
        case SvtModuleOptions::EFactory::CALC          :  sURL = "private:factory/scalc";
                                                  break;
        case SvtModuleOptions::EFactory::DRAW          :  sURL = "private:factory/sdraw";
                                                  break;
        case SvtModuleOptions::EFactory::IMPRESS       :  sURL = "private:factory/simpress?slot=6686";
                                                  break;
        case SvtModuleOptions::EFactory::MATH          :  sURL = "private:factory/smath";
                                                  break;
        case SvtModuleOptions::EFactory::CHART         :  sURL = "private:factory/schart";
                                                  break;
        case SvtModuleOptions::EFactory::BASIC         :  sURL = "private:factory/sbasic";
                                                  break;
        case SvtModuleOptions::EFactory::DATABASE     :  sURL = "private:factory/sdatabase?Interactive";
                                                  break;
        default:
            OSL_FAIL( "unknown factory" );
            break;
    }
    return sURL;
}

OUString SvtModuleOptions_Impl::GetFactoryDefaultFilter( SvtModuleOptions::EFactory eFactory ) const
{
    return m_lFactories[eFactory].getDefaultFilter();
}

bool SvtModuleOptions_Impl::IsDefaultFilterReadonly( SvtModuleOptions::EFactory eFactory   ) const
{
    return m_lFactories[eFactory].isDefaultFilterReadonly();
}

sal_Int32 SvtModuleOptions_Impl::GetFactoryIcon( SvtModuleOptions::EFactory eFactory ) const
{
    return m_lFactories[eFactory].getIcon();
}

void SvtModuleOptions_Impl::SetFactoryStandardTemplate(       SvtModuleOptions::EFactory eFactory   ,
                                                        const OUString&           sTemplate  )
{
    m_lFactories[eFactory].setTemplateFile( sTemplate );
    SetModified();
}

void SvtModuleOptions_Impl::SetFactoryDefaultFilter(       SvtModuleOptions::EFactory eFactory,
                                                     const OUString&           sFilter )
{
    m_lFactories[eFactory].setDefaultFilter( sFilter );
    SetModified();
}

/*-************************************************************************************************************
    @short      return list of key names of our configuration management which represent our module tree
    @descr      You give use a list of current existing set node names .. and we expand it for all
                well known properties which are necessary for this implementation.
                These full expanded list should be used to get values of this properties.

    @seealso    ctor
    @return     List of all relative addressed properties of given set entry names.

    @onerror    List will be empty.
    @threadsafe no
*//*-*************************************************************************************************************/
css::uno::Sequence< OUString > SvtModuleOptions_Impl::impl_ExpandSetNames( const css::uno::Sequence< OUString >& lSetNames )
{
    sal_Int32 nCount     = lSetNames.getLength();
    css::uno::Sequence< OUString > lPropNames ( nCount*PROPERTYCOUNT );
    OUString* pPropNames = lPropNames.getArray();
    sal_Int32 nPropStart = 0;

    for( sal_Int32 nName=0; nName<nCount; ++nName )
    {
        pPropNames[nPropStart+PROPERTYHANDLE_SHORTNAME       ] = lSetNames[nName] + PATHSEPARATOR PROPERTYNAME_SHORTNAME;
        pPropNames[nPropStart+PROPERTYHANDLE_TEMPLATEFILE    ] = lSetNames[nName] + PATHSEPARATOR PROPERTYNAME_TEMPLATEFILE;
        pPropNames[nPropStart+PROPERTYHANDLE_WINDOWATTRIBUTES] = lSetNames[nName] + PATHSEPARATOR PROPERTYNAME_WINDOWATTRIBUTES;
        pPropNames[nPropStart+PROPERTYHANDLE_EMPTYDOCUMENTURL] = lSetNames[nName] + PATHSEPARATOR PROPERTYNAME_EMPTYDOCUMENTURL;
        pPropNames[nPropStart+PROPERTYHANDLE_DEFAULTFILTER   ] = lSetNames[nName] + PATHSEPARATOR PROPERTYNAME_DEFAULTFILTER;
        pPropNames[nPropStart+PROPERTYHANDLE_ICON            ] = lSetNames[nName] + PATHSEPARATOR PROPERTYNAME_ICON;
        nPropStart += PROPERTYCOUNT;
    }

    return lPropNames;
}

/*-************************************************************************************************************
    @short      helper to classify given factory by name
    @descr      Every factory has his own long and short name. So we can match right enum value for internal using.

    @attention  We change in/out parameter "eFactory" in every case! But you should use it only, if return value is sal_True!
                Algorithm:  Set out-parameter to probably value ... and check the longname.
                            If it matches with these factory - break operation and return true AND right set parameter.
                            Otherwise try next one and so on. If no factory was found return false. Out parameter eFactory
                            is set to last tried value but shouldn't be used! Because our return value is false!
    @param      "sLongName" , long name of factory, which should be classified
    @return     "eFactory"  , right enum value, which match given long name
                and true for successfully classification, false otherwise

    @onerror    We return false.
    @threadsafe no
*//*-*************************************************************************************************************/
bool SvtModuleOptions_Impl::ClassifyFactoryByName( const OUString& sName, SvtModuleOptions::EFactory& eFactory )
{
    bool bState;

    eFactory = SvtModuleOptions::EFactory::WRITER;
    bState   = ( sName == FACTORYNAME_WRITER );

    if( !bState )
    {
        eFactory = SvtModuleOptions::EFactory::WRITERWEB;
        bState   = ( sName == FACTORYNAME_WRITERWEB );
    }
    // no else!
    if( !bState )
    {
        eFactory = SvtModuleOptions::EFactory::WRITERGLOBAL;
        bState   = ( sName == FACTORYNAME_WRITERGLOBAL );
    }
    // no else!
    if( !bState )
    {
        eFactory = SvtModuleOptions::EFactory::CALC;
        bState   = ( sName == FACTORYNAME_CALC );
    }
    // no else!
    if( !bState )
    {
        eFactory = SvtModuleOptions::EFactory::DRAW;
        bState   = ( sName == FACTORYNAME_DRAW );
    }
    // no else!
    if( !bState )
    {
        eFactory = SvtModuleOptions::EFactory::IMPRESS;
        bState   = ( sName == FACTORYNAME_IMPRESS );
    }
    // no else!
    if( !bState )
    {
        eFactory = SvtModuleOptions::EFactory::MATH;
        bState   = ( sName == FACTORYNAME_MATH );
    }
    // no else!
    if( !bState )
    {
        eFactory = SvtModuleOptions::EFactory::CHART;
        bState   = ( sName == FACTORYNAME_CHART );
    }
    // no else!
    if( !bState )
    {
        eFactory = SvtModuleOptions::EFactory::DATABASE;
        bState   = ( sName == FACTORYNAME_DATABASE );
    }
    // no else!
    if( !bState )
    {
        eFactory = SvtModuleOptions::EFactory::STARTMODULE;
        bState   = ( sName == FACTORYNAME_STARTMODULE);
    }
    // no else!
    if( !bState )
    {
        eFactory = SvtModuleOptions::EFactory::BASIC;
        bState   = ( sName == FACTORYNAME_BASIC);
    }

    return bState;
}

/*-************************************************************************************************************
    @short      read factory configuration
    @descr      Give us a list of pure factory names (long names!) which can be used as
                direct set node names ... and we read her property values and fill internal list.
                These method can be used by initial reading at ctor and later updating by "Notify()".

    @seealso    ctor
    @seealso    method Notify()

    @param      "lFactories" is the list of set node entries which should be readed.
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

    sal_Int32                   nPropertyStart  = 0;
    sal_Int32                   nNodeCount      = lFactories.getLength();
    FactoryInfo*                pInfo           = nullptr;
    SvtModuleOptions::EFactory  eFactory;

    for( sal_Int32 nSetNode=0; nSetNode<nNodeCount; ++nSetNode )
    {
        const OUString& sFactoryName = lFactories[nSetNode];
        if( ClassifyFactoryByName( sFactoryName, eFactory ) )
        {
            OUString sTemp;
            sal_Int32       nTemp = 0;

            pInfo = &(m_lFactories[eFactory]);
            pInfo->free();

            pInfo->initInstalled();
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
        SvtModuleOptions::EFactory  eFactory;

        if (!ClassifyFactoryByName(rFactoryName, eFactory))
            continue;

        FactoryInfo& rInfo = m_lFactories[eFactory];
        rInfo.setDefaultFilterReadonly(lReadonlyStates[i]);
    }

    m_bReadOnlyStatesWellKnown = true;
}

namespace {
    //global
    std::weak_ptr<SvtModuleOptions_Impl> g_pModuleOptions;
}

/*-************************************************************************************************************
    @short      standard constructor and destructor
    @descr      This will initialize an instance with default values. We initialize/deinitialize our static data
                container and create a static mutex, which is used for threadsafe code in further time of this object.

    @seealso    method impl_GetOwnStaticMutex()
    @threadsafe yes
*//*-*************************************************************************************************************/
SvtModuleOptions::SvtModuleOptions()
{
    // Global access, must be guarded (multithreading!)
    ::osl::MutexGuard aGuard( impl_GetOwnStaticMutex() );

    m_pImpl = g_pModuleOptions.lock();
    if( !m_pImpl )
    {
        m_pImpl = std::make_shared<SvtModuleOptions_Impl>();
        g_pModuleOptions = m_pImpl;
        ItemHolder1::holdConfigItem(EItem::ModuleOptions);
    }
}

SvtModuleOptions::~SvtModuleOptions()
{
    // Global access, must be guarded (multithreading!)
    ::osl::MutexGuard aGuard( impl_GetOwnStaticMutex() );

    m_pImpl.reset();
}

/*-************************************************************************************************************
    @short      access to configuration data
    @descr      This methods allow read/write access to configuration values.
                They are threadsafe. All calls are forwarded to impl-data-container. See there for further information!

    @seealso    method impl_GetOwnStaticMutex()
    @threadsafe yes
*//*-*************************************************************************************************************/
bool SvtModuleOptions::IsModuleInstalled( EModule eModule ) const
{
    ::osl::MutexGuard aGuard( impl_GetOwnStaticMutex() );
    return m_pImpl->IsModuleInstalled( eModule );
}

OUString SvtModuleOptions::GetFactoryName( EFactory eFactory ) const
{
    ::osl::MutexGuard aGuard( impl_GetOwnStaticMutex() );
    return m_pImpl->GetFactoryName( eFactory );
}

OUString SvtModuleOptions::GetFactoryStandardTemplate( EFactory eFactory ) const
{
    ::osl::MutexGuard aGuard( impl_GetOwnStaticMutex() );
    return m_pImpl->GetFactoryStandardTemplate( eFactory );
}

OUString SvtModuleOptions::GetFactoryEmptyDocumentURL( EFactory eFactory ) const
{
    ::osl::MutexGuard aGuard( impl_GetOwnStaticMutex() );
    return SvtModuleOptions_Impl::GetFactoryEmptyDocumentURL( eFactory );
}

OUString SvtModuleOptions::GetFactoryDefaultFilter( EFactory eFactory ) const
{
    ::osl::MutexGuard aGuard( impl_GetOwnStaticMutex() );
    return m_pImpl->GetFactoryDefaultFilter( eFactory );
}

bool SvtModuleOptions::IsDefaultFilterReadonly( EFactory eFactory   ) const
{
    ::osl::MutexGuard aGuard( impl_GetOwnStaticMutex() );
    m_pImpl->MakeReadonlyStatesAvailable();
    return m_pImpl->IsDefaultFilterReadonly( eFactory );
}

sal_Int32 SvtModuleOptions::GetFactoryIcon( EFactory eFactory ) const
{
    ::osl::MutexGuard aGuard( impl_GetOwnStaticMutex() );
    return m_pImpl->GetFactoryIcon( eFactory );
}

bool SvtModuleOptions::ClassifyFactoryByName( const OUString& sName    ,
                                                        EFactory&        eFactory )
{
    // We don't need any mutex here ... because we don't use any member here!
    return SvtModuleOptions_Impl::ClassifyFactoryByName( sName, eFactory );
}

void SvtModuleOptions::SetFactoryStandardTemplate(       EFactory         eFactory   ,
                                                   const OUString& sTemplate  )
{
    ::osl::MutexGuard aGuard( impl_GetOwnStaticMutex() );
    m_pImpl->SetFactoryStandardTemplate( eFactory, sTemplate );
}

void SvtModuleOptions::SetFactoryDefaultFilter(       EFactory         eFactory,
                                                const OUString& sFilter )
{
    ::osl::MutexGuard aGuard( impl_GetOwnStaticMutex() );
    m_pImpl->SetFactoryDefaultFilter( eFactory, sFilter );
}

bool SvtModuleOptions::IsMath() const
{
    ::osl::MutexGuard aGuard( impl_GetOwnStaticMutex() );
    return m_pImpl->IsModuleInstalled( EModule::MATH );
}

bool SvtModuleOptions::IsChart() const
{
    ::osl::MutexGuard aGuard( impl_GetOwnStaticMutex() );
    return m_pImpl->IsModuleInstalled( EModule::CHART );
}

bool SvtModuleOptions::IsCalc() const
{
    ::osl::MutexGuard aGuard( impl_GetOwnStaticMutex() );
    return m_pImpl->IsModuleInstalled( EModule::CALC );
}

bool SvtModuleOptions::IsDraw() const
{
    ::osl::MutexGuard aGuard( impl_GetOwnStaticMutex() );
    return m_pImpl->IsModuleInstalled( EModule::DRAW );
}

bool SvtModuleOptions::IsWriter() const
{
    ::osl::MutexGuard aGuard( impl_GetOwnStaticMutex() );
    return m_pImpl->IsModuleInstalled( EModule::WRITER );
}

bool SvtModuleOptions::IsImpress() const
{
    ::osl::MutexGuard aGuard( impl_GetOwnStaticMutex() );
    return m_pImpl->IsModuleInstalled( EModule::IMPRESS );
}

bool SvtModuleOptions::IsDataBase() const
{
    ::osl::MutexGuard aGuard( impl_GetOwnStaticMutex() );
    return m_pImpl->IsModuleInstalled( EModule::DATABASE );
}

namespace
{
    class theModuleOptionsMutex : public rtl::Static<osl::Mutex, theModuleOptionsMutex> {};
}
/*-****************************************************************************************************
    @short      return a reference to a static mutex
    @descr      These class is threadsafe.
                We create a static mutex only for one time and use it to protect our refcount and container
                member!
    @return     A reference to a static mutex member.
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
        case SvtModuleOptions::EModule::WRITER    :   { return OUString("Writer"); }
        case SvtModuleOptions::EModule::WEB       :   { return OUString("Web"); }
        case SvtModuleOptions::EModule::GLOBAL    :   { return OUString("Global"); }
        case SvtModuleOptions::EModule::CALC      :   { return OUString("Calc"); }
        case SvtModuleOptions::EModule::DRAW      :   { return OUString("Draw"); }
        case SvtModuleOptions::EModule::IMPRESS   :   { return OUString("Impress"); }
        case SvtModuleOptions::EModule::MATH      :   { return OUString("Math"); }
        case SvtModuleOptions::EModule::CHART     :   { return OUString("Chart"); }
        case SvtModuleOptions::EModule::BASIC     :   { return OUString("Basic"); }
        case SvtModuleOptions::EModule::DATABASE  :   { return OUString("Database"); }
        default:
            OSL_FAIL( "unknown module" );
            break;
    }

    return OUString();
}

SvtModuleOptions::EFactory SvtModuleOptions::ClassifyFactoryByShortName(const OUString& sName)
{
    if ( sName == "swriter" )
        return EFactory::WRITER;
    if (sName.equalsIgnoreAsciiCase("swriter/Web")) // sometimes they are registered for swriter/web :-(
        return EFactory::WRITERWEB;
    if (sName.equalsIgnoreAsciiCase("swriter/GlobalDocument")) // sometimes they are registered for swriter/globaldocument :-(
        return EFactory::WRITERGLOBAL;
    if ( sName == "scalc" )
        return EFactory::CALC;
    if ( sName == "sdraw" )
        return EFactory::DRAW;
    if ( sName == "simpress" )
        return EFactory::IMPRESS;
    if ( sName == "schart" )
        return EFactory::CHART;
    if ( sName == "smath" )
        return EFactory::MATH;
    if ( sName == "sbasic" )
        return EFactory::BASIC;
    if ( sName == "sdatabase" )
        return EFactory::DATABASE;

    return EFactory::UNKNOWN_FACTORY;
}

SvtModuleOptions::EFactory SvtModuleOptions::ClassifyFactoryByServiceName(const OUString& sName)
{
    if (sName == FACTORYNAME_WRITERGLOBAL)
        return EFactory::WRITERGLOBAL;
    if (sName == FACTORYNAME_WRITERWEB)
        return EFactory::WRITERWEB;
    if (sName == FACTORYNAME_WRITER)
        return EFactory::WRITER;
    if (sName == FACTORYNAME_CALC)
        return EFactory::CALC;
    if (sName == FACTORYNAME_DRAW)
        return EFactory::DRAW;
    if (sName == FACTORYNAME_IMPRESS)
        return EFactory::IMPRESS;
    if (sName == FACTORYNAME_MATH)
        return EFactory::MATH;
    if (sName == FACTORYNAME_CHART)
        return EFactory::CHART;
    if (sName == FACTORYNAME_DATABASE)
        return EFactory::DATABASE;
    if (sName == FACTORYNAME_STARTMODULE)
        return EFactory::STARTMODULE;
    if (sName == FACTORYNAME_BASIC)
        return EFactory::BASIC;

    return EFactory::UNKNOWN_FACTORY;
}

SvtModuleOptions::EFactory SvtModuleOptions::ClassifyFactoryByURL(const OUString&                                 sURL            ,
                                                                  const css::uno::Sequence< css::beans::PropertyValue >& lMediaDescriptor)
{
    css::uno::Reference< css::uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();

    css::uno::Reference< css::container::XNameAccess > xFilterCfg;
    css::uno::Reference< css::container::XNameAccess > xTypeCfg;
    try
    {
        xFilterCfg.set(
            xContext->getServiceManager()->createInstanceWithContext("com.sun.star.document.FilterFactory", xContext), css::uno::UNO_QUERY);
        xTypeCfg.set(
            xContext->getServiceManager()->createInstanceWithContext("com.sun.star.document.TypeDetection", xContext), css::uno::UNO_QUERY);
    }
    catch(const css::uno::RuntimeException&)
        { throw; }
    catch(const css::uno::Exception&)
        { return EFactory::UNKNOWN_FACTORY; }

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

            if (eApp != EFactory::UNKNOWN_FACTORY)
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
        return EFactory::UNKNOWN_FACTORY;

    // yes - there is a type info
    // Try to find the preferred filter.
    try
    {
        ::comphelper::SequenceAsHashMap stlTypeProps     (xTypeCfg->getByName(sTypeName));
        OUString                 sPreferredFilter = stlTypeProps.getUnpackedValueOrDefault("PreferredFilter", OUString());
        ::comphelper::SequenceAsHashMap stlFilterProps   (xFilterCfg->getByName(sPreferredFilter));
        OUString                 sDocumentService = stlFilterProps.getUnpackedValueOrDefault("DocumentService", OUString());
        SvtModuleOptions::EFactory      eApp             = SvtModuleOptions::ClassifyFactoryByServiceName(sDocumentService);

        if (eApp != EFactory::UNKNOWN_FACTORY)
            return eApp;
    }
    catch(const css::uno::RuntimeException&)
        { throw; }
    catch(const css::uno::Exception&)
        { /* do nothing here ... may the following code can help!*/ }

    // no filter/no type/no detection result => no fun :-)
    return EFactory::UNKNOWN_FACTORY;
}

SvtModuleOptions::EFactory SvtModuleOptions::ClassifyFactoryByModel(const css::uno::Reference< css::frame::XModel >& xModel)
{
    css::uno::Reference< css::lang::XServiceInfo > xInfo(xModel, css::uno::UNO_QUERY);
    if (!xInfo.is())
        return EFactory::UNKNOWN_FACTORY;

    const css::uno::Sequence< OUString > lServices = xInfo->getSupportedServiceNames();
    const OUString*                      pServices = lServices.getConstArray();

    for (sal_Int32 i=0; i<lServices.getLength(); ++i)
    {
        SvtModuleOptions::EFactory eApp = SvtModuleOptions::ClassifyFactoryByServiceName(pServices[i]);
        if (eApp != EFactory::UNKNOWN_FACTORY)
            return eApp;
    }

    return EFactory::UNKNOWN_FACTORY;
}

css::uno::Sequence < OUString > SvtModuleOptions::GetAllServiceNames()
{
    ::osl::MutexGuard aGuard( impl_GetOwnStaticMutex() );
    return m_pImpl->GetAllServiceNames();
}

OUString SvtModuleOptions::GetDefaultModuleName()
{
    OUString aModule;
    if (m_pImpl->IsModuleInstalled(SvtModuleOptions::EModule::WRITER))
        aModule = GetFactoryShortName(SvtModuleOptions::EFactory::WRITER);
    else if (m_pImpl->IsModuleInstalled(SvtModuleOptions::EModule::CALC))
        aModule = GetFactoryShortName(SvtModuleOptions::EFactory::CALC);
    else if (m_pImpl->IsModuleInstalled(SvtModuleOptions::EModule::IMPRESS))
        aModule = GetFactoryShortName(SvtModuleOptions::EFactory::IMPRESS);
    else if (m_pImpl->IsModuleInstalled(SvtModuleOptions::EModule::DATABASE))
        aModule = GetFactoryShortName(SvtModuleOptions::EFactory::DATABASE);
    else if (m_pImpl->IsModuleInstalled(SvtModuleOptions::EModule::DRAW))
        aModule = GetFactoryShortName(SvtModuleOptions::EFactory::DRAW);
    else if (m_pImpl->IsModuleInstalled(SvtModuleOptions::EModule::WEB))
        aModule = GetFactoryShortName(SvtModuleOptions::EFactory::WRITERWEB);
    else if (m_pImpl->IsModuleInstalled(SvtModuleOptions::EModule::GLOBAL))
        aModule = GetFactoryShortName(SvtModuleOptions::EFactory::WRITERGLOBAL);
    else if (m_pImpl->IsModuleInstalled(SvtModuleOptions::EModule::MATH))
        aModule = GetFactoryShortName(SvtModuleOptions::EFactory::MATH);
    return aModule;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
