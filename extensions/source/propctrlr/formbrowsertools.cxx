/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "formbrowsertools.hxx"
#include <com/sun/star/form/FormComponentType.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include "strings.hrc"
#include "modulepcr.hxx"
#include "formstrings.hxx"


namespace pcr
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::form;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;


    OUString GetUIHeadlineName(sal_Int16 nClassId, const Any& aUnoObj)
    {
        PcrClient aResourceAccess;
            // this ensures that we have our resource file loaded

        OUString sClassName;
        switch (nClassId)
        {
            case FormComponentType::TEXTFIELD:
            {
                Reference< XInterface >  xIFace;
                aUnoObj >>= xIFace;
                sClassName = PcrRes(RID_STR_PROPTITLE_EDIT);
                if (xIFace.is())
                {   // we have a chance to check if it's a formatted field model
                    Reference< XServiceInfo >  xInfo(xIFace, UNO_QUERY);
                    if (xInfo.is() && (xInfo->supportsService(SERVICE_COMPONENT_FORMATTEDFIELD)))
                        sClassName = PcrRes(RID_STR_PROPTITLE_FORMATTED);
                    else if (!xInfo.is())
                    {
                        // couldn't distinguish between formatted and edit with the service name, so try with the properties
                        Reference< XPropertySet >  xProps(xIFace, UNO_QUERY);
                        if (xProps.is())
                        {
                            Reference< XPropertySetInfo >  xPropsInfo = xProps->getPropertySetInfo();
                            if (xPropsInfo.is() && xPropsInfo->hasPropertyByName(PROPERTY_FORMATSSUPPLIER))
                                sClassName = PcrRes(RID_STR_PROPTITLE_FORMATTED);
                        }
                    }
                }
            }
            break;

            case FormComponentType::COMMANDBUTTON:
                sClassName = PcrRes(RID_STR_PROPTITLE_PUSHBUTTON); break;
            case FormComponentType::RADIOBUTTON:
                sClassName = PcrRes(RID_STR_PROPTITLE_RADIOBUTTON); break;
            case FormComponentType::CHECKBOX:
                sClassName = PcrRes(RID_STR_PROPTITLE_CHECKBOX); break;
            case FormComponentType::LISTBOX:
                sClassName = PcrRes(RID_STR_PROPTITLE_LISTBOX); break;
            case FormComponentType::COMBOBOX:
                sClassName = PcrRes(RID_STR_PROPTITLE_COMBOBOX); break;
            case FormComponentType::GROUPBOX:
                sClassName = PcrRes(RID_STR_PROPTITLE_GROUPBOX); break;
            case FormComponentType::IMAGEBUTTON:
                sClassName = PcrRes(RID_STR_PROPTITLE_IMAGEBUTTON); break;
            case FormComponentType::FIXEDTEXT:
                sClassName = PcrRes(RID_STR_PROPTITLE_FIXEDTEXT); break;
            case FormComponentType::GRIDCONTROL:
                sClassName = PcrRes(RID_STR_PROPTITLE_DBGRID); break;
            case FormComponentType::FILECONTROL:
                sClassName = PcrRes(RID_STR_PROPTITLE_FILECONTROL); break;

            case FormComponentType::DATEFIELD:
                sClassName = PcrRes(RID_STR_PROPTITLE_DATEFIELD); break;
            case FormComponentType::TIMEFIELD:
                sClassName = PcrRes(RID_STR_PROPTITLE_TIMEFIELD); break;
            case FormComponentType::NUMERICFIELD:
                sClassName = PcrRes(RID_STR_PROPTITLE_NUMERICFIELD); break;
            case FormComponentType::CURRENCYFIELD:
                sClassName = PcrRes(RID_STR_PROPTITLE_CURRENCYFIELD); break;
            case FormComponentType::PATTERNFIELD:
                sClassName = PcrRes(RID_STR_PROPTITLE_PATTERNFIELD); break;
            case FormComponentType::IMAGECONTROL:
                sClassName = PcrRes(RID_STR_PROPTITLE_IMAGECONTROL); break;
            case FormComponentType::HIDDENCONTROL:
                sClassName = PcrRes(RID_STR_PROPTITLE_HIDDENCONTROL); break;

            case FormComponentType::CONTROL:
            default:
                sClassName = PcrRes(RID_STR_PROPTITLE_UNKNOWNCONTROL); break;
        }

        return sClassName;
    }


    sal_Int16 classifyComponent( const Reference< XInterface >& _rxComponent )
    {
        Reference< XPropertySet > xComponentProps( _rxComponent, UNO_QUERY_THROW );
        Reference< XPropertySetInfo > xPSI( xComponentProps->getPropertySetInfo(), UNO_SET_THROW );

        sal_Int16 nControlType( FormComponentType::CONTROL );
        if ( xPSI->hasPropertyByName( PROPERTY_CLASSID ) )
        {
            OSL_VERIFY( xComponentProps->getPropertyValue( PROPERTY_CLASSID ) >>= nControlType );
        }
        return nControlType;
    }


} // namespace pcr


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
