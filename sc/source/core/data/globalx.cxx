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
#include "precompiled_sc.hxx"


#include "callform.hxx"
#include "global.hxx"
#include <tools/urlobj.hxx>
#include <ucbhelper/contentbroker.hxx>
#include <ucbhelper/content.hxx>
#include <unotools/localfilehelper.hxx>

#include <tools/debug.hxx>
#include <unotools/pathoptions.hxx>

#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>

#include <com/sun/star/i18n/XOrdinalSuffix.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>
#include <unotools/localedatawrapper.hxx>


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;


// static
void ScGlobal::InitAddIns()
{
    // multi paths separated by semicolons
    SvtPathOptions aPathOpt;
    String aMultiPath = aPathOpt.GetAddinPath();
    if ( aMultiPath.Len() > 0 )
    {
        xub_StrLen nTokens = aMultiPath.GetTokenCount( ';' );
        xub_StrLen nIndex = 0;
        for ( xub_StrLen j=0; j<nTokens; j++ )
        {
            String aPath( aMultiPath.GetToken( 0, ';', nIndex ) );
            if ( aPath.Len() > 0 )
            {
                //  use LocalFileHelper to convert the path to a URL that always points
                //  to the file on the server
                String aUrl;
                if ( utl::LocalFileHelper::ConvertPhysicalNameToURL( aPath, aUrl ) )
                    aPath = aUrl;

                INetURLObject aObj;
                aObj.SetSmartURL( aPath );
                aObj.setFinalSlash();
                try
                {
                    ::ucbhelper::Content aCnt( aObj.GetMainURL(INetURLObject::NO_DECODE),
                        Reference< XCommandEnvironment > () );
                    Reference< sdbc::XResultSet > xResultSet;
                    Sequence< rtl::OUString > aProps;
                    try
                    {
                        xResultSet = aCnt.createCursor(
                            aProps, ::ucbhelper::INCLUDE_DOCUMENTS_ONLY );
                    }
                    catch ( Exception& )
                    {
                        // ucb may throw different exceptions on failure now
                        // no assertion if AddIn directory doesn't exist
                    }

                    if ( xResultSet.is() )
                    {
                        Reference< sdbc::XRow > xRow( xResultSet, UNO_QUERY );
                        Reference< XContentAccess >
                            xContentAccess( xResultSet, UNO_QUERY );
                        try
                        {
                            if ( xResultSet->first() )
                            {
                                do
                                {
                                    rtl::OUString aId( xContentAccess->queryContentIdentifierString() );
                                    InitExternalFunc( aId );
                                }
                                while ( xResultSet->next() );
                            }
                        }
                        catch ( Exception& )
                        {
                            DBG_ERRORFILE( "ResultSetException catched!" );
                        }
                    }
                }
                catch ( Exception& )
                {
                    DBG_ERRORFILE( "Exception catched!" );
                }
                catch ( ... )
                {

                    DBG_ERRORFILE( "unexpected exception caught!" );
                }
            }
        }
    }
}


// static
String ScGlobal::GetOrdinalSuffix( sal_Int32 nNumber)
{
    if (!xOrdinalSuffix.is())
    {
        try
        {
            Reference< lang::XMultiServiceFactory > xServiceManager =
                ::comphelper::getProcessServiceFactory();
            Reference< XInterface > xInterface =
                xServiceManager->createInstance(
                    ::rtl::OUString::createFromAscii("com.sun.star.i18n.OrdinalSuffix"));
            if  (xInterface.is())
                xOrdinalSuffix = Reference< i18n::XOrdinalSuffix >( xInterface, UNO_QUERY);
        }
        catch ( Exception& )
        {
            DBG_ERRORFILE( "GetOrdinalSuffix: exception caught during init" );
        }
    }
    DBG_ASSERT( xOrdinalSuffix.is(), "GetOrdinalSuffix: createInstance failed");
    if (xOrdinalSuffix.is())
    {
        try
        {
            return xOrdinalSuffix->getOrdinalSuffix( nNumber,
                    ScGlobal::pLocaleData->getLocale());
        }
        catch ( Exception& )
        {
            DBG_ERRORFILE( "GetOrdinalSuffix: exception caught during getOrdinalSuffix" );
        }
    }
    return String();
}
