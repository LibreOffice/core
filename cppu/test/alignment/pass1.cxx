/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pass1.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 09:01:18 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include <stdio.h>
#include <vector>

#include <rtl/string.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/bootstrap.hxx>
#include <rtl/process.h>

#include <registry/reflread.hxx>

// starting the executable:
// -env:UNO_CFG_URL=local;<absolute_path>..\\..\\test\\cfg_data;<absolute_path>\\cfg_update
// -env:UNO_TYPES=cpputest.rdb

#include <cppuhelper/bootstrap.hxx>

#include <com/sun/star/lang/XComponent.hpp>

#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )
#define OSTR(x) ::rtl::OUStringToOString( x, RTL_TEXTENCODING_ASCII_US )


using namespace ::cppu;
using namespace ::rtl;
using namespace ::osl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;


static void find_all_structs(
    Reference< registry::XRegistryKey > const & xKey,
    ::std::vector< OUString > * pNames )
{
    static RegistryTypeReaderLoader s_loader;
    OSL_VERIFY( s_loader.isLoaded() );

    if (xKey.is() && xKey->isValid())
    {
        if (xKey->getValueType() == registry::RegistryValueType_BINARY)
        {
            Sequence< sal_Int8 > aBytes( xKey->getBinaryValue() );
            RegistryTypeReader aReader(
                s_loader, (const sal_uInt8 *)aBytes.getConstArray(),
                aBytes.getLength(), sal_False );

            switch (aReader.getTypeClass())
            {
            case RT_TYPE_EXCEPTION:
            case RT_TYPE_STRUCT:
                pNames->push_back( aReader.getTypeName().replace( '/', '.' ) );
                break;
            }
        }

        Sequence< Reference< registry::XRegistryKey > > keys( xKey->openKeys() );
        Reference< registry::XRegistryKey > const * pKeys = keys.getConstArray();
        for ( sal_Int32 nPos = keys.getLength(); nPos--; )
        {
            find_all_structs( pKeys[ nPos ], pNames );
        }
    }
}

static OString makeIncludeName( OUString const & name ) SAL_THROW( () )
{
    return OSTR(name.replace( '.', '/' ));
}
static OString makeCppName( OUString const & name ) SAL_THROW( () )
{
    OStringBuffer buf( 64 );
    OString str( OSTR(name) );
    sal_Int32 n = 0;
    do
    {
        buf.append( str.getToken( 0, '.', n ) );
        if (n >= 0)
            buf.append( "::" );
    }
    while (n >= 0);
    return buf.makeStringAndClear();
}

//==================================================================================================
int SAL_CALL main( int, char const ** )
{
    sal_Int32 argc = rtl_getAppCommandArgCount();
    if (argc < 1)
    {
        fprintf( stderr, "usage: pass1 pass2_source [typelist_to_stdout]\n" );
        return 1;
    }

    try
    {
        // determine types rdb
        OUString rdb_name;
        Bootstrap bootstrap;
        if (!bootstrap.getFrom( OUSTR("UNO_TYPES"), rdb_name ) || !rdb_name.getLength())
        {
            fprintf(
                stderr,
                "### no UNO_TYPES registry found!!!\n\n"
                "usage: pass1 pass2_source [typelist_to_stdout]\n" );
            return 1;
        }

        Reference< XComponentContext > xContext( defaultBootstrap_InitialComponentContext() );

        // read out all struct names from given registry
        Reference< registry::XSimpleRegistry > xSimReg( createSimpleRegistry() );
        OSL_ASSERT( xSimReg.is() );
        xSimReg->open( rdb_name, sal_True, sal_False );
        OSL_ASSERT( xSimReg->isValid() );
        Reference< registry::XRegistryKey > xKey( xSimReg->getRootKey() );
        OSL_ASSERT( xKey.is() && xKey->isValid() );

        ::std::vector< OUString > names;
        names.reserve( 128 );
        find_all_structs( xKey->openKey( OUSTR("UCR") ), &names );

        OUString fileName;
        OSL_VERIFY( osl_Process_E_None == rtl_getAppCommandArg( 0, &fileName.pData ) );
        bool bDumpStdOut = (argc > 1);

        // generate pass2 output file [and type list]
        OString str( OSTR(fileName) );
        FILE * hPass2 = fopen( str.getStr(), "w" );
        OSL_ASSERT( hPass2 );

        size_t nPos;
        for ( nPos = names.size(); nPos--; )
        {
            OUString const & name = names[ nPos ];
            if (bDumpStdOut)
            {
                // type name on stdout
                OString str( OSTR(name) );
                fprintf( stdout, "%s\n", str.getStr() );
            }
            // all includes
            OString includeName( makeIncludeName( name ) );
            fprintf( hPass2, "#include <%s.hdl>\n", includeName.getStr() );
        }
        // include diagnose.h
        fprintf(
            hPass2,
            "\n#include <diagnose.h>\n\n"
            "int SAL_CALL main( int argc, char const * argv[] )\n{\n" );
        // generate all type checks
        for ( nPos = names.size(); nPos--; )
        {
            OUString const & name = names[ nPos ];
            typelib_TypeDescription * pTD = 0;
            typelib_typedescription_getByName( &pTD, name.pData );
            if (pTD)
            {
                if (! pTD->bComplete)
                {
                    typelib_typedescription_complete( &pTD );
                }
                typelib_CompoundTypeDescription * pCTD = (typelib_CompoundTypeDescription *)pTD;

                OString cppName( makeCppName( name ) );
                fprintf(
                    hPass2, "\tBINTEST_VERIFYSIZE( %s, %d );\n",
                    cppName.getStr(), pTD->nSize );
                fprintf(
                    hPass2, "\tBINTEST_VERIFYALIGNMENT( %s, %d );\n",
                    cppName.getStr(), pTD->nAlignment );
                // offset checks
                for ( sal_Int32 nPos = pCTD->nMembers; nPos--; )
                {
                    OString memberName( OSTR(pCTD->ppMemberNames[ nPos ]) );
                    fprintf(
                        hPass2, "\tBINTEST_VERIFYOFFSET( %s, %s, %d );\n",
                        cppName.getStr(), memberName.getStr(), pCTD->pMemberOffsets[ nPos ] );
                }
                typelib_typedescription_release( pTD );
            }
            else
            {
                OString str( OSTR(name) );
                fprintf( stderr, "### cannot dump type %s!!!\n", str.getStr() );
            }
        }
        fprintf(
            hPass2,
            "\n\tfprintf( stdout, \"> alignment test succeeded.\\n\" );\n"
            "\treturn 0;\n}\n\n" );
        fclose( hPass2 );

        Reference< lang::XComponent > xComp( xContext, UNO_QUERY );
        if (xComp.is())
        {
            xComp->dispose();
        }
        return 0;
    }
    catch (Exception & exc)
    {
        OString str( OUStringToOString( exc.Message, RTL_TEXTENCODING_ASCII_US ) );
        ::fprintf( stderr, "# caught exception: %s\n", str.getStr() );
        return 1;
    }
}
