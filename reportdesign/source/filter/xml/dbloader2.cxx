/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dbloader2.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-09 11:56:16 $
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
#include "precompiled_reportdesign.hxx"

#ifndef RPTXML_DBLOADER2_HXX
#include "dbloader2.hxx"
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
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

