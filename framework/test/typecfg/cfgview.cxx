/*************************************************************************
 *
 *  $RCSfile: cfgview.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 18:22:06 $
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

#ifndef __FRAMEWORK_QUERIES_H_
#include <queries.h>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
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

#ifndef __SGI_STL_HASH_MAP
#include <hash_map>
#endif

#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/msgbox.hxx>
#include <stdio.h>

//_________________________________________________________________________________________________________________
//  const
//_________________________________________________________________________________________________________________

#define RDBFILE                                     DECLARE_ASCII("typecfg.rdb" )
#define ARGUMENT_DIRNAME                            DECLARE_ASCII("-dir="       )           // argument for output directory
#define ARGUMENT_VERSION                            DECLARE_ASCII("-ver="       )           // argument for file version to read    [1|2|3]
#define ARGUMENTLENGTH                              5                                       // same length for all arguemnts make it easier to detect it :-)
#define ARGUMENTFOUND                               0                                       // OUString::compareTo returns 0 if searched string match given one

#define MENU_HTML                                   "menu.html"
#define BLANK_HTML                                  "blank.html"

#define FRAMESET_START_HTML                         "index.html"
#define FRAMESET_TYPES_HTML                         "fs_types.html"
#define FRAMESET_FILTERS_HTML                       "fs_filters.html"
#define FRAMESET_MODULFILTERS_HTML                  "fs_modulfilters.html"
#define FRAMESET_DETECTORS_HTML                     "fs_detectors.html"
#define FRAMESET_LOADERS_HTML                       "fs_loaders.html"
#define FRAMESET_INVALIDFILTERS_HTML                "fs_invalidfilters.html"
#define FRAMESET_INVALIDDETECTORS_HTML              "fs_invaliddetectors.html"
#define FRAMESET_INVALIDLOADERS_HTML                "fs_invalidloaders.html"
#define FRAMESET_DOUBLEFILTERUINAMES_HTML           "fs_doublefilteruinames.html"

#define ALLTYPES_HTML                               "alltypes.html"
#define ALLFILTERS_HTML                             "allfilters.html"
#define ALLDETECTORS_HTML                           "alldetectors.html"
#define ALLLOADERS_HTML                             "allloaders.html"

#define TYPEPROPERTIES_HTML                         "typeproperties.html"
#define FILTERPROPERTIES_HTML                       "filterproperties.html"
#define DETECTORPROPERTIES_HTML                     "detectorproperties.html"
#define LOADERPROPERTIES_HTML                       "loaderproperties.html"

#define INVALIDFILTERS_HTML                         "invalidfilters.html"
#define INVALIDDETECTORS_HTML                       "invaliddetectors.html"
#define INVALIDLOADERS_HTML                         "invalidloaders.html"

#define FILTERFLAGS_HTML                            "filterflags.html"
#define MODULFILTERS_HTML                           "modulfilters.html"
#define DOUBLEFILTERUINAMES_HTML                    "doublefilteruinames.html"

#define TARGET_MENU                                 "menu"
#define TARGET_VIEW                                 "view"
#define TARGET_LIST                                 "list"
#define TARGET_PROPERTIES                           "properties"

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

using namespace ::std                       ;
using namespace ::vos                       ;
using namespace ::rtl                       ;
using namespace ::framework                 ;
using namespace ::comphelper                ;
using namespace ::com::sun::star::uno       ;
using namespace ::com::sun::star::lang      ;
using namespace ::com::sun::star::container ;
using namespace ::com::sun::star::beans     ;

//_________________________________________________________________________________________________________________
//  defines
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

/*-***************************************************************************************************************/
struct AppMember
{
        Reference< XMultiServiceFactory >   xServiceManager     ;
        FilterCache*                        pCache              ;
        ::rtl::OUString                     sDirectory          ;
        sal_Int32                           nVersion            ;
};

/*-***************************************************************************************************************/
class CFGView : public Application
{
    //*************************************************************************************************************
    public:
        void Main();

    //*************************************************************************************************************
    private:
        void impl_parseCommandLine                  ( AppMember& rMember );
        void impl_generateHTMLView                  ();

    //*************************************************************************************************************
    private:
        void impl_printCopyright                    ();
        void impl_printSyntax                       ();
        void impl_generateTypeListHTML              ();
        void impl_generateFilterListHTML            ();
        void impl_generateFilterModulListHTML       ();
        void impl_generateDetectorListHTML          ();
        void impl_generateLoaderListHTML            ();
        void impl_generateInvalidFiltersHTML        ();
        void impl_generateInvalidDetectorsHTML      ();
        void impl_generateInvalidLoadersHTML        ();
        void impl_generateFilterFlagsHTML           ();
        void impl_generateDefaultFiltersHTML        ();
        void impl_generateDoubleFilterUINamesHTML   ();
        void impl_writeFile                         ( const ::rtl::OString& sFile, const ::rtl::OString& sContent );

    //*************************************************************************************************************
    private:
        AppMember m_aData ;

};  //  class CFGView

//_________________________________________________________________________________________________________________
//  global variables
//_________________________________________________________________________________________________________________

CFGView gApplication;

//*****************************************************************************************************************
void CFGView::Main()
{
    // Init global servicemanager and set it.
    // It's neccessary for other services ... e.g. configuration.
    ServiceManager aManager;
    ::comphelper::setProcessServiceFactory( aManager.getGlobalUNOServiceManager() );

    // Get optional commands from command line.
    impl_parseCommandLine( m_aData );

    // Read configuration and fill cache ... use given file version (see parameter "-ver=" too)
    m_aData.pCache = new FilterCache( m_aData.nVersion );

    // Generate view as html
    impl_generateHTMLView();

    // Free all used memory
    delete m_aData.pCache;
    m_aData.pCache = NULL;
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
void CFGView::impl_printCopyright()
{
    fprintf( stderr, "\n(c) Copyright by Sun microsystems, 2001\n" );
}

//*****************************************************************************************************************
void CFGView::impl_printSyntax()
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
                regulate creation of this html view. Follow arguments are supported:
                    "-dir=<directory for output files>"
                    "-ver=<version of input file>[1|2|3]"

    @seealso    -

    @param      "rMember", reference to struct of global application member to fill arguments in it
    @return     right filled member struct or unchanged struct if an error occure!

    @onerror    We do nothing - or warn programmer!
*//*-*************************************************************************************************************/
void CFGView::impl_parseCommandLine( AppMember& rMember )
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
        // look for "-dir="
        if( sArgument.compareTo( ARGUMENT_DIRNAME, ARGUMENTLENGTH ) == ARGUMENTFOUND )
        {
            rMember.sDirectory = sArgument.copy( ARGUMENTLENGTH, sArgument.getLength()-ARGUMENTLENGTH );
            ++nMinCount;
        }
        else
        //_____________________________________________________________________________________________________
        // look for "-ver="
        if( sArgument.compareTo( ARGUMENT_VERSION, ARGUMENTLENGTH ) == ARGUMENTFOUND )
        {
            ::rtl::OUString sVersion = sArgument.copy( ARGUMENTLENGTH, sArgument.getLength()-ARGUMENTLENGTH );
            rMember.nVersion = sVersion.toInt32();
            ++nMinCount;
        }

        ++nArgument;
    }

    // Show help if user don't call us right!
    if( nMinCount != 2 )
    {
        impl_printSyntax();
        exit(-1);
    }
}

//*****************************************************************************************************************
void CFGView::impl_generateHTMLView()
{
    //-------------------------------------------------------------------------------------------------------------
    // generate start frameset
    OUStringBuffer sStartFramesetHTML( 10000 );

    sStartFramesetHTML.appendAscii( "<html>\n\t<head>\n\t\t<title>\n\t\t\tTypeDetection CFG\n\t\t</title>\n\t</head>\n"     );  // open html
    sStartFramesetHTML.appendAscii( "\t\t<frameset rows=\"25%,75%\">\n"                                                     );  // open frameset
    sStartFramesetHTML.appendAscii( "\t\t\t<frame name=\""                                                                  );  // generate frame "menu"
    sStartFramesetHTML.appendAscii( TARGET_MENU                                                                             );
    sStartFramesetHTML.appendAscii( "\" src=\""                                                                             );
    sStartFramesetHTML.appendAscii( MENU_HTML                                                                               );
    sStartFramesetHTML.appendAscii( "\" title=\"List\">\n"                                                                  );
    sStartFramesetHTML.appendAscii( "\t\t\t<frame name=\""                                                                  );  // generate frame "view"
    sStartFramesetHTML.appendAscii( TARGET_VIEW                                                                             );
    sStartFramesetHTML.appendAscii( "\" src=\""                                                                             );
    sStartFramesetHTML.appendAscii( BLANK_HTML                                                                              );
    sStartFramesetHTML.appendAscii( "\" title=\"Properties\">\n"                                                            );
    sStartFramesetHTML.appendAscii( "\t\t</frameset>\n"                                                                     );  // close frameset
    sStartFramesetHTML.appendAscii( "</html>\n"                                                                             );  // close html

    impl_writeFile( FRAMESET_START_HTML, U2B(sStartFramesetHTML.makeStringAndClear()) );

    //-------------------------------------------------------------------------------------------------------------
    // generate blank html
    OUStringBuffer sBlankHTML( 10000 );

    sBlankHTML.appendAscii( "<html>\n\t<head>\n\t\t<title>\n\t\t\tBlank\n\t\t</title>\n\t</head>\n\t<body>\n\t</body>Please select ...\n</html>\n"  );  // open html

    impl_writeFile( BLANK_HTML, U2B(sBlankHTML.makeStringAndClear()) );

    //-------------------------------------------------------------------------------------------------------------
    // generate menu
    OUStringBuffer sMenuHTML( 10000 );

    sMenuHTML.appendAscii( "<html>\n\t<head>\n\t\t<title>\n\t\t\tMenu\n\t\t</title>\n\t</head>\n\t<body>\n"             );  // open html
    sMenuHTML.appendAscii( "\t\t<ul>\n"                                                                                 );  // open list

    sMenuHTML.appendAscii( "\t\t<li><a href=\""                                                                         );  // list entry for "All Types"
    sMenuHTML.appendAscii( FRAMESET_TYPES_HTML                                                                          );
    sMenuHTML.appendAscii( "\" target=\""                                                                               );
    sMenuHTML.appendAscii( TARGET_VIEW                                                                                  );
    sMenuHTML.appendAscii( "\">All Types</a></li>\n"                                                                    );

    sMenuHTML.appendAscii( "\t\t<li><a href=\""                                                                         );  // list entry for "All Filters"
    sMenuHTML.appendAscii( FRAMESET_FILTERS_HTML                                                                        );
    sMenuHTML.appendAscii( "\" target=\""                                                                               );
    sMenuHTML.appendAscii( TARGET_VIEW                                                                                  );
    sMenuHTML.appendAscii( "\">All Filters</a></li>\n"                                                                  );

    sMenuHTML.appendAscii( "\t\t<li><a href=\""                                                                         );  // list entry for "All Filters sorted by modules"
    sMenuHTML.appendAscii( FRAMESET_MODULFILTERS_HTML                                                                   );
    sMenuHTML.appendAscii( "\" target=\""                                                                               );
    sMenuHTML.appendAscii( TARGET_VIEW                                                                                  );
    sMenuHTML.appendAscii( "\">Filters by Moduls</a></li>\n"                                                            );

    sMenuHTML.appendAscii( "\t\t<li><a href=\""                                                                         );  // list entry for "All Detectors"
    sMenuHTML.appendAscii( FRAMESET_DETECTORS_HTML                                                                      );
    sMenuHTML.appendAscii( "\" target=\""                                                                               );
    sMenuHTML.appendAscii( TARGET_VIEW                                                                                  );
    sMenuHTML.appendAscii( "\">All Detector Services</a></li>\n"                                                        );

    sMenuHTML.appendAscii( "\t\t<li><a href=\""                                                                         );  // list entry for "All Loaders"
    sMenuHTML.appendAscii( FRAMESET_LOADERS_HTML                                                                        );
    sMenuHTML.appendAscii( "\" target=\""                                                                               );
    sMenuHTML.appendAscii( TARGET_VIEW                                                                                  );
    sMenuHTML.appendAscii( "\">All Loader Services</a></li>\n"                                                          );

    sMenuHTML.appendAscii( "\t\t<li><a href=\""                                                                         );  // list entry for "Invalid Filter"
    sMenuHTML.appendAscii( FRAMESET_INVALIDFILTERS_HTML                                                                 );
    sMenuHTML.appendAscii( "\" target=\""                                                                               );
    sMenuHTML.appendAscii( TARGET_VIEW                                                                                  );
    sMenuHTML.appendAscii( "\">Invalid Filter</a></li>\n"                                                               );

    sMenuHTML.appendAscii( "\t\t<li><a href=\""                                                                         );  // list entry for "Invalid Detect Services"
    sMenuHTML.appendAscii( FRAMESET_INVALIDDETECTORS_HTML                                                               );
    sMenuHTML.appendAscii( "\" target=\""                                                                               );
    sMenuHTML.appendAscii( TARGET_VIEW                                                                                  );
    sMenuHTML.appendAscii( "\">Invalid Detect Services</a></li>\n"                                                      );

    sMenuHTML.appendAscii( "\t\t<li><a href=\""                                                                         );  // list entry for "Double Filter UINames"
    sMenuHTML.appendAscii( FRAMESET_DOUBLEFILTERUINAMES_HTML                                                            );
    sMenuHTML.appendAscii( "\" target=\""                                                                               );
    sMenuHTML.appendAscii( TARGET_VIEW                                                                                  );
    sMenuHTML.appendAscii( "\">Double Filter UINames</a></li>\n"                                                        );

    sMenuHTML.appendAscii( "\t\t<li><a href=\""                                                                         );  // list entry for "Show Filter Flags"
    sMenuHTML.appendAscii( FILTERFLAGS_HTML                                                                             );
    sMenuHTML.appendAscii( "\" target=\""                                                                               );
    sMenuHTML.appendAscii( TARGET_VIEW                                                                                  );
    sMenuHTML.appendAscii( "\">Show Filter Flags</a></li>\n"                                                            );

    sMenuHTML.appendAscii( "\t\t</ul>\n"                                                                                );  // close list
    sMenuHTML.appendAscii( "\t</body>\n</html>\n"                                                                       );  // close html

    impl_writeFile( MENU_HTML, U2B(sMenuHTML.makeStringAndClear()) );

    impl_generateTypeListHTML           ();
    impl_generateFilterListHTML         ();
    impl_generateFilterModulListHTML    ();
    impl_generateDetectorListHTML       ();
    impl_generateLoaderListHTML         ();
    impl_generateInvalidFiltersHTML     ();
    impl_generateInvalidDetectorsHTML   ();
    impl_generateInvalidLoadersHTML     ();
    impl_generateFilterFlagsHTML        ();
    impl_generateDoubleFilterUINamesHTML();
}

//*****************************************************************************************************************
void CFGView::impl_generateTypeListHTML()
{
    //-------------------------------------------------------------------------------------------------------------
    // generate frameset for types
    OUStringBuffer sTypesFramesetHTML( 10000 );

    sTypesFramesetHTML.appendAscii( "<html>\n\t<head>\n\t\t<title>\n\t\t\tFrameset: Types\n\t\t</title>\n\t</head>\n"       );  // open html
    sTypesFramesetHTML.appendAscii( "\t\t<frameset cols=\"40%,60%\">\n"                                                     );  // open frameset for cols
    sTypesFramesetHTML.appendAscii( "\t\t\t<frame name=\""                                                                  );  // generate frame "list"
    sTypesFramesetHTML.appendAscii( TARGET_LIST                                                                             );
    sTypesFramesetHTML.appendAscii( "\" src=\""                                                                             );
    sTypesFramesetHTML.appendAscii( ALLTYPES_HTML                                                                           );
    sTypesFramesetHTML.appendAscii( "\" title=\"List\">\n"                                                                  );
    sTypesFramesetHTML.appendAscii( "\t\t\t<frame name=\""                                                                  );  // generate frame "properties"
    sTypesFramesetHTML.appendAscii( TARGET_PROPERTIES                                                                       );
    sTypesFramesetHTML.appendAscii( "\" src=\""                                                                             );
    sTypesFramesetHTML.appendAscii( TYPEPROPERTIES_HTML                                                                     );
    sTypesFramesetHTML.appendAscii( "\" title=\"Properties\">\n"                                                            );
    sTypesFramesetHTML.appendAscii( "\t\t</frameset>\n"                                                                     );  // close frameset cols
    sTypesFramesetHTML.appendAscii( "</html>\n"                                                                             );  // close html

    impl_writeFile( FRAMESET_TYPES_HTML, U2B(sTypesFramesetHTML.makeStringAndClear()) );

    //-------------------------------------------------------------------------------------------------------------
    // generate type list (names and links only!)
    // use same loop to generate type property list!
    OUStringBuffer sAllTypesHTML( 10000 );
    OUStringBuffer sTypePropHTML( 10000 );

    sAllTypesHTML.appendAscii( "<html>\n\t<head>\n\t\t<title>\n\t\t\tAll Types\n\t\t</title>\n\t</head>\n\t<body>\n"                                        );  // open html
    sAllTypesHTML.appendAscii( "\t\t<table border=0><tr><td bgcolor=#ff8040><strong>Nr.</strong></td><td bgcolor=#ff8040><strong>Type</strong></td></tr>\n" );  // open table

    sTypePropHTML.appendAscii( "<html>\n\t<head>\n\t\t<title>\n\t\t\tTypeProperties\n\t\t</title>\n\t</head>\n\t<body>\n"                                   );  // open html

    css::uno::Sequence< ::rtl::OUString > lNames = m_aData.pCache->getAllTypeNames();
    sal_Int32                             nCount = lNames.getLength()               ;
    for( sal_Int32 nItem=0; nItem<nCount; ++nItem )
    {
        ::rtl::OUString sName = lNames[nItem]                   ;
        FileType        aItem = m_aData.pCache->getType( sName );

        // write entry in type list table
        sAllTypesHTML.appendAscii   ( "\t\t\t<tr>\n"                                                                                                            );  // open row
        sAllTypesHTML.appendAscii   ( "\t\t\t\t<td bgcolor=#f5f9d5 valign=\"top\" align=\"top\">"                                                               );  // open column "nr"
        sAllTypesHTML.append        ( OUString::valueOf( nItem )                                                                                                );  // write nr
        sAllTypesHTML.appendAscii   ( "</td>\n"                                                                                                                 );  // close column "nr"
        sAllTypesHTML.appendAscii   ( "\t\t\t\t<td>"                                                                                                            );  // open column "name"
        sAllTypesHTML.appendAscii   ( "<a href=\""                                                                                                              );  // open href="typeproperties.html#<typename>"
        sAllTypesHTML.appendAscii   ( TYPEPROPERTIES_HTML                                                                                                       );
        sAllTypesHTML.appendAscii   ( "#"                                                                                                                       );
        sAllTypesHTML.append        ( aItem.sName                                                                                                               );
        sAllTypesHTML.appendAscii   ( "\" target=\""                                                                                                            );
        sAllTypesHTML.appendAscii   ( TARGET_PROPERTIES                                                                                                         );
        sAllTypesHTML.appendAscii   ( "\">"                                                                                                                     );
        sAllTypesHTML.append        ( aItem.sName                                                                                                               );  // write name
        sAllTypesHTML.appendAscii   ( "</a>"                                                                                                                    );  // close href
        sAllTypesHTML.appendAscii   ( "</td>\n"                                                                                                                 );  // close column "name"
        sAllTypesHTML.appendAscii   ( "\t\t\t</tr>\n"                                                                                                           );  // close row

        // write entry in type property table
        sTypePropHTML.appendAscii   ( "\t\t<a name=\""                                                                                                              );  // set target="#<typename>" to follow table
        sTypePropHTML.append        ( aItem.sName                                                                                                                   );
        sTypePropHTML.appendAscii   ( "\"></a>"                                                                                                                     );
        sTypePropHTML.appendAscii   ( "\t\t<table border=0>\n"                                                                                                      );  // open table
        sTypePropHTML.appendAscii   ( "\t\t\t<tr><td bgcolor=#f5f9d5 valign=\"top\" align=\"top\">Nr.</td><td bgcolor=#f5f9d5 valign=\"top\" align=\"top\">&nbsp;"  );    // generate row "Nr <value>"
        sTypePropHTML.append        ( OUString::valueOf( nItem )                                                                                                    );
        sTypePropHTML.appendAscii   ( "</td></tr>\n"                                                                                                                );
        sTypePropHTML.appendAscii   ( "\t\t\t<tr><td bgcolor=#f5f9d5 valign=\"top\" align=\"top\">Name</td><td valign=\"top\" align=\"top\">&nbsp;"                 );  // generate row "Name <value>"
        sTypePropHTML.append        ( aItem.sName                                                                                                                   );
        sTypePropHTML.appendAscii   ( "</td></tr>\n"                                                                                                                );
        sTypePropHTML.appendAscii   ( "\t\t\t<tr><td bgcolor=#f5f9d5 valign=\"top\" align=\"top\">UIName</td><td valign=\"top\" align=\"top\">"                     );  // generate row "UIName <value>"
        for(    ConstStringHashIterator pUIName=aItem.lUINames.begin()  ;
                pUIName!=aItem.lUINames.end()                           ;
                ++pUIName                                                       )
        {
            sTypePropHTML.appendAscii   ( "&nbsp;["       );
            sTypePropHTML.append        ( pUIName->first  );
            sTypePropHTML.appendAscii   ( "] \""          );
            sTypePropHTML.append        ( pUIName->second );
            sTypePropHTML.appendAscii   ( "\"<br>"        );
        }
        sTypePropHTML.appendAscii   ( "</td></tr>\n"                                                                                                            );
        sTypePropHTML.appendAscii   ( "\t\t\t<tr><td bgcolor=#f5f9d5 valign=\"top\" align=\"top\">MediaType</td><td valign=\"top\" align=\"top\">&nbsp;"        );  // generate row "MediaType <value>"
        sTypePropHTML.append        ( aItem.sMediaType                                                                                                          );
        sTypePropHTML.appendAscii   ( "</td></tr>\n"                                                                                                            );
        sTypePropHTML.appendAscii   ( "\t\t\t<tr><td bgcolor=#f5f9d5 valign=\"top\" align=\"top\">ClipboardFormat</td><td valign=\"top\" align=\"top\">&nbsp;"  );  // generate row "ClipboardFormat <value>"
        sTypePropHTML.append        ( aItem.sClipboardFormat                                                                                                    );
        sTypePropHTML.appendAscii   ( "</td></tr>\n"                                                                                                            );
        sTypePropHTML.appendAscii   ( "\t\t\t<tr><td bgcolor=#f5f9d5 valign=\"top\" align=\"top\">URLPattern</td><td valign=\"top\" align=\"top\">"             );  // generate row "URLPattern <value>"
        for(    ConstStringListIterator pPattern=aItem.lURLPattern.begin()  ;
                pPattern!=aItem.lURLPattern.end()                           ;
                ++pPattern                                                  )
        {
            sTypePropHTML.appendAscii   ( "&nbsp;\"");
            sTypePropHTML.append        ( *pPattern );
            sTypePropHTML.appendAscii   ( "\"<br>"  );
        }
        sTypePropHTML.appendAscii   ( "</td></tr>\n"                                                                                                            );
        sTypePropHTML.appendAscii   ( "\t\t\t<tr><td bgcolor=#f5f9d5 valign=\"top\" align=\"top\">Extensions</td><td valign=\"top\" align=\"top\">"             );  // generate row "Extensions <value>"
        for(    ConstStringListIterator pExtension=aItem.lExtensions.begin()    ;
                pExtension!=aItem.lExtensions.end()                             ;
                ++pExtension                                                    )
        {
            sTypePropHTML.appendAscii   ( "&nbsp;\""    );
            sTypePropHTML.append        ( *pExtension   );
            sTypePropHTML.appendAscii   ( "\"<br>"      );
        }
        sTypePropHTML.appendAscii   ( "</td></tr>\n"                                                                                                            );
        sTypePropHTML.appendAscii   ( "\t\t\t<tr><td bgcolor=#f5f9d5 valign=\"top\" align=\"top\">DocumentIconID</td><td valign=\"top\" align=\"top\">&nbsp;"   );  // generate row "DocumentIconID <value>"
        sTypePropHTML.append        ( OUString::valueOf( aItem.nDocumentIconID )                                                                                );
        sTypePropHTML.appendAscii   ( "</td></tr>\n"                                                                                                            );
        sTypePropHTML.appendAscii   ( "\t\t</table>\n"                                                                                                          );  // close table
        sTypePropHTML.appendAscii   ( "\t\t<p>\n"                                                                                                               );  // add space between this and following table
    }

    sAllTypesHTML.appendAscii( "</table>\n"         );  // close table
    sAllTypesHTML.appendAscii( "</body>\n</html>\n" );  // close html

    sTypePropHTML.appendAscii( "</body>\n</html>\n" );  // close html

    impl_writeFile( ALLTYPES_HTML        , U2B(sAllTypesHTML.makeStringAndClear()) );
    impl_writeFile( TYPEPROPERTIES_HTML  , U2B(sTypePropHTML.makeStringAndClear()) );
}

//*****************************************************************************************************************
void CFGView::impl_generateFilterListHTML()
{
    //-------------------------------------------------------------------------------------------------------------
    // generate frameset for types
    OUStringBuffer sFiltersFramesetHTML( 10000 );

    sFiltersFramesetHTML.appendAscii( "<html>\n\t<head>\n\t\t<title>\n\t\t\tFrameset: Filters\n\t\t</title>\n\t</head>\n"       );  // open html
    sFiltersFramesetHTML.appendAscii( "\t\t<frameset cols=\"40%,60%\">\n"                                                       );  // open frameset for cols
    sFiltersFramesetHTML.appendAscii( "\t\t\t<frame name=\""                                                                    );  // generate frame "list"
    sFiltersFramesetHTML.appendAscii( TARGET_LIST                                                                               );
    sFiltersFramesetHTML.appendAscii( "\" src=\""                                                                               );
    sFiltersFramesetHTML.appendAscii( ALLFILTERS_HTML                                                                           );
    sFiltersFramesetHTML.appendAscii( "\" title=\"List\">\n"                                                                    );
    sFiltersFramesetHTML.appendAscii( "\t\t\t<frame name=\""                                                                    );  // generate frame "properties"
    sFiltersFramesetHTML.appendAscii( TARGET_PROPERTIES                                                                         );
    sFiltersFramesetHTML.appendAscii( "\" src=\""                                                                               );
    sFiltersFramesetHTML.appendAscii( FILTERPROPERTIES_HTML                                                                     );
    sFiltersFramesetHTML.appendAscii( "\" title=\"Properties\">\n"                                                              );
    sFiltersFramesetHTML.appendAscii( "\t\t</frameset>\n"                                                                       );  // close frameset cols
    sFiltersFramesetHTML.appendAscii( "</html>\n"                                                                               );  // close html

    impl_writeFile( FRAMESET_FILTERS_HTML, U2B(sFiltersFramesetHTML.makeStringAndClear()) );

    //-------------------------------------------------------------------------------------------------------------
    // generate filter list (names and links only!)
    // use same loop to generate filter property list!
    OUStringBuffer sAllFiltersHTML( 10000 );
    OUStringBuffer sFilterPropHTML( 10000 );

    sAllFiltersHTML.appendAscii( "<html>\n\t<head>\n\t\t<title>\n\t\t\tAll Filters\n\t\t</title>\n\t</head>\n\t<body>\n"                                        );  // open html
    sAllFiltersHTML.appendAscii( "\t\t<table border=0><tr><td bgcolor=#ff8040><strong>Nr.</strong></td><td bgcolor=#ff8040><strong>Filter</strong></td></tr>\n" );  // open table

    sFilterPropHTML.appendAscii( "<html>\n\t<head>\n\t\t<title>\n\t\t\tFilterProperties\n\t\t</title>\n\t</head>\n\t<body>\n"                                   );  // open html
/*
    ::framework::StringList lFilterNames;
    for( ConstFilterIterator pFilter=m_pData->aCFGView.begin(); pFilter!=m_pData->aCFGView.end(); ++pFilter )
    {
        lFilterNames.push_back( pFilter->first );
    }
    ::std::stable_sort( lFilterNames.begin(), lFilterNames.end() );
    css::uno::Sequence< ::rtl::OUString > lNames;
    ::framework::DataContainer::convertStringVectorToSequence( lFilterNames, lNames );
*/
    css::uno::Sequence< ::rtl::OUString > lNames         = m_aData.pCache->getAllFilterNames()  ;
    sal_Int32                             nFilterCounter = 0                                    ;
    sal_Int32                             nCount         = lNames.getLength()                   ;
    Filter                                aFilter                                               ;
    for( nFilterCounter=0; nFilterCounter<nCount; ++nFilterCounter )
    {
        aFilter = m_aData.pCache->getFilter( lNames[nFilterCounter] );

        // write entry in filter list table
        sAllFiltersHTML.appendAscii ( "\t\t\t<tr>\n"                                                                                                            );  // open row
        sAllFiltersHTML.appendAscii ( "\t\t\t\t<td bgcolor=#f5f9d5 valign=\"top\" align=\"top\">"                                                               );  // open column "nr"
        sAllFiltersHTML.append      ( OUString::valueOf( nFilterCounter )                                                                                       );  // write nr
        sAllFiltersHTML.appendAscii ( "</td>\n"                                                                                                                 );  // close column "nr"
        sAllFiltersHTML.appendAscii ( "\t\t\t\t<td>"                                                                                                            );  // open column "name"
        sAllFiltersHTML.appendAscii ( "<a href=\""                                                                                                              );  // open href="filterproperties.html#<filtername>"
        sAllFiltersHTML.appendAscii ( FILTERPROPERTIES_HTML                                                                                                     );
        sAllFiltersHTML.appendAscii ( "#"                                                                                                                       );
        sAllFiltersHTML.append      ( aFilter.sName                                                                                                             );
        sAllFiltersHTML.appendAscii ( "\" target=\""                                                                                                            );
        sAllFiltersHTML.appendAscii ( TARGET_PROPERTIES                                                                                                         );
        sAllFiltersHTML.appendAscii ( "\">"                                                                                                                     );
        sAllFiltersHTML.append      ( aFilter.sName                                                                                                             );  // write name
        sAllFiltersHTML.appendAscii ( "</a>"                                                                                                                    );  // close href
        sAllFiltersHTML.appendAscii ( "</td>\n"                                                                                                                 );  // close column "name"
        sAllFiltersHTML.appendAscii ( "\t\t\t</tr>\n"                                                                                                           );  // close row

        // write entry in filter property table
        sFilterPropHTML.appendAscii ( "\t\t<a name=\""                                                                                                          );  // set target="#<typename>" to follow table
        sFilterPropHTML.append      ( aFilter.sName                                                                                                             );
        sFilterPropHTML.appendAscii ( "\"></a>"                                                                                                                 );
        sFilterPropHTML.appendAscii ( "\t\t<table border=0>\n"                                                                                                  );  // open table
        sFilterPropHTML.appendAscii ( "\t\t\t<tr><td bgcolor=#f5f9d5 valign=\"top\" align=\"top\">Nr.</td><td bgcolor=#f5f9d5 valign=\"top\" align=\"top\">&nbsp;");    // generate row "Nr <value>"
        sFilterPropHTML.append      ( OUString::valueOf( nFilterCounter )                                                                                       );
        sFilterPropHTML.appendAscii ( "</td></tr>\n"                                                                                                            );
        sFilterPropHTML.appendAscii ( "\t\t\t<tr><td bgcolor=#f5f9d5 valign=\"top\" align=\"top\">Name</td><td valign=\"top\" align=\"top\">&nbsp;"             );  // generate row "Name <value>"
        sFilterPropHTML.append      ( aFilter.sName                                                                                                             );
        sFilterPropHTML.appendAscii ( "</td></tr>\n"                                                                                                            );
        sFilterPropHTML.appendAscii ( "\t\t\t<tr><td bgcolor=#f5f9d5 valign=\"top\" align=\"top\">Order</td><td valign=\"top\" align=\"top\">&nbsp;\""          );  // generate row "Order <value>"
        sFilterPropHTML.append      ( aFilter.nOrder                                                                                                            );
        sFilterPropHTML.appendAscii ( "\"</td></tr>\n"                                                                                                          );
        sFilterPropHTML.appendAscii ( "\t\t\t<tr><td bgcolor=#f5f9d5 valign=\"top\" align=\"top\">Type</td><td valign=\"top\" align=\"top\">&nbsp;\""           );  // generate row "Type <value>"
        sFilterPropHTML.append      ( aFilter.sType                                                                                                             );
        sFilterPropHTML.appendAscii ( "\"</td></tr>\n"                                                                                                          );
        sFilterPropHTML.appendAscii ( "\t\t\t<tr><td bgcolor=#f5f9d5 valign=\"top\" align=\"top\">UIName</td><td valign=\"top\" align=\"top\">"                 );  // generate row "UIName <value>"
        for(    ConstStringHashIterator pUIName=aFilter.lUINames.begin()  ;
                pUIName!=aFilter.lUINames.end()                           ;
                ++pUIName                                                 )
        {
            sFilterPropHTML.appendAscii   ( "&nbsp;["       );
            sFilterPropHTML.append        ( pUIName->first  );
            sFilterPropHTML.appendAscii   ( "] \""          );
            sFilterPropHTML.append        ( pUIName->second );
            sFilterPropHTML.appendAscii   ( "\"<br>"        );
        }
        sFilterPropHTML.appendAscii ( "</td></tr>\n"                                                                                                          );
        sFilterPropHTML.appendAscii ( "\t\t\t<tr><td bgcolor=#f5f9d5 valign=\"top\" align=\"top\">DocumentService</td><td valign=\"top\" align=\"top\">&nbsp;"  );  // generate row "DocumentService <value>"
        sFilterPropHTML.append      ( aFilter.sDocumentService                                                                                          );
        sFilterPropHTML.appendAscii ( "</td></tr>\n"                                                                                                            );
        sFilterPropHTML.appendAscii ( "\t\t\t<tr><td bgcolor=#f5f9d5 valign=\"top\" align=\"top\">FilterService</td><td valign=\"top\" align=\"top\">&nbsp;"    );  // generate row "FilterService <value>"
        sFilterPropHTML.append      ( aFilter.sFilterService                                                                                            );
        sFilterPropHTML.appendAscii ( "</td></tr>\n"                                                                                                            );
        sFilterPropHTML.appendAscii ( "\t\t\t<tr><td bgcolor=#f5f9d5 valign=\"top\" align=\"top\">Flags</td><td valign=\"top\" align=\"top\">&nbsp;"            );  // generate row "Flags <value>"
        if( aFilter.nFlags & FILTERFLAG_IMPORT          ) { sFilterPropHTML.append( FILTERFLAGNAME_IMPORT          ); sFilterPropHTML.appendAscii( "<br>&nbsp;" ); };
        if( aFilter.nFlags & FILTERFLAG_EXPORT          ) { sFilterPropHTML.append( FILTERFLAGNAME_EXPORT          ); sFilterPropHTML.appendAscii( "<br>&nbsp;" ); };
        if( aFilter.nFlags & FILTERFLAG_TEMPLATE        ) { sFilterPropHTML.append( FILTERFLAGNAME_TEMPLATE        ); sFilterPropHTML.appendAscii( "<br>&nbsp;" ); };
        if( aFilter.nFlags & FILTERFLAG_INTERNAL        ) { sFilterPropHTML.append( FILTERFLAGNAME_INTERNAL        ); sFilterPropHTML.appendAscii( "<br>&nbsp;" ); };
        if( aFilter.nFlags & FILTERFLAG_TEMPLATEPATH    ) { sFilterPropHTML.append( FILTERFLAGNAME_TEMPLATEPATH    ); sFilterPropHTML.appendAscii( "<br>&nbsp;" ); };
        if( aFilter.nFlags & FILTERFLAG_OWN             ) { sFilterPropHTML.append( FILTERFLAGNAME_OWN             ); sFilterPropHTML.appendAscii( "<br>&nbsp;" ); };
        if( aFilter.nFlags & FILTERFLAG_ALIEN           ) { sFilterPropHTML.append( FILTERFLAGNAME_ALIEN           ); sFilterPropHTML.appendAscii( "<br>&nbsp;" ); };
        if( aFilter.nFlags & FILTERFLAG_USESOPTIONS     ) { sFilterPropHTML.append( FILTERFLAGNAME_USESOPTIONS     ); sFilterPropHTML.appendAscii( "<br>&nbsp;" ); };
        if( aFilter.nFlags & FILTERFLAG_DEFAULT         ) { sFilterPropHTML.append( FILTERFLAGNAME_DEFAULT         ); sFilterPropHTML.appendAscii( "<br>&nbsp;" ); };
        if( aFilter.nFlags & FILTERFLAG_NOTINFILEDIALOG ) { sFilterPropHTML.append( FILTERFLAGNAME_NOTINFILEDIALOG ); sFilterPropHTML.appendAscii( "<br>&nbsp;" ); };
        if( aFilter.nFlags & FILTERFLAG_NOTINCHOOSER    ) { sFilterPropHTML.append( FILTERFLAGNAME_NOTINCHOOSER    ); sFilterPropHTML.appendAscii( "<br>&nbsp;" ); };
        if( aFilter.nFlags & FILTERFLAG_ASYNCHRON       ) { sFilterPropHTML.append( FILTERFLAGNAME_ASYNCHRON       ); sFilterPropHTML.appendAscii( "<br>&nbsp;" ); };
        if( aFilter.nFlags & FILTERFLAG_READONLY        ) { sFilterPropHTML.append( FILTERFLAGNAME_READONLY        ); sFilterPropHTML.appendAscii( "<br>&nbsp;" ); };
        if( aFilter.nFlags & FILTERFLAG_NOTINSTALLED    ) { sFilterPropHTML.append( FILTERFLAGNAME_NOTINSTALLED    ); sFilterPropHTML.appendAscii( "<br>&nbsp;" ); };
        if( aFilter.nFlags & FILTERFLAG_CONSULTSERVICE  ) { sFilterPropHTML.append( FILTERFLAGNAME_CONSULTSERVICE  ); sFilterPropHTML.appendAscii( "<br>&nbsp;" ); };
        if( aFilter.nFlags & FILTERFLAG_3RDPARTYFILTER  ) { sFilterPropHTML.append( FILTERFLAGNAME_3RDPARTYFILTER  ); sFilterPropHTML.appendAscii( "<br>&nbsp;" ); };
        if( aFilter.nFlags & FILTERFLAG_PACKED          ) { sFilterPropHTML.append( FILTERFLAGNAME_PACKED          ); sFilterPropHTML.appendAscii( "<br>&nbsp;" ); };
        if( aFilter.nFlags & FILTERFLAG_SILENTEXPORT    ) { sFilterPropHTML.append( FILTERFLAGNAME_SILENTEXPORT    ); sFilterPropHTML.appendAscii( "<br>&nbsp;" ); };
        if( aFilter.nFlags & FILTERFLAG_BROWSERPREFERED ) { sFilterPropHTML.append( FILTERFLAGNAME_BROWSERPREFERED ); sFilterPropHTML.appendAscii( "<br>&nbsp;" ); };
        if( aFilter.nFlags & FILTERFLAG_PREFERED        ) { sFilterPropHTML.append( FILTERFLAGNAME_PREFERED        ); sFilterPropHTML.appendAscii( "<br>&nbsp;" ); };

        sFilterPropHTML.appendAscii ( "</td></tr>\n"                                                                                                            );
        sFilterPropHTML.appendAscii ( "\t\t\t<tr><td bgcolor=#f5f9d5 valign=\"top\" align=\"top\">UserData</td><td valign=\"top\" align=\"top\">"               );  // generate row "UserData <value>"
        for(    ConstStringListIterator pUserData=aFilter.lUserData.begin() ;
                pUserData!=aFilter.lUserData.end()                          ;
                ++pUserData                                                 )
        {
            sFilterPropHTML.appendAscii ( "&nbsp;\""    );
            sFilterPropHTML.append      ( *pUserData    );
            sFilterPropHTML.appendAscii ( "\"<br>"      );
        }
        sFilterPropHTML.appendAscii ( "</td></tr>\n"                                                                                                            );
        sFilterPropHTML.appendAscii ( "\t\t\t<tr><td bgcolor=#f5f9d5 valign=\"top\" align=\"top\">FileFormatVersion</td><td valign=\"top\" align=\"top\">&nbsp;");  // generate row "FileFormatVersion <value>"
        sFilterPropHTML.append      ( OUString::valueOf( aFilter.nFileFormatVersion )                                                                           );
        sFilterPropHTML.appendAscii ( "</td></tr>\n"                                                                                                            );
        sFilterPropHTML.appendAscii ( "\t\t\t<tr><td bgcolor=#f5f9d5 valign=\"top\" align=\"top\">TemplateName</td><td valign=\"top\" align=\"top\">&nbsp;"     );  // generate row "TemplateName <value>"
        sFilterPropHTML.append      ( aFilter.sTemplateName                                                                                                     );
        sFilterPropHTML.appendAscii ( "</td></tr>\n"                                                                                                            );
        sFilterPropHTML.appendAscii ( "\t\t</table>\n"                                                                                                          );  // close table
        sFilterPropHTML.appendAscii ( "\t\t<p>\n"                                                                                                               );  // add space between this and following table
    }

    sAllFiltersHTML.appendAscii( "</table>\n"           );  // close table
    sAllFiltersHTML.appendAscii( "</body>\n</html>\n"   );  // close html

    sFilterPropHTML.appendAscii( "</body>\n</html>\n"   );  // close html

    impl_writeFile( ALLFILTERS_HTML      , U2B(sAllFiltersHTML.makeStringAndClear()) );
    impl_writeFile( FILTERPROPERTIES_HTML, U2B(sFilterPropHTML.makeStringAndClear()) );
}

//*****************************************************************************************************************
void CFGView::impl_generateFilterModulListHTML()
{
    //-------------------------------------------------------------------------------------------------------------
    // generate frameset for filters sorted by modules
    OUStringBuffer sFiltersFramesetHTML( 10000 );

    sFiltersFramesetHTML.appendAscii( "<html>\n\t<head>\n\t\t<title>\n\t\t\tFrameset: Filters sorted by modules\n\t\t</title>\n\t</head>\n" );  // open html
    sFiltersFramesetHTML.appendAscii( "\t\t<frameset cols=\"40%,60%\">\n"                                                                   );  // open frameset for cols
    sFiltersFramesetHTML.appendAscii( "\t\t\t<frame name=\""                                                                                );  // generate frame "list"
    sFiltersFramesetHTML.appendAscii( TARGET_LIST                                                                                           );
    sFiltersFramesetHTML.appendAscii( "\" src=\""                                                                                           );
    sFiltersFramesetHTML.appendAscii( MODULFILTERS_HTML                                                                                     );
    sFiltersFramesetHTML.appendAscii( "\" title=\"List\">\n"                                                                                );
    sFiltersFramesetHTML.appendAscii( "\t\t\t<frame name=\""                                                                                );  // generate frame "properties"
    sFiltersFramesetHTML.appendAscii( TARGET_PROPERTIES                                                                                     );
    sFiltersFramesetHTML.appendAscii( "\" src=\""                                                                                           );
    sFiltersFramesetHTML.appendAscii( FILTERPROPERTIES_HTML                                                                                 );
    sFiltersFramesetHTML.appendAscii( "\" title=\"Properties\">\n"                                                                          );
    sFiltersFramesetHTML.appendAscii( "\t\t</frameset>\n"                                                                                   );  // close frameset cols
    sFiltersFramesetHTML.appendAscii( "</html>\n"                                                                                           );  // close html

    impl_writeFile( FRAMESET_FILTERS_HTML, U2B(sFiltersFramesetHTML.makeStringAndClear()) );

    //-------------------------------------------------------------------------------------------------------------
    // generate filter list (names and links only!)
    // use same loop to generate filter property list!
    OUStringBuffer sAllFiltersHTML( 10000 );
    OUStringBuffer sFilterPropHTML( 10000 );

    sAllFiltersHTML.appendAscii( "<html>\n\t<head>\n\t\t<title>\n\t\t\tAll Filters\n\t\t</title>\n\t</head>\n\t<body>\n"                                        );  // open html
    sAllFiltersHTML.appendAscii( "\t\t<table border=0><tr><td bgcolor=#ff8040><strong>Nr.</strong></td><td bgcolor=#ff8040><strong>Filter</strong></td></tr>\n" );  // open table

    sFilterPropHTML.appendAscii( "<html>\n\t<head>\n\t\t<title>\n\t\t\tFilterProperties\n\t\t</title>\n\t</head>\n\t<body>\n"                                   );  // open html

    css::uno::Sequence< ::rtl::OUString > lWriter ;
    css::uno::Sequence< ::rtl::OUString > lWeb    ;
    css::uno::Sequence< ::rtl::OUString > lGlobal ;
    css::uno::Sequence< ::rtl::OUString > lChart  ;
    css::uno::Sequence< ::rtl::OUString > lCalc   ;
    css::uno::Sequence< ::rtl::OUString > lImpress;
    css::uno::Sequence< ::rtl::OUString > lDraw   ;
    css::uno::Sequence< ::rtl::OUString > lMath   ;
    css::uno::Sequence< ::rtl::OUString > lGraphic;
    css::uno::Sequence< ::rtl::OUString > lDefault;
    css::uno::Sequence< ::rtl::OUString > lNames  ;

    m_aData.pCache->queryFilters( FILTERQUERY_TEXTDOCUMENT_WITHDEFAULT        ) >>= lWriter   ;
    m_aData.pCache->queryFilters( FILTERQUERY_WEBDOCUMENT_WITHDEFAULT         ) >>= lWeb      ;
    m_aData.pCache->queryFilters( FILTERQUERY_GLOBALDOCUMENT_WITHDEFAULT      ) >>= lGlobal   ;
    m_aData.pCache->queryFilters( FILTERQUERY_CHARTDOCUMENT_WITHDEFAULT       ) >>= lChart    ;
    m_aData.pCache->queryFilters( FILTERQUERY_SPREADSHEETDOCUMENT_WITHDEFAULT ) >>= lCalc     ;
    m_aData.pCache->queryFilters( FILTERQUERY_PRESENTATIONDOCUMENT_WITHDEFAULT) >>= lImpress  ;
    m_aData.pCache->queryFilters( FILTERQUERY_DRAWINGDOCUMENT_WITHDEFAULT     ) >>= lDraw     ;
    m_aData.pCache->queryFilters( FILTERQUERY_FORMULARPROPERTIES_WITHDEFAULT  ) >>= lMath     ;
    m_aData.pCache->queryFilters( FILTERQUERY_GRAPHICFILTERS                  ) >>= lGraphic  ;
    m_aData.pCache->queryFilters( FILTERQUERY_DEFAULTFILTERS                  ) >>= lDefault  ;

    sal_Int32       nModuls       = 0;
    sal_Int32       nFilters      = 0;
    sal_Int32       nModulCount   = 0;
    sal_Int32       nFilterCount  = 0;
    Filter          aFilter          ;
    ::rtl::OString  sModul           ;

    for( nModuls=0; nModuls<nModulCount; ++nModuls )
    {
        switch( nModuls )
        {
            case 0: {
                        lNames = lWriter;
                        sModul = "Writer";
                    }
                    break;
            case 1: {
                        lNames = lWeb    ;
                        sModul = "Web";
                    }
                    break;
            case 2: {
                        lNames = lGlobal ;
                        sModul = "GlobalDokument";
                    }
                    break;
            case 3: {
                        lNames = lChart  ;
                        sModul = "Chart";
                    }
                    break;
            case 4: {
                        lNames = lCalc   ;
                        sModul = "Calc";
                    }
                    break;
            case 5: {
                        lNames = lImpress;
                        sModul = "Impress";
                    }
                    break;
            case 6: {
                        lNames = lDraw   ;
                        sModul = "Draw";
                    }
                    break;
            case 7: {
                        lNames = lMath   ;
                        sModul = "Math";
                    }
                    break;
            case 8: {
                        lNames = lGraphic;
                        sModul = "Graphic";
                    }
                    break;
            case 9: {
                        lNames = lDefault;
                        sModul = "Default Filter!";
                    }
                    break;
        }

        sAllFiltersHTML.appendAscii ( "\t\t\t<tr>\n"                                                                        );
        sAllFiltersHTML.appendAscii ( "\t\t\t\t<td bgcolor=#000000 fgcolor=#ffffff valign=\"top\" align=\"top\">-</td>\n"   );
        sAllFiltersHTML.appendAscii ( "\t\t\t\t<td bgcolor=#000000 fgcolor=#ffffff valign=\"top\" align=\"top\">"           );
        sAllFiltersHTML.appendAscii ( sModul                                                                                );
        sAllFiltersHTML.appendAscii ( "</td>\n"                                                                             );
        sAllFiltersHTML.appendAscii ( "\t\t\t</tr>\n"                                                                       );

        nFilterCount = lNames.getLength();

        for( nFilters=0; nFilters<nFilterCount; ++nFilters )
        {
            aFilter = m_aData.pCache->getFilter( lNames[nFilters] );

            // write entry in filter list table
            sAllFiltersHTML.appendAscii ( "\t\t\t<tr>\n"                                                                                                            );  // open row
            sAllFiltersHTML.appendAscii ( "\t\t\t\t<td bgcolor=#f5f9d5 valign=\"top\" align=\"top\">"                                                               );  // open column "nr"
            sAllFiltersHTML.append      ( OUString::valueOf( nFilters )                                                                                             );  // write nr
            sAllFiltersHTML.appendAscii ( "</td>\n"                                                                                                                 );  // close column "nr"
            sAllFiltersHTML.appendAscii ( "\t\t\t\t<td>"                                                                                                            );  // open column "name"
            sAllFiltersHTML.appendAscii ( "<a href=\""                                                                                                              );  // open href="filterproperties.html#<filtername>"
            sAllFiltersHTML.appendAscii ( FILTERPROPERTIES_HTML                                                                                                     );
            sAllFiltersHTML.appendAscii ( "#"                                                                                                                       );
            sAllFiltersHTML.append      ( aFilter.sName                                                                                                             );
            sAllFiltersHTML.appendAscii ( "\" target=\""                                                                                                            );
            sAllFiltersHTML.appendAscii ( TARGET_PROPERTIES                                                                                                         );
            sAllFiltersHTML.appendAscii ( "\">"                                                                                                                     );
            sAllFiltersHTML.append      ( aFilter.sName                                                                                                             );  // write name
            sAllFiltersHTML.appendAscii ( "</a>"                                                                                                                    );  // close href
            sAllFiltersHTML.appendAscii ( "</td>\n"                                                                                                                 );  // close column "name"
            sAllFiltersHTML.appendAscii ( "\t\t\t</tr>\n"                                                                                                           );  // close row

            // write entry in filter property table
            sFilterPropHTML.appendAscii ( "\t\t<a name=\""                                                                                                          );  // set target="#<typename>" to follow table
            sFilterPropHTML.append      ( aFilter.sName                                                                                                             );
            sFilterPropHTML.appendAscii ( "\"></a>"                                                                                                                 );
            sFilterPropHTML.appendAscii ( "\t\t<table border=0>\n"                                                                                                  );  // open table
            sFilterPropHTML.appendAscii ( "\t\t\t<tr><td bgcolor=#f5f9d5 valign=\"top\" align=\"top\">Nr.</td><td bgcolor=#f5f9d5 valign=\"top\" align=\"top\">&nbsp;");    // generate row "Nr <value>"
            sFilterPropHTML.append      ( OUString::valueOf( nFilters )                                                                                             );
            sFilterPropHTML.appendAscii ( "</td></tr>\n"                                                                                                            );
            sFilterPropHTML.appendAscii ( "\t\t\t<tr><td bgcolor=#f5f9d5 valign=\"top\" align=\"top\">Name</td><td valign=\"top\" align=\"top\">&nbsp;"             );  // generate row "Name <value>"
            sFilterPropHTML.append      ( aFilter.sName                                                                                                             );
            sFilterPropHTML.appendAscii ( "</td></tr>\n"                                                                                                            );
            sFilterPropHTML.appendAscii ( "\t\t\t<tr><td bgcolor=#f5f9d5 valign=\"top\" align=\"top\">Order</td><td valign=\"top\" align=\"top\">&nbsp;\""          );  // generate row "Order <value>"
            sFilterPropHTML.append      ( aFilter.nOrder                                                                                                            );
            sFilterPropHTML.appendAscii ( "\"</td></tr>\n"                                                                                                          );
            sFilterPropHTML.appendAscii ( "\t\t\t<tr><td bgcolor=#f5f9d5 valign=\"top\" align=\"top\">Type</td><td valign=\"top\" align=\"top\">&nbsp;\""           );  // generate row "Type <value>"
            sFilterPropHTML.append      ( aFilter.sType                                                                                                             );
            sFilterPropHTML.appendAscii ( "\"</td></tr>\n"                                                                                                          );
            sFilterPropHTML.appendAscii ( "\t\t\t<tr><td bgcolor=#f5f9d5 valign=\"top\" align=\"top\">UIName</td><td valign=\"top\" align=\"top\">"                 );  // generate row "UIName <value>"
            for(    ConstStringHashIterator pUIName=aFilter.lUINames.begin()  ;
                    pUIName!=aFilter.lUINames.end()                           ;
                    ++pUIName                                                 )
            {
                sFilterPropHTML.appendAscii   ( "&nbsp;["       );
                sFilterPropHTML.append        ( pUIName->first  );
                sFilterPropHTML.appendAscii   ( "] \""          );
                sFilterPropHTML.append        ( pUIName->second );
                sFilterPropHTML.appendAscii   ( "\"<br>"        );
            }
            sFilterPropHTML.appendAscii ( "</td></tr>\n"                                                                                                          );
            sFilterPropHTML.appendAscii ( "\t\t\t<tr><td bgcolor=#f5f9d5 valign=\"top\" align=\"top\">DocumentService</td><td valign=\"top\" align=\"top\">&nbsp;"  );  // generate row "DocumentService <value>"
            sFilterPropHTML.append      ( aFilter.sDocumentService                                                                                          );
            sFilterPropHTML.appendAscii ( "</td></tr>\n"                                                                                                            );
            sFilterPropHTML.appendAscii ( "\t\t\t<tr><td bgcolor=#f5f9d5 valign=\"top\" align=\"top\">FilterService</td><td valign=\"top\" align=\"top\">&nbsp;"    );  // generate row "FilterService <value>"
            sFilterPropHTML.append      ( aFilter.sFilterService                                                                                            );
            sFilterPropHTML.appendAscii ( "</td></tr>\n"                                                                                                            );
            sFilterPropHTML.appendAscii ( "\t\t\t<tr><td bgcolor=#f5f9d5 valign=\"top\" align=\"top\">Flags</td><td valign=\"top\" align=\"top\">&nbsp;"            );  // generate row "Flags <value>"
            if( aFilter.nFlags & FILTERFLAG_IMPORT          ) { sFilterPropHTML.append( FILTERFLAGNAME_IMPORT          ); sFilterPropHTML.appendAscii( "<br>&nbsp;" ); };
            if( aFilter.nFlags & FILTERFLAG_EXPORT          ) { sFilterPropHTML.append( FILTERFLAGNAME_EXPORT          ); sFilterPropHTML.appendAscii( "<br>&nbsp;" ); };
            if( aFilter.nFlags & FILTERFLAG_TEMPLATE        ) { sFilterPropHTML.append( FILTERFLAGNAME_TEMPLATE        ); sFilterPropHTML.appendAscii( "<br>&nbsp;" ); };
            if( aFilter.nFlags & FILTERFLAG_INTERNAL        ) { sFilterPropHTML.append( FILTERFLAGNAME_INTERNAL        ); sFilterPropHTML.appendAscii( "<br>&nbsp;" ); };
            if( aFilter.nFlags & FILTERFLAG_TEMPLATEPATH    ) { sFilterPropHTML.append( FILTERFLAGNAME_TEMPLATEPATH    ); sFilterPropHTML.appendAscii( "<br>&nbsp;" ); };
            if( aFilter.nFlags & FILTERFLAG_OWN             ) { sFilterPropHTML.append( FILTERFLAGNAME_OWN             ); sFilterPropHTML.appendAscii( "<br>&nbsp;" ); };
            if( aFilter.nFlags & FILTERFLAG_ALIEN           ) { sFilterPropHTML.append( FILTERFLAGNAME_ALIEN           ); sFilterPropHTML.appendAscii( "<br>&nbsp;" ); };
            if( aFilter.nFlags & FILTERFLAG_USESOPTIONS     ) { sFilterPropHTML.append( FILTERFLAGNAME_USESOPTIONS     ); sFilterPropHTML.appendAscii( "<br>&nbsp;" ); };
            if( aFilter.nFlags & FILTERFLAG_DEFAULT         ) { sFilterPropHTML.append( FILTERFLAGNAME_DEFAULT         ); sFilterPropHTML.appendAscii( "<br>&nbsp;" ); };
            if( aFilter.nFlags & FILTERFLAG_NOTINFILEDIALOG ) { sFilterPropHTML.append( FILTERFLAGNAME_NOTINFILEDIALOG ); sFilterPropHTML.appendAscii( "<br>&nbsp;" ); };
            if( aFilter.nFlags & FILTERFLAG_NOTINCHOOSER    ) { sFilterPropHTML.append( FILTERFLAGNAME_NOTINCHOOSER    ); sFilterPropHTML.appendAscii( "<br>&nbsp;" ); };
            if( aFilter.nFlags & FILTERFLAG_ASYNCHRON       ) { sFilterPropHTML.append( FILTERFLAGNAME_ASYNCHRON       ); sFilterPropHTML.appendAscii( "<br>&nbsp;" ); };
            if( aFilter.nFlags & FILTERFLAG_READONLY        ) { sFilterPropHTML.append( FILTERFLAGNAME_READONLY        ); sFilterPropHTML.appendAscii( "<br>&nbsp;" ); };
            if( aFilter.nFlags & FILTERFLAG_NOTINSTALLED    ) { sFilterPropHTML.append( FILTERFLAGNAME_NOTINSTALLED    ); sFilterPropHTML.appendAscii( "<br>&nbsp;" ); };
            if( aFilter.nFlags & FILTERFLAG_CONSULTSERVICE  ) { sFilterPropHTML.append( FILTERFLAGNAME_CONSULTSERVICE  ); sFilterPropHTML.appendAscii( "<br>&nbsp;" ); };
            if( aFilter.nFlags & FILTERFLAG_3RDPARTYFILTER  ) { sFilterPropHTML.append( FILTERFLAGNAME_3RDPARTYFILTER  ); sFilterPropHTML.appendAscii( "<br>&nbsp;" ); };
            if( aFilter.nFlags & FILTERFLAG_PACKED          ) { sFilterPropHTML.append( FILTERFLAGNAME_PACKED          ); sFilterPropHTML.appendAscii( "<br>&nbsp;" ); };
            if( aFilter.nFlags & FILTERFLAG_SILENTEXPORT    ) { sFilterPropHTML.append( FILTERFLAGNAME_SILENTEXPORT    ); sFilterPropHTML.appendAscii( "<br>&nbsp;" ); };
            if( aFilter.nFlags & FILTERFLAG_BROWSERPREFERED ) { sFilterPropHTML.append( FILTERFLAGNAME_BROWSERPREFERED ); sFilterPropHTML.appendAscii( "<br>&nbsp;" ); };
            if( aFilter.nFlags & FILTERFLAG_PREFERED        ) { sFilterPropHTML.append( FILTERFLAGNAME_PREFERED        ); sFilterPropHTML.appendAscii( "<br>&nbsp;" ); };

            sFilterPropHTML.appendAscii ( "</td></tr>\n"                                                                                                            );
            sFilterPropHTML.appendAscii ( "\t\t\t<tr><td bgcolor=#f5f9d5 valign=\"top\" align=\"top\">UserData</td><td valign=\"top\" align=\"top\">"               );  // generate row "UserData <value>"
            for(    ConstStringListIterator pUserData=aFilter.lUserData.begin() ;
                    pUserData!=aFilter.lUserData.end()                          ;
                    ++pUserData                                                 )
            {
                sFilterPropHTML.appendAscii ( "&nbsp;\""    );
                sFilterPropHTML.append      ( *pUserData    );
                sFilterPropHTML.appendAscii ( "\"<br>"      );
            }
            sFilterPropHTML.appendAscii ( "</td></tr>\n"                                                                                                            );
            sFilterPropHTML.appendAscii ( "\t\t\t<tr><td bgcolor=#f5f9d5 valign=\"top\" align=\"top\">FileFormatVersion</td><td valign=\"top\" align=\"top\">&nbsp;");  // generate row "FileFormatVersion <value>"
            sFilterPropHTML.append      ( OUString::valueOf( aFilter.nFileFormatVersion )                                                                           );
            sFilterPropHTML.appendAscii ( "</td></tr>\n"                                                                                                            );
            sFilterPropHTML.appendAscii ( "\t\t\t<tr><td bgcolor=#f5f9d5 valign=\"top\" align=\"top\">TemplateName</td><td valign=\"top\" align=\"top\">&nbsp;"     );  // generate row "TemplateName <value>"
            sFilterPropHTML.append      ( aFilter.sTemplateName                                                                                                     );
            sFilterPropHTML.appendAscii ( "</td></tr>\n"                                                                                                            );
            sFilterPropHTML.appendAscii ( "\t\t</table>\n"                                                                                                          );  // close table
            sFilterPropHTML.appendAscii ( "\t\t<p>\n"                                                                                                               );  // add space between this and following table
        }
    }

    sAllFiltersHTML.appendAscii( "</table>\n"           );  // close table
    sAllFiltersHTML.appendAscii( "</body>\n</html>\n"   );  // close html

    sFilterPropHTML.appendAscii( "</body>\n</html>\n"   );  // close html

    impl_writeFile( ALLFILTERS_HTML      , U2B(sAllFiltersHTML.makeStringAndClear()) );
    impl_writeFile( FILTERPROPERTIES_HTML, U2B(sFilterPropHTML.makeStringAndClear()) );
}

//*****************************************************************************************************************
void CFGView::impl_generateDetectorListHTML()
{
    //-------------------------------------------------------------------------------------------------------------
    // generate frameset for detector services
    OUStringBuffer sDetectorsFramesetHTML( 10000 );

    sDetectorsFramesetHTML.appendAscii( "<html>\n\t<head>\n\t\t<title>\n\t\t\tFrameset: Detector Services\n\t\t</title>\n\t</head>\n"   );  // open html
    sDetectorsFramesetHTML.appendAscii( "\t\t<frameset cols=\"40%,60%\">\n"                                                             );  // open frameset for cols
    sDetectorsFramesetHTML.appendAscii( "\t\t\t<frame name=\""                                                                          );  // generate frame "list"
    sDetectorsFramesetHTML.appendAscii( TARGET_LIST                                                                                     );
    sDetectorsFramesetHTML.appendAscii( "\" src=\""                                                                                     );
    sDetectorsFramesetHTML.appendAscii( ALLDETECTORS_HTML                                                                               );
    sDetectorsFramesetHTML.appendAscii( "\" title=\"List\">\n"                                                                          );
    sDetectorsFramesetHTML.appendAscii( "\t\t\t<frame name=\""                                                                          );  // generate frame "properties"
    sDetectorsFramesetHTML.appendAscii( TARGET_PROPERTIES                                                                               );
    sDetectorsFramesetHTML.appendAscii( "\" src=\""                                                                                     );
    sDetectorsFramesetHTML.appendAscii( DETECTORPROPERTIES_HTML                                                                         );
    sDetectorsFramesetHTML.appendAscii( "\" title=\"Properties\">\n"                                                                    );
    sDetectorsFramesetHTML.appendAscii( "\t\t</frameset>\n"                                                                             );  // close frameset cols
    sDetectorsFramesetHTML.appendAscii( "</html>\n"                                                                                     );  // close html

    impl_writeFile( FRAMESET_DETECTORS_HTML, U2B(sDetectorsFramesetHTML.makeStringAndClear()) );

    //-------------------------------------------------------------------------------------------------------------
    // generate detector list (names and links only!)
    // use same loop to generate detector property list!
    OUStringBuffer sAllDetectorsHTML( 10000 );
    OUStringBuffer sDetectorPropHTML( 10000 );

    sAllDetectorsHTML.appendAscii( "<html>\n\t<head>\n\t\t<title>\n\t\t\tAll Detectors\n\t\t</title>\n\t</head>\n\t<body>\n"                                        );  // open html
    sAllDetectorsHTML.appendAscii( "\t\t<table border=0><tr><td bgcolor=#ff8040><strong>Nr.</strong></td><td bgcolor=#ff8040><strong>Detector</strong></td></tr>\n" );  // open table

    sDetectorPropHTML.appendAscii( "<html>\n\t<head>\n\t\t<title>\n\t\t\tDetectorProperties\n\t\t</title>\n\t</head>\n\t<body>\n"                                   );  // open html

    css::uno::Sequence< ::rtl::OUString > lNames = m_aData.pCache->getAllDetectorNames();
    sal_Int32                             nCount = lNames.getLength()                   ;
    for( sal_Int32 nItem=0; nItem<nCount; ++nItem )
    {
        Detector aDetector = m_aData.pCache->getDetector( lNames[nItem] );

        // write entry in detector list table
        sAllDetectorsHTML.appendAscii   ( "\t\t\t<tr>\n"                                                                                                            );  // open row
        sAllDetectorsHTML.appendAscii   ( "\t\t\t\t<td bgcolor=#f5f9d5 valign=\"top\" align=\"top\">"                                                               );  // open column "nr"
        sAllDetectorsHTML.append        ( OUString::valueOf( nItem )                                                                                                );  // write nr
        sAllDetectorsHTML.appendAscii   ( "</td>\n"                                                                                                                 );  // close column "nr"
        sAllDetectorsHTML.appendAscii   ( "\t\t\t\t<td>"                                                                                                            );  // open column "name"
        sAllDetectorsHTML.appendAscii   ( "<a href=\""                                                                                                              );  // open href="detectorproperties.html#<detectorname>"
        sAllDetectorsHTML.appendAscii   ( DETECTORPROPERTIES_HTML                                                                                                   );
        sAllDetectorsHTML.appendAscii   ( "#"                                                                                                                       );
        sAllDetectorsHTML.append        ( aDetector.sName                                                                                                           );
        sAllDetectorsHTML.appendAscii   ( "\" target=\""                                                                                                            );
        sAllDetectorsHTML.appendAscii   ( TARGET_PROPERTIES                                                                                                         );
        sAllDetectorsHTML.appendAscii   ( "\">"                                                                                                                     );
        sAllDetectorsHTML.append        ( aDetector.sName                                                                                                           );  // write name
        sAllDetectorsHTML.appendAscii   ( "</a>"                                                                                                                    );  // close href
        sAllDetectorsHTML.appendAscii   ( "</td>\n"                                                                                                                 );  // close column "name"
        sAllDetectorsHTML.appendAscii   ( "\t\t\t</tr>\n"                                                                                                           );  // close row

        // write entry in detector property table
        sDetectorPropHTML.appendAscii   ( "\t\t<a name=\""                                                                                                          );  // set target="#<detectorname>" to follow table
        sDetectorPropHTML.append        ( aDetector.sName                                                                                                           );
        sDetectorPropHTML.appendAscii   ( "\"></a>"                                                                                                                 );
        sDetectorPropHTML.appendAscii   ( "\t\t<table border=0>\n"                                                                                                  );  // open table
        sDetectorPropHTML.appendAscii   ( "\t\t\t<tr><td bgcolor=#f5f9d5 valign=\"top\" align=\"top\">Nr.</td><td valign=\"top\" align=\"top\">&nbsp;"              );  // generate row "Nr <value>"
        sDetectorPropHTML.append        ( OUString::valueOf( nItem )                                                                                                );
        sDetectorPropHTML.appendAscii   ( "</td></tr>\n"                                                                                                            );
        sDetectorPropHTML.appendAscii   ( "\t\t\t<tr><td bgcolor=#f5f9d5 valign=\"top\" align=\"top\">Name</td><td valign=\"top\" align=\"top\">&nbsp;"             );  // generate row "Name <value>"
        sDetectorPropHTML.append        ( aDetector.sName                                                                                                           );
        sDetectorPropHTML.appendAscii   ( "</td></tr>\n"                                                                                                            );
        sDetectorPropHTML.appendAscii   ( "\t\t\t<tr><td bgcolor=#f5f9d5 valign=\"top\" align=\"top\">Types</td><td valign=\"top\" align=\"top\">"                  );  // generate row "Types <value>"
        for(    ConstStringListIterator pType=aDetector.lTypes.begin()  ;
                pType!=aDetector.lTypes.end()                           ;
                ++pType                                                 )
        {
            sDetectorPropHTML.appendAscii   ( "&nbsp;\"");
            sDetectorPropHTML.append        ( *pType    );
            sDetectorPropHTML.appendAscii   ( "\"<br>"  );
        }

        sDetectorPropHTML.appendAscii   ( "\t\t</table>\n"                                                                                                          );  // close table
        sDetectorPropHTML.appendAscii   ( "\t\t<p>\n"                                                                                                               );  // add space between this and following table
    }

    sAllDetectorsHTML.appendAscii( "</table>\n"         );  // close table
    sAllDetectorsHTML.appendAscii( "</body>\n</html>\n" );  // close html

    sDetectorPropHTML.appendAscii( "</body>\n</html>\n" );  // close html

    impl_writeFile( ALLDETECTORS_HTML        , U2B(sAllDetectorsHTML.makeStringAndClear()) );
    impl_writeFile( DETECTORPROPERTIES_HTML  , U2B(sDetectorPropHTML.makeStringAndClear()) );
}

//*****************************************************************************************************************
void CFGView::impl_generateLoaderListHTML()
{
    //-------------------------------------------------------------------------------------------------------------
    // generate frameset for loader services
    OUStringBuffer sLoadersFramesetHTML( 10000 );

    sLoadersFramesetHTML.appendAscii( "<html>\n\t<head>\n\t\t<title>\n\t\t\tFrameset: Loader Services\n\t\t</title>\n\t</head>\n"   );  // open html
    sLoadersFramesetHTML.appendAscii( "\t\t<frameset cols=\"40%,60%\">\n"                                                           );  // open frameset for cols
    sLoadersFramesetHTML.appendAscii( "\t\t\t<frame name=\""                                                                        );  // generate frame "list"
    sLoadersFramesetHTML.appendAscii( TARGET_LIST                                                                                   );
    sLoadersFramesetHTML.appendAscii( "\" src=\""                                                                                   );
    sLoadersFramesetHTML.appendAscii( ALLLOADERS_HTML                                                                               );
    sLoadersFramesetHTML.appendAscii( "\" title=\"List\">\n"                                                                        );
    sLoadersFramesetHTML.appendAscii( "\t\t\t<frame name=\""                                                                        );  // generate frame "properties"
    sLoadersFramesetHTML.appendAscii( TARGET_PROPERTIES                                                                             );
    sLoadersFramesetHTML.appendAscii( "\" src=\""                                                                                   );
    sLoadersFramesetHTML.appendAscii( LOADERPROPERTIES_HTML                                                                         );
    sLoadersFramesetHTML.appendAscii( "\" title=\"Properties\">\n"                                                                  );
    sLoadersFramesetHTML.appendAscii( "\t\t</frameset>\n"                                                                           );  // close frameset cols
    sLoadersFramesetHTML.appendAscii( "</html>\n"                                                                                   );  // close html

    impl_writeFile( FRAMESET_LOADERS_HTML, U2B(sLoadersFramesetHTML.makeStringAndClear()) );

    //-------------------------------------------------------------------------------------------------------------
    // generate loader list (names and links only!)
    // use same loop to generate loader property list!
    OUStringBuffer sAllLoadersHTML( 10000 );
    OUStringBuffer sLoaderPropHTML( 10000 );

    sAllLoadersHTML.appendAscii( "<html>\n\t<head>\n\t\t<title>\n\t\t\tAll Loaders\n\t\t</title>\n\t</head>\n\t<body>\n"                                        );  // open html
    sAllLoadersHTML.appendAscii( "\t\t<table border=0><tr><td bgcolor=#ff8040><strong>Nr.</strong></td><td bgcolor=#ff8040><strong>Loader</strong></td></tr>\n" );  // open table

    sLoaderPropHTML.appendAscii( "<html>\n\t<head>\n\t\t<title>\n\t\t\tLoaderProperties\n\t\t</title>\n\t</head>\n\t<body>\n"                                   );  // open html

    css::uno::Sequence< ::rtl::OUString > lNames = m_aData.pCache->getAllLoaderNames();
    sal_Int32                             nCount = lNames.getLength()                 ;
    for( sal_Int32 nItem=0; nItem<nCount; ++nItem )
    {
        Loader aLoader = m_aData.pCache->getLoader( lNames[nItem] );

        // write entry in loader list table
        sAllLoadersHTML.appendAscii ( "\t\t\t<tr>\n"                                                                                                            );  // open row
        sAllLoadersHTML.appendAscii ( "\t\t\t\t<td bgcolor=#f5f9d5 valign=\"top\" align=\"top\">"                                                               );  // open column "nr"
        sAllLoadersHTML.append      ( OUString::valueOf( nItem )                                                                                                );  // write nr
        sAllLoadersHTML.appendAscii ( "</td>\n"                                                                                                                 );  // close column "nr"
        sAllLoadersHTML.appendAscii ( "\t\t\t\t<td>"                                                                                                            );  // open column "name"
        sAllLoadersHTML.appendAscii ( "<a href=\""                                                                                                              );  // open href="loaderproperties.html#<loadername>"
        sAllLoadersHTML.appendAscii ( LOADERPROPERTIES_HTML                                                                                                     );
        sAllLoadersHTML.appendAscii ( "#"                                                                                                                       );
        sAllLoadersHTML.append      ( aLoader.sName                                                                                                             );
        sAllLoadersHTML.appendAscii ( "\" target=\""                                                                                                            );
        sAllLoadersHTML.appendAscii ( TARGET_PROPERTIES                                                                                                         );
        sAllLoadersHTML.appendAscii ( "\">"                                                                                                                     );
        sAllLoadersHTML.append      ( aLoader.sName                                                                                                             );  // write name
        sAllLoadersHTML.appendAscii ( "</a>"                                                                                                                    );  // close href
        sAllLoadersHTML.appendAscii ( "</td>\n"                                                                                                                 );  // close column "name"
        sAllLoadersHTML.appendAscii ( "\t\t\t</tr>\n"                                                                                                           );  // close row

        // write entry in loader property table
        sLoaderPropHTML.appendAscii ( "\t\t<a name=\""                                                                                                          );  // set target="#<loadername>" to follow table
        sLoaderPropHTML.append      ( aLoader.sName                                                                                                             );
        sLoaderPropHTML.appendAscii ( "\"></a>"                                                                                                                 );
        sLoaderPropHTML.appendAscii ( "\t\t<table border=0>\n"                                                                                                  );  // open table
        sLoaderPropHTML.appendAscii ( "\t\t\t<tr><td bgcolor=#f5f9d5 valign=\"top\" align=\"top\">Nr.</td><td valign=\"top\" align=\"top\">&nbsp;"              );  // generate row "Nr <value>"
        sLoaderPropHTML.append      ( OUString::valueOf( nItem )                                                                                                );
        sLoaderPropHTML.appendAscii ( "</td></tr>\n"                                                                                                            );
        sLoaderPropHTML.appendAscii ( "\t\t\t<tr><td bgcolor=#f5f9d5 valign=\"top\" align=\"top\">Name</td><td valign=\"top\" align=\"top\">&nbsp;"             );  // generate row "Name <value>"
        sLoaderPropHTML.append      ( aLoader.sName                                                                                                             );
        sLoaderPropHTML.appendAscii ( "</td></tr>\n"                                                                                                            );
        sLoaderPropHTML.appendAscii ( "\t\t\t<tr><td bgcolor=#f5f9d5 valign=\"top\" align=\"top\">UIName</td><td valign=\"top\" align=\"top\">"                 );  // generate row "UIName <value>"
        for(    ConstStringHashIterator pUIName=aLoader.lUINames.begin()  ;
                pUIName!=aLoader.lUINames.end()                           ;
                ++pUIName                                                 )
        {
            sLoaderPropHTML.appendAscii   ( "&nbsp;["       );
            sLoaderPropHTML.append        ( pUIName->first  );
            sLoaderPropHTML.appendAscii   ( "] \""          );
            sLoaderPropHTML.append        ( pUIName->second );
            sLoaderPropHTML.appendAscii   ( "\"<br>"        );
        }
        sLoaderPropHTML.appendAscii ( "</td></tr>\n"                                                                                                            );
        sLoaderPropHTML.appendAscii ( "\t\t\t<tr><td bgcolor=#f5f9d5 valign=\"top\" align=\"top\">Types</td><td valign=\"top\" align=\"top\">"                  );  // generate row "Types <value>"
        for(    ConstStringListIterator pType=aLoader.lTypes.begin()    ;
                pType!=aLoader.lTypes.end()                             ;
                ++pType                                                 )
        {
            sLoaderPropHTML.appendAscii ( "&nbsp;\"");
            sLoaderPropHTML.append      ( *pType    );
            sLoaderPropHTML.appendAscii ( "\"<br>"  );
        }

        sLoaderPropHTML.appendAscii ( "\t\t</table>\n"                                                                                                          );  // close table
        sLoaderPropHTML.appendAscii ( "\t\t<p>\n"                                                                                                               );  // add space between this and following table
    }

    sAllLoadersHTML.appendAscii( "</table>\n"           );  // close table
    sAllLoadersHTML.appendAscii( "</body>\n</html>\n"   );  // close html

    sLoaderPropHTML.appendAscii( "</body>\n</html>\n"   );  // close html

    impl_writeFile( ALLLOADERS_HTML      , U2B(sAllLoadersHTML.makeStringAndClear()) );
    impl_writeFile( LOADERPROPERTIES_HTML, U2B(sLoaderPropHTML.makeStringAndClear()) );
}

//*****************************************************************************************************************
void CFGView::impl_generateInvalidFiltersHTML()
{
    //-------------------------------------------------------------------------------------------------------------
    // generate frameset for invalid filters
    OUStringBuffer sInvalidFiltersFramesetHTML( 10000 );

    sInvalidFiltersFramesetHTML.appendAscii( "<html>\n\t<head>\n\t\t<title>\n\t\t\tFrameset: Invalid Filters\n\t\t</title>\n\t</head>\n"    );  // open html
    sInvalidFiltersFramesetHTML.appendAscii( "\t\t<frameset cols=\"40%,60%\">\n"                                                            );  // open frameset for cols
    sInvalidFiltersFramesetHTML.appendAscii( "\t\t\t<frame name=\""                                                                         );  // generate frame "list"
    sInvalidFiltersFramesetHTML.appendAscii( TARGET_LIST                                                                                    );
    sInvalidFiltersFramesetHTML.appendAscii( "\" src=\""                                                                                    );
    sInvalidFiltersFramesetHTML.appendAscii( INVALIDFILTERS_HTML                                                                            );
    sInvalidFiltersFramesetHTML.appendAscii( "\" title=\"List\">\n"                                                                         );
    sInvalidFiltersFramesetHTML.appendAscii( "\t\t\t<frame name=\""                                                                         );  // generate frame "properties"
    sInvalidFiltersFramesetHTML.appendAscii( TARGET_PROPERTIES                                                                              );
    sInvalidFiltersFramesetHTML.appendAscii( "\" src=\""                                                                                    );
    sInvalidFiltersFramesetHTML.appendAscii( FILTERPROPERTIES_HTML                                                                          );
    sInvalidFiltersFramesetHTML.appendAscii( "\" title=\"Properties\">\n"                                                                   );
    sInvalidFiltersFramesetHTML.appendAscii( "\t\t</frameset>\n"                                                                            );  // close frameset cols
    sInvalidFiltersFramesetHTML.appendAscii( "</html>\n"                                                                                    );  // close html

    impl_writeFile( FRAMESET_INVALIDFILTERS_HTML, U2B(sInvalidFiltersFramesetHTML.makeStringAndClear()) );

    //-------------------------------------------------------------------------------------------------------------
    // Search invalid registered filters!
    OUStringBuffer sInvalidFilterHTML( 10000 );

    sInvalidFilterHTML.appendAscii( "<html>\n\t<head>\n\t\t<title>\n\t\t\tInvalid Filter\n\t\t</title>\n\t</head>\n\t<body>\n"                                                                          );  // open html
    sInvalidFilterHTML.appendAscii( "\t\tPlease check follow filter entries in configuration. They are registered for invalid types!<p>\n"                                                              );  // write "Note"
    sInvalidFilterHTML.appendAscii( "\t\t<table border=0>\n"                                                                                                                                            );  // open table
    sInvalidFilterHTML.appendAscii( "\t<tr><td bgcolor=#ff8040><strong>Nr.</strong></td><td bgcolor=#ff8040><strong>Filter</strong></td><td bgcolor=#ff8040><strong>Wrong Type</strong></td></tr>\n"    );  // generate table header

    css::uno::Sequence< ::rtl::OUString > lFilters     = m_aData.pCache->getAllFilterNames();
    sal_Int32                             nFilterCount = lFilters.getLength()               ;
    css::uno::Sequence< ::rtl::OUString > lTypes       = m_aData.pCache->getAllTypeNames()  ;
    sal_Int32                             nTypeCount   = lTypes.getLength()                 ;
    FileType                              aType  ;
    Filter                                aFilter;
    for( sal_Int32 nFilter=0; nFilter<nFilterCount; ++nFilter )
    {
        aFilter = m_aData.pCache->getFilter( lFilters[nFilter] );
        sal_Bool bFound = sal_False;
        for( sal_Int32 nType=0; nType<nTypeCount; ++nType )
        {
            aType = m_aData.pCache->getType( lTypes[nType] );
            if( aFilter.sType == aType.sName )
            {
                bFound = sal_True;
                break;
            }
        }
        if( bFound == sal_False )
        {
            sInvalidFilterHTML.appendAscii  ( "\t<tr><td bgcolor=#ff0000 color=#00ffff>"        );  // generate row for invalid filter entry
            sInvalidFilterHTML.append       ( OUString::valueOf( nFilter )                      );
            sInvalidFilterHTML.appendAscii  ( "</td><td><a href=\""                             );
            sInvalidFilterHTML.appendAscii  ( FILTERPROPERTIES_HTML                             );
            sInvalidFilterHTML.appendAscii  ( "#"                                               );
            sInvalidFilterHTML.append       ( aFilter.sName                                     );
            sInvalidFilterHTML.appendAscii  ( "\" target=\""                                    );
            sInvalidFilterHTML.appendAscii  ( TARGET_PROPERTIES                                 );
            sInvalidFilterHTML.appendAscii  ( "\">"                                             );
            sInvalidFilterHTML.append       ( aFilter.sName                                     );
            sInvalidFilterHTML.appendAscii  ( "\"</a></td><td>\""                               );
            sInvalidFilterHTML.append       ( aFilter.sType                                     );
            sInvalidFilterHTML.appendAscii  ( "\"</td></tr>\n"                                  );
        }
    }
    sInvalidFilterHTML.appendAscii( "</table>\n"            );  // close table
    sInvalidFilterHTML.appendAscii( "</body>\n</html>\n"    );  // close html
    impl_writeFile( INVALIDFILTERS_HTML, U2B(sInvalidFilterHTML.makeStringAndClear()) );
}

//*****************************************************************************************************************
void CFGView::impl_generateInvalidDetectorsHTML()
{
    //-------------------------------------------------------------------------------------------------------------
    // generate frameset for invalid detectors
    OUStringBuffer sInvalidDetectorsFramesetHTML( 10000 );

    sInvalidDetectorsFramesetHTML.appendAscii( "<html>\n\t<head>\n\t\t<title>\n\t\t\tFrameset: Invalid Detectors\n\t\t</title>\n\t</head>\n"    );  // open html
    sInvalidDetectorsFramesetHTML.appendAscii( "\t\t<frameset cols=\"40%,60%\">\n"                                                              );  // open frameset for cols
    sInvalidDetectorsFramesetHTML.appendAscii( "\t\t\t<frame name=\""                                                                           );  // generate frame "list"
    sInvalidDetectorsFramesetHTML.appendAscii( TARGET_LIST                                                                                      );
    sInvalidDetectorsFramesetHTML.appendAscii( "\" src=\""                                                                                      );
    sInvalidDetectorsFramesetHTML.appendAscii( INVALIDDETECTORS_HTML                                                                            );
    sInvalidDetectorsFramesetHTML.appendAscii( "\" title=\"List\">\n"                                                                           );
    sInvalidDetectorsFramesetHTML.appendAscii( "\t\t\t<frame name=\""                                                                           );  // generate frame "properties"
    sInvalidDetectorsFramesetHTML.appendAscii( TARGET_PROPERTIES                                                                                );
    sInvalidDetectorsFramesetHTML.appendAscii( "\" src=\""                                                                                      );
    sInvalidDetectorsFramesetHTML.appendAscii( DETECTORPROPERTIES_HTML                                                                          );
    sInvalidDetectorsFramesetHTML.appendAscii( "\" title=\"Properties\">\n"                                                                     );
    sInvalidDetectorsFramesetHTML.appendAscii( "\t\t</frameset>\n"                                                                              );  // close frameset cols
    sInvalidDetectorsFramesetHTML.appendAscii( "</html>\n"                                                                                      );  // close html

    impl_writeFile( FRAMESET_INVALIDDETECTORS_HTML, U2B(sInvalidDetectorsFramesetHTML.makeStringAndClear()) );

    //-------------------------------------------------------------------------------------------------------------
    // Search invalid registered detect services!
    OUStringBuffer sInvalidDetectorsHTML( 10000 );

    sInvalidDetectorsHTML.appendAscii( "<html>\n\t<head>\n\t\t<title>\n\t\t\tInvalid Detector Services\n\t\t</title>\n\t</head>\n\t<body>\n"            );  // open html
    sInvalidDetectorsHTML.appendAscii( "\t\tPlease check follow detect service entries in configuration. They are registered for invalid types!<p>\n"   );  // write "Note"
    sInvalidDetectorsHTML.appendAscii( "\t\t<table border=0>\n"                                                                                         );  // open table
    sInvalidDetectorsHTML.appendAscii( "\t<tr><td bgcolor=#ff8040><strong>Nr.</strong></td><td bgcolor=#ff8040><strong>Detector</strong></td></tr>\n"   );  // generate table header

    css::uno::Sequence< ::rtl::OUString > lDetectors     = m_aData.pCache->getAllDetectorNames();
    sal_Int32                             nDetectorCount = lDetectors.getLength()               ;
    css::uno::Sequence< ::rtl::OUString > lTypes       = m_aData.pCache->getAllTypeNames()  ;
    sal_Int32                             nTypeCount   = lTypes.getLength()                 ;
    FileType                              aType    ;
    Detector                              aDetector;
    for( sal_Int32 nDetector=0; nDetector<nDetectorCount; ++nDetector )
    {
        aDetector = m_aData.pCache->getDetector( lDetectors[nDetector] );
        sal_Bool bFound = sal_False;
        for( sal_Int32 nType=0; nType<nTypeCount; ++nType )
        {
            aType = m_aData.pCache->getType( lTypes[nType] );
            for( ConstStringListIterator pTypeListEntry=aDetector.lTypes.begin(); pTypeListEntry!=aDetector.lTypes.end(); ++pTypeListEntry )
            {
                if( *pTypeListEntry == aType.sName )
                {
                    bFound = sal_True;
                    break;
                }
            }
        }
        if( bFound == sal_False )
        {
            sInvalidDetectorsHTML.appendAscii   ( "\t<tr><td bgcolor=#ff0000 color=#00ffff>"        );  // generate row for invalid detector entry
            sInvalidDetectorsHTML.append        ( OUString::valueOf( nDetector )                    );
            sInvalidDetectorsHTML.appendAscii   ( "</td><td><a href=\""                             );
            sInvalidDetectorsHTML.appendAscii   ( DETECTORPROPERTIES_HTML                           );
            sInvalidDetectorsHTML.appendAscii   ( "#"                                               );
            sInvalidDetectorsHTML.append        ( aDetector.sName                                   );
            sInvalidDetectorsHTML.appendAscii   ( "\" target=\""                                    );
            sInvalidDetectorsHTML.appendAscii   ( TARGET_PROPERTIES                                 );
            sInvalidDetectorsHTML.appendAscii   ( "\">"                                             );
            sInvalidDetectorsHTML.append        ( aDetector.sName                                   );
            sInvalidDetectorsHTML.appendAscii   ( "\"</a></td></tr>\n"                              );
        }
    }
    sInvalidDetectorsHTML.appendAscii( "</table>\n"         );  // close table
    sInvalidDetectorsHTML.appendAscii( "</body>\n</html>\n" );  // close html
    impl_writeFile( INVALIDDETECTORS_HTML, U2B(sInvalidDetectorsHTML.makeStringAndClear()) );
}

//*****************************************************************************************************************
void CFGView::impl_generateInvalidLoadersHTML()
{
    //-------------------------------------------------------------------------------------------------------------
    // generate frameset for invalid loaders
    OUStringBuffer sInvalidLoadersFramesetHTML( 10000 );

    sInvalidLoadersFramesetHTML.appendAscii( "<html>\n\t<head>\n\t\t<title>\n\t\t\tFrameset: Invalid Loaders\n\t\t</title>\n\t</head>\n"    );  // open html
    sInvalidLoadersFramesetHTML.appendAscii( "\t\t<frameset cols=\"40%,60%\">\n"                                                            );  // open frameset for cols
    sInvalidLoadersFramesetHTML.appendAscii( "\t\t\t<frame name=\""                                                                         );  // generate frame "list"
    sInvalidLoadersFramesetHTML.appendAscii( TARGET_LIST                                                                                    );
    sInvalidLoadersFramesetHTML.appendAscii( "\" src=\""                                                                                    );
    sInvalidLoadersFramesetHTML.appendAscii( INVALIDLOADERS_HTML                                                                            );
    sInvalidLoadersFramesetHTML.appendAscii( "\" title=\"List\">\n"                                                                         );
    sInvalidLoadersFramesetHTML.appendAscii( "\t\t\t<frame name=\""                                                                         );  // generate frame "properties"
    sInvalidLoadersFramesetHTML.appendAscii( TARGET_PROPERTIES                                                                              );
    sInvalidLoadersFramesetHTML.appendAscii( "\" src=\""                                                                                    );
    sInvalidLoadersFramesetHTML.appendAscii( LOADERPROPERTIES_HTML                                                                          );
    sInvalidLoadersFramesetHTML.appendAscii( "\" title=\"Properties\">\n"                                                                   );
    sInvalidLoadersFramesetHTML.appendAscii( "\t\t</frameset>\n"                                                                            );  // close frameset cols
    sInvalidLoadersFramesetHTML.appendAscii( "</html>\n"                                                                                    );  // close html

    impl_writeFile( FRAMESET_INVALIDLOADERS_HTML, U2B(sInvalidLoadersFramesetHTML.makeStringAndClear()) );

    //-------------------------------------------------------------------------------------------------------------
    // Search invalid registered detect services!
    OUStringBuffer sInvalidLoadersHTML( 10000 );

    sInvalidLoadersHTML.appendAscii( "<html>\n\t<head>\n\t\t<title>\n\t\t\tInvalid Loader Services\n\t\t</title>\n\t</head>\n\t<body>\n"            );  // open html
    sInvalidLoadersHTML.appendAscii( "\t\tPlease check follow loader service entries in configuration. They are registered for invalid types!<p>\n" );  // write "Note"
    sInvalidLoadersHTML.appendAscii( "\t\t<table border=0>\n"                                                                                       );  // open table
    sInvalidLoadersHTML.appendAscii( "\t<tr><td bgcolor=#ff8040><strong>Nr.</strong></td><td bgcolor=#ff8040><strong>Loader</strong></td></tr>\n"   );  // generate table header

    css::uno::Sequence< ::rtl::OUString > lLoaders     = m_aData.pCache->getAllLoaderNames();
    sal_Int32                             nLoaderCount = lLoaders.getLength()               ;
    css::uno::Sequence< ::rtl::OUString > lTypes       = m_aData.pCache->getAllTypeNames()  ;
    sal_Int32                             nTypeCount   = lTypes.getLength()                 ;
    FileType                              aType    ;
    Loader                                aLoader  ;
    for( sal_Int32 nLoader=0; nLoader<nLoaderCount; ++nLoader )
    {
        aLoader = m_aData.pCache->getLoader( lLoaders[nLoader] );
        sal_Bool bFound = sal_False;
        for( sal_Int32 nType=0; nType<nTypeCount; ++nType )
        {
            aType = m_aData.pCache->getType( lTypes[nType] );
            for( ConstStringListIterator pTypeListEntry=aLoader.lTypes.begin(); pTypeListEntry!=aLoader.lTypes.end(); ++pTypeListEntry )
            {
                if( *pTypeListEntry == aType.sName )
                {
                    bFound = sal_True;
                    break;
                }
            }
        }
        if( bFound == sal_False )
        {
            sInvalidLoadersHTML.appendAscii ( "\t<tr><td bgcolor=#ff0000 color=#00ffff>"        );  // generate row for invalid loader entry
            sInvalidLoadersHTML.append      ( OUString::valueOf( nLoader )                      );
            sInvalidLoadersHTML.appendAscii ( "</td><td><a href=\""                             );
            sInvalidLoadersHTML.appendAscii ( LOADERPROPERTIES_HTML                             );
            sInvalidLoadersHTML.appendAscii ( "#"                                               );
            sInvalidLoadersHTML.append      ( aLoader.sName                                     );
            sInvalidLoadersHTML.appendAscii ( "\" target=\""                                    );
            sInvalidLoadersHTML.appendAscii ( TARGET_PROPERTIES                                 );
            sInvalidLoadersHTML.appendAscii ( "\">"                                             );
            sInvalidLoadersHTML.append      ( aLoader.sName                                     );
            sInvalidLoadersHTML.appendAscii ( "\"</a></td></tr>\n"                              );
        }
    }
    sInvalidLoadersHTML.appendAscii( "</table>\n"           );  // close table
    sInvalidLoadersHTML.appendAscii( "</body>\n</html>\n"   );  // close html
    impl_writeFile( INVALIDLOADERS_HTML, U2B(sInvalidLoadersHTML.makeStringAndClear()) );
}

//*****************************************************************************************************************
void CFGView::impl_generateFilterFlagsHTML()
{
    //-------------------------------------------------------------------------------------------------------------
    // Create view of all filters and his flags.
    OUStringBuffer sFilterFlagsHTML( 10000 );

    sFilterFlagsHTML.appendAscii( "<html>\n\t<head>\n\t\t<title>\n\t\t\tFilter and Flags\n\t\t</title>\n\t</head>\n\t<body>\n"                  );  // open html
    sFilterFlagsHTML.appendAscii( "\t\tThis is a list of all filters and his supported flags!<p>\n"                                             );  // write "Note"
    sFilterFlagsHTML.appendAscii( "\t\t<table border=0>\n"                                                                                      );  // open table
    sFilterFlagsHTML.appendAscii( "\t<tr><td bgcolor=#ff8040><strong>Nr.</strong></td><td bgcolor=#ff8040><strong>Loader</strong></td></tr>\n"  );  // generate table header

    css::uno::Sequence< ::rtl::OUString > lNames = m_aData.pCache->getAllFilterNames();
    sal_Int32                             nCount = lNames.getLength();
    for( sal_Int32 nItem=0; nItem!=nCount; ++nItem )
    {
        Filter aFilter = m_aData.pCache->getFilter( lNames[nItem] );

        sFilterFlagsHTML.appendAscii( "\t<tr><td bgcolor=#ff0000 color=#00ffff>"        );
        sFilterFlagsHTML.append     ( OUString::valueOf( nItem )                        );
        sFilterFlagsHTML.appendAscii( "</td><td><a href=\""                             );
        sFilterFlagsHTML.appendAscii( LOADERPROPERTIES_HTML                             );
        sFilterFlagsHTML.appendAscii( "#"                                               );
        sFilterFlagsHTML.append     ( aFilter.sName                                     );
        sFilterFlagsHTML.appendAscii( "\" target=\""                                    );
        sFilterFlagsHTML.appendAscii( TARGET_PROPERTIES                                 );
        sFilterFlagsHTML.appendAscii( "\">"                                             );
        sFilterFlagsHTML.append     ( aFilter.sName                                     );
        sFilterFlagsHTML.appendAscii( "\"</a></td><td>"                                 );

        if( aFilter.nFlags & FILTERFLAG_IMPORT          ) { sFilterFlagsHTML.append( FILTERFLAGNAME_IMPORT          ); sFilterFlagsHTML.appendAscii( " | " ); }
        if( aFilter.nFlags & FILTERFLAG_EXPORT          ) { sFilterFlagsHTML.append( FILTERFLAGNAME_EXPORT          ); sFilterFlagsHTML.appendAscii( " | " ); }
        if( aFilter.nFlags & FILTERFLAG_TEMPLATE        ) { sFilterFlagsHTML.append( FILTERFLAGNAME_TEMPLATE        ); sFilterFlagsHTML.appendAscii( " | " ); }
        if( aFilter.nFlags & FILTERFLAG_INTERNAL        ) { sFilterFlagsHTML.append( FILTERFLAGNAME_INTERNAL        ); sFilterFlagsHTML.appendAscii( " | " ); }
        if( aFilter.nFlags & FILTERFLAG_TEMPLATEPATH    ) { sFilterFlagsHTML.append( FILTERFLAGNAME_TEMPLATEPATH    ); sFilterFlagsHTML.appendAscii( " | " ); }
        if( aFilter.nFlags & FILTERFLAG_OWN             ) { sFilterFlagsHTML.append( FILTERFLAGNAME_OWN             ); sFilterFlagsHTML.appendAscii( " | " ); }
        if( aFilter.nFlags & FILTERFLAG_ALIEN           ) { sFilterFlagsHTML.append( FILTERFLAGNAME_ALIEN           ); sFilterFlagsHTML.appendAscii( " | " ); }
        if( aFilter.nFlags & FILTERFLAG_USESOPTIONS     ) { sFilterFlagsHTML.append( FILTERFLAGNAME_USESOPTIONS     ); sFilterFlagsHTML.appendAscii( " | " ); }
        if( aFilter.nFlags & FILTERFLAG_DEFAULT         ) { sFilterFlagsHTML.append( FILTERFLAGNAME_DEFAULT         ); sFilterFlagsHTML.appendAscii( " | " ); }
        if( aFilter.nFlags & FILTERFLAG_NOTINFILEDIALOG ) { sFilterFlagsHTML.append( FILTERFLAGNAME_NOTINFILEDIALOG ); sFilterFlagsHTML.appendAscii( " | " ); }
        if( aFilter.nFlags & FILTERFLAG_NOTINCHOOSER    ) { sFilterFlagsHTML.append( FILTERFLAGNAME_NOTINCHOOSER    ); sFilterFlagsHTML.appendAscii( " | " ); }
        if( aFilter.nFlags & FILTERFLAG_ASYNCHRON       ) { sFilterFlagsHTML.append( FILTERFLAGNAME_ASYNCHRON       ); sFilterFlagsHTML.appendAscii( " | " ); }
        if( aFilter.nFlags & FILTERFLAG_READONLY        ) { sFilterFlagsHTML.append( FILTERFLAGNAME_READONLY        ); sFilterFlagsHTML.appendAscii( " | " ); }
        if( aFilter.nFlags & FILTERFLAG_NOTINSTALLED    ) { sFilterFlagsHTML.append( FILTERFLAGNAME_NOTINSTALLED    ); sFilterFlagsHTML.appendAscii( " | " ); }
        if( aFilter.nFlags & FILTERFLAG_CONSULTSERVICE  ) { sFilterFlagsHTML.append( FILTERFLAGNAME_CONSULTSERVICE  ); sFilterFlagsHTML.appendAscii( " | " ); }
        if( aFilter.nFlags & FILTERFLAG_3RDPARTYFILTER  ) { sFilterFlagsHTML.append( FILTERFLAGNAME_3RDPARTYFILTER  ); sFilterFlagsHTML.appendAscii( " | " ); }
        if( aFilter.nFlags & FILTERFLAG_PACKED          ) { sFilterFlagsHTML.append( FILTERFLAGNAME_PACKED          ); sFilterFlagsHTML.appendAscii( " | " ); }
        if( aFilter.nFlags & FILTERFLAG_SILENTEXPORT    ) { sFilterFlagsHTML.append( FILTERFLAGNAME_SILENTEXPORT    ); sFilterFlagsHTML.appendAscii( " | " ); }
        if( aFilter.nFlags & FILTERFLAG_BROWSERPREFERED ) { sFilterFlagsHTML.append( FILTERFLAGNAME_BROWSERPREFERED ); sFilterFlagsHTML.appendAscii( " | " ); }
        if( aFilter.nFlags & FILTERFLAG_PREFERED        ) { sFilterFlagsHTML.append( FILTERFLAGNAME_PREFERED        ); }

        sFilterFlagsHTML.appendAscii( "</td></tr>\n"    );
    }
    sFilterFlagsHTML.appendAscii( "</table>\n"          );  // close table
    sFilterFlagsHTML.appendAscii( "</body>\n</html>\n"  );  // close html
    impl_writeFile( FILTERFLAGS_HTML, U2B(sFilterFlagsHTML.makeStringAndClear()) );
}

//*****************************************************************************************************************
void CFGView::impl_generateDefaultFiltersHTML()
{
}

//*****************************************************************************************************************
void CFGView::impl_generateDoubleFilterUINamesHTML()
{
    //-------------------------------------------------------------------------------------------------------------
    // generate frameset for double UINames
    OUStringBuffer sFrameSet( 10000 );

    sFrameSet.appendAscii( "<html>\n\t<head>\n\t\t<title>\n\t\t\tFrameset: Search doubl UINames\n\t\t</title>\n\t</head>\n" );  // open html
    sFrameSet.appendAscii( "\t\t<frameset cols=\"40%,60%\">\n"                                                              );  // open frameset for cols
    sFrameSet.appendAscii( "\t\t\t<frame name=\""                                                                           );  // generate frame "list"
    sFrameSet.appendAscii( TARGET_LIST                                                                                      );
    sFrameSet.appendAscii( "\" src=\""                                                                                      );
    sFrameSet.appendAscii( DOUBLEFILTERUINAMES_HTML                                                                         );
    sFrameSet.appendAscii( "\" title=\"Double UINames\">\n"                                                                 );
    sFrameSet.appendAscii( "\t\t\t<frame name=\""                                                                           );  // generate frame "properties"
    sFrameSet.appendAscii( TARGET_PROPERTIES                                                                                );
    sFrameSet.appendAscii( "\" src=\""                                                                                      );
    sFrameSet.appendAscii( FILTERPROPERTIES_HTML                                                                            );
    sFrameSet.appendAscii( "\" title=\"Properties\">\n"                                                                     );
    sFrameSet.appendAscii( "\t\t</frameset>\n"                                                                              );  // close frameset cols
    sFrameSet.appendAscii( "</html>\n"                                                                                      );  // close html

    impl_writeFile( FRAMESET_DOUBLEFILTERUINAMES_HTML, U2B(sFrameSet.makeStringAndClear()) );

    //-------------------------------------------------------------------------------------------------------------
    // Search invalid registered detect services!
    OUStringBuffer sHTML( 10000 );

    sHTML.appendAscii( "<html>\n\t<head>\n\t\t<title>\n\t\t\tDouble Filter UINames\n\t\t</title>\n\t</head>\n\t<body>\n"    );  // open html
    sHTML.appendAscii( "\t\tPlease check follow filter entries in configuration. Her UINames are registered twice!<p>\n"    );  // write "Note"
    sHTML.appendAscii( "\t\t<table border=0>\n"                                                                             );  // open table
    sHTML.appendAscii( "\t<tr><td bgcolor=#ff8040><strong>Nr.</strong></td>\n"                                              );  // generate table header
    sHTML.appendAscii( "\t\t<td bgcolor=#ff8040><strong>UIName</strong></td>\n"                                             );
    sHTML.appendAscii( "\t\t<td bgcolor=#ff8040><strong>Filters</strong></td>\n"                                            );
    sHTML.appendAscii( "\t</tr>\n"                                                                                          );

    StringHash                            lUINames                                          ;
    css::uno::Sequence< ::rtl::OUString > lFilters     = m_aData.pCache->getAllFilterNames();
    sal_Int32                             nFilterCount = lFilters.getLength()               ;
    Filter                                aFilter                                           ;
    ::rtl::OUStringBuffer                 sBuffer                                           ;
    ::rtl::OUString                       sUIName                                           ;

    for( sal_Int32 nFilter=0; nFilter<nFilterCount; ++nFilter )
    {
        aFilter = m_aData.pCache->getFilter( lFilters[nFilter] );
        for( ConstStringHashIterator pUIName=aFilter.lUINames.begin(); pUIName!= aFilter.lUINames.end(); ++pUIName )
        {
            // Build key value by using localized UIName to register filter name
            sBuffer.appendAscii( "["             );
            sBuffer.append     ( pUIName->first  );
            sBuffer.appendAscii( "] \""          );
            sBuffer.append     ( pUIName->second );
            sBuffer.appendAscii( "\""            );
            sUIName = sBuffer.makeStringAndClear();

            // insert filter into hash table
            sBuffer.append     ( lUINames[ sUIName ]     );
            sBuffer.appendAscii ( "<a href=\""           );
            sBuffer.appendAscii ( FILTERPROPERTIES_HTML  );
            sBuffer.appendAscii ( "#"                    );
            sBuffer.append      ( aFilter.sName          );
            sBuffer.appendAscii ( "\" target=\""         );
            sBuffer.appendAscii ( TARGET_PROPERTIES      );
            sBuffer.appendAscii ( "\">"                  );
            sBuffer.append      ( aFilter.sName          );
            sBuffer.appendAscii ( "\"</a><br>\n"         );
            lUINames[ sUIName ] = sBuffer.makeStringAndClear();
        }
    }

    nFilter = 1;
    for( ConstStringHashIterator pIterator=lUINames.begin(); pIterator!=lUINames.end(); ++pIterator )
    {
        if( pIterator->second.indexOf( '\n' ) != pIterator->second.lastIndexOf( '\n' ) )
        {
            sHTML.appendAscii ( "\t<tr><td bgcolor=#ff0000 color=#00ffff valign=top>"   );  // generate row for uiname->filter entry
            sHTML.append      ( OUString::valueOf( nFilter )                            );
            sHTML.appendAscii ( "</td><td valign=top>"                                  );
            sHTML.append      ( pIterator->first                                        );
            sHTML.appendAscii ( "</td><td bgcolor=#f0f0f0 valign=top>"                  );
            sHTML.append      ( pIterator->second                                       );
            sHTML.appendAscii ( "</td></tr>\n"                                          );

            ++nFilter;
        }
    }

    sHTML.appendAscii( "</table>\n"           );  // close table
    sHTML.appendAscii( "</body>\n</html>\n"   );  // close html
    impl_writeFile( DOUBLEFILTERUINAMES_HTML, U2B(sHTML.makeStringAndClear()) );
}

//*****************************************************************************************************************
void CFGView::impl_writeFile( const ::rtl::OString& sFile, const ::rtl::OString& sContent )
{
    ::rtl::OUStringBuffer sFullPath( 1000 );
    sFullPath.append        ( m_aData.sDirectory    );
    sFullPath.appendAscii   ( "\\"                  );
    sFullPath.appendAscii   ( sFile.getStr()        );

    ::rtl::OUString s = sFullPath.makeStringAndClear();

    WRITE_LOGFILE( U2B(s), sContent )
}
