/*************************************************************************
 *
 *  $RCSfile: climaker_app.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: vg $ $Date: 2003-10-06 13:05:25 $
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

#include <stdio.h>
#include <vector>
#include <memory>

#include "climaker_share.h"

#include "osl/process.h"
#include "osl/file.hxx"
#include "osl/thread.h"
#include "rtl/ustrbuf.hxx"
#include "cppuhelper/shlib.hxx"
#include "cppuhelper/bootstrap.hxx"
#include "com/sun/star/lang/XInitialization.hpp"
#include "com/sun/star/lang/XSingleComponentFactory.hpp"
#include "com/sun/star/lang/XComponent.hpp"
#include "com/sun/star/container/XSet.hpp"
#include "com/sun/star/reflection/XTypeDescriptionEnumerationAccess.hpp"


using namespace ::std;
using namespace ::System::Reflection;

using namespace ::rtl;
using namespace ::osl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace climaker
{

//------------------------------------------------------------------------------
static char const s_usingText [] =
"\n"
"using: climaker <switches> [registry-file-1 registry-file-2 ...]\n"
"\n"
"switches:\n"
" -O, --out <output-file>       output assembly file;\n"
"                               defaults to cli_unotypes.dll if more than one\n"
"                               registry-file is given, else <registry-file>.dll\n"
" -T, --types                   types to be generated (if none is given,\n"
"   <type1[;type2;...]>         then all types of given registries are emitted\n"
" -X, --extra <rdb-file>        additional rdb to saturate referenced types in\n"
"                               given registry file(s); these types will not be\n"
"                               emitted into the output assembly file\n"
" -r, --reference               reference metadata from assembly file\n"
"   <assembly-file>\n"
" --assembly-version <version>  sets assembly version\n"
" --assembly-description <text> sets assembly description text\n"
" --assembly-product <text>     sets assembly product name\n"
" --assembly-company <text>     sets assembly company\n"
" --assembly-copyright <text>   sets assembly copyright\n"
" --assembly-trademark <text>   sets assembly trademark\n"
" -v, --verbose                 verbose output to stdout\n"
" -h, --help                    this message\n"
"\n"
"example: climaker --out cli_mytypes.dll \\\n"
"                  --reference cli_types.dll \\\n"
"                  --extra types.rdb \\\n"
"                  mytypes.rdb\n"
"\n";

struct OptionInfo
{
    char const * m_name;
    sal_uInt32 m_name_length;
    sal_Unicode m_short_option;
    bool m_has_argument;
};

bool g_verbose = false;

//------------------------------------------------------------------------------
static const OptionInfo s_option_infos [] = {
    { RTL_CONSTASCII_STRINGPARAM("out"), 'O', true },
    { RTL_CONSTASCII_STRINGPARAM("types"), 'T', true },
    { RTL_CONSTASCII_STRINGPARAM("extra"), 'X', true },
    { RTL_CONSTASCII_STRINGPARAM("reference"), 'r', true },
    { RTL_CONSTASCII_STRINGPARAM("assembly-version"), '\0', true },
    { RTL_CONSTASCII_STRINGPARAM("assembly-description"), '\0', true },
    { RTL_CONSTASCII_STRINGPARAM("assembly-product"), '\0', true },
    { RTL_CONSTASCII_STRINGPARAM("assembly-company"), '\0', true },
    { RTL_CONSTASCII_STRINGPARAM("assembly-copyright"), '\0', true },
    { RTL_CONSTASCII_STRINGPARAM("assembly-trademark"), '\0', true },
    { RTL_CONSTASCII_STRINGPARAM("verbose"), 'v', false },
    { RTL_CONSTASCII_STRINGPARAM("help"), 'h', false }
};

//==============================================================================
static OptionInfo const * get_option_info(
    OUString const & opt, sal_Unicode copt = '\0' )
{
    for ( sal_Int32 pos = 0;
          pos < (sizeof (s_option_infos) / sizeof (OptionInfo));
          ++pos )
    {
        OptionInfo const & option_info = s_option_infos[ pos ];

        if (opt.getLength() > 0)
        {
            if (opt.equalsAsciiL(
                    option_info.m_name, option_info.m_name_length ) &&
                (copt == '\0' || copt == option_info.m_short_option))
            {
                return &option_info;
            }
        }
        else
        {
            OSL_ASSERT( copt != '\0' );
            if (copt == option_info.m_short_option)
            {
                return &option_info;
            }
        }
    }
    OSL_ENSURE(
        0, OUStringToOString( opt, osl_getThreadTextEncoding() ).getStr() );
    return 0;
}

//==============================================================================
static bool is_option(
    OptionInfo const * option_info, sal_uInt32 * pIndex )
{
    OSL_ASSERT( option_info != 0 );
    if (osl_getCommandArgCount() <= *pIndex)
        return false;

    OUString arg;
    osl_getCommandArg( *pIndex, &arg.pData );
    sal_Int32 len = arg.getLength();

    if (len < 2 || arg[ 0 ] != '-')
        return false;

    if (len == 2 && arg[ 1 ] == option_info->m_short_option)
    {
        ++(*pIndex);
#if OSL_DEBUG_LEVEL > 1
        OSL_TRACE(
            __FILE__": identified option \'%c\'", option_info->m_short_option );
#endif
        return true;
    }
    if (arg[ 1 ] == '-' && rtl_ustr_ascii_compare(
            arg.pData->buffer + 2, option_info->m_name ) == 0)
    {
        ++(*pIndex);
#if OSL_DEBUG_LEVEL > 1
        OSL_TRACE( __FILE__": identified option \'%s\'", option_info->m_name );
#endif
        return true;
    }
    return false;
}

//==============================================================================
static inline bool read_option(
    bool * flag, OptionInfo const * option_info, sal_uInt32 * pIndex )
{
    bool ret = is_option( option_info, pIndex );
    if (ret)
        *flag = true;
    return ret;
}

//==============================================================================
static bool read_argument(
    OUString * pValue, OptionInfo const * option_info, sal_uInt32 * pIndex )
{
    if (is_option( option_info, pIndex ))
    {
        if (*pIndex < osl_getCommandArgCount())
        {
            osl_getCommandArg( *pIndex, &pValue->pData );
            ++(*pIndex);
#if OSL_DEBUG_LEVEL > 1
            OString cstr_val(
                OUStringToOString( *pValue, osl_getThreadTextEncoding() ) );
            OSL_TRACE( __FILE__": argument value: %s\n", cstr_val.getStr() );
#endif
            return true;
        }
        --(*pIndex);
    }
    return false;
}

//==============================================================================
static OUString const & path_get_working_dir()
{
    static OUString s_workingDir;
    if (! s_workingDir.getLength())
        osl_getProcessWorkingDir( &s_workingDir.pData );
    return s_workingDir;
}

//==============================================================================
static OUString path_make_absolute_file_url( OUString const & path )
{
    OUString file_url;
    oslFileError rc = osl_getFileURLFromSystemPath(
        path.pData, &file_url.pData );
    if (osl_File_E_None == rc)
    {
        OUString abs;
        rc = osl_getAbsoluteFileURL(
            path_get_working_dir().pData, file_url.pData, &abs.pData );
        if (osl_File_E_None == rc)
        {
            return abs;
        }
        else
        {
            throw RuntimeException(
                OUSTR("cannot make absolute: ") + file_url,
                Reference< XInterface >() );
        }
    }
    else
    {
        throw RuntimeException(
            OUSTR("cannot get file url from system path: ") + path,
            Reference< XInterface >() );
    }
}

//==============================================================================
Reference< registry::XSimpleRegistry > open_registries(
    vector< OUString > const & registries,
    Reference< XComponentContext > xContext )
{
    if (registries.empty())
    {
        throw RuntimeException(
            OUSTR("no registries given!"),
            Reference< XInterface >() );
    }

    Reference< registry::XSimpleRegistry > xSimReg;
    for ( size_t nPos = registries.size(); nPos--; )
    {
        Reference< registry::XSimpleRegistry > xReg(
            xContext->getServiceManager()->createInstanceWithContext(
                OUSTR("com.sun.star.registry.SimpleRegistry"), xContext ),
            UNO_QUERY_THROW );
        xReg->open( registries[ nPos ], sal_True, sal_False );
        if (! xReg->isValid())
        {
            throw RuntimeException(
                OUSTR("invalid registry: ") + registries[ nPos ],
                Reference< XInterface >() );
        }

        if (xSimReg.is()) // nest?
        {
            Reference< registry::XSimpleRegistry > xNested(
                xContext->getServiceManager()->createInstanceWithContext(
                    OUSTR("com.sun.star.registry.NestedRegistry"), xContext ),
                UNO_QUERY_THROW );
            Reference< lang::XInitialization > xInit(
                xNested, UNO_QUERY_THROW );
            Sequence< Any > args( 2 );
            args[ 0 ] <<= xReg;
            args[ 1 ] <<= xSimReg;
            xInit->initialize( args );
            xSimReg = xNested;
        }
        else
        {
            xSimReg = xReg;
        }
    }

    return xSimReg;
}

}

using namespace ::climaker;

//##############################################################################
extern "C" int SAL_CALL main( int argc, char const * argv [] )
{
    sal_uInt32 nCount = osl_getCommandArgCount();
    if (0 == nCount)
    {
        printf( s_usingText );
        return 0;
    }

    int ret = 0;
    Reference< XComponentContext > xContext;

    try
    {
        OptionInfo const * info_help =
            get_option_info( OUSTR("help") );
        OptionInfo const * info_verbose =
            get_option_info( OUSTR("verbose") );
        OptionInfo const * info_out =
            get_option_info( OUSTR("out") );
        OptionInfo const * info_types =
            get_option_info( OUSTR("types") );
        OptionInfo const * info_reference =
            get_option_info( OUSTR("reference") );
        OptionInfo const * info_extra =
            get_option_info( OUSTR("extra") );
        OptionInfo const * info_version =
            get_option_info( OUSTR("assembly-version") );
        OptionInfo const * info_product =
            get_option_info( OUSTR("assembly-product") );
        OptionInfo const * info_description =
            get_option_info( OUSTR("assembly-description") );
        OptionInfo const * info_company =
            get_option_info( OUSTR("assembly-company") );
        OptionInfo const * info_copyright =
            get_option_info( OUSTR("assembly-copyright") );
        OptionInfo const * info_trademark =
            get_option_info( OUSTR("assembly-trademark") );

        OUString output;
        vector< OUString > mandatory_registries;
        vector< OUString > extra_registries;
        vector< OUString > extra_assemblies;
        vector< OUString > explicit_types;
        OUString version, product, description, company, copyright, trademark;

        OUString cmd_arg;
        for ( sal_uInt32 nPos = 0; nPos < nCount; )
        {
            // options
            if (is_option( info_help, &nPos ))
            {
                printf( s_usingText );
                return 0;
            }
            else if (read_argument( &cmd_arg, info_types, &nPos ))
            {
                sal_Int32 index = 0;
                do
                {
                    explicit_types.push_back(
                        cmd_arg.getToken( 0, ';', index ) );
                }
                while (index >= 0);
            }
            else if (read_argument( &cmd_arg, info_extra, &nPos ))
            {
                extra_registries.push_back(
                    path_make_absolute_file_url( cmd_arg ) );
            }
            else if (read_argument( &cmd_arg, info_reference, &nPos ))
            {
                extra_assemblies.push_back(
                    path_make_absolute_file_url( cmd_arg ) );
            }
            else if (!read_option( &g_verbose, info_verbose, &nPos ) &&
                     !read_argument( &output, info_out, &nPos ) &&
                     !read_argument( &version, info_version, &nPos ) &&
                     !read_argument( &description, info_description, &nPos ) &&
                     !read_argument( &product, info_product, &nPos ) &&
                     !read_argument( &company, info_company, &nPos ) &&
                     !read_argument( &copyright, info_copyright, &nPos ) &&
                     !read_argument( &trademark, info_trademark, &nPos ))
            {
                oslProcessError rc = osl_getCommandArg( nPos, &cmd_arg.pData );
                OSL_ASSERT( rc == osl_Process_E_None );
                ++nPos;
                cmd_arg = cmd_arg.trim();
                if (cmd_arg.getLength() > 0)
                {
                    if (cmd_arg[ 0 ] == '-') // is option
                    {
                        OptionInfo const * option_info = 0;
                        if (cmd_arg.getLength() > 2 &&
                            cmd_arg[ 1 ] == '-')
                        {
                            // long option
                            option_info = get_option_info(
                                cmd_arg.copy( 2 ), '\0' );
                        }
                        else if (cmd_arg.getLength() == 2 &&
                                 cmd_arg[ 1 ] != '-')
                        {
                            // short option
                            option_info = get_option_info(
                                OUString(), cmd_arg[ 1 ] );
                        }
                        if (option_info == 0)
                        {
                            OUStringBuffer buf;
                            buf.appendAscii(
                                RTL_CONSTASCII_STRINGPARAM("unknown option ") );
                            buf.append( cmd_arg );
                            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(
                                                 "!  Use climaker --help "
                                                 "to print all options.") );
                            throw RuntimeException(
                                buf.makeStringAndClear(),
                                Reference< XInterface >() );
                        }
                        else
                        {
                            OSL_ENSURE( 0, "unhandled valid option?!" );
                            if (option_info->m_has_argument)
                                ++nPos;
                        }
                    }
                    else
                    {
                        mandatory_registries.push_back(
                            path_make_absolute_file_url( cmd_arg ) );
                    }
                }
            }
        }

        // bootstrap uno
        xContext = ::cppu::bootstrap_InitialComponentContext(
            Reference< registry::XSimpleRegistry >() );
        Reference< container::XHierarchicalNameAccess > xTDmgr(
            xContext->getValueByName(
                OUSTR("/singletons/com.sun.star.reflection."
                      "theTypeDescriptionManager") ),
            UNO_QUERY_THROW );

        // get rdb tdprovider factory
        Reference< lang::XSingleComponentFactory > xTDprov_factory(
            ::cppu::loadSharedLibComponentFactory(
                OUSTR("regtypeprov.uno" SAL_DLLEXTENSION), OUString(),
                OUSTR("com.sun.star.comp.stoc.RegistryTypeDescriptionProvider"),
                Reference< lang::XMultiServiceFactory >(
                    xContext->getServiceManager(), UNO_QUERY ),
                Reference< registry::XRegistryKey >() ), UNO_QUERY );
        if (! xTDprov_factory.is())
        {
            throw RuntimeException(
                OUSTR("cannot get registry typedescription provider: "
                      "regtypeprov.uno" SAL_DLLEXTENSION "!"),
                Reference< XInterface >() );
        }

        // create registry td provider for mandatory registry files
        Any arg( makeAny( open_registries( mandatory_registries, xContext ) ) );
        Reference< XInterface > xTD_provider(
            xTDprov_factory->createInstanceWithArgumentsAndContext(
                Sequence< Any >( &arg, 1 ), xContext ) );
        // insert provider to tdmgr
        Reference< container::XSet > xSet( xTDmgr, UNO_QUERY_THROW );
        Any provider( makeAny( xTD_provider ) );
        xSet->insert( provider );
        OSL_ASSERT( xSet->has( provider ) );
        if (! extra_registries.empty())
        {
            arg = makeAny( open_registries( extra_registries, xContext ) );
            provider = makeAny(
                xTDprov_factory->createInstanceWithArgumentsAndContext(
                    Sequence< Any >( &arg, 1 ), xContext ) );
            xSet->insert( provider );
            OSL_ASSERT( xSet->has( provider ) );
        }

        if (0 == output.getLength()) // no output file specified
        {
            // if only one rdb has been given, then take rdb name
            if (1 == mandatory_registries.size())
            {
                output = mandatory_registries[ 0 ];
                output = output.copy( output.lastIndexOf( '/' ) +1 );
                sal_Int32 dot = output.lastIndexOf( '.' );
                if (dot > 0)
                    output = output.copy( 0, dot );
            }
            else
            {
                output = OUSTR("cli_unotypes");
            }
        }
        output = path_make_absolute_file_url( output );
        sal_Int32 slash = output.lastIndexOf( '/' );
        OUString sys_output_dir;
        if (FileBase::E_None != FileBase::getSystemPathFromFileURL(
                output.copy( 0, slash ), sys_output_dir ))
        {
            throw RuntimeException(
                OUSTR("cannot get system path from file url ") +
                output.copy( 0, slash ),
                Reference< XInterface >() );
        }
        OUString filename( output.copy( slash +1 ) );
        sal_Int32 dot = filename.lastIndexOf( '.' );
        OUString name( filename );
        if (dot < 0) // has no extension
            filename += OUSTR(".dll");
        else
            name = name.copy( 0, dot );
        ::System::String * output_dir = ustring_to_String( sys_output_dir );
        ::System::String * output_file = ustring_to_String( filename );

        // setup assembly info: xxx todo set more? e.g. avoid strong versioning
        AssemblyName * assembly_name = new AssemblyName();
        assembly_name->set_CodeBase( output_dir );
        assembly_name->set_Name( name );
        if (version.getLength() != 0)
        {
            assembly_name->set_Version(
                new ::System::Version( ustring_to_String( version ) ) );
        }

        // app domain
        ::System::AppDomain * current_appdomain =
              ::System::AppDomain::get_CurrentDomain();
        // target assembly
        Emit::AssemblyBuilder * assembly_builder =
            current_appdomain->DefineDynamicAssembly(
                assembly_name, Emit::AssemblyBuilderAccess::Save, output_dir );
        if (product.getLength() != 0)
        {
            ::System::Type * params __gc [] = new ::System::Type * __gc [ 1 ];
            ::System::Object * args __gc [] = new ::System::Object * __gc [ 1 ];
            params[ 0 ] = __typeof (::System::String);
            args[ 0 ] = ustring_to_String( product );
            assembly_builder->SetCustomAttribute(
                new Emit::CustomAttributeBuilder(
                    __typeof (AssemblyProductAttribute)->GetConstructor(
                        params ), args ) );
        }
        if (description.getLength() != 0)
        {
            ::System::Type * params __gc [] = new ::System::Type * __gc [ 1 ];
            ::System::Object * args __gc [] = new ::System::Object * __gc [ 1 ];
            params[ 0 ] = __typeof (::System::String);
            args[ 0 ] = ustring_to_String( description );
            assembly_builder->SetCustomAttribute(
                new Emit::CustomAttributeBuilder(
                    __typeof (AssemblyDescriptionAttribute)->GetConstructor(
                        params ), args ) );
        }
        if (company.getLength() != 0)
        {
            ::System::Type * params __gc [] = new ::System::Type * __gc [ 1 ];
            ::System::Object * args __gc [] = new ::System::Object * __gc [ 1 ];
            params[ 0 ] = __typeof (::System::String);
            args[ 0 ] = ustring_to_String( company );
            assembly_builder->SetCustomAttribute(
                new Emit::CustomAttributeBuilder(
                    __typeof (AssemblyCompanyAttribute)->GetConstructor(
                        params ), args ) );
        }
        if (copyright.getLength() != 0)
        {
            ::System::Type * params __gc [] = new ::System::Type * __gc [ 1 ];
            ::System::Object * args __gc [] = new ::System::Object * __gc [ 1 ];
            params[ 0 ] = __typeof (::System::String);
            args[ 0 ] = ustring_to_String( copyright );
            assembly_builder->SetCustomAttribute(
                new Emit::CustomAttributeBuilder(
                    __typeof (AssemblyCopyrightAttribute)->GetConstructor(
                        params ), args ) );
        }
        if (trademark.getLength() != 0)
        {
            ::System::Type * params __gc [] = new ::System::Type * __gc [ 1 ];
            ::System::Object * args __gc [] = new ::System::Object * __gc [ 1 ];
            params[ 0 ] = __typeof (::System::String);
            args[ 0 ] = ustring_to_String( trademark );
            assembly_builder->SetCustomAttribute(
                new Emit::CustomAttributeBuilder(
                    __typeof (AssemblyTrademarkAttribute)->GetConstructor(
                        params ), args ) );
        }

        // load extra assemblies
        Assembly * assemblies __gc [] =
            new Assembly * __gc [ extra_assemblies.size() ];
        for ( size_t pos = 0; pos < extra_assemblies.size(); ++pos )
        {
            assemblies[ pos ] = Assembly::LoadFrom(
                ustring_to_String( extra_assemblies[ pos ] ) );
        }

        // type emitter
        TypeEmitter * type_emitter = new TypeEmitter(
            assembly_builder->DefineDynamicModule( output_file ), assemblies );
        // add handler resolving assembly's types
        ::System::ResolveEventHandler * type_resolver =
              new ::System::ResolveEventHandler(
                  type_emitter, &TypeEmitter::type_resolve );
        current_appdomain->add_TypeResolve( type_resolver );

        // and emit types to it
        if (explicit_types.empty())
        {
            Reference< reflection::XTypeDescriptionEnumeration > xTD_enum(
                Reference< reflection::XTypeDescriptionEnumerationAccess >(
                    xTD_provider, UNO_QUERY_THROW )
                  ->createTypeDescriptionEnumeration(
                      OUString() /* all IDL modules */,
                      Sequence< TypeClass >() /* all classes of types */,
                      reflection::TypeDescriptionSearchDepth_INFINITE ) );
            while (xTD_enum->hasMoreElements())
            {
                type_emitter->get_type( xTD_enum->nextTypeDescription() );
            }
        }
        else
        {
            Reference< container::XHierarchicalNameAccess > xHNA(
                xTD_provider, UNO_QUERY_THROW );
            for ( size_t nPos = explicit_types.size(); nPos--; )
            {
                type_emitter->get_type(
                    Reference< reflection::XTypeDescription >(
                        xHNA->getByHierarchicalName( explicit_types[ nPos ] ),
                        UNO_QUERY_THROW ) );
            }
        }
        type_emitter->Dispose();

        if (g_verbose)
        {
            ::System::Console::Write(
                S"> saving assembly {0}{1}{2}...",
                output_dir,
                new ::System::String(
                    ::System::IO::Path::DirectorySeparatorChar, 1 ),
                output_file );
        }
        assembly_builder->Save( output_file );
        if (g_verbose)
        {
            ::System::Console::WriteLine( S"ok." );
        }
        current_appdomain->remove_TypeResolve( type_resolver );
    }
    catch (Exception & exc)
    {
        OString msg(
            OUStringToOString( exc.Message, osl_getThreadTextEncoding() ) );
        fprintf(
            stderr, "\n> error: %s\n> dying abnormally...\n", msg.getStr() );
        ret = 1;
    }
    catch (::System::Exception * exc)
    {
        OString msg( OUStringToOString(
                         String_to_ustring( exc->ToString() ),
                         osl_getThreadTextEncoding() ) );
        fprintf(
            stderr,
            "\n> error: .NET exception occured: %s\n> dying abnormally...",
            msg.getStr() );
        ret = 1;
    }

    try
    {
        Reference< lang::XComponent > xComp( xContext, UNO_QUERY );
        if (xComp.is())
            xComp->dispose();
    }
    catch (Exception & exc)
    {
        OString msg(
            OUStringToOString( exc.Message, osl_getThreadTextEncoding() ) );
        fprintf(
            stderr,
            "\n> error disposing component context: %s\n"
            "> dying abnormally...\n",
            msg.getStr() );
        ret = 1;
    }

    return ret;
}
