/*************************************************************************
 *
 *  $RCSfile: xml2xcd.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: as $ $Date: 2001-06-05 10:12:11 $
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

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#define VCL_NEED_BASETSD
#include <tools/presys.h>
#include <windows.h>
#include <tools/postsys.h>

#ifndef __FRAMEWORK_CLASSES_SERVICEMANAGER_HXX_
#include <classes/servicemanager.hxx>
#endif

#ifndef __FRAMEWORK_CLASSES_FILTERCACHE_HXX_
#include <classes/filtercache.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_GENERIC_HXX_
#include <macros/generic.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_DEBUG_HXX_
#include <macros/debug.hxx>
#endif

#ifndef __FRAMEWORK_SERVICES_H_
#include <services.h>
#endif

#ifndef __FRAMEWORK_FILTERFLAGS_H_
#include <filterflags.h>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _VOS_PROCESS_HXX_
#include <vos/process.hxx>
#endif

#ifndef _RTL_USTRING_
#include <rtl/ustring.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

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

#define PREFIX_TYPES                "t"
#define PREFIX_FILTERS              "f"
#define PREFIX_DETECTORS            "d"
#define PREFIX_LOADERS              "l"

#define ARGUMENT_FILENAME           DECLARE_ASCII("-fi=")           // argument for filename                <filename in system notation>
#define ARGUMENT_WRITEABLE          DECLARE_ASCII("-wr=")           // argument for "writeable"             [true|false]
#define ARGUMENT_VERSION_INPUT      DECLARE_ASCII("-vi=")           // argument for file version to read    [1|2|3]
#define ARGUMENT_VERSION_OUTPUT     DECLARE_ASCII("-vo=")           // argument for file version to read    [1|2|3]
#define ARGUMENTLENGTH              4                               // All arguments should have the same lenght ... it's better to detect it!
#define ARGUMENTFOUND               0                               // OUString::compareTo returns 0 if searched string match given one
#define WRITEABLE_ON                DECLARE_ASCII("true" )
#define WRITEABLE_OFF               DECLARE_ASCII("false")

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

/*-***************************************************************************************************************/
struct AppMember
{
        FilterCache*                pFilterCache          ;
        StringHash                  aOldFilterNamesHash   ;
        ::rtl::OUString             sXCDFileName          ;
        sal_Bool                    bWriteable            ;
        sal_Int32                   nVersionInput         ;
        sal_Int32                   nVersionOutput        ;

        sal_Int32                   nOriginalTypes        ;
        sal_Int32                   nOriginalFilters      ;
        sal_Int32                   nOriginalDetectors    ;
        sal_Int32                   nOriginalLoaders      ;
        sal_Int32                   nWrittenTypes         ;
        sal_Int32                   nWrittenFilters       ;
        sal_Int32                   nWrittenDetectors     ;
        sal_Int32                   nWrittenLoaders       ;
};

/*-***************************************************************************************************************/
class XCDGenerator : public Application
{
    //*************************************************************************************************************
    public:

        void Main();

    //*************************************************************************************************************
    private:

        void            impl_printCopyright             (                                                                           );
        void            impl_printSyntax                (                                                                           );
        void            impl_parseCommandLine           (           AppMember&                  rMember                             );
        void            impl_generateXCD                (   const   ::rtl::OUString&            sFileName                           ,
                                                                    sal_Bool                    bWriteable                          );
        void            impl_generateCopyright          (           ::rtl::OUStringBuffer&      sXCD                                );
        void            impl_generateTypeTemplate       (           ::rtl::OUStringBuffer&      sXCD                                ,
                                                                    sal_Bool                    bWriteable                          );
        void            impl_generateFilterTemplate     (           ::rtl::OUStringBuffer&      sXCD                                ,
                                                                    sal_Bool                    bWriteable                          );
        void            impl_generateDetectorTemplate   (           ::rtl::OUStringBuffer&      sXCD                                ,
                                                                    sal_Bool                    bWriteable                          );
        void            impl_generateLoaderTemplate     (           ::rtl::OUStringBuffer&      sXCD                                ,
                                                                    sal_Bool                    bWriteable                          );
        void            impl_generateFilterFlagTemplate (           ::rtl::OUStringBuffer&      sXCD                                ,
                                                            const   ::rtl::OUString&            sName                               ,
                                                                    sal_Int32                   nValue                              ,
                                                            const   ::rtl::OString&             sDescription = ::rtl::OString()     );
        void            impl_generateTypeSet            (           ::rtl::OUStringBuffer&      sXCD                                ,
                                                                    sal_Bool                    bWriteable                          );
        void            impl_generateFilterSet          (           ::rtl::OUStringBuffer&      sXCD                                ,
                                                                    sal_Bool                    bWriteable                          );
        void            impl_generateDetectorSet        (           ::rtl::OUStringBuffer&      sXCD                                ,
                                                                    sal_Bool                    bWriteable                          );
        void            impl_generateLoaderSet          (           ::rtl::OUStringBuffer&      sXCD                                ,
                                                                    sal_Bool                    bWriteable                          );
        void            impl_generateDefaults           (           ::rtl::OUStringBuffer&      sXCD                                );
        void            impl_generateIntProperty        (           ::rtl::OUStringBuffer&      sXCD                                ,
                                                            const   ::rtl::OUString&            sName                               ,
                                                                    sal_Int32                   nValue                              ,
                                                                    sal_Bool                    bWriteable                          );
        void            impl_generateBoolProperty       (           ::rtl::OUStringBuffer&      sXCD                                ,
                                                            const   ::rtl::OUString&            sName                               ,
                                                                    sal_Bool                    bValue                              ,
                                                                    sal_Bool                    bWriteable                          );
        void            impl_generateStringProperty     (           ::rtl::OUStringBuffer&      sXCD                                ,
                                                            const   ::rtl::OUString&            sName                               ,
                                                            const   ::rtl::OUString&            sValue                              ,
                                                                    sal_Bool                    bWriteable                          );
        void            impl_generateStringListProperty (           ::rtl::OUStringBuffer&      sXCD                                ,
                                                            const   ::rtl::OUString&            sName                               ,
                                                            const   ::framework::StringList&    lValue                              ,
                                                                    sal_Bool                    bWriteable                          );
        void            impl_generateUINamesProperty    (           ::rtl::OUStringBuffer&      sXCD                                ,
                                                            const   ::rtl::OUString&            sName                               ,
                                                            const   StringHash&                 lUINames                            ,
                                                                    sal_Bool                    bWriteable                          );
        ::rtl::OUString impl_filterSpecialSigns         (   const   ::rtl::OUString&            sValue                              );
        sal_Unicode     impl_defineSeperator            (   const   ::framework::StringList&    lList                               );
        void            impl_initFilterHashNew2Old      (           StringHash&                 aHash                               );
        ::rtl::OUString impl_getOldFilterName           (   const   ::rtl::OUString&            sNewName                            );

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
    impl_printCopyright();

    // Init global servicemanager and set it.
    // It's neccessary for other services ... e.g. configuration.
    ServiceManager aManager;
    ::comphelper::setProcessServiceFactory( aManager.getGlobalUNOServiceManager() );

    // Get optional commands from command line.
    impl_parseCommandLine( m_aData );

    // initialize converter table to match new to old filter names!
    if( m_aData.nVersionOutput >= 3 )
    {
        impl_initFilterHashNew2Old( m_aData.aOldFilterNamesHash );
    }

    // Create access to current set filter configuration.
    // Attention: Please use it for a full fat office installation only!!
    //            We need an installation with ALL filters.
    // Member m_pData is used in some impl-methods directly ...
    m_aData.pFilterCache = new FilterCache( m_aData.nVersionInput );

    // Get some statistic informations of current filled filter cache ... (e.g. count of current activae filters)
    // because we need it to check if all filters are converted and written to disk.
    // May be it's possible to lose some of them during convertion!!!
    m_aData.nOriginalTypes     = m_aData.pFilterCache->getAllTypeNames().getLength()     ;
    m_aData.nOriginalFilters   = m_aData.pFilterCache->getAllFilterNames().getLength()   ;
    m_aData.nOriginalDetectors = m_aData.pFilterCache->getAllDetectorNames().getLength() ;
    m_aData.nOriginalLoaders   = m_aData.pFilterCache->getAllLoaderNames().getLength()   ;

    // Start generation of xcd file.
    impl_generateXCD( m_aData.sXCDFileName, m_aData.bWriteable );

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
    fprintf( stderr, "\n(c) Copyright by Sun microsystems, 2001\n" );
}

//*****************************************************************************************************************
void XCDGenerator::impl_printSyntax()
{
    fprintf( stderr, "\nusing: xml2xcd -fi=<outputfile> -vi=<version input> -vo=<version output> [-wr=<true|false>]\n\n"    );
    fprintf( stderr, "\tneccessary parameters:\n"                                                                           );
    fprintf( stderr, "\t\t-fi=<outputfile>\tname of output file in system notation\n"                                       );
    fprintf( stderr, "\t\t-vi=<version input>\tformat version of input xml file\n"                                          );
    fprintf( stderr, "\t\t-vo=<version output>\tformat version of generated xcd file\n\n"                                   );
    fprintf( stderr, "\toptional parameters:\n"                                                                             );
    fprintf( stderr, "\t\t-wr=<true|false>\tconfig items should be writeable ... [true|false]\n"                            );
}

/*-************************************************************************************************************//**
    @short      analyze command line arguments
    @descr      Created binary accept different command line arguments. These parameters
                regulate creation of xcd file. Follow arguments are supported:
                    "-fi=<filename of xcd>"
                    "-wr=<writeable>[true|false]"
                    "-vi=<version of input file>[1|2|3]"
                    "-vo=<version of output file>[1|2|3]"

    @seealso    -

    @param      "rMember", reference to struct of global application member to fill arguments in it
    @return     right filled member struct or unchanged struct if an error occure!

    @onerror    We do nothing - or warn programmer!
*//*-*************************************************************************************************************/
void XCDGenerator::impl_parseCommandLine( AppMember& rMember )
{
    ::vos::OStartupInfo aInfo                                   ;
    ::rtl::OUString     sArgument                               ;
    sal_Int32           nArgument   = 0                         ;
    sal_Int32           nCount      = aInfo.getCommandArgCount();
    sal_Int32           nMinCount   = 0                         ;

    while( nArgument<nCount )
    {
        aInfo.getCommandArg( nArgument, sArgument );

        //_____________________________________________________________________________________________________
        // look for "-f=<file name of xcd>"
        if( sArgument.compareTo( ARGUMENT_FILENAME, ARGUMENTLENGTH ) == ARGUMENTFOUND )
        {
            rMember.sXCDFileName = sArgument.copy( ARGUMENTLENGTH, sArgument.getLength()-ARGUMENTLENGTH );
            ++nMinCount;
        }
        else
        //_____________________________________________________________________________________________________
        // look for "-w=<writeable>"
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
        }
        //_____________________________________________________________________________________________________
        // look for "-vi=<version of input file>"
        if( sArgument.compareTo( ARGUMENT_VERSION_INPUT, ARGUMENTLENGTH ) == ARGUMENTFOUND )
        {
            ::rtl::OUString sVersion = sArgument.copy( ARGUMENTLENGTH, sArgument.getLength()-ARGUMENTLENGTH );
            rMember.nVersionInput = sVersion.toInt32();
            ++nMinCount;
        }
        //_____________________________________________________________________________________________________
        // look for "-vo=<version of output file>"
        if( sArgument.compareTo( ARGUMENT_VERSION_OUTPUT, ARGUMENTLENGTH ) == ARGUMENTFOUND )
        {
            ::rtl::OUString sVersion = sArgument.copy( ARGUMENTLENGTH, sArgument.getLength()-ARGUMENTLENGTH );
            rMember.nVersionOutput = sVersion.toInt32();
            ++nMinCount;
        }

        ++nArgument;
    }

    // Show help if user don't call us right!
    if( nMinCount != 3 )
    {
        impl_printSyntax();
        exit(-1);
    }
}

/*-************************************************************************************************************//**
    @short      regulate generation of complete xcd file
    @descr      This method is the toppest one and implement the global structure of generated xcd file.
                We create a unicode string buffer for complete xcd file in memory ...
                use different helper methods to fill it ...
                and write it to disk at the end of this method!

    @seealso    -

    @param      "sFileName" , name of generated xcd file
    @param      "bWriteable", make config properties writeable or not
    @return     -

    @onerror    -
*//*-*************************************************************************************************************/
void XCDGenerator::impl_generateXCD( const ::rtl::OUString& sFileName, sal_Bool bWriteable )
{
    // A complete TypeDetection.xcd needs ~ 1.3 ... 1.5 MB!
    ::rtl::OUStringBuffer  sXCD( 1500000 );

    impl_generateCopyright( sXCD );

    sXCD.appendAscii( "<!DOCTYPE schema:component SYSTEM \"../../../../schema/schema.description.dtd\">\n"                                                                                                                                                                                                                                  );
    sXCD.appendAscii( "<schema:component cfg:name=\"TypeDetection\" cfg:package=\"org.openoffice.Office\" xml:lang=\"en-US\" xmlns:schema=\"http://openoffice.org/2000/registry/schema/description\" xmlns:default=\"http://openoffice.org/2000/registry/schema/default\" xmlns:cfg=\"http://openoffice.org/2000/registry/instance\">\n"    );
    sXCD.appendAscii( "\t<schema:templates>\n"                                                                                                                                                                                                                                                                                              );
    sXCD.appendAscii( "\n\n\n<!-- PLEASE DON'T CHANGE TEMPLATES OR FILE FORMAT BY HAND! USE \"XML2XCD.EXE\" TO DO THAT. CONTACT andreas.schluens@germany.sun.com FOR FURTHER INFORMATIONS. THANKS. -->\n\n\n"                                                                                                                               );

    impl_generateTypeTemplate       ( sXCD, bWriteable  );
    impl_generateFilterTemplate     ( sXCD, bWriteable  );
    impl_generateDetectorTemplate   ( sXCD, bWriteable  );
    impl_generateLoaderTemplate     ( sXCD, bWriteable  );

    sXCD.appendAscii( "\t</schema:templates>\n"                     );
    sXCD.appendAscii( "<schema:schema cfg:localized=\"false\">\n"   );

    impl_generateTypeSet            ( sXCD, bWriteable  );
    impl_generateFilterSet          ( sXCD, bWriteable  );
    impl_generateDetectorSet        ( sXCD, bWriteable  );
    impl_generateLoaderSet          ( sXCD, bWriteable  );
    impl_generateDefaults           ( sXCD              );

    sXCD.appendAscii( "\t</schema:schema>\n"  );
    sXCD.appendAscii( "</schema:component>\n" );

    WRITE_LOGFILE( U2B(sFileName), U2B(sXCD.makeStringAndClear()) )
}

//*****************************************************************************************************************
void XCDGenerator::impl_generateCopyright( ::rtl::OUStringBuffer& sXCD )
{
    sXCD.appendAscii( "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"                                );
    sXCD.appendAscii( "<!-- The Contents of this file are made available subject to the terms of\n" );
    sXCD.appendAscii( " either of the following licenses\n"                                         );
    sXCD.appendAscii( "\n"                                                                          );
    sXCD.appendAscii( "          - GNU Lesser General Public License Version 2.1\n"                 );
    sXCD.appendAscii( "          - Sun Industry Standards Source License Version 1.1\n"             );
    sXCD.appendAscii( "\n"                                                                          );
    sXCD.appendAscii( "   Sun Microsystems Inc., October, 2000\n"                                   );
    sXCD.appendAscii( "\n"                                                                          );
    sXCD.appendAscii( "   GNU Lesser General Public License Version 2.1\n"                          );
    sXCD.appendAscii( "   =============================================\n"                          );
    sXCD.appendAscii( "   Copyright 2000 by Sun Microsystems, Inc.\n"                               );
    sXCD.appendAscii( "   901 San Antonio Road, Palo Alto, CA 94303, USA\n"                         );
    sXCD.appendAscii( "   This library is free software; you can redistribute it and/or\n"          );
    sXCD.appendAscii( "   modify it under the terms of the GNU Lesser General Public\n"             );
    sXCD.appendAscii( "   License version 2.1, as published by the Free Software Foundation.\n"     );
    sXCD.appendAscii( "\n"                                                                          );
    sXCD.appendAscii( "   This library is distributed in the hope that it will be useful,\n"        );
    sXCD.appendAscii( "   but WITHOUT ANY WARRANTY; without even the implied warranty of\n"         );
    sXCD.appendAscii( "   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU\n"      );
    sXCD.appendAscii( "   Lesser General Public License for more details.\n"                        );
    sXCD.appendAscii( "\n"                                                                          );
    sXCD.appendAscii( "   You should have received a copy of the GNU Lesser General Public\n"       );
    sXCD.appendAscii( "   License along with this library; if not, write to the Free Software\n"    );
    sXCD.appendAscii( "   Foundation, Inc., 59 Temple Place, Suite 330, Boston,\n"                  );
    sXCD.appendAscii( "   MA  02111-1307  USA\n"                                                    );
    sXCD.appendAscii( "\n"                                                                          );
    sXCD.appendAscii( "\n"                                                                          );
    sXCD.appendAscii( "   Sun Industry Standards Source License Version 1.1\n"                      );
    sXCD.appendAscii( "   =================================================\n"                      );
    sXCD.appendAscii( "   The contents of this file are subject to the Sun Industry Standards\n"    );
    sXCD.appendAscii( "   Source License Version 1.1 (the \"License\"); You may not use this file\n");
    sXCD.appendAscii( "   except in compliance with the License. You may obtain a copy of the\n"    );
    sXCD.appendAscii( "   License at http://www.openoffice.org/license.html.\n"                     );
    sXCD.appendAscii( "\n"                                                                          );
    sXCD.appendAscii( "   Software provided under this License is provided on an \"AS IS\" basis,\n");
    sXCD.appendAscii( "   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,\n"    );
    sXCD.appendAscii( "   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,\n"   );
    sXCD.appendAscii( "   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.\n"         );
    sXCD.appendAscii( "   See the License for the specific provisions governing your rights and\n"  );
    sXCD.appendAscii( "   obligations concerning the Software.\n"                                   );
    sXCD.appendAscii( "\n"                                                                          );
    sXCD.appendAscii( "   The Initial Developer of the Original Code is: Sun Microsystems, Inc.\n"  );
    sXCD.appendAscii( "\n"                                                                          );
    sXCD.appendAscii( "   Copyright: 2000 by Sun Microsystems, Inc.\n"                              );
    sXCD.appendAscii( "\n"                                                                          );
    sXCD.appendAscii( "   All Rights Reserved.\n"                                                   );
    sXCD.appendAscii( "\n"                                                                          );
    sXCD.appendAscii( "   Contributor(s): _______________________________________\n"                );
    sXCD.appendAscii( "-->\n"                                                                       );
}

//*****************************************************************************************************************
void XCDGenerator::impl_generateTypeTemplate( ::rtl::OUStringBuffer& sXCD, sal_Bool bWriteable )
{
//_________________________________________________________________________________________________________________
if( m_aData.nVersionOutput==1 || m_aData.nVersionOutput==2 )
{
    sXCD.appendAscii( "\t\t<schema:group cfg:name=\"Type\">\n"                                                                                                                                                                  );

    sXCD.appendAscii( "\t\t\t<schema:value cfg:name=\"Preferred\" cfg:type=\"boolean\" cfg:writable=\""                                                                                                                         );
    sXCD.appendAscii( bWriteable==sal_True ? "true\">\n" : "false\">\n"                                                                                                                                                         );
    sXCD.appendAscii("\t\t\t\t<schema:documentation>\n"                                                                                                                                                                         );
    sXCD.appendAscii( "\t\t\t\t\t<schema:description>Specifies the preferred type for an extension if more then one match given URL</schema:description>\n"                                                                     );
    sXCD.appendAscii( "\t\t\t\t</schema:documentation>\n"                                                                                                                                                                       );
    sXCD.appendAscii( "\t\t\t\t<default:data>false</default:data>\n"                                                                                                                                                            );
    sXCD.appendAscii( "\t\t\t</schema:value>\n"                                                                                                                                                                                 );

    sXCD.appendAscii( "\t\t\t<schema:value cfg:name=\"UIName\" cfg:type=\"string\" cfg:localized=\"true\" cfg:writable=\""                                                                                                      );
    sXCD.appendAscii( bWriteable==sal_True ? "true\">\n" : "false\">\n"                                                                                                                                                         );
    sXCD.appendAscii( "\t\t\t\t<schema:documentation>\n"                                                                                                                                                                        );
    sXCD.appendAscii( "\t\t\t\t\t<schema:description>Specifies the external name of this type</schema:description>\n"                                                                                                           );
    sXCD.appendAscii( "\t\t\t\t</schema:documentation>\n"                                                                                                                                                                       );
    sXCD.appendAscii( "\t\t\t</schema:value>\n"                                                                                                                                                                                 );

    sXCD.appendAscii( "\t\t\t<schema:value cfg:name=\"MediaType\" cfg:type=\"string\" cfg:writable=\""                                                                                                                          );
    sXCD.appendAscii( bWriteable==sal_True ? "true\">\n" : "false\">\n"                                                                                                                                                         );
    sXCD.appendAscii( "\t\t\t\t<schema:documentation>\n"                                                                                                                                                                        );
    sXCD.appendAscii( "\t\t\t\t\t<schema:description>Specifies the mime type </schema:description>\n"                                                                                                                           );
    sXCD.appendAscii( "\t\t\t\t</schema:documentation>\n"                                                                                                                                                                       );
    sXCD.appendAscii( "\t\t\t</schema:value>\n"                                                                                                                                                                                 );

    sXCD.appendAscii( "\t\t\t<schema:value cfg:name=\"ClipboardFormat\" cfg:type=\"string\" cfg:writable=\""                                                                                                                    );
    sXCD.appendAscii( bWriteable==sal_True ? "true\">\n" : "false\">\n"                                                                                                                                                         );
    sXCD.appendAscii( "\t\t\t\t<schema:documentation>\n"                                                                                                                                                                        );
    sXCD.appendAscii( "\t\t\t\t\t<schema:description>Specifies the clipboard format name</schema:description>\n"                                                                                                                );
    sXCD.appendAscii( "\t\t\t\t</schema:documentation>\n"                                                                                                                                                                       );
    sXCD.appendAscii( "\t\t\t</schema:value>\n"                                                                                                                                                                                 );

    sXCD.appendAscii( "\t\t\t<schema:value cfg:name=\"URLPattern\" cfg:type=\"string\" cfg:derivedBy=\"list\" cfg:writable=\""                                                                                                  );
    sXCD.appendAscii( bWriteable==sal_True ? "true\">\n" : "false\">\n"                                                                                                                                                         );
    sXCD.appendAscii( "\t\t\t\t<schema:documentation>\n"                                                                                                                                                                        );
    sXCD.appendAscii( "\t\t\t\t\t<schema:description>Specifies the patterns used for URLs. This type is only relevant for HTTP, FTP etc. and is used for internal URL formats like private:factory etc.</schema:description>\n" );
    sXCD.appendAscii( "\t\t\t\t</schema:documentation>\n"                                                                                                                                                                       );
    sXCD.appendAscii( "\t\t\t</schema:value>\n"                                                                                                                                                                                 );

    sXCD.appendAscii( "\t\t\t<schema:value cfg:name=\"Extensions\" cfg:type=\"string\" cfg:derivedBy=\"list\" cfg:writable=\""                                                                                                  );
    sXCD.appendAscii( bWriteable==sal_True ? "true\">\n" : "false\">\n"                                                                                                                                                         );
    sXCD.appendAscii( "\t\t\t\t<schema:documentation>\n"                                                                                                                                                                        );
    sXCD.appendAscii( "\t\t\t\t\t<schema:description>Specifies the possible file extensions.</schema:description>\n"                                                                                                            );
    sXCD.appendAscii( "\t\t\t\t</schema:documentation>\n"                                                                                                                                                                       );
    sXCD.appendAscii( "\t\t\t</schema:value>\n"                                                                                                                                                                                 );

    sXCD.appendAscii( "\t\t\t<schema:value cfg:name=\"DocumentIconID\" cfg:type=\"int\" cfg:writable=\""                                                                                                                        );
    sXCD.appendAscii( bWriteable==sal_True ? "true\">\n" : "false\">\n"                                                                                                                                                         );
    sXCD.appendAscii( "\t\t\t\t<schema:documentation>\n"                                                                                                                                                                        );
    sXCD.appendAscii( "\t\t\t\t\t<schema:description>Specifies the document icon ID of this type</schema:description>\n"                                                                                                        );
    sXCD.appendAscii( "\t\t\t\t</schema:documentation>\n"                                                                                                                                                                       );
    sXCD.appendAscii( "\t\t\t\t<default:data>0</default:data>\n"                                                                                                                                                                );
    sXCD.appendAscii( "\t\t\t</schema:value>\n"                                                                                                                                                                                 );

    sXCD.appendAscii( "\t\t</schema:group>\n"                                                                                                                                                                                   );
}
//_________________________________________________________________________________________________________________
else if( m_aData.nVersionOutput==3 )
{
    sXCD.appendAscii( "\t\t<schema:group cfg:name=\"Type\">\n"                                                                                                                                                                  );

    sXCD.appendAscii( "\t\t\t<schema:value cfg:name=\"UIName\" cfg:type=\"string\" cfg:localized=\"true\" cfg:writable=\""                                                                                                      );
    sXCD.appendAscii( bWriteable==sal_True ? "true\">\n" : "false\">\n"                                                                                                                                                         );
    sXCD.appendAscii( "\t\t\t\t<schema:documentation>\n"                                                                                                                                                                        );
    sXCD.appendAscii( "\t\t\t\t\t<schema:description>Specifies the external name of this type</schema:description>\n"                                                                                                           );
    sXCD.appendAscii( "\t\t\t\t</schema:documentation>\n"                                                                                                                                                                       );
    sXCD.appendAscii( "\t\t\t</schema:value>\n"                                                                                                                                                                                 );

    sXCD.appendAscii( "\t\t\t<schema:value cfg:name=\"Data\" cfg:type=\"string\" cfg:writable=\""                                                                                                                               );
    sXCD.appendAscii( bWriteable==sal_True ? "true\">\n" : "false\">\n"                                                                                                                                                         );
    sXCD.appendAscii("\t\t\t\t<schema:documentation>\n"                                                                                                                                                                         );
    sXCD.appendAscii( "\t\t\t\t\t<schema:description>Containes all data of a type as an own formated string.{Preferred, MediaType, ClipboardFormat, URLPattern, Extensions, DocumentIconID}</schema:description>\n"             );
    sXCD.appendAscii( "\t\t\t\t</schema:documentation>\n"                                                                                                                                                                       );
    sXCD.appendAscii( "\t\t\t\t<default:data>false</default:data>\n"                                                                                                                                                            );
    sXCD.appendAscii( "\t\t\t</schema:value>\n"                                                                                                                                                                                 );

    sXCD.appendAscii( "\t\t</schema:group>\n"                                                                                                                                                                                   );
}
}

//*****************************************************************************************************************
void XCDGenerator::impl_generateFilterTemplate( ::rtl::OUStringBuffer& sXCD, sal_Bool bWriteable )
{
//_________________________________________________________________________________________________________________
if( m_aData.nVersionOutput==1 || m_aData.nVersionOutput==2 )
{
    sXCD.appendAscii( "\t\t<schema:group cfg:name=\"Filter\">\n"                                                                                                            );

    sXCD.appendAscii( "\t\t\t<schema:value cfg:name=\"Installed\" cfg:type=\"boolean\" cfg:writable=\""                                                                     );
    sXCD.appendAscii( bWriteable==sal_True ? "true\">\n" : "false\">\n"                                                                                                     );
    sXCD.appendAscii( "\t\t\t\t<schema:documentation>\n"                                                                                                                    );
    sXCD.appendAscii( "\t\t\t\t\t<schema:description>Make it possible to enable or disable filter by setup!</schema:description>\n"                                         );
    sXCD.appendAscii( "\t\t\t\t</schema:documentation>\n"                                                                                                                   );
    sXCD.appendAscii( "\t\t\t\t<default:data>false</default:data>\n"                                                                                                        );
    sXCD.appendAscii( "\t\t\t</schema:value>\n"                                                                                                                             );

    if( m_aData.nVersionOutput==2 )
    {
    sXCD.appendAscii( "\t\t\t<schema:value cfg:name=\"Order\" cfg:type=\"int\" cfg:writable=\""                                                                             );
    sXCD.appendAscii( bWriteable==sal_True ? "true\">\n" : "false\">\n"                                                                                                     );
    sXCD.appendAscii( "\t\t\t\t<schema:documentation>\n"                                                                                                                    );
    sXCD.appendAscii( "\t\t\t\t\t<schema:description>Specifies order of filters for relevant module; don't used for default filter!</schema:description>\n"                 );
    sXCD.appendAscii( "\t\t\t\t</schema:documentation>\n"                                                                                                                   );
    sXCD.appendAscii( "\t\t\t\t<default:data>0</default:data>\n"                                                                                                            );
    sXCD.appendAscii( "\t\t\t</schema:value>\n"                                                                                                                             );
    }

    sXCD.appendAscii( "\t\t\t<schema:value cfg:name=\"UIName\" cfg:type=\"string\" cfg:localized=\"true\" cfg:writable=\""                                                  );
    sXCD.appendAscii( bWriteable==sal_True ? "true\">\n" : "false\">\n"                                                                                                     );
    sXCD.appendAscii( "\t\t\t\t<schema:documentation>\n"                                                                                                                    );
    sXCD.appendAscii( "\t\t\t\t\t<schema:description>Specifies the external name of the filter which is displayed at the user interface (dialog).</schema:description>\n"   );
    sXCD.appendAscii( "\t\t\t\t</schema:documentation>\n"                                                                                                                   );
    sXCD.appendAscii( "\t\t\t</schema:value>\n"                                                                                                                             );

    sXCD.appendAscii( "\t\t\t<schema:value cfg:name=\"Type\" cfg:type=\"string\" cfg:writable=\""                                                                           );
    sXCD.appendAscii( bWriteable==sal_True ? "true\">\n" : "false\">\n"                                                                                                     );
    sXCD.appendAscii( "\t\t\t\t<schema:documentation>\n"                                                                                                                    );
    sXCD.appendAscii( "\t\t\t\t\t<schema:description>Specifies the relative type key name of the filter, e.g. Type/T1</schema:description>\n"                               );
    sXCD.appendAscii( "\t\t\t\t</schema:documentation>\n"                                                                                                                   );
    sXCD.appendAscii( "\t\t\t</schema:value>\n"                                                                                                                             );

    sXCD.appendAscii( "\t\t\t<schema:value cfg:name=\"DocumentService\" cfg:type=\"string\" cfg:writable=\""                                                                );
    sXCD.appendAscii( bWriteable==sal_True ? "true\">\n" : "false\">\n"                                                                                                     );
    sXCD.appendAscii( "\t\t\t\t<schema:documentation>\n"                                                                                                                    );
    sXCD.appendAscii( "\t\t\t\t\t<schema:description>Specifies the name of the UNO service to implement the document.</schema:description>\n"                               );
    sXCD.appendAscii( "\t\t\t\t</schema:documentation>\n"                                                                                                                   );
    sXCD.appendAscii( "\t\t\t</schema:value>\n"                                                                                                                             );

    sXCD.appendAscii( "\t\t\t<schema:value cfg:name=\"FilterService\" cfg:type=\"string\" cfg:writable=\""                                                                  );
    sXCD.appendAscii( bWriteable==sal_True ? "true\">\n" : "false\">\n"                                                                                                     );
    sXCD.appendAscii( "\t\t\t\t<schema:documentation>\n"                                                                                                                    );
    sXCD.appendAscii( "\t\t\t\t\t<schema:description>Specifies the name of the UNO service for importing the document.</schema:description>\n"                              );
    sXCD.appendAscii( "\t\t\t\t</schema:documentation>\n"                                                                                                                   );
    sXCD.appendAscii( "\t\t\t</schema:value>\n"                                                                                                                             );

    sXCD.appendAscii( "\t\t\t<schema:value cfg:name=\"Flags\" cfg:type=\"int\" cfg:writable=\""                                                                             );
    sXCD.appendAscii( bWriteable==sal_True ? "true\">\n" : "false\">\n"                                                                                                     );
    sXCD.appendAscii( "\t\t\t\t<schema:documentation>\n"                                                                                                                    );
    sXCD.appendAscii( "\t\t\t\t\t<schema:description>Specifies the properties of the filter</schema:description>\n"                                                         );
    sXCD.appendAscii( "\t\t\t\t</schema:documentation>\n"                                                                                                                   );
    sXCD.appendAscii( "\t\t\t\t<schema:type-info>\n"                                                                                                                        );
    sXCD.appendAscii( "\t\t\t\t\t<schema:value-names>\n"                                                                                                                    );
    impl_generateFilterFlagTemplate( sXCD, FILTERFLAGNAME_IMPORT            , FILTERFLAG_IMPORT                                                                             );
    impl_generateFilterFlagTemplate( sXCD, FILTERFLAGNAME_EXPORT            , FILTERFLAG_EXPORT                                                                             );
    impl_generateFilterFlagTemplate( sXCD, FILTERFLAGNAME_TEMPLATE          , FILTERFLAG_TEMPLATE                                                                           );
    impl_generateFilterFlagTemplate( sXCD, FILTERFLAGNAME_INTERNAL          , FILTERFLAG_INTERNAL                                                                           );
    impl_generateFilterFlagTemplate( sXCD, FILTERFLAGNAME_TEMPLATEPATH      , FILTERFLAG_TEMPLATEPATH                                                                       );
    impl_generateFilterFlagTemplate( sXCD, FILTERFLAGNAME_OWN               , FILTERFLAG_OWN                                                                                );
    impl_generateFilterFlagTemplate( sXCD, FILTERFLAGNAME_ALIEN             , FILTERFLAG_ALIEN                                                                              );
    impl_generateFilterFlagTemplate( sXCD, FILTERFLAGNAME_USESOPTIONS       , FILTERFLAG_USESOPTIONS                                                                        );
    impl_generateFilterFlagTemplate( sXCD, FILTERFLAGNAME_DEFAULT           , FILTERFLAG_DEFAULT                                                                            );
    impl_generateFilterFlagTemplate( sXCD, FILTERFLAGNAME_NOTINFILEDIALOG   , FILTERFLAG_NOTINFILEDIALOG                                                                    );
    impl_generateFilterFlagTemplate( sXCD, FILTERFLAGNAME_NOTINCHOOSER      , FILTERFLAG_NOTINCHOOSER                                                                       );
    impl_generateFilterFlagTemplate( sXCD, FILTERFLAGNAME_ASYNCHRON         , FILTERFLAG_ASYNCHRON                                                                          );
    impl_generateFilterFlagTemplate( sXCD, FILTERFLAGNAME_NOTINSTALLED      , FILTERFLAG_NOTINSTALLED   , "set, if the filter is not installed, but available on CD"        );
    impl_generateFilterFlagTemplate( sXCD, FILTERFLAGNAME_CONSULTSERVICE    , FILTERFLAG_CONSULTSERVICE , "set, if the filter is not installed and not available an CD"     );
    impl_generateFilterFlagTemplate( sXCD, FILTERFLAGNAME_3RDPARTYFILTER    , FILTERFLAG_3RDPARTYFILTER , "must set, if the filter is an external one"                      );
    impl_generateFilterFlagTemplate( sXCD, FILTERFLAGNAME_PACKED            , FILTERFLAG_PACKED                                                                             );
    impl_generateFilterFlagTemplate( sXCD, FILTERFLAGNAME_SILENTEXPORT      , FILTERFLAG_SILENTEXPORT                                                                       );
    impl_generateFilterFlagTemplate( sXCD, FILTERFLAGNAME_PREFERED          , FILTERFLAG_PREFERED                                                                           );
    sXCD.appendAscii( "\t\t\t\t\t</schema:value-names>\n"                                                                                                                   );
    sXCD.appendAscii( "\t\t\t\t\t<schema:constraints xmlns:xsd=\"http://www.w3.org/1999/XMLSchema\"/>\n"                                                                    );
    sXCD.appendAscii( "\t\t\t\t</schema:type-info>\n"                                                                                                                       );
    sXCD.appendAscii( "\t\t\t\t<default:data>0</default:data>\n"                                                                                                            );
    sXCD.appendAscii( "\t\t\t</schema:value>\n"                                                                                                                             );

    sXCD.appendAscii( "\t\t\t<schema:value cfg:name=\"UserData\" cfg:type=\"string\" cfg:derivedBy=\"list\" cfg:writable=\""                                                );
    sXCD.appendAscii( bWriteable==sal_True ? "true\">\n" : "false\">\n"                                                                                                     );
    sXCD.appendAscii( "\t\t\t\t<schema:documentation>\n"                                                                                                                    );
    sXCD.appendAscii( "\t\t\t\t\t<schema:description>Specifies the user-defined data</schema:description>\n"                                                                );
    sXCD.appendAscii( "\t\t\t\t</schema:documentation>\n"                                                                                                                   );
    sXCD.appendAscii( "\t\t\t\t<default:data/>\n"                                                                                                                           );
    sXCD.appendAscii( "\t\t\t</schema:value>\n"                                                                                                                             );

    sXCD.appendAscii( "\t\t\t<schema:value cfg:name=\"FileFormatVersion\" cfg:type=\"int\" cfg:writable=\""                                                                 );
    sXCD.appendAscii( bWriteable==sal_True ? "true\">\n" : "false\">\n"                                                                                                     );
    sXCD.appendAscii( "\t\t\t\t<!--This should be removed to UserData later-->\n"                                                                                           );
    sXCD.appendAscii( "\t\t\t\t<schema:documentation>\n"                                                                                                                    );
    sXCD.appendAscii( "\t\t\t\t\t<schema:description>Specifies the file format version of the filter</schema:description>\n"                                                );
    sXCD.appendAscii( "\t\t\t\t</schema:documentation>\n"                                                                                                                   );
    sXCD.appendAscii( "\t\t\t\t<default:data>0</default:data>\n"                                                                                                            );
    sXCD.appendAscii( "\t\t\t</schema:value>\n"                                                                                                                             );

    sXCD.appendAscii( "\t\t\t<schema:value cfg:name=\"TemplateName\" cfg:type=\"string\" cfg:writable=\""                                                                   );
    sXCD.appendAscii( bWriteable==sal_True ? "true\">\n" : "false\">\n"                                                                                                     );
    sXCD.appendAscii( "\t\t\t\t<!--This should be removed to UserData later-->\n"                                                                                           );
    sXCD.appendAscii( "\t\t\t\t<schema:documentation>\n"                                                                                                                    );
    sXCD.appendAscii( "\t\t\t\t\t<schema:description>Specifies the template used for importing the file with the specified filter.</schema:description>\n"                  );
    sXCD.appendAscii( "\t\t\t\t\t</schema:documentation>\n"                                                                                                                 );
    sXCD.appendAscii( "\t\t\t</schema:value>\n"                                                                                                                             );

    sXCD.appendAscii( "\t\t</schema:group>\n"                                                                                                                               );
//_________________________________________________________________________________________________________________
}
else if( m_aData.nVersionOutput==3 )
{
    sXCD.appendAscii( "\t\t<schema:group cfg:name=\"Filter\">\n"                                                                                                                                                                    );

    sXCD.appendAscii( "\t\t\t<schema:value cfg:name=\"Installed\" cfg:type=\"boolean\" cfg:writable=\""                                                                                                                             );
    sXCD.appendAscii( bWriteable==sal_True ? "true\">\n" : "false\">\n"                                                                                                                                                             );
    sXCD.appendAscii( "\t\t\t\t<schema:documentation>\n"                                                                                                                                                                            );
    sXCD.appendAscii( "\t\t\t\t\t<schema:description>Make it possible to enable or disable filter by setup!</schema:description>\n"                                                                                                 );
    sXCD.appendAscii( "\t\t\t\t</schema:documentation>\n"                                                                                                                                                                           );
    sXCD.appendAscii( "\t\t\t\t<default:data>false</default:data>\n"                                                                                                                                                                );
    sXCD.appendAscii( "\t\t\t</schema:value>\n"                                                                                                                                                                                     );

    sXCD.appendAscii( "\t\t\t<schema:value cfg:name=\"UIName\" cfg:type=\"string\" cfg:localized=\"true\" cfg:writable=\""                                                                                                          );
    sXCD.appendAscii( bWriteable==sal_True ? "true\">\n" : "false\">\n"                                                                                                                                                             );
    sXCD.appendAscii( "\t\t\t\t<schema:documentation>\n"                                                                                                                                                                            );
    sXCD.appendAscii( "\t\t\t\t\t<schema:description>Specifies the external name of the filter which is displayed at the user interface (dialog).</schema:description>\n"                                                           );
    sXCD.appendAscii( "\t\t\t\t</schema:documentation>\n"                                                                                                                                                                           );
    sXCD.appendAscii( "\t\t\t</schema:value>\n"                                                                                                                                                                                     );

    sXCD.appendAscii( "\t\t\t<schema:value cfg:name=\"Data\" cfg:type=\"string\" cfg:writable=\""                                                                                                                                   );
    sXCD.appendAscii( bWriteable==sal_True ? "true\">\n" : "false\">\n"                                                                                                                                                             );
    sXCD.appendAscii( "\t\t\t\t<schema:documentation>\n"                                                                                                                                                                            );
    sXCD.appendAscii( "\t\t\t\t\t<schema:description>All data of filter written in own format. {Order, OldName, Type, DocumentService, FilterService, Flags, UserData, FilteFormatVersion, TemplateName}</schema:description>\n"    );
    sXCD.appendAscii( "\t\t\t\t</schema:documentation>\n"                                                                                                                                                                           );
    sXCD.appendAscii( "\t\t\t</schema:value>\n"                                                                                                                                                                                     );

    sXCD.appendAscii( "\t\t</schema:group>\n"                                                                                                                                                                                       );
}
}

//*****************************************************************************************************************
void XCDGenerator::impl_generateFilterFlagTemplate( ::rtl::OUStringBuffer& sXCD, const ::rtl::OUString& sName, sal_Int32 nValue, const ::rtl::OString& sDescription )
{
    sXCD.appendAscii( "\t\t\t\t\t\t<schema:named-value name=\"" );
    sXCD.append     ( sName                                     );
    sXCD.appendAscii( "\" value=\""                             );
    sXCD.append     ( nValue                                    );
    sXCD.appendAscii( "\""                                      );

    if( sDescription.getLength() > 0 )
    {
        sXCD.appendAscii( ">\n\t\t\t\t\t\t\t<schema:description>"   );
        sXCD.appendAscii( sDescription                              );
        sXCD.appendAscii( "</schema:description>\n"                 );
        sXCD.appendAscii( "\t\t\t\t\t\t</schema:named-value>\n"     );
    }
    else
    {
        sXCD.appendAscii( "/>\n"                                    );
    }
}

//*****************************************************************************************************************
void XCDGenerator::impl_generateDetectorTemplate( ::rtl::OUStringBuffer& sXCD, sal_Bool bWriteable )
{
    sXCD.appendAscii( "\t\t<schema:group cfg:name=\"DetectService\">\n"                                                             );
    sXCD.appendAscii( "\t\t\t<schema:value cfg:name=\"Types\" cfg:type=\"string\" cfg:derivedBy=\"list\" cfg:writable=\""           );
    sXCD.appendAscii( bWriteable==sal_True ? "true\">\n" : "false\">\n"                                                             );
    sXCD.appendAscii( "\t\t\t\t<schema:documentation>\n"                                                                            );
    sXCD.appendAscii( "\t\t\t\t\t<schema:description>List of types which the service has registered for.</schema:description>\n"    );
    sXCD.appendAscii( "\t\t\t\t</schema:documentation>\n"                                                                           );
    sXCD.appendAscii( "\t\t\t</schema:value>\n"                                                                                     );
    sXCD.appendAscii( "\t\t</schema:group>\n"                                                                                       );
}

//*****************************************************************************************************************
void XCDGenerator::impl_generateLoaderTemplate( ::rtl::OUStringBuffer& sXCD, sal_Bool bWriteable )
{
    sXCD.appendAscii( "\t\t<schema:group cfg:name=\"FrameLoader\">\n"                                                                                                       );
    sXCD.appendAscii( "\t\t\t<schema:value cfg:name=\"UIName\" cfg:type=\"string\" cfg:localized=\"true\" cfg:writable=\""                                                  );
    sXCD.appendAscii( bWriteable==sal_True ? "true\">\n" : "false\">\n"                                                                                                     );
    sXCD.appendAscii( "\t\t\t\t<schema:documentation>\n"                                                                                                                    );
    sXCD.appendAscii( "\t\t\t\t\t<schema:description>Specifies the external name of the filter which is displayed at the user interface (dialog).</schema:description>\n"   );
    sXCD.appendAscii( "\t\t\t\t</schema:documentation>\n"                                                                                                                   );
    sXCD.appendAscii( "\t\t\t</schema:value>\n"                                                                                                                             );
    sXCD.appendAscii( "\t\t\t<schema:value cfg:name=\"Types\" cfg:type=\"string\" cfg:derivedBy=\"list\" cfg:writable=\""                                                   );
    sXCD.appendAscii( bWriteable==sal_True ? "true\">\n" : "false\">\n"                                                                                                     );
    sXCD.appendAscii( "\t\t\t\t<schema:documentation>\n"                                                                                                                    );
    sXCD.appendAscii( "\t\t\t\t\t<schema:description>List of types which the service has registered for.</schema:description>\n"                                            );
    sXCD.appendAscii( "\t\t\t\t</schema:documentation>\n"                                                                                                                   );
    sXCD.appendAscii( "\t\t\t</schema:value>\n"                                                                                                                             );
    sXCD.appendAscii( "\t\t</schema:group>\n"                                                                                                                               );
}

//*****************************************************************************************************************
void XCDGenerator::impl_generateTypeSet( ::rtl::OUStringBuffer& sXCD, sal_Bool bWriteable )
{
    if( m_aData.pFilterCache->hasTypes() == sal_False )
    {
        // generate empty set!
        sXCD.appendAscii( "\t<schema:set cfg:name=\"Types\" cfg:element-type=\"Type\"/>\n" );
    }
    else
    {
        // generate filled set
        // open set
        sXCD.appendAscii( "\t<schema:set cfg:name=\"Types\" cfg:element-type=\"Type\">\n" );

        if( m_aData.nVersionOutput==1 || m_aData.nVersionOutput==2 )
        {
            css::uno::Sequence< ::rtl::OUString > lNames = m_aData.pFilterCache->getAllTypeNames();
            sal_Int32                             nCount = lNames.getLength()                     ;
            for( sal_Int32 nItem=0; nItem<nCount; ++nItem )
            {
                ::rtl::OUString sName = lNames[nItem]                         ;
                FileType        aItem = m_aData.pFilterCache->getType( sName );

                ++m_aData.nWrittenTypes;

                // open set entry by using name
                sXCD.appendAscii( "\t\t<default:group cfg:name=\""  );
                sXCD.append     ( sName                             );
                sXCD.appendAscii( "\">\n"                           );

                // write properties
                impl_generateBoolProperty       ( sXCD, SUBKEY_PREFERRED        , aItem.bPreferred          , bWriteable );
                impl_generateUINamesProperty    ( sXCD, SUBKEY_UINAME           , aItem.lUINames            , bWriteable );
                impl_generateStringProperty     ( sXCD, SUBKEY_MEDIATYPE        , aItem.sMediaType          , bWriteable );
                impl_generateStringProperty     ( sXCD, SUBKEY_CLIPBOARDFORMAT  , aItem.sClipboardFormat    , bWriteable );
                impl_generateStringListProperty ( sXCD, SUBKEY_URLPATTERN       , aItem.lURLPattern         , bWriteable );
                impl_generateStringListProperty ( sXCD, SUBKEY_EXTENSIONS       , aItem.lExtensions         , bWriteable );
                impl_generateIntProperty        ( sXCD, SUBKEY_DOCUMENTICONID   , aItem.nDocumentIconID     , bWriteable );

                // close set node
                sXCD.appendAscii( "\t\t</default:group>\n" );
            }
        }
        else if( m_aData.nVersionOutput==3 )
        {
            css::uno::Sequence< ::rtl::OUString > lNames = m_aData.pFilterCache->getAllTypeNames();
            sal_Int32                             nCount = lNames.getLength()                     ;
            for( sal_Int32 nItem=0; nItem<nCount; ++nItem )
            {
                ::rtl::OUString sName = lNames[nItem]                         ;
                FileType        aItem = m_aData.pFilterCache->getType( sName );

                ++m_aData.nWrittenTypes;

                // open set entry by using name
                sXCD.appendAscii( "\t\t<default:group cfg:name=\""  );
                sXCD.append     ( sName                             );
                sXCD.appendAscii( "\">\n"                           );

                // write properties
                impl_generateUINamesProperty( sXCD, SUBKEY_UINAME, aItem.lUINames                          , bWriteable );
                impl_generateStringProperty ( sXCD, SUBKEY_DATA  , FilterCFGAccess::encodeTypeData( aItem ), bWriteable );

                // close set node
                sXCD.appendAscii( "\t\t</default:group>\n" );
            }
        }

        // close set
        sXCD.appendAscii( "\t</schema:set>\n" );
    }
}

//*****************************************************************************************************************
void XCDGenerator::impl_generateFilterSet( ::rtl::OUStringBuffer& sXCD, sal_Bool bWriteable )
{
    if( m_aData.pFilterCache->hasFilters() == sal_False )
    {
        // write empty filter set.
        sXCD.appendAscii( "\t<schema:set cfg:name=\"Filters\" cfg:element-type=\"Filter\"/>\n" );
    }
    else
    {
        // open set
        sXCD.appendAscii( "\t<schema:set cfg:name=\"Filters\" cfg:element-type=\"Filter\">\n" );

        if( m_aData.nVersionOutput==1 || m_aData.nVersionOutput==2 )
        {
            css::uno::Sequence< ::rtl::OUString > lNames = m_aData.pFilterCache->getAllFilterNames();
            sal_Int32                             nCount = lNames.getLength()                       ;
            for( sal_Int32 nItem=0; nItem<nCount; ++nItem )
            {
                Filter aItem = m_aData.pFilterCache->getFilter( lNames[nItem] );

                ++m_aData.nWrittenFilters;

                // open set node by using name
                sXCD.appendAscii( "\t\t<default:group cfg:name=\""  );
                sXCD.append     ( lNames[nItem]                     );
                sXCD.appendAscii( "\">\n"                           );

                // write properties
                // Attention:
                // We generate "Installed=false" for all entries ... because it's the default for all filters.
                // You must work with a full office installation and change this to "true" in generated XML file!!!
                impl_generateBoolProperty       ( sXCD, SUBKEY_INSTALLED        , sal_False               , bWriteable  );
                impl_generateIntProperty        ( sXCD, SUBKEY_ORDER            , aItem.nOrder            , bWriteable  );
                impl_generateStringProperty     ( sXCD, SUBKEY_TYPE             , aItem.sType             , bWriteable  );
                impl_generateUINamesProperty    ( sXCD, SUBKEY_UINAME           , aItem.lUINames          , bWriteable  );
                impl_generateStringProperty     ( sXCD, SUBKEY_DOCUMENTSERVICE  , aItem.sDocumentService  , bWriteable  );
                impl_generateStringProperty     ( sXCD, SUBKEY_FILTERSERVICE    , aItem.sFilterService    , bWriteable  );
                impl_generateIntProperty        ( sXCD, SUBKEY_FLAGS            , aItem.nFlags            , bWriteable  );
                impl_generateStringListProperty ( sXCD, SUBKEY_USERDATA         , aItem.lUserData         , bWriteable  );
                impl_generateIntProperty        ( sXCD, SUBKEY_FILEFORMATVERSION, aItem.nFileFormatVersion, bWriteable  );
                impl_generateStringProperty     ( sXCD, SUBKEY_TEMPLATENAME     , aItem.sTemplateName     , bWriteable  );

                // close set node
                sXCD.appendAscii( "\t\t</default:group>\n" );
            }
        }
        else if( m_aData.nVersionOutput==3 )
        {
            css::uno::Sequence< ::rtl::OUString > lNames = m_aData.pFilterCache->getAllFilterNames();
            sal_Int32                             nCount = lNames.getLength()                       ;
            for( sal_Int32 nItem=0; nItem<nCount; ++nItem )
            {
                ::rtl::OUString     sNewName;
                ::rtl::OUString     sOldName;

                sNewName = lNames[nItem];
                sOldName = impl_getOldFilterName  ( sNewName );
                sOldName = impl_filterSpecialSigns( sOldName );

                Filter aItem = m_aData.pFilterCache->getFilter( sNewName );

                ++m_aData.nWrittenFilters;

                // open set node by using name
                sXCD.appendAscii( "\t\t<default:group cfg:name=\""               );
                sXCD.append     ( FilterCFGAccess::encodeFilterName( sOldName )  );
                sXCD.appendAscii( "\">\n"                                        );

                // write properties
                // Attention:
                // We generate "Installed=false" for all entries ... because it's the default for all filters.
                // You must work with a full office installation and change this to "true" in generated XML file!!!
                impl_generateBoolProperty   ( sXCD, SUBKEY_INSTALLED, sal_False                                 , bWriteable  );
                impl_generateUINamesProperty( sXCD, SUBKEY_UINAME   , aItem.lUINames                            , bWriteable  );
                impl_generateStringProperty ( sXCD, SUBKEY_DATA     , FilterCFGAccess::encodeFilterData( aItem ), bWriteable  );

                // close set node
                sXCD.appendAscii( "\t\t</default:group>\n" );
            }
        }

        // close set
        sXCD.appendAscii( "\t</schema:set>\n" );
    }
}

//*****************************************************************************************************************
void XCDGenerator::impl_generateDetectorSet( ::rtl::OUStringBuffer& sXCD, sal_Bool bWriteable )
{
    if( m_aData.pFilterCache->hasDetectors() == sal_False )
    {
        // write empty detector set!
        sXCD.appendAscii( "\t<schema:set cfg:name=\"DetectServices\" cfg:element-type=\"DetectService\"/>\n" );
    }
    else
    {
        // open set
        sXCD.appendAscii( "\t<schema:set cfg:name=\"DetectServices\" cfg:element-type=\"DetectService\">\n" );

        css::uno::Sequence< ::rtl::OUString > lNames = m_aData.pFilterCache->getAllDetectorNames();
        sal_Int32                             nCount = lNames.getLength()                         ;
        for( sal_Int32 nItem=0; nItem<nCount; ++nItem )
        {
            ::rtl::OUString sName = lNames[nItem]                             ;
            Detector        aItem = m_aData.pFilterCache->getDetector( sName );

            ++m_aData.nWrittenDetectors;

            // open set node by using name
            sXCD.appendAscii( "\t\t<default:group cfg:name=\""  );
            sXCD.append     ( sName                             );
            sXCD.appendAscii( "\">\n"                           );

            // write properties
            impl_generateStringListProperty ( sXCD, SUBKEY_TYPES, aItem.lTypes, bWriteable );

            // close set node
            sXCD.appendAscii( "\t\t</default:group>\n" );
        }

        // close set
        sXCD.appendAscii( "\t</schema:set>\n" );
    }
}

//*****************************************************************************************************************
void XCDGenerator::impl_generateLoaderSet( ::rtl::OUStringBuffer& sXCD, sal_Bool bWriteable )
{
    if( m_aData.pFilterCache->hasLoaders() == sal_False )
    {
        // write empty loader set!
        sXCD.appendAscii( "\t<schema:set cfg:name=\"FrameLoaders\" cfg:element-type=\"FrameLoader\"/>\n" );
    }
    else
    {
        // open set
        sXCD.appendAscii( "\t<schema:set cfg:name=\"FrameLoaders\" cfg:element-type=\"FrameLoader\">\n" );

        css::uno::Sequence< ::rtl::OUString > lNames = m_aData.pFilterCache->getAllLoaderNames();
        sal_Int32                             nCount = lNames.getLength()                       ;
        for( sal_Int32 nItem=0; nItem<nCount; ++nItem )
        {
            ::rtl::OUString sName = lNames[nItem]                           ;
            Loader          aItem = m_aData.pFilterCache->getLoader( sName );

            ++m_aData.nWrittenLoaders;

            // open set node by using name
            sXCD.appendAscii( "\t\t<default:group cfg:name=\""  );
            sXCD.append     ( sName                             );
            sXCD.appendAscii( "\">\n"                           );

            // write properties
            impl_generateUINamesProperty    ( sXCD, SUBKEY_UINAME, aItem.lUINames, bWriteable );
            impl_generateStringListProperty ( sXCD, SUBKEY_TYPES , aItem.lTypes  , bWriteable );

            // close set node
            sXCD.appendAscii( "\t\t</default:group>\n" );
        }

        // close set
        sXCD.appendAscii( "\t</schema:set>\n" );
    }
}

//*****************************************************************************************************************
void XCDGenerator::impl_generateDefaults( ::rtl::OUStringBuffer& sXCD )
{
    // open group
    sXCD.appendAscii( "\t<schema:group cfg:name=\"Defaults\">\n" );

    // write generic loader
    sXCD.appendAscii( "\t\t<schema:value cfg:name=\"FrameLoader\" cfg:type=\"string\">\n"   );
    sXCD.appendAscii( "\t\t\t<default:data>"                                                );
    sXCD.append     ( m_aData.pFilterCache->getDefaultLoader()                              );
    sXCD.appendAscii( "</default:data>\n"                                                   );
    sXCD.appendAscii( "\t\t</schema:value>\n"                                               );

    // write default detector
    sXCD.appendAscii( "\t\t<schema:value cfg:name=\"DetectService\" cfg:type=\"string\">\n" );
    sXCD.appendAscii( "\t\t\t<default:data>"                                                );
    sXCD.append     ( m_aData.pFilterCache->getDefaultDetector()                            );
    sXCD.appendAscii( "</default:data>\n"                                                   );
    sXCD.appendAscii( "\t\t</schema:value>\n"                                               );

    // close group
    sXCD.appendAscii( "\t</schema:group>\n" );
}

//*****************************************************************************************************************
void XCDGenerator::impl_generateIntProperty(        ::rtl::OUStringBuffer& sXCD        ,
                                            const   ::rtl::OUString&       sName       ,
                                                    sal_Int32              nValue      ,
                                                    sal_Bool               bWriteable  )
{
    sXCD.appendAscii( "\t\t\t<default:value cfg:name=\""                );
    sXCD.append     ( sName                                             );
    sXCD.appendAscii( "\" cfg:type=\"int\" cfg:writable=\""             );
    sXCD.appendAscii( bWriteable==sal_True ? "true\">\n" : "false\">\n" );
    sXCD.appendAscii( "\t\t\t\t<default:data>"                          );
    sXCD.append     ( (sal_Int32)(nValue)                               );
    sXCD.appendAscii( "</default:data>\n\t\t\t</default:value>\n"       );
}

//*****************************************************************************************************************
void XCDGenerator::impl_generateBoolProperty(           ::rtl::OUStringBuffer& sXCD        ,
                                                const   ::rtl::OUString&       sName       ,
                                                        sal_Bool               bValue      ,
                                                        sal_Bool               bWriteable  )
{
    sXCD.appendAscii( "\t\t\t<default:value cfg:name=\""                );
    sXCD.append     ( sName                                             );
    sXCD.appendAscii( "\" cfg:type=\"boolean\" cfg:writable=\""         );
    sXCD.appendAscii( bWriteable==sal_True ? "true\">\n" : "false\">\n" );
    sXCD.appendAscii( "\t\t\t\t<default:data>"                          );
    sXCD.appendAscii( bValue==sal_True ? "true" : "false"               );
    sXCD.appendAscii( "</default:data>\n\t\t\t</default:value>\n"       );
}

//*****************************************************************************************************************
void XCDGenerator::impl_generateStringProperty(         ::rtl::OUStringBuffer& sXCD        ,
                                                const   ::rtl::OUString&       sName       ,
                                                const   ::rtl::OUString&       sValue      ,
                                                        sal_Bool               bWriteable  )
{
    sXCD.appendAscii( "\t\t\t<default:value cfg:name=\""            );
    sXCD.append     ( sName                                         );
    sXCD.appendAscii( "\" cfg:type=\"string\" cfg:writable=\""      );
    sXCD.appendAscii( bWriteable==sal_True ? "true\"" : "false\""   );
    if( sValue.getLength() > 0 )
    {
        sXCD.appendAscii( ">\n\t\t\t\t<default:data>"                   );
        sXCD.append     ( impl_filterSpecialSigns( sValue )             );
        sXCD.appendAscii( "</default:data>\n\t\t\t</default:value>\n"   );
    }
    else
    {
        sXCD.appendAscii( "/>\n" );
    }
}

//*****************************************************************************************************************
void XCDGenerator::impl_generateStringListProperty(         ::rtl::OUStringBuffer&      sXCD        ,
                                                    const   ::rtl::OUString&            sName       ,
                                                    const   ::framework::StringList&    lValue      ,
                                                            sal_Bool                    bWriteable  )
{
    sXCD.appendAscii( "\t\t\t<default:value cfg:name=\""                );
    sXCD.append     ( sName                                             );
    sXCD.appendAscii( "\" cfg:type=\"string\" cfg:derivedBy=\"list\""   );

    sal_Unicode cSeperator = impl_defineSeperator( lValue );
    if( cSeperator != ' ' )
    {
        sXCD.appendAscii( " cfg:separator=\""  );
        sXCD.append     ( cSeperator           );
        sXCD.appendAscii( "\""                 );
    }

    sXCD.appendAscii( " cfg:writable=\""                                );
    sXCD.appendAscii( bWriteable==sal_True ? "true\"" : "false\""       );

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
                                                const   StringHash&                 lUINames    ,
                                                        sal_Bool                    bWriteable  )
{
    sXCD.appendAscii( "\t\t\t<default:value cfg:name=\""                                );
    sXCD.append     ( sName                                                             );
    sXCD.appendAscii( "\" cfg:type=\"string\" cfg:localized=\"true\" cfg:writable=\""   );
    sXCD.appendAscii( bWriteable==sal_True ? "true\"" : "false\""                       );

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
                sXCD.appendAscii( "\t\t\t\t<default:data xml:lang=\""       );
                sXCD.append     ( pUIName->first                            );
                sXCD.appendAscii( "\">"                                     );
                sXCD.append     ( impl_filterSpecialSigns( pUIName->second ));
                sXCD.appendAscii( "</default:data>\n"                       );
            }
        }
        // Generate ONE entry as default for our configuration if all localized values are equal!
        else
        {
            sXCD.appendAscii( "\t\t\t\t<default:data xml:lang=\""                                       );
            sXCD.appendAscii( "en-US"                                                                   );
            sXCD.appendAscii( "\">"                                                                     );
            sXCD.append     ( impl_filterSpecialSigns( lUINames.find(DECLARE_ASCII("en-US"))->second )  );
            sXCD.appendAscii( "</default:data>\n"                                                       );
        }
        sXCD.appendAscii( "\t\t\t</default:value>\n" );
    }
    else
    {
        sXCD.appendAscii( "/>\n" );
    }
}

//*****************************************************************************************************************
::rtl::OUString XCDGenerator::impl_filterSpecialSigns( const ::rtl::OUString& sValue )
{
    ::rtl::OUStringBuffer  sSource     ( sValue );
    ::rtl::OUStringBuffer  sDestination( 10000  );
    sal_Int32       nCount      = sValue.getLength();
    sal_Int32       i           = 0;

    for( i=0; i<nCount; ++i )
    {
        sal_Unicode c = sSource.charAt(i);
        switch( c )
        {
            // replace "&" with "&amp;"
            case '&':   {
                            sDestination.appendAscii( "&amp;" );
                        }
                        break;
            // replace "<" with "&lt;"
            case '<':   {
                            sDestination.appendAscii( "&lt;"  );
                        }
                        break;
            // replace ">" with "&gt;"
            case '>':   {
                            sDestination.appendAscii( "&gt;"  );
                        }
                        break;
            // copy all other letters
            default :   {
                            sDestination.append( sSource.charAt(i) );
                        }
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
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_StarOffice_XML_Writer"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: StarOffice XML (Writer)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_StarWriter_50"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: StarWriter 5.0"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_StarWriter_50_VorlageTemplate"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: StarWriter 5.0 Vorlage/Template"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_StarWriter_40"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: StarWriter 4.0"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_StarWriter_40_VorlageTemplate"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: StarWriter 4.0 Vorlage/Template"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_StarWriter_30"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: StarWriter 3.0"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_StarWriter_30_VorlageTemplate"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: StarWriter 3.0 Vorlage/Template"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_StarWriter_20"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: StarWriter 2.0"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_StarWriter_10"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: StarWriter 1.0"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_StarWriter_DOS"))]   =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: StarWriter DOS"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_HTML_StarWriter"))]  =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: HTML (StarWriter)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Text"))] =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Text"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Text_Unix"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Text Unix"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Text_Mac"))] =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Text Mac"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Text_DOS"))] =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Text DOS"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Rich_Text_Format"))] =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Rich Text Format"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Word_97"))]   =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Word 97"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Word_95"))]   =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Word 95"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Word_97_Vorlage"))]   =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Word 97 Vorlage"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Word_95_Vorlage"))]   =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Word 95 Vorlage"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_WinWord_60"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS WinWord 6.0"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Word_6x_W4W"))]   =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Word 6.x (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_WinWord_5"))] =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS WinWord 5"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_WinWord_2x_W4W"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS WinWord 2.x (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_MacWord_5x_W4W"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS MacWord 5.x (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordPerfect_Win_61_W4W"))]   =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect (Win) 6.1 (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordPerfect_Win_70_W4W"))]   =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect (Win) 7.0 (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordStar_Win_1x_20_W4W"))]   =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordStar (Win) 1.x-2.0 (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordStar_70_W4W"))]  =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordStar 7.0  (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Ami_Pro_11_12_W4W"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Ami Pro 1.1-1.2 (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Ami_Pro_20_31_W4W"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Ami Pro 2.0-3.1 (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Excel_40_StarWriter"))]   =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Excel 4.0 (StarWriter)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Excel_50_StarWriter"))]   =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Excel 5.0 (StarWriter)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Excel_95_StarWriter"))]   =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Excel 95 (StarWriter)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Works_20_DOS_W4W"))]  =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Works 2.0 DOS (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Works_30_Win_W4W"))]  =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Works 3.0 Win (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Lotus_1_2_3_10_DOS_StarWriter"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Lotus 1-2-3 1.0 (DOS) (StarWriter)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Lotus_1_2_3_10_WIN_StarWriter"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Lotus 1-2-3 1.0 (WIN) (StarWriter)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Frame_Maker_MIF_50_W4W"))]   =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Frame Maker MIF 5.0 (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Win_Write_3x_W4W"))] =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Win Write 3.x (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Text_encoded"))] =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Text (encoded)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_WinWord_1x_W4W"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS WinWord 1.x (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Word_5x_W4W"))]   =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Word 5.x (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Word_4x_W4W"))]   =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Word 4.x (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Word_3x_W4W"))]   =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Word 3.x (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_MacWord_40_W4W"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS MacWord 4.0 (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_MacWord_30_W4W"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS MacWord 3.0 (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordPerfect_Mac_1_W4W"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect Mac 1 (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordPerfect_Mac_2_W4W"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect Mac 2 (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordPerfect_Mac_3_W4W"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect Mac 3 (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordPerfect_Win_51_52_W4W"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect (Win) 5.1-5.2 (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordPerfect_Win_60_W4W"))]   =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect (Win) 6.0 (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordPerfect_41_W4W"))]   =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect 4.1 (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordPerfect_42_W4W"))]   =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect 4.2 (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordPerfect_50_W4W"))]   =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect 5.0 (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordPerfect_51_W4W"))]   =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect 5.1 (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordPerfect_60_W4W"))]   =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect 6.0 (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordPerfect_61_W4W"))]   =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect 6.1 (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordStar_2000_Rel_30_W4W"))] =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordStar 2000 Rel. 3.0 (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordStar_2000_Rel_35_W4W"))] =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordStar 2000 Rel. 3.5 (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordStar_33x_W4W"))] =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordStar 3.3x (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordStar_345_W4W"))] =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordStar 3.45 (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordStar_40_W4W"))]  =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordStar 4.0  (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordStar_50_W4W"))]  =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordStar 5.0  (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordStar_55_W4W"))]  =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordStar 5.5  (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordStar_60_W4W"))]  =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordStar 6.0  (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Works_40_Mac_W4W"))]  =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Works 4.0 Mac (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Mac_Write_4x_50_W4W"))]  =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Mac Write 4.x 5.0 (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Mac_Write_II_W4W"))] =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Mac Write II (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Mac_Write_Pro_W4W"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Mac Write Pro (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Lotus_Manuscript_W4W"))] =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Lotus Manuscript (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MASS_11_Rel_80_83_W4W"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MASS 11 Rel. 8.0-8.3 (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MASS_11_Rel_85_90_W4W"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MASS 11 Rel. 8.5-9.0 (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Claris_Works_W4W"))] =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Claris Works (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_CTOS_DEF_W4W"))] =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: CTOS DEF (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_OfficeWriter_40_W4W"))]  =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: OfficeWriter 4.0 (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_OfficeWriter_50_W4W"))]  =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: OfficeWriter 5.0 (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_OfficeWriter_6x_W4W"))]  =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: OfficeWriter 6.x (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_XyWrite_III_W4W"))]  =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: XyWrite III ( W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_XyWrite_IIIP_W4W"))] =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: XyWrite III+ ( W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_XyWrite_Signature_W4W"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: XyWrite Signature (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_XyWrite_Sig_Win_W4W"))]  =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: XyWrite Sig. (Win) (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_XyWrite_IV_W4W"))]   =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: XyWrite IV (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_XyWrite_Win_10_W4W"))]   =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: XyWrite (Win) 1.0 (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_XEROX_XIF_50_W4W"))] =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: XEROX XIF 5.0 (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_XEROX_XIF_50_Illustrator_W4W"))] =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: XEROX XIF 5.0 (Illustrator) (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_XEROX_XIF_60_Color_Bitmap_W4W"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: XEROX XIF 6.0 (Color Bitmap) (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_XEROX_XIF_60_Res_Graphic_W4W"))] =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: XEROX XIF 6.0 (Res Graphic) (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WriteNow_30_Macintosh_W4W"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WriteNow 3.0 (Macintosh) (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Writing_Assistant_W4W"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Writing Assistant (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_VolksWriter_Deluxe_W4W"))]   =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: VolksWriter Deluxe (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_VolksWriter_3_and_4_W4W"))]  =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: VolksWriter 3 and 4 (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MultiMate_33_W4W"))] =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MultiMate 3.3 (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MultiMate_Adv_36_W4W"))] =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MultiMate Adv. 3.6 (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MultiMate_Adv_II_37_W4W"))]  =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MultiMate Adv. II 3.7 (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MultiMate_4_W4W"))]  =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MultiMate 4 (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_NAVY_DIF_W4W"))] =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: NAVY DIF (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_PFS_Write_W4W"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: PFS Write (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_PFS_First_Choice_10_W4W"))]  =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: PFS First Choice 1.0 (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_PFS_First_Choice_20_W4W"))]  =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: PFS First Choice 2.0 (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_PFS_First_Choice_30_W4W"))]  =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: PFS First Choice 3.0 (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Professional_Write_10_W4W"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Professional Write 1.0 (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Professional_Write_2x_W4W"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Professional Write 2.x (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Professional_Write_Plus_W4W"))]  =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Professional Write Plus (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Peach_Text_W4W"))]   =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Peach Text (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_DCA_Revisable_Form_Text_W4W"))]  =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: DCA Revisable Form Text (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_DCA_with_Display_Write_5_W4W"))] =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: DCA with Display Write 5 (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_DCAFFT_Final_Form_Text_W4W"))]   =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: DCA/FFT-Final Form Text (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_DEC_DX_W4W"))]   =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: DEC DX (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_DEC_WPS_PLUS_W4W"))] =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: DEC WPS-PLUS (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_DisplayWrite_20_4x_W4W"))]   =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: DisplayWrite 2.0-4.x (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_DisplayWrite_5x_W4W"))]  =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: DisplayWrite 5.x (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_DataGeneral_CEO_Write_W4W"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: DataGeneral CEO Write (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_EBCDIC_W4W"))]   =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: EBCDIC (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Enable_W4W"))]   =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Enable (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Frame_Maker_MIF_30_W4W"))]   =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Frame Maker MIF 3.0 (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Frame_Maker_MIF_40_W4W"))]   =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Frame Maker MIF 4.0 (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Frame_Work_III_W4W"))]   =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Frame Work III (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Frame_Work_IV_W4W"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Frame Work IV  (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_HP_AdvanceWrite_Plus_W4W"))] =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: HP AdvanceWrite Plus (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_ICL_Office_Power_6_W4W"))]   =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: ICL Office Power 6 (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_ICL_Office_Power_7_W4W"))]   =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: ICL Office Power 7 (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Interleaf_W4W"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Interleaf (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Interleaf_5_6_W4W"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Interleaf 5 - 6 (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Legacy_Winstar_onGO_W4W"))]  =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Legacy Winstar onGO (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_QA_Write_10_30_W4W"))]   =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Q&A Write 1.0-3.0 (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_QA_Write_40_W4W"))]  =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Q&A Write 4.0 (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Rapid_File_10_W4W"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Rapid File 1.0 (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Rapid_File_12_W4W"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Rapid File 1.2 (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Samna_Word_IV_IV_Plus_W4W"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Samna Word IV-IV Plus (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Total_Word_W4W"))]   =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Total Word (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Uniplex_onGO_W4W"))] =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Uniplex onGO (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Uniplex_V7_V8_W4W"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Uniplex V7-V8 (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Wang_PC_W4W"))]  =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Wang PC (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Wang_II_SWP_W4W"))]  =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Wang II SWP (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Wang_WP_Plus_W4W"))] =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Wang WP Plus (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WITA_W4W"))] =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WITA (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WiziWord_30_W4W"))]  =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WiziWord 3.0 (W4W)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_web_HTML"))] =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: HTML"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_web_StarWriterWeb_50_VorlageTemplate"))] =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: StarWriter/Web 5.0 Vorlage/Template"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_web_StarWriterWeb_40_VorlageTemplate"))] =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: StarWriter/Web 4.0 Vorlage/Template"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_web_Text_StarWriterWeb"))]   =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: Text (StarWriter/Web)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_web_Text_DOS_StarWriterWeb"))]   =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: Text DOS (StarWriter/Web)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_web_Text_Mac_StarWriterWeb"))]   =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: Text Mac (StarWriter/Web)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_web_Text_Unix_StarWriterWeb"))]  =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: Text Unix (StarWriter/Web)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_web_StarWriter_50"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: StarWriter 5.0"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_web_StarWriter_40"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: StarWriter 4.0"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_web_StarWriter_30"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: StarWriter 3.0"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_web_Text_encoded"))] =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: Text (encoded)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_globaldocument_StarWriter_60GlobalDocument"))]   =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/GlobalDocument: StarOffice XML (GlobalDocument)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_globaldocument_StarWriter_50GlobalDocument"))]   =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/GlobalDocument: StarWriter 5.0/GlobalDocument"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_globaldocument_StarWriter_40GlobalDocument"))]   =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/GlobalDocument: StarWriter 4.0/GlobalDocument"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_globaldocument_StarWriter_50"))] =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/GlobalDocument: StarWriter 5.0"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_globaldocument_StarWriter_40"))] =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/GlobalDocument: StarWriter 4.0"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_globaldocument_StarWriter_30"))] =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/GlobalDocument: StarWriter 3.0"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_globaldocument_Text_encoded"))]  =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/GlobalDocument: Text (encoded)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("chart_StarOffice_XML_Chart"))]  =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("schart: StarOffice XML (Chart)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("chart_StarChart_50"))]  =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("schart: StarChart 5.0"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("chart_StarChart_40"))]  =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("schart: StarChart 4.0"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("chart_StarChart_30"))]  =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("schart: StarChart 3.0"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("calc_StarOffice_XML_Calc"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: StarOffice XML (Calc)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("calc_StarCalc_50"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: StarCalc 5.0"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("calc_StarCalc_50_VorlageTemplate"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: StarCalc 5.0 Vorlage/Template"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("calc_StarCalc_40"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: StarCalc 4.0"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("calc_StarCalc_40_VorlageTemplate"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: StarCalc 4.0 Vorlage/Template"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("calc_StarCalc_30"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: StarCalc 3.0"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("calc_StarCalc_30_VorlageTemplate"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: StarCalc 3.0 Vorlage/Template"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("calc_MS_Excel_97"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: MS Excel 97"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("calc_MS_Excel_97_VorlageTemplate"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: MS Excel 97 Vorlage/Template"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("calc_MS_Excel_95"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: MS Excel 95"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("calc_MS_Excel_95_VorlageTemplate"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: MS Excel 95 Vorlage/Template"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("calc_MS_Excel_5095"))]  =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: MS Excel 5.0/95"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("calc_MS_Excel_5095_VorlageTemplate"))]  =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: MS Excel 5.0/95 Vorlage/Template"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("calc_MS_Excel_40"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: MS Excel 4.0"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("calc_MS_Excel_40_VorlageTemplate"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: MS Excel 4.0 Vorlage/Template"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("calc_Rich_Text_Format_StarCalc"))]  =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: Rich Text Format (StarCalc)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("calc_SYLK"))]   =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: SYLK"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("calc_DIF"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: DIF"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("calc_HTML_StarCalc"))]  =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: HTML (StarCalc)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("calc_dBase"))]  =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: dBase"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("calc_Lotus"))]  =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: Lotus"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("calc_StarCalc_10"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: StarCalc 1.0"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("calc_Text_txt_csv_StarCalc"))]  =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: Text - txt - csv (StarCalc)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("impress_StarOffice_XML_Impress"))]  =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: StarOffice XML (Impress)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("impress_StarImpress_50"))]  =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: StarImpress 5.0"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("impress_StarImpress_50_Vorlage"))]  =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: StarImpress 5.0 Vorlage"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("impress_StarImpress_40"))]  =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: StarImpress 4.0"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("impress_StarImpress_40_Vorlage"))]  =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: StarImpress 4.0 Vorlage"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("impress_StarDraw_50_StarImpress"))] =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: StarDraw 5.0 (StarImpress)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("impress_StarDraw_50_Vorlage_StarImpress"))] =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: StarDraw 5.0 Vorlage (StarImpress)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("impress_StarDraw_30_StarImpress"))] =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: StarDraw 3.0 (StarImpress)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("impress_StarDraw_30_Vorlage_StarImpress"))] =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: StarDraw 3.0 Vorlage (StarImpress)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("impress_MS_PowerPoint_97"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: MS PowerPoint 97"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("impress_MS_PowerPoint_97_Vorlage"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: MS PowerPoint 97 Vorlage"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("impress_CGM_Computer_Graphics_Metafile"))]  =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: CGM - Computer Graphics Metafile"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("impress_StarImpress_50_packed"))]   =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: StarImpress 5.0 (packed)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("draw_StarOffice_XML_Draw"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: StarOffice XML (Draw)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("draw_GIF_Graphics_Interchange"))]   =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: GIF - Graphics Interchange"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("draw_PCD_Photo_CD"))]   =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: PCD - Photo CD"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("draw_PCX_Zsoft_Paintbrush"))]   =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: PCX - Zsoft Paintbrush"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("draw_PSD_Adobe_Photoshop"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: PSD - Adobe Photoshop"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("draw_PNG_Portable_Network_Graphic"))]   =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: PNG - Portable Network Graphic"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("draw_StarDraw_50"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: StarDraw 5.0"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("draw_PBM_Portable_Bitmap"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: PBM - Portable Bitmap"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("draw_PGM_Portable_Graymap"))]   =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: PGM - Portable Graymap"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("draw_PPM_Portable_Pixelmap"))]  =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: PPM - Portable Pixelmap"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("draw_RAS_Sun_Rasterfile"))] =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: RAS - Sun Rasterfile"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("draw_TGA_Truevision_TARGA"))]   =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: TGA - Truevision TARGA"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("draw_SGV_StarDraw_20"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: SGV - StarDraw 2.0"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("draw_TIF_Tag_Image_File"))] =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: TIF - Tag Image File"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("draw_SGF_StarOffice_Writer_SGF"))]  =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: SGF - StarOffice Writer SGF"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("draw_XPM"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: XPM"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("gif_Graphics_Interchange"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: GIF - Graphics Interchange"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("pcd_Photo_CD"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: PCD - Photo CD"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("pcx_Zsoft_Paintbrush"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: PCX - Zsoft Paintbrush"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("psd_Adobe_Photoshop"))] =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: PSD - Adobe Photoshop"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("png_Portable_Network_Graphic"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: PNG - Portable Network Graphic"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("pbm_Portable_Bitmap"))] =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: PBM - Portable Bitmap"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("pgm_Portable_Graymap"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: PGM - Portable Graymap"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ppm_Portable_Pixelmap"))]   =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: PPM - Portable Pixelmap"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ras_Sun_Rasterfile"))]  =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: RAS - Sun Rasterfile"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("tga_Truevision_TARGA"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: TGA - Truevision TARGA"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sgv_StarDraw_20"))] =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: SGV - StarDraw 2.0"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("tif_Tag_Image_File"))]  =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: TIF - Tag Image File"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sgf_StarOffice_Writer_SGF"))]   =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: SGF - StarOffice Writer SGF"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("xpm_XPM"))] =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: XPM"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("draw_StarDraw_50_Vorlage"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: StarDraw 5.0 Vorlage"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("draw_StarImpress_50_StarDraw"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: StarImpress 5.0 (StarDraw)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("draw_StarImpress_50_Vorlage_StarDraw"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: StarImpress 5.0 Vorlage (StarDraw)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("draw_StarImpress_40_StarDraw"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: StarImpress 4.0 (StarDraw)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("draw_StarImpress_40_Vorlage_StarDraw"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: StarImpress 4.0 Vorlage (StarDraw)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("draw_StarDraw_30"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: StarDraw 3.0"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("draw_StarDraw_30_Vorlage"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: StarDraw 3.0 Vorlage"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("draw_EMF_MS_Windows_Metafile"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: EMF - MS Windows Metafile"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("draw_MET_OS2_Metafile"))]   =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: MET - OS/2 Metafile"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("draw_DXF_AutoCAD_Interchange"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: DXF - AutoCAD Interchange"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("draw_EPS_Encapsulated_PostScript"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: EPS - Encapsulated PostScript"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("draw_WMF_MS_Windows_Metafile"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: WMF - MS Windows Metafile"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("draw_PCT_Mac_Pict"))]   =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: PCT - Mac Pict"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("draw_SVM_StarView_Metafile"))]  =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: SVM - StarView Metafile"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("draw_BMP_MS_Windows"))] =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: BMP - MS Windows"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("draw_JPG_JPEG"))]   =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: JPG - JPEG"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("draw_XBM_X_Consortium"))]   =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: XBM - X-Consortium"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("emf_MS_Windows_Metafile"))] =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: EMF - MS Windows Metafile"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("met_OS2_Metafile"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: MET - OS/2 Metafile"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("dxf_AutoCAD_Interchange"))] =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: DXF - AutoCAD Interchange"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("eps_Encapsulated_PostScript"))] =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: EPS - Encapsulated PostScript"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("wmf_MS_Windows_Metafile"))] =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: WMF - MS Windows Metafile"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("pct_Mac_Pict"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: PCT - Mac Pict"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("svm_StarView_Metafile"))]   =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: SVM - StarView Metafile"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("bmp_MS_Windows"))]  =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: BMP - MS Windows"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("jpg_JPEG"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: JPG - JPEG"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("xbm_X_Consortium"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: XBM - X-Consortium"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("math_StarOffice_XML_Math"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("smath: StarOffice XML (Math)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("math_MathML_XML_Math"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("smath: MathML XML (Math)"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("math_StarMath_50"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("smath: StarMath 5.0"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("math_StarMath_40"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("smath: StarMath 4.0"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("math_StarMath_30"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("smath: StarMath 3.0"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("math_StarMath_20"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("smath: StarMath 2.0"));
    aHash[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("math_MathType_3x"))]    =   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("smath: MathType 3.x"));
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
