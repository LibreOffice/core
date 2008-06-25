/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: xmlImage.cxx,v $
 * $Revision: 1.7 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#include "precompiled_reportdesign.hxx"
#include "xmlImage.hxx"
#include "xmlfilter.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include "xmlEnums.hxx"
#include "xmlComponent.hxx"
#include "xmlReportElement.hxx"
#include "xmlControlProperty.hxx"
#include <tools/debug.hxx>

#include <comphelper/componentcontext.hxx>
#include <com/sun/star/util/XStringSubstitution.hpp>

namespace rptxml
{
    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::xml::sax;
DBG_NAME( rpt_OXMLImage )


//--------------------------------------------------------------------
::rtl::OUString OXMLImage::lcl_doStringsubstitution_nothrow( ::rtl::OUString const& _inout_rURL )
{
    try
    {
        Reference< XInterface > xInt = m_rImport.getORB()->createInstance( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.util.PathSubstitution" ) ) );
        Reference< util::XStringSubstitution > xStringSubst(xInt, UNO_QUERY);

        return xStringSubst->substituteVariables( _inout_rURL, true );
    }
    catch( const Exception& )
    {
        // DBG_UNHANDLED_EXCEPTION();
    }
    return ::rtl::OUString();
}
// -----------------------------------------------------------------------------

OXMLImage::OXMLImage( ORptFilter& rImport,
                sal_uInt16 nPrfx, const ::rtl::OUString& rLName,
                const Reference< XAttributeList > & _xAttrList
                ,const Reference< XImageControl > & _xComponent
                ,OXMLTable* _pContainer) :
    OXMLReportElementBase( rImport, nPrfx, rLName,_xComponent.get(),_pContainer)
{
    DBG_CTOR( rpt_OXMLImage,NULL);

    OSL_ENSURE(m_xComponent.is(),"Component is NULL!");
    const SvXMLNamespaceMap& rMap = m_rImport.GetNamespaceMap();
    const SvXMLTokenMap& rTokenMap = m_rImport.GetControlElemTokenMap();
    static const ::rtl::OUString s_sTRUE = ::xmloff::token::GetXMLToken(XML_TRUE);

    const sal_Int16 nLength = (_xAttrList.is()) ? _xAttrList->getLength() : 0;
    try
    {
        for(sal_Int16 i = 0; i < nLength; ++i)
        {
         ::rtl::OUString sLocalName;
            const rtl::OUString sAttrName = _xAttrList->getNameByIndex( i );
            const sal_uInt16 nPrefix = rMap.GetKeyByAttrName( sAttrName,&sLocalName );
            /* const */ rtl::OUString sValue = _xAttrList->getValueByIndex( i );

            switch( rTokenMap.Get( nPrefix, sLocalName ) )
            {
                case XML_TOK_IMAGE_DATA:
                    {
                        // rtl::OUString sTest = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("$(inst)"));
                        // sTest = lcl_doStringsubstitution_nothrow( sTest );
                        //
                        // sTest = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("$(prog)"));
                        // sTest = lcl_doStringsubstitution_nothrow( sTest );
                        //
                        // sTest = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("$(user)"));
                        // sTest = lcl_doStringsubstitution_nothrow( sTest );
                        //
                        // sTest = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("$(work)"));
                        // sTest = lcl_doStringsubstitution_nothrow( sTest );
                        //
                        // sTest = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("$(home)"));
                        // sTest = lcl_doStringsubstitution_nothrow( sTest );
                        //
                        // sTest = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("$(temp)"));
                        // sTest = lcl_doStringsubstitution_nothrow( sTest );
                        //
                        // sTest = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("$(path)"));
                        // sTest = lcl_doStringsubstitution_nothrow( sTest );
                        //
                        // sTest = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("$(lang)"));
                        // sTest = lcl_doStringsubstitution_nothrow( sTest );
                        //
                        // sTest = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("$(langid)"));
                        // sTest = lcl_doStringsubstitution_nothrow( sTest );
                        //
                        // sTest = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("$(vlang)"));
                        // sTest = lcl_doStringsubstitution_nothrow( sTest );

                        sValue = lcl_doStringsubstitution_nothrow( sValue );
                        _xComponent->setImageURL(rImport.GetAbsoluteReference( sValue ));
                    }

                    break;
                case XML_TOK_PRESERVE_IRI:
                    _xComponent->setPreserveIRI(s_sTRUE == sValue);
                    break;
                case XML_TOK_SCALE:
                    _xComponent->setScaleImage(s_sTRUE == sValue);
                    break;
                case XML_TOK_DATA_FORMULA:
                    _xComponent->setDataField(ORptFilter::convertFormula(sValue));
                    break;
                default:
                    break;
            }
        }
    }
    catch(Exception&)
    {
        OSL_ENSURE(0,"Exception catched while filling the image props");
    }
}
// -----------------------------------------------------------------------------

OXMLImage::~OXMLImage()
{

    DBG_DTOR( rpt_OXMLImage,NULL);
}
// -----------------------------------------------------------------------------

//----------------------------------------------------------------------------
} // namespace rptxml
// -----------------------------------------------------------------------------
