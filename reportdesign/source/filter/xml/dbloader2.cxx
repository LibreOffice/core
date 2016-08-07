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


#include "precompiled_rptxml.hxx"
#include "dbloader2.hxx"
#include <comphelper/sequence.hxx>
// -------------------------------------------------------------------------
namespace rptxml
{

using namespace ::ucbhelper;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::embed;
using namespace ::com::sun::star::ui::dialogs;
using ::com::sun::star::awt::XWindow;

// -------------------------------------------------------------------------
ORptTypeDetection::ORptTypeDetection(Reference< XComponentContext > const & xContext)
: m_xContext(xContext)
{
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ORptTypeDetection::detect( Sequence< ::com::sun::star::beans::PropertyValue >& Descriptor ) throw (RuntimeException)
{

    ::comphelper::SequenceAsHashMap aTemp(Descriptor);
    ::rtl::OUString sTemp = aTemp.getUnpackedValueOrDefault(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("URL")),::rtl::OUString());

    if ( sTemp.getLength() )
    {
        INetURLObject aURL(sTemp);
        if ( aURL.GetExtension().equalsIgnoreAsciiCaseAscii("orp") )
            return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("StarBaseReport"));
        else
        {
            try
            {
                Reference<XPropertySet> xProp(::comphelper::OStorageHelper::GetStorageFromURL(sTemp,ElementModes::READ,Reference< XMultiServiceFactory >(m_xContext->getServiceManager(),UNO_QUERY)),UNO_QUERY);
                if ( xProp.is() )
                {
                    ::rtl::OUString sMediaType;
                    xProp->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MediaType")) ) >>= sMediaType;
                    if ( sMediaType.equalsAscii(MIMETYPE_OASIS_OPENDOCUMENT_REPORT_ASCII) )
                        return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("StarBaseReport"));
                    ::comphelper::disposeComponent(xProp);
                } // if ( xProp.is() )
            }
            catch(Exception&)
            {
            }
        }
    }
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------
Reference< XInterface > SAL_CALL
        ORptTypeDetection::create(Reference< XComponentContext > const & xContext)
{
    return *(new ORptTypeDetection(xContext));
}
// -------------------------------------------------------------------------
// XServiceInfo
::rtl::OUString SAL_CALL ORptTypeDetection::getImplementationName() throw(  )
{
    return getImplementationName_Static();
}
// -------------------------------------------------------------------------

// XServiceInfo
sal_Bool SAL_CALL ORptTypeDetection::supportsService(const ::rtl::OUString& ServiceName) throw(  )
{
    return ::comphelper::existsValue(ServiceName,getSupportedServiceNames_Static());
}
// -------------------------------------------------------------------------
// XServiceInfo
Sequence< ::rtl::OUString > SAL_CALL ORptTypeDetection::getSupportedServiceNames(void) throw(  )
{
    return getSupportedServiceNames_Static();
}
// -------------------------------------------------------------------------
// ORegistryServiceManager_Static
Sequence< ::rtl::OUString > ORptTypeDetection::getSupportedServiceNames_Static(void) throw( RuntimeException )
{
    Sequence< ::rtl::OUString > aSNS( 1 );
    aSNS.getArray()[0] = ::rtl::OUString::createFromAscii("com.sun.star.document.ExtendedTypeDetection");
    return aSNS;
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
}//rptxml
// -----------------------------------------------------------------------------

