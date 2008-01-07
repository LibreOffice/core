/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: globalx.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-07 08:58:27 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"


#include "callform.hxx"
#include "global.hxx"

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _UCBHELPER_CONTENTBROKER_HXX
#include <ucbhelper/contentbroker.hxx>
#endif
#ifndef _UCBHELPER_CONTENT_HXX
#include <ucbhelper/content.hxx>
#endif
#ifndef _UNOTOOLS_LOCALFILEHELPER_HXX
#include <unotools/localfilehelper.hxx>
#endif

#include <tools/debug.hxx>
#include <svtools/pathoptions.hxx>

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
