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

#include "optiongrouplayouter.hxx"
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/drawing/ShapeCollection.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/drawing/XShapeGrouper.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include "groupboxwiz.hxx"
#include "dbptools.hxx"
#include <tools/diagnose_ex.h>


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
        Reference< XShapes > xPageShapes = _rContext.xDrawPage;
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

        // no. of buttons to create
        sal_Int32 nRadioButtons = _rSettings.aLabels.size();

        // the shape of the groupbox
        css::awt::Size aControlShapeSize = _rContext.xObjectShape->getSize();
        // maybe need to adjust the size if the control shapes
        sal_Int32 nMinShapeHeight = BUTTON_HEIGHT*(nRadioButtons+1) + BUTTON_HEIGHT + BUTTON_HEIGHT/4;
        if (aControlShapeSize.Height < nMinShapeHeight)
            aControlShapeSize.Height = nMinShapeHeight;
        if (aControlShapeSize.Width < MIN_WIDTH)
            aControlShapeSize.Width = MIN_WIDTH;
        _rContext.xObjectShape->setSize(aControlShapeSize);

        // if we're working on a writer document, we need to anchor the shape
        implAnchorShape(Reference< XPropertySet >(_rContext.xObjectShape, UNO_QUERY));

        // shape collection (for grouping the shapes)
        Reference< XShapes > xButtonCollection( ShapeCollection::create(mxContext) );
        // first member : the shape of the control
        xButtonCollection->add(_rContext.xObjectShape.get());

        sal_Int32 nTempHeight = (aControlShapeSize.Height - BUTTON_HEIGHT/4) / (nRadioButtons + 1);

        css::awt::Point aShapePosition = _rContext.xObjectShape->getPosition();

        css::awt::Size aButtonSize(aControlShapeSize);
        aButtonSize.Width = aControlShapeSize.Width - OFFSET;
        aButtonSize.Height = HEIGHT;
        css::awt::Point aButtonPosition;
        aButtonPosition.X = aShapePosition.X + OFFSET;

        OUString sElementsName("RadioGroup");
        disambiguateName(Reference< XNameAccess >(_rContext.xForm, UNO_QUERY), sElementsName);

        auto aLabelIter = _rSettings.aLabels.cbegin();
        auto aValueIter = _rSettings.aValues.cbegin();
        for (sal_Int32 i=0; i<nRadioButtons; ++i, ++aLabelIter, ++aValueIter)
        {
            aButtonPosition.Y = aShapePosition.Y + (i+1) * nTempHeight;

            Reference< XPropertySet > xRadioModel(
                    xDocFactory->createInstance("com.sun.star.form.component.RadioButton"),
                UNO_QUERY);

            // the label
            xRadioModel->setPropertyValue("Label", makeAny(*aLabelIter));
            // the value
            xRadioModel->setPropertyValue("RefValue", makeAny(*aValueIter));

            // default selection
            if (_rSettings.sDefaultField == *aLabelIter)
                xRadioModel->setPropertyValue("DefaultState", makeAny(sal_Int16(1)));

            // the connection to the database field
            if (!_rSettings.sDBField.isEmpty())
                xRadioModel->setPropertyValue("DataField", makeAny(_rSettings.sDBField));

            // the name for the model
            xRadioModel->setPropertyValue("Name", makeAny(sElementsName));

            // create a shape for the radio button
            Reference< XControlShape > xRadioShape(
                    xDocFactory->createInstance("com.sun.star.drawing.ControlShape"),
                UNO_QUERY);
            Reference< XPropertySet > xShapeProperties(xRadioShape, UNO_QUERY);

            // if we're working on a writer document, we need to anchor the shape
            implAnchorShape(xShapeProperties);

            // position it
            xRadioShape->setSize(aButtonSize);
            xRadioShape->setPosition(aButtonPosition);
            // knitting with the model
            xRadioShape->setControl(Reference< XControlModel >(xRadioModel, UNO_QUERY));

            // the name of the shape
            // tdf#117282 com.sun.star.drawing.ControlShape *has* no property
            // of type 'Name'. In older versions it was an error that this did
            // not throw an UnknownPropertyException. Still, it was never set
            // at the Shape/SdrObject and was lost.
            // Thus - just do no tset it. It is/stays part of the FormControl
            // data, so it will be shown in the FormControl dialogs. It is not
            // shown/used in SdrObject::Name dialog (e.g. context menu/Name...)
            // if (xShapeProperties.is())
            //     xShapeProperties->setPropertyValue("Name", makeAny(sElementsName));

            // add to the page
            xPageShapes->add(xRadioShape.get());
            // add to the collection (for the later grouping)
            xButtonCollection->add(xRadioShape.get());

            // set the GroupBox as "LabelControl" for the RadioButton
            // (_after_ having inserted the model into the page!)
            xRadioModel->setPropertyValue("LabelControl", makeAny(_rContext.xObjectModel));
        }

        // group the shapes
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
            TOOLS_WARN_EXCEPTION("extensions.dbpilots",
                                 "caught an exception while grouping the shapes!");
        }
    }


    void OOptionGroupLayouter::implAnchorShape(const Reference< XPropertySet >& _rxShapeProps)
    {
        static const char s_sAnchorPropertyName[] = "AnchorType";
        Reference< XPropertySetInfo > xPropertyInfo;
        if (_rxShapeProps.is())
            xPropertyInfo = _rxShapeProps->getPropertySetInfo();
        if (xPropertyInfo.is() && xPropertyInfo->hasPropertyByName(s_sAnchorPropertyName))
            _rxShapeProps->setPropertyValue(s_sAnchorPropertyName, makeAny(TextContentAnchorType_AT_PAGE));
    }


}   // namespace dbp


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
