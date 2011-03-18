/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#define VCL_NEED_BASETSD
#include <tools/presys.h>
#include <windows.h>
#include <tools/postsys.h>
#include <classes/servicemanager.hxx>
#include <classes/filtercache.hxx>
#include <macros/generic.hxx>
#include <macros/debug.hxx>
#include <services.h>
#include <filterflags.h>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________
#include <comphelper/processfactory.hxx>
#include <unotools/processfactory.hxx>
#include <osl/process.h>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>

#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/msgbox.hxx>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

using namespace ::framework ;

//_________________________________________________________________________________________________________________
//  const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  defines
//_________________________________________________________________________________________________________________

/*
    Versions:       1)  first revision
                        - one entry for every property
                        - full loclized values
                    2)  new property "Order" for filters ... but not right set!
                        all values are 0
                    3)  decrease size of xml file
                        - don't write full localized values
                        - use own formated string for all non localized values
                        - seperate "Installed" flag for filters
                    4)  set right values for "Order" property of filters
                    5)  support for ContentHandler
              draft 6)  reactivate old filter names
          ??? draft 7)  split xml into standard/optional => use DRAFT_SPLIT_VERSION till this version is well known!
 */
#define DRAFT_SPLIT_VERSION             7

#define ARGUMENT_PACKAGE_STANDARD       DECLARE_ASCII("-pas=")          // argument for package name of standard filters
#define ARGUMENT_PACKAGE_ADDITIONAL     DECLARE_ASCII("-paa=")          // argument for package name of additional filters
#define ARGUMENT_WRITEABLE              DECLARE_ASCII("-wri=")          // argument for "writeable"                         [true|false]
#define ARGUMENT_VERSION_INPUT          DECLARE_ASCII("-vin=")          // argument for file version to read                [1|2|3]
#define ARGUMENT_VERSION_OUTPUT         DECLARE_ASCII("-vou=")          // argument for file version to write               [1|2|3]

#define ARGUMENTLENGTH                  5                               // All arguments should have the same lenght ... it's better to detect it!
#define ARGUMENTFOUND                   0                               // OUString::compareTo returns 0 if searched string match given one

#define WRITEABLE_ON                    DECLARE_ASCII("true" )
#define WRITEABLE_OFF                   DECLARE_ASCII("false")

#define MINARGUMENTCOUNT                5                               // no optional arguments allowed yet!

#define LISTFILE_STANDARDTYPES          "typelist_standard.txt"
#define LISTFILE_ADDITIONALTYPES        "typelist_additional.txt"
#define LISTFILE_STANDARDFILTER         "filterlist_standard.txt"
#define LISTFILE_ADDITIONALFILTER       "filterlist_additional.txt"
#define SCPFILE_STANDARD                "scp_standard.txt"
#define SCPFILE_ADDITIONAL              "scp_additional.txt"

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

/*-***************************************************************************************************************/
struct AppMember
{
        FilterCache*                pFilterCache                ; // pointer to configuration
        StringHash                  aOldFilterNamesHash         ; // converter tabel to restaurate old filter names
        EFilterPackage              ePackage                    ; // specify which package should be used => specify using of file name and buffer too!
//        ::rtl::OUString             sFileNameStandard           ; // file name of our standard filter cfg
//        ::rtl::OUString             sFileNameAdditional         ; // file name of our additional filter cfg
        ::rtl::OUString             sPackageStandard            ; // package name of our standard filter cfg
        ::rtl::OUString             sPackageAdditional          ; // package name of our additional filter cfg
        ::rtl::OUStringBuffer       sBufferStandard             ; // buffer of our standard filter cfg
        ::rtl::OUStringBuffer       sBufferAdditional           ; // buffer of our standard filter cfg
        ::rtl::OUStringBuffer       sNew2OldSCPStandard         ; // setup script to convert new to old filternames (standard filter)
        ::rtl::OUStringBuffer       sNew2OldSCPAdditional       ; // setup script to convert new to old filternames (additional filter)
        ::rtl::OUStringBuffer       sStandardFilterList         ;
        ::rtl::OUStringBuffer       sAdditionalFilterList       ;
        ::rtl::OUStringBuffer       sStandardTypeList           ;
        ::rtl::OUStringBuffer       sAdditionalTypeList         ;
        sal_Bool                    bWriteable                  ; // enable/disable writable configuration items
        sal_Int32                   nVersionInput               ; // format version of input xml file
        sal_Int32                   nVersionOutput              ; // format version of output xcd file

        sal_Int32                   nOriginalTypes              ;
        sal_Int32                   nOriginalFilters            ;
        sal_Int32                   nOriginalDetectors          ;
        sal_Int32                   nOriginalLoaders            ;
        sal_Int32                   nOriginalContentHandlers    ;

        sal_Int32                   nWrittenTypes               ;
        sal_Int32                   nWrittenFilters             ;
        sal_Int32                   nWrittenDetectors           ;
        sal_Int32                   nWrittenLoaders             ;
        sal_Int32                   nWrittenContentHandlers     ;
};

/*-***************************************************************************************************************/
class XCDGenerator : public Application
{
    //*************************************************************************************************************
    public:
        void Main();

    //*************************************************************************************************************
    private:
        void                    impl_printCopyright                     (                                                                                       ); // print copyright to stdout :-)
        void                    impl_printSyntax                        (                                                                                       ); // print help to stout for user
        void                    impl_parseCommandLine                   (           AppMember&                              rMember                             ); // parse command line arguments and fill given struct

        void                    impl_generateXCD                        (                                                                                       ); // generate all xcd files by using current configuration
        void                    impl_generateCopyright                  (                                                                                       ); // generate copyrights
        void                    impl_generateTypeTemplate               (                                                                                       ); // generate templates ...
        void                    impl_generateFilterTemplate             (                                                                                       );
        void                    impl_generateDetectorTemplate           (                                                                                       );
        void                    impl_generateLoaderTemplate             (                                                                                       );
        void                    impl_generateTypeSet                    (                                                                                       ); // generate sets
        void                    impl_generateFilterSet                  (                                                                                       );
        void                    impl_generateDetectorSet                (                                                                                       );
        void                    impl_generateLoaderSet                  (                                                                                       );
        void                    impl_generateDefaults                   (                                                                                       ); // generate defaults
        void                    impl_generateContentHandlerTemplate     (                                                                                       );
        void                    impl_generateContentHandlerSet          (                                                                                       );
        void                    impl_generateFilterFlagTemplate         (   const   ::rtl::OUString&                        sName                               ,  // helper to write atomic elements
                                                                                    sal_Int32                               nValue                              ,
                                                                            const   ::rtl::OString&                         sDescription = ::rtl::OString()     );
        void                    impl_generateIntProperty                (           ::rtl::OUStringBuffer&                  sXCD                                ,
                                                                            const   ::rtl::OUString&                        sName                               ,
                                                                                    sal_Int32                               nValue                              );
        void                    impl_generateBoolProperty               (           ::rtl::OUStringBuffer&                  sXCD                                ,
                                                                            const   ::rtl::OUString&                        sName                               ,
                                                                                    sal_Bool                                bValue                              );
        void                    impl_generateStringProperty             (           ::rtl::OUStringBuffer&                  sXCD                                ,
                                                                            const   ::rtl::OUString&                        sName                               ,
                                                                            const   ::rtl::OUString&                        sValue                              );
        void                    impl_generateStringListProperty         (           ::rtl::OUStringBuffer&                  sXCD                                ,
                                                                            const   ::rtl::OUString&                        sName                               ,
                                                                            const   ::framework::StringList&                lValue                              );
        void                    impl_generateUINamesProperty            (           ::rtl::OUStringBuffer&                  sXCD                                ,
                                                                            const   ::rtl::OUString&                        sName                               ,
                                                                            const   StringHash&                             lUINames                            );
        ::rtl::OUString         impl_getOldFilterName                   (   const   ::rtl::OUString&                        sNewName                            ); // convert filter names to old format

        static void             impl_classifyType                       (   const   AppMember&                              rData                               ,
                                                                            const   ::rtl::OUString&                        sTypeName                           ,
                                                                                    EFilterPackage&                         ePackage                            ); // classify type as STANDARD or ADDITIONAL one
        static void             impl_classifyFilter                     (   const   AppMember&                              rData                               ,
                                                                            const   ::rtl::OUString&                        sFilterName                         ,
                                                                                    EFilterPackage&                         ePackage                            ,
                                                                                    sal_Int32&                              nOrder                              ); // classify filter as STANDARD or ADDITIONAL filter, set order of standard filter too
        static ::rtl::OUString  impl_encodeSpecialSigns                 (   const   ::rtl::OUString&                        sValue                              ); // encode strings for xml
        static sal_Unicode      impl_defineSeperator                    (   const   ::framework::StringList&                lList                               ); // search seperator for lists
        static void             impl_initFilterHashNew2Old              (           StringHash&                             aHash                               ); // initialize converter table to restaurate old filter names
        static void             impl_orderAlphabetical                  (           css::uno::Sequence< ::rtl::OUString >&  lList                               ); // sort stringlist of internal type-, filter- ... names in alphabetical order to generate xcd files everytime in the same way
        static sal_Bool         impl_isUsAsciiAlphaDigit                (           sal_Unicode                             c                                   ,
                                                                                    sal_Bool                                bDigitAllowed = sal_True            );
        static ::rtl::OUString  impl_encodeSetName                      (   const   ::rtl::OUString&                        rSource                             );

    //*************************************************************************************************************
    private:
        AppMember       m_aData;

};  //  class XCDGenerator

//_________________________________________________________________________________________________________________
//  global variables
//_________________________________________________________________________________________________________________

XCDGenerator gGenerator;

//*****************************************************************************************************************
void XCDGenerator::Main()
{
    // Must be :-)
//    impl_printCopyright();

    // Init global servicemanager and set it.
    // It's neccessary for other services ... e.g. configuration.
    ServiceManager aManager;
    ::comphelper::setProcessServiceFactory( aManager.getGlobalUNOServiceManager() );
    ::utl::setProcessServiceFactory      ( aManager.getGlobalUNOServiceManager() );

    // Get optional commands from command line.
    impl_parseCommandLine( m_aData );

    // initialize converter table to match new to old filter names!
    if( m_aData.nVersionOutput == 6 && m_aData.nVersionInput < 6 )
    {
        XCDGenerator::impl_initFilterHashNew2Old( m_aData.aOldFilterNamesHash );
    }

    // Create access to current set filter configuration.
    // Attention: Please use it for a full fat office installation only!!
    //            We need an installation with ALL filters.
    // Member m_pData is used in some impl-methods directly ...
    m_aData.pFilterCache = new FilterCache( m_aData.nVersionInput, CONFIG_MODE_ALL_LOCALES );

    // Get some statistic informations of current filled filter cache ... (e.g. count of current activae filters)
    // because we need it to check if all filters are converted and written to disk.
    // May be it's possible to lose some of them during convertion!!!
    m_aData.nOriginalTypes     = m_aData.pFilterCache->getAllTypeNames().getLength()     ;
    m_aData.nOriginalFilters   = m_aData.pFilterCache->getAllFilterNames().getLength()   ;
    m_aData.nOriginalDetectors = m_aData.pFilterCache->getAllDetectorNames().getLength() ;
    m_aData.nOriginalLoaders   = m_aData.pFilterCache->getAllLoaderNames().getLength()   ;
    if( m_aData.nVersionInput >= 5 )
    {
        m_aData.nOriginalContentHandlers  = m_aData.pFilterCache->getAllContentHandlerNames().getLength()  ;
    }

    // Start generation of xcd file(s).
    impl_generateXCD();

    // Warn programmer if some items couldn't written to file!
    LOG_ASSERT2( m_aData.nOriginalTypes    != m_aData.nWrittenTypes    , "XCDGenerator::Main()", "Generated xcd file could be invalid ... because I miss some types!"     )
    LOG_ASSERT2( m_aData.nOriginalFilters  != m_aData.nWrittenFilters  , "XCDGenerator::Main()", "Generated xcd file could be invalid ... because I miss some filters!"   )
    LOG_ASSERT2( m_aData.nOriginalDetectors!= m_aData.nWrittenDetectors, "XCDGenerator::Main()", "Generated xcd file could be invalid ... because I miss some detectors!" )
    LOG_ASSERT2( m_aData.nOriginalLoaders  != m_aData.nWrittenLoaders  , "XCDGenerator::Main()", "Generated xcd file could be invalid ... because I miss some loaders!"   )

    // Free memory.
    delete m_aData.pFilterCache;
    m_aData.pFilterCache = NULL;
}

/*-************************************************************************************************************//**
    @short      print some info messages to stderr
    @descr      We must show an copyright or help for using this file.
                This two methods do that.

    @seealso    -

    @param      -
    @return     -

    @onerror    -
*//*-*************************************************************************************************************/
void XCDGenerator::impl_printCopyright()
{
    fprintf( stderr, "\n(c) Copyright 2000, 2010 Oracle and/or its affiliates.\n" );
}

//*****************************************************************************************************************
void XCDGenerator::impl_printSyntax()
{
    // It's not possible to print it out to stdout in a svdem binary :-(
    // So we show an assert.
    ::rtl::OStringBuffer sBuffer( 500 );
    sBuffer.append( "\nusing: xml2xcd -fis=<file standard filter> -fia=<file additional filter> -pas=<package standard filter> -paa=<package additional filter> -vin=<version input> -vou=<version output> [-wri=<true|false>]\n\n" );
    sBuffer.append( "\tneccessary parameters:\n"                                                                                                                                                                                    );
    sBuffer.append( "\t\t-fis\tname of output file in system notation\n"                                                                                                                                                            );
    sBuffer.append( "\t\t-fia\tname of output file in system notation\n"                                                                                                                                                            );
    sBuffer.append( "\t\t-pas\tpackage of standard filters\n"                                                                                                                                                                       );
    sBuffer.append( "\t\t-paa\tpackage of additional filters\n"                                                                                                                                                                     );
    sBuffer.append( "\t\t-vin\tformat version of input xml file\n"                                                                                                                                                                  );
    sBuffer.append( "\t\t-vou\tformat version of generated xcd file\n\n"                                                                                                                                                            );
    sBuffer.append( "\toptional parameters:\n"                                                                                                                                                                                      );
    sBuffer.append( "\t\t-wri\tconfig items should be writeable ... [true|false]\n"                                                                                                                                                 );
    LOG_ERROR( "", sBuffer.makeStringAndClear() )
}

/*-************************************************************************************************************//**
    @short      analyze command line arguments
    @descr      Created binary accept different command line arguments. These parameters
                regulate creation of xcd file. Follow arguments are supported:
                    "-fis=<filename of standard xcd>"
                    "-fia=<filename of additional xcd>"
                    "-wri=<writeable>[true|false]"
                    "-vin=<version of input file>[1|2|3]"
                    "-vou=<version of output file>[1|2|3]"

    @seealso    -

    @param      "rMember", reference to struct of global application member to fill arguments in it
    @return     right filled member struct or unchanged struct if an error occure!

    @onerror    We do nothing - or warn programmer!
*//*-*************************************************************************************************************/
void XCDGenerator::impl_parseCommandLine( AppMember& rMember )
{
    ::rtl::OUString     sArgument                               ;
    sal_Int32           nArgument   = 0                         ;
    sal_Int32           nCount      = osl_getCommandArgCount();
    sal_Int32           nMinCount   = 0                         ;

    while( nArgument<nCount )
    {
        osl_getCommandArg( nArgument, &sArgument.pData );
/*OBSOLETE
        //_____________________________________________________________________________________________________
        // look for "-fis=..."
        if( sArgument.compareTo( ARGUMENT_FILENAME_STANDARD, ARGUMENTLENGTH ) == ARGUMENTFOUND )
        {
            rMember.sFileNameStandard = sArgument.copy( ARGUMENTLENGTH, sArgument.getLength()-ARGUMENTLENGTH );
            ++nMinCount;
        }
        else
        //_____________________________________________________________________________________________________
        // look for "-fia=..."
        if( sArgument.compareTo( ARGUMENT_FILENAME_ADDITIONAL, ARGUMENTLENGTH ) == ARGUMENTFOUND )
        {
            rMember.sFileNameAdditional = sArgument.copy( ARGUMENTLENGTH, sArgument.getLength()-ARGUMENTLENGTH );
            ++nMinCount;
        }
        else
*/
        //_____________________________________________________________________________________________________
        // look for "-pas=..."
        if( sArgument.compareTo( ARGUMENT_PACKAGE_STANDARD, ARGUMENTLENGTH ) == ARGUMENTFOUND )
        {
            rMember.sPackageStandard = sArgument.copy( ARGUMENTLENGTH, sArgument.getLength()-ARGUMENTLENGTH );
            ++nMinCount;
        }
        else
        //_____________________________________________________________________________________________________
        // look for "-paa=..."
        if( sArgument.compareTo( ARGUMENT_PACKAGE_ADDITIONAL, ARGUMENTLENGTH ) == ARGUMENTFOUND )
        {
            rMember.sPackageAdditional = sArgument.copy( ARGUMENTLENGTH, sArgument.getLength()-ARGUMENTLENGTH );
            ++nMinCount;
        }
        else
        //_____________________________________________________________________________________________________
        // look for "-wri=..."
        if( sArgument.compareTo( ARGUMENT_WRITEABLE, ARGUMENTLENGTH ) == ARGUMENTFOUND )
        {
            ::rtl::OUString sWriteable = sArgument.copy( ARGUMENTLENGTH, sArgument.getLength()-ARGUMENTLENGTH );
            if( sWriteable == WRITEABLE_ON )
            {
                rMember.bWriteable = sal_True;
            }
            else
            {
                rMember.bWriteable = sal_False;
            }
            ++nMinCount;
        }
        //_____________________________________________________________________________________________________
        // look for "-vin=..."
        if( sArgument.compareTo( ARGUMENT_VERSION_INPUT, ARGUMENTLENGTH ) == ARGUMENTFOUND )
        {
            ::rtl::OUString sVersion = sArgument.copy( ARGUMENTLENGTH, sArgument.getLength()-ARGUMENTLENGTH );
            rMember.nVersionInput = sVersion.toInt32();
            ++nMinCount;
        }
        //_____________________________________________________________________________________________________
        // look for "-vou=..."
        if( sArgument.compareTo( ARGUMENT_VERSION_OUTPUT, ARGUMENTLENGTH ) == ARGUMENTFOUND )
        {
            ::rtl::OUString sVersion = sArgument.copy( ARGUMENTLENGTH, sArgument.getLength()-ARGUMENTLENGTH );
            rMember.nVersionOutput = sVersion.toInt32();
            ++nMinCount;
        }

        ++nArgument;
    }

    // Show help if user don't call us right!
    if( nMinCount != MINARGUMENTCOUNT )
    {
        impl_printSyntax();
        exit(-1);
    }
}

/*-************************************************************************************************************//**
    @short      regulate generation of complete xcd file(s)
    @descr      This method is the toppest one and implement the global structure of generated xcd file(s).
                We create a unicode string buffer for complete xcd file in memory ...
                use different helper methods to fill it ...
                and write it to disk at the end of this method!

    @seealso    struct AppMember

    @param      -
    @return     -

    @onerror    -
*//*-*************************************************************************************************************/
void XCDGenerator::impl_generateXCD()
{
    impl_generateCopyright();

    // Write header
    m_aData.sBufferStandard.appendAscii     ( "\n<!-- PLEASE DON'T CHANGE TEMPLATES OR FILE FORMAT BY HAND! USE \"XML2XCD.EXE\" TO DO THAT. THANKS. -->\n\n"                                                                                                                                                               );
    m_aData.sBufferStandard.appendAscii     ( "<!DOCTYPE schema:component SYSTEM \"../../../../schema/schema.description.dtd\">\n"                                                                                                                                                                                         );
    m_aData.sBufferStandard.appendAscii     ( "<schema:component cfg:name=\""                                                                                                                                                                                                                                              );
    m_aData.sBufferStandard.append          ( m_aData.sPackageStandard                                                                                                                                                                                                                                                     );
    m_aData.sBufferStandard.appendAscii     ( "\" cfg:package=\"org.openoffice.Office\" xml:lang=\"en-US\" xmlns:schema=\"http://openoffice.org/2000/registry/schema/description\" xmlns:default=\"http://openoffice.org/2000/registry/schema/default\" xmlns:cfg=\"http://openoffice.org/2000/registry/instance\">\n"     );
    m_aData.sBufferStandard.appendAscii     ( "\t<schema:templates>\n"                                                                                                                                                                                                                                                     );

    if( m_aData.nVersionOutput >= DRAFT_SPLIT_VERSION )
    {
        m_aData.sBufferAdditional.appendAscii   ( "\n<!-- PLEASE DON'T CHANGE TEMPLATES OR FILE FORMAT BY HAND! USE \"XML2XCD.EXE\" TO DO THAT. THANKS. -->\n\n"                                                                                                                                                           );
        m_aData.sBufferAdditional.appendAscii   ( "<!DOCTYPE schema:component SYSTEM \"../../../../schema/schema.description.dtd\">\n"                                                                                                                                                                                     );
        m_aData.sBufferAdditional.appendAscii   ( "<schema:component cfg:name=\""                                                                                                                                                                                                                                          );
        m_aData.sBufferAdditional.append        ( m_aData.sPackageAdditional                                                                                                                                                                                                                                               );
        m_aData.sBufferAdditional.appendAscii   ( "\" cfg:package=\"org.openoffice.Office\" xml:lang=\"en-US\" xmlns:schema=\"http://openoffice.org/2000/registry/schema/description\" xmlns:default=\"http://openoffice.org/2000/registry/schema/default\" xmlns:cfg=\"http://openoffice.org/2000/registry/instance\">\n" );
        m_aData.sBufferAdditional.appendAscii   ( "\t<schema:import cfg:name=\""                                                                                                                                                                                                                                             );
        m_aData.sBufferAdditional.append        ( m_aData.sPackageStandard                                                                                                                                                                                                                                                 );
        m_aData.sBufferAdditional.appendAscii   ( "\"/>\n"                                                                                                                                                                                                                                                                 );
    }

    // Follow ...generate... methods to nothing for additional filters!
    impl_generateTypeTemplate    ();
    impl_generateFilterTemplate  ();
    impl_generateDetectorTemplate();
    if( m_aData.nVersionOutput >= 5 )
    {
        impl_generateContentHandlerTemplate ();
    }
    impl_generateLoaderTemplate  ();

    m_aData.sBufferStandard.appendAscii     ( "\t</schema:templates>\n"                     );
    m_aData.sBufferStandard.appendAscii     ( "<schema:schema cfg:localized=\"false\">\n"   );

    if( m_aData.nVersionOutput >= DRAFT_SPLIT_VERSION )
    {
        m_aData.sBufferAdditional.appendAscii( "\t<schema:schema cfg:localized=\"false\">\n"   );
    }

    impl_generateTypeSet         ();
    impl_generateFilterSet       ();
    impl_generateDetectorSet     ();
    if( m_aData.nVersionInput >= 5 )
    {
        impl_generateContentHandlerSet  ();
    }
    impl_generateLoaderSet       ();
    impl_generateDefaults        ();

    m_aData.sBufferStandard.appendAscii     ( "\t</schema:schema>\n"  );
    m_aData.sBufferStandard.appendAscii     ( "</schema:component>\n" );

    if( m_aData.nVersionOutput >= DRAFT_SPLIT_VERSION )
    {
        m_aData.sBufferAdditional.appendAscii ( "\t</schema:schema>\n"  );
        m_aData.sBufferAdditional.appendAscii ( "</schema:component>\n" );
    }

    ::rtl::OUString sFileName  = m_aData.sPackageStandard   ;
                    sFileName += DECLARE_ASCII(".xcd")      ;

    WRITE_LOGFILE( U2B( sFileName )         , U2B(m_aData.sBufferStandard.makeStringAndClear()       ))
    WRITE_LOGFILE( LISTFILE_STANDARDFILTER  , U2B(m_aData.sStandardFilterList.makeStringAndClear()   ))
    WRITE_LOGFILE( LISTFILE_STANDARDTYPES   , U2B(m_aData.sStandardTypeList.makeStringAndClear()     ))
    WRITE_LOGFILE( SCPFILE_STANDARD         , U2B(m_aData.sNew2OldSCPStandard.makeStringAndClear()   ))

    if( m_aData.nVersionOutput >= DRAFT_SPLIT_VERSION )
    {
        sFileName  = m_aData.sPackageAdditional ;
        sFileName += DECLARE_ASCII(".xcd")      ;

        WRITE_LOGFILE( U2B(sFileName)           , U2B(m_aData.sBufferAdditional.makeStringAndClear()     ))
        WRITE_LOGFILE( LISTFILE_ADDITIONALFILTER, U2B(m_aData.sAdditionalFilterList.makeStringAndClear() ))
        WRITE_LOGFILE( LISTFILE_ADDITIONALTYPES , U2B(m_aData.sAdditionalTypeList.makeStringAndClear()   ))
        WRITE_LOGFILE( SCPFILE_ADDITIONAL       , U2B(m_aData.sNew2OldSCPAdditional.makeStringAndClear() ))
    }
}

//*****************************************************************************************************************
void XCDGenerator::impl_generateCopyright()
{
    m_aData.sBufferStandard.appendAscii( "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"                                );
    m_aData.sBufferStandard.appendAscii( "<!-- \n"                                                                     );
    m_aData.sBufferStandard.appendAscii( "  DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.\n"           );
    m_aData.sBufferStandard.appendAscii( "\n"                                                                          );
    m_aData.sBufferStandard.appendAscii( "   Copyright 2000, 2010 Oracle and/or its affiliates.\n"                               );
    m_aData.sBufferStandard.appendAscii( "\n"                                                                          );
    m_aData.sBufferStandard.appendAscii( "   OpenOffice.org - a multi-platform office productivity suite\n"            );
    m_aData.sBufferStandard.appendAscii( "\n"                                                                          );
    m_aData.sBufferStandard.appendAscii( "   This file is part of OpenOffice.org.\n"                                   );
    m_aData.sBufferStandard.appendAscii( "\n"                                                                          );
    m_aData.sBufferStandard.appendAscii( "   OpenOffice.org is free software: you can redistribute it and/or modify\n" );
    m_aData.sBufferStandard.appendAscii( "   it under the terms of the GNU Lesser General Public License version 3\n"  );
    m_aData.sBufferStandard.appendAscii( "   only, as published by the Free Software Foundation.\n"                    );
    m_aData.sBufferStandard.appendAscii( "\n"                                                                          );
    m_aData.sBufferStandard.appendAscii( "   OpenOffice.org is distributed in the hope that it will be useful,\n"      );
    m_aData.sBufferStandard.appendAscii( "   but WITHOUT ANY WARRANTY; without even the implied warranty of\n"         );
    m_aData.sBufferStandard.appendAscii( "   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"          );
    m_aData.sBufferStandard.appendAscii( "   GNU Lesser General Public License version 3 for more details\n"           );
    m_aData.sBufferStandard.appendAscii( "   (a copy is included in the LICENSE file that accompanied this code).\n"   );
    m_aData.sBufferStandard.appendAscii( "\n"                                                                          );
    m_aData.sBufferStandard.appendAscii( "   You should have received a copy of the GNU Lesser General Public License\n");
    m_aData.sBufferStandard.appendAscii( "   version 3 along with OpenOffice.org.  If not, see\n"                      );
    m_aData.sBufferStandard.appendAscii( "   <http://www.openoffice.org/license.html>\n"                               );
    m_aData.sBufferStandard.appendAscii( "   for a copy of the LGPLv3 License.\n"                                      );
    m_aData.sBufferStandard.appendAscii( "\n"                                                                          );
    m_aData.sBufferStandard.appendAscii( "-->\n"                                                                       );

    if( m_aData.nVersionOutput >= DRAFT_SPLIT_VERSION )
    {
        m_aData.sBufferAdditional.appendAscii( "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"                                );
        m_aData.sBufferAdditional.appendAscii( "<!-- \n"                                                                     );
        m_aData.sBufferAdditional.appendAscii( "\n"                                                                          );
        m_aData.sBufferAdditional.appendAscii( "  DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.\n"           );
        m_aData.sBufferAdditional.appendAscii( "\n"                                                                          );
        m_aData.sBufferAdditional.appendAscii( "  Copyright 2000, 2010 Oracle and/or its affiliates.\n"                                );
        m_aData.sBufferAdditional.appendAscii( "\n"                                                                          );
        m_aData.sBufferAdditional.appendAscii( "  OpenOffice.org - a multi-platform office productivity suite\n"             );
        m_aData.sBufferAdditional.appendAscii( "\n"                                                                          );
        m_aData.sBufferAdditional.appendAscii( "  This file is part of OpenOffice.org.\n"                                    );
        m_aData.sBufferAdditional.appendAscii( "\n"                                                                          );
        m_aData.sBufferAdditional.appendAscii( "  OpenOffice.org is free software: you can redistribute it and/or modify\n"  );
        m_aData.sBufferAdditional.appendAscii( "  it under the terms of the GNU Lesser General Public License version 3\n"   );
        m_aData.sBufferAdditional.appendAscii( "  only, as published by the Free Software Foundation.\n"                     );
        m_aData.sBufferAdditional.appendAscii( "\n"                                                                          );
        m_aData.sBufferAdditional.appendAscii( "  OpenOffice.org is distributed in the hope that it will be useful,\n"       );
        m_aData.sBufferAdditional.appendAscii( "  but WITHOUT ANY WARRANTY; without even the implied warranty of\n"          );
        m_aData.sBufferAdditional.appendAscii( "  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"           );
        m_aData.sBufferAdditional.appendAscii( "  GNU Lesser General Public License version 3 for more details\n"            );
        m_aData.sBufferAdditional.appendAscii( "  (a copy is included in the LICENSE file that accompanied this code).\n"    );
        m_aData.sBufferAdditional.appendAscii( "\n"                                                                          );
        m_aData.sBufferAdditional.appendAscii( "  You should have received a copy of the GNU Lesser General Public License\n");
        m_aData.sBufferAdditional.appendAscii( "  version 3 along with OpenOffice.org.  If not, see\n"                       );
        m_aData.sBufferAdditional.appendAscii( "  <http://www.openoffice.org/license.html>\n"                                );
        m_aData.sBufferAdditional.appendAscii( "  for a copy of the LGPLv3 License.\n"                                       );
        m_aData.sBufferAdditional.appendAscii( "\n"                                                                          );
        m_aData.sBufferAdditional.appendAscii( "-->\n"                                                                       );
    }
}

//*****************************************************************************************************************
void XCDGenerator::impl_generateTypeTemplate()
{
    //_________________________________________________________________________________________________________________
    if( m_aData.nVersionOutput==1 || m_aData.nVersionOutput==2 )
    {
        m_aData.sBufferStandard.appendAscii( "\t\t<schema:group cfg:name=\"Type\">\n"                                                                                                                                                                  );

        m_aData.sBufferStandard.appendAscii( "\t\t\t<schema:value cfg:name=\"Preferred\" cfg:type=\"boolean\" cfg:writable=\""                                                                                                                         );
        m_aData.sBufferStandard.appendAscii( m_aData.bWriteable==sal_True ? "true\">\n" : "false\">\n"                                                                                                                                                 );
        m_aData.sBufferStandard.appendAscii("\t\t\t\t<schema:documentation>\n"                                                                                                                                                                         );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t\t<schema:description>Specifies the preferred type for an extension if more then one match given URL</schema:description>\n"                                                                     );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t</schema:documentation>\n"                                                                                                                                                                       );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t<default:data>false</default:data>\n"                                                                                                                                                            );
        m_aData.sBufferStandard.appendAscii( "\t\t\t</schema:value>\n"                                                                                                                                                                                 );

        m_aData.sBufferStandard.appendAscii( "\t\t\t<schema:value cfg:name=\"UIName\" cfg:type=\"string\" cfg:localized=\"true\" cfg:writable=\""                                                                                                      );
        m_aData.sBufferStandard.appendAscii( m_aData.bWriteable==sal_True ? "true\">\n" : "false\">\n"                                                                                                                                                 );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t<schema:documentation>\n"                                                                                                                                                                        );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t\t<schema:description>Specifies the external name of this type</schema:description>\n"                                                                                                           );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t</schema:documentation>\n"                                                                                                                                                                       );
        m_aData.sBufferStandard.appendAscii( "\t\t\t</schema:value>\n"                                                                                                                                                                                 );

        m_aData.sBufferStandard.appendAscii( "\t\t\t<schema:value cfg:name=\"MediaType\" cfg:type=\"string\" cfg:writable=\""                                                                                                                          );
        m_aData.sBufferStandard.appendAscii( m_aData.bWriteable==sal_True ? "true\">\n" : "false\">\n"                                                                                                                                                 );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t<schema:documentation>\n"                                                                                                                                                                        );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t\t<schema:description>Specifies the mime type </schema:description>\n"                                                                                                                           );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t</schema:documentation>\n"                                                                                                                                                                       );
        m_aData.sBufferStandard.appendAscii( "\t\t\t</schema:value>\n"                                                                                                                                                                                 );

        m_aData.sBufferStandard.appendAscii( "\t\t\t<schema:value cfg:name=\"ClipboardFormat\" cfg:type=\"string\" cfg:writable=\""                                                                                                                    );
        m_aData.sBufferStandard.appendAscii( m_aData.bWriteable==sal_True ? "true\">\n" : "false\">\n"                                                                                                                                                 );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t<schema:documentation>\n"                                                                                                                                                                        );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t\t<schema:description>Specifies the clipboard format name</schema:description>\n"                                                                                                                );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t</schema:documentation>\n"                                                                                                                                                                       );
        m_aData.sBufferStandard.appendAscii( "\t\t\t</schema:value>\n"                                                                                                                                                                                 );

        m_aData.sBufferStandard.appendAscii( "\t\t\t<schema:value cfg:name=\"URLPattern\" cfg:type=\"string\" cfg:derivedBy=\"list\" cfg:writable=\""                                                                                                  );
        m_aData.sBufferStandard.appendAscii( m_aData.bWriteable==sal_True ? "true\">\n" : "false\">\n"                                                                                                                                                 );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t<schema:documentation>\n"                                                                                                                                                                        );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t\t<schema:description>Specifies the patterns used for URLs. This type is only relevant for HTTP, FTP etc. and is used for internal URL formats like private:factory etc.</schema:description>\n" );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t</schema:documentation>\n"                                                                                                                                                                       );
        m_aData.sBufferStandard.appendAscii( "\t\t\t</schema:value>\n"                                                                                                                                                                                 );

        m_aData.sBufferStandard.appendAscii( "\t\t\t<schema:value cfg:name=\"Extensions\" cfg:type=\"string\" cfg:derivedBy=\"list\" cfg:writable=\""                                                                                                  );
        m_aData.sBufferStandard.appendAscii( m_aData.bWriteable==sal_True ? "true\">\n" : "false\">\n"                                                                                                                                                 );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t<schema:documentation>\n"                                                                                                                                                                        );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t\t<schema:description>Specifies the possible file extensions.</schema:description>\n"                                                                                                            );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t</schema:documentation>\n"                                                                                                                                                                       );
        m_aData.sBufferStandard.appendAscii( "\t\t\t</schema:value>\n"                                                                                                                                                                                 );

        m_aData.sBufferStandard.appendAscii( "\t\t\t<schema:value cfg:name=\"DocumentIconID\" cfg:type=\"int\" cfg:writable=\""                                                                                                                        );
        m_aData.sBufferStandard.appendAscii( m_aData.bWriteable==sal_True ? "true\">\n" : "false\">\n"                                                                                                                                                 );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t<schema:documentation>\n"                                                                                                                                                                        );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t\t<schema:description>Specifies the document icon ID of this type</schema:description>\n"                                                                                                        );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t</schema:documentation>\n"                                                                                                                                                                       );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t<default:data>0</default:data>\n"                                                                                                                                                                );
        m_aData.sBufferStandard.appendAscii( "\t\t\t</schema:value>\n"                                                                                                                                                                                 );

        m_aData.sBufferStandard.appendAscii( "\t\t</schema:group>\n"                                                                                                                                                                                   );
    }
    //_________________________________________________________________________________________________________________
    else if( m_aData.nVersionOutput>=3 )
    {
        m_aData.sBufferStandard.appendAscii( "\t\t<schema:group cfg:name=\"Type\">\n"                                                                                                                                                                  );

        m_aData.sBufferStandard.appendAscii( "\t\t\t<schema:value cfg:name=\"UIName\" cfg:type=\"string\" cfg:localized=\"true\" cfg:writable=\""                                                                                                      );
        m_aData.sBufferStandard.appendAscii( m_aData.bWriteable==sal_True ? "true\">\n" : "false\">\n"                                                                                                                                                 );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t<schema:documentation>\n"                                                                                                                                                                        );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t\t<schema:description>Specifies the external name of this type</schema:description>\n"                                                                                                           );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t</schema:documentation>\n"                                                                                                                                                                       );
        m_aData.sBufferStandard.appendAscii( "\t\t\t</schema:value>\n"                                                                                                                                                                                 );

        m_aData.sBufferStandard.appendAscii( "\t\t\t<schema:value cfg:name=\"Data\" cfg:type=\"string\" cfg:writable=\""                                                                                                                               );
        m_aData.sBufferStandard.appendAscii( m_aData.bWriteable==sal_True ? "true\">\n" : "false\">\n"                                                                                                                                                 );
        m_aData.sBufferStandard.appendAscii("\t\t\t\t<schema:documentation>\n"                                                                                                                                                                         );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t\t<schema:description>Containes all data of a type as an own formated string.{Preferred, MediaType, ClipboardFormat, URLPattern, Extensions, DocumentIconID}</schema:description>\n"             );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t</schema:documentation>\n"                                                                                                                                                                       );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t<default:data>false</default:data>\n"                                                                                                                                                            );
        m_aData.sBufferStandard.appendAscii( "\t\t\t</schema:value>\n"                                                                                                                                                                                 );

        m_aData.sBufferStandard.appendAscii( "\t\t</schema:group>\n"                                                                                                                                                                                   );
    }
}

//*****************************************************************************************************************
void XCDGenerator::impl_generateFilterTemplate()
{
    //_________________________________________________________________________________________________________________
    if( m_aData.nVersionOutput==1 || m_aData.nVersionOutput==2 )
    {
        m_aData.sBufferStandard.appendAscii( "\t\t<schema:group cfg:name=\"Filter\">\n"                                                                                                            );

        m_aData.sBufferStandard.appendAscii( "\t\t\t<schema:value cfg:name=\"Installed\" cfg:type=\"boolean\" cfg:writable=\""                                                                     );
        m_aData.sBufferStandard.appendAscii( m_aData.bWriteable==sal_True ? "true\">\n" : "false\">\n"                                                                                             );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t<schema:documentation>\n"                                                                                                                    );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t\t<schema:description>Make it possible to enable or disable filter by setup!</schema:description>\n"                                         );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t</schema:documentation>\n"                                                                                                                   );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t<default:data>false</default:data>\n"                                                                                                        );
        m_aData.sBufferStandard.appendAscii( "\t\t\t</schema:value>\n"                                                                                                                             );

        if( m_aData.nVersionOutput==2 )
        {
        m_aData.sBufferStandard.appendAscii( "\t\t\t<schema:value cfg:name=\"Order\" cfg:type=\"int\" cfg:writable=\""                                                                             );
        m_aData.sBufferStandard.appendAscii( m_aData.bWriteable==sal_True ? "true\">\n" : "false\">\n"                                                                                             );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t<schema:documentation>\n"                                                                                                                    );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t\t<schema:description>Specifies order of filters for relevant module; don't used for default filter!</schema:description>\n"                 );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t</schema:documentation>\n"                                                                                                                   );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t<default:data>0</default:data>\n"                                                                                                            );
        m_aData.sBufferStandard.appendAscii( "\t\t\t</schema:value>\n"                                                                                                                             );
        }

        m_aData.sBufferStandard.appendAscii( "\t\t\t<schema:value cfg:name=\"UIName\" cfg:type=\"string\" cfg:localized=\"true\" cfg:writable=\""                                                  );
        m_aData.sBufferStandard.appendAscii( m_aData.bWriteable==sal_True ? "true\">\n" : "false\">\n"                                                                                             );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t<schema:documentation>\n"                                                                                                                    );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t\t<schema:description>Specifies the external name of the filter which is displayed at the user interface (dialog).</schema:description>\n"   );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t</schema:documentation>\n"                                                                                                                   );
        m_aData.sBufferStandard.appendAscii( "\t\t\t</schema:value>\n"                                                                                                                             );

        m_aData.sBufferStandard.appendAscii( "\t\t\t<schema:value cfg:name=\"Type\" cfg:type=\"string\" cfg:writable=\""                                                                           );
        m_aData.sBufferStandard.appendAscii( m_aData.bWriteable==sal_True ? "true\">\n" : "false\">\n"                                                                                             );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t<schema:documentation>\n"                                                                                                                    );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t\t<schema:description>Specifies the relative type key name of the filter, e.g. Type/T1</schema:description>\n"                               );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t</schema:documentation>\n"                                                                                                                   );
        m_aData.sBufferStandard.appendAscii( "\t\t\t</schema:value>\n"                                                                                                                             );

        m_aData.sBufferStandard.appendAscii( "\t\t\t<schema:value cfg:name=\"DocumentService\" cfg:type=\"string\" cfg:writable=\""                                                                );
        m_aData.sBufferStandard.appendAscii( m_aData.bWriteable==sal_True ? "true\">\n" : "false\">\n"                                                                                             );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t<schema:documentation>\n"                                                                                                                    );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t\t<schema:description>Specifies the name of the UNO service to implement the document.</schema:description>\n"                               );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t</schema:documentation>\n"                                                                                                                   );
        m_aData.sBufferStandard.appendAscii( "\t\t\t</schema:value>\n"                                                                                                                             );

        m_aData.sBufferStandard.appendAscii( "\t\t\t<schema:value cfg:name=\"FilterService\" cfg:type=\"string\" cfg:writable=\""                                                                  );
        m_aData.sBufferStandard.appendAscii( m_aData.bWriteable==sal_True ? "true\">\n" : "false\">\n"                                                                                             );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t<schema:documentation>\n"                                                                                                                    );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t\t<schema:description>Specifies the name of the UNO service for importing the document.</schema:description>\n"                              );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t</schema:documentation>\n"                                                                                                                   );
        m_aData.sBufferStandard.appendAscii( "\t\t\t</schema:value>\n"                                                                                                                             );

        m_aData.sBufferStandard.appendAscii( "\t\t\t<schema:value cfg:name=\"Flags\" cfg:type=\"int\" cfg:writable=\""                                                                             );
        m_aData.sBufferStandard.appendAscii( m_aData.bWriteable==sal_True ? "true\">\n" : "false\">\n"                                                                                             );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t<schema:documentation>\n"                                                                                                                    );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t\t<schema:description>Specifies the properties of the filter</schema:description>\n"                                                         );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t</schema:documentation>\n"                                                                                                                   );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t<schema:type-info>\n"                                                                                                                        );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t\t<schema:value-names>\n"                                                                                                                    );
        impl_generateFilterFlagTemplate( FILTERFLAGNAME_IMPORT            , FILTERFLAG_IMPORT         , "mark filter for import"                                          );
        impl_generateFilterFlagTemplate( FILTERFLAGNAME_EXPORT            , FILTERFLAG_EXPORT         , "mark filter for export"                                          );
        impl_generateFilterFlagTemplate( FILTERFLAGNAME_TEMPLATE          , FILTERFLAG_TEMPLATE                                                                           );
        impl_generateFilterFlagTemplate( FILTERFLAGNAME_INTERNAL          , FILTERFLAG_INTERNAL                                                                           );
        impl_generateFilterFlagTemplate( FILTERFLAGNAME_TEMPLATEPATH      , FILTERFLAG_TEMPLATEPATH                                                                       );
        impl_generateFilterFlagTemplate( FILTERFLAGNAME_OWN               , FILTERFLAG_OWN                                                                                );
        impl_generateFilterFlagTemplate( FILTERFLAGNAME_ALIEN             , FILTERFLAG_ALIEN                                                                              );
        impl_generateFilterFlagTemplate( FILTERFLAGNAME_USESOPTIONS       , FILTERFLAG_USESOPTIONS                                                                        );
        impl_generateFilterFlagTemplate( FILTERFLAGNAME_DEFAULT           , FILTERFLAG_DEFAULT        , "most important filter, if more then ones available"              );
        impl_generateFilterFlagTemplate( FILTERFLAGNAME_NOTINFILEDIALOG   , FILTERFLAG_NOTINFILEDIALOG, "don't show it in file dialogs!"                                  );
        impl_generateFilterFlagTemplate( FILTERFLAGNAME_NOTINCHOOSER      , FILTERFLAG_NOTINCHOOSER   , "don't show it in chooser!"                                       );
        impl_generateFilterFlagTemplate( FILTERFLAGNAME_ASYNCHRON         , FILTERFLAG_ASYNCHRON                                                                          );
        impl_generateFilterFlagTemplate( FILTERFLAGNAME_NOTINSTALLED      , FILTERFLAG_NOTINSTALLED   , "set, if the filter is not installed, but available on CD"        );
        impl_generateFilterFlagTemplate( FILTERFLAGNAME_CONSULTSERVICE    , FILTERFLAG_CONSULTSERVICE , "set, if the filter is not installed and not available an CD"     );
        impl_generateFilterFlagTemplate( FILTERFLAGNAME_3RDPARTYFILTER    , FILTERFLAG_3RDPARTYFILTER , "must set, if the filter is an external one"                      );
        impl_generateFilterFlagTemplate( FILTERFLAGNAME_PACKED            , FILTERFLAG_PACKED                                                                             );
        impl_generateFilterFlagTemplate( FILTERFLAGNAME_SILENTEXPORT      , FILTERFLAG_SILENTEXPORT                                                                       );
        impl_generateFilterFlagTemplate( FILTERFLAGNAME_PREFERED          , FILTERFLAG_PREFERED                                                                           );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t\t</schema:value-names>\n"                                                                                                                   );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t\t<schema:constraints xmlns:xsd=\"http://www.w3.org/1999/XMLSchema\"/>\n"                                                                    );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t</schema:type-info>\n"                                                                                                                       );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t<default:data>0</default:data>\n"                                                                                                            );
        m_aData.sBufferStandard.appendAscii( "\t\t\t</schema:value>\n"                                                                                                                             );

        m_aData.sBufferStandard.appendAscii( "\t\t\t<schema:value cfg:name=\"UserData\" cfg:type=\"string\" cfg:derivedBy=\"list\" cfg:writable=\""                                                );
        m_aData.sBufferStandard.appendAscii( m_aData.bWriteable==sal_True ? "true\">\n" : "false\">\n"                                                                                             );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t<schema:documentation>\n"                                                                                                                    );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t\t<schema:description>Specifies the user-defined data</schema:description>\n"                                                                );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t</schema:documentation>\n"                                                                                                                   );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t<default:data/>\n"                                                                                                                           );
        m_aData.sBufferStandard.appendAscii( "\t\t\t</schema:value>\n"                                                                                                                             );

        m_aData.sBufferStandard.appendAscii( "\t\t\t<schema:value cfg:name=\"FileFormatVersion\" cfg:type=\"int\" cfg:writable=\""                                                                 );
        m_aData.sBufferStandard.appendAscii( m_aData.bWriteable==sal_True ? "true\">\n" : "false\">\n"                                                                                             );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t<!--This should be removed to UserData later-->\n"                                                                                           );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t<schema:documentation>\n"                                                                                                                    );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t\t<schema:description>Specifies the file format version of the filter</schema:description>\n"                                                );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t</schema:documentation>\n"                                                                                                                   );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t<default:data>0</default:data>\n"                                                                                                            );
        m_aData.sBufferStandard.appendAscii( "\t\t\t</schema:value>\n"                                                                                                                             );

        m_aData.sBufferStandard.appendAscii( "\t\t\t<schema:value cfg:name=\"TemplateName\" cfg:type=\"string\" cfg:writable=\""                                                                   );
        m_aData.sBufferStandard.appendAscii( m_aData.bWriteable==sal_True ? "true\">\n" : "false\">\n"                                                                                             );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t<!--This should be removed to UserData later-->\n"                                                                                           );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t<schema:documentation>\n"                                                                                                                    );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t\t<schema:description>Specifies the template used for importing the file with the specified filter.</schema:description>\n"                  );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t\t</schema:documentation>\n"                                                                                                                 );
        m_aData.sBufferStandard.appendAscii( "\t\t\t</schema:value>\n"                                                                                                                             );

        m_aData.sBufferStandard.appendAscii( "\t\t</schema:group>\n"                                                                                                                               );
    //_________________________________________________________________________________________________________________
    }
    else if( m_aData.nVersionOutput>=3 )
    {
        m_aData.sBufferStandard.appendAscii( "\t\t<schema:group cfg:name=\"Filter\">\n"                                                                                                                                                                    );

        m_aData.sBufferStandard.appendAscii( "\t\t\t<schema:value cfg:name=\"Installed\" cfg:type=\"boolean\" cfg:writable=\""                                                                                                                             );
        m_aData.sBufferStandard.appendAscii( m_aData.bWriteable==sal_True ? "true\">\n" : "false\">\n"                                                                                                                                                     );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t<schema:documentation>\n"                                                                                                                                                                            );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t\t<schema:description>Make it possible to enable or disable filter by setup!</schema:description>\n"                                                                                                 );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t</schema:documentation>\n"                                                                                                                                                                           );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t<default:data>false</default:data>\n"                                                                                                                                                                );
        m_aData.sBufferStandard.appendAscii( "\t\t\t</schema:value>\n"                                                                                                                                                                                     );

        m_aData.sBufferStandard.appendAscii( "\t\t\t<schema:value cfg:name=\"UIName\" cfg:type=\"string\" cfg:localized=\"true\" cfg:writable=\""                                                                                                          );
        m_aData.sBufferStandard.appendAscii( m_aData.bWriteable==sal_True ? "true\">\n" : "false\">\n"                                                                                                                                                     );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t<schema:documentation>\n"                                                                                                                                                                            );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t\t<schema:description>Specifies the external name of the filter which is displayed at the user interface (dialog).</schema:description>\n"                                                           );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t</schema:documentation>\n"                                                                                                                                                                           );
        m_aData.sBufferStandard.appendAscii( "\t\t\t</schema:value>\n"                                                                                                                                                                                     );

        m_aData.sBufferStandard.appendAscii( "\t\t\t<schema:value cfg:name=\"Data\" cfg:type=\"string\" cfg:writable=\""                                                                                                                                   );
        m_aData.sBufferStandard.appendAscii( m_aData.bWriteable==sal_True ? "true\">\n" : "false\">\n"                                                                                                                                                     );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t<schema:documentation>\n"                                                                                                                                                                            );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t\t<schema:description>All data of filter written in own format. {Order, OldName, Type, DocumentService, FilterService, Flags, UserData, FilteFormatVersion, TemplateName}</schema:description>\n"    );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t</schema:documentation>\n"                                                                                                                                                                           );
        m_aData.sBufferStandard.appendAscii( "\t\t\t</schema:value>\n"                                                                                                                                                                                     );

        m_aData.sBufferStandard.appendAscii( "\t\t</schema:group>\n"                                                                                                                                                                                       );
    }
}

//*****************************************************************************************************************
void XCDGenerator::impl_generateFilterFlagTemplate( const ::rtl::OUString& sName, sal_Int32 nValue, const ::rtl::OString& sDescription )
{
    m_aData.sBufferStandard.appendAscii( "\t\t\t\t\t\t<schema:named-value name=\"" );
    m_aData.sBufferStandard.append     ( sName                                     );
    m_aData.sBufferStandard.appendAscii( "\" value=\""                             );
    m_aData.sBufferStandard.append     ( nValue                                    );
    m_aData.sBufferStandard.appendAscii( "\""                                      );

    if( sDescription.getLength() > 0 )
    {
        m_aData.sBufferStandard.appendAscii( ">\n\t\t\t\t\t\t\t<schema:description>"   );
        m_aData.sBufferStandard.appendAscii( sDescription                              );
        m_aData.sBufferStandard.appendAscii( "</schema:description>\n"                 );
        m_aData.sBufferStandard.appendAscii( "\t\t\t\t\t\t</schema:named-value>\n"     );
    }
    else
    {
        m_aData.sBufferStandard.appendAscii( "/>\n"                                    );
    }
}

//*****************************************************************************************************************
void XCDGenerator::impl_generateDetectorTemplate()
{
    m_aData.sBufferStandard.appendAscii( "\t\t<schema:group cfg:name=\"DetectService\">\n"                                                             );
    m_aData.sBufferStandard.appendAscii( "\t\t\t<schema:value cfg:name=\"Types\" cfg:type=\"string\" cfg:derivedBy=\"list\" cfg:writable=\""           );
    m_aData.sBufferStandard.appendAscii( m_aData.bWriteable==sal_True ? "true\">\n" : "false\">\n"                                                     );
    m_aData.sBufferStandard.appendAscii( "\t\t\t\t<schema:documentation>\n"                                                                            );
    m_aData.sBufferStandard.appendAscii( "\t\t\t\t\t<schema:description>List of types which the service has registered for.</schema:description>\n"    );
    m_aData.sBufferStandard.appendAscii( "\t\t\t\t</schema:documentation>\n"                                                                           );
    m_aData.sBufferStandard.appendAscii( "\t\t\t</schema:value>\n"                                                                                     );
    m_aData.sBufferStandard.appendAscii( "\t\t</schema:group>\n"                                                                                       );
}

//*****************************************************************************************************************
void XCDGenerator::impl_generateLoaderTemplate()
{
    m_aData.sBufferStandard.appendAscii( "\t\t<schema:group cfg:name=\"FrameLoader\">\n"                                                                                                       );
    m_aData.sBufferStandard.appendAscii( "\t\t\t<schema:value cfg:name=\"UIName\" cfg:type=\"string\" cfg:localized=\"true\" cfg:writable=\""                                                  );
    m_aData.sBufferStandard.appendAscii( m_aData.bWriteable==sal_True ? "true\">\n" : "false\">\n"                                                                                             );
    m_aData.sBufferStandard.appendAscii( "\t\t\t\t<schema:documentation>\n"                                                                                                                    );
    m_aData.sBufferStandard.appendAscii( "\t\t\t\t\t<schema:description>Specifies the external name of the filter which is displayed at the user interface (dialog).</schema:description>\n"   );
    m_aData.sBufferStandard.appendAscii( "\t\t\t\t</schema:documentation>\n"                                                                                                                   );
    m_aData.sBufferStandard.appendAscii( "\t\t\t</schema:value>\n"                                                                                                                             );
    m_aData.sBufferStandard.appendAscii( "\t\t\t<schema:value cfg:name=\"Types\" cfg:type=\"string\" cfg:derivedBy=\"list\" cfg:writable=\""                                                   );
    m_aData.sBufferStandard.appendAscii( m_aData.bWriteable==sal_True ? "true\">\n" : "false\">\n"                                                                                             );
    m_aData.sBufferStandard.appendAscii( "\t\t\t\t<schema:documentation>\n"                                                                                                                    );
    m_aData.sBufferStandard.appendAscii( "\t\t\t\t\t<schema:description>List of types which the service has registered for.</schema:description>\n"                                            );
    m_aData.sBufferStandard.appendAscii( "\t\t\t\t</schema:documentation>\n"                                                                                                                   );
    m_aData.sBufferStandard.appendAscii( "\t\t\t</schema:value>\n"                                                                                                                             );
    m_aData.sBufferStandard.appendAscii( "\t\t</schema:group>\n"                                                                                                                               );
}

//*****************************************************************************************************************
void XCDGenerator::impl_generateContentHandlerTemplate()
{
    m_aData.sBufferStandard.appendAscii( "\t\t<schema:group cfg:name=\"ContentHandler\">\n"                                                            );
    m_aData.sBufferStandard.appendAscii( "\t\t\t<schema:value cfg:name=\"Types\" cfg:type=\"string\" cfg:derivedBy=\"list\" cfg:writable=\""           );
    m_aData.sBufferStandard.appendAscii( m_aData.bWriteable==sal_True ? "true\">\n" : "false\">\n"                                                     );
    m_aData.sBufferStandard.appendAscii( "\t\t\t\t<schema:documentation>\n"                                                                            );
    m_aData.sBufferStandard.appendAscii( "\t\t\t\t\t<schema:description>List of types which could be handled by this service.</schema:description>\n"  );
    m_aData.sBufferStandard.appendAscii( "\t\t\t\t</schema:documentation>\n"                                                                           );
    m_aData.sBufferStandard.appendAscii( "\t\t\t</schema:value>\n"                                                                                     );
    m_aData.sBufferStandard.appendAscii( "\t\t</schema:group>\n"                                                                                       );
}

//*****************************************************************************************************************
void XCDGenerator::impl_generateTypeSet()
{
    if( m_aData.pFilterCache->hasTypes() == sal_False )
    {
        // generate empty set!
        m_aData.sBufferStandard.appendAscii  ( "\t<schema:set cfg:name=\"Types\" cfg:element-type=\"Type\"/>\n"                 );

        if( m_aData.nVersionOutput >= DRAFT_SPLIT_VERSION )
        {
            m_aData.sBufferAdditional.appendAscii( "\t<schema:set cfg:name=\"Types\" cfg:element-type=\"Type\" cfg:component=\""    );
            m_aData.sBufferAdditional.append     ( m_aData.sPackageStandard                                                         );
            m_aData.sBufferAdditional.appendAscii( "\"/>\n"                                                                         );
        }
    }
    else
    {
        // generate filled set
        // open set
        m_aData.sBufferStandard.appendAscii  ( "\t<schema:set cfg:name=\"Types\" cfg:element-type=\"Type\">\n"                  );

        if( m_aData.nVersionOutput >= DRAFT_SPLIT_VERSION )
        {
            m_aData.sBufferAdditional.appendAscii( "\t<schema:set cfg:name=\"Types\" cfg:element-type=\"Type\" cfg:component=\""    );
            m_aData.sBufferAdditional.append     ( m_aData.sPackageStandard                                                         );
            m_aData.sBufferAdditional.appendAscii( "\">\n"                                                                          );
        }

        css::uno::Sequence< ::rtl::OUString > lNames    = m_aData.pFilterCache->getAllTypeNames();
        css::uno::Sequence< ::rtl::OUString > lEncNames ( lNames )                               ;
        sal_Int32                             nCount    = lNames.getLength()                     ;
        sal_Int32                             nItem     = 0                                      ;

        XCDGenerator::impl_orderAlphabetical( lNames );

        if( m_aData.nVersionOutput == 6 && m_aData.nVersionInput < 6 )
        {
            ::rtl::OUString sName   ;
            ::rtl::OUString sEncName;
            for( nItem=0; nItem<nCount; ++nItem )
            {
                sName            = lNames[nItem]              ;
                lEncNames[nItem] = impl_encodeSetName( sName );
            }
        }

        for( nItem=0; nItem<nCount; ++nItem )
        {
            ::rtl::OUString sName   = lNames[nItem]                         ;
            FileType        aItem   = m_aData.pFilterCache->getType( sName );
            EFilterPackage  ePackage                                        ;

            ++m_aData.nWrittenTypes;

            if( m_aData.nVersionOutput==1 || m_aData.nVersionOutput==2 )
            {
                // open set entry by using name
                m_aData.sBufferStandard.appendAscii( "\t\t<default:group cfg:name=\""  );
                m_aData.sBufferStandard.append     ( sName                             );
                m_aData.sBufferStandard.appendAscii( "\">\n"                           );

                // write properties
                impl_generateBoolProperty       ( m_aData.sBufferStandard, SUBKEY_PREFERRED        , aItem.bPreferred       );
                impl_generateUINamesProperty    ( m_aData.sBufferStandard, SUBKEY_UINAME           , aItem.lUINames         );
                impl_generateStringProperty     ( m_aData.sBufferStandard, SUBKEY_MEDIATYPE        , aItem.sMediaType       );
                impl_generateStringProperty     ( m_aData.sBufferStandard, SUBKEY_CLIPBOARDFORMAT  , aItem.sClipboardFormat );
                impl_generateStringListProperty ( m_aData.sBufferStandard, SUBKEY_URLPATTERN       , aItem.lURLPattern      );
                impl_generateStringListProperty ( m_aData.sBufferStandard, SUBKEY_EXTENSIONS       , aItem.lExtensions      );
                impl_generateIntProperty        ( m_aData.sBufferStandard, SUBKEY_DOCUMENTICONID   , aItem.nDocumentIconID  );

                // close set node
                m_aData.sBufferStandard.appendAscii( "\t\t</default:group>\n" );
            }
            else if( m_aData.nVersionOutput >= 3 )
            {
                ::rtl::OUString        sPath       = DECLARE_ASCII("org.openoffice.Office.");
                ::rtl::OUStringBuffer* pXCDBuffer  = &(m_aData.sBufferStandard      );
                ::rtl::OUStringBuffer* pSCPBuffer  = &(m_aData.sNew2OldSCPStandard  );
                ::rtl::OUStringBuffer* pListBuffer = &(m_aData.sStandardTypeList    );

                if( m_aData.nVersionOutput >= DRAFT_SPLIT_VERSION )
                {
                    XCDGenerator::impl_classifyType( m_aData, sName, ePackage );
                    switch( ePackage )
                    {
                        case E_ADDITIONAL   :   {
                                                    sPath      += m_aData.sPackageAdditional      ;
                                                    pXCDBuffer  = &(m_aData.sBufferAdditional    );
                                                    pSCPBuffer  = &(m_aData.sNew2OldSCPAdditional);
                                                    pListBuffer = &(m_aData.sAdditionalTypeList  );
                                                }
                    }
                }
                else
                {
                    sPath += m_aData.sPackageStandard;
                }

                sPath += CFG_PATH_SEPERATOR      ;
                sPath += DECLARE_ASCII( "Types" );
                sPath += CFG_PATH_SEPERATOR      ;

                pListBuffer->append     ( sName );
                pListBuffer->appendAscii( "\n"  );

                if( m_aData.nVersionOutput == 6 && m_aData.nVersionInput < 6 )
                {
                    pSCPBuffer->appendAscii( "\""               );
                    pSCPBuffer->append     ( sPath              );
                    pSCPBuffer->append     ( lNames[nItem]      );
                    pSCPBuffer->appendAscii( "\"\t\""           );
                    pSCPBuffer->append     ( sPath              );
                    pSCPBuffer->appendAscii( "Type"             );
                    pSCPBuffer->append     ( CFG_ENCODING_OPEN  );
                    pSCPBuffer->append     ( lNames[nItem]      );
                    pSCPBuffer->append     ( CFG_ENCODING_CLOSE );
                    pSCPBuffer->appendAscii( "\"\n"             );

                    sName       = lEncNames[nItem];
                    aItem.sName = sName;
                }

                // open set entry by using name
                pXCDBuffer->appendAscii( "\t\t<default:group cfg:name=\""  );
                pXCDBuffer->append     ( sName                             );
                pXCDBuffer->appendAscii( "\">\n"                           );

                // write properties
                impl_generateUINamesProperty( *pXCDBuffer, SUBKEY_UINAME, aItem.lUINames                           );
                impl_generateStringProperty ( *pXCDBuffer, SUBKEY_DATA  , FilterCFGAccess::encodeTypeData( aItem ) );

                // close set node
                pXCDBuffer->appendAscii( "\t\t</default:group>\n" );
            }
        }

        // close set
        m_aData.sBufferStandard.appendAscii( "\t</schema:set>\n" );
        if( m_aData.nVersionOutput >= DRAFT_SPLIT_VERSION )
        {
            m_aData.sBufferAdditional.appendAscii( "\t</schema:set>\n" );
        }
    }
}

//*****************************************************************************************************************
void XCDGenerator::impl_generateFilterSet()
{
    if( m_aData.pFilterCache->hasFilters() == sal_False )
    {
        // write empty filter set.
        m_aData.sBufferStandard.appendAscii( "\t<schema:set cfg:name=\"Filters\" cfg:element-type=\"Filter\"/>\n" );
        if( m_aData.nVersionOutput >= DRAFT_SPLIT_VERSION )
        {
            m_aData.sBufferAdditional.appendAscii( "\t<schema:set cfg:name=\"Filters\" cfg:element-type=\"Filter\" cfg:component=\""    );
            m_aData.sBufferAdditional.append     ( m_aData.sPackageStandard                                                             );
            m_aData.sBufferAdditional.appendAscii( "\"/>\n"                                                                             );
        }
    }
    else
    {
        // open set
        m_aData.sBufferStandard.appendAscii( "\t<schema:set cfg:name=\"Filters\" cfg:element-type=\"Filter\">\n" );
        if( m_aData.nVersionOutput >= DRAFT_SPLIT_VERSION )
        {
            m_aData.sBufferAdditional.appendAscii( "\t<schema:set cfg:name=\"Filters\" cfg:element-type=\"Filter\" cfg:component=\""    );
            m_aData.sBufferAdditional.append     ( m_aData.sPackageStandard                                                             );
            m_aData.sBufferAdditional.appendAscii( "\">\n"                                                                              );
        }

        css::uno::Sequence< ::rtl::OUString > lNewNames = m_aData.pFilterCache->getAllFilterNames();
        css::uno::Sequence< ::rtl::OUString > lOldNames ( lNewNames )                              ;
        css::uno::Sequence< ::rtl::OUString > lEncNames ( lNewNames )                              ;
        sal_Int32                             nCount    = lNewNames.getLength()                    ;
        sal_Int32                             nItem     = 0                                        ;

        XCDGenerator::impl_orderAlphabetical( lNewNames );

        if( m_aData.nVersionOutput == 6 && m_aData.nVersionInput < 6 )
        {
            ::rtl::OUString sNewName;
            ::rtl::OUString sOldName;
            for( nItem=0; nItem<nCount; ++nItem )
            {
                sNewName         = lNewNames[nItem]                   ;
                sOldName         = impl_getOldFilterName  ( sNewName );
                lOldNames[nItem] = sOldName                           ;
                lEncNames[nItem] = impl_encodeSetName     ( sOldName );
            }
        }

        for( nItem=0; nItem<nCount; ++nItem )
        {
            ::rtl::OUString sName = lNewNames[nItem]                                    ;
            Filter          aItem = m_aData.pFilterCache->getFilter( lNewNames[nItem] ) ;
            EFilterPackage  ePackage                                                    ;

            ++m_aData.nWrittenFilters;

            if( m_aData.nVersionOutput==1 || m_aData.nVersionOutput==2 )
            {
                // open set node by using name
                m_aData.sBufferStandard.appendAscii( "\t\t<default:group cfg:name=\""  );
                m_aData.sBufferStandard.append     ( sName                             );
                m_aData.sBufferStandard.appendAscii( "\">\n"                           );

                // write properties
                // Attention:
                // We generate "Installed=false" for all entries ... because it's the default for all filters.
                // You must work with a full office installation and change this to "true" in generated XML file!!!
                impl_generateBoolProperty       ( m_aData.sBufferStandard, SUBKEY_INSTALLED        , sal_False                );
                impl_generateIntProperty        ( m_aData.sBufferStandard, SUBKEY_ORDER            , aItem.nOrder             );
                impl_generateStringProperty     ( m_aData.sBufferStandard, SUBKEY_TYPE             , aItem.sType              );
                impl_generateUINamesProperty    ( m_aData.sBufferStandard, SUBKEY_UINAME           , aItem.lUINames           );
                impl_generateStringProperty     ( m_aData.sBufferStandard, SUBKEY_DOCUMENTSERVICE  , aItem.sDocumentService   );
                impl_generateStringProperty     ( m_aData.sBufferStandard, SUBKEY_FILTERSERVICE    , aItem.sFilterService     );
                impl_generateIntProperty        ( m_aData.sBufferStandard, SUBKEY_FLAGS            , aItem.nFlags             );
                impl_generateStringListProperty ( m_aData.sBufferStandard, SUBKEY_USERDATA         , aItem.lUserData          );
                impl_generateIntProperty        ( m_aData.sBufferStandard, SUBKEY_FILEFORMATVERSION, aItem.nFileFormatVersion );
                impl_generateStringProperty     ( m_aData.sBufferStandard, SUBKEY_TEMPLATENAME     , aItem.sTemplateName      );

                // close set node
                m_aData.sBufferStandard.appendAscii( "\t\t</default:group>\n" );
            }
            else if( m_aData.nVersionOutput>=3 )
            {
                ::rtl::OUString        sPath       = DECLARE_ASCII("org.openoffice.Office.");
                ::rtl::OUStringBuffer* pXCDBuffer  = &(m_aData.sBufferStandard      );
                ::rtl::OUStringBuffer* pSCPBuffer  = &(m_aData.sNew2OldSCPStandard  );
                ::rtl::OUStringBuffer* pListBuffer = &(m_aData.sStandardFilterList  );

                if( m_aData.nVersionOutput >= DRAFT_SPLIT_VERSION )
                {
                    XCDGenerator::impl_classifyFilter( m_aData, sName, ePackage, aItem.nOrder );
                    switch( ePackage )
                    {
                        case E_ADDITIONAL   :   {
                                                    sPath      += m_aData.sPackageAdditional      ;
                                                    pXCDBuffer  = &(m_aData.sBufferAdditional    );
                                                    pSCPBuffer  = &(m_aData.sNew2OldSCPAdditional);
                                                    pListBuffer = &(m_aData.sAdditionalFilterList);
                                                }
                    }
                }
                else
                {
                    sPath += m_aData.sPackageStandard;
                }

                sPath += CFG_PATH_SEPERATOR        ;
                sPath += DECLARE_ASCII( "Filters" );
                sPath += CFG_PATH_SEPERATOR        ;

                pListBuffer->append     ( sName );
                pListBuffer->appendAscii( "\n"  );

                if( m_aData.nVersionOutput == 6 && m_aData.nVersionInput < 6 )
                {
                    pSCPBuffer->appendAscii( "\""               );
                    pSCPBuffer->append     ( sPath              );
                    pSCPBuffer->append     ( lNewNames[nItem]   );
                    pSCPBuffer->appendAscii( "\"\t\""           );
                    pSCPBuffer->append     ( sPath              );
                    pSCPBuffer->appendAscii( "Filter"           );
                    pSCPBuffer->append     ( CFG_ENCODING_OPEN  );
                    pSCPBuffer->append     ( lOldNames[nItem]   );
                    pSCPBuffer->append     ( CFG_ENCODING_CLOSE );
                    pSCPBuffer->appendAscii( "\"\n"             );

                    sName       = lEncNames[nItem];
                    aItem.sName = sName;
                }

                // open set node by using name
                pXCDBuffer->appendAscii( "\t\t<default:group cfg:name=\"" );
                pXCDBuffer->append     ( sName                            );
                pXCDBuffer->appendAscii( "\">\n"                          );

                // write properties
                // Attention:
                // We generate "Installed=false" for all entries ... because it's the default for all filters.
                // You must work with a full office installation and change this to "true" in generated XML file!!!
                impl_generateBoolProperty   ( *pXCDBuffer, SUBKEY_INSTALLED, sal_False                                  );
                impl_generateUINamesProperty( *pXCDBuffer, SUBKEY_UINAME   , aItem.lUINames                             );
                impl_generateStringProperty ( *pXCDBuffer, SUBKEY_DATA     , FilterCFGAccess::encodeFilterData( aItem ) );

                // close set node
                pXCDBuffer->appendAscii( "\t\t</default:group>\n" );
            }
        }

        // close set
        m_aData.sBufferStandard.appendAscii( "\t</schema:set>\n" );
        if( m_aData.nVersionOutput >= DRAFT_SPLIT_VERSION )
        {
            m_aData.sBufferAdditional.appendAscii( "\t</schema:set>\n" );
        }
    }
}

//*****************************************************************************************************************
void XCDGenerator::impl_generateDetectorSet()
{
    if( m_aData.pFilterCache->hasDetectors() == sal_False )
    {
        // write empty detector set!
        m_aData.sBufferStandard.appendAscii( "\t<schema:set cfg:name=\"DetectServices\" cfg:element-type=\"DetectService\"/>\n" );
    }
    else
    {
        // open set
        m_aData.sBufferStandard.appendAscii( "\t<schema:set cfg:name=\"DetectServices\" cfg:element-type=\"DetectService\">\n" );

        css::uno::Sequence< ::rtl::OUString > lNames    = m_aData.pFilterCache->getAllDetectorNames();
        css::uno::Sequence< ::rtl::OUString > lEncNames ( lNames )                                   ;
        sal_Int32                             nCount    = lNames.getLength()                         ;
        sal_Int32                             nItem     = 0                                          ;

        XCDGenerator::impl_orderAlphabetical( lNames );

        if( m_aData.nVersionOutput == 6 && m_aData.nVersionInput < 6 )
        {
            ::rtl::OUString sName   ;
            ::rtl::OUString sEncName;
            for( nItem=0; nItem<nCount; ++nItem )
            {
                sName            = lNames[nItem]              ;
                lEncNames[nItem] = impl_encodeSetName( sName );

                m_aData.sNew2OldSCPStandard.appendAscii ( "org.openoffice.Office."  );
                m_aData.sNew2OldSCPStandard.append      ( m_aData.sPackageStandard  );
                m_aData.sNew2OldSCPStandard.append      ( CFG_PATH_SEPERATOR        );
                m_aData.sNew2OldSCPStandard.append      ( sName                     );
                m_aData.sNew2OldSCPStandard.appendAscii ( "\torg.openoffice.Office.");
                m_aData.sNew2OldSCPStandard.append      ( m_aData.sPackageStandard  );
                m_aData.sNew2OldSCPStandard.append      ( CFG_PATH_SEPERATOR        );
                m_aData.sNew2OldSCPStandard.appendAscii ( "DetectService"           );
                m_aData.sNew2OldSCPStandard.append      ( CFG_ENCODING_OPEN         );
                m_aData.sNew2OldSCPStandard.append      ( sName                     );
                m_aData.sNew2OldSCPStandard.append      ( CFG_ENCODING_CLOSE        );
                m_aData.sNew2OldSCPStandard.appendAscii ( "\n"                      );
            }
        }

        for( nItem=0; nItem<nCount; ++nItem )
        {
            ::rtl::OUString sName = lNames[nItem]                             ;
            Detector        aItem = m_aData.pFilterCache->getDetector( sName );

            if( m_aData.nVersionOutput == 6 && m_aData.nVersionInput < 6 )
            {
                sName = lEncNames[nItem];
            }

            ++m_aData.nWrittenDetectors;

            // open set node by using name
            m_aData.sBufferStandard.appendAscii( "\t\t<default:group cfg:name=\""  );
            m_aData.sBufferStandard.append     ( sName                             );
            m_aData.sBufferStandard.appendAscii( "\">\n"                           );

            // write properties
            impl_generateStringListProperty ( m_aData.sBufferStandard, SUBKEY_TYPES, aItem.lTypes );

            // close set node
            m_aData.sBufferStandard.appendAscii( "\t\t</default:group>\n" );
        }

        // close set
        m_aData.sBufferStandard.appendAscii( "\t</schema:set>\n" );
    }
}

//*****************************************************************************************************************
void XCDGenerator::impl_generateLoaderSet()
{
    if( m_aData.pFilterCache->hasLoaders() == sal_False )
    {
        // write empty loader set!
        m_aData.sBufferStandard.appendAscii( "\t<schema:set cfg:name=\"FrameLoaders\" cfg:element-type=\"FrameLoader\"/>\n" );
    }
    else
    {
        // open set
        m_aData.sBufferStandard.appendAscii( "\t<schema:set cfg:name=\"FrameLoaders\" cfg:element-type=\"FrameLoader\">\n" );

        css::uno::Sequence< ::rtl::OUString > lNames = m_aData.pFilterCache->getAllLoaderNames();
        css::uno::Sequence< ::rtl::OUString > lEncNames ( lNames )                              ;
        sal_Int32                             nCount = lNames.getLength()                       ;
        sal_Int32                             nItem  = 0                                        ;

        XCDGenerator::impl_orderAlphabetical( lNames );

        if( m_aData.nVersionOutput == 6 && m_aData.nVersionInput < 6 )
        {
            ::rtl::OUString sName   ;
            ::rtl::OUString sEncName;
            for( nItem=0; nItem<nCount; ++nItem )
            {
                sName            = lNames[nItem]              ;
                lEncNames[nItem] = impl_encodeSetName( sName );

                m_aData.sNew2OldSCPStandard.appendAscii ( "org.openoffice.Office."  );
                m_aData.sNew2OldSCPStandard.append      ( m_aData.sPackageStandard  );
                m_aData.sNew2OldSCPStandard.append      ( CFG_PATH_SEPERATOR        );
                m_aData.sNew2OldSCPStandard.append      ( sName                     );
                m_aData.sNew2OldSCPStandard.appendAscii ( "\torg.openoffice.Office.");
                m_aData.sNew2OldSCPStandard.append      ( m_aData.sPackageStandard  );
                m_aData.sNew2OldSCPStandard.append      ( CFG_PATH_SEPERATOR        );
                m_aData.sNew2OldSCPStandard.appendAscii ( "FrameLoader"             );
                m_aData.sNew2OldSCPStandard.append      ( CFG_ENCODING_OPEN         );
                m_aData.sNew2OldSCPStandard.append      ( sName                     );
                m_aData.sNew2OldSCPStandard.append      ( CFG_ENCODING_CLOSE        );
                m_aData.sNew2OldSCPStandard.appendAscii ( "\n"                      );
            }
        }

        for( nItem=0; nItem<nCount; ++nItem )
        {
            ::rtl::OUString sName = lNames[nItem]                           ;
            Loader          aItem = m_aData.pFilterCache->getLoader( sName );

            if( m_aData.nVersionOutput == 6 && m_aData.nVersionInput < 6 )
            {
                sName = lEncNames[nItem];
            }

            ++m_aData.nWrittenLoaders;

            // open set node by using name
            m_aData.sBufferStandard.appendAscii( "\t\t<default:group cfg:name=\""  );
            m_aData.sBufferStandard.append     ( sName                             );
            m_aData.sBufferStandard.appendAscii( "\">\n"                           );

            // write properties
            impl_generateUINamesProperty    ( m_aData.sBufferStandard, SUBKEY_UINAME, aItem.lUINames );
            impl_generateStringListProperty ( m_aData.sBufferStandard, SUBKEY_TYPES , aItem.lTypes   );

            // close set node
            m_aData.sBufferStandard.appendAscii( "\t\t</default:group>\n" );
        }

        // close set
        m_aData.sBufferStandard.appendAscii( "\t</schema:set>\n" );
    }
}

//*****************************************************************************************************************
void XCDGenerator::impl_generateDefaults()
{
    // open group
    m_aData.sBufferStandard.appendAscii( "\t<schema:group cfg:name=\"Defaults\">\n" );

    // write generic loader
    m_aData.sBufferStandard.appendAscii( "\t\t<schema:value cfg:name=\"FrameLoader\" cfg:type=\"string\">\n"   );
    m_aData.sBufferStandard.appendAscii( "\t\t\t<default:data>"                                                );
    m_aData.sBufferStandard.append     ( m_aData.pFilterCache->getDefaultLoader()                              );
    m_aData.sBufferStandard.appendAscii( "</default:data>\n"                                                   );
    m_aData.sBufferStandard.appendAscii( "\t\t</schema:value>\n"                                               );

    // write default detector
    m_aData.sBufferStandard.appendAscii( "\t\t<schema:value cfg:name=\"DetectService\" cfg:type=\"string\">\n" );
    m_aData.sBufferStandard.appendAscii( "\t\t\t<default:data>"                                                );
    m_aData.sBufferStandard.append     ( m_aData.pFilterCache->getDefaultDetector()                            );
    m_aData.sBufferStandard.appendAscii( "</default:data>\n"                                                   );
    m_aData.sBufferStandard.appendAscii( "\t\t</schema:value>\n"                                               );

    // close group
    m_aData.sBufferStandard.appendAscii( "\t</schema:group>\n" );
}

//*****************************************************************************************************************
void XCDGenerator::impl_generateContentHandlerSet()
{
    if( m_aData.pFilterCache->hasContentHandlers() == sal_False )
    {
        // write empty handler set!
        m_aData.sBufferStandard.appendAscii( "\t<schema:set cfg:name=\"ContentHandlers\" cfg:element-type=\"ContentHandler\"/>\n" );
    }
    else
    {
        // open set
        m_aData.sBufferStandard.appendAscii( "\t<schema:set cfg:name=\"ContentHandlers\" cfg:element-type=\"ContentHandler\">\n" );

        css::uno::Sequence< ::rtl::OUString > lNames = m_aData.pFilterCache->getAllContentHandlerNames();
        css::uno::Sequence< ::rtl::OUString > lEncNames ( lNames )                              ;
        sal_Int32                             nCount = lNames.getLength()                       ;
        sal_Int32                             nItem  = 0                                        ;

        XCDGenerator::impl_orderAlphabetical( lNames );

        if( m_aData.nVersionOutput == 6 && m_aData.nVersionInput < 6 )
        {
            ::rtl::OUString sName   ;
            ::rtl::OUString sEncName;
            for( nItem=0; nItem<nCount; ++nItem )
            {
                sName            = lNames[nItem]              ;
                lEncNames[nItem] = impl_encodeSetName( sName );

                m_aData.sNew2OldSCPStandard.appendAscii ( "org.openoffice.Office."  );
                m_aData.sNew2OldSCPStandard.append      ( m_aData.sPackageStandard  );
                m_aData.sNew2OldSCPStandard.append      ( CFG_PATH_SEPERATOR        );
                m_aData.sNew2OldSCPStandard.append      ( sName                     );
                m_aData.sNew2OldSCPStandard.appendAscii ( "\torg.openoffice.Office.");
                m_aData.sNew2OldSCPStandard.append      ( m_aData.sPackageStandard  );
                m_aData.sNew2OldSCPStandard.append      ( CFG_PATH_SEPERATOR        );
                m_aData.sNew2OldSCPStandard.appendAscii ( "ContentHandler"          );
                m_aData.sNew2OldSCPStandard.append      ( CFG_ENCODING_OPEN         );
                m_aData.sNew2OldSCPStandard.append      ( sName                     );
                m_aData.sNew2OldSCPStandard.append      ( CFG_ENCODING_CLOSE        );
                m_aData.sNew2OldSCPStandard.appendAscii ( "\n"                      );
            }
        }

        for( nItem=0; nItem<nCount; ++nItem )
        {
            ::rtl::OUString sName = lNames[nItem]                                   ;
            ContentHandler  aItem = m_aData.pFilterCache->getContentHandler( sName );

            if( m_aData.nVersionOutput == 6 && m_aData.nVersionInput < 6 )
            {
                sName = lEncNames[nItem];
            }

            ++m_aData.nWrittenContentHandlers;

            // open set node by using name
            m_aData.sBufferStandard.appendAscii( "\t\t<default:group cfg:name=\""  );
            m_aData.sBufferStandard.append     ( sName                             );
            m_aData.sBufferStandard.appendAscii( "\">\n"                           );

            // write properties
            impl_generateStringListProperty( m_aData.sBufferStandard, SUBKEY_TYPES, aItem.lTypes );

            // close set node
            m_aData.sBufferStandard.appendAscii( "\t\t</default:group>\n" );
        }

        // close set
        m_aData.sBufferStandard.appendAscii( "\t</schema:set>\n" );
    }
}

//*****************************************************************************************************************
void XCDGenerator::impl_generateIntProperty(        ::rtl::OUStringBuffer& sXCD        ,
                                            const   ::rtl::OUString&       sName       ,
                                                    sal_Int32              nValue      )
{
    sXCD.appendAscii( "\t\t\t<default:value cfg:name=\""                );
    sXCD.append     ( sName                                             );
    sXCD.appendAscii( "\" cfg:type=\"int\" cfg:writable=\""             );
    sXCD.appendAscii( m_aData.bWriteable==sal_True ? "true\">\n" : "false\">\n" );
    sXCD.appendAscii( "\t\t\t\t<default:data>"                          );
    sXCD.append     ( (sal_Int32)(nValue)                               );
    sXCD.appendAscii( "</default:data>\n\t\t\t</default:value>\n"       );
}

//*****************************************************************************************************************
void XCDGenerator::impl_generateBoolProperty(           ::rtl::OUStringBuffer& sXCD        ,
                                                const   ::rtl::OUString&       sName       ,
                                                        sal_Bool               bValue      )
{
    sXCD.appendAscii( "\t\t\t<default:value cfg:name=\""                );
    sXCD.append     ( sName                                             );
    sXCD.appendAscii( "\" cfg:type=\"boolean\" cfg:writable=\""         );
    sXCD.appendAscii( m_aData.bWriteable==sal_True ? "true\">\n" : "false\">\n" );
    sXCD.appendAscii( "\t\t\t\t<default:data>"                          );
    sXCD.appendAscii( bValue==sal_True ? "true" : "false"               );
    sXCD.appendAscii( "</default:data>\n\t\t\t</default:value>\n"       );
}

//*****************************************************************************************************************
void XCDGenerator::impl_generateStringProperty(         ::rtl::OUStringBuffer& sXCD        ,
                                                const   ::rtl::OUString&       sName       ,
                                                const   ::rtl::OUString&       sValue      )
{
    sXCD.appendAscii( "\t\t\t<default:value cfg:name=\""            );
    sXCD.append     ( sName                                         );
    sXCD.appendAscii( "\" cfg:type=\"string\" cfg:writable=\""      );
    sXCD.appendAscii( m_aData.bWriteable==sal_True ? "true\"" : "false\""   );
    if( sValue.getLength() > 0 )
    {
        sXCD.appendAscii( ">\n\t\t\t\t<default:data>"                       );
        sXCD.append     ( XCDGenerator::impl_encodeSpecialSigns( sValue )   );
        sXCD.appendAscii( "</default:data>\n\t\t\t</default:value>\n"       );
    }
    else
    {
        sXCD.appendAscii( "/>\n" );
    }
}

//*****************************************************************************************************************
void XCDGenerator::impl_generateStringListProperty(         ::rtl::OUStringBuffer&      sXCD        ,
                                                    const   ::rtl::OUString&            sName       ,
                                                    const   ::framework::StringList&    lValue      )
{
    sXCD.appendAscii( "\t\t\t<default:value cfg:name=\""                );
    sXCD.append     ( sName                                             );
    sXCD.appendAscii( "\" cfg:type=\"string\" cfg:derivedBy=\"list\""   );

    sal_Unicode cSeperator = XCDGenerator::impl_defineSeperator( lValue );
    if( cSeperator != ' ' )
    {
        sXCD.appendAscii( " cfg:separator=\""  );
        sXCD.append     ( cSeperator           );
        sXCD.appendAscii( "\""                 );
    }

    sXCD.appendAscii( " cfg:writable=\""                                );
    sXCD.appendAscii( m_aData.bWriteable==sal_True ? "true\"" : "false\""       );

    sal_Int32 nCount    = (sal_Int32)(lValue.size());
    sal_Int32 nPosition = 1;

    if( nCount > 0 )
    {
        sXCD.appendAscii( ">\n\t\t\t\t<default:data>"   );
        for( ConstStringListIterator pEntry=lValue.begin(); pEntry!=lValue.end(); ++pEntry )
        {
            sXCD.append( *pEntry );
            if( nPosition < nCount )
            {
                // Seperator for lists allowed only between two values!
                // Don't write leading or leaving seperators ...
                sXCD.append( cSeperator );
            }
            ++nPosition;
        }
        sXCD.appendAscii( "</default:data>\n\t\t\t</default:value>\n" );
    }
    else
    {
        sXCD.appendAscii( "/>\n" );
    }
}

//*****************************************************************************************************************
void XCDGenerator::impl_generateUINamesProperty(        ::rtl::OUStringBuffer&      sXCD        ,
                                                const   ::rtl::OUString&            sName       ,
                                                const   StringHash&                 lUINames    )
{
    sXCD.appendAscii( "\t\t\t<default:value cfg:name=\""                                );
    sXCD.append     ( sName                                                             );
    sXCD.appendAscii( "\" cfg:type=\"string\" cfg:localized=\"true\" cfg:writable=\""   );
    sXCD.appendAscii( m_aData.bWriteable==sal_True ? "true\"" : "false\""               );

    if( lUINames.size() > 0 )
    {
        sXCD.appendAscii( ">\n" );

        // Search for localized values, which doesn't need full localized set ...
        // because all values for all locales are the same!
        sal_Bool                   bDifferent  = sal_False       ;
        ConstStringHashIterator    pUIName     = lUINames.begin();
        ::rtl::OUString            sUIName     = pUIName->second ;
        while( pUIName!=lUINames.end() )
        {
            if( sUIName != pUIName->second )
            {
                bDifferent = sal_True;
                break;
            }
            ++pUIName;
        }

        // Generate full localized set, if some values are realy loclaized.
        if( bDifferent == sal_True )
        {
            for( ConstStringHashIterator pUIName=lUINames.begin(); pUIName!=lUINames.end(); ++pUIName )
            {
                sXCD.appendAscii( "\t\t\t\t<default:data xml:lang=\""                       );
                sXCD.append     ( pUIName->first                                            );
                sXCD.appendAscii( "\">"                                                     );
                sXCD.append     ( XCDGenerator::impl_encodeSpecialSigns( pUIName->second )  );
                sXCD.appendAscii( "</default:data>\n"                                       );
            }
        }
        // Generate ONE entry as default for our configuration if all localized values are equal!
        else
        {
            sXCD.appendAscii( "\t\t\t\t<default:data xml:lang=\""                                                   );
            sXCD.appendAscii( "en-US"                                                                               );
            sXCD.appendAscii( "\">"                                                                                 );
            sXCD.append     ( XCDGenerator::impl_encodeSpecialSigns( lUINames.find(DECLARE_ASCII("en-US"))->second ));
            sXCD.appendAscii( "</default:data>\n"                                                                   );
        }
        sXCD.appendAscii( "\t\t\t</default:value>\n" );
    }
    else
    {
        sXCD.appendAscii( "/>\n" );
    }
}

//*****************************************************************************************************************
::rtl::OUString XCDGenerator::impl_encodeSpecialSigns( const ::rtl::OUString& sValue )
{
    ::rtl::OUStringBuffer  sSource     ( sValue );
    ::rtl::OUStringBuffer  sDestination( 10000  );
    sal_Int32       nCount      = sValue.getLength();
    sal_Int32       i           = 0;

    for( i=0; i<nCount; ++i )
    {
        sal_Unicode cSign = sSource.charAt(i);
        switch( cSign )
        {
            // code &, ", ', <, > ...
            case '&' :  sDestination.appendAscii( "&amp;"   );
                        break;
            case '<' :  sDestination.appendAscii( "&lt;"    );
                        break;
            case '>' :  sDestination.appendAscii( "&gt;"    );
                        break;
            case '\'':  sDestination.appendAscii( "&rsquo;" );
                        break;
            case '\"':  sDestination.appendAscii( "&quot;"  );
                        break;
            // copy all other letters
            default :   sDestination.append( cSign );
                        break;
        }
    }

    return sDestination.makeStringAndClear();
}

//*****************************************************************************************************************
// Step over all elements of list to find one seperator, which isn't used for any value in list.
// We return an empty string if list contains no elements - because we must disable writing of
// "... cfg:seperator="<seperatorvalue> ..."
// => Otherwise we get a Sequence< OUString > with one empty element from configuration!!!
sal_Unicode XCDGenerator::impl_defineSeperator( const ::framework::StringList& lList )
{
    static cSeperator1 = ' ';
    static cSeperator2 = ';';
    static cSeperator3 = '+';
    static cSeperator4 = '-';
    static cSeperator5 = '*';

    // Start with first seperator.
    // Step over all list items.
    // If one item contains this seperator - try next one!
    // If no new one avaliable (5 tests failed!) - show an error message for user.
    // => File will be wrong then!
    // If seperator was changed start search during list again ... because
    // new seperator could exist at already compared elements!

    sal_Unicode             cSeperator = cSeperator1  ;
    sal_Bool                bOK        = sal_False    ;
    ConstStringListIterator pItem      = lList.begin();

    while( bOK == sal_False )
    {
        if( pItem == lList.end() )
        {
            bOK = sal_True;
        }
        else
        {
            while( pItem!=lList.end() )
            {
                if( pItem->indexOf( cSeperator, 0 ) != -1 )
                {
                    if( cSeperator == cSeperator1 )
                    {
                        cSeperator = cSeperator2;
                        pItem      = lList.begin();
                        break;
                    }
                    else
                    if( cSeperator == cSeperator2 )
                    {
                        cSeperator = cSeperator3;
                        pItem      = lList.begin();
                        break;
                    }
                    else
                    if( cSeperator == cSeperator3 )
                    {
                        cSeperator = cSeperator4;
                        pItem      = lList.begin();
                        break;
                    }
                    else
                    if( cSeperator == cSeperator4 )
                    {
                        cSeperator = cSeperator5;
                        pItem      = lList.begin();
                        break;
                    }
                    else
                    if( cSeperator == cSeperator5 )
                    {
                        LOG_ERROR( "XCDGenerator::impl_defineSeperator()", "Can't find seperator for given list! Generated XCD file will be wrong!" )
                        exit(-1);
                    }
                }
                else
                {
                    ++pItem;
                }
            }
        }
    }

    return cSeperator;
}

//*****************************************************************************************************************
void XCDGenerator::impl_initFilterHashNew2Old( StringHash& aHash )
{
    // key = new filter name, value = old name
    aHash[DECLARE_ASCII("writer_StarOffice_XML_Writer"                      )] =   DECLARE_ASCII("swriter: StarOffice XML (Writer)"                            );
    aHash[DECLARE_ASCII("writer_StarWriter_50"                              )] =   DECLARE_ASCII("swriter: StarWriter 5.0"                                     );
    aHash[DECLARE_ASCII("writer_StarWriter_50_VorlageTemplate"              )] =   DECLARE_ASCII("swriter: StarWriter 5.0 Vorlage/Template"                    );
    aHash[DECLARE_ASCII("writer_StarWriter_40"                              )] =   DECLARE_ASCII("swriter: StarWriter 4.0"                                     );
    aHash[DECLARE_ASCII("writer_StarWriter_40_VorlageTemplate"              )] =   DECLARE_ASCII("swriter: StarWriter 4.0 Vorlage/Template"                    );
    aHash[DECLARE_ASCII("writer_StarWriter_30"                              )] =   DECLARE_ASCII("swriter: StarWriter 3.0"                                     );
    aHash[DECLARE_ASCII("writer_StarWriter_30_VorlageTemplate"              )] =   DECLARE_ASCII("swriter: StarWriter 3.0 Vorlage/Template"                    );
    aHash[DECLARE_ASCII("writer_StarWriter_20"                              )] =   DECLARE_ASCII("swriter: StarWriter 2.0"                                     );
    aHash[DECLARE_ASCII("writer_StarWriter_10"                              )] =   DECLARE_ASCII("swriter: StarWriter 1.0"                                     );
    aHash[DECLARE_ASCII("writer_StarWriter_DOS"                             )] =   DECLARE_ASCII("swriter: StarWriter DOS"                                     );
    aHash[DECLARE_ASCII("writer_HTML_StarWriter"                            )] =   DECLARE_ASCII("swriter: HTML (StarWriter)"                                  );
    aHash[DECLARE_ASCII("writer_Text"                                       )] =   DECLARE_ASCII("swriter: Text"                                               );
    aHash[DECLARE_ASCII("writer_Text_Unix"                                  )] =   DECLARE_ASCII("swriter: Text Unix"                                          );
    aHash[DECLARE_ASCII("writer_Text_Mac"                                   )] =   DECLARE_ASCII("swriter: Text Mac"                                           );
    aHash[DECLARE_ASCII("writer_Text_DOS"                                   )] =   DECLARE_ASCII("swriter: Text DOS"                                           );
    aHash[DECLARE_ASCII("writer_Rich_Text_Format"                           )] =   DECLARE_ASCII("swriter: Rich Text Format"                                   );
    aHash[DECLARE_ASCII("writer_MS_Word_97"                                 )] =   DECLARE_ASCII("swriter: MS Word 97"                                         );
    aHash[DECLARE_ASCII("writer_MS_Word_95"                                 )] =   DECLARE_ASCII("swriter: MS Word 95"                                         );
    aHash[DECLARE_ASCII("writer_MS_Word_97_Vorlage"                         )] =   DECLARE_ASCII("swriter: MS Word 97 Vorlage"                                 );
    aHash[DECLARE_ASCII("writer_MS_Word_95_Vorlage"                         )] =   DECLARE_ASCII("swriter: MS Word 95 Vorlage"                                 );
    aHash[DECLARE_ASCII("writer_MS_WinWord_60"                              )] =   DECLARE_ASCII("swriter: MS WinWord 6.0"                                     );
    aHash[DECLARE_ASCII("writer_MS_Word_6x_W4W"                             )] =   DECLARE_ASCII("swriter: MS Word 6.x (W4W)"                                  );
    aHash[DECLARE_ASCII("writer_MS_WinWord_5"                               )] =   DECLARE_ASCII("swriter: MS WinWord 5"                                       );
    aHash[DECLARE_ASCII("writer_MS_WinWord_2x_W4W"                          )] =   DECLARE_ASCII("swriter: MS WinWord 2.x (W4W)"                               );
    aHash[DECLARE_ASCII("writer_MS_MacWord_5x_W4W"                          )] =   DECLARE_ASCII("swriter: MS MacWord 5.x (W4W)"                               );
    aHash[DECLARE_ASCII("writer_WordPerfect_Win_61_W4W"                     )] =   DECLARE_ASCII("swriter: WordPerfect (Win) 6.1 (W4W)"                        );
    aHash[DECLARE_ASCII("writer_WordPerfect_Win_70_W4W"                     )] =   DECLARE_ASCII("swriter: WordPerfect (Win) 7.0 (W4W)"                        );
    aHash[DECLARE_ASCII("writer_WordStar_Win_1x_20_W4W"                     )] =   DECLARE_ASCII("swriter: WordStar (Win) 1.x-2.0 (W4W)"                       );
    aHash[DECLARE_ASCII("writer_WordStar_70_W4W"                            )] =   DECLARE_ASCII("swriter: WordStar 7.0  (W4W)"                                );
    aHash[DECLARE_ASCII("writer_Ami_Pro_11_12_W4W"                          )] =   DECLARE_ASCII("swriter: Ami Pro 1.1-1.2 (W4W)"                              );
    aHash[DECLARE_ASCII("writer_Ami_Pro_20_31_W4W"                          )] =   DECLARE_ASCII("swriter: Ami Pro 2.0-3.1 (W4W)"                              );
    aHash[DECLARE_ASCII("writer_MS_Excel_40_StarWriter"                     )] =   DECLARE_ASCII("swriter: MS Excel 4.0 (StarWriter)"                          );
    aHash[DECLARE_ASCII("writer_MS_Excel_50_StarWriter"                     )] =   DECLARE_ASCII("swriter: MS Excel 5.0 (StarWriter)"                          );
    aHash[DECLARE_ASCII("writer_MS_Excel_95_StarWriter"                     )] =   DECLARE_ASCII("swriter: MS Excel 95 (StarWriter)"                           );
    aHash[DECLARE_ASCII("writer_MS_Works_20_DOS_W4W"                        )] =   DECLARE_ASCII("swriter: MS Works 2.0 DOS (W4W)"                             );
    aHash[DECLARE_ASCII("writer_MS_Works_30_Win_W4W"                        )] =   DECLARE_ASCII("swriter: MS Works 3.0 Win (W4W)"                             );
    aHash[DECLARE_ASCII("writer_Lotus_1_2_3_10_DOS_StarWriter"              )] =   DECLARE_ASCII("swriter: Lotus 1-2-3 1.0 (DOS) (StarWriter)"                 );
    aHash[DECLARE_ASCII("writer_Lotus_1_2_3_10_WIN_StarWriter"              )] =   DECLARE_ASCII("swriter: Lotus 1-2-3 1.0 (WIN) (StarWriter)"                 );
    aHash[DECLARE_ASCII("writer_Frame_Maker_MIF_50_W4W"                     )] =   DECLARE_ASCII("swriter: Frame Maker MIF 5.0 (W4W)"                          );
    aHash[DECLARE_ASCII("writer_Win_Write_3x_W4W"                           )] =   DECLARE_ASCII("swriter: Win Write 3.x (W4W)"                                );
    aHash[DECLARE_ASCII("writer_Text_encoded"                               )] =   DECLARE_ASCII("swriter: Text (encoded)"                                     );
    aHash[DECLARE_ASCII("writer_MS_WinWord_1x_W4W"                          )] =   DECLARE_ASCII("swriter: MS WinWord 1.x (W4W)"                               );
    aHash[DECLARE_ASCII("writer_MS_Word_5x_W4W"                             )] =   DECLARE_ASCII("swriter: MS Word 5.x (W4W)"                                  );
    aHash[DECLARE_ASCII("writer_MS_Word_4x_W4W"                             )] =   DECLARE_ASCII("swriter: MS Word 4.x (W4W)"                                  );
    aHash[DECLARE_ASCII("writer_MS_Word_3x_W4W"                             )] =   DECLARE_ASCII("swriter: MS Word 3.x (W4W)"                                  );
    aHash[DECLARE_ASCII("writer_MS_MacWord_40_W4W"                          )] =   DECLARE_ASCII("swriter: MS MacWord 4.0 (W4W)"                               );
    aHash[DECLARE_ASCII("writer_MS_MacWord_30_W4W"                          )] =   DECLARE_ASCII("swriter: MS MacWord 3.0 (W4W)"                               );
    aHash[DECLARE_ASCII("writer_WordPerfect_Mac_1_W4W"                      )] =   DECLARE_ASCII("swriter: WordPerfect Mac 1 (W4W)"                            );
    aHash[DECLARE_ASCII("writer_WordPerfect_Mac_2_W4W"                      )] =   DECLARE_ASCII("swriter: WordPerfect Mac 2 (W4W)"                            );
    aHash[DECLARE_ASCII("writer_WordPerfect_Mac_3_W4W"                      )] =   DECLARE_ASCII("swriter: WordPerfect Mac 3 (W4W)"                            );
    aHash[DECLARE_ASCII("writer_WordPerfect_Win_51_52_W4W"                  )] =   DECLARE_ASCII("swriter: WordPerfect (Win) 5.1-5.2 (W4W)"                    );
    aHash[DECLARE_ASCII("writer_WordPerfect_Win_60_W4W"                     )] =   DECLARE_ASCII("swriter: WordPerfect (Win) 6.0 (W4W)"                        );
    aHash[DECLARE_ASCII("writer_WordPerfect_41_W4W"                         )] =   DECLARE_ASCII("swriter: WordPerfect 4.1 (W4W)"                              );
    aHash[DECLARE_ASCII("writer_WordPerfect_42_W4W"                         )] =   DECLARE_ASCII("swriter: WordPerfect 4.2 (W4W)"                              );
    aHash[DECLARE_ASCII("writer_WordPerfect_50_W4W"                         )] =   DECLARE_ASCII("swriter: WordPerfect 5.0 (W4W)"                              );
    aHash[DECLARE_ASCII("writer_WordPerfect_51_W4W"                         )] =   DECLARE_ASCII("swriter: WordPerfect 5.1 (W4W)"                              );
    aHash[DECLARE_ASCII("writer_WordPerfect_60_W4W"                         )] =   DECLARE_ASCII("swriter: WordPerfect 6.0 (W4W)"                              );
    aHash[DECLARE_ASCII("writer_WordPerfect_61_W4W"                         )] =   DECLARE_ASCII("swriter: WordPerfect 6.1 (W4W)"                              );
    aHash[DECLARE_ASCII("writer_WordStar_2000_Rel_30_W4W"                   )] =   DECLARE_ASCII("swriter: WordStar 2000 Rel. 3.0 (W4W)"                       );
    aHash[DECLARE_ASCII("writer_WordStar_2000_Rel_35_W4W"                   )] =   DECLARE_ASCII("swriter: WordStar 2000 Rel. 3.5 (W4W)"                       );
    aHash[DECLARE_ASCII("writer_WordStar_33x_W4W"                           )] =   DECLARE_ASCII("swriter: WordStar 3.3x (W4W)"                                );
    aHash[DECLARE_ASCII("writer_WordStar_345_W4W"                           )] =   DECLARE_ASCII("swriter: WordStar 3.45 (W4W)"                                );
    aHash[DECLARE_ASCII("writer_WordStar_40_W4W"                            )] =   DECLARE_ASCII("swriter: WordStar 4.0  (W4W)"                                );
    aHash[DECLARE_ASCII("writer_WordStar_50_W4W"                            )] =   DECLARE_ASCII("swriter: WordStar 5.0  (W4W)"                                );
    aHash[DECLARE_ASCII("writer_WordStar_55_W4W"                            )] =   DECLARE_ASCII("swriter: WordStar 5.5  (W4W)"                                );
    aHash[DECLARE_ASCII("writer_WordStar_60_W4W"                            )] =   DECLARE_ASCII("swriter: WordStar 6.0  (W4W)"                                );
    aHash[DECLARE_ASCII("writer_MS_Works_40_Mac_W4W"                        )] =   DECLARE_ASCII("swriter: MS Works 4.0 Mac (W4W)"                             );
    aHash[DECLARE_ASCII("writer_Mac_Write_4x_50_W4W"                        )] =   DECLARE_ASCII("swriter: Mac Write 4.x 5.0 (W4W)"                            );
    aHash[DECLARE_ASCII("writer_Mac_Write_II_W4W"                           )] =   DECLARE_ASCII("swriter: Mac Write II (W4W)"                                 );
    aHash[DECLARE_ASCII("writer_Mac_Write_Pro_W4W"                          )] =   DECLARE_ASCII("swriter: Mac Write Pro (W4W)"                                );
    aHash[DECLARE_ASCII("writer_Lotus_Manuscript_W4W"                       )] =   DECLARE_ASCII("swriter: Lotus Manuscript (W4W)"                             );
    aHash[DECLARE_ASCII("writer_MASS_11_Rel_80_83_W4W"                      )] =   DECLARE_ASCII("swriter: MASS 11 Rel. 8.0-8.3 (W4W)"                         );
    aHash[DECLARE_ASCII("writer_MASS_11_Rel_85_90_W4W"                      )] =   DECLARE_ASCII("swriter: MASS 11 Rel. 8.5-9.0 (W4W)"                         );
    aHash[DECLARE_ASCII("writer_Claris_Works_W4W"                           )] =   DECLARE_ASCII("swriter: Claris Works (W4W)"                                 );
    aHash[DECLARE_ASCII("writer_CTOS_DEF_W4W"                               )] =   DECLARE_ASCII("swriter: CTOS DEF (W4W)"                                     );
    aHash[DECLARE_ASCII("writer_OfficeWriter_40_W4W"                        )] =   DECLARE_ASCII("swriter: OfficeWriter 4.0 (W4W)"                             );
    aHash[DECLARE_ASCII("writer_OfficeWriter_50_W4W"                        )] =   DECLARE_ASCII("swriter: OfficeWriter 5.0 (W4W)"                             );
    aHash[DECLARE_ASCII("writer_OfficeWriter_6x_W4W"                        )] =   DECLARE_ASCII("swriter: OfficeWriter 6.x (W4W)"                             );
    aHash[DECLARE_ASCII("writer_XyWrite_III_W4W"                            )] =   DECLARE_ASCII("swriter: XyWrite III ( W4W)"                                 );
    aHash[DECLARE_ASCII("writer_XyWrite_IIIP_W4W"                           )] =   DECLARE_ASCII("swriter: XyWrite III+ ( W4W)"                                );
    aHash[DECLARE_ASCII("writer_XyWrite_Signature_W4W"                      )] =   DECLARE_ASCII("swriter: XyWrite Signature (W4W)"                            );
    aHash[DECLARE_ASCII("writer_XyWrite_Sig_Win_W4W"                        )] =   DECLARE_ASCII("swriter: XyWrite Sig. (Win) (W4W)"                           );
    aHash[DECLARE_ASCII("writer_XyWrite_IV_W4W"                             )] =   DECLARE_ASCII("swriter: XyWrite IV (W4W)"                                   );
    aHash[DECLARE_ASCII("writer_XyWrite_Win_10_W4W"                         )] =   DECLARE_ASCII("swriter: XyWrite (Win) 1.0 (W4W)"                            );
    aHash[DECLARE_ASCII("writer_XEROX_XIF_50_W4W"                           )] =   DECLARE_ASCII("swriter: XEROX XIF 5.0 (W4W)"                                );
    aHash[DECLARE_ASCII("writer_XEROX_XIF_50_Illustrator_W4W"               )] =   DECLARE_ASCII("swriter: XEROX XIF 5.0 (Illustrator) (W4W)"                  );
    aHash[DECLARE_ASCII("writer_XEROX_XIF_60_Color_Bitmap_W4W"              )] =   DECLARE_ASCII("swriter: XEROX XIF 6.0 (Color Bitmap) (W4W)"                 );
    aHash[DECLARE_ASCII("writer_XEROX_XIF_60_Res_Graphic_W4W"               )] =   DECLARE_ASCII("swriter: XEROX XIF 6.0 (Res Graphic) (W4W)"                  );
    aHash[DECLARE_ASCII("writer_WriteNow_30_Macintosh_W4W"                  )] =   DECLARE_ASCII("swriter: WriteNow 3.0 (Macintosh) (W4W)"                     );
    aHash[DECLARE_ASCII("writer_Writing_Assistant_W4W"                      )] =   DECLARE_ASCII("swriter: Writing Assistant (W4W)"                            );
    aHash[DECLARE_ASCII("writer_VolksWriter_Deluxe_W4W"                     )] =   DECLARE_ASCII("swriter: VolksWriter Deluxe (W4W)"                           );
    aHash[DECLARE_ASCII("writer_VolksWriter_3_and_4_W4W"                    )] =   DECLARE_ASCII("swriter: VolksWriter 3 and 4 (W4W)"                          );
    aHash[DECLARE_ASCII("writer_MultiMate_33_W4W"                           )] =   DECLARE_ASCII("swriter: MultiMate 3.3 (W4W)"                                );
    aHash[DECLARE_ASCII("writer_MultiMate_Adv_36_W4W"                       )] =   DECLARE_ASCII("swriter: MultiMate Adv. 3.6 (W4W)"                           );
    aHash[DECLARE_ASCII("writer_MultiMate_Adv_II_37_W4W"                    )] =   DECLARE_ASCII("swriter: MultiMate Adv. II 3.7 (W4W)"                        );
    aHash[DECLARE_ASCII("writer_MultiMate_4_W4W"                            )] =   DECLARE_ASCII("swriter: MultiMate 4 (W4W)"                                  );
    aHash[DECLARE_ASCII("writer_NAVY_DIF_W4W"                               )] =   DECLARE_ASCII("swriter: NAVY DIF (W4W)"                                     );
    aHash[DECLARE_ASCII("writer_PFS_Write_W4W"                              )] =   DECLARE_ASCII("swriter: PFS Write (W4W)"                                    );
    aHash[DECLARE_ASCII("writer_PFS_First_Choice_10_W4W"                    )] =   DECLARE_ASCII("swriter: PFS First Choice 1.0 (W4W)"                         );
    aHash[DECLARE_ASCII("writer_PFS_First_Choice_20_W4W"                    )] =   DECLARE_ASCII("swriter: PFS First Choice 2.0 (W4W)"                         );
    aHash[DECLARE_ASCII("writer_PFS_First_Choice_30_W4W"                    )] =   DECLARE_ASCII("swriter: PFS First Choice 3.0 (W4W)"                         );
    aHash[DECLARE_ASCII("writer_Professional_Write_10_W4W"                  )] =   DECLARE_ASCII("swriter: Professional Write 1.0 (W4W)"                       );
    aHash[DECLARE_ASCII("writer_Professional_Write_2x_W4W"                  )] =   DECLARE_ASCII("swriter: Professional Write 2.x (W4W)"                       );
    aHash[DECLARE_ASCII("writer_Professional_Write_Plus_W4W"                )] =   DECLARE_ASCII("swriter: Professional Write Plus (W4W)"                      );
    aHash[DECLARE_ASCII("writer_Peach_Text_W4W"                             )] =   DECLARE_ASCII("swriter: Peach Text (W4W)"                                   );
    aHash[DECLARE_ASCII("writer_DCA_Revisable_Form_Text_W4W"                )] =   DECLARE_ASCII("swriter: DCA Revisable Form Text (W4W)"                      );
    aHash[DECLARE_ASCII("writer_DCA_with_Display_Write_5_W4W"               )] =   DECLARE_ASCII("swriter: DCA with Display Write 5 (W4W)"                     );
    aHash[DECLARE_ASCII("writer_DCAFFT_Final_Form_Text_W4W"                 )] =   DECLARE_ASCII("swriter: DCA/FFT-Final Form Text (W4W)"                      );
    aHash[DECLARE_ASCII("writer_DEC_DX_W4W"                                 )] =   DECLARE_ASCII("swriter: DEC DX (W4W)"                                       );
    aHash[DECLARE_ASCII("writer_DEC_WPS_PLUS_W4W"                           )] =   DECLARE_ASCII("swriter: DEC WPS-PLUS (W4W)"                                 );
    aHash[DECLARE_ASCII("writer_DisplayWrite_20_4x_W4W"                     )] =   DECLARE_ASCII("swriter: DisplayWrite 2.0-4.x (W4W)"                         );
    aHash[DECLARE_ASCII("writer_DisplayWrite_5x_W4W"                        )] =   DECLARE_ASCII("swriter: DisplayWrite 5.x (W4W)"                             );
    aHash[DECLARE_ASCII("writer_DataGeneral_CEO_Write_W4W"                  )] =   DECLARE_ASCII("swriter: DataGeneral CEO Write (W4W)"                        );
    aHash[DECLARE_ASCII("writer_EBCDIC_W4W"                                 )] =   DECLARE_ASCII("swriter: EBCDIC (W4W)"                                       );
    aHash[DECLARE_ASCII("writer_Enable_W4W"                                 )] =   DECLARE_ASCII("swriter: Enable (W4W)"                                       );
    aHash[DECLARE_ASCII("writer_Frame_Maker_MIF_30_W4W"                     )] =   DECLARE_ASCII("swriter: Frame Maker MIF 3.0 (W4W)"                          );
    aHash[DECLARE_ASCII("writer_Frame_Maker_MIF_40_W4W"                     )] =   DECLARE_ASCII("swriter: Frame Maker MIF 4.0 (W4W)"                          );
    aHash[DECLARE_ASCII("writer_Frame_Work_III_W4W"                         )] =   DECLARE_ASCII("swriter: Frame Work III (W4W)"                               );
    aHash[DECLARE_ASCII("writer_Frame_Work_IV_W4W"                          )] =   DECLARE_ASCII("swriter: Frame Work IV  (W4W)"                               );
    aHash[DECLARE_ASCII("writer_HP_AdvanceWrite_Plus_W4W"                   )] =   DECLARE_ASCII("swriter: HP AdvanceWrite Plus (W4W)"                         );
    aHash[DECLARE_ASCII("writer_ICL_Office_Power_6_W4W"                     )] =   DECLARE_ASCII("swriter: ICL Office Power 6 (W4W)"                           );
    aHash[DECLARE_ASCII("writer_ICL_Office_Power_7_W4W"                     )] =   DECLARE_ASCII("swriter: ICL Office Power 7 (W4W)"                           );
    aHash[DECLARE_ASCII("writer_Interleaf_W4W"                              )] =   DECLARE_ASCII("swriter: Interleaf (W4W)"                                    );
    aHash[DECLARE_ASCII("writer_Interleaf_5_6_W4W"                          )] =   DECLARE_ASCII("swriter: Interleaf 5 - 6 (W4W)"                              );
    aHash[DECLARE_ASCII("writer_Legacy_Winstar_onGO_W4W"                    )] =   DECLARE_ASCII("swriter: Legacy Winstar onGO (W4W)"                          );
    aHash[DECLARE_ASCII("writer_QA_Write_10_30_W4W"                         )] =   DECLARE_ASCII("swriter: Q&A Write 1.0-3.0 (W4W)"                            );
    aHash[DECLARE_ASCII("writer_QA_Write_40_W4W"                            )] =   DECLARE_ASCII("swriter: Q&A Write 4.0 (W4W)"                                );
    aHash[DECLARE_ASCII("writer_Rapid_File_10_W4W"                          )] =   DECLARE_ASCII("swriter: Rapid File 1.0 (W4W)"                               );
    aHash[DECLARE_ASCII("writer_Rapid_File_12_W4W"                          )] =   DECLARE_ASCII("swriter: Rapid File 1.2 (W4W)"                               );
    aHash[DECLARE_ASCII("writer_Samna_Word_IV_IV_Plus_W4W"                  )] =   DECLARE_ASCII("swriter: Samna Word IV-IV Plus (W4W)"                        );
    aHash[DECLARE_ASCII("writer_Total_Word_W4W"                             )] =   DECLARE_ASCII("swriter: Total Word (W4W)"                                   );
    aHash[DECLARE_ASCII("writer_Uniplex_onGO_W4W"                           )] =   DECLARE_ASCII("swriter: Uniplex onGO (W4W)"                                 );
    aHash[DECLARE_ASCII("writer_Uniplex_V7_V8_W4W"                          )] =   DECLARE_ASCII("swriter: Uniplex V7-V8 (W4W)"                                );
    aHash[DECLARE_ASCII("writer_Wang_PC_W4W"                                )] =   DECLARE_ASCII("swriter: Wang PC (W4W)"                                      );
    aHash[DECLARE_ASCII("writer_Wang_II_SWP_W4W"                            )] =   DECLARE_ASCII("swriter: Wang II SWP (W4W)"                                  );
    aHash[DECLARE_ASCII("writer_Wang_WP_Plus_W4W"                           )] =   DECLARE_ASCII("swriter: Wang WP Plus (W4W)"                                 );
    aHash[DECLARE_ASCII("writer_WITA_W4W"                                   )] =   DECLARE_ASCII("swriter: WITA (W4W)"                                         );
    aHash[DECLARE_ASCII("writer_WiziWord_30_W4W"                            )] =   DECLARE_ASCII("swriter: WiziWord 3.0 (W4W)"                                 );
    aHash[DECLARE_ASCII("writer_web_HTML"                                   )] =   DECLARE_ASCII("swriter/web: HTML"                                           );
    aHash[DECLARE_ASCII("writer_web_StarWriterWeb_50_VorlageTemplate"       )] =   DECLARE_ASCII("swriter/web: StarWriter/Web 5.0 Vorlage/Template"            );
    aHash[DECLARE_ASCII("writer_web_StarWriterWeb_40_VorlageTemplate"       )] =   DECLARE_ASCII("swriter/web: StarWriter/Web 4.0 Vorlage/Template"            );
    aHash[DECLARE_ASCII("writer_web_Text_StarWriterWeb"                     )] =   DECLARE_ASCII("swriter/web: Text (StarWriter/Web)"                          );
    aHash[DECLARE_ASCII("writer_web_Text_DOS_StarWriterWeb"                 )] =   DECLARE_ASCII("swriter/web: Text DOS (StarWriter/Web)"                      );
    aHash[DECLARE_ASCII("writer_web_Text_Mac_StarWriterWeb"                 )] =   DECLARE_ASCII("swriter/web: Text Mac (StarWriter/Web)"                      );
    aHash[DECLARE_ASCII("writer_web_Text_Unix_StarWriterWeb"                )] =   DECLARE_ASCII("swriter/web: Text Unix (StarWriter/Web)"                     );
    aHash[DECLARE_ASCII("writer_web_StarWriter_50"                          )] =   DECLARE_ASCII("swriter/web: StarWriter 5.0"                                 );
    aHash[DECLARE_ASCII("writer_web_StarWriter_40"                          )] =   DECLARE_ASCII("swriter/web: StarWriter 4.0"                                 );
    aHash[DECLARE_ASCII("writer_web_StarWriter_30"                          )] =   DECLARE_ASCII("swriter/web: StarWriter 3.0"                                 );
    aHash[DECLARE_ASCII("writer_web_Text_encoded"                           )] =   DECLARE_ASCII("swriter/web: Text (encoded)"                                 );
    aHash[DECLARE_ASCII("writer_globaldocument_StarWriter_60GlobalDocument" )] =   DECLARE_ASCII("swriter/GlobalDocument: StarOffice XML (GlobalDocument)"     );
    aHash[DECLARE_ASCII("writer_globaldocument_StarWriter_50GlobalDocument" )] =   DECLARE_ASCII("swriter/GlobalDocument: StarWriter 5.0/GlobalDocument"       );
    aHash[DECLARE_ASCII("writer_globaldocument_StarWriter_40GlobalDocument" )] =   DECLARE_ASCII("swriter/GlobalDocument: StarWriter 4.0/GlobalDocument"       );
    aHash[DECLARE_ASCII("writer_globaldocument_StarWriter_50"               )] =   DECLARE_ASCII("swriter/GlobalDocument: StarWriter 5.0"                      );
    aHash[DECLARE_ASCII("writer_globaldocument_StarWriter_40"               )] =   DECLARE_ASCII("swriter/GlobalDocument: StarWriter 4.0"                      );
    aHash[DECLARE_ASCII("writer_globaldocument_StarWriter_30"               )] =   DECLARE_ASCII("swriter/GlobalDocument: StarWriter 3.0"                      );
    aHash[DECLARE_ASCII("writer_globaldocument_Text_encoded"                )] =   DECLARE_ASCII("swriter/GlobalDocument: Text (encoded)"                      );
    aHash[DECLARE_ASCII("chart_StarOffice_XML_Chart"                        )] =   DECLARE_ASCII("schart: StarOffice XML (Chart)"                              );
    aHash[DECLARE_ASCII("chart_StarChart_50"                                )] =   DECLARE_ASCII("schart: StarChart 5.0"                                       );
    aHash[DECLARE_ASCII("chart_StarChart_40"                                )] =   DECLARE_ASCII("schart: StarChart 4.0"                                       );
    aHash[DECLARE_ASCII("chart_StarChart_30"                                )] =   DECLARE_ASCII("schart: StarChart 3.0"                                       );
    aHash[DECLARE_ASCII("calc_StarOffice_XML_Calc"                          )] =   DECLARE_ASCII("scalc: StarOffice XML (Calc)"                                );
    aHash[DECLARE_ASCII("calc_StarCalc_50"                                  )] =   DECLARE_ASCII("scalc: StarCalc 5.0"                                         );
    aHash[DECLARE_ASCII("calc_StarCalc_50_VorlageTemplate"                  )] =   DECLARE_ASCII("scalc: StarCalc 5.0 Vorlage/Template"                        );
    aHash[DECLARE_ASCII("calc_StarCalc_40"                                  )] =   DECLARE_ASCII("scalc: StarCalc 4.0"                                         );
    aHash[DECLARE_ASCII("calc_StarCalc_40_VorlageTemplate"                  )] =   DECLARE_ASCII("scalc: StarCalc 4.0 Vorlage/Template"                        );
    aHash[DECLARE_ASCII("calc_StarCalc_30"                                  )] =   DECLARE_ASCII("scalc: StarCalc 3.0"                                         );
    aHash[DECLARE_ASCII("calc_StarCalc_30_VorlageTemplate"                  )] =   DECLARE_ASCII("scalc: StarCalc 3.0 Vorlage/Template"                        );
    aHash[DECLARE_ASCII("calc_MS_Excel_97"                                  )] =   DECLARE_ASCII("scalc: MS Excel 97"                                          );
    aHash[DECLARE_ASCII("calc_MS_Excel_97_VorlageTemplate"                  )] =   DECLARE_ASCII("scalc: MS Excel 97 Vorlage/Template"                         );
    aHash[DECLARE_ASCII("calc_MS_Excel_95"                                  )] =   DECLARE_ASCII("scalc: MS Excel 95"                                          );
    aHash[DECLARE_ASCII("calc_MS_Excel_95_VorlageTemplate"                  )] =   DECLARE_ASCII("scalc: MS Excel 95 Vorlage/Template"                         );
    aHash[DECLARE_ASCII("calc_MS_Excel_5095"                                )] =   DECLARE_ASCII("scalc: MS Excel 5.0/95"                                      );
    aHash[DECLARE_ASCII("calc_MS_Excel_5095_VorlageTemplate"                )] =   DECLARE_ASCII("scalc: MS Excel 5.0/95 Vorlage/Template"                     );
    aHash[DECLARE_ASCII("calc_MS_Excel_40"                                  )] =   DECLARE_ASCII("scalc: MS Excel 4.0"                                         );
    aHash[DECLARE_ASCII("calc_MS_Excel_40_VorlageTemplate"                  )] =   DECLARE_ASCII("scalc: MS Excel 4.0 Vorlage/Template"                        );
    aHash[DECLARE_ASCII("calc_Rich_Text_Format_StarCalc"                    )] =   DECLARE_ASCII("scalc: Rich Text Format (StarCalc)"                          );
    aHash[DECLARE_ASCII("calc_SYLK"                                         )] =   DECLARE_ASCII("scalc: SYLK"                                                 );
    aHash[DECLARE_ASCII("calc_DIF"                                          )] =   DECLARE_ASCII("scalc: DIF"                                                  );
    aHash[DECLARE_ASCII("calc_HTML_StarCalc"                                )] =   DECLARE_ASCII("scalc: HTML (StarCalc)"                                      );
    aHash[DECLARE_ASCII("calc_dBase"                                        )] =   DECLARE_ASCII("scalc: dBase"                                                );
    aHash[DECLARE_ASCII("calc_Lotus"                                        )] =   DECLARE_ASCII("scalc: Lotus"                                                );
    aHash[DECLARE_ASCII("calc_StarCalc_10"                                  )] =   DECLARE_ASCII("scalc: StarCalc 1.0"                                         );
    aHash[DECLARE_ASCII("calc_Text_txt_csv_StarCalc"                        )] =   DECLARE_ASCII("scalc: Text - txt - csv (StarCalc)"                          );
    aHash[DECLARE_ASCII("impress_StarOffice_XML_Impress"                    )] =   DECLARE_ASCII("simpress: StarOffice XML (Impress)"                          );
    aHash[DECLARE_ASCII("impress_StarImpress_50"                            )] =   DECLARE_ASCII("simpress: StarImpress 5.0"                                   );
    aHash[DECLARE_ASCII("impress_StarImpress_50_Vorlage"                    )] =   DECLARE_ASCII("simpress: StarImpress 5.0 Vorlage"                           );
    aHash[DECLARE_ASCII("impress_StarImpress_40"                            )] =   DECLARE_ASCII("simpress: StarImpress 4.0"                                   );
    aHash[DECLARE_ASCII("impress_StarImpress_40_Vorlage"                    )] =   DECLARE_ASCII("simpress: StarImpress 4.0 Vorlage"                           );
    aHash[DECLARE_ASCII("impress_StarDraw_50_StarImpress"                   )] =   DECLARE_ASCII("simpress: StarDraw 5.0 (StarImpress)"                        );
    aHash[DECLARE_ASCII("impress_StarDraw_50_Vorlage_StarImpress"           )] =   DECLARE_ASCII("simpress: StarDraw 5.0 Vorlage (StarImpress)"                );
    aHash[DECLARE_ASCII("impress_StarDraw_30_StarImpress"                   )] =   DECLARE_ASCII("simpress: StarDraw 3.0 (StarImpress)"                        );
    aHash[DECLARE_ASCII("impress_StarDraw_30_Vorlage_StarImpress"           )] =   DECLARE_ASCII("simpress: StarDraw 3.0 Vorlage (StarImpress)"                );
    aHash[DECLARE_ASCII("impress_MS_PowerPoint_97"                          )] =   DECLARE_ASCII("simpress: MS PowerPoint 97"                                  );
    aHash[DECLARE_ASCII("impress_MS_PowerPoint_97_Vorlage"                  )] =   DECLARE_ASCII("simpress: MS PowerPoint 97 Vorlage"                          );
    aHash[DECLARE_ASCII("impress_CGM_Computer_Graphics_Metafile"            )] =   DECLARE_ASCII("simpress: CGM - Computer Graphics Metafile"                  );
    aHash[DECLARE_ASCII("impress_StarImpress_50_packed"                     )] =   DECLARE_ASCII("simpress: StarImpress 5.0 (packed)"                          );
    aHash[DECLARE_ASCII("draw_StarOffice_XML_Draw"                          )] =   DECLARE_ASCII("sdraw: StarOffice XML (Draw)"                                );
    aHash[DECLARE_ASCII("draw_GIF_Graphics_Interchange"                     )] =   DECLARE_ASCII("sdraw: GIF - Graphics Interchange"                           );
    aHash[DECLARE_ASCII("draw_PCD_Photo_CD"                                 )] =   DECLARE_ASCII("sdraw: PCD - Photo CD"                                       );
    aHash[DECLARE_ASCII("draw_PCX_Zsoft_Paintbrush"                         )] =   DECLARE_ASCII("sdraw: PCX - Zsoft Paintbrush"                               );
    aHash[DECLARE_ASCII("draw_PSD_Adobe_Photoshop"                          )] =   DECLARE_ASCII("sdraw: PSD - Adobe Photoshop"                                );
    aHash[DECLARE_ASCII("draw_PNG_Portable_Network_Graphic"                 )] =   DECLARE_ASCII("sdraw: PNG - Portable Network Graphic"                       );
    aHash[DECLARE_ASCII("draw_StarDraw_50"                                  )] =   DECLARE_ASCII("sdraw: StarDraw 5.0"                                         );
    aHash[DECLARE_ASCII("draw_PBM_Portable_Bitmap"                          )] =   DECLARE_ASCII("sdraw: PBM - Portable Bitmap"                                );
    aHash[DECLARE_ASCII("draw_PGM_Portable_Graymap"                         )] =   DECLARE_ASCII("sdraw: PGM - Portable Graymap"                               );
    aHash[DECLARE_ASCII("draw_PPM_Portable_Pixelmap"                        )] =   DECLARE_ASCII("sdraw: PPM - Portable Pixelmap"                              );
    aHash[DECLARE_ASCII("draw_RAS_Sun_Rasterfile"                           )] =   DECLARE_ASCII("sdraw: RAS - Sun Rasterfile"                                 );
    aHash[DECLARE_ASCII("draw_TGA_Truevision_TARGA"                         )] =   DECLARE_ASCII("sdraw: TGA - Truevision TARGA"                               );
    aHash[DECLARE_ASCII("draw_SGV_StarDraw_20"                              )] =   DECLARE_ASCII("sdraw: SGV - StarDraw 2.0"                                   );
    aHash[DECLARE_ASCII("draw_TIF_Tag_Image_File"                           )] =   DECLARE_ASCII("sdraw: TIF - Tag Image File"                                 );
    aHash[DECLARE_ASCII("draw_SGF_StarOffice_Writer_SGF"                    )] =   DECLARE_ASCII("sdraw: SGF - StarOffice Writer SGF"                          );
    aHash[DECLARE_ASCII("draw_XPM"                                          )] =   DECLARE_ASCII("sdraw: XPM"                                                  );
    aHash[DECLARE_ASCII("gif_Graphics_Interchange"                          )] =   DECLARE_ASCII("sdraw: GIF - Graphics Interchange"                           );
    aHash[DECLARE_ASCII("pcd_Photo_CD"                                      )] =   DECLARE_ASCII("sdraw: PCD - Photo CD"                                       );
    aHash[DECLARE_ASCII("pcx_Zsoft_Paintbrush"                              )] =   DECLARE_ASCII("sdraw: PCX - Zsoft Paintbrush"                               );
    aHash[DECLARE_ASCII("psd_Adobe_Photoshop"                               )] =   DECLARE_ASCII("sdraw: PSD - Adobe Photoshop"                                );
    aHash[DECLARE_ASCII("png_Portable_Network_Graphic"                      )] =   DECLARE_ASCII("sdraw: PNG - Portable Network Graphic"                       );
    aHash[DECLARE_ASCII("pbm_Portable_Bitmap"                               )] =   DECLARE_ASCII("sdraw: PBM - Portable Bitmap"                                );
    aHash[DECLARE_ASCII("pgm_Portable_Graymap"                              )] =   DECLARE_ASCII("sdraw: PGM - Portable Graymap"                               );
    aHash[DECLARE_ASCII("ppm_Portable_Pixelmap"                             )] =   DECLARE_ASCII("sdraw: PPM - Portable Pixelmap"                              );
    aHash[DECLARE_ASCII("ras_Sun_Rasterfile"                                )] =   DECLARE_ASCII("sdraw: RAS - Sun Rasterfile"                                 );
    aHash[DECLARE_ASCII("tga_Truevision_TARGA"                              )] =   DECLARE_ASCII("sdraw: TGA - Truevision TARGA"                               );
    aHash[DECLARE_ASCII("sgv_StarDraw_20"                                   )] =   DECLARE_ASCII("sdraw: SGV - StarDraw 2.0"                                   );
    aHash[DECLARE_ASCII("tif_Tag_Image_File"                                )] =   DECLARE_ASCII("sdraw: TIF - Tag Image File"                                 );
    aHash[DECLARE_ASCII("sgf_StarOffice_Writer_SGF"                         )] =   DECLARE_ASCII("sdraw: SGF - StarOffice Writer SGF"                          );
    aHash[DECLARE_ASCII("xpm_XPM"                                           )] =   DECLARE_ASCII("sdraw: XPM"                                                  );
    aHash[DECLARE_ASCII("draw_StarDraw_50_Vorlage"                          )] =   DECLARE_ASCII("sdraw: StarDraw 5.0 Vorlage"                                 );
    aHash[DECLARE_ASCII("draw_StarImpress_50_StarDraw"                      )] =   DECLARE_ASCII("sdraw: StarImpress 5.0 (StarDraw)"                           );
    aHash[DECLARE_ASCII("draw_StarImpress_50_Vorlage_StarDraw"              )] =   DECLARE_ASCII("sdraw: StarImpress 5.0 Vorlage (StarDraw)"                   );
    aHash[DECLARE_ASCII("draw_StarImpress_40_StarDraw"                      )] =   DECLARE_ASCII("sdraw: StarImpress 4.0 (StarDraw)"                           );
    aHash[DECLARE_ASCII("draw_StarImpress_40_Vorlage_StarDraw"              )] =   DECLARE_ASCII("sdraw: StarImpress 4.0 Vorlage (StarDraw)"                   );
    aHash[DECLARE_ASCII("draw_StarDraw_30"                                  )] =   DECLARE_ASCII("sdraw: StarDraw 3.0"                                         );
    aHash[DECLARE_ASCII("draw_StarDraw_30_Vorlage"                          )] =   DECLARE_ASCII("sdraw: StarDraw 3.0 Vorlage"                                 );
    aHash[DECLARE_ASCII("draw_EMF_MS_Windows_Metafile"                      )] =   DECLARE_ASCII("sdraw: EMF - MS Windows Metafile"                            );
    aHash[DECLARE_ASCII("draw_MET_OS2_Metafile"                             )] =   DECLARE_ASCII("sdraw: MET - OS/2 Metafile"                                  );
    aHash[DECLARE_ASCII("draw_DXF_AutoCAD_Interchange"                      )] =   DECLARE_ASCII("sdraw: DXF - AutoCAD Interchange"                            );
    aHash[DECLARE_ASCII("draw_EPS_Encapsulated_PostScript"                  )] =   DECLARE_ASCII("sdraw: EPS - Encapsulated PostScript"                        );
    aHash[DECLARE_ASCII("draw_WMF_MS_Windows_Metafile"                      )] =   DECLARE_ASCII("sdraw: WMF - MS Windows Metafile"                            );
    aHash[DECLARE_ASCII("draw_PCT_Mac_Pict"                                 )] =   DECLARE_ASCII("sdraw: PCT - Mac Pict"                                       );
    aHash[DECLARE_ASCII("draw_SVM_StarView_Metafile"                        )] =   DECLARE_ASCII("sdraw: SVM - StarView Metafile"                              );
    aHash[DECLARE_ASCII("draw_BMP_MS_Windows"                               )] =   DECLARE_ASCII("sdraw: BMP - MS Windows"                                     );
    aHash[DECLARE_ASCII("draw_JPG_JPEG"                                     )] =   DECLARE_ASCII("sdraw: JPG - JPEG"                                           );
    aHash[DECLARE_ASCII("draw_XBM_X_Consortium"                             )] =   DECLARE_ASCII("sdraw: XBM - X-Consortium"                                   );
    aHash[DECLARE_ASCII("emf_MS_Windows_Metafile"                           )] =   DECLARE_ASCII("sdraw: EMF - MS Windows Metafile"                            );
    aHash[DECLARE_ASCII("met_OS2_Metafile"                                  )] =   DECLARE_ASCII("sdraw: MET - OS/2 Metafile"                                  );
    aHash[DECLARE_ASCII("dxf_AutoCAD_Interchange"                           )] =   DECLARE_ASCII("sdraw: DXF - AutoCAD Interchange"                            );
    aHash[DECLARE_ASCII("eps_Encapsulated_PostScript"                       )] =   DECLARE_ASCII("sdraw: EPS - Encapsulated PostScript"                        );
    aHash[DECLARE_ASCII("wmf_MS_Windows_Metafile"                           )] =   DECLARE_ASCII("sdraw: WMF - MS Windows Metafile"                            );
    aHash[DECLARE_ASCII("pct_Mac_Pict"                                      )] =   DECLARE_ASCII("sdraw: PCT - Mac Pict"                                       );
    aHash[DECLARE_ASCII("svm_StarView_Metafile"                             )] =   DECLARE_ASCII("sdraw: SVM - StarView Metafile"                              );
    aHash[DECLARE_ASCII("bmp_MS_Windows"                                    )] =   DECLARE_ASCII("sdraw: BMP - MS Windows"                                     );
    aHash[DECLARE_ASCII("jpg_JPEG"                                          )] =   DECLARE_ASCII("sdraw: JPG - JPEG"                                           );
    aHash[DECLARE_ASCII("xbm_X_Consortium"                                  )] =   DECLARE_ASCII("sdraw: XBM - X-Consortium"                                   );
    aHash[DECLARE_ASCII("math_StarOffice_XML_Math"                          )] =   DECLARE_ASCII("smath: StarOffice XML (Math)"                                );
    aHash[DECLARE_ASCII("math_MathML_XML_Math"                              )] =   DECLARE_ASCII("smath: MathML XML (Math)"                                    );
    aHash[DECLARE_ASCII("math_StarMath_50"                                  )] =   DECLARE_ASCII("smath: StarMath 5.0"                                         );
    aHash[DECLARE_ASCII("math_StarMath_40"                                  )] =   DECLARE_ASCII("smath: StarMath 4.0"                                         );
    aHash[DECLARE_ASCII("math_StarMath_30"                                  )] =   DECLARE_ASCII("smath: StarMath 3.0"                                         );
    aHash[DECLARE_ASCII("math_StarMath_20"                                  )] =   DECLARE_ASCII("smath: StarMath 2.0"                                         );
    aHash[DECLARE_ASCII("math_MathType_3x"                                  )] =   DECLARE_ASCII("smath: MathType 3.x"                                         );
}

//*****************************************************************************************************************
::rtl::OUString XCDGenerator::impl_getOldFilterName( const ::rtl::OUString& sNewName )
{
    ::rtl::OUString sOldName;
    ConstStringHashIterator pEntry = m_aData.aOldFilterNamesHash.find(sNewName);
    if( pEntry==m_aData.aOldFilterNamesHash.end() )
    {
        sOldName = sNewName;
    }
    else
    {
        sOldName = m_aData.aOldFilterNamesHash[sNewName];
    }
    return sOldName;
}

//*****************************************************************************************************************
void XCDGenerator::impl_classifyType( const AppMember& rData, const ::rtl::OUString& sTypeName, EFilterPackage& ePackage )
{
    ePackage = E_STANDARD;

    // Step over all registered filters for this type ...
    // Classify all of these filters. If one of them a standard filter ...
    // type must be a standard type too - otherwise not!

    CheckedStringListIterator   pIterator   ;
    ::rtl::OUString             sFilterName ;
    sal_Int32                   nOrder      ;
    while( rData.pFilterCache->searchFilterForType( sTypeName, pIterator, sFilterName ) == sal_True )
    {
        EFilterPackage eFilterPackage;
        XCDGenerator::impl_classifyFilter( rData, sFilterName, eFilterPackage, nOrder );
        if( eFilterPackage == E_STANDARD )
        {
            ePackage = E_STANDARD;
            break;
        }
    }
}

//*****************************************************************************************************************
void XCDGenerator::impl_classifyFilter( const AppMember& rData, const ::rtl::OUString& sFilterName, EFilterPackage& ePackage, sal_Int32& nOrder )
{
    // a) For versions less then 4 => use hard coded list of filter names to differ between standard or additional filters.
    //    Why? This version don't support the order flag or hasn't set it right!
    // b) For version greater then 3 => use order of currently cached types in FilterCache!

    ePackage = E_STANDARD;
    nOrder   = 0;

    // writer
    if( sFilterName == DECLARE_ASCII("writer_StarOffice_XML_Writer"                               ) ) { ePackage = E_STANDARD; nOrder =  1; } else
    if( sFilterName == DECLARE_ASCII("writer_StarOffice_XML_Writer_Template"                      ) ) { ePackage = E_STANDARD; nOrder =  2; } else
    if( sFilterName == DECLARE_ASCII("writer_StarWriter_50"                                       ) ) { ePackage = E_STANDARD; nOrder =  3; } else
    if( sFilterName == DECLARE_ASCII("writer_StarWriter_50_VorlageTemplate"                       ) ) { ePackage = E_STANDARD; nOrder =  4; } else
    if( sFilterName == DECLARE_ASCII("writer_StarWriter_40"                                       ) ) { ePackage = E_STANDARD; nOrder =  5; } else
    if( sFilterName == DECLARE_ASCII("writer_StarWriter_40_VorlageTemplate"                       ) ) { ePackage = E_STANDARD; nOrder =  6; } else
    if( sFilterName == DECLARE_ASCII("writer_StarWriter_30"                                       ) ) { ePackage = E_STANDARD; nOrder =  7; } else
    if( sFilterName == DECLARE_ASCII("writer_StarWriter_30_VorlageTemplate"                       ) ) { ePackage = E_STANDARD; nOrder =  8; } else
    if( sFilterName == DECLARE_ASCII("writer_StarWriter_20"                                       ) ) { ePackage = E_STANDARD; nOrder =  9; } else
    if( sFilterName == DECLARE_ASCII("writer_MS_Word_97"                                          ) ) { ePackage = E_STANDARD; nOrder = 10; } else
    if( sFilterName == DECLARE_ASCII("writer_MS_Word_97_Vorlage"                                  ) ) { ePackage = E_STANDARD; nOrder = 11; } else
    if( sFilterName == DECLARE_ASCII("writer_MS_Word_95"                                          ) ) { ePackage = E_STANDARD; nOrder = 12; } else
    if( sFilterName == DECLARE_ASCII("writer_MS_Word_95_Vorlage"                                  ) ) { ePackage = E_STANDARD; nOrder = 13; } else
    if( sFilterName == DECLARE_ASCII("writer_MS_WinWord_2x_W4W"                                   ) ) { ePackage = E_STANDARD; nOrder = 14; } else
    if( sFilterName == DECLARE_ASCII("writer_MS_WinWord_1x_W4W"                                   ) ) { ePackage = E_STANDARD; nOrder = 15; } else
    if( sFilterName == DECLARE_ASCII("writer_MS_Word_6x_W4W"                                      ) ) { ePackage = E_STANDARD; nOrder = 16; } else
    if( sFilterName == DECLARE_ASCII("writer_MS_Word_5x_W4W"                                      ) ) { ePackage = E_STANDARD; nOrder = 17; } else
    if( sFilterName == DECLARE_ASCII("writer_MS_Word_4x_W4W"                                      ) ) { ePackage = E_STANDARD; nOrder = 18; } else
    if( sFilterName == DECLARE_ASCII("writer_MS_Word_3x_W4W"                                      ) ) { ePackage = E_STANDARD; nOrder = 19; } else
    if( sFilterName == DECLARE_ASCII("writer_WordPerfect_Win_70_W4W"                              ) ) { ePackage = E_STANDARD; nOrder = 20; } else
    if( sFilterName == DECLARE_ASCII("writer_WordPerfect_Win_61_W4W"                              ) ) { ePackage = E_STANDARD; nOrder = 21; } else
    if( sFilterName == DECLARE_ASCII("writer_WordPerfect_Win_60_W4W"                              ) ) { ePackage = E_STANDARD; nOrder = 22; } else
    if( sFilterName == DECLARE_ASCII("writer_WordPerfect_Win_51_52_W4W"                           ) ) { ePackage = E_STANDARD; nOrder = 23; } else
    if( sFilterName == DECLARE_ASCII("writer_HTML_StarWriter"                                     ) ) { ePackage = E_STANDARD; nOrder = 24; } else
    if( sFilterName == DECLARE_ASCII("writer_Text"                                                ) ) { ePackage = E_STANDARD; nOrder = 25; } else
    if( sFilterName == DECLARE_ASCII("writer_Text_encoded"                                        ) ) { ePackage = E_STANDARD; nOrder = 26; } else
    if( sFilterName == DECLARE_ASCII("writer_Text_DOS"                                            ) ) { ePackage = E_STANDARD; nOrder = 27; } else
    if( sFilterName == DECLARE_ASCII("writer_Text_Unix"                                           ) ) { ePackage = E_STANDARD; nOrder = 28; } else
    if( sFilterName == DECLARE_ASCII("writer_Text_Mac"                                            ) ) { ePackage = E_STANDARD; nOrder = 29; } else
    if( sFilterName == DECLARE_ASCII("writer_Rich_Text_Format"                                    ) ) { ePackage = E_STANDARD; nOrder = 30; }

    // writer web
    if( sFilterName == DECLARE_ASCII("writer_web_HTML"                                            ) ) { ePackage = E_STANDARD; nOrder =  1; } else
    if( sFilterName == DECLARE_ASCII("writer_web_StarOffice_XML_Writer"                           ) ) { ePackage = E_STANDARD; nOrder =  2; } else
    if( sFilterName == DECLARE_ASCII("writer_web_StarOffice_XML_Writer_Web_Template"              ) ) { ePackage = E_STANDARD; nOrder =  3; } else
    if( sFilterName == DECLARE_ASCII("writer_web_StarWriter_50"                                   ) ) { ePackage = E_STANDARD; nOrder =  4; } else
    if( sFilterName == DECLARE_ASCII("writer_web_StarWriterWeb_50_VorlageTemplate"                ) ) { ePackage = E_STANDARD; nOrder =  5; } else
    if( sFilterName == DECLARE_ASCII("writer_web_StarWriter_40"                                   ) ) { ePackage = E_STANDARD; nOrder =  6; } else
    if( sFilterName == DECLARE_ASCII("writer_web_StarWriterWeb_40_VorlageTemplate"                ) ) { ePackage = E_STANDARD; nOrder =  7; } else
    if( sFilterName == DECLARE_ASCII("writer_web_StarWriter_30"                                   ) ) { ePackage = E_STANDARD; nOrder =  8; } else
    if( sFilterName == DECLARE_ASCII("writer_web_Text_StarWriterWeb"                              ) ) { ePackage = E_STANDARD; nOrder =  9; } else
    if( sFilterName == DECLARE_ASCII("writer_web_Text_encoded"                                    ) ) { ePackage = E_STANDARD; nOrder = 10; } else
    if( sFilterName == DECLARE_ASCII("writer_web_Text_DOS_StarWriterWeb"                          ) ) { ePackage = E_STANDARD; nOrder = 11; } else
    if( sFilterName == DECLARE_ASCII("writer_web_Text_Unix_StarWriterWeb"                         ) ) { ePackage = E_STANDARD; nOrder = 12; } else
    if( sFilterName == DECLARE_ASCII("writer_web_Text_Mac_StarWriterWeb"                          ) ) { ePackage = E_STANDARD; nOrder = 13; }

    // global document
    if( sFilterName == DECLARE_ASCII("writer_globaldocument_StarOffice_XML_Writer_GlobalDocument" ) ) { ePackage = E_STANDARD; nOrder =  1; } else
    if( sFilterName == DECLARE_ASCII("writer_globaldocument_StarOffice_XML_Writer"                ) ) { ePackage = E_STANDARD; nOrder =  2; } else
    if( sFilterName == DECLARE_ASCII("writer_globaldocument_StarWriter_50"                        ) ) { ePackage = E_STANDARD; nOrder =  3; } else
    if( sFilterName == DECLARE_ASCII("writer_globaldocument_StarWriter_50GlobalDocument"          ) ) { ePackage = E_STANDARD; nOrder =  4; } else
    if( sFilterName == DECLARE_ASCII("writer_globaldocument_StarWriter_40"                        ) ) { ePackage = E_STANDARD; nOrder =  5; } else
    if( sFilterName == DECLARE_ASCII("writer_globaldocument_StarWriter_40GlobalDocument"          ) ) { ePackage = E_STANDARD; nOrder =  6; } else
    if( sFilterName == DECLARE_ASCII("writer_globaldocument_StarWriter_30"                        ) ) { ePackage = E_STANDARD; nOrder =  7; } else
    if( sFilterName == DECLARE_ASCII("writer_globaldocument_Text_encoded"                         ) ) { ePackage = E_STANDARD; nOrder =  8; }

    // calc
    if( sFilterName == DECLARE_ASCII("calc_StarOffice_XML_Calc"                                   ) ) { ePackage = E_STANDARD; nOrder =  1; } else
    if( sFilterName == DECLARE_ASCII("calc_StarOffice_XML_Calc_Template"                          ) ) { ePackage = E_STANDARD; nOrder =  2; } else
    if( sFilterName == DECLARE_ASCII("calc_StarCalc_50"                                           ) ) { ePackage = E_STANDARD; nOrder =  3; } else
    if( sFilterName == DECLARE_ASCII("calc_StarCalc_50_VorlageTemplate"                           ) ) { ePackage = E_STANDARD; nOrder =  4; } else
    if( sFilterName == DECLARE_ASCII("calc_StarCalc_40"                                           ) ) { ePackage = E_STANDARD; nOrder =  5; } else
    if( sFilterName == DECLARE_ASCII("calc_StarCalc_40_VorlageTemplate"                           ) ) { ePackage = E_STANDARD; nOrder =  6; } else
    if( sFilterName == DECLARE_ASCII("calc_StarCalc_30"                                           ) ) { ePackage = E_STANDARD; nOrder =  7; } else
    if( sFilterName == DECLARE_ASCII("calc_StarCalc_30_VorlageTemplate"                           ) ) { ePackage = E_STANDARD; nOrder =  8; } else
    if( sFilterName == DECLARE_ASCII("calc_StarCalc_10"                                           ) ) { ePackage = E_STANDARD; nOrder =  9; } else
    if( sFilterName == DECLARE_ASCII("calc_MS_Excel_97"                                           ) ) { ePackage = E_STANDARD; nOrder = 10; } else
    if( sFilterName == DECLARE_ASCII("calc_MS_Excel_97_VorlageTemplate"                           ) ) { ePackage = E_STANDARD; nOrder = 11; } else
    if( sFilterName == DECLARE_ASCII("calc_MS_Excel_95"                                           ) ) { ePackage = E_STANDARD; nOrder = 12; } else
    if( sFilterName == DECLARE_ASCII("calc_MS_Excel_95_VorlageTemplate"                           ) ) { ePackage = E_STANDARD; nOrder = 13; } else
    if( sFilterName == DECLARE_ASCII("calc_MS_Excel_5095"                                         ) ) { ePackage = E_STANDARD; nOrder = 14; } else
    if( sFilterName == DECLARE_ASCII("calc_MS_Excel_5095_VorlageTemplate"                         ) ) { ePackage = E_STANDARD; nOrder = 15; } else
    if( sFilterName == DECLARE_ASCII("calc_MS_Excel_40"                                           ) ) { ePackage = E_STANDARD; nOrder = 16; } else
    if( sFilterName == DECLARE_ASCII("calc_MS_Excel_40_VorlageTemplate"                           ) ) { ePackage = E_STANDARD; nOrder = 17; } else
    if( sFilterName == DECLARE_ASCII("calc_HTML_StarCalc"                                         ) ) { ePackage = E_STANDARD; nOrder = 18; } else
    if( sFilterName == DECLARE_ASCII("calc_HTML_WebQuery"                                         ) ) { ePackage = E_STANDARD; nOrder = 19; } else
    if( sFilterName == DECLARE_ASCII("calc_Rich_Text_Format_StarCalc"                             ) ) { ePackage = E_STANDARD; nOrder = 20; } else
    if( sFilterName == DECLARE_ASCII("calc_Text_txt_csv_StarCalc"                                 ) ) { ePackage = E_STANDARD; nOrder = 21; } else
    if( sFilterName == DECLARE_ASCII("calc_dBase"                                                 ) ) { ePackage = E_STANDARD; nOrder = 22; } else
    if( sFilterName == DECLARE_ASCII("calc_Lotus"                                                 ) ) { ePackage = E_STANDARD; nOrder = 23; } else
    if( sFilterName == DECLARE_ASCII("calc_SYLK"                                                  ) ) { ePackage = E_STANDARD; nOrder = 24; } else
    if( sFilterName == DECLARE_ASCII("calc_DIF"                                                   ) ) { ePackage = E_STANDARD; nOrder = 25; }

    // impress
    if( sFilterName == DECLARE_ASCII("impress_StarOffice_XML_Impress"                             ) ) { ePackage = E_STANDARD; nOrder =  1; } else
    if( sFilterName == DECLARE_ASCII("impress_StarOffice_XML_Impress_Template"                    ) ) { ePackage = E_STANDARD; nOrder =  2; } else
    if( sFilterName == DECLARE_ASCII("impress_StarImpress_50"                                     ) ) { ePackage = E_STANDARD; nOrder =  3; } else
    if( sFilterName == DECLARE_ASCII("impress_StarImpress_50_Vorlage"                             ) ) { ePackage = E_STANDARD; nOrder =  4; } else
    if( sFilterName == DECLARE_ASCII("impress_StarImpress_50_packed"                              ) ) { ePackage = E_STANDARD; nOrder =  5; } else
    if( sFilterName == DECLARE_ASCII("impress_StarImpress_40"                                     ) ) { ePackage = E_STANDARD; nOrder =  6; } else
    if( sFilterName == DECLARE_ASCII("impress_StarImpress_40_Vorlage"                             ) ) { ePackage = E_STANDARD; nOrder =  7; } else
    if( sFilterName == DECLARE_ASCII("impress_MS_PowerPoint_97"                                   ) ) { ePackage = E_STANDARD; nOrder =  8; } else
    if( sFilterName == DECLARE_ASCII("impress_MS_PowerPoint_97_Vorlage"                           ) ) { ePackage = E_STANDARD; nOrder =  9; } else
    if( sFilterName == DECLARE_ASCII("impress_StarOffice_XML_Draw"                                ) ) { ePackage = E_STANDARD; nOrder = 10; } else
    if( sFilterName == DECLARE_ASCII("impress_StarDraw_50_StarImpress"                            ) ) { ePackage = E_STANDARD; nOrder = 11; } else
    if( sFilterName == DECLARE_ASCII("impress_StarDraw_50_Vorlage_StarImpress"                    ) ) { ePackage = E_STANDARD; nOrder = 12; } else
    if( sFilterName == DECLARE_ASCII("impress_StarDraw_30_StarImpress"                            ) ) { ePackage = E_STANDARD; nOrder = 13; } else
    if( sFilterName == DECLARE_ASCII("impress_StarDraw_30_Vorlage_StarImpress"                    ) ) { ePackage = E_STANDARD; nOrder = 14; } else
    if( sFilterName == DECLARE_ASCII("impress_CGM_Computer_Graphics_Metafile"                     ) ) { ePackage = E_STANDARD; nOrder = 15; }

    // draw
    if( sFilterName == DECLARE_ASCII("draw_StarOffice_XML_Draw"                                   ) ) { ePackage = E_STANDARD; nOrder =  1; } else
    if( sFilterName == DECLARE_ASCII("draw_StarOffice_XML_Draw_Template"                          ) ) { ePackage = E_STANDARD; nOrder =  2; } else
    if( sFilterName == DECLARE_ASCII("draw_StarDraw_50"                                           ) ) { ePackage = E_STANDARD; nOrder =  3; } else
    if( sFilterName == DECLARE_ASCII("draw_StarDraw_50_Vorlage"                                   ) ) { ePackage = E_STANDARD; nOrder =  4; } else
    if( sFilterName == DECLARE_ASCII("draw_StarDraw_30"                                           ) ) { ePackage = E_STANDARD; nOrder =  5; } else
    if( sFilterName == DECLARE_ASCII("draw_StarDraw_30_Vorlage"                                   ) ) { ePackage = E_STANDARD; nOrder =  6; } else
    if( sFilterName == DECLARE_ASCII("draw_StarOffice_XML_Impress"                                ) ) { ePackage = E_STANDARD; nOrder =  7; } else
    if( sFilterName == DECLARE_ASCII("draw_StarImpress_50_StarDraw"                               ) ) { ePackage = E_STANDARD; nOrder =  8; } else
    if( sFilterName == DECLARE_ASCII("draw_StarImpress_50_Vorlage_StarDraw"                       ) ) { ePackage = E_STANDARD; nOrder =  9; } else
    if( sFilterName == DECLARE_ASCII("draw_StarImpress_40_StarDraw"                               ) ) { ePackage = E_STANDARD; nOrder = 10; } else
    if( sFilterName == DECLARE_ASCII("draw_StarImpress_40_Vorlage_StarDraw"                       ) ) { ePackage = E_STANDARD; nOrder = 11; } else
    if( sFilterName == DECLARE_ASCII("draw_SGV_StarDraw_20"                                       ) ) { ePackage = E_STANDARD; nOrder = 12; } else
    if( sFilterName == DECLARE_ASCII("draw_SGF_StarOffice_Writer_SGF"                             ) ) { ePackage = E_STANDARD; nOrder = 13; } else
    if( sFilterName == DECLARE_ASCII("draw_SVM_StarView_Metafile"                                 ) ) { ePackage = E_STANDARD; nOrder = 14; } else
    if( sFilterName == DECLARE_ASCII("draw_WMF_MS_Windows_Metafile"                               ) ) { ePackage = E_STANDARD; nOrder = 15; } else
    if( sFilterName == DECLARE_ASCII("draw_EMF_MS_Windows_Metafile"                               ) ) { ePackage = E_STANDARD; nOrder = 16; } else
    if( sFilterName == DECLARE_ASCII("draw_EPS_Encapsulated_PostScript"                           ) ) { ePackage = E_STANDARD; nOrder = 17; } else
    if( sFilterName == DECLARE_ASCII("draw_DXF_AutoCAD_Interchange"                               ) ) { ePackage = E_STANDARD; nOrder = 18; } else
    if( sFilterName == DECLARE_ASCII("draw_BMP_MS_Windows"                                        ) ) { ePackage = E_STANDARD; nOrder = 19; } else
    if( sFilterName == DECLARE_ASCII("draw_GIF_Graphics_Interchange"                              ) ) { ePackage = E_STANDARD; nOrder = 20; } else
    if( sFilterName == DECLARE_ASCII("draw_JPG_JPEG"                                              ) ) { ePackage = E_STANDARD; nOrder = 21; } else
    if( sFilterName == DECLARE_ASCII("draw_MET_OS2_Metafile"                                      ) ) { ePackage = E_STANDARD; nOrder = 22; } else
    if( sFilterName == DECLARE_ASCII("draw_PBM_Portable_Bitmap"                                   ) ) { ePackage = E_STANDARD; nOrder = 23; } else
    if( sFilterName == DECLARE_ASCII("draw_PCD_Photo_CD_Base"                                     ) ) { ePackage = E_STANDARD; nOrder = 24; } else
    if( sFilterName == DECLARE_ASCII("draw_PCD_Photo_CD_Base4"                                    ) ) { ePackage = E_STANDARD; nOrder = 25; } else
    if( sFilterName == DECLARE_ASCII("draw_PCD_Photo_CD_Base16"                                   ) ) { ePackage = E_STANDARD; nOrder = 26; } else
    if( sFilterName == DECLARE_ASCII("draw_PCT_Mac_Pict"                                          ) ) { ePackage = E_STANDARD; nOrder = 27; } else
    if( sFilterName == DECLARE_ASCII("draw_PCX_Zsoft_Paintbrush"                                  ) ) { ePackage = E_STANDARD; nOrder = 28; } else
    if( sFilterName == DECLARE_ASCII("draw_PGM_Portable_Graymap"                                  ) ) { ePackage = E_STANDARD; nOrder = 29; } else
    if( sFilterName == DECLARE_ASCII("draw_PNG_Portable_Network_Graphic"                          ) ) { ePackage = E_STANDARD; nOrder = 30; } else
    if( sFilterName == DECLARE_ASCII("draw_PPM_Portable_Pixelmap"                                 ) ) { ePackage = E_STANDARD; nOrder = 31; } else
    if( sFilterName == DECLARE_ASCII("draw_PSD_Adobe_Photoshop"                                   ) ) { ePackage = E_STANDARD; nOrder = 32; } else
    if( sFilterName == DECLARE_ASCII("draw_RAS_Sun_Rasterfile"                                    ) ) { ePackage = E_STANDARD; nOrder = 33; } else
    if( sFilterName == DECLARE_ASCII("draw_TGA_Truevision_TARGA"                                  ) ) { ePackage = E_STANDARD; nOrder = 34; } else
    if( sFilterName == DECLARE_ASCII("draw_TIF_Tag_Image_File"                                    ) ) { ePackage = E_STANDARD; nOrder = 35; } else
    if( sFilterName == DECLARE_ASCII("draw_XBM_X_Consortium"                                      ) ) { ePackage = E_STANDARD; nOrder = 36; } else
    if( sFilterName == DECLARE_ASCII("draw_XPM"                                                   ) ) { ePackage = E_STANDARD; nOrder = 37; }

    // chart
    if( sFilterName == DECLARE_ASCII("chart_StarOffice_XML_Chart"                                 ) ) { ePackage = E_STANDARD; nOrder =  1; } else
    if( sFilterName == DECLARE_ASCII("chart_StarChart_50"                                         ) ) { ePackage = E_STANDARD; nOrder =  2; } else
    if( sFilterName == DECLARE_ASCII("chart_StarChart_40"                                         ) ) { ePackage = E_STANDARD; nOrder =  3; } else
    if( sFilterName == DECLARE_ASCII("chart_StarChart_30"                                         ) ) { ePackage = E_STANDARD; nOrder =  4; }

    // math
    if( sFilterName == DECLARE_ASCII("math_StarOffice_XML_Math"                                   ) ) { ePackage = E_STANDARD; nOrder =  1; } else
    if( sFilterName == DECLARE_ASCII("math_StarMath_50"                                           ) ) { ePackage = E_STANDARD; nOrder =  2; } else
    if( sFilterName == DECLARE_ASCII("math_StarMath_40"                                           ) ) { ePackage = E_STANDARD; nOrder =  3; } else
    if( sFilterName == DECLARE_ASCII("math_StarMath_30"                                           ) ) { ePackage = E_STANDARD; nOrder =  4; } else
    if( sFilterName == DECLARE_ASCII("math_StarMath_20"                                           ) ) { ePackage = E_STANDARD; nOrder =  5; } else
    if( sFilterName == DECLARE_ASCII("math_MathML_XML_Math"                                       ) ) { ePackage = E_STANDARD; nOrder =  6; } else
    if( sFilterName == DECLARE_ASCII("math_MathType_3x"                                           ) ) { ePackage = E_STANDARD; nOrder =  7; }

    // graphics
    if( sFilterName == DECLARE_ASCII("bmp_Import"                                                 ) ) { ePackage = E_STANDARD; nOrder =  1; } else
    if( sFilterName == DECLARE_ASCII("bmp_Export"                                                 ) ) { ePackage = E_STANDARD; nOrder =  2; } else
    if( sFilterName == DECLARE_ASCII("dxf_Import"                                                 ) ) { ePackage = E_STANDARD; nOrder =  3; } else
    if( sFilterName == DECLARE_ASCII("emf_Import"                                                 ) ) { ePackage = E_STANDARD; nOrder =  4; } else
    if( sFilterName == DECLARE_ASCII("emf_Export"                                                 ) ) { ePackage = E_STANDARD; nOrder =  5; } else
    if( sFilterName == DECLARE_ASCII("eps_Import"                                                 ) ) { ePackage = E_STANDARD; nOrder =  6; } else
    if( sFilterName == DECLARE_ASCII("eps_Export"                                                 ) ) { ePackage = E_STANDARD; nOrder =  7; } else
    if( sFilterName == DECLARE_ASCII("gif_Import"                                                 ) ) { ePackage = E_STANDARD; nOrder =  8; } else
    if( sFilterName == DECLARE_ASCII("gif_Export"                                                 ) ) { ePackage = E_STANDARD; nOrder =  9; } else
    if( sFilterName == DECLARE_ASCII("jpg_Import"                                                 ) ) { ePackage = E_STANDARD; nOrder = 10; } else
    if( sFilterName == DECLARE_ASCII("jpg_Export"                                                 ) ) { ePackage = E_STANDARD; nOrder = 11; } else
    if( sFilterName == DECLARE_ASCII("met_Import"                                                 ) ) { ePackage = E_STANDARD; nOrder = 12; } else
    if( sFilterName == DECLARE_ASCII("met_Export"                                                 ) ) { ePackage = E_STANDARD; nOrder = 13; } else
    if( sFilterName == DECLARE_ASCII("pbm_Import"                                                 ) ) { ePackage = E_STANDARD; nOrder = 14; } else
    if( sFilterName == DECLARE_ASCII("pbm_Export"                                                 ) ) { ePackage = E_STANDARD; nOrder = 15; } else
    if( sFilterName == DECLARE_ASCII("pcd_Import_Base16"                                          ) ) { ePackage = E_STANDARD; nOrder = 16; } else
    if( sFilterName == DECLARE_ASCII("pcd_Import_Base4"                                           ) ) { ePackage = E_STANDARD; nOrder = 17; } else
    if( sFilterName == DECLARE_ASCII("pcd_Import_Base"                                            ) ) { ePackage = E_STANDARD; nOrder = 18; } else
    if( sFilterName == DECLARE_ASCII("pct_Import"                                                 ) ) { ePackage = E_STANDARD; nOrder = 19; } else
    if( sFilterName == DECLARE_ASCII("pct_Export"                                                 ) ) { ePackage = E_STANDARD; nOrder = 20; } else
    if( sFilterName == DECLARE_ASCII("pcx_Import"                                                 ) ) { ePackage = E_STANDARD; nOrder = 21; } else
    if( sFilterName == DECLARE_ASCII("pgm_Import"                                                 ) ) { ePackage = E_STANDARD; nOrder = 22; } else
    if( sFilterName == DECLARE_ASCII("pgm_Export"                                                 ) ) { ePackage = E_STANDARD; nOrder = 23; } else
    if( sFilterName == DECLARE_ASCII("png_Import"                                                 ) ) { ePackage = E_STANDARD; nOrder = 24; } else
    if( sFilterName == DECLARE_ASCII("png_Export"                                                 ) ) { ePackage = E_STANDARD; nOrder = 25; } else
    if( sFilterName == DECLARE_ASCII("ppm_Import"                                                 ) ) { ePackage = E_STANDARD; nOrder = 26; } else
    if( sFilterName == DECLARE_ASCII("ppm_Export"                                                 ) ) { ePackage = E_STANDARD; nOrder = 27; } else
    if( sFilterName == DECLARE_ASCII("psd_Import"                                                 ) ) { ePackage = E_STANDARD; nOrder = 28; } else
    if( sFilterName == DECLARE_ASCII("ras_Import"                                                 ) ) { ePackage = E_STANDARD; nOrder = 29; } else
    if( sFilterName == DECLARE_ASCII("ras_Export"                                                 ) ) { ePackage = E_STANDARD; nOrder = 30; } else
    if( sFilterName == DECLARE_ASCII("sgf_Import"                                                 ) ) { ePackage = E_STANDARD; nOrder = 31; } else
    if( sFilterName == DECLARE_ASCII("sgv_Import"                                                 ) ) { ePackage = E_STANDARD; nOrder = 32; } else
    if( sFilterName == DECLARE_ASCII("svg_Export"                                                 ) ) { ePackage = E_STANDARD; nOrder = 33; } else
    if( sFilterName == DECLARE_ASCII("svm_Import"                                                 ) ) { ePackage = E_STANDARD; nOrder = 34; } else
    if( sFilterName == DECLARE_ASCII("svm_Export"                                                 ) ) { ePackage = E_STANDARD; nOrder = 35; } else
    if( sFilterName == DECLARE_ASCII("tga_Import"                                                 ) ) { ePackage = E_STANDARD; nOrder = 36; } else
    if( sFilterName == DECLARE_ASCII("tif_Import"                                                 ) ) { ePackage = E_STANDARD; nOrder = 37; } else
    if( sFilterName == DECLARE_ASCII("tif_Export"                                                 ) ) { ePackage = E_STANDARD; nOrder = 38; } else
    if( sFilterName == DECLARE_ASCII("wmf_Import"                                                 ) ) { ePackage = E_STANDARD; nOrder = 39; } else
    if( sFilterName == DECLARE_ASCII("wmf_Export"                                                 ) ) { ePackage = E_STANDARD; nOrder = 40; } else
    if( sFilterName == DECLARE_ASCII("xbm_Import"                                                 ) ) { ePackage = E_STANDARD; nOrder = 41; } else
    if( sFilterName == DECLARE_ASCII("xpm_Import"                                                 ) ) { ePackage = E_STANDARD; nOrder = 42; } else
    if( sFilterName == DECLARE_ASCII("xpm_Export"                                                 ) ) { ePackage = E_STANDARD; nOrder = 43; }
}

//*****************************************************************************************************************
void XCDGenerator::impl_orderAlphabetical( css::uno::Sequence< ::rtl::OUString >& lList )
{
    ::std::vector< ::rtl::OUString > lSortedList;
    sal_Int32                        nCount     ;
    sal_Int32                        nItem      ;

    // Copy sequence to vector
    nCount = lList.getLength();
    for( nItem=0; nItem<nCount; ++nItem )
    {
        lSortedList.push_back( lList[nItem] );
    }

    // sort in a alphabetical order
    ::std::sort( lSortedList.begin(), lSortedList.end() );

    // copy sorted list back to sequence
    nItem = 0;
    for( ::std::vector< ::rtl::OUString >::iterator pIterator=lSortedList.begin(); pIterator!=lSortedList.end(); ++pIterator )
    {
        lList[nItem] = *pIterator;
        ++nItem;
    }
}

//*****************************************************************************************************************
class ModifiedUTF7Buffer
{
    rtl::OUStringBuffer & m_rBuffer;
    sal_uInt32 m_nValue;
    int m_nFilled;

public:
    ModifiedUTF7Buffer(rtl::OUStringBuffer * pTheBuffer):
        m_rBuffer(*pTheBuffer), m_nFilled(0) {}

    inline void write(sal_Unicode c);

    void flush();
};

inline void ModifiedUTF7Buffer::write(sal_Unicode c)
{
    switch (m_nFilled)
    {
        case 0:
            m_nValue = sal_uInt32(c) << 8;
            m_nFilled = 2;
            break;

        case 1:
            m_nValue |= sal_uInt32(c);
            m_nFilled = 3;
            flush();
            break;

        case 2:
            m_nValue |= sal_uInt32(c) >> 8;
            m_nFilled = 3;
            flush();
            m_nValue = (sal_uInt32(c) & 0xFF) << 16;
            m_nFilled = 1;
            break;
    }
}

void ModifiedUTF7Buffer::flush()
{
    static sal_Unicode const aModifiedBase64[64]
        = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
            'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
            'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
            'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
            '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '-', '.' };
    switch (m_nFilled)
    {
        case 1:
            m_rBuffer.append(aModifiedBase64[m_nValue >> 18]);
            m_rBuffer.append(aModifiedBase64[m_nValue >> 12 & 63]);
            break;

        case 2:
            m_rBuffer.append(aModifiedBase64[m_nValue >> 18]);
            m_rBuffer.append(aModifiedBase64[m_nValue >> 12 & 63]);
            m_rBuffer.append(aModifiedBase64[m_nValue >> 6 & 63]);
            break;

        case 3:
            m_rBuffer.append(aModifiedBase64[m_nValue >> 18]);
            m_rBuffer.append(aModifiedBase64[m_nValue >> 12 & 63]);
            m_rBuffer.append(aModifiedBase64[m_nValue >> 6 & 63]);
            m_rBuffer.append(aModifiedBase64[m_nValue & 63]);
            break;
    }
    m_nFilled = 0;
    m_nValue = 0;
}


sal_Bool XCDGenerator::impl_isUsAsciiAlphaDigit(sal_Unicode c, sal_Bool bDigitAllowed)
{
    return c >= 'A' && c <= 'Z' || c >= 'a' && c <= 'z'
           || bDigitAllowed && c >= '0' && c <= '9';
}

::rtl::OUString XCDGenerator::impl_encodeSetName( const ::rtl::OUString& rSource )
{
    return impl_encodeSpecialSigns( rSource );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
