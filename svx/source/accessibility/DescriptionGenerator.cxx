/*************************************************************************
 *
 *  $RCSfile: DescriptionGenerator.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: af $ $Date: 2002-03-06 15:59:26 $
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


#include "DescriptionGenerator.hxx"

#ifndef _COM_SUN_STAR_BEANS_PROPERTYSTATE_HPP_
#include <com/sun/star/beans/PropertyState.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSTATE_HPP_
#include <com/sun/star/beans/XPropertyState.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMED_HPP_
#include <com/sun/star/container/XNamed.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_FILLSTYLE_HPP_
#include <com/sun/star/drawing/FillStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPES_HPP_
#include <com/sun/star/drawing/XShapes.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPEDESCRIPTOR_HPP_
#include <com/sun/star/drawing/XShapeDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_INDEXOUTOFBOUNDSEXCEPTION_HPP_
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_XSTYLE_HPP_
#include <com/sun/star/style/XStyle.hpp>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#include <com/sun/star/uno/Exception.hpp>

// Includes for string resources.
#ifndef _SVX_ACCESSIBILITY_HRC
#include "accessibility.hrc"
#endif
#ifndef _SVX_DIALMGR_HXX
#include "dialmgr.hxx"
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#include "xdef.hxx"
#include "unoapi.hxx"
#include "accessibility.hrc"


using namespace ::rtl;
using namespace ::com::sun::star;


void SvxUnogetInternalNameForItem( const sal_Int16 nWhich, const rtl::OUString& rApiName, String& rInternalName ) throw();

namespace accessibility {


DescriptionGenerator::DescriptionGenerator (
    const uno::Reference<drawing::XShape>& xShape)
    : mxShape (xShape),
      mxSet (mxShape, uno::UNO_QUERY),
      mbIsFirstProperty (true)
{
}




DescriptionGenerator::~DescriptionGenerator (void)
{
}




DescriptionGenerator::initialize (::rtl::OUString sPrefix)
{
    msDescription = sPrefix;
    if (mxSet.is())
    {
        msDescription.append (sal_Unicode (' '));
        msDescription.append (OUString (SVX_RESSTR(RID_SVXSTR_A11Y_WITH)));
        msDescription.append (sal_Unicode (' '));

        msDescription.append (OUString (SVX_RESSTR (RID_SVXSTR_A11Y_STYLE)));
        msDescription.append (sal_Unicode ('='));
        try
        {
            if (mxSet.is())
            {
                uno::Any aValue = mxSet->getPropertyValue (OUString::createFromAscii ("Style"));
                uno::Reference<container::XNamed> xStyle (aValue, uno::UNO_QUERY);
                if (xStyle.is())
                    msDescription.append (xStyle->getName());
            }
            else
                msDescription.append (
                    OUString::createFromAscii("<no style>"));
        }
        catch (::com::sun::star::beans::UnknownPropertyException)
        {
            msDescription.append (
                OUString::createFromAscii("<unknown>"));
        }
    }
}




::rtl::OUString DescriptionGenerator::operator() (void)
{
    msDescription.append (sal_Unicode ('.'));
    return msDescription.makeStringAndClear();
}




void DescriptionGenerator::addProperty (const OUString& sPropertyName,
    PropertyType aType, const OUString& sLocalizedName, long nWhichId)
{
    uno::Reference<beans::XPropertyState> xState (mxShape, uno::UNO_QUERY);
    if (xState.is()
        && xState->getPropertyState(sPropertyName)!=beans::PropertyState_DEFAULT_VALUE)
        if (mxSet.is())
        {
            // Append a seperator from previous Properties.
            if ( ! mbIsFirstProperty)
                msDescription.append (sal_Unicode (','));
            else
            {
                msDescription.append (sal_Unicode (' '));
                msDescription.append (OUString (SVX_RESSTR(RID_SVXSTR_A11Y_AND)));
                msDescription.append (sal_Unicode (' '));
                mbIsFirstProperty = false;
            }

            // Delegate to type specific property handling.
            switch (aType)
            {
                case COLOR:
                    addColor (sPropertyName, sLocalizedName);
                    break;
                case INTEGER:
                    addInteger (sPropertyName, sLocalizedName);
                    break;
                case STRING:
                    addString (sPropertyName, sLocalizedName, nWhichId);
                    break;
                case FILL_STYLE:
                    addFillStyle (sPropertyName, sLocalizedName);
                    break;
            }
        }
}




void DescriptionGenerator::appendString (const ::rtl::OUString& sString)
{
    msDescription.append (sString);
}




void DescriptionGenerator::addLineProperties (void)
{
    addProperty (OUString::createFromAscii ("LineColor"),
        DescriptionGenerator::COLOR,
        SVX_RESSTR(RID_SVXSTR_A11Y_LINE_COLOR));
    addProperty (OUString::createFromAscii ("LineDashName"),
        DescriptionGenerator::STRING,
        SVX_RESSTR(RID_SVXSTR_A11Y_LINE_DASH_NAME),
        XATTR_LINEDASH);
    addProperty (OUString::createFromAscii ("LineWidth"),
        DescriptionGenerator::INTEGER,
        SVX_RESSTR(RID_SVXSTR_A11Y_LINE_WIDTH));
}




/** The fill style is described by the property "FillStyle".  Depending on
    its value a hatch-, gradient-, or bitmap name is appended.
*/
void DescriptionGenerator::addFillProperties (void)
{
    addProperty (OUString::createFromAscii ("FillStyle"),
        DescriptionGenerator::FILL_STYLE,
        SVX_RESSTR(RID_SVXSTR_A11Y_FILL_STYLE));
}




void DescriptionGenerator::add3DProperties (void)
{
    addProperty (OUString::createFromAscii ("D3DMaterialColor"),
        DescriptionGenerator::COLOR,
        SVX_RESSTR(RID_SVXSTR_A11Y_3D_MATERIAL_COLOR));
    addLineProperties ();
    addFillProperties ();
}




void DescriptionGenerator::addTextProperties (void)
{
    addProperty (OUString::createFromAscii ("CharColor"),
        DescriptionGenerator::COLOR);
    addFillProperties ();
}




/** Search for the given color in the global color table.  If found append
    its name to the description.  Otherwise append its RGB tuple.
*/
void DescriptionGenerator::addColor (const OUString& sPropertyName,
    const OUString& sLocalizedName)
{
    msDescription.append (sLocalizedName);
    msDescription.append (sal_Unicode('='));

    try
    {

        long nValue;
        if (mxSet.is())
        {
            uno::Any aValue = mxSet->getPropertyValue (sPropertyName);
            aValue >>= nValue;
        }

        // Create color table in which to look up the given color.
        uno::Reference<container::XNameContainer> xColorTable (
            ::comphelper::getProcessServiceFactory()->createInstance(
                OUString::createFromAscii("com.sun.star.drawing.ColorTable")),
            uno::UNO_QUERY);

        uno::Reference<container::XNameAccess> xNA (xColorTable, uno::UNO_QUERY);
        if (xNA.is())
        {
            // Get list of color names in order to iterate over the color table.
            uno::Sequence<OUString> aNames = xNA->getElementNames();
            long i;
            for (i=0; i<aNames.getLength(); i++)
            {
                // Compare the given color with the i-th entry of the table.
                uno::Any aColor (xNA->getByName (aNames[i]));
                long nColor;
                aColor >>= nColor;
                if (nColor == nValue)
                {
                    // Found the given color.  Append its name to the description.
                    msDescription.append (aNames[i]);
                    break;
                }
            }
            if (i == aNames.getLength())
            {
                // Did not find the given color.  Append its rgb tuple to the
                // description.
                msDescription.append (sal_Unicode('#'));
                msDescription.append (nValue, 16);
            }
        }
    }
    catch (::com::sun::star::beans::UnknownPropertyException)
    {
        msDescription.append (
            OUString::createFromAscii("<unknown>"));
    }
}




void DescriptionGenerator::addUnknown (const OUString& sPropertyName,
    const OUString& sLocalizedName)
{
    //        uno::Any aValue = mxSet->getPropertyValue (sPropertyName);
    msDescription.append (sLocalizedName);
}




void DescriptionGenerator::addInteger (const OUString& sPropertyName,
    const OUString& sLocalizedName)
{
    msDescription.append (sLocalizedName);
    msDescription.append (sal_Unicode('='));

    try
    {
        if (mxSet.is())
        {
            uno::Any aValue = mxSet->getPropertyValue (sPropertyName);
            long nValue;
            aValue >>= nValue;
            msDescription.append (nValue);
        }
    }
    catch (::com::sun::star::beans::UnknownPropertyException)
    {
        msDescription.append (
            OUString::createFromAscii("<unknown>"));
    }
}




void DescriptionGenerator::addString (const OUString& sPropertyName,
    const OUString& sLocalizedName, long nWhichId)
{
    msDescription.append (sLocalizedName);
    msDescription.append (sal_Unicode('='));

    try
    {
        if (mxSet.is())
        {
            uno::Any aValue = mxSet->getPropertyValue (sPropertyName);
            OUString sValue;
            aValue >>= sValue;

            if (nWhichId >= 0)
            {
                String sLocalizedValue;
                SvxUnogetInternalNameForItem (nWhichId, sValue, sLocalizedValue);
                msDescription.append (sLocalizedValue);
            }
            else
                msDescription.append (sValue);
        }
    }
    catch (::com::sun::star::beans::UnknownPropertyException)
    {
        msDescription.append (
            OUString::createFromAscii("<unknown>"));
    }
}




void DescriptionGenerator::addFillStyle (const OUString& sPropertyName,
    const OUString& sLocalizedName)
{
    msDescription.append (sLocalizedName);
    msDescription.append (sal_Unicode('='));

    try
    {
        if (mxSet.is())
        {
            uno::Any aValue = mxSet->getPropertyValue (sPropertyName);
            drawing::FillStyle aFillStyle;
            aValue >>= aFillStyle;

            OUString sFillStyleName;
            switch (aFillStyle)
            {
                case drawing::FillStyle_NONE:
                    sFillStyleName = SVX_RESSTR(RID_SVXSTR_A11Y_FILLSTYLE_NONE);
                    msDescription.append (sFillStyleName);
                    break;
                case drawing::FillStyle_SOLID:
                    sFillStyleName = SVX_RESSTR(RID_SVXSTR_A11Y_FILLSTYLE_SOLID);
                    msDescription.append (sFillStyleName);
                    addProperty (OUString::createFromAscii ("FillColor"),
                        COLOR,
                        SVX_RESSTR(RID_SVXSTR_A11Y_FILL_COLOR));
                    break;
                case drawing::FillStyle_GRADIENT:
                    sFillStyleName = SVX_RESSTR(RID_SVXSTR_A11Y_FILLSTYLE_GRADIENT);
                    msDescription.append (sFillStyleName);
                    addProperty (OUString::createFromAscii ("FillGradientName"),
                        STRING,
                        SVX_RESSTR(RID_SVXSTR_A11Y_FILL_GRADIENT_NAME),
                        XATTR_FILLGRADIENT);
                    break;
                case drawing::FillStyle_HATCH:
                    sFillStyleName = SVX_RESSTR(RID_SVXSTR_A11Y_FILLSTYLE_HATCH);
                    msDescription.append (sFillStyleName);
                    addProperty (OUString::createFromAscii ("FillColor"),
                        COLOR,
                        SVX_RESSTR(RID_SVXSTR_A11Y_FILL_COLOR));
                    addProperty (OUString::createFromAscii ("FillHatchName"),
                        STRING,
                        SVX_RESSTR(RID_SVXSTR_A11Y_FILL_HATCH_NAME),
                        XATTR_FILLHATCH);
                    break;
                case drawing::FillStyle_BITMAP:
                    sFillStyleName = SVX_RESSTR(RID_SVXSTR_A11Y_FILLSTYLE_BITMAP);
                    msDescription.append (sFillStyleName);
                    addProperty (OUString::createFromAscii ("FillBitmapName"),
                        STRING,
                        SVX_RESSTR(RID_SVXSTR_A11Y_FILL_BITMAP_NAME),
                        XATTR_FILLBITMAP);
                    break;
            }
        }
    }
    catch (::com::sun::star::beans::UnknownPropertyException)
    {
        msDescription.append (
            OUString::createFromAscii("<unknown>"));
    }
}




void DescriptionGenerator::addPropertyNames (void)
{
    if (mxSet.is())
    {
        uno::Reference<beans::XPropertySetInfo> xInfo (mxSet->getPropertySetInfo());
        if (xInfo.is())
        {
            uno::Sequence<beans::Property> aPropertyList (xInfo->getProperties ());
            for (int i=0; i<aPropertyList.getLength(); i++)
            {
                msDescription.append (aPropertyList[i].Name);
                msDescription.append (sal_Unicode(','));
            }
        }
    }
}


} // end of namespace accessibility
