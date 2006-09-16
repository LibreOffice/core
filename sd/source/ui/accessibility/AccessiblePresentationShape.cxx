/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AccessiblePresentationShape.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 18:25:50 $
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
#include "precompiled_sd.hxx"

#ifndef _SD_ACCESSIBILITY_ACCESSIBLE_PRESENTATION_SHAPE_HXX
#include "AccessiblePresentationShape.hxx"
#endif

#include "SdShapeTypes.hxx"

#include <svx/DescriptionGenerator.hxx>
#ifndef _RTL_USTRING_H_
#include <rtl/ustring.h>
#endif

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

namespace accessibility {

//=====  internal  ============================================================

AccessiblePresentationShape::AccessiblePresentationShape (
    const AccessibleShapeInfo& rShapeInfo,
    const AccessibleShapeTreeInfo& rShapeTreeInfo)
    : AccessibleShape (rShapeInfo, rShapeTreeInfo)
{
}




AccessiblePresentationShape::~AccessiblePresentationShape (void)
{
}




//=====  XServiceInfo  ========================================================

::rtl::OUString SAL_CALL
    AccessiblePresentationShape::getImplementationName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AccessiblePresentationShape"));
}




/// Set this object's name if is different to the current name.
::rtl::OUString
    AccessiblePresentationShape::CreateAccessibleBaseName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    ::rtl::OUString sName;

    ShapeTypeId nShapeType = ShapeTypeHandler::Instance().GetTypeId (mxShape);
    switch (nShapeType)
    {
        case PRESENTATION_TITLE:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("ImpressTitle"));
            break;
        case PRESENTATION_OUTLINER:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("ImpressOutliner"));
            break;
        case PRESENTATION_SUBTITLE:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("ImpressSubtitle"));
            break;
        case PRESENTATION_PAGE:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("ImpressPage"));
            break;
        case PRESENTATION_NOTES:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("ImpressNotes"));
            break;
        case PRESENTATION_HANDOUT:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("ImpressHandout"));
            break;
        case PRESENTATION_HEADER:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("ImpressHeader"));
            break;
        case PRESENTATION_FOOTER:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("ImpressFooter"));
            break;
        case PRESENTATION_DATETIME:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("ImpressDateAndTime"));
            break;
        case PRESENTATION_PAGENUMBER:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("ImpressPageNumber"));
            break;
        default:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("UnknownAccessibleImpressShape"));
            uno::Reference<drawing::XShapeDescriptor> xDescriptor (mxShape, uno::UNO_QUERY);
            if (xDescriptor.is())
                sName += ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM(": "))
                    + xDescriptor->getShapeType();
    }

    return sName;
}




::rtl::OUString
    AccessiblePresentationShape::CreateAccessibleDescription (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    //    return createAccessibleName ();
    DescriptionGenerator aDG (mxShape);
    ShapeTypeId nShapeType = ShapeTypeHandler::Instance().GetTypeId (mxShape);
    switch (nShapeType)
    {
        case PRESENTATION_TITLE:
            aDG.Initialize (::rtl::OUString::createFromAscii ("PresentationTitleShape"));
            break;
        case PRESENTATION_OUTLINER:
            aDG.Initialize (::rtl::OUString::createFromAscii ("PresentationOutlinerShape"));
            break;
        case PRESENTATION_SUBTITLE:
            aDG.Initialize (::rtl::OUString::createFromAscii ("PresentationSubtitleShape"));
            break;
        case PRESENTATION_PAGE:
            aDG.Initialize (::rtl::OUString::createFromAscii ("PresentationPageShape"));
            break;
        case PRESENTATION_NOTES:
            aDG.Initialize (::rtl::OUString::createFromAscii ("PresentationNotesShape"));
            break;
        case PRESENTATION_HANDOUT:
            aDG.Initialize (::rtl::OUString::createFromAscii ("PresentationHandoutShape"));
            break;
        case PRESENTATION_HEADER:
            aDG.Initialize (::rtl::OUString::createFromAscii ("PresentationHeaderShape"));
            break;
        case PRESENTATION_FOOTER:
            aDG.Initialize (::rtl::OUString::createFromAscii ("PresentationFooterShape"));
            break;
        case PRESENTATION_DATETIME:
            aDG.Initialize (::rtl::OUString::createFromAscii ("PresentationDateAndTimeShape"));
            break;
        case PRESENTATION_PAGENUMBER:
            aDG.Initialize (::rtl::OUString::createFromAscii ("PresentationPageNumberShape"));
            break;
        default:
            aDG.Initialize (::rtl::OUString::createFromAscii ("Unknown accessible presentation shape"));
            uno::Reference<drawing::XShapeDescriptor> xDescriptor (mxShape, uno::UNO_QUERY);
            if (xDescriptor.is())
            {
                aDG.AppendString (::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("service name=")));
                aDG.AppendString (xDescriptor->getShapeType());
            }
    }

    return aDG();
}

} // end of namespace accessibility
