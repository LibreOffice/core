/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: formbrowsertools.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 13:17:03 $
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
#include "precompiled_extensions.hxx"

#ifndef _EXTENSIONS_FORMSCTRLR_FORMBROWSERTOOLS_HXX_
#include "formbrowsertools.hxx"
#endif
#ifndef _COM_SUN_STAR_FORM_FORMCOMPONENTTYPE_HPP_
#include <com/sun/star/form/FormComponentType.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _EXTENSIONS_FORMCTRLR_PROPRESID_HRC_
#include "formresid.hrc"
#endif
#ifndef _EXTENSIONS_PROPCTRLR_MODULEPRC_HXX_
#include "modulepcr.hxx"
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _EXTENSIONS_FORMSCTRLR_FORMSTRINGS_HXX_
#include "formstrings.hxx"
#endif

//............................................................................
namespace pcr
{
//............................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::form;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;

    ::rtl::OUString GetUIHeadlineName(sal_Int16 nClassId, const Any& aUnoObj)
    {
        PcrClient aResourceAccess;
            // this ensures that we have our resource file loaded

        ::rtl::OUString sClassName;
        switch (nClassId)
        {
            case FormComponentType::TEXTFIELD:
            {
                Reference< XInterface >  xIFace;
                aUnoObj >>= xIFace;
                sClassName = String(PcrRes(RID_STR_PROPTITLE_EDIT));
                if (xIFace.is())
                {   // we have a chance to check if it's a formatted field model
                    Reference< XServiceInfo >  xInfo(xIFace, UNO_QUERY);
                    if (xInfo.is() && (xInfo->supportsService(SERVICE_COMPONENT_FORMATTEDFIELD)))
                        sClassName = String(PcrRes(RID_STR_PROPTITLE_FORMATTED));
                    else if (!xInfo.is())
                    {
                        // couldn't distinguish between formatted and edit with the service name, so try with the properties
                        Reference< XPropertySet >  xProps(xIFace, UNO_QUERY);
                        if (xProps.is())
                        {
                            Reference< XPropertySetInfo >  xPropsInfo = xProps->getPropertySetInfo();
                            if (xPropsInfo.is() && xPropsInfo->hasPropertyByName(PROPERTY_FORMATSSUPPLIER))
                                sClassName = String(PcrRes(RID_STR_PROPTITLE_FORMATTED));
                        }
                    }
                }
            }
            break;

            case FormComponentType::COMMANDBUTTON:
                sClassName = String(PcrRes(RID_STR_PROPTITLE_PUSHBUTTON)); break;
            case FormComponentType::RADIOBUTTON:
                sClassName = String(PcrRes(RID_STR_PROPTITLE_RADIOBUTTON)); break;
            case FormComponentType::CHECKBOX:
                sClassName = String(PcrRes(RID_STR_PROPTITLE_CHECKBOX)); break;
            case FormComponentType::LISTBOX:
                sClassName = String(PcrRes(RID_STR_PROPTITLE_LISTBOX)); break;
            case FormComponentType::COMBOBOX:
                sClassName = String(PcrRes(RID_STR_PROPTITLE_COMBOBOX)); break;
            case FormComponentType::GROUPBOX:
                sClassName = String(PcrRes(RID_STR_PROPTITLE_GROUPBOX)); break;
            case FormComponentType::IMAGEBUTTON:
                sClassName = String(PcrRes(RID_STR_PROPTITLE_IMAGEBUTTON)); break;
            case FormComponentType::FIXEDTEXT:
                sClassName = String(PcrRes(RID_STR_PROPTITLE_FIXEDTEXT)); break;
            case FormComponentType::GRIDCONTROL:
                sClassName = String(PcrRes(RID_STR_PROPTITLE_DBGRID)); break;
            case FormComponentType::FILECONTROL:
                sClassName = String(PcrRes(RID_STR_PROPTITLE_FILECONTROL)); break;

            case FormComponentType::DATEFIELD:
                sClassName = String(PcrRes(RID_STR_PROPTITLE_DATEFIELD)); break;
            case FormComponentType::TIMEFIELD:
                sClassName = String(PcrRes(RID_STR_PROPTITLE_TIMEFIELD)); break;
            case FormComponentType::NUMERICFIELD:
                sClassName = String(PcrRes(RID_STR_PROPTITLE_NUMERICFIELD)); break;
            case FormComponentType::CURRENCYFIELD:
                sClassName = String(PcrRes(RID_STR_PROPTITLE_CURRENCYFIELD)); break;
            case FormComponentType::PATTERNFIELD:
                sClassName = String(PcrRes(RID_STR_PROPTITLE_PATTERNFIELD)); break;
            case FormComponentType::IMAGECONTROL:
                sClassName = String(PcrRes(RID_STR_PROPTITLE_IMAGECONTROL)); break;
            case FormComponentType::HIDDENCONTROL:
                sClassName = String(PcrRes(RID_STR_PROPTITLE_HIDDENCONTROL)); break;

            case FormComponentType::CONTROL:
            default:
                sClassName = String(PcrRes(RID_STR_PROPTITLE_UNKNOWNCONTROL)); break;
        }

        return sClassName;
    }

//............................................................................
} // namespace pcr
//............................................................................

