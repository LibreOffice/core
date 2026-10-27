/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "../helper/qahelper.hxx"
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <comphelper/diagnose_ex.hxx>

using namespace css::beans;
using namespace css::container;
using namespace css::drawing;
using namespace css::lang;
using namespace css::sheet;
using namespace css::text;
using namespace ::cpo::uno;

/*
 * This testcase checks the calcrtl feature added to cws_calcrtl in three steps
 * <br>
 * (<b>REMARK: </b> the testcases expects the TableLayout to be LR_TB)
 * <br>
 * 1. A calc document is opened and the properties in css.sheet.Spreadsheet are checked
 * <br>
 * 2. A calc document is opened and a shape inserted, afterwards the properties in css.sheet.Shape are checked
 * <br>
 * 3. A calc document is opened and a shape inserted, afterwards the effect of changing the TableLayout to the shape is checked
 */

class ScComplexTest : public ScModelTestBase
{
public:
    ScComplexTest();

    Reference<XShape> getInsertedShape();
    Reference<XSpreadsheet> getSpreadsheet();
    bool changeProperty(const Reference<XPropertySet>& set, const OUString& pName,
                        const Any& pValue);
    std::array<Any, 4> getShapeProps(const Reference<XPropertySet>& set,
                                     const Reference<XShape>& oShape);
    bool checkResult(const Reference<XPropertySet>& set, const OUString& propName, Any oldValue,
                     Any newValue, Any& resValue, const Any& exception);
    Any getRealValue(const Any& obj);
    Reference<XShape> createShape(const Reference<XComponent>& oDoc, int height, int width, int x,
                                  int y, std::u16string_view kind);
};

ScComplexTest::ScComplexTest()
    : ScModelTestBase(u"sc/qa/unit/uicalc/data"_ustr)
{
}

/*
 * In this method a spreadsheet document is opened and a shape inserted<br>
 * afterwards all calc specific properties of the Shape are checked.<br>
 * <p>
 * These are
 * <ul>
 *<li> Anchor
 *<li> HoriOrientPosition
 *<li> VertOrientPosition
 *</ul>
 */
CPPUNIT_TEST_FIXTURE(ScComplexTest, checkShapeProperties)
{
    createScDoc();

    Reference<XPropertySet> set(getInsertedShape(), UNO_QUERY);

    try
    {
        Any cellValue(getSpreadsheet()->getCellByPosition(5, 5));
        CPPUNIT_ASSERT_MESSAGE("Problems when setting property 'Anchor'",
                               changeProperty(set, u"Anchor"_ustr, cellValue));
    }
    catch (IndexOutOfBoundsException)
    {
        // assure("Problems when setting property 'Anchor'", false);
        CPPUNIT_FAIL("Problems when setting property 'Anchor'");
    }

    CPPUNIT_ASSERT_MESSAGE("Problems when setting property 'HoriOrientPosition'",
                           changeProperty(set, u"HoriOrientPosition"_ustr, Any(sal_Int32(1000))));

    CPPUNIT_ASSERT_MESSAGE("Problems when setting property 'VertOrientPosition'",
                           changeProperty(set, u"VertOrientPosition"_ustr, Any(sal_Int32(1000))));
}

CPPUNIT_TEST_FIXTURE(ScComplexTest, checkInfluenceOfSpreadsheetChange)
{
    createScDoc();

    Reference<XShape> oShape = getInsertedShape();
    Reference<XPropertySet> sheetProps(getSpreadsheet(), UNO_QUERY);
    Reference<XPropertySet> shapeProps(oShape, UNO_QUERY);

    std::array<Any, 4> previous = getShapeProps(shapeProps, oShape);
    CPPUNIT_ASSERT_MESSAGE(
        "Problems when setting property 'TableLayout'",
        changeProperty(sheetProps, u"TableLayout"_ustr, Any(sal_Int16(WritingMode2::RL_TB))));

    std::array<Any, 4> RL_TB = getShapeProps(shapeProps, oShape);
    CPPUNIT_ASSERT_MESSAGE(
        "Problems when setting property 'TableLayout'",
        changeProperty(sheetProps, u"TableLayout"_ustr, Any(sal_Int16(WritingMode2::LR_TB))));

    std::array<Any, 4> LR_TB = getShapeProps(shapeProps, oShape);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Anchor has changed", previous[0], RL_TB[0]);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Anchor has changed", previous[0], LR_TB[0]);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("HoriOrientPosition has changed", sal_Int32(2100),
                                 previous[1].get<sal_Int32>() + RL_TB[1].get<sal_Int32>()
                                     + LR_TB[1].get<sal_Int32>());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("VertOrientPosition has changed", 3 * previous[2].get<sal_Int32>(),
                                 previous[2].get<sal_Int32>() + RL_TB[2].get<sal_Int32>()
                                     + LR_TB[2].get<sal_Int32>());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("x-position hasn't changed", previous[3], LR_TB[3]);
    CPPUNIT_ASSERT_MESSAGE("x-position hasn't changed",
                           (previous[3].get<sal_Int32>() * (-1)) - oShape->getSize().Width
                               != LR_TB[2].get<sal_Int32>());
}

std::array<Any, 4> ScComplexTest::getShapeProps(const Reference<XPropertySet>& set,
                                                const Reference<XShape>& oShape)
{
    std::array<Any, 4> reValue;

    try
    {
        reValue[0] = getRealValue(set->getPropertyValue(u"Anchor"_ustr));
        reValue[1] = set->getPropertyValue(u"HoriOrientPosition"_ustr);
        reValue[2] = set->getPropertyValue(u"VertOrientPosition"_ustr);
        reValue[3] <<= oShape->getPosition().X;
    }
    catch (UnknownPropertyException)
    {
    }
    catch (WrappedTargetException)
    {
    }

    return reValue;
}

/*
 * This method gets the first Sheet of the SpreadsheetDocument
 *
 */
Reference<XSpreadsheet> ScComplexTest::getSpreadsheet()
{
    Reference<XSpreadsheetDocument> xSheetDoc(mxComponent, UNO_QUERY_THROW);
    Reference<XSpreadsheets> xSpreadsheets = xSheetDoc->getSheets();
    Reference<XIndexAccess> oIndexAccess(xSpreadsheets, UNO_QUERY);

    Reference<XSpreadsheet> oSheet;
    try
    {
        oSheet.set(oIndexAccess->getByIndex(0), UNO_QUERY);
    }
    catch (WrappedTargetException)
    {
        TOOLS_WARN_EXCEPTION("sc", "");
    }
    catch (IndexOutOfBoundsException)
    {
        TOOLS_WARN_EXCEPTION("sc", "");
    }

    return oSheet;
}

/*
 * This method changes a given property to a given value
 * Parameters:
 * @param set : The XPropertySet that contains the properties
 * @param pName : The name of the Property that has to be changed
 * @param pValue : The value, the property should be changed too.
 */
bool ScComplexTest::changeProperty(const Reference<XPropertySet>& set, const OUString& pName,
                                   const Any& pValue)
{
    bool worked = true;

    try
    {
        Any oldValue = set->getPropertyValue(pName);

        // for an exception thrown during setting new value
        // to pass it to checkResult method
        Any exception;

        try
        {
            set->setPropertyValue(pName, pValue);
        }
        catch (IllegalArgumentException)
        {
            exception = ::cppu::getCaughtException();
        }
        catch (PropertyVetoException)
        {
            exception = ::cppu::getCaughtException();
        }
        catch (WrappedTargetException)
        {
            exception = ::cppu::getCaughtException();
        }
        catch (UnknownPropertyException)
        {
            exception = ::cppu::getCaughtException();
        }
        catch (RuntimeException)
        {
            exception = ::cppu::getCaughtException();
        }

        // getting result value
        Any resValue = set->getPropertyValue(pName);

        // checking results
        worked = checkResult(set, pName, oldValue, pValue, resValue, exception);
    }
    catch (Exception)
    {
        TOOLS_WARN_EXCEPTION("sc", "Exception occurred while testing property");
        worked = false;
    }

    return worked;
}

/*
 * This methods checks if a property has changed as expected
 * Parameters:
 * @param set : the given XPropertySet
 * @param propName : the name of the property that has been changed
 * @param oldValue : the value of the property before it has been changed
 * @param newValue : the value the property has been set to
 * @param resValue : the value getPropertyValue returned for the property
 * @param exception : the exception thrown during the change of the property
 */
bool ScComplexTest::checkResult(const Reference<XPropertySet>& set, const OUString& propName,
                                Any oldValue, Any newValue, Any& resValue, const Any& exception)
{
    Reference<XPropertySetInfo> info = set->getPropertySetInfo();
    Property prop = info->getPropertyByName(propName);

    oldValue = getRealValue(oldValue);
    newValue = getRealValue(newValue);
    resValue = getRealValue(resValue);

    bool readOnly = (prop.Attributes & PropertyAttribute::READONLY) != 0;
    bool maybeVoid = (prop.Attributes & PropertyAttribute::MAYBEVOID) != 0;

    //check get-set methods

    if (!oldValue.hasValue() && !maybeVoid)
    {
        // is void, but it's not MAYBEVOID
        return false;
    }
    else if (!oldValue.hasValue())
    {
        // has null value, and therefore can't be changed
        return true;
    }
    else if (readOnly)
    {
        // check if exception was thrown
        if (exception.hasValue())
        {
            if (exception.has<PropertyVetoException>())
            {
                // the change of read only prohibited - OK
                // Property is ReadOnly and wasn't changed
                return true;
            }
            else if (exception.has<IllegalArgumentException>())
            {
                // the change of read only prohibited - OK
                return true;
            }
            else if (exception.has<UnknownPropertyException>())
            {
                // the change of read only prohibited - OK
                // Property is ReadOnly and wasn't changed");
                return true;
            }
            else if (exception.has<RuntimeException>())
            {
                // the change of read only prohibited - OK
                // Property is ReadOnly and wasn't changed
                return true;
            }
            else
            {
                throw exception;
            }
        }
        else
        {
            // if no exception - check that value
            // has not changed
            if (resValue != oldValue)
            {
                //Read only property has changed
                return false;
            }
            else
            {
                // Read only property hasn't changed
                return true;
            }
        }
    }
    else
    {
        if (!exception.hasValue())
        {
            // if no exception thrown
            // check that the new value is set
            if (resValue != newValue)
            {
                // Value hasn't changed as expected
                if (resValue != oldValue)
                {
                    // But it has changed.
                    return true;
                }
                else
                {
                    return false;
                }
            }
            else
            {
                // Property OK
                return true;
            }
        }
        else
        {
            throw exception;
        }
    }
}

/*
 * Returns a human readable String representation for CellRangeAddressable
 */
Any ScComplexTest::getRealValue(const Any& obj)
{
    Reference<XCellRangeAddressable> xCell(obj, UNO_QUERY);
    if (!xCell)
        return obj;
    return Any("Cell in Column " + OUString::number(xCell->getRangeAddress().StartColumn)
               + " and Row " + OUString::number(xCell->getRangeAddress().StartRow));
}

/*
 * This method inserts a RectangleShape into the calc document xSheetDoc and returns the resulting XShape-object
 */
Reference<XShape> ScComplexTest::getInsertedShape()
{
    Reference<XShape> insertedShape;

    try
    {
        Reference<XSpreadsheetDocument> xSheetDoc(mxComponent, UNO_QUERY_THROW);
        Reference<XDrawPagesSupplier> oDPS(xSheetDoc, UNO_QUERY);
        Reference<XDrawPages> oDP = oDPS->getDrawPages();
        Reference<XDrawPage> firstDrawPage(oDP->getByIndex(0), UNO_QUERY);

        Reference<XComponent> xComp(xSheetDoc, UNO_QUERY);
        insertedShape = createShape(xComp, 5000, 3500, 700, 500, u"Rectangle"_ustr);
        firstDrawPage->add(insertedShape);
    }
    catch (WrappedTargetException)
    {
        TOOLS_WARN_EXCEPTION("sc", "Couldn't create instance");
        return nullptr;
    }
    catch (IndexOutOfBoundsException)
    {
        TOOLS_WARN_EXCEPTION("sc", "Couldn't create instance");
        return nullptr;
    }

    return insertedShape;
}

Reference<XShape> ScComplexTest::createShape(const Reference<XComponent>& oDoc, int height,
                                             int width, int x, int y, std::u16string_view kind)
{
    //possible values for kind are 'Ellipse', 'Line' and 'Rectangle'

    Reference<XMultiServiceFactory> docMSF(oDoc, UNO_QUERY);
    Reference<XShape> xShape(
        docMSF->createInstance(OUString::Concat("com.sun.star.drawing.") + kind + "Shape"),
        UNO_QUERY);
    css::awt::Size size;
    css::awt::Point position;
    size.Height = height;
    size.Width = width;
    position.X = x;
    position.Y = y;
    xShape->setSize(size);
    xShape->setPosition(position);

    return xShape;
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
