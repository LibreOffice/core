/*************************************************************************
 *
 *  $RCSfile: optiongrouplayouter.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: fs $ $Date: 2001-03-30 13:28:32 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _EXTENSIONS_DBP_OPTIONGROUPLAYOUTER_HXX_
#include "optiongrouplayouter.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _COM_SUN_STAR_AWT_SIZE_HPP_
#include <com/sun/star/awt/Size.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_POINT_HPP_
#include <com/sun/star/awt/Point.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPES_HPP_
#include <com/sun/star/drawing/XShapes.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPEGROUPER_HPP_
#include <com/sun/star/drawing/XShapeGrouper.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_TEXTCONTENTANCHORTYPE_HPP_
#include <com/sun/star/text/TextContentAnchorType.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XSELECTIONSUPPLIER_HPP_
#include <com/sun/star/view/XSelectionSupplier.hpp>
#endif
#ifndef _EXTENSIONS_DBP_CONTROLWIZARD_HXX
#include "controlwizard.hxx"
#endif
#ifndef _EXTENSIONS_DBP_GROUPBOXWIZ_HXX_
#include "groupboxwiz.hxx"
#endif
#ifndef _EXTENSIONS_DBP_DBPTOOLS_HXX_
#include "dbptools.hxx"
#endif

//.........................................................................
namespace dbp
{
//.........................................................................

#define BUTTON_HEIGHT       300
#define TOP_HEIGHT          300
#define HEIGHT              400
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

    //=====================================================================
    //= OOptionGroupLayouter
    //=====================================================================
    //---------------------------------------------------------------------
    OOptionGroupLayouter::OOptionGroupLayouter(const Reference< XMultiServiceFactory >& _rxORB)
        :m_xORB(_rxORB)
    {
    }

    //---------------------------------------------------------------------
    void OOptionGroupLayouter::doLayout(const OControlWizardContext& _rContext, const OOptionGroupSettings& _rSettings)
    {
        Reference< XShapes > xPageShapes(_rContext.xDrawPage, UNO_QUERY);
        if (!xPageShapes.is())
        {
            DBG_ERROR("OOptionGroupLayouter::OOptionGroupLayouter: missing the XShapes interface for the page!");
            return;
        }

        Reference< XMultiServiceFactory > xDocFactory(_rContext.xDocumentModel, UNO_QUERY);
        if (!xDocFactory.is())
        {
            DBG_ERROR("OOptionGroupLayouter::OOptionGroupLayouter: no document service factory!");
            return;
        }

        // no. of buttons to create
        sal_Int32 nRadioButtons = _rSettings.aLabels.size();

        sal_Int32 nTopSpace = 0;
        sal_Int32 nButtonSpace = 0;

        // the shape of the groupbox
        ::com::sun::star::awt::Size aControlShapeSize = _rContext.xObjectShape->getSize();
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
        Reference< XShapes > xButtonCollection(m_xORB->createInstance(
                ::rtl::OUString::createFromAscii("com.sun.star.drawing.ShapeCollection")),
            UNO_QUERY);
        // first member : the shape of the control
        xButtonCollection->add(_rContext.xObjectShape.get());

        sal_Int32 nTempHeight = (aControlShapeSize.Height - BUTTON_HEIGHT/4) / (nRadioButtons + 1);

        ::com::sun::star::awt::Point aShapePosition = _rContext.xObjectShape->getPosition();

        ::com::sun::star::awt::Size aButtonSize(aControlShapeSize);
        aButtonSize.Width = aControlShapeSize.Width - OFFSET;
        aButtonSize.Height = HEIGHT;
        ::com::sun::star::awt::Point aButtonPosition;
        aButtonPosition.X = aShapePosition.X + OFFSET;

        ::rtl::OUString sElementsName = ::rtl::OUString::createFromAscii("RadioGroup");
        disambiguateName(Reference< XNameAccess >(_rContext.xForm, UNO_QUERY), sElementsName);

        const String* pLabels = _rSettings.aLabels.begin();
        const String* pValues = _rSettings.aValues.begin();
        for (sal_Int32 i=0; i<nRadioButtons; ++i, ++pLabels, ++pValues)
        {
            aButtonPosition.Y = aShapePosition.Y + (i+1) * nTempHeight + nTopSpace;

            Reference< XPropertySet > xRadioModel(
                    xDocFactory->createInstance(::rtl::OUString::createFromAscii("com.sun.star.form.component.RadioButton")),
                UNO_QUERY);

            // the label
            xRadioModel->setPropertyValue(::rtl::OUString::createFromAscii("Label"), makeAny(::rtl::OUString(*pLabels)));
            // the value
            xRadioModel->setPropertyValue(::rtl::OUString::createFromAscii("RefValue"), makeAny(::rtl::OUString(*pValues)));

            // default selection
            if (_rSettings.sDefaultField == *pLabels)
                xRadioModel->setPropertyValue(::rtl::OUString::createFromAscii("DefaultState"), makeAny(sal_Int16(1)));

            // the connection to the database field
            if (0 != _rSettings.sDBField.Len())
                xRadioModel->setPropertyValue(::rtl::OUString::createFromAscii("DataField"), makeAny(::rtl::OUString(_rSettings.sDBField)));

            // the name for the model
            xRadioModel->setPropertyValue(::rtl::OUString::createFromAscii("Name"), makeAny(sElementsName));

            // create a shape for the radio button
            Reference< XControlShape > xRadioShape(
                    xDocFactory->createInstance(::rtl::OUString::createFromAscii("com.sun.star.drawing.ControlShape")),
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
            if (xShapeProperties.is())
                xShapeProperties->setPropertyValue(::rtl::OUString::createFromAscii("Name"), makeAny(sElementsName));

            // add to the page
            xPageShapes->add(xRadioShape.get());
            // add to the collection (for the later grouping)
            xButtonCollection->add(xRadioShape.get());

            // set the GroupBox as "LabelControl" for the RadioButton
            // (_after_ having inserted the model into the page!)
            xRadioModel->setPropertyValue(::rtl::OUString::createFromAscii("LabelControl"), makeAny(_rContext.xObjectModel));
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
            DBG_ERROR("OOptionGroupLayouter::doLayout: caught an exception while grouping the shapes!");
        }
    }

    //---------------------------------------------------------------------
    void OOptionGroupLayouter::implAnchorShape(const Reference< XPropertySet >& _rxShapeProps)
    {
        static const ::rtl::OUString s_sAnchorPropertyName = ::rtl::OUString::createFromAscii("AnchorType");
        Reference< XPropertySetInfo > xPropertyInfo;
        if (_rxShapeProps.is())
            xPropertyInfo = _rxShapeProps->getPropertySetInfo();
        if (xPropertyInfo.is() && xPropertyInfo->hasPropertyByName(s_sAnchorPropertyName))
            _rxShapeProps->setPropertyValue(s_sAnchorPropertyName, makeAny(TextContentAnchorType_AT_PAGE));
    }

//.........................................................................
}   // namespace dbp
//.........................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.3  2001/03/05 14:53:13  fs
 *  finished the grid control wizard
 *
 *  Revision 1.2  2001/02/21 12:11:58  fs
 *  +disambiguateName
 *
 *  Revision 1.1  2001/02/21 09:24:28  fs
 *  initial checkin - form control auto pilots
 *
 *
 *  Revision 1.0 15.02.01 14:24:31  fs
 ************************************************************************/

