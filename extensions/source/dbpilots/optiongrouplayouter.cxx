/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "optiongrouplayouter.hxx"
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/drawing/ShapeCollection.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/drawing/XShapeGrouper.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include "controlwizard.hxx"
#include "groupboxwiz.hxx"
#include "dbptools.hxx"


namespace dbp
{


#define BUTTON_HEIGHT       300
#define HEIGHT              450
#define OFFSET              300
#define MIN_WIDTH           600

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::drawing;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::text;
    using namespace ::com::sun::star::view;

    
    
    
    
    OOptionGroupLayouter::OOptionGroupLayouter(const Reference< XComponentContext >& _rxContext)
        :mxContext(_rxContext)
    {
    }

    
    void OOptionGroupLayouter::doLayout(const OControlWizardContext& _rContext, const OOptionGroupSettings& _rSettings)
    {
        Reference< XShapes > xPageShapes(_rContext.xDrawPage, UNO_QUERY);
        if (!xPageShapes.is())
        {
            OSL_FAIL("OOptionGroupLayouter::OOptionGroupLayouter: missing the XShapes interface for the page!");
            return;
        }

        Reference< XMultiServiceFactory > xDocFactory(_rContext.xDocumentModel, UNO_QUERY);
        if (!xDocFactory.is())
        {
            OSL_FAIL("OOptionGroupLayouter::OOptionGroupLayouter: no document service factory!");
            return;
        }

        
        sal_Int32 nRadioButtons = _rSettings.aLabels.size();

        sal_Int32 nTopSpace = 0;

        
        ::com::sun::star::awt::Size aControlShapeSize = _rContext.xObjectShape->getSize();
        
        sal_Int32 nMinShapeHeight = BUTTON_HEIGHT*(nRadioButtons+1) + BUTTON_HEIGHT + BUTTON_HEIGHT/4;
        if (aControlShapeSize.Height < nMinShapeHeight)
            aControlShapeSize.Height = nMinShapeHeight;
        if (aControlShapeSize.Width < MIN_WIDTH)
            aControlShapeSize.Width = MIN_WIDTH;
        _rContext.xObjectShape->setSize(aControlShapeSize);

        
        implAnchorShape(Reference< XPropertySet >(_rContext.xObjectShape, UNO_QUERY));

        
        Reference< XShapes > xButtonCollection( ShapeCollection::create(mxContext) );
        
        xButtonCollection->add(_rContext.xObjectShape.get());

        sal_Int32 nTempHeight = (aControlShapeSize.Height - BUTTON_HEIGHT/4) / (nRadioButtons + 1);

        ::com::sun::star::awt::Point aShapePosition = _rContext.xObjectShape->getPosition();

        ::com::sun::star::awt::Size aButtonSize(aControlShapeSize);
        aButtonSize.Width = aControlShapeSize.Width - OFFSET;
        aButtonSize.Height = HEIGHT;
        ::com::sun::star::awt::Point aButtonPosition;
        aButtonPosition.X = aShapePosition.X + OFFSET;

        OUString sElementsName("RadioGroup");
        disambiguateName(Reference< XNameAccess >(_rContext.xForm, UNO_QUERY), sElementsName);

        StringArray::const_iterator aLabelIter = _rSettings.aLabels.begin();
        StringArray::const_iterator aValueIter = _rSettings.aValues.begin();
        for (sal_Int32 i=0; i<nRadioButtons; ++i, ++aLabelIter, ++aValueIter)
        {
            aButtonPosition.Y = aShapePosition.Y + (i+1) * nTempHeight + nTopSpace;

            Reference< XPropertySet > xRadioModel(
                    xDocFactory->createInstance("com.sun.star.form.component.RadioButton"),
                UNO_QUERY);

            
            xRadioModel->setPropertyValue("Label", makeAny(OUString(*aLabelIter)));
            
            xRadioModel->setPropertyValue("RefValue", makeAny(OUString(*aValueIter)));

            
            if (_rSettings.sDefaultField == *aLabelIter)
                xRadioModel->setPropertyValue("DefaultState", makeAny(sal_Int16(1)));

            
            if (!_rSettings.sDBField.isEmpty())
                xRadioModel->setPropertyValue("DataField", makeAny(OUString(_rSettings.sDBField)));

            
            xRadioModel->setPropertyValue("Name", makeAny(sElementsName));

            
            Reference< XControlShape > xRadioShape(
                    xDocFactory->createInstance("com.sun.star.drawing.ControlShape"),
                UNO_QUERY);
            Reference< XPropertySet > xShapeProperties(xRadioShape, UNO_QUERY);

            
            implAnchorShape(xShapeProperties);

            
            xRadioShape->setSize(aButtonSize);
            xRadioShape->setPosition(aButtonPosition);
            
            xRadioShape->setControl(Reference< XControlModel >(xRadioModel, UNO_QUERY));

            
            if (xShapeProperties.is())
                xShapeProperties->setPropertyValue("Name", makeAny(sElementsName));

            
            xPageShapes->add(xRadioShape.get());
            
            xButtonCollection->add(xRadioShape.get());

            
            
            xRadioModel->setPropertyValue("LabelControl", makeAny(_rContext.xObjectModel));
        }

        
        try
        {
            Reference< XShapeGrouper > xGrouper(_rContext.xDrawPage, UNO_QUERY);
            if (xGrouper.is())
            {
                Reference< XShapeGroup > xGroupedOptions = xGrouper->group(xButtonCollection);
                Reference< XSelectionSupplier > xSelector(_rContext.xDocumentModel->getCurrentController(), UNO_QUERY);
                if (xSelector.is())
                    xSelector->select(makeAny(xGroupedOptions));
            }
        }
        catch(Exception&)
        {
            OSL_FAIL("OOptionGroupLayouter::doLayout: caught an exception while grouping the shapes!");
        }
    }

    
    void OOptionGroupLayouter::implAnchorShape(const Reference< XPropertySet >& _rxShapeProps)
    {
        static const OUString s_sAnchorPropertyName("AnchorType");
        Reference< XPropertySetInfo > xPropertyInfo;
        if (_rxShapeProps.is())
            xPropertyInfo = _rxShapeProps->getPropertySetInfo();
        if (xPropertyInfo.is() && xPropertyInfo->hasPropertyByName(s_sAnchorPropertyName))
            _rxShapeProps->setPropertyValue(s_sAnchorPropertyName, makeAny(TextContentAnchorType_AT_PAGE));
    }


}   


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
