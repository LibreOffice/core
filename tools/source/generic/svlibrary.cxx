/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_tools.hxx"

#include <tools/svlibrary.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/XMacroExpander.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <comphelper/processfactory.hxx>
#include <tools/string.hxx>
#include <rtl/uri.hxx>

using namespace com::sun::star;

static uno::Sequence< rtl::OUString > GetMultiPaths_Impl()
{
    uno::Sequence< rtl::OUString >   aRes;
    uno::Sequence< rtl::OUString >   aInternalPaths;
    uno::Sequence< rtl::OUString >   aUserPaths;

    bool bSuccess = true;
    uno::Reference< lang::XMultiServiceFactory >  xMgr( comphelper::getProcessServiceFactory() );
    if (xMgr.is())
    {
        try
        {
            String aInternal;
            aInternal.AppendAscii("Libraries");
            String aUser;
            aUser.AppendAscii("Libraries");
            aInternal .AppendAscii( "_internal" );
            aUser     .AppendAscii( "_user" );

            uno::Reference< beans::XPropertySet > xPathSettings( xMgr->createInstance(
                rtl::OUString::createFromAscii( "com.sun.star.util.PathSettings" ) ), uno::UNO_QUERY_THROW );
            xPathSettings->getPropertyValue( aInternal )  >>= aInternalPaths;
            xPathSettings->getPropertyValue( aUser )      >>= aUserPaths;
        }
        catch (uno::Exception &)
        {
            bSuccess = false;
        }
    }
    if (bSuccess)
    {
        sal_Int32 nMaxEntries = aInternalPaths.getLength() + aUserPaths.getLength();
        aRes.realloc( nMaxEntries );
        rtl::OUString *pRes = aRes.getArray();
        sal_Int32 nCount = 0;   // number of actually added entries
        for (int i = 0;  i < 2;  ++i)
        {
            const uno::Sequence< rtl::OUString > &rPathSeq = i == 0 ? aUserPaths : aInternalPaths;
            const rtl::OUString *pPathSeq = rPathSeq.getConstArray();
            for (sal_Int32 k = 0;  k < rPathSeq.getLength();  ++k)
            {
                const bool bAddUser     = (&rPathSeq == &aUserPaths);
                const bool bAddInternal = (&rPathSeq == &aInternalPaths);
                if ((bAddUser || bAddInternal) && pPathSeq[k].getLength() > 0)
                    pRes[ nCount++ ] = pPathSeq[k];
            }
        }
        aRes.realloc( nCount );
    }

    return aRes;
}

bool SvLibrary::LoadModule( osl::Module& rModule, const rtl::OUString& rLibName, ::oslGenericFunction baseModule, ::sal_Int32 mode )
{
    static uno::Sequence < rtl::OUString > aPaths = GetMultiPaths_Impl();   
    bool bLoaded = false;

    for (sal_Int32 n=0; n<aPaths.getLength(); n++)
    {
        rtl::OUString aMod = aPaths[n];
        if ( aPaths[n].indexOfAsciiL("vnd.sun.star.expand",19) == 0)
        {
            uno::Reference< uno::XComponentContext > xComponentContext = comphelper::getProcessComponentContext();
            uno::Reference< util::XMacroExpander > xMacroExpander;
            xComponentContext->getValueByName(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("/singletons/com.sun.star.util.theMacroExpander") ) )
                    >>= xMacroExpander;

            aMod = aMod.copy( sizeof("vnd.sun.star.expand:") -1 );
            aMod = ::rtl::Uri::decode( aMod, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8 );
            aMod = xMacroExpander->expandMacros( aMod ); 
        }

        aMod += ::rtl::OUString( sal_Unicode('/') );
        aMod += rLibName;
        bLoaded = rModule.load( aMod, mode );
        if ( bLoaded )
            break;
    }
    
    if (!bLoaded )
        bLoaded = rModule.loadRelative( baseModule, rLibName, mode );

    return bLoaded;
}
