/*************************************************************************
 *
 *  $RCSfile: mediatypedetectionhelper.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: as $ $Date: 2000-11-23 14:52:10 $
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

#ifndef __FRAMEWORK_BAEHSERVICES_MEDIATYPEDETECTIONHELPER_HXX_
#include <baeh_services/mediatypedetectionhelper.hxx>
#endif

#ifndef __FRAMEWORK_DEFINES_HXX_
#include <defines.hxx>
#endif

#ifndef _INETTYPE_HXX
#include <svtools/inettype.hxx>
#endif

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#if !TF_FILTER//MUSTFILTER
#ifndef _COM_SUN_STAR_FRAME_XFRAMELOADERQUERY_HPP_
#include <com/sun/star/frame/XFrameLoaderQuery.hpp>
#endif
#endif//MUSTFILTER
//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework
{

#if !TF_FILTER//MUSTFILTER
using namespace ::com::sun::star::frame ;
#endif//MUSTFILTER
using namespace ::com::sun::star        ;
using namespace ::rtl                   ;

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

//*****************************************************************************************************************
//  constructor
//*****************************************************************************************************************
MediaTypeDetectionHelper::MediaTypeDetectionHelper( const uno::Reference< lang::XMultiServiceFactory >& xFactory )
    : m_xFactory( xFactory )
{
}

//*****************************************************************************************************************
//  destructor
//*****************************************************************************************************************
MediaTypeDetectionHelper::~MediaTypeDetectionHelper()
{
}

//*****************************************************************************************************************
//  XInterface, XTypeProvider, XServiceInfo
//*****************************************************************************************************************

DEFINE_XINTERFACE_3                 (   MediaTypeDetectionHelper
                                        , OWeakObject
                                        , DIRECT_INTERFACE( lang::XTypeProvider )
                                        , DIRECT_INTERFACE( lang::XServiceInfo  )
                                        , DIRECT_INTERFACE( util::XStringMapping )
                                    )

DEFINE_XTYPEPROVIDER_3              (   MediaTypeDetectionHelper
                                        , lang::XTypeProvider
                                        , lang::XServiceInfo
                                        , util::XStringMapping
                                    )

DEFINE_XSERVICEINFO_ONEINSTANCESERVICE  (   MediaTypeDetectionHelper
                                        , SERVICENAME_MEDIATYPEDETECTIONHELPER
                                        , IMPLEMENTATIONNAME_MEDIATYPEDETECTIONHELPER
                                    )

//*****************************************************************************************************************
//   XStringMapping
//*****************************************************************************************************************

//virtual
sal_Bool SAL_CALL MediaTypeDetectionHelper::mapStrings(
        uno::Sequence< OUString >& rSeq )
        throw(uno::RuntimeException)
{
    //osl::Guard aGuard( m_aMutex );
    //uno::Reference<   lang::XMultiServiceFactory > xFactory

    if( !m_xFactory.is() )
    {
        LOG_ASSERT( !(m_xFactory.is()==sal_False), "MediaTypeDetectionHelper::mapStrings()\n member m_xFactory( XMultiServiceFactory ) not set\n" )
        return sal_False;
    }

#if !TF_FILTER//MUSTFILTER
    uno::Reference< frame::XFrameLoaderQuery > xQ(
        m_xFactory->createInstance( SERVICENAME_FRAMELOADERFACTORY ),uno::UNO_QUERY );
    //IMPLEMENTATIONNAME_FRAMELOADERFACTORY

    if( !xQ.is() )
    {
        LOG_ASSERT( !(xQ.is()==sal_False), "MediaTypeDetectionHelper::mapStrings()\n could not get Service FrameLoaderQuery\n" )
        return sal_False;
    }
#endif//MUSTFILTER

    sal_Bool bModified = sal_False;
    //uno::Sequence< beans::PropertyValue > aTmpSeq(1);
    //aTmpSeq[0].Name = OUString(RTL_CONSTASCII_USTRINGPARAM("ContentType"));
    for( sal_Int32 i = rSeq.getLength(); i--; )
    {

        OUString& rUrl = rSeq[i];
        INetContentType eType = INetContentTypes::GetContentTypeFromURL( rUrl );

        UniString aType( INetContentTypes::GetContentType( eType ) );
        if( aType.Len() )
        {
            rUrl = aType;
            bModified = sal_True;
        }
        /*
        OUString aFilter( xQ->searchFilter( rUrl, aTmpSeq ) );
        if( !aFilter.getLength() )
            continue;
        uno::Sequence< beans::PropertyValue > aProps = xQ->getLoaderProperties( aFilter );
        for( sal_Int32 nN = aProps.getLength(); nN--; )
        {
            const beans::PropertyValue& rProp = aProps[nN];
            if( rProp.Name.compareToAscii("ContentType") == 0 )
            {
                rProp.Value >>= rUrl;
                bModified = sal_True;
                break;
            }
        }
        */
    }
    return bModified;
}

}   // namespace framework
/*-------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.1.1.1  2000/09/18 16:29:23  hr
    initial import

    Revision 1.4  2000/09/15 15:08:57  willem.vandorp
    OpenOffice header added.

    Revision 1.3  2000/09/01 13:05:41  as
    new targeting, new macros, new dispatching ...

    Revision 1.2  2000/07/31 10:13:38  iha
    typemapping


-------------------------------------------------------------------------*/
