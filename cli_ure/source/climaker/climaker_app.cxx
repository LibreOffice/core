/*************************************************************************
 *
 *  $RCSfile: climaker_app.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: dbo $ $Date: 2003-07-16 10:42:21 $
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

bool g_verbose = false;

//------------------------------------------------------------------------------
static bool read_option(
    sal_Unicode copt, OUString const & opt, sal_uInt32 * pIndex )
{
    if (osl_getCommandArgCount() <= *pIndex)
        return false;

    OUString arg;
    osl_getCommandArg( *pIndex, &arg.pData );
    sal_Int32 len = arg.getLength();

    if (len < 2 || '-' != arg[ 0 ])
        return false;

    if (2 == len && copt == arg[ 1 ])
    {
        ++(*pIndex);
#if OSL_DEBUG_LEVEL > 1
        OSL_TRACE( __FILE__": identified option \'%c\'\n", copt );
#endif
        return true;
    }
    if ('-' == arg[ 1 ] &&
        0 == rtl_ustr_compare( arg.pData->buffer + 2, opt.pData->buffer ))
    {
        ++(*pIndex);
#if OSL_DEBUG_LEVEL > 1
        OString cstr_opt(
            OUStringToOString( opt, osl_getThreadTextEncoding() ) );
        OSL_TRACE( __FILE__": identified option \'%s\'\n", cstr_opt.getStr() );
#endif
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
static bool read_argument(
    OUString * pValue, sal_Unicode copt, OUString const & opt,
    sal_uInt32 * pIndex )
{
    if (read_option( copt, opt, pIndex ))
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

//------------------------------------------------------------------------------
static OUString const & path_get_working_dir()
{
    static OUString s_workingDir;
    if (! s_workingDir.getLength())
        osl_getProcessWorkingDir( &s_workingDir.pData );
    return s_workingDir;
}

//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
static char const s_usingText [] =
"\n"
"using: climaker <switches> [registry-file-1 registry-file-2 ...]\n"
"\n"
"switches:\n"
" -O, --out <output-file>     output assembly file;\n"
"                             defaults to cli_unotypes.dll if more than one\n"
"                             registry-file is given, else <registry-file>.dll\n"
" -T, --types                 types to be generated (if none is given,\n"
"   <type1[;type2;...]>       then all types of given registries are emitted\n"
" -X, --extra <rdb-file>      additional rdb to saturate referenced types in\n"
"                             given registry file(s); these types will not be\n"
"                             emitted into the output assembly file\n"
" -r, --reference             reference metadata from assembly file\n"
"   <assembly-file>\n"
" --version <version>         sets assembly version\n"
" --product <name>            sets assembly product name\n"
" --description <text>        sets assembly description text\n"
" -v, --verbose               verbose output to stdout\n"
" -h, --help                  this message\n"
"\n"
"example: climaker --out cli_mytypes.dll \\\n"
"                  --reference cli_types.dll \\\n"
"                  --extra types.rdb \\\n"
"                  mytypes.rdb\n"
"\n";

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
        OUString str_help = OUSTR("help");
        OUString str_verbose = OUSTR("verbose");
        OUString str_out = OUSTR("out");
        OUString str_reference = OUSTR("reference");
        OUString str_types = OUSTR("types");
        OUString str_extra = OUSTR("extra");
        OUString str_version = OUSTR("version");
        OUString str_product = OUSTR("product");
        OUString str_description = OUSTR("description");

        OUString output;
        vector< OUString > mandatory_registries;
        vector< OUString > extra_registries;
        vector< OUString > extra_assemblies;
        vector< OUString > explicit_types;
        OUString version, product, description;

        OUString cmd_arg;
        for ( sal_uInt32 nPos = 0; nPos < nCount; )
        {
            // options
            if (read_option( 'h', str_help, &nPos ))
            {
                printf( s_usingText );
                return 0;
            }
            else if (read_option( 'v', str_verbose, &nPos ))
            {
                g_verbose = true;
            }
            else if (read_argument( &cmd_arg, 'T', str_types, &nPos ))
            {
                sal_Int32 index = 0;
                do
                {
                    explicit_types.push_back(
                        cmd_arg.getToken( 0, ';', index ) );
                }
                while (index >= 0);
            }
            else if (read_argument( &cmd_arg, 'X', str_extra, &nPos ))
            {
                extra_registries.push_back(
                    path_make_absolute_file_url( cmd_arg ) );
            }
            else if (read_argument( &cmd_arg, 'r', str_reference, &nPos ))
            {
                extra_assemblies.push_back(
                    path_make_absolute_file_url( cmd_arg ) );
            }
            else if (!read_argument( &version, '\0', str_version, &nPos ) &&
                     !read_argument( &product, '\0', str_product, &nPos ) &&
                     !read_argument(
                         &description, '\0', str_description, &nPos ) &&
                     !read_argument( &output, 'O', str_out, &nPos ))
            {
                OSL_VERIFY(
                    osl_Process_E_None == osl_getCommandArg(
                        nPos, &cmd_arg.pData ) );
                ++nPos;
                cmd_arg = cmd_arg.trim();
                if (cmd_arg.getLength() && '-' != cmd_arg[ 0 ]) // no option
                {
                    mandatory_registries.push_back(
                        path_make_absolute_file_url( cmd_arg ) );
                }
                else
                {
                    throw RuntimeException(
                        OUSTR("unknown option ") + cmd_arg,
                        Reference< XInterface >() );
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
        if (0 != version.getLength())
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
        if (0 != product.getLength())
        {
            ::System::Type * params __gc [] = new ::System::Type * __gc [ 1 ];
            ::System::Object * args __gc [] = new ::System::Object * __gc [ 1 ];
            params[ 0 ] = __typeof (::System::String);
            args[ 0 ] = ustring_to_String( product );
            assembly_builder->SetCustomAttribute(
                new Emit::CustomAttributeBuilder(
                    __typeof (AssemblyProductAttribute) ->GetConstructor(
                        params ), args ) );
        }
        if (0 != description.getLength())
        {
            ::System::Type * params __gc [] = new ::System::Type * __gc [ 1 ];
            ::System::Object * args __gc [] = new ::System::Object * __gc [ 1 ];
            params[ 0 ] = __typeof (::System::String);
            args[ 0 ] = ustring_to_String( description );
            assembly_builder->SetCustomAttribute(
                new Emit::CustomAttributeBuilder(
                    __typeof (AssemblyDescriptionAttribute)
                      ->GetConstructor( params ), args ) );
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
