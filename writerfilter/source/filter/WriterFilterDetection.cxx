/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WriterFilterDetection.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: os $ $Date: 2007-02-22 13:44:11 $
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

#ifndef _CPPUHELPER_IMPLEMENTATIONENTRY_
#include <cppuhelper/implementationentry.hxx>
#endif

#ifndef _WriterFilterDetection_DETECTION_HXX
#include <WriterFilterDetection.hxx>
#endif
#ifndef _COMPHELPER_STORAGEHELPER_HXX
#include <comphelper/storagehelper.hxx>
#endif
#ifndef _COM_SUN_STAR_EMBED_ELEMENTMODES_HPP_
#include <com/sun/star/embed/ElementModes.HPP>
#endif
using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star;

/*-- 22.02.2007 12:17:53---------------------------------------------------

  -----------------------------------------------------------------------*/
WriterFilterDetection::WriterFilterDetection(
    const uno::Reference< uno::XComponentContext >& rxContext) :
    m_xContext( rxContext )
{
}
/*-- 22.02.2007 12:17:53---------------------------------------------------

  -----------------------------------------------------------------------*/
WriterFilterDetection::~WriterFilterDetection()
{
}
/*-- 22.02.2007 12:11:38---------------------------------------------------

  -----------------------------------------------------------------------*/
OUString WriterFilterDetection_getImplementationName () throw (uno::RuntimeException)
{
   return OUString ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.comp.Writer.WriterFilterDetector" ) );
}

#define SERVICE_NAME1 "com.sun.star.document.ExtendedTypeDetection"
/*-- 22.02.2007 12:11:38---------------------------------------------------

  -----------------------------------------------------------------------*/
OUString WriterFilterDetection::detect( uno::Sequence< beans::PropertyValue >& rDescriptor )
   throw( uno::RuntimeException )
{
    OUString sTypeName;
    bool bContent_Types = false;
    bool bWord = false;
    sal_Int32 nPropertyCount = rDescriptor.getLength();
    const beans::PropertyValue* pValues = rDescriptor.getConstArray();
    rtl::OUString sURL;
    for( sal_Int32 nProperty = 0; nProperty < nPropertyCount; ++nProperty )
    {
        if( pValues[nProperty].Name == OUString(RTL_CONSTASCII_USTRINGPARAM("TypeName")) )
        {
            rDescriptor[nProperty].Value >>= sTypeName;
        }
        else if( pValues[nProperty].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM ( "URL" )) )
        {
            pValues[nProperty].Value >>= sURL;
        }
    }
    try
    {
        uno::Reference< embed::XStorage > xDocStorage = comphelper::OStorageHelper::GetStorageFromURL(
                                        sURL, embed::ElementModes::READ );
        if( xDocStorage.is() )
        {
            uno::Sequence< ::rtl::OUString > aNames = xDocStorage->getElementNames();
            const ::rtl::OUString* pNames = aNames.getConstArray();
            for(sal_Int32 nName = 0; nName < aNames.getLength(); ++nName)
            {
                if( pNames[nName].equalsAsciiL(RTL_CONSTASCII_STRINGPARAM ( "[Content_Types].xml" )))
                    bContent_Types = true;
                else if(pNames[nName].equalsAsciiL(RTL_CONSTASCII_STRINGPARAM ( "word" )))
                    bWord = true;
            }
        }
    }
    catch(const uno::Exception&)
    {
        OSL_ASSERT("exception while opening storage");
    }
    if( !bContent_Types || !bWord )
        sTypeName = ::rtl::OUString();
   return sTypeName;
}
/*-- 22.02.2007 12:11:38---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool WriterFilterDetection_supportsService( const OUString& ServiceName ) throw (uno::RuntimeException)
{
   return (ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( SERVICE_NAME1 ) ) );
}
/*-- 22.02.2007 12:11:38---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Sequence< OUString > WriterFilterDetection_getSupportedServiceNames(  ) throw (uno::RuntimeException)
{
   uno::Sequence < OUString > aRet(1);
   OUString* pArray = aRet.getArray();
   pArray[0] =  OUString ( RTL_CONSTASCII_USTRINGPARAM ( SERVICE_NAME1 ) );
   return aRet;
}
#undef SERVICE_NAME1
/*-- 22.02.2007 12:11:38---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< uno::XInterface > WriterFilterDetection_createInstance( const uno::Reference< uno::XComponentContext >& xContext)
                throw( uno::Exception )
{
   return (cppu::OWeakObject*) new WriterFilterDetection( xContext );
}
/*-- 22.02.2007 12:11:38---------------------------------------------------

  -----------------------------------------------------------------------*/
OUString WriterFilterDetection::getImplementationName(  ) throw (uno::RuntimeException)
{
   return WriterFilterDetection_getImplementationName();
}
/*-- 22.02.2007 12:11:38---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool WriterFilterDetection::supportsService( const OUString& rServiceName ) throw (uno::RuntimeException)
{
    return WriterFilterDetection_supportsService( rServiceName );
}
/*-- 22.02.2007 12:11:38---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Sequence< OUString > WriterFilterDetection::getSupportedServiceNames(  ) throw (uno::RuntimeException)
{
    return WriterFilterDetection_getSupportedServiceNames();
}

