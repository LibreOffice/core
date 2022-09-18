/***************************************************************************
    imathutils.cxx  -  Utility functions for iMath
                             -------------------
    begin                : Fri May 30 2008
    copyright            : (C) 2008 by Jan Rheinlaender
    email                : jrheinlaender@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// OLE object types
// TextContent -> CLSID_FORMULA
// XEmbeddedObjectSupplier
// getEmbeddedObject() -> XModel
// FormulaDocument

// OLE2Shape -> CLSID_FORMULA
// XShape
// property Model -> XModel
// FormulaDocument

#ifndef _COM_SUN_STAR_AWT_WINDOWATTRIBUTE_HPP_
#include <com/sun/star/awt/WindowAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XMESSAGEBOX_HPP_
#include <com/sun/star/awt/XMessageBox.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XMESSAGEBOXFACTORY_HPP_
#include <com/sun/star/awt/XMessageBoxFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOWPEER_HPP_
#include <com/sun/star/awt/XWindowPeer.hpp>
#endif
#ifndef  _COM_SUN_STAR_TEXT_XTEXTDOCUMENT_HPP_
#include <com/sun/star/text/XTextDocument.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTVIEWCURSOR_HPP_
#include <com/sun/star/text/XTextViewCursor.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXT_HPP_
#include <com/sun/star/text/XText.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#include "com/sun/star/document/XEmbeddedObjectSupplier.hpp"
#include "com/sun/star/text/XTextViewCursorSupplier.hpp"
#include "com/sun/star/view/XSelectionSupplier.hpp"
#include "com/sun/star/util/XRefreshable.hpp"
#include "com/sun/star/awt/XNumericField.hpp"
#include "com/sun/star/awt/XCheckBox.hpp"
#include "com/sun/star/container/XEnumerationAccess.hpp"
#include "com/sun/star/container/XEnumeration.hpp"
#include "com/sun/star/container/XContentEnumerationAccess.hpp"
#include "com/sun/star/container/XNamed.hpp"
#include "com/sun/star/text/XTextTable.hpp"
#include "com/sun/star/text/XFootnote.hpp"
#include "com/sun/star/text/XTextFieldsSupplier.hpp"
#include "com/sun/star/text/XTextField.hpp"
#include "com/sun/star/text/XDependentTextField.hpp"
#include "com/sun/star/text/XTextTablesSupplier.hpp"
#include "com/sun/star/text/XParagraphCursor.hpp"
#include "com/sun/star/table/XCell.hpp"
#include "com/sun/star/util/XChangesBatch.hpp"
#include "com/sun/star/document/XEmbeddedObjectSupplier2.hpp"
#include "com/sun/star/embed/EmbedStates.hpp"
#include "com/sun/star/embed/EmbedMisc.hpp"
#include "com/sun/star/deployment/PackageInformationProvider.hpp"
#include "com/sun/star/deployment/XPackageInformationProvider.hpp"
#include "com/sun/star/chart/XChartData.hpp"
#include "com/sun/star/chart/XDiagram.hpp"
#include "com/sun/star/chart/XAxisXSupplier.hpp"
#include "com/sun/star/chart/XAxisYSupplier.hpp"
#include "com/sun/star/awt/MessageBoxButtons.hpp"
#include "com/sun/star/chart/ChartDataRowSource.hpp"
#include "com/sun/star/beans/XMultiPropertySet.hpp"
#include "com/sun/star/chart2/data/XDataProvider.hpp"
#include "com/sun/star/chart2/data/XDataSource.hpp"
#include "com/sun/star/chart2/data/XDataSink.hpp"
#include "com/sun/star/chart2/data/XDataSequence.hpp"
#include "com/sun/star/chart2/data/XLabeledDataSequence.hpp"
#include "com/sun/star/chart2/XChartTypeContainer.hpp"
#include "com/sun/star/chart2/XChartType.hpp"
#include "com/sun/star/chart2/XCoordinateSystemContainer.hpp"
#include "com/sun/star/chart2/XDataSeries.hpp"
#include "com/sun/star/chart2/XDataSeriesContainer.hpp"
#include "com/sun/star/chart2/XTitled.hpp"
#include "com/sun/star/chart2/XTitle.hpp"
#include "com/sun/star/chart2/XFormattedString.hpp"
#if (OO_IS_AOO == 0)
#include "com/sun/star/chart2/XFormattedString2.hpp"
#endif
#include "com/sun/star/chart2/XLegend.hpp"
#include "com/sun/star/chart2/XCoordinateSystem.hpp"
#include "com/sun/star/chart2/XAxis.hpp"
#include "com/sun/star/lang/IndexOutOfBoundsException.hpp"
#include "com/sun/star/util/XCloneable.hpp"
#include "com/sun/star/awt/XDialogProvider2.hpp"
#include "com/sun/star/rdf/XDocumentMetadataAccess.hpp"
#include "com/sun/star/rdf/URI.hpp"
#include "com/sun/star/rdf/URIs.hpp"
#include "com/sun/star/rdf/XURI.hpp"
#if (OO_MAJOR_VERSION == 3) && (OO_MINOR_VERSION <= 5) || (OO_MAJOR_VERSION >= 7) || (OO_IS_AOO == 1)
#include "com/sun/star/rdf/XResource.hpp"
#endif
#include "com/sun/star/rdf/XRepository.hpp"
#include "com/sun/star/rdf/Statement.hpp"
#include "com/sun/star/container/ElementExistException.hpp"
#include "com/sun/star/container/XEnumeration.hpp"
#include "com/sun/star/rdf/Literal.hpp"
#include "com/sun/star/rdf/XLiteral.hpp"
#if (OO_MAJOR_VERSION == 3) && (OO_MINOR_VERSION <= 5) || (OO_MAJOR_VERSION >= 7) || (OO_IS_AOO == 1)
#include "com/sun/star/rdf/XNode.hpp"
#endif
#include "com/sun/star/frame/XComponentLoader.hpp"
#include "com/sun/star/sheet/XSpreadsheets.hpp"
#include "com/sun/star/sheet/XSpreadsheet.hpp"
#include "com/sun/star/table/XCellRange.hpp"
#include "com/sun/star/table/XTableColumns.hpp"
#include "com/sun/star/table/XTableRows.hpp"
#include "com/sun/star/drawing/XDrawPagesSupplier.hpp"
#include "com/sun/star/drawing/XDrawPages.hpp"
#include "com/sun/star/container/XIndexAccess.hpp"
#include "com/sun/star/drawing/XDrawPage.hpp"
#include "com/sun/star/drawing/XShapes.hpp"
#include "com/sun/star/drawing/XShape.hpp"
#include "com/sun/star/util/XCloseable.hpp"
#include "com/sun/star/frame/XStorable.hpp"
#include "com/sun/star/ucb/XFileIdentifierConverter.hpp"

#include <cmath>
#include <limits>
#include <sstream>
#include <fstream>
#ifdef _MSC_VER
#ifndef LIBO_INTERNAL_ONLY
#define NOMINMAX
#endif#include <windows.h>
#else
#include <unistd.h>
#endif

#include "imathutils.hxx"
#include "unit.hxx"
#include "msgdriver.hxx"
#include "stringex.hxx"
#include "iIterator.hxx"

using com::sun::star::awt::WindowDescriptor;
using com::sun::star::awt::XControlModel;
using com::sun::star::awt::XMessageBox;
using com::sun::star::awt::XMessageBoxFactory;
using com::sun::star::awt::Rectangle;
using com::sun::star::awt::XWindowPeer;
using com::sun::star::document::XEmbeddedObjectSupplier;
using com::sun::star::lang::XComponent;
using com::sun::star::text::XTextDocument;
using com::sun::star::lang::XMultiServiceFactory;
using com::sun::star::text::XTextViewCursorSupplier;
using com::sun::star::view::XSelectionSupplier;
using com::sun::star::container::XIndexAccess;
using com::sun::star::text::XTextCursor;
using com::sun::star::text::XParagraphCursor;
using com::sun::star::text::XText;
using com::sun::star::text::XTextViewCursor;
using com::sun::star::util::XRefreshable;
using com::sun::star::awt::XNumericField;
using com::sun::star::awt::XCheckBox;
using com::sun::star::container::XEnumerationAccess;
using com::sun::star::container::XEnumeration;
using com::sun::star::container::XContentEnumerationAccess;
using com::sun::star::container::XNamed;
using com::sun::star::lang::XServiceInfo;
using com::sun::star::text::XTextTable;
using com::sun::star::text::XTextTablesSupplier;
using com::sun::star::text::XFootnote;
using com::sun::star::text::XTextFieldsSupplier;
using com::sun::star::text::XTextField;
using com::sun::star::text::XDependentTextField;
using com::sun::star::table::XCell;
using com::sun::star::container::XNameAccess;
using com::sun::star::util::XChangesBatch;
using com::sun::star::document::XEmbeddedObjectSupplier2;
using com::sun::star::deployment::XPackageInformationProvider;
using com::sun::star::chart::XChartData;
using com::sun::star::chart::XDiagram;
using com::sun::star::chart::XAxisXSupplier;
using com::sun::star::chart::XAxisYSupplier;
using com::sun::star::beans::XMultiPropertySet;
using com::sun::star::chart2::data::XDataProvider;
using com::sun::star::chart2::data::XLabeledDataSequence;
using com::sun::star::chart2::XChartTypeContainer;
using com::sun::star::chart2::data::XDataSequence;
using com::sun::star::chart2::data::XDataSource;
using com::sun::star::chart2::data::XDataSink;
using com::sun::star::chart2::XChartType;
using com::sun::star::chart2::XCoordinateSystemContainer;
using com::sun::star::chart2::XDataSeries;
using com::sun::star::chart2::XDataSeriesContainer;
using com::sun::star::chart2::XTitled;
using com::sun::star::chart2::XTitle;
using com::sun::star::chart2::XFormattedString;
#if (OO_IS_AOO == 0)
using com::sun::star::chart2::XFormattedString2;
#endif
using com::sun::star::chart2::XLegend;
using com::sun::star::chart2::XCoordinateSystem;
using com::sun::star::chart2::XAxis;
using com::sun::star::lang::IndexOutOfBoundsException;
using com::sun::star::util::XCloneable;
using com::sun::star::awt::XDialogProvider2;
using com::sun::star::lang::XMultiComponentFactory;
using com::sun::star::rdf::XDocumentMetadataAccess;
using com::sun::star::rdf::URI;
using com::sun::star::rdf::XURI;
#if (OO_MAJOR_VERSION == 3) && (OO_MINOR_VERSION <= 5) || (OO_MAJOR_VERSION >= 7) || (OO_IS_AOO == 1)
using com::sun::star::rdf::XResource;
#endif
using com::sun::star::rdf::XRepository;
using com::sun::star::rdf::Statement;
using com::sun::star::container::ElementExistException;
using com::sun::star::container::XEnumeration;
using com::sun::star::rdf::Literal;
using com::sun::star::rdf::XLiteral;
#if (OO_MAJOR_VERSION == 3) && (OO_MINOR_VERSION <= 5) || (OO_MAJOR_VERSION >= 7) || (OO_IS_AOO == 1)
using com::sun::star::rdf::XNode;
#endif
using com::sun::star::frame::XComponentLoader;
using com::sun::star::sheet::XSpreadsheets;
using com::sun::star::sheet::XSpreadsheet;
using com::sun::star::table::XCellRange;
using com::sun::star::table::XTableColumns;
using com::sun::star::table::XTableRows;
using com::sun::star::drawing::XDrawPagesSupplier;
using com::sun::star::drawing::XDrawPages;
using com::sun::star::container::XIndexAccess;
using com::sun::star::drawing::XDrawPage;
using com::sun::star::drawing::XShapes;
using com::sun::star::drawing::XShape;
using com::sun::star::util::XCloseable;
using com::sun::star::frame::XStorable;
using com::sun::star::ucb::XFileIdentifierConverter;

using namespace GiNaC;

/**
  * Show a message box with the UNO based toolkit
  */
short ShowMessageBox(const Reference< XToolkit >& rToolkit, const Reference< XFrame >& rFrame,
                           const OUString& aTitle, const OUString& aMsgText, const unsigned width, const long buttons) {
  if (rFrame.is() && rToolkit.is()) {
    // describe window properties.
    WindowDescriptor                aDescriptor;
    aDescriptor.Type              = com::sun::star::awt::WindowClass_MODALTOP;
    aDescriptor.WindowServiceName = OU("infobox");
    aDescriptor.ParentIndex       = -1;
    aDescriptor.Parent            = Reference< XWindowPeer >(rFrame->getContainerWindow(), UNO_QUERY_THROW);
    aDescriptor.Bounds            = com::sun::star::awt::Rectangle(0,0, width, 200); // the Height attribute seems to have no effect
    aDescriptor.WindowAttributes  = com::sun::star::awt::WindowAttribute::BORDER | com::sun::star::awt::WindowAttribute::MOVEABLE |
                                    com::sun::star::awt::WindowAttribute::CLOSEABLE | com::sun::star::awt::WindowAttribute::SIZEABLE;

    Reference< XWindowPeer > xPeer = rToolkit->createWindow(aDescriptor);

    if (xPeer.is()) {
      Reference< XMessageBoxFactory > xMsgBoxFactory(rToolkit, UNO_QUERY_THROW);
#if (OO_MAJOR_VERSION == 4)
  #if (OO_MINOR_VERSION >= 2) || (OO_IS_AOO == 1)
      Reference< XMessageBox > xMessageBox = xMsgBoxFactory->createMessageBox(xPeer, ::com::sun::star::awt::MessageBoxType_MESSAGEBOX, buttons, aTitle, aMsgText);
  #else
    Reference< XMessageBox > xMessageBox = xMsgBoxFactory->createMessageBox(xPeer, com::sun::star::awt::Rectangle(), OU("errorbox"), buttons, aTitle, aMsgText);
  #endif
#elif (OO_MAJOR_VERSION > 4)
      Reference< XMessageBox > xMessageBox = xMsgBoxFactory->createMessageBox(xPeer, ::com::sun::star::awt::MessageBoxType_MESSAGEBOX, buttons, aTitle, aMsgText);
#else
  Reference< XMessageBox > xMessageBox = xMsgBoxFactory->createMessageBox(xPeer, com::sun::star::awt::Rectangle(), OU("errorbox"), buttons, aTitle, aMsgText);
#endif
      if (xMessageBox.is())
        return xMessageBox->execute();
    }
  }

  return 0;
} // ShowMessageBox()

Reference<XDialog> createBasicDialog (const Reference < XComponentContext > &xCC, const Reference< XToolkit >& xToolkit,
                                      const Reference< XFrame >& xFrame, const OUString &DialogURL, const Reference< XInterface >& xHandler) {
  try {
    Reference< XDialogProvider2 > xDialogProvider;
    Reference < XController > xCtrl = xFrame->getController();
    Reference< XMultiComponentFactory > xMCF = xCC->getServiceManager();

    // If valid we must pass the XModel when creating a DialogProvider object
    Reference< XModel > xModel = xCtrl->getModel();
    if (xModel.is()) {
      Sequence< Any > args(1);
      args[0] = makeAny(xModel);

      xDialogProvider = Reference< XDialogProvider2 >(
        xMCF->createInstanceWithArgumentsAndContext(OU("com.sun.star.awt.DialogProvider2"), args, xCC), UNO_QUERY_THROW);
    } else {
      xDialogProvider = Reference< XDialogProvider2 >(
       xMCF->createInstanceWithContext(OU("com.sun.star.awt.DialogProvider2"), xCC), UNO_QUERY_THROW);
    }
    if (xHandler.is())
      return (xDialogProvider->createDialogWithHandler(DialogURL, xHandler));
    else
      return (xDialogProvider->createDialog( DialogURL));
  } catch (Exception &e) {
    ShowMessageBox(xToolkit, xFrame, OU("Dialog creation failed"), e.Message);
    return Reference<XDialog>();
  }
} // createDialog()

// get the first user selection (even if multiple items have been selected)
Reference < XInterface > getUserSelection(const Reference < XController > &xCtrl) {
  Reference < XSelectionSupplier > xSS(xCtrl, UNO_QUERY_THROW);
  Any xSelection = xSS->getSelection();

  Reference < XInterface > oSelection(xSelection, UNO_QUERY_THROW);
  Reference < XIndexAccess > xIndexAccess(oSelection, UNO_QUERY); // UNO_QUERY_THROW is not appropriate here!
  Reference < XInterface > oIndex;

  if (xIndexAccess.is()) { // The selection is a container
    if (xIndexAccess->getCount() == 0) {
      throw  RuntimeException(OU("Nothing selected"), oSelection);
    }

    Any Index = xIndexAccess->getByIndex(0);
    Reference < XInterface > oI(Index, UNO_QUERY_THROW);
    oIndex = oI;
  } else { // single selection
    oIndex = oSelection;
  }

  if (!oIndex.is()) throw RuntimeException(OU("Invalid selection"), oIndex);
  return oIndex;
} // getUserSelection()

// get the user selections
Sequence<Reference < XInterface > > getUserSelections(const Reference < XController > &xCtrl) {
  Reference < XSelectionSupplier > xSS(xCtrl, UNO_QUERY_THROW);
  Any xSelection = xSS->getSelection();

  Reference < XInterface > oSelection(xSelection, UNO_QUERY_THROW);
  Reference < XIndexAccess > xIndexAccess(oSelection, UNO_QUERY); // UNO_QUERY_THROW is not appropriate here!


  if (xIndexAccess.is()) { // The selection is a container
    if (xIndexAccess->getCount() == 0) {
      throw  RuntimeException(OU("Nothing selected"), oSelection);
    }

    Sequence< Reference < XInterface > > result(xIndexAccess->getCount());
    for (int i = 0; i < xIndexAccess->getCount(); i++) {
      Any Index = xIndexAccess->getByIndex(i);
      Reference < XInterface > oI(Index, UNO_QUERY_THROW);
      if (!oI.is()) throw RuntimeException(OU("Invalid selection"), oSelection);
      result[i] = oI;
    }
    return(result);
  } else { // single selection
    if (!oSelection.is()) throw RuntimeException(OU("Invalid selection"), oSelection);
    Sequence< Reference < XInterface > > result(1);
    result[0] = oSelection;
    return(result);
  }
} // getUserSelections()

/// Set the user selection
void setUserSelection(const Reference < XController > &xCtrl, const Reference < XComponent >& newSelection) {
  Reference < XSelectionSupplier > xSS(xCtrl, UNO_QUERY_THROW);
  Any xSelection = makeAny(newSelection);
  xSS->select(xSelection);
} // setUserSelection()

bool getSelectedFormula(const Reference < XController >& xCtrl, Reference < XComponent >& mFormula, const bool strict) {
  try {
    Reference < XInterface > oSelection(getUserSelection(xCtrl));
    mFormula = Reference < XComponent >(oSelection, UNO_QUERY); // UNO_QUERY_THROW makes .is() redundant

    if (strict)
      return (mFormula.is() && checkIsiFormula(mFormula));
    else
      return (mFormula.is() && checkIsFormula(mFormula));
  } catch (Exception &e) {
    (void)e; // Nothing selected
  }

  return false;
}

OUString getDocUID(const Reference< XModel >& xModel) {
  Reference< XPropertySet > xProps(xModel, UNO_QUERY);
  if (!xProps.is()) { // Happens in ooBase
    MSG_INFO(3,  "No UID for this document (are you running sbase?)" << endline);
    return OU("");
  }
  Any value = xProps->getPropertyValue(OU("RuntimeUID"));

  OUString result = OU("");
  value >>= result;
  return result;
} // getDocUID()

/// Extract the text from an edit field
OUString getEditfield(const Sequence < PropertyValue >& lArgs) {
  OUString result = OUString::createFromAscii("");
  for (sal_Int32 i = 0; i < lArgs.getLength(); i++) {
    //ShowMessageBox(mxToolkit, mxFrame, lArgs[i].Name, OUString::createFromAscii(""));
    if (lArgs[i].Name.equalsAsciiL( "Text", 4 )) {
      lArgs[i].Value >>= result;
      break;
    }
  }
  return result;
} // getEditfield()

Reference < XText > getDocumentText(const Reference < XController > &xCtrl, Reference < XTextCursor > &xModelCursor) {
  // First check whether the user has selected something
  // Because if he has selected a table or a formula, the other code will crash on getText()
  Reference < XInterface > oSelection;
  try {
    oSelection = getUserSelection(xCtrl);
  } catch (Exception &e) {
    (void)e; // Nothing selected, continue
  }

  // What has been selected?
  // TODO: If the user selects more than one table cell, the code still crashes
  Reference < XTextContent > xTextContent(oSelection, UNO_QUERY); // UNO_QUERY_THROW is not appropriate here
  Reference < XText > xDocumentText;

  if (xTextContent.is()) { // This needs separate treatment
    // Insert at the anchor point of the selected XTextContent
    xDocumentText = xTextContent->getAnchor()->getText();

    // the text creates a model cursor from the viewcursor after the current position
    xModelCursor = xDocumentText->createTextCursorByRange(xTextContent->getAnchor()->getEnd());
    xModelCursor->goRight(1, false); // Move to after the anchor point
  } else {
    // Get the view cursor and create a model cursor from it and insert there
    Reference < XTextViewCursorSupplier > xCursor(xCtrl, UNO_QUERY_THROW );
    Reference < XTextViewCursor > xTextViewCursor = xCursor->getViewCursor();
    xDocumentText = xTextViewCursor->getText();
    // the text creates a model cursor from the viewcursor after the current position
    xModelCursor = xDocumentText->createTextCursorByRange(xTextViewCursor->getEnd());
  }

  return xDocumentText;
} // getDocumentText();

Reference < XComponent > createObject(const Reference < XModel > &xModel, const OUString& clsid) {
  if (docType(xModel).equalsAscii("TextDocument")) {
    // Get a factory to create objects for this document
    Reference < XMultiServiceFactory > xDocMSF(xModel, UNO_QUERY_THROW);

    // Have the factory create an empty TextEmbeddedObject
    Reference < XComponent > xObj;
    try {
      xObj = Reference < XComponent >(xDocMSF->createInstance(OU("com.sun.star.text.TextEmbeddedObject")), UNO_QUERY_THROW);
    } catch (Exception &e) {
      MSG_INFO(3,  "Problem creating object: " << STR(e.Message) << endline);
    }
    Reference < XPropertySet > xPS (xObj, UNO_QUERY_THROW);

    // Give the empty object a type, and anchor it as a character, and set the margins to zero
    Any Aclsid;
    Aclsid = makeAny(clsid);
    Any Aanchor;
    Aanchor = makeAny(sal_uInt32(1)); // TextContentAnchorType_AS_CHARACTER);
    Any Amargin;
    Amargin = makeAny(sal_uInt32(0));
    xPS->setPropertyValue(OU("CLSID"), Aclsid);
    xPS->setPropertyValue(OU("AnchorType"), Aanchor);
    xPS->setPropertyValue(OU("LeftMargin"), Amargin);
    xPS->setPropertyValue(OU("RightMargin"), Amargin);

    return xObj;
  } else if (docType(xModel).equalsAscii("Presentation")) {
    // Get a factory to create objects for this document
    Reference < XMultiServiceFactory > xDocMSF(xModel, UNO_QUERY_THROW);

    // Have the factory create an empty OLE2Shape
    Reference < XComponent > xObj;
    try {
      xObj = Reference < XComponent >(xDocMSF->createInstance(OU("com.sun.star.drawing.OLE2Shape")), UNO_QUERY_THROW);
    } catch (Exception &e) {
      MSG_INFO(3,  "Problem creating object: " << STR(e.Message) << endline);
    }
    Reference < XPropertySet > xPS (xObj, UNO_QUERY_THROW);

    // Give the empty object a type
    Any Aclsid;
    Aclsid = makeAny(clsid);
    xPS->setPropertyValue(OU("CLSID"), Aclsid);
    return xObj;
  } else {
    return Reference< XComponent >();
  }
} // createObject()

Reference < XComponent > insertObject(const Reference < XModel > &xModel, const OUString& clsid) {
  Reference < XComponent > xObject = createObject(xModel, clsid);

  if (docType(xModel).equalsAscii("TextDocument")) {
    Reference < XTextCursor > xModelCursor;
    Reference < XText > xDocumentText = getDocumentText(xModel->getCurrentController(), xModelCursor);

    // Remove user selection
    try {
      Reference < XInterface > oSelection = getUserSelection(xModel->getCurrentController());
      Reference < XTextRange > oText(oSelection, UNO_QUERY_THROW);
      oText->setString(OU(""));
    } catch (Exception& e) { (void)e; /* nothing was selected */ }

    // Insert the object
    Reference < XTextRange > xRange(xModelCursor, UNO_QUERY_THROW);
    xDocumentText->insertTextContent(xRange, Reference< XTextContent >(xObject, UNO_QUERY_THROW), true);
  } else if (docType(xModel).equalsAscii("Presentation")) {
    MSG_INFO(0, "insertObject() in Presentation" << endline);
    // Get current page
    Reference< XPropertySet >xPropSet(xModel->getCurrentController(), UNO_QUERY_THROW);
    Any aCurrentPage = xPropSet->getPropertyValue(OU("CurrentPage"));
    Reference< XDrawPage > xDrawPage;
    aCurrentPage >>= xDrawPage;

    // Get page size (in 100th mm)
    Reference< XPropertySet > pageProps(xDrawPage, UNO_QUERY_THROW);
    Any aWidth = pageProps->getPropertyValue(OU("Width"));
    Any aHeight = pageProps->getPropertyValue(OU("Width"));
    sal_Int32 pageWidth = 0;
    sal_Int32 pageHeight = 0;
    aWidth >>= pageWidth;
    aHeight >>= pageHeight;

    // Position the object
    Reference< XShape > xShape(xObject, UNO_QUERY_THROW);
    Reference< XPropertySet > shapeProps(xShape, UNO_QUERY_THROW);
    com::sun::star::awt::Point pos;
    if (clsid == CLSID_FORMULA) {
      pos.X = pageWidth/3;
      pos.Y = pageHeight/3;
    } else if (clsid == CLSID_CHART) {
      pos.X = 6020; // Values taken from Presentation UI default values on my system
      pos.Y = 3390;
    }
    xShape->setPosition(pos);
    Reference< XShapes > xShapes(xDrawPage, UNO_QUERY_THROW);
    xShapes->add(xShape);

    // Enforce proper sizing (without this, the size is the maximum possible size on the DrawPage for formulas, and 101x101 for charts)
    Reference< XPropertySet > fPS(xShape, UNO_QUERY);
    com::sun::star::awt::Size size;

    if (clsid == CLSID_FORMULA) {
      Any aSize = fPS->getPropertyValue(OU("OriginalSize")); // Note: OriginalSize is always zero before the Shape is added to xShapes
      aSize >>= size;
      //Reference< XModel > fModel = extractModel(xObject);
      //size.Height = (getFormulaUnsignedProperty(fModel, OU("BaseFontHeight")) +
      //               getFormulaUnsignedProperty(fModel, OU("TopMargin")) +
      //               getFormulaUnsignedProperty(fModel, OU("BottomMargin"))) * 35.278 * 1.1;
    } else if (clsid == CLSID_CHART) {
      size.Height = 9000;
      size.Width = 16000;
      fPS->setPropertyValue(OU("SizeProtect"), makeAny(false));
    }

    xShape->setSize(size);

    // Set a proper object name (is empty by default)
    Any aName = fPS->getPropertyValue(OU("PersistName"));
    OUString objName;
    aName >>= objName;
    setObjectName(xObject, objName);
    MSG_INFO(0, "Set automatic object name to " << STR(objName) << endline);
  } else {
    return Reference< XComponent >();
  }

  return xObject;
} // insertObject()

// Append a new XY-data series using an existing XY-series as a template
void addDataSeries(const Reference < com::sun::star::chart2::XChartDocument >& chart, const sal_Int32 idx) {
  Reference< XDataProvider > dataProvider = chart->getDataProvider();
  // Clone an X data sequence
  Reference< XCloneable > templateDataSeq(dataProvider->createDataSequenceByRangeRepresentation(OU("0")), UNO_QUERY_THROW);
  Reference< XDataSequence > seqDataX(templateDataSeq->createClone(), UNO_QUERY_THROW);
  Reference < XPropertySet > seqProps (seqDataX, UNO_QUERY_THROW);
  seqProps->setPropertyValue(OU("Role"), makeAny(OU("values-x")));
  Reference < XNamed > oName(seqDataX, UNO_QUERY_THROW);
  oName->setName(OUSTRINGNUMBER(idx));
  Reference< XCloneable > templateLabelSeq(dataProvider->createDataSequenceByRangeRepresentation(OU("label 0")), UNO_QUERY_THROW);
  Reference< XDataSequence > seqLabelX(templateLabelSeq->createClone(), UNO_QUERY_THROW);
  oName = Reference < XNamed >(seqLabelX, UNO_QUERY_THROW);
  oName->setName(OU("label ") + OUSTRINGNUMBER(idx));
  // Clone an Y data sequence
  Reference< XDataSequence > seqDataY(templateDataSeq->createClone(), UNO_QUERY_THROW);
  seqProps = Reference< XPropertySet >(seqDataY, UNO_QUERY_THROW);
  seqProps->setPropertyValue(OU("Role"), makeAny(OU("values-y")));
  oName = Reference < XNamed >(seqDataY, UNO_QUERY_THROW);
  oName->setName(OUSTRINGNUMBER(idx+1));
  Reference< XDataSequence > seqLabelY(templateLabelSeq->createClone(), UNO_QUERY_THROW);
  oName = Reference < XNamed >(seqLabelY, UNO_QUERY_THROW);
  oName->setName(OU("label ") + OUSTRINGNUMBER(idx));

  Reference< ::com::sun::star::chart2::XDiagram > diagram = chart->getFirstDiagram();
  Reference< XCoordinateSystemContainer > xCoordCnt(diagram, UNO_QUERY_THROW);
  Reference< XChartTypeContainer > xChartTypeCnt(xCoordCnt->getCoordinateSystems()[0], UNO_QUERY_THROW);
  Reference< XDataSeriesContainer > chartType(xChartTypeCnt->getChartTypes()[0], UNO_QUERY_THROW);
  Reference< XDataSource > XYSource(chartType->getDataSeries()[0], UNO_QUERY_THROW);

  Sequence< Reference< XLabeledDataSequence > > sequences(2);
  Reference< XCloneable > templateSeq(XYSource->getDataSequences()[0], UNO_QUERY_THROW);
  sequences[0] = Reference< XLabeledDataSequence >(templateSeq->createClone(), UNO_QUERY_THROW);
  sequences[1] = Reference< XLabeledDataSequence >(templateSeq->createClone(), UNO_QUERY_THROW);
  sequences[0]->setValues(seqDataX);
  sequences[0]->setLabel(seqLabelX);
  sequences[1]->setValues(seqDataY);
  sequences[1]->setLabel(seqLabelY);

  // Add a new series
  Reference< XCloneable > templateSeries(chartType->getDataSeries()[0], UNO_QUERY_THROW);
  Reference< XDataSeries > xDataSeries(templateSeries->createClone(), UNO_QUERY_THROW);
  Reference< XDataSink > XYSink(xDataSeries, UNO_QUERY_THROW);
  XYSink->setData(sequences);
  chartType->addDataSeries(xDataSeries);
}

void forceDiagramUpdate(const Reference< XComponent >& xChart) {
  // Hack: Force the diagram to recognize changed data points
  Reference < XEmbeddedObjectSupplier2 >xEOS2(xChart, UNO_QUERY);
  if (xEOS2.is()) {
    Reference < XEmbeddedObject > xEmbObj(xEOS2->getExtendedControlOverEmbeddedObject());
    if (xEmbObj->getCurrentState() == com::sun::star::embed::EmbedStates::LOADED)
        xEmbObj->changeState( com::sun::star::embed::EmbedStates::RUNNING);
    if (xEmbObj->getCurrentState() == com::sun::star::embed::EmbedStates::RUNNING)
        xEmbObj->changeState( com::sun::star::embed::EmbedStates::INPLACE_ACTIVE);
  }

  Reference < ::com::sun::star::chart::XChartDocument > cDoc(extractModel(xChart), UNO_QUERY_THROW);
  Reference< XPropertySet > dProperties(cDoc->getDiagram(), UNO_QUERY_THROW);
  Any type = dProperties->getPropertyValue(OU("SplineType"));
  dProperties->setPropertyValue(OU("SplineType"), makeAny(sal_uInt32(0)));
  dProperties->setPropertyValue(OU("SplineType"), makeAny(sal_uInt32(1)));
  dProperties->setPropertyValue(OU("SplineType"), type);

  if (xEOS2.is()) {
    Reference < XEmbeddedObject > xEmbObj(xEOS2->getExtendedControlOverEmbeddedObject());
    if (xEmbObj->getCurrentState() == com::sun::star::embed::EmbedStates::INPLACE_ACTIVE)
      xEmbObj->changeState( com::sun::star::embed::EmbedStates::RUNNING);
  }
}

Reference < XComponent > insertChart(const Reference < XModel > &xModel, const Reference < XComponentContext > &xCC) {
  Reference< XComponent > xChart = insertObject(xModel, CLSID_CHART);
  Reference < com::sun::star::chart::XChartDocument > cDoc(extractModel(xChart), UNO_QUERY_THROW);
  Reference < XMultiServiceFactory > cDocMSF(cDoc, UNO_QUERY_THROW);

  // Make sure that we have a clear interpretation of where data series are
  Reference < XPropertySet > cDocProps (cDoc, UNO_QUERY_THROW);
  cDocProps->setPropertyValue(OU("DataRowSource"), makeAny(::com::sun::star::chart::ChartDataRowSource_ROWS));

  // Create a new XYDiagram
  Reference < XDiagram > xyDiagram(cDocMSF->createInstance(OU("com.sun.star.chart.XYDiagram")), UNO_QUERY_THROW);
  cDoc->setDiagram(xyDiagram);

  // Create two new sequences based on the two first default sequences, setting the roles correctly for an XYDiagram
  Reference < ::com::sun::star::chart2::XChartDocument > chart(cDoc, UNO_QUERY_THROW);
  chart->createInternalDataProvider(true);
  Reference< XDataProvider > dataProvider = chart->getDataProvider();
  Reference< XDataSequence > seqDataX = dataProvider->createDataSequenceByRangeRepresentation(OU("0"));
  Reference < XPropertySet > seqProps (seqDataX, UNO_QUERY_THROW);
  seqProps->setPropertyValue(OU("Role"), makeAny(OU("values-x")));
  Reference< XDataSequence > seqLabelX = dataProvider->createDataSequenceByRangeRepresentation(OU("label 0"));
  Reference< XDataSequence > seqDataY = dataProvider->createDataSequenceByRangeRepresentation(OU("1"));
  seqProps = Reference< XPropertySet >(seqDataY, UNO_QUERY_THROW);
  seqProps->setPropertyValue(OU("Role"), makeAny(OU("values-y")));
  Reference< XDataSequence > seqLabelY = dataProvider->createDataSequenceByRangeRepresentation(OU("label 1"));

  // Get write access to the chart series
  Reference< ::com::sun::star::chart2::XDiagram > diagram = chart->getFirstDiagram();
  Reference< XCoordinateSystemContainer > xCoordCnt(diagram, UNO_QUERY_THROW);
  Reference< XChartTypeContainer > xChartTypeCnt(xCoordCnt->getCoordinateSystems()[0], UNO_QUERY_THROW);
  Reference< XDataSeriesContainer > chartType(xChartTypeCnt->getChartTypes()[0], UNO_QUERY_THROW);

  // Create two sequences
  Reference< XMultiComponentFactory > xServiceManager(xCC->getServiceManager()); // get the service manager (the document service factory cannot create a LabeledDataSequence!)
  Sequence< Reference< XLabeledDataSequence > > sequences(2);
  sequences[0] = Reference< XLabeledDataSequence > (xServiceManager->createInstanceWithContext(OU("com.sun.star.chart2.data.LabeledDataSequence"), xCC), UNO_QUERY_THROW);
  sequences[1] = Reference< XLabeledDataSequence > (xServiceManager->createInstanceWithContext(OU("com.sun.star.chart2.data.LabeledDataSequence"), xCC), UNO_QUERY_THROW);
  sequences[0]->setValues(seqDataX);
  sequences[0]->setLabel(seqLabelX);
  sequences[1]->setValues(seqDataY);
  sequences[1]->setLabel(seqLabelY);

  // Set the data sequences into the chart
  Reference< XDataSink > XYSink(xServiceManager->createInstanceWithContext(OU("com.sun.star.chart2.DataSeries"), xCC), UNO_QUERY_THROW);
  XYSink->setData(sequences);
  Sequence< Reference< XDataSeries > > XYSeries(1);
  XYSeries[0] = Reference< XDataSeries > (XYSink, UNO_QUERY_THROW);
  chartType->setDataSeries(XYSeries);

  // Fill in some data
  Sequence< double > emptyRow(2);
  emptyRow[0] = 0;
  emptyRow[1] = 0;
  Sequence< Sequence<double> > emptyData(1);
  emptyData[0] = emptyRow;
  Reference < XChartDataArray > cDataArray(chart->getDataProvider(), UNO_QUERY_THROW);
  cDataArray->setData(emptyData);
  forceDiagramUpdate(xChart);

  Reference< XPropertySet > dProperties(xyDiagram, UNO_QUERY_THROW);
  dProperties->setPropertyValue(OU("SplineType"), makeAny(sal_uInt32(1)));

  return xChart;
} // insertChart()

void setSeriesProperties(const Reference< XComponent >& xChart, const sal_uInt16 series,
      const sal_uInt16 pointsize, const sal_uInt16 linewidth, const sal_uInt32 linecolor) {
  Reference < com::sun::star::chart::XChartDocument > cDoc(extractModel(xChart), UNO_QUERY_THROW);
  Reference < XDiagram > xyDiagram = cDoc->getDiagram();
  Reference < XPropertySet > xyProps;
  try {
    xyProps = xyDiagram->getDataRowProperties(series);
  } catch (IndexOutOfBoundsException&) {
    return;
  }
  com::sun::star::awt::Size psize;
  psize.Height = pointsize;
  psize.Width = pointsize;
  xyProps->setPropertyValue(OU("SymbolSize"), makeAny(psize));
  xyProps->setPropertyValue(OU("LineWidth"), makeAny(linewidth));
  xyProps->setPropertyValue(OU("LineColor"), makeAny(linecolor));
} // setSeriesProperties()

double forceDouble(const expression& val, const double not_a_number) {
  if (is_a<numeric>(val))
    try { // info_flags::rational or ::real doesn't work for e.g. -%pi:%pi as the x range
      return ex_to<numeric>(val).to_double();
    } catch(Exception& e) { (void)e; };
  return not_a_number;
} // forceDouble()

void setChartData(const Reference< com::sun::star::chart2::XChartDocument > cDoc, const OUString& cName,
                  const matrix& newx, const matrix& newy, const unsigned iseries) {
  MSG_INFO(3,  "cName: " << STR(cName) << endline);
  MSG_INFO(3,  "xval: '" << newx << "'" << endline);
  MSG_INFO(3,  "yval: '" << newy << "'" << endline);
  MSG_INFO(4,  "iseries: '" << iseries << "'" << endline);

  // get existing data
  Reference < XChartDataArray > cDataArray = getChartDataArray(cDoc);
  Sequence< Sequence< double > > data = cDataArray->getData();
  unsigned rows = data.getLength();
  unsigned series = data[0].getLength();
  MSG_INFO(4,  "series: '" << series << "'" << endline);

  while (iseries > series) { // Add series (x- and y-column) to the data
    addDataSeries(cDoc, series+1);
    // There seems to be an "invisible" extra series before the new x-y-series in the data table that does not need
    // to be added but must be skipped when filling in the data
    series += 3;
  }
  MSG_INFO(4,  "new series: '" << series << "'" << endline);

  if (newx.rows() > rows) { // Make space for more points
    rows = newx.rows();
  } else if ((iseries == 1) && (series == 2) && (newx.rows() < rows)) {
    rows = newx.rows(); // Reduce number of points if there is only one series
  }

  double not_a_number;
  not_a_number = cDataArray->getNotANumber();
  Sequence< Sequence< double > > newData(rows);
  Sequence< double > newPoint(series);

  for (unsigned r = 0; r < rows; r++) {
    if (r < newx.rows()) {
      newPoint[iseries-1] = forceDouble(expression(newx(r,0)), not_a_number); // X-values
      newPoint[iseries] = forceDouble(expression(newy(r,0)), not_a_number); // Y-values
    }
    for (unsigned s = 0; s < series; s++) // copy old data
      if ((s != iseries-1) && (s != iseries))
        if (r < (unsigned)data.getLength())
          if (s < (unsigned)data[r].getLength())
            newPoint[s] = data[r][s];
    newData[r] = newPoint;
  }

  cDataArray->setData(newData);
  MSG_INFO(4,  "Finished setChartData()" << endline);
} // setChartData()

Sequence< Sequence<double> > getChartData(const Reference< XComponent >& xChart) {
  Reference < com::sun::star::chart2::XChartDocument > cDoc(extractModel(xChart), UNO_QUERY_THROW);
  Reference < XChartDataArray > cDataArray(cDoc->getDataProvider(), UNO_QUERY_THROW);
  return cDataArray->getData();
} // getChartData()

Reference<XComponent> getChartObjectByName(const Reference < XModel >& xModel, const OUString& cName) {
  try {
    return getObjectByName(xModel, cName);
  } catch (Exception& e) {
    (void)e;
    throw RuntimeException(OU("Error: Chart ") + cName + OU(" does not exist"), xModel);
  }
}

Reference < com::sun::star::chart2::XChartDocument > getChartDoc(const Reference < XModel >& xModel, const OUString& cName) {
  Reference < XComponent > xChart = getChartObjectByName(xModel, cName);
  Reference < com::sun::star::chart2::XChartDocument > cDoc(extractModel(xChart), UNO_QUERY_THROW);
  return cDoc;
}

Reference < XChartDataArray > getChartDataArray(const Reference < com::sun::star::chart2::XChartDocument >& cDoc) {
  // Fix changed interpretation of the data series source between ooo/lo versions
  Reference < XPropertySet > cDocProps (cDoc, UNO_QUERY_THROW);
  cDocProps->setPropertyValue(OU("DataRowSource"), makeAny(::com::sun::star::chart::ChartDataRowSource_ROWS));

  return (Reference < XChartDataArray >(cDoc->getDataProvider(), UNO_QUERY_THROW));
} // getChartDataArray()

void setSeriesDescription(Reference < com::sun::star::chart2::XChartDocument >& cDoc, const OUString& desc, const int idx) {
  Reference < XChartDataArray > cDataArray(cDoc->getDataProvider(), UNO_QUERY_THROW);
  Sequence< OUString > descriptions = cDataArray->getColumnDescriptions();
  //for (int c = 0; c < descriptions.getLength(); c++)
  //  MSG_INFO(3,  "Column " << c << ": " << descriptions[c] << endline);
  if (idx < descriptions.getLength()) {
    descriptions[idx] = desc;
    cDataArray->setColumnDescriptions(descriptions);
  }
} // setSeriesDescription()

void setSeriesDescription(const Reference< XComponent >& xChart, const OUString& desc, const int idx) {
  Reference < com::sun::star::chart2::XChartDocument > cDoc(extractModel(xChart), UNO_QUERY_THROW);
  setSeriesDescription(cDoc, desc, idx);
}

void setSeriesDescription(const Reference< XModel >& xModel, const OUString& cName, const OUString& desc, const int idx) {
  Reference < com::sun::star::chart2::XChartDocument > cDoc = getChartDoc(xModel, cName);
  setSeriesDescription(cDoc, desc, idx);
}

void setTitles(const Reference< XComponent >& xChart, const OUString& main, const OUString& xAxis, const OUString& yAxis) {
  Reference < com::sun::star::chart::XChartDocument > cDoc(extractModel(xChart), UNO_QUERY_THROW);
  Reference < XPropertySet > cProperties(cDoc, UNO_QUERY_THROW);
  if (!main.equalsAscii("")) {
    cProperties->setPropertyValue(OU("HasMainTitle"), makeAny(true));
    Reference < XPropertySet > cTProperties(cDoc->getTitle(), UNO_QUERY_THROW);
    cTProperties->setPropertyValue(OU("String"), makeAny(main));
  }
  Reference < XDiagram > cDiagram = cDoc->getDiagram();
  Reference < XAxisXSupplier > cAxisXSupplier(cDiagram, UNO_QUERY_THROW);
  Reference < XPropertySet > axProperties(cAxisXSupplier->getXAxisTitle(), UNO_QUERY_THROW);
  OUString xAx = xAxis;
  OUString yAx = yAxis;
  if (!xAxis.equalsAscii("")) {
    if (xAxis.matchAsciiL("\"",1,0))
      xAx = xAxis.copy(1, xAxis.getLength()-2);
    else if (xAxis.matchAsciiL("%",1,0))
      xAx = xAxis.copy(1);
    else if (xAxis == OU("1"))
      xAx = OU("");
  }
  axProperties->setPropertyValue(OU("String"), makeAny(xAx));
  Reference < XAxisYSupplier > cAxisYSupplier(cDiagram, UNO_QUERY_THROW);
  Reference < XPropertySet > ayProperties(cAxisYSupplier->getYAxisTitle(), UNO_QUERY_THROW);
  if (!yAxis.equalsAscii("")) {
    if (yAxis.matchAsciiL("\"",1,0))
      yAx = yAxis.copy(1, yAxis.getLength()-2);
    else if (yAxis.matchAsciiL("%",1,0))
      yAx = yAxis.copy(1);
    else if (yAxis == OU("1"))
      yAx = OU("");
  }
  ayProperties->setPropertyValue(OU("String"), makeAny(yAx));
} // setTitles()

void setChartData(const Reference < XModel >& xModel, const OUString& cName, const matrix& xval, const matrix& yval, const unsigned iseries) {
  // Sanity checks
  if (xval.cols() != 1) throw RuntimeException(OU("Vector of X values can only have one column"), xModel);
  if (yval.cols() != 1) throw RuntimeException(OU("Vector of Y values can only have one column"), xModel);
  if (xval.rows() != yval.rows()) throw RuntimeException(OU("Arguments to chart must be vectors of equal length"), xModel);

  // Find the chart
  Reference < com::sun::star::chart2::XChartDocument > cDoc = getChartDoc(xModel, cName);

  // Fill in the data
  setChartData(cDoc, cName, ex_to<matrix>(expression(xval.evalm()).evalf()), ex_to<matrix>(expression(yval.evalm()).evalf()), iseries);
  // Force diagram update
  Reference < XComponent > xChart = getChartObjectByName(xModel, cName);
  forceDiagramUpdate(xChart);
} // setChartData()

void setChartData(const Reference < XModel >& xModel, const OUString& cName,
  const extsymbol& s, const matrix& xval, const expression& yexpr, const unsigned iseries) {
  // Sanity checks
  if (xval.cols() != 1) throw RuntimeException(OU("Vector of X values can only have one column"), xModel);

  // Get X values
  matrix x = ex_to<matrix>(expression(xval).evalf());

  // Create Y values
  matrix yval(xval.rows(), 1);
  std::vector<std::string> vunsafe;
  vunsafe.emplace_back("unsafe");
  //MSG_INFO(3,  "Function for y: " << yexpr << endline);

  for (unsigned r = 0; r < yval.rows(); r++) {
    //MSG_INFO(3,  "Calculating yval for " << x(r,0) << endline);
    yval(r,0) = expression(yexpr.subs(s == xval(r,0))).evalf().simplify(vunsafe);
  }

  // Fill in the data
  Reference < com::sun::star::chart2::XChartDocument > cDoc = getChartDoc(xModel, cName);
  setChartData(cDoc, cName, x, yval, iseries);

  // Force diagram update
  Reference < XComponent > xChart = getChartObjectByName(xModel, cName);
  forceDiagramUpdate(xChart);
} // setChartData()

void setChartData(const Reference < XModel >& xModel, const OUString& cName, const matrix& yval, const unsigned iseries) {
  // Sanity checks
  if (yval.cols() != 1) throw RuntimeException(OU("Vector of Y values can only have one column"), xModel);

  // Find the chart
  Reference < com::sun::star::chart2::XChartDocument > cDoc = getChartDoc(xModel, cName);
  Reference < XChartDataArray > cDataArray = getChartDataArray(cDoc);

  // get old data
  Sequence< Sequence< double > > data = cDataArray->getData();
  unsigned rows = data.getLength();
  if (rows <= 1) throw RuntimeException(OU("Arguments to chart must be vectors or equation and expression"), xModel);
  if (rows != yval.rows()) throw RuntimeException(OU("Arguments to chart must be vectors of equal length"), xModel);

  // // Use existing X-values. Rows are 1 if the chart is empty, see insertChart().
  matrix x(rows, 1);
  for (unsigned r = 0; r < rows; r++) x(r,0) = data[r][0];

  // Fill in the data
  setChartData(cDoc, cName, x, ex_to<matrix>(expression(yval.evalm()).evalf()), iseries);

  // Force diagram update
  Reference < XComponent > xChart = getChartObjectByName(xModel, cName);
  forceDiagramUpdate(xChart);
} // setChartData()

sal_Bool activateOLE(const Reference< XComponent >& xComponent) {
  if (!xComponent.is()) return sal_False;
  Reference < XEmbeddedObjectSupplier2 >xEOS2(xComponent, UNO_QUERY);
  if (!xEOS2.is()) return sal_False;
  Reference < XEmbeddedObject > xEmbObj(xEOS2->getExtendedControlOverEmbeddedObject());
  if ( !xEmbObj.is() ) return sal_False;

  try {
    if (xEmbObj->getCurrentState() == com::sun::star::embed::EmbedStates::LOADED)
      xEmbObj->changeState( com::sun::star::embed::EmbedStates::RUNNING);
    if (xEmbObj->getCurrentState() == com::sun::star::embed::EmbedStates::RUNNING)
      xEmbObj->changeState( com::sun::star::embed::EmbedStates::UI_ACTIVE); // Note: This may block when triggered from XAsyncJob updateLoop()
  } catch ( Exception& ) {
    return sal_False;
  }
  return sal_True;
} // activateOLE()

sal_Bool deactivateOLE(const Reference< XComponent >& xComponent) {
  Reference < XEmbeddedObjectSupplier2 >xEOS2(xComponent, UNO_QUERY);
  if (!xEOS2.is()) return sal_False;
  Reference < XEmbeddedObject > xEmbObj(xEOS2->getExtendedControlOverEmbeddedObject());
  if ( !xEmbObj.is() ) return sal_False;

  try {
    if ((xEmbObj->getCurrentState() == com::sun::star::embed::EmbedStates::UI_ACTIVE) ||
        (xEmbObj->getCurrentState() == com::sun::star::embed::EmbedStates::INPLACE_ACTIVE) ||
        (xEmbObj->getCurrentState() == com::sun::star::embed::EmbedStates::ACTIVE))
      xEmbObj->changeState( com::sun::star::embed::EmbedStates::RUNNING);
    if (xEmbObj->getCurrentState() == com::sun::star::embed::EmbedStates::RUNNING)
      xEmbObj->changeState( com::sun::star::embed::EmbedStates::LOADED);
  } catch ( Exception& ) {
    return sal_False;
  }

  return sal_True;
} // deactivateOLE()

sal_Bool isOLEactivated(const Reference< XComponent >& xComponent) {
  Reference < XEmbeddedObjectSupplier2 >xEOS2(xComponent, UNO_QUERY);
  if (!xEOS2.is()) return sal_False;
  Reference < XEmbeddedObject > xEmbObj(xEOS2->getExtendedControlOverEmbeddedObject());
  if ( !xEmbObj.is() ) return sal_False;
  return (xEmbObj->getCurrentState() == com::sun::star::embed::EmbedStates::UI_ACTIVE);
}

sal_Bool formulaActive(const Reference< XModel >& xModel) {
  iIterator it(xModel, CLSID_FORMULA);
  while (it.next()) if (isOLEactivated(*it)) return sal_True;
  return sal_False;
}

sal_Bool checkIsObject(const Reference < XComponent >& xComponent, const OUString& clsid) {
  MSG_INFO(4, "checkIsObject(XComponent)" << endline);
  Reference < XPropertySet > xPS (xComponent, UNO_QUERY_THROW);
  Any aClsid;

  try {
    aClsid = xPS->getPropertyValue(OU("CLSID"));
  } catch(UnknownPropertyException &e) {
    (void)e;
    return false;
  }

  OUString clsid_str;
  aClsid >>= clsid_str;
  return (clsid_str.equals(clsid));
} // checkIsObject()

// Check if this object is a smath formula
sal_Bool checkIsFormula(const Reference < XComponent >& xComponent) {
  return checkIsObject(xComponent, CLSID_FORMULA);
}

sal_Bool checkIsChart(const Reference < XComponent >& xComponent) {
  return checkIsObject(xComponent, CLSID_CHART);
}

/// Check if this object is an iFormula
sal_Bool checkIsiFormula(const Reference < XComponent >& xComponent) {
  MSG_INFO(2,  "checkIsiFormula(XComponent)"<< endline);
  if (checkIsFormula(xComponent)) { // we have a formula, is it an iFormula?
    Reference< XModel > fModel = extractModel(xComponent);
    OUString fText(getFormulaText(fModel));
    int iipos = fText.indexOfAsciiL("%%ii", 4);
    return (iipos >= 0);
  } else {
    return false;
  }
} // checkIsiFormula

Reference< XModel > extractModel(const Reference< XComponent >& xComponent) {
  Reference < XEmbeddedObjectSupplier > xEOS(xComponent, UNO_QUERY);
  if (xEOS.is())
    return Reference< XModel >(xEOS->getEmbeddedObject(), UNO_QUERY_THROW);

  Reference< XPropertySet > xPS(xComponent, UNO_QUERY_THROW);
  Any fModelAny;
  fModelAny = xPS->getPropertyValue(OU("Model"));
  Reference < XModel > result;
  fModelAny >>= result;
  return result;
}

OUString getFormulaText(const Reference < XModel >& fModel) {
  Reference < XPropertySet > fPS(fModel, UNO_QUERY);
  if (!fPS.is()) return OU(""); // Avoid crash after an Undo action

  // get the formula text
  Any fTextAny;
  fTextAny = fPS->getPropertyValue(OU("Formula"));
  OUString fText;
  fTextAny >>= fText;
  return fText;
}

void setFormulaText(const Reference < XModel > &fModel, const OUString &fText) {
  if (!fModel.is()) return;
  Reference < XPropertySet > fPS(fModel, UNO_QUERY_THROW);

  // set the formula text
  Any fTextAny = makeAny(fText);
  fPS->setPropertyValue(OU("Formula"), fTextAny);
  // Cannot be set earlier because of error "Unknown property"
  fPS->setPropertyValue(OU("IsScaleAllBrackets"), makeAny(true));
} // setFormulaText()

void setFormulaProperty(const Reference < XModel >& fModel, const OUString &propName, const Any& value) {
  Reference< XPropertySet > fPS(fModel, UNO_QUERY);
  if (!fPS.is()) return;
  fPS->setPropertyValue(propName, value);
}

unsigned getFormulaUnsignedProperty(const Reference < XModel >& fModel, const OUString &propName) {
  Reference< XPropertySet > fPS(fModel, UNO_QUERY);
  if (!fPS.is()) return 0; // Avoid crash after an Undo action

  Any aResult = fPS->getPropertyValue(propName);
  long result = 0; // Must use long for the Any cast!
  aResult >>= result;
  return result;
}

OUString getObjectName(const Reference < XComponent > &comp) {
  Reference < XNamed > oName(comp, UNO_QUERY_THROW);
  return oName->getName();
}

void setObjectName(const Reference < XComponent > &comp, const OUString& name) {
  Reference < XNamed > oName(comp, UNO_QUERY_THROW);
  return oName->setName(name);
}

Reference< XChartDocument > getChart(const Reference < XComponent >& xComponent) {
  if (checkIsChart(xComponent)) {
    Reference < XChartDocument > chart(extractModel(xComponent), UNO_QUERY_THROW);
    return chart;
  } else {
    return Reference < XChartDocument>();
  }
}

Reference < XPropertySet > getChartTitleProperties(const Reference< XChartDocument >& chart) {
  Reference < XTitled > xTitled(chart, UNO_QUERY_THROW);
  Reference <com::sun::star::chart2::XTitle> title = xTitled->getTitleObject();
  if (!title.is()) return Reference < XPropertySet >();
  Sequence< Reference < XFormattedString > > titleText = title->getText();
  Reference < XFormattedString > titleTextPortion = titleText[0]; // No point iterating because UI only allows one font for the title
  Reference < XPropertySet > titleTextProps(titleTextPortion, UNO_QUERY_THROW);
  return titleTextProps;
}

Reference< XPropertySet > getDiagramTitleProperties(const Reference< com::sun::star::chart2::XDiagram >& diagram) {
  Reference < XTitled > xTitled(diagram, UNO_QUERY_THROW);
  Reference <com::sun::star::chart2::XTitle> title = xTitled->getTitleObject();
  if (!title.is()) return Reference < XPropertySet >();
  Sequence< Reference < XFormattedString > > titleText = title->getText();
  Reference < XFormattedString > titleTextPortion = titleText[0]; // No point iterating because UI only allows one font for the title
  Reference < XPropertySet > titleTextProps(titleTextPortion, UNO_QUERY_THROW);
  return titleTextProps;
}

Reference< XPropertySet > getDiagramLegendProperties(const Reference< com::sun::star::chart2::XDiagram >& diagram) {
  Reference < XLegend > legend = diagram->getLegend();
  if (!legend.is()) return Reference< XPropertySet >();
  Reference < XPropertySet > legendTextProps(legend, UNO_QUERY_THROW);
  return legendTextProps;
}

Sequence< Sequence< Sequence< Reference < XPropertySet > > > > getDiagramAxesTitleProperties(const Reference< com::sun::star::chart2::XDiagram >& diagram) {
  Reference< XCoordinateSystemContainer > xCoordCnt(diagram, UNO_QUERY_THROW);
  Sequence< Reference< XCoordinateSystem> > cSystems = xCoordCnt->getCoordinateSystems();
  Sequence< Sequence< Sequence< Reference < XPropertySet > > > > result_csystems(cSystems.getLength());

  for (int c = 0; c < cSystems.getLength(); ++c) {
    Sequence< Sequence< Reference < XPropertySet > > > result_dimensions(cSystems[c]->getDimension());
    for (int d = 0; d < cSystems[c]->getDimension(); ++d) {
      Sequence< Reference < XPropertySet > > result_axes(cSystems[c]->getMaximumAxisIndexByDimension(d));
      for (int n = 0; n < cSystems[c]->getMaximumAxisIndexByDimension(d); ++n) {
        Reference< XAxis > axis = cSystems[c]->getAxisByDimension(d, n);
        if (axis.is()) {
          Reference < XTitled > xTitled(axis, UNO_QUERY_THROW);
          Reference <com::sun::star::chart2::XTitle> title = xTitled->getTitleObject();
          if (!title.is()) {
            result_axes[n] = Reference < XPropertySet >();
          } else {
            Sequence< Reference < XFormattedString > > titleText = title->getText();
            Reference < XFormattedString > titleTextPortion = titleText[0]; // No point iterating because UI only allows one font for the title
            Reference < XPropertySet > titleTextProps(titleTextPortion, UNO_QUERY_THROW);
            result_axes[n] = titleTextProps;
          }
        }
      }
      result_dimensions[d] = result_axes;
    }
    result_csystems[c] = result_dimensions;
  }

  return result_csystems;
}

Sequence< Sequence< Sequence< Reference < XPropertySet > > > > getDiagramAxesProperties(const Reference< com::sun::star::chart2::XDiagram >& diagram) {
  Reference< XCoordinateSystemContainer > xCoordCnt(diagram, UNO_QUERY_THROW);
  Sequence< Reference< XCoordinateSystem> > cSystems = xCoordCnt->getCoordinateSystems();
  Sequence< Sequence< Sequence< Reference < XPropertySet > > > > result_csystems(cSystems.getLength());

  for (int c = 0; c < cSystems.getLength(); ++c) {
    Sequence< Sequence< Reference < XPropertySet > > > result_dimensions(cSystems[c]->getDimension());
    for (int d = 0; d < cSystems[c]->getDimension(); ++d) {
      Sequence< Reference < XPropertySet > > result_axes(cSystems[c]->getMaximumAxisIndexByDimension(d));
      for (int n = 0; n < cSystems[c]->getMaximumAxisIndexByDimension(d); ++n) {
        Reference< XAxis > axis = cSystems[c]->getAxisByDimension(d, n);
        if (axis.is()) {
          Reference < XPropertySet > axisProps(axis, UNO_QUERY_THROW);
          result_axes[n] = axisProps;
        }
      }
      result_dimensions[d] = result_axes;
    }
    result_csystems[c] = result_dimensions;
  }

  return result_csystems;
}

void setDiagramAxesTitleProperties(Reference< com::sun::star::chart2::XDiagram >& diagram, const Sequence< Sequence< Sequence< Reference < XPropertySet > > > >& props) {
  Reference< XCoordinateSystemContainer > xCoordCnt(diagram, UNO_QUERY_THROW);
  Sequence< Reference< XCoordinateSystem> > cSystems = xCoordCnt->getCoordinateSystems();

  for (int c = 0; (c < cSystems.getLength()) && (c < props.getLength()); ++c) {
    const Sequence< Sequence< Reference < XPropertySet > > >& dimensions = props[c];
    for (int d = 0; (d < cSystems[c]->getDimension()) && (d < dimensions.getLength()); ++d) {
      Sequence< Reference < XPropertySet > > axes = dimensions[d];
      for (int n = 0; (n < cSystems[c]->getMaximumAxisIndexByDimension(d)) && (n < axes.getLength()); ++n) {
        Reference< XAxis > axis = cSystems[c]->getAxisByDimension(d, n);
        if (axis.is()) {
          Reference < XTitled > xTitled(axis, UNO_QUERY_THROW);
          Reference <com::sun::star::chart2::XTitle> title = xTitled->getTitleObject();

          if (title.is()) {
            Sequence< Reference < XFormattedString > > titleText = title->getText();
            Reference < XFormattedString > titleTextPortion = titleText[0]; // No point iterating because UI only allows one font for the title
            Reference < XPropertySet > titleTextProps(titleTextPortion, UNO_QUERY_THROW);
            titleTextProps->setPropertyValue(OU("CharFontName"), axes[n]->getPropertyValue(OU("CharFontName")));
          }
        }
      }
    }
  }
}

void setDiagramAxesProperties(Reference< com::sun::star::chart2::XDiagram >& diagram, const Sequence< Sequence< Sequence< Reference < XPropertySet > > > >& props) {
  Reference< XCoordinateSystemContainer > xCoordCnt(diagram, UNO_QUERY_THROW);
  Sequence< Reference< XCoordinateSystem> > cSystems = xCoordCnt->getCoordinateSystems();

  for (int c = 0; (c < cSystems.getLength()) && (c < props.getLength()); ++c) {
    const Sequence< Sequence< Reference < XPropertySet > > >& dimensions = props[c];
    for (int d = 0; (d < cSystems[c]->getDimension()) && (d < dimensions.getLength()); ++d) {
      Sequence< Reference < XPropertySet > > axes = dimensions[d];
      for (int n = 0; (n < cSystems[c]->getMaximumAxisIndexByDimension(d)) && (n < axes.getLength()); ++n) {
        Reference< XAxis > axis = cSystems[c]->getAxisByDimension(d, n);

        if (axis.is()) {
          Reference < XPropertySet > axisProps(axis, UNO_QUERY_THROW);
          axisProps->setPropertyValue(OU("CharFontName"), axes[n]->getPropertyValue(OU("CharFontName")));
        }
      }
    }
  }
}

void toggleTextMode(const Reference< XComponent >& f) {
  Reference< XModel > fModel = extractModel(f);
  Reference< XPropertySet > fPS(fModel, UNO_QUERY);
  if (!fPS.is()) return;

  Any aOldValue = fPS->getPropertyValue(OU("IsTextMode"));
  bool oldValue = false;
  aOldValue >>= oldValue;
  fPS->setPropertyValue(OU("IsTextMode"), makeAny(!oldValue));
}

void copyProperties(const Reference< XComponent >& source, const Reference< XComponent >& target) {
  Reference< XPropertySet > sPS(extractModel(source), UNO_QUERY);
  if (!sPS.is()) return; // Avoid crash after an Undo action

  Reference< XPropertySet > tPS(extractModel(target), UNO_QUERY);
  if (!tPS.is()) return;

  // Formula properties, available in the "Format" menu when the formula editor is active
  Sequence< com::sun::star::beans::Property > properties(sPS->getPropertySetInfo()->getProperties());
  for (int p = 0; p < properties.getLength(); ++p) {
    OUString pName = properties[p].Name;
    if (!pName.equalsAscii("Formula") && !pName.equalsAscii("IsTextMode")) {
      try {
        tPS->setPropertyValue(pName, sPS->getPropertyValue(pName));
      } catch(const Exception& e) { (void)e; /* Unknown property */ }
    }
  }

  // Embedded object properties, available from the context menu entry "Object"
  sPS = Reference< XPropertySet > (source, UNO_QUERY);
  tPS = Reference< XPropertySet > (target, UNO_QUERY);
  try {
    tPS->setPropertyValue(OU("LeftMargin"),  sPS->getPropertyValue(OU("LeftMargin")));
    tPS->setPropertyValue(OU("RightMargin"), sPS->getPropertyValue(OU("RightMargin")));
  } catch(const Exception& e) {  (void)e; /* Unknown property, e.g. for Presentation */ }
}

Reference < XComponent > insertFormula(const Reference < XModel > &xModel) {
  return insertObject(xModel, CLSID_FORMULA);
}

void deleteFormula(const Reference < XModel > &xModel, const Reference < XComponent >& iFormula) {
  if (docType(xModel).equalsAscii("TextDocument")) {
    deactivateOLE(iFormula); // Why is this necessary??? (started being necessary in LibreOffice 3.3)
    Reference < XTextCursor > xModelCursor;
    Reference < XText > xDocumentText = getDocumentText(xModel->getCurrentController(), xModelCursor);
    xDocumentText->removeTextContent(Reference< XTextContent>(iFormula, UNO_QUERY_THROW));
  } else if (docType(xModel).equalsAscii("Presentation")) {
    iIterator it(xModel);

    while (it.next()) {
      if (getObjectName(*it).equals(getObjectName(iFormula))) {
        Reference< XShapes > xShapes = it.getCurrentShapes();
        xShapes->remove(Reference< XShape >(iFormula, UNO_QUERY_THROW));
        break;
      }
    }
  }
} // deleteFormula()

Reference < XComponent > getFormulaByName(const Reference < XNameAccess > &embeddedObjects, const OUString &iFormulaName) {
  try {
    Any iFormulaEmbedded = embeddedObjects->getByName(iFormulaName);
    Reference < XComponent > iFormula;
    iFormulaEmbedded >>= iFormula;
    if (!checkIsFormula(iFormula))
      return Reference< XComponent >();
    return iFormula;
  } catch (const Exception&) {
    return Reference< XComponent >();
  }
} // getFormulaByName()

Reference < XComponent > getObjectByName(const Reference < XNameAccess > &embeddedObjects, const OUString &objectName) {
  try {
    Any object = embeddedObjects->getByName(objectName);
    Reference< XComponent > xComponent;
    object >>= xComponent;
    return xComponent;
  } catch (const Exception&) {
    return Reference< XComponent >();
  }
}

Reference < XComponent > getObjectByName(const Reference< XIndexAccess > &xDrawPages, const OUString &objectName) {
  for (int idx = 0; idx < xDrawPages->getCount(); ++idx) {
    Reference< XDrawPage > page(xDrawPages->getByIndex(idx), UNO_QUERY_THROW);
    Reference< XIndexAccess > shapes(page, UNO_QUERY_THROW);

    for (int sh = 0; sh < shapes->getCount(); ++sh) {
      Reference< XServiceInfo > xSI(shapes->getByIndex(sh), UNO_QUERY_THROW);

      if (xSI.is() && xSI->supportsService(OU("com.sun.star.drawing.OLE2Shape"))) {
        Reference< XComponent > comp(shapes->getByIndex(sh), UNO_QUERY_THROW);

        if (getObjectName(comp).equals(objectName))
          return comp;
      }
    }
  }

  return Reference< XComponent >();
}

Reference < XComponent > getObjectByName(const Reference < XModel >& xModel, const OUString &objectName) {
  if (docType(xModel).equalsAscii("TextDocument")) {
    Reference < XTextEmbeddedObjectsSupplier > xTEOS(xModel, UNO_QUERY_THROW);
    return getObjectByName(xTEOS->getEmbeddedObjects(), objectName);
  } else {
    Reference< XDrawPagesSupplier > xPresDoc(xModel, UNO_QUERY_THROW);
    Reference< XIndexAccess > xDrawPages(xPresDoc->getDrawPages(), UNO_QUERY_THROW);
    return getObjectByName(xDrawPages, objectName);
  }
} // getObjectByName()

OUString getInterText(const Reference<XTextContent>& f1, const Reference<XTextContent>& f2) {
  Reference< XText > xDocumentText = f2->getAnchor()->getText();
  Reference< XParagraphCursor > xCursor(xDocumentText->createTextCursorByRange(f1->getAnchor()->getEnd()), UNO_QUERY_THROW);
  xCursor->gotoRange(f2->getAnchor()->getEnd(), true);
  return xCursor->getString();
}

Reference< XHierarchicalPropertySet > getRegistryAccess(const Reference< XComponentContext >& mxCC, const OUString& nodepath) {
  // Get access to the registry node which contains the data for our option page using the nodepath
  Reference< ::com::sun::star::lang::XMultiComponentFactory > xMCF = mxCC->getServiceManager(); // The mxMCF of the component is undefined here, why?
  Reference< XMultiServiceFactory > xConfig = Reference< XMultiServiceFactory >(xMCF->createInstanceWithContext(
    OU("com.sun.star.configuration.ConfigurationProvider"), mxCC), UNO_QUERY_THROW);
  Sequence< Any > args(1);
  PropertyValue path;
  path.Name = OU("nodepath");
  path.Value = makeAny(nodepath);
  args[0] = makeAny(path);
  Reference< XHierarchicalPropertySet > xProperties;
  xProperties = Reference< XHierarchicalPropertySet >(xConfig->createInstanceWithArguments(
      OU("com.sun.star.configuration.ConfigurationUpdateAccess"), args), UNO_QUERY_THROW);

  return xProperties;
} // getRegistryAccess()

// Change number of cached inline objects
void setInlineCache(const Reference< XComponentContext >& mxCC, const sal_Int32 num) {
  Reference< XHierarchicalPropertySet > xProperties = getRegistryAccess(mxCC, OU("/org.openoffice.Office.Common/"));
  xProperties->setHierarchicalPropertyValue(OU("Cache/Writer/OLE_Objects"), makeAny(num));
  Reference< XChangesBatch > xUpdateCommit(xProperties, UNO_QUERY_THROW);
  xUpdateCommit->commitChanges();
} // setInlineCache()

// Get number of cacheable inline objects
sal_Int32 getInlineCache(const Reference< XComponentContext >& mxCC) {
  Reference< XHierarchicalPropertySet > xProperties = getRegistryAccess(mxCC, OU("/org.openoffice.Office.Common/"));

  Any anyCacheSize = xProperties->getHierarchicalPropertyValue(OU("Cache/Writer/OLE_Objects"));
  sal_Int32 cacheSize = 0;
  anyCacheSize >>= cacheSize;
  return cacheSize;
} // getInlineCache()

bool hasControl(const Reference < XControlContainer >& xControlContainer, const OUString& controlName) {
  try {
    Reference< XControl > xControl = xControlContainer->getControl(controlName);
    return xControl.is();
  } catch (Exception& e) {
    (void)e;
    return false;
  }
}

// get selections of list box control
Sequence<OUString> getListBoxSelections(const Reference < XControlContainer >& xControlContainer, const OUString& controlName) {
  try {
    Reference< XControl > xControl = xControlContainer->getControl(controlName);
    Reference< XListBox > xListBox(xControl, UNO_QUERY_THROW);
    return xListBox->getSelectedItems();
  } catch (Exception& e) {
    (void)e;
    return Sequence<OUString>();
  }
} // getListBoxSelections()

OUString getFirstListBoxSelection(const Reference < XControlContainer >& xControlContainer, const OUString& controlName) {
  Sequence<OUString> selections = getListBoxSelections(xControlContainer, controlName);
  if (selections.getLength() == 0)
    return OU("");
  else
    return selections[0];
}

Sequence<short> getListBoxSelectionsPos(const Reference < XControlContainer >& xControlContainer, const OUString& controlName) {
  try {
    Reference< XControl > xControl = xControlContainer->getControl(controlName);
    Reference< XListBox > xListBox(xControl, UNO_QUERY_THROW);
    return xListBox->getSelectedItemsPos();
  } catch (Exception& e) {
    (void)e;
    return Sequence<short>();
  }
} // getListBoxSelectionsPos()

void setListBox(const Reference< XControlContainer >& xContainer, const OUString& controlName, const OUString& itemName) {
  Reference< XControl > xControl = xContainer->getControl(controlName);
  if (xControl == NULL) return;
  Reference< XListBox > xListBox(xControl, UNO_QUERY_THROW);
  xListBox->selectItem(itemName, true);
  xListBox->makeVisible(xListBox->getSelectedItemPos());
} // setListBox

void setListBox(const Reference< XControlContainer >& xContainer, const OUString& controlName, const short itemPos) {
  Reference< XControl > xControl = xContainer->getControl(controlName);
  if (xControl == NULL) return;
  Reference< XListBox > xListBox(xControl, UNO_QUERY_THROW);
  xListBox->selectItemPos(itemPos, true);
  xListBox->makeVisible(xListBox->getSelectedItemPos());
}

void setListBox(const Reference< XControlContainer >& xContainer, const OUString& controlName, const std::list<OUString>& itemNames) {
  Reference< XControl > xControl = xContainer->getControl(controlName);
  if (xControl == NULL) return;
  Reference< XListBox > xListBox(xControl, UNO_QUERY_THROW);
  for (const auto& i : itemNames)
    xListBox->selectItem(i, true);
  xListBox->makeVisible(xListBox->getSelectedItemPos());
}

void setListBox(const Reference< XControlContainer >& xContainer, const OUString& controlName, const Sequence<short>& itemPos) {
  Reference< XControl > xControl = xContainer->getControl(controlName);
  if (xControl == NULL) return;
  Reference< XListBox > xListBox(xControl, UNO_QUERY_THROW);
  xListBox->selectItemsPos(itemPos, true);
  xListBox->makeVisible(xListBox->getSelectedItemPos());
}

void deselectListBox(const Reference< XControlContainer >& xContainer, const OUString& controlName) {
  Reference< XControl > xControl = xContainer->getControl(controlName);
  if (xControl == NULL) return;
  Reference< XListBox > xListBox(xControl, UNO_QUERY_THROW);
  for (int i = 0; i < xListBox->getItemCount(); ++i)
    xListBox->selectItemPos(i, false);
}

// get value of text control
OUString getTextcontrol (const Reference < XControlContainer >& xControlContainer, const OUString& controlName) {
  try {
    Reference< XControl > xControl = xControlContainer->getControl(controlName);
    Reference< XTextComponent > xTextComponent(xControl, UNO_QUERY_THROW);
    return xTextComponent->getText();
  } catch (Exception& e) {
    (void)e;
    return OUString::createFromAscii("");
  }
} // getTextcontrol

void setTextcontrol(const Reference< XControlContainer >& xContainer, const OUString& controlName, const OUString& text) {
  Any aText;
  aText <<= text;
  setTextcontrol(xContainer, controlName, aText);
}

void setTextcontrol(const Reference< XControlContainer >& xContainer, const OUString& controlName, const Any& value) {
  Reference< XControl > xControl = xContainer->getControl(controlName);
  if (xControl == NULL) return;
  Reference< XPropertySet > xProp(xControl->getModel(), UNO_QUERY_THROW);
  xProp->setPropertyValue(OUString::createFromAscii("Text"), value);
} // setTextcontrol

void setLabelcontrol(const Reference< XControlContainer >& xContainer, const OUString& controlName, const OUString& text) {
  Any aText;
  aText <<= text;
  setLabelcontrol(xContainer, controlName, aText);
}

void setLabelcontrol(const Reference< XControlContainer >& xContainer, const OUString& controlName, const Any& value) {
  Reference< XControl > xControl = xContainer->getControl(controlName);
  if (xControl == NULL) return;
  Reference< XPropertySet > xProp(xControl->getModel(), UNO_QUERY_THROW);
  xProp->setPropertyValue(OUString::createFromAscii("Label"), value);
} // setTextcontrol

// get value of radio button
sal_Bool getRadioButton(const Reference < XControlContainer >& xControlContainer, const OUString& buttonName) {
  try {
    Reference< XControl > xControl = xControlContainer->getControl(buttonName);
    Reference< XRadioButton > xRadioButton(xControl,UNO_QUERY_THROW);
    return xRadioButton->getState();
  } catch (Exception& e) {
    (void)e;
    return sal_False;
  }
} // getRadioButton

void setRadioButton(const Reference< XControlContainer >& xContainer, const OUString& controlName, const sal_Bool value) {
  Reference< XControl > xControl = xContainer->getControl(controlName);
  if (xControl == NULL) return;
  Reference< XRadioButton > xRadioButton(xControl,UNO_QUERY_THROW);
  xRadioButton->setState(value);
} // setRadiobutton()

void setCheckBox(const Reference< XControlContainer >& xContainer, const OUString& controlName, const sal_Bool value) {
  Reference< XControl > xControl = xContainer->getControl(controlName);
  if (xControl == NULL) return;
  Reference< XCheckBox > xCheckBox(xControl,UNO_QUERY_THROW);
  xCheckBox->setState(value);
} // setCheckBox()

sal_Int32 getCheckBox(const Reference < XControlContainer >& xControlContainer, const OUString& controlName) {
  try {
    Reference< XControl > xControl = xControlContainer->getControl(controlName);
    Reference< XCheckBox > xCheckBox(xControl,UNO_QUERY_THROW);
    return xCheckBox->getState();
  } catch (Exception& e) {
    (void)e;
    return -1;
  }
} // getCheckBox

// extract value of numeric field
sal_Int32 getNumericFieldPosInt(const Reference < XControlContainer >& xControlContainer, const OUString& controlName) {
  try {
    Reference< XControl > xControl = xControlContainer->getControl(controlName);
    Reference< XNumericField > xNumericField(xControl,UNO_QUERY_THROW);
    return std::lround(xNumericField->getValue());
  } catch (Exception& e) {
    (void)e;
    return -1; // So that error can be recognized
  }
} // getNumericFieldPosInt()

// extract value of numeric field
sal_Int32 getNumericFieldInt(const Reference < XControlContainer >& xControlContainer, const OUString& controlName) {
  Reference< XControl > xControl = xControlContainer->getControl(controlName);
  Reference< XNumericField > xNumericField(xControl,UNO_QUERY_THROW);
  return std::lround(xNumericField->getValue());
} // getNumericFieldInt()

void setNumericFieldPosInt(const Reference< XControlContainer >& xContainer, const OUString& controlName, const sal_uInt32 value) {
  Reference< XControl > xControl = xContainer->getControl(controlName);
  if (xControl == NULL) return;
  Reference< XNumericField > xNumericField(xControl,UNO_QUERY_THROW);
  return xNumericField->setValue(value);
} // setNumericFieldPosInt()

void setNumericFieldInt(const Reference< XControlContainer >& xContainer, const OUString& controlName, const sal_Int32 value) {
  Reference< XControl > xControl = xContainer->getControl(controlName);
  if (xControl == NULL) return;
  Reference< XNumericField > xNumericField(xControl,UNO_QUERY_THROW);
  return xNumericField->setValue(value);
} // setNumericFieldInt()

void showControl(const Reference< XControlContainer >& xContainer, const OUString& controlName, const bool show) {
  MSG_INFO(3, "Show/Hide control " << STR(controlName) << endline);
  Reference< XWindow > control(xContainer->getControl(controlName), UNO_QUERY_THROW);
  control->setVisible(show);
}

void enableControl(const Reference< XControlContainer >& xContainer, const OUString& controlName, const bool enable) {
  MSG_INFO(3, "Enable/Disable control " << STR(controlName) << endline);
  Reference< XWindow > control(xContainer->getControl(controlName), UNO_QUERY);
  control->setEnable(enable);
}

void moveControlRel(const Reference< XControlContainer >& xContainer, const OUString& controlName, const int deltaX, const int deltaY) {
  Reference< XWindow > control(xContainer->getControl(controlName), UNO_QUERY_THROW);
  com::sun::star::awt::Rectangle controlPosSize = control->getPosSize();
  control->setPosSize(controlPosSize.X + deltaX, controlPosSize.Y + deltaY, controlPosSize.Width, controlPosSize.Height, 3);
}

  // Checks if the name property of the window is one of the supported names and returns
  // always a valid string or null
OUString getWindowName(const Reference< XWindow >& aWindow) {
  if (!aWindow.is())
    throw Exception(OUString::createFromAscii("Method getWindowName requires that a window is passed as argument"), aWindow);

  //We need to get the control model of the window. Therefore the first step is
  //to query for it.
  Reference< XControl > xControlDlg(aWindow, UNO_QUERY_THROW);

 //Now get model
  Reference< XControlModel > xModelDlg = xControlDlg->getModel();
  if (!xModelDlg.is())
    throw Exception(OUString::createFromAscii("Cannot obtain XControlModel from XWindow in method getWindowName."), aWindow);

  //The model itself does not provide any information except that its
  //implementation supports XPropertySet which is used to access the data.
  Reference< XPropertySet > xPropDlg(xModelDlg, UNO_QUERY_THROW);

  //Get the "Name" property of the window
  Any aWindowName = xPropDlg->getPropertyValue(OUString::createFromAscii("Name"));

  //Get the string from the returned com.sun.star.uno.Any
  OUString sName;
  aWindowName >>= sName;
  return sName;
} // getWindowName()

sal_Bool propertyIs(const Reference< XPropertyContainer >& xUserPropsContainer, const OUString& propName) {
  try {
    Reference< XPropertySet > xUserProps(xUserPropsContainer, UNO_QUERY_THROW);
    Any test = xUserProps->getPropertyValue(propName);
    (void)test;
  } catch (Exception &e) {
    (void)e;
    return sal_False;
  }
  return sal_True;
} // propertyIs()

OUString getTextProperty(const Reference< XPropertyContainer >& xUserPropsContainer, const OUString& propName) {
  // We assume that it has been tested that this property exists
  Reference< XPropertySet > xUserProps(xUserPropsContainer, UNO_QUERY_THROW);
  Any Avalue = xUserProps->getPropertyValue(propName);
  OUString value(OU(""));
  Avalue >>= value;
  return value;
} // getTextProperty()

sal_Bool getBoolProperty(const Reference< XPropertyContainer >& xUserPropsContainer, const OUString& propName) {
  // We assume that it has been tested that this property exists
  Reference< XPropertySet > xUserProps(xUserPropsContainer, UNO_QUERY_THROW);
  Any Avalue = xUserProps->getPropertyValue(propName);
  sal_Bool value(sal_True);
  Avalue >>= value;
  return value;
} // getBoolProperty()

#if (OO_MAJOR_VERSION == 3) && (OO_MINOR_VERSION <= 5) || (OO_IS_AOO == 1)
sal_Int64 getPosIntProperty(const Reference< XPropertyContainer >& xUserPropsContainer, const OUString& propName) {
  return floor(getNumberProperty(xUserPropsContainer, propName));
}
#else
sal_uInt32 getPosIntProperty(const Reference< XPropertyContainer >& xUserPropsContainer, const OUString& propName) {
  return std::lround(getNumberProperty(xUserPropsContainer, propName));
} // getPosIntProperty()
#endif

sal_Int32 getIntProperty(const Reference< XPropertyContainer >& xUserPropsContainer, const OUString& propName) {
  return std::lround(getNumberProperty(xUserPropsContainer, propName));
} // getPosIntProperty()

double getNumberProperty(const Reference< XPropertyContainer >& xUserPropsContainer, const OUString& propName) {
  // We assume that it has been tested that this property exists
  Reference< XPropertySet > xUserProps(xUserPropsContainer, UNO_QUERY_THROW);
  Any Avalue = xUserProps->getPropertyValue(propName);
  double value = 0.0;
  Avalue >>= value;
  return value;
} // getNumberProperty()

sal_Bool propertyIs(const Reference< XHierarchicalPropertySet >& xProperties, const OUString& propName) {
  try {
    Any test = xProperties->getHierarchicalPropertyValue(propName);
    (void)test;
  } catch (Exception &e) {
    (void)e;
    return sal_False;
  }
  return sal_True;
} // propertyIs()

OUString getTextProperty(const Reference< XComponentContext >& mxCC, const Reference<XModel>& xModel,
                               const Reference<XNamedGraph>& xGraph, const Reference< XHierarchicalPropertySet >& xProperties,
                               const OUString& userPropName, const OUString& propName) {
  OUString value;
  if (!hasStatement(mxCC, xModel, xGraph, userPropName)) {
    Any Avalue = xProperties->getHierarchicalPropertyValue(propName);
    Avalue >>= value;
    addStatement(mxCC, xModel, xGraph, userPropName, value);
  } else {
    value = getStatementString(mxCC, xModel, xGraph, userPropName);
  }
  return value;
} // getTextProperty()

sal_Bool getBoolProperty(const Reference< XComponentContext >& mxCC, const Reference<XModel>& xModel,
                               const Reference<XNamedGraph>& xGraph, const Reference< XHierarchicalPropertySet >& xProperties,
                               const OUString& userPropName, const OUString& propName) {
  sal_Bool value = false; // Initialize to get rid of compiler warning
  if (!hasStatement(mxCC, xModel, xGraph, userPropName)) {
    Any Avalue = xProperties->getHierarchicalPropertyValue(propName);
    Avalue >>= value;
    addStatement(mxCC, xModel, xGraph, userPropName, OU(value ? "true" : "false"));
  } else {
    value = getStatementBool(mxCC, xModel, xGraph, userPropName);
  }
  return value;
} // getBoolProperty()

#if (OO_MAJOR_VERSION == 3) && (OO_MINOR_VERSION <= 5) || (OO_IS_AOO == 1)
sal_Int64 getPosIntProperty(const Reference< XComponentContext >& mxCC, const Reference<XModel>& xModel,
                                   const Reference<XNamedGraph>& xGraph, const Reference< XHierarchicalPropertySet >& xProperties,
                                   const OUString& userPropName, const OUString& propName) {
  sal_Int64 value = 0;
  if (!hasStatement(mxCC, xModel, xGraph, userPropName)) {
    Any Avalue = xProperties->getHierarchicalPropertyValue(propName);
    Avalue >>= value;
    addStatement(mxCC, xModel, xGraph, userPropName, OUSTRINGNUMBER(value));
  } else {
    value = getStatementPosInt(mxCC, xModel, xGraph, userPropName);
  }
  return value;
} // getPosIntProperty()
#else
sal_uInt32 getPosIntProperty(const Reference< XComponentContext >& mxCC, const Reference<XModel>& xModel,
                                   const Reference<XNamedGraph>& xGraph, const Reference< XHierarchicalPropertySet >& xProperties,
                                   const OUString& userPropName, const OUString& propName) {
  long value = 0; // Must use long for the Any cast!
  if (!hasStatement(mxCC, xModel, xGraph, userPropName)) {
    Any Avalue = xProperties->getHierarchicalPropertyValue(propName);
    Avalue >>= value;
    addStatement(mxCC, xModel, xGraph, userPropName, OUSTRINGNUMBER(value));
  } else {
    value = getStatementPosInt(mxCC, xModel, xGraph, userPropName);
  }
  return value;
} // getPosIntProperty()
#endif

sal_Int32 getIntProperty(const Reference< XComponentContext >& mxCC, const Reference<XModel>& xModel,
                               const Reference<XNamedGraph>& xGraph, const Reference< XHierarchicalPropertySet >& xProperties,
                               const OUString& userPropName, const OUString& propName) {
  sal_Int32 value = 0;
  if (!hasStatement(mxCC, xModel, xGraph, userPropName)) {
    Any Avalue = xProperties->getHierarchicalPropertyValue(propName);
    Avalue >>= value;
    addStatement(mxCC, xModel, xGraph, userPropName, OUSTRINGNUMBER(value));
  } else {
    value = getStatementInt(mxCC, xModel, xGraph, userPropName);
  }
  return value;
} // getPosIntProperty()

OUString extractReferences(const Reference< XControlContainer >& xContainer) {
  std::map<std::string, OUString> refmap; // This automatically orders by key
  Sequence< Reference< XControl > > allControls = xContainer->getControls   ();
  for (sal_Int32 i = 0; i < allControls.getLength(); i++) {
    Reference< XControlModel > cModel = allControls[i]->getModel();
    Reference< XPropertySet >  cProps(cModel, UNO_QUERY_THROW);
    Any aName = cProps->getPropertyValue(OU("Name"));
    OUString cName;
    aName >>= cName;
    sal_Bool isChecked = getCheckBox(xContainer, cName) == 1;
    if (isChecked && (cName.indexOfAsciiL("O_", 2) == 0) && (cName.getLength() > 4))
      refmap[STR(cName.copy(2,2))] = cName.copy(2);
  }
  OUString references = OU("");
  for (const auto& r : refmap)
    references = references + OU(" ") + r.second;
  MSG_INFO(0, "Found references '" << STR(references) << "'" << endline);
  return references;
}

void setReferences(const Reference< XControlContainer >& xContainer, const OUString& references) {
  std::list<OUString> files = splitString(references, ' ');

  for (const auto& f : files) {
    setCheckBox(xContainer, OU("O_") + f, true);
    MSG_INFO(0, "Set reference '" << STR(f) << "'" << endline);
  }
}

bool checkTextmodeFormula(const Reference< XTextContent >& (formula)) {
  if (!formula.is()) return false;

  Reference< XText > xDocumentText = formula->getAnchor()->getText();
  Reference< XParagraphCursor > xCursor(xDocumentText->createTextCursorByRange(formula->getAnchor()->getEnd()), UNO_QUERY_THROW);

  // Look for non-whitespace to the left and right of the anchor point, inside the paragraph where the anchor point is
  xCursor->gotoStartOfParagraph(true);
  if (xCursor->getString().trim().getLength() > 0) return true; // Text exists before the formula
  xCursor->gotoEndOfParagraph (true);
  return (xCursor->getString().trim().getLength() > 0);
}

// Iterate through all formulas of the document IN THEIR TEXTUAL ORDER. The list supplied by getEmbeddedObjects() is only in this order after
// loading a document, before the user modifies the order of the elements (closing and re-opening a document seems to re-order the elements in textual order)
// This returns a list of formula labels which allows the formulas to be accessed by XNameAccess from getEmbeddedObjects() when required
void orderXText(const Reference<XText>& xText, std::list< OUString >& formulas, unsigned& count, const Reference < XStatusIndicator >& xStatus) {
  Reference< XEnumerationAccess > xEnum(xText, UNO_QUERY_THROW);
  Reference< XEnumeration > xParaEnum = xEnum->createEnumeration();
  //MSG_INFO(3,  "orderXText called" << endline);

  while (xParaEnum->hasMoreElements()) { //iterate through all paragraphs of the document
     Any paragraph = xParaEnum->nextElement();
     //MSG_INFO(3,  "Paragraph loop" << endline);

     // Check what the "paragraph" actually is
     Reference< XServiceInfo > xInfo;
     paragraph >>= xInfo;
     if (xInfo->supportsService(OU("com.sun.star.text.TextTable"))) { // paragraph is really a table
       Reference< XTextTable > xTable;
       paragraph >>= xTable;
       Sequence< OUString > cellnames = xTable->getCellNames();

       for (int i = 0; i < cellnames.getLength(); i++) { // Iterate through all cells of the table
         //MSG_INFO(3,  "Table cells loop: " << STR(cellnames[i]) << endline);
         Reference< XText > xTableText(xTable->getCellByName(cellnames[i]), UNO_QUERY_THROW);
         orderXText(xTableText, formulas, count, xStatus);
       }
       continue; // skip the rest, it is for paragraphs only
     }

     Reference< XEnumerationAccess > xParaEnumAccess;
     paragraph >>= xParaEnumAccess;
     Reference< XEnumeration > xPortionsEnum = xParaEnumAccess->createEnumeration();

     while (xPortionsEnum->hasMoreElements()) { // iterate through all text portions of the paragraph
       //MSG_INFO(3,  "Paragraph text portion loop" << endline);
       Any portion = xPortionsEnum->nextElement();
       Reference < XPropertySet > xPS;
       portion >>= xPS;
       Any portionType = xPS->getPropertyValue(OU("TextPortionType"));
       OUString pTypeStr;
       portionType >>= pTypeStr;
       //MSG_INFO(3,  "This is a " << STR(pTypeStr) << endline);

       if (pTypeStr == OU("Footnote")) {
         Any anyFootnote = xPS->getPropertyValue(OU("Footnote"));
         Reference< XFootnote > xF;
         anyFootnote >>= xF;
         Reference< XText > xFootnote;
         anyFootnote >>= xFootnote;
         orderXText(xFootnote, formulas, count, xStatus);
       } else if ((pTypeStr == OU("TextContent")) || (pTypeStr == OU("Frame"))) {
         count++;

         if (xStatus.is()) xStatus->setValue(count);

         Reference< XContentEnumerationAccess > xContentEnumAccess;
         portion >>= xContentEnumAccess;
         Reference< XEnumeration > xContentEnum = xContentEnumAccess->createContentEnumeration(OU("TextContent"));

         while (xContentEnum->hasMoreElements()) { // iterate through all text contents of the text portion
           //MSG_INFO(3,  "Text portion text content loop" << endline);
           Any tc = xContentEnum->nextElement();
           Reference< XComponent > comp;
           tc >>= comp;
           //MSG_INFO(3,  "Name: " << STR(getObjectName(xTC)) << endline);

           if (checkIsiFormula(comp)) {
             //MSG_INFO(3,  "Found formula: " << STR(getObjectName(xTC)) << endline);
             formulas.emplace_back(getObjectName(comp));
           }
         }
       }
     }
  }
  //MSG_INFO(3,  "orderXText() finished" << endline);
} // orderXText()

void orderPresentation(const Reference< XModel >& xModel, std::list< OUString >& formulas, unsigned& count, const Reference < XStatusIndicator >& xStatus) {
  Reference< XDrawPagesSupplier > xPresDoc(xModel, UNO_QUERY);
  Reference< XIndexAccess > xDrawPages(xPresDoc->getDrawPages(), UNO_QUERY_THROW);

  for (int idx = 0; idx < xDrawPages->getCount(); ++idx) {
    Reference< XDrawPage > page(xDrawPages->getByIndex(idx), UNO_QUERY_THROW);
    MSG_INFO(3, "Presentation page " << idx << endline);
    Reference< XIndexAccess > shapes(page, UNO_QUERY_THROW);
    std::set<OUString> pageFormulas; // Set is an ordered collection

    for (int sh = 0; sh < shapes->getCount(); ++sh) {
      MSG_INFO(3, "Shape " << sh << endline);
      Reference< XServiceInfo > xSI(shapes->getByIndex(sh), UNO_QUERY_THROW);

      if (xSI.is() && xSI->supportsService(OU("com.sun.star.drawing.OLE2Shape"))) {
        Reference< XComponent > comp(shapes->getByIndex(sh), UNO_QUERY_THROW);
        MSG_INFO(2, "Found OLE shape" << endline);

        if (checkIsFormula(comp)) {
          OUString objName = getObjectName(comp);
          MSG_INFO(2,  "Found formula: " << STR(objName) << endline);
          ++count;
          if (xStatus.is()) xStatus->setValue(count);
          pageFormulas.emplace(objName);
        }
      }
    }

    formulas.insert(formulas.end(), pageFormulas.begin(), pageFormulas.end());
  }
}

unsigned countFormulas(const Reference< XModel >& xModel) {
  unsigned result = 0;
  Reference< XDrawPagesSupplier > xPresDoc(xModel, UNO_QUERY);
  Reference< XIndexAccess > xDrawPages(xPresDoc->getDrawPages(), UNO_QUERY_THROW);

  for (int idx = 0; idx < xDrawPages->getCount(); ++idx) {
    Reference< XDrawPage > page(xDrawPages->getByIndex(idx), UNO_QUERY_THROW);
    Reference< XIndexAccess > shapes(page, UNO_QUERY_THROW);

    for (int sh = 0; sh < shapes->getCount(); ++sh) {
      Reference< XServiceInfo > xSI(shapes->getByIndex(sh), UNO_QUERY_THROW);

      if (xSI.is() && xSI->supportsService(OU("com.sun.star.drawing.OLE2Shape"))) {
        Reference< XComponent > comp(shapes->getByIndex(sh), UNO_QUERY_THROW);

        if (checkIsFormula(comp)) ++result;
      }
    }
  }

  return result;
}

OUString getTextFieldContent(const Reference< XTextDocument >& xDoc, const OUString& textFieldName) {
  Reference< XTextFieldsSupplier > xTFSupplier(xDoc, UNO_QUERY_THROW);
  Reference< XNameAccess > xTextFieldMasters = xTFSupplier->getTextFieldMasters();

  Reference< XPropertySet > xTextFieldMaster(xTextFieldMasters->getByName(OU("com.sun.star.text.fieldmaster.") + textFieldName), UNO_QUERY);

  if (xTextFieldMaster.is()) {
     /*
     // Note: This only works if there actually are dependent text fields displayed in the document
     Any aDependentTextFields = xTextFieldMaster->getPropertyValue(OU("DependentTextFields"));
     Sequence< Reference< XDependentTextField > > xDependentTextFields;
     aDependentTextFields >>= xDependentTextFields;
     if (xDependentTextFields.getLength() > 0) {
      MSG_INFO(0, "Representation of text field is '" << STR(xDependentTextFields[0]->getPresentation(false)) << "'" << endline);
      return xDependentTextFields[0]->getPresentation(false);
    }
    */

    // Note: This also does not update correctly if no text field is actually displayed in the document
    Any aContent = xTextFieldMaster->getPropertyValue(OU("Content"));
    OUString c;
    aContent >>= c;
    return c;
  }

  return OU("notfound");
}

Reference< XCell > getTableCell(const Reference< XTextDocument >& xDoc, const OUString& tableName, const OUString& tableCellName) {
  Reference< XTextTablesSupplier > xTSupplier(xDoc, UNO_QUERY_THROW);
  Reference< XNameAccess > xTextTables = xTSupplier->getTextTables();

  if (xTextTables->hasByName(tableName)) {
    Any aTextTable = xTextTables->getByName(tableName);
    Reference< XTextTable > xTextTable;
    aTextTable >>= xTextTable;
    Reference< XCell > xCell(xTextTable->getCellByName(tableCellName), UNO_QUERY);
    if (xCell.is())
      return xCell;
    else
      throw std::runtime_error("Referenced cell '" + STR(tableCellName) + "' does not exist");
  } else {
    throw std::runtime_error("Referenced table '" + STR(tableName) + "' does not exist");
  }
}

expression getCellExpression(const Reference< XCell >& xCell) {
  if (xCell.is()) {
    if ((xCell->getType() == com::sun::star::table::CellContentType_VALUE) ||
        ((xCell->getType() == com::sun::star::table::CellContentType_FORMULA) && (xCell->getError() == 0))) {
      double val = xCell->getValue();
      if (std::floor(val) - val == 0)
        return dynallocate<numeric>(std::floor(val));
      else
        return dynallocate<numeric>(val);
    } else if (xCell->getType() == com::sun::star::table::CellContentType_FORMULA) {
      return dynallocate<stringex>(STR(xCell->getFormula()));
    } else if (xCell->getType() == com::sun::star::table::CellContentType_TEXT) {
      Reference< XText > xCellText(xCell, UNO_QUERY);
      if (xCellText.is())
      return dynallocate<stringex>(STR(xCellText->getString()));
    }
  }

  return dynallocate<stringex>("");
}

void setCellString(const Reference< XCell >& xCell, const OUString& value) {
  if (xCell.is()) {
    if (value.trim()[0] == '=') {
      xCell->setFormula(value);
    } else {
      Reference< XText > xCellText(xCell, UNO_QUERY);
      if (xCellText.is()) xCellText->setString(value);
    }
  }
}

void setCellDouble(const Reference< XCell >& xCell, const double value) {
  if (xCell.is()) xCell->setValue(value);
}

void setCellExpression(const Reference< XCell >& xCell, const expression& value) {
  if (xCell.is()) {
    if (is_a<stringex>(value)) {
      // This avoids quotation marks around the string
      setCellString(xCell, OUS8(ex_to<stringex>(value).get_string()));
    } else {
      if (is_a<numeric>(value) && ex_to<numeric>(value).info(info_flags::real)) {
        setCellDouble(xCell, ex_to<numeric>(value).to_double());
      } else {
        std::ostringstream os;
        os << value;
        setCellString(xCell, OUS8(os.str()));
      }
    }
  }
}

expression calcCellRangeContent(const Reference<XComponentContext>& xContext, const OUString& calcURL, const OUString& sheetName, const OUString& cellRange) {
  Reference<XMultiComponentFactory> xMCF = xContext->getServiceManager();
  Reference< XDesktop > xDesktop(xMCF->createInstanceWithContext(OU("com.sun.star.frame.Desktop" ), xContext), UNO_QUERY_THROW);
  Reference< XModel > docModel = checkDocumentLoaded(xDesktop, calcURL);
  bool docIsLoaded = docModel.is();
  if (!docIsLoaded) docModel = loadDocument(xDesktop, calcURL, true);

  Reference< XSpreadsheetDocument >xCalcDoc(docModel, UNO_QUERY_THROW);
  Reference< XColumnRowRange > xColumnRowRange = getCalcCellRange(xCalcDoc, sheetName, cellRange);
  expression result = getCalcRangeExpression(xColumnRowRange);

  if (!docIsLoaded) {
    Reference< XCloseable > xClose(xCalcDoc, UNO_QUERY_THROW);
    xClose->close(true);
  }

  return result;
}

void setCalcCellRange(const Reference<XComponentContext>& xContext, const OUString& calcURL, const OUString& sheetName, const OUString& cellRange, const ex& value) {
  Reference<XMultiComponentFactory> xMCF = xContext->getServiceManager();
  Reference< XDesktop > xDesktop(xMCF->createInstanceWithContext(OU("com.sun.star.frame.Desktop" ), xContext), UNO_QUERY_THROW);
  Reference< XModel > docModel = checkDocumentLoaded(xDesktop, calcURL);
  bool docIsLoaded = docModel.is();
  if (!docIsLoaded) docModel = loadDocument(xDesktop, calcURL, false);

  Reference< XSpreadsheetDocument >xCalcDoc(docModel, UNO_QUERY_THROW);
  Reference< XColumnRowRange > xColumnRowRange = getCalcCellRange(xCalcDoc, sheetName, cellRange);
  setCalcCellRangeExpression(xColumnRowRange, value);

  if (!docIsLoaded) {
    Reference< XStorable > xStore(xCalcDoc, UNO_QUERY_THROW);
    xStore->store();
    Reference< XCloseable > xClose(xCalcDoc, UNO_QUERY_THROW);
    xClose->close(true);
  }
}

Reference< XModel > checkDocumentLoaded(Reference< XDesktop >& xDesktop, const OUString& URL) {
  Reference< XEnumerationAccess > xLoadedDocsEnumAccess = xDesktop->getComponents();
  Reference< XEnumeration > xDocsEnum = xLoadedDocsEnumAccess->createEnumeration();

  while (xDocsEnum->hasMoreElements()) {
    Any docModel = xDocsEnum->nextElement();
    Reference< XModel > xModel;
    docModel >>= xModel;
    if (xModel.is() && (xModel->getURL() == URL)) return xModel;
  }

  return Reference< XModel >();
}

Reference< XModel > loadDocument(const Reference < XDesktop >& xDesktop, const OUString& calcURL, const bool readonly) {
  Sequence< PropertyValue > args(2);
  PropertyValue hidden;
  hidden.Name = OU("Hidden");
  hidden.Value = makeAny(true);
  args[0] = hidden;
  PropertyValue ro;
  ro.Name = OU("ReadOnly");
  ro.Value = makeAny(readonly);
  args[1] = ro;

  try {
    Reference< XComponentLoader > xComponentLoader(xDesktop, UNO_QUERY_THROW);
    return Reference< XModel >(xComponentLoader->loadComponentFromURL(calcURL, OU("_default"), 0, args), UNO_QUERY_THROW);
  } catch(Exception &e) {
    (void)e;
    throw std::runtime_error("Referenced document '" + STR(calcURL) + "' does not exist");
  }
}

OUString makeSystemPathFor(const OUString& theURL, const Reference<XComponentContext>& xContext) {
  MSG_INFO(2,  "makeSystemPathFor() '" << STR(theURL) << "'" << endline);
  if (!theURL.matchAsciiL("file:///", 8)) // No URL given, so we assume it is already a system path
    return theURL;

  Reference<XMultiComponentFactory> xMCF = xContext->getServiceManager();
  Reference< XFileIdentifierConverter > xFIConverter(
    xMCF->createInstanceWithContext(OU("com.sun.star.ucb.FileContentProvider"), xContext), UNO_QUERY_THROW);
  return(xFIConverter->getSystemPathFromFileURL(theURL));
} // makeSystemPathFor()

OUString makeURLFor(const OUString& newURL, const OUString& absoluteURL, const Reference<XComponentContext>& xContext) {
  MSG_INFO(2,  "makeURLFor()" << endline);
  OUString result = OU("");

  Reference<XMultiComponentFactory> xMCF = xContext->getServiceManager();
  Reference< XFileIdentifierConverter > xFIConverter(
    xMCF->createInstanceWithContext(OU("com.sun.star.ucb.FileContentProvider"), xContext), UNO_QUERY_THROW);

  if (!newURL.equalsAscii("")) {
    if (newURL.matchAsciiL("/",1)) { // Absolute Unix path
      return xFIConverter->getFileURLFromSystemPath(OU(""), newURL);
    } else if (newURL.copy(1,1).equalsAscii(":") || newURL.matchAsciiL("\\\\", 2)) {
      // Windows absolute path, e.g. C: or D:, or Windows network path
      return xFIConverter->getFileURLFromSystemPath(OU(""), newURL);
    } else if (newURL.matchAsciiL("file:///", 8)) { // Absolute office URL
      return newURL;
    } else { // relative URL
       if (absoluteURL.equalsAscii("")) { // unsaved new document!
         return newURL;
       } else {
         int slashpos = absoluteURL.lastIndexOfAsciiL("/", 1);
         return absoluteURL.copy(0, slashpos) + OU("/") + xFIConverter->getFileURLFromSystemPath(OU(""), newURL);
       }
    }
  }

  return result;
} // makeURLFor()

sal_Bool isGlobalDocument(const Reference< XModel >& xModel) {
  Reference< XServiceInfo > xServiceInfo(xModel, UNO_QUERY_THROW);
  return (xServiceInfo->supportsService(OU("com.sun.star.text.GlobalDocument")));
}

OUString docType(const Reference< XModel >& xModel) {
  Reference< XTextDocument > doc(xModel, UNO_QUERY);
  return doc.is() ? OU("TextDocument") : OU("Presentation");
}

Reference< XColumnRowRange > getCalcCellRange(const Reference < XSpreadsheetDocument >& xCalcDoc, const OUString& sheetName, const OUString& cellRange) {
  try {
    Reference< XSpreadsheets > xCalcSheets = xCalcDoc->getSheets();
    Reference< XSpreadsheet >  xCalcSheet(xCalcSheets->getByName(sheetName), UNO_QUERY_THROW);
    Reference< XColumnRowRange > xColumnRowRange(xCalcSheet->getCellRangeByName(cellRange), UNO_QUERY_THROW);
    return xColumnRowRange;
  } catch(Exception &e) {
    (void)e;
    throw std::runtime_error("Referenced cell range '" + STR(sheetName) + "." + STR(cellRange) + "' does not exist");
  }
}

expression getCalcRangeExpression(const Reference < XColumnRowRange >& xColumnRowRange) {
  if (xColumnRowRange.is()) {
    Reference< XCellRange > xCellRange(xColumnRowRange, UNO_QUERY_THROW);
    Reference< XTableColumns > xCols = xColumnRowRange->getColumns();
    Reference< XTableRows > xRows = xColumnRowRange->getRows();

    unsigned rows = xRows->getCount();
    unsigned cols = xCols->getCount();
    MSG_INFO(2, "Found cell range with " << rows << " rows and " << cols << " coumns" << endline);

    if ((rows == 1) && (cols == 1)) {
      Reference< XCell > xCell = xCellRange->getCellByPosition(0, 0);
      return getCellExpression(xCell);
    } else {
      matrix m(rows, cols);

      for (unsigned r = 0; r < rows; ++r) {
        for (unsigned c = 0; c < cols; ++c) {
          Reference< XCell > xCell = xCellRange->getCellByPosition(c, r);
          m(r,c) = getCellExpression(xCell);
        }
      }
      return m;
    }
  }
  return dynallocate<stringex>("Error: Non-existant calc cell range or internal error");
}

void setCalcCellRangeExpression(const Reference < XColumnRowRange >& xColumnRowRange, const expression& value) {
  if (xColumnRowRange.is()) {
    Reference< XCellRange > xCellRange(xColumnRowRange, UNO_QUERY_THROW);
    Reference< XTableColumns > xCols = xColumnRowRange->getColumns();
    Reference< XTableRows > xRows = xColumnRowRange->getRows();

    unsigned rows = xRows->getCount();
    unsigned cols = xCols->getCount();
    MSG_INFO(2, "Found cell range with " << rows << " rows and " << cols << " coumns" << endline);

    if (is_a<matrix>(value)) {
      const matrix& m = ex_to<matrix>(value);
      unsigned mrows = m.rows();
      unsigned mcols = m.cols();

      if ((rows != mrows) || (cols != mcols))
        throw std::runtime_error("Cell range dimensions do not match vector/matrix dimensions");

      for (unsigned r = 0; r < rows; ++r) {
        for (unsigned c = 0; c < cols; ++c) {
          Reference< XCell > xCell = xCellRange->getCellByPosition(c, r);
          setCellExpression(xCell, m(r, c));
        }
      }
    } else if ((rows == 1) && (cols == 1)) {
      Reference< XCell > xCell = xCellRange->getCellByPosition(0, 0);
      setCellExpression(xCell, value);
    } else {
      throw std::runtime_error("Cell range dimensions do not match scalar");
    }
  }
}

expression parseNumber(const std::string& s) {
  std::istringstream text(s);
  int inumber;
  double dnumber;
  bool success;
  size_t cpos;

  if ((cpos = s.find(",")) != std::string::npos) {
    std::string rs = s;
    std::istringstream text2(rs.replace(cpos, 1, "."));
    text2 >> dnumber;
    success = text2.rdstate() == std::ios::goodbit;
  } else {
    text >> dnumber; // Note: this cuts off number at decimal "," separator
    success = text.rdstate() == std::ios::goodbit;
  }

  if (success) {
    text >> inumber; // This will always work: decimal places are cut off

    if ((double)inumber != dnumber)
      return expression(dnumber);
    else
      return expression(inumber); // Preserve integers if possible
  } else {
  return dynallocate<stringex>(s);
  }
}

expression getExpressionFromString(const OUString& s) {
  std::string tf = STR(s);

  if ((tf.find("\n") != std::string::npos) || (tf.find("\t") != std::string::npos)) {
    // String represents a matrix. Split rows at line breaks and columns at tabs
    std::istringstream mtext(tf);
    std::string row;
    std::vector<std::string> rows;
    while (std::getline(mtext, row, '\n')) rows.emplace_back(row);

    std::vector< std::vector<std::string> > strmatrix;
    size_t colnum = 0;

    for (const auto& r : rows) {
      std::istringstream rtext(r);
      std::string col;
      strmatrix.emplace_back();
      std::vector<std::string>& cols = strmatrix.back();
      while (std::getline(rtext, col, '\t')) cols.emplace_back(col);

      size_t oldcolnum = colnum;
      colnum = cols.size();
      if ((oldcolnum != 0) && (oldcolnum != colnum)) // Column number mismatch
        return dynallocate<stringex>(tf);
    }

    matrix m((unsigned)strmatrix.size(), (unsigned)colnum);
    unsigned irow = 0;
    for (std::vector< std::vector<std::string> >::const_iterator r = strmatrix.begin(); r != strmatrix.end(); ++r, ++irow) {
      unsigned icol = 0;
      for (std::vector< std::string>::const_iterator e = r->begin(); e != r->end(); ++e, ++icol) {
        m(irow, icol) = parseNumber(*e);
      }
    }

    return m;
  } else {
    return parseNumber(tf);
  }
}

Reference<XNamedGraph> createGraph(const Reference< XComponentContext >& mxCC, const Reference<XModel>& xModel) {
  Reference<XDocumentMetadataAccess> xDMA(xModel, UNO_QUERY_THROW);
  Reference<XURI> xType = URI::create(mxCC, OU("http://jan.rheinlaender.gmx.de/imath/options/v1.0"));
  Sequence<Reference<XURI> > types(1);
  types[0] = xType;

  try {
    Reference<XURI> xGraphName = xDMA->addMetadataFile(OU("imathoptions.rdf"), types);
    return xDMA->getRDFRepository()->getGraph(xGraphName);
  } catch (ElementExistException e) { // filename exists?
  }

  throw std::runtime_error("Internal error: RDF graph already exists");
}

Reference<XNamedGraph> getGraph(const Reference< XComponentContext >& mxCC, const Reference<XModel>& xModel) {
  Reference<XDocumentMetadataAccess> xDMA(xModel, UNO_QUERY_THROW);
  Reference<XURI> xType = URI::create(mxCC, OU("http://jan.rheinlaender.gmx.de/imath/options/v1.0"));
  Sequence<Reference<XURI> > graphNames = xDMA->getMetadataGraphsWithType(xType);
  Reference<XNamedGraph> result;

  if (graphNames.getLength() > 0) {
    // There should only be one single graph
    result = xDMA->getRDFRepository()->getGraph(graphNames[0]);
  }

  return result;
}

void addStatement(const Reference< XComponentContext >& mxCC, const Reference<XModel>& xModel,
                  const Reference<XNamedGraph>& xGraph, const OUString& predicate, const OUString& value) {
#if (OO_MAJOR_VERSION == 3) && (OO_MINOR_VERSION <= 5) || (OO_MAJOR_VERSION >= 7) || (OO_IS_AOO == 1)
  Reference<XResource> docURI(xModel, UNO_QUERY_THROW);
#else
  Reference<XURI> docURI(xModel, UNO_QUERY_THROW);
#endif
  Reference<XURI> xPredicate = URI::create(mxCC, OU("http://jan.rheinlaender.gmx.de/imath/predicates/") + predicate);
#if (OO_MAJOR_VERSION == 3) && (OO_MINOR_VERSION <= 5) || (OO_MAJOR_VERSION >= 7) || (OO_IS_AOO == 1)
  Reference<XLiteral> xLit = Literal::create(mxCC, value);
  Reference<XNode> xObj(xLit, UNO_QUERY_THROW);
#else
  Reference<XLiteral> xObj = Literal::create(mxCC, value);
#endif
  xGraph->addStatement(docURI, xPredicate, xObj);
}

void updateStatement(const Reference< XComponentContext >& mxCC, const Reference<XModel>& xModel,
                     const Reference<XNamedGraph>& xGraph, const OUString& predicate, const OUString& value) {
#if (OO_MAJOR_VERSION == 3) && (OO_MINOR_VERSION <= 5) || (OO_MAJOR_VERSION >= 7) || (OO_IS_AOO == 1)
  Reference<XResource> docURI(xModel, UNO_QUERY_THROW);
#else
  Reference<XURI> docURI(xModel, UNO_QUERY_THROW);
#endif
  Reference<XURI> xPredicate = URI::create(mxCC, OU("http://jan.rheinlaender.gmx.de/imath/predicates/") + predicate);
#if (OO_MAJOR_VERSION == 3) && (OO_MINOR_VERSION <= 5) || (OO_MAJOR_VERSION >= 7) || (OO_IS_AOO == 1)
  Reference<XLiteral> xLit = Literal::create(mxCC, value);
  Reference<XNode> xObj(xLit, UNO_QUERY_THROW);
#else
  Reference<XLiteral> xObj = Literal::create(mxCC, value);
#endif
  xGraph->removeStatements(docURI, xPredicate, NULL);
  xGraph->addStatement(docURI, xPredicate, xObj);
}

bool hasStatement(const Reference< XComponentContext >& mxCC, const Reference<XModel>& xModel,
                        const Reference<XNamedGraph>& xGraph, const OUString& predicate) {
#if (OO_MAJOR_VERSION == 3) && (OO_MINOR_VERSION <= 5) || (OO_MAJOR_VERSION >= 7) || (OO_IS_AOO == 1)
  Reference<XResource> docURI(xModel, UNO_QUERY_THROW);
#else
  Reference<XURI> docURI(xModel, UNO_QUERY_THROW);
#endif
  Reference<XURI> xPredicate = URI::create(mxCC, OU("http://jan.rheinlaender.gmx.de/imath/predicates/") + predicate);
  Reference<XEnumeration> xResult = xGraph->getStatements(docURI, xPredicate, NULL); // All statements must have this document as subject

  return xResult->hasMoreElements();
}

OUString getStatementString(const Reference< XComponentContext >& mxCC, const Reference<XModel>& xModel,
                                  const Reference<XNamedGraph>& xGraph, const OUString& predicate) {
#if (OO_MAJOR_VERSION == 3) && (OO_MINOR_VERSION <= 5) || (OO_MAJOR_VERSION >= 7) || (OO_IS_AOO == 1)
  Reference<XResource> docURI(xModel, UNO_QUERY_THROW);
#else
  Reference<XURI> docURI(xModel, UNO_QUERY_THROW);
#endif
  Reference<XURI> xPredicate = URI::create(mxCC, OU("http://jan.rheinlaender.gmx.de/imath/predicates/") + predicate);
  Reference<XEnumeration> xResult = xGraph->getStatements(docURI, xPredicate, NULL); // All statements must have this document as subject

  if (xResult->hasMoreElements()) {
    Any element = xResult->nextElement();
    Statement stmt;
    element >>= stmt;
    Reference<XLiteral> object(stmt.Object, UNO_QUERY_THROW);
    return object->getValue();
  } else {
    return OU("");
  }
}

sal_Bool getStatementBool(const Reference< XComponentContext >& mxCC, const Reference<XModel>& xModel,
                                const Reference<XNamedGraph>& xGraph, const OUString& predicate) {
  return getStatementString(mxCC, xModel, xGraph, predicate) == OU("true");
}
sal_uInt32 getStatementPosInt(const Reference< XComponentContext >& mxCC, const Reference<XModel>& xModel,
                                    const Reference<XNamedGraph>& xGraph, const OUString& predicate) {
  return std::lround(getStatementString(mxCC, xModel, xGraph, predicate).toDouble());
}
sal_Int32 getStatementInt(const Reference< XComponentContext >& mxCC, const Reference<XModel>& xModel,
                                const Reference<XNamedGraph>& xGraph, const OUString& predicate) {
  return std::lround(getStatementString(mxCC, xModel, xGraph, predicate).toDouble());
}

void removeStatement(const Reference< XComponentContext >& mxCC, const Reference<XModel>& xModel,
                     const Reference<XNamedGraph>& xGraph, const OUString& predicate) {
#if (OO_MAJOR_VERSION == 3) && (OO_MINOR_VERSION <= 5) || (OO_MAJOR_VERSION >= 7) || (OO_IS_AOO == 1)
  Reference<XResource> docURI(xModel, UNO_QUERY_THROW);
#else
  Reference<XURI> docURI(xModel, UNO_QUERY_THROW);
#endif
  Reference<XURI> xPredicate = URI::create(mxCC, OU("http://jan.rheinlaender.gmx.de/imath/predicates/") + predicate);
  xGraph->removeStatements(docURI, xPredicate, NULL);
}

OUString getPackageLocation(const Reference < XComponentContext >& mxContext, const OUString& id) {
  Reference< XPackageInformationProvider > xInfoProvider( com::sun::star::deployment::PackageInformationProvider::get(mxContext));
  return xInfoProvider->getPackageLocation(id);
}

std::string trimstring(const std::string& s) {
  std::string result = s;

  size_t endpos = result.find_last_not_of(" \t");
  if (std::string::npos != endpos) result = result.substr(0, endpos+1);

  size_t startpos = result.find_first_not_of(" \t");
  if (std::string::npos != startpos) result = result.substr(startpos);

  return result;
}

OUString replaceString(const OUString& str, const OUString& substr, const OUString& repl) {
  int idx = 0;
  int lastidx = 0;
  OUString result(OU(""));

  while (idx < str.getLength()) {
    lastidx = idx;
    idx = str.indexOf(substr, lastidx);
    if (idx < 0) {
      return result + str.copy(lastidx);
    } else {
      result = result + str.copy(lastidx, idx - lastidx) + repl;
      idx = idx + substr.getLength();
    }
  }

  return result; // This is never reached, but pacifies the compiler
}

std::list<OUString> splitString(const OUString& str, const sal_Unicode boundary) {
  std::list<OUString> result;
  sal_Int32 idx = 0;

  do {
    OUString token = str.getToken(0, boundary, idx);
    if (token.getLength() > 0)
      result.emplace_back(token);
  } while (idx >= 0);

  return result;
}

OUString getLocaleName(const Reference< XComponentContext >& mxCC) {
  Reference< XHierarchicalPropertySet > xProperties = getRegistryAccess(mxCC, OU("/org.openoffice.Setup/L10N"));
  Any aLocale = xProperties->getHierarchicalPropertyValue(OU("ooLocale")); // UI language
  OUString ooLocale;
  aLocale >>= ooLocale;
  aLocale = xProperties->getHierarchicalPropertyValue(OU("ooSetupSystemLocale")); // Locale set by user (might be empty)
  OUString ooSSLocale;
  aLocale >>= ooSSLocale;
  return (ooSSLocale == OU("") ? ooLocale : ooSSLocale);
  // Note: There is also "DecimalSeparatorAsLocale" with the following description in Setup.xcs:
  //      Indicates that the decimal separator (dot or commma) is used as appropriate for the selected locale instead of the one related to the default keyboard layout
} // getLocaleName()

bool hasEnclosingBrackets(const OUString& arg) {
  if (arg.matchAsciiL("(", 1, 0) && arg.endsWithAsciiL(")", 1))  {
    if (arg.getLength() == 3) return true; // single letter or digit

    // Check for enclosing brackets at start and end of string
    int blevel = 1; // bracket level
    int startpos = 1;
    int bopenpos = arg.indexOfAsciiL("(", 1, startpos); // Could be -1 (not found)
    int bclosepos = arg.indexOfAsciiL(")", 1, startpos); // Could be -1 (not found)
    int bpos;
    if (bopenpos < 0)
      bpos = bclosepos;
    else if (bclosepos < 0)
      bpos = bopenpos;
    else
      bpos = std::min(bopenpos, bclosepos);
    bool hasBrackets = false;

    while (bpos > 0) {
      if (bpos == bopenpos) blevel++; else blevel--;
      if (blevel == 0) {
        hasBrackets = (bpos == arg.getLength() - 1); // Bracket level zero and closing bracket is last character of string
        break;
      }
      startpos = bpos + 1;
      bopenpos = arg.indexOfAsciiL("(", 1, startpos);
      bclosepos = arg.indexOfAsciiL(")", 1, startpos);
      if (bopenpos < 0)
        bpos = bclosepos;
      else if (bclosepos < 0)
        bpos = bopenpos;
      else
        bpos = std::min(bopenpos, bclosepos);
    }

    return hasBrackets;
  }

  return false;
}

int versionCompare(const OUString& file, const OUString& prog) {
  MSG_INFO(0, "Comparing file version " << STR(file) << " with program version " << STR(prog) << endline);
  if (file.equals(prog)) return 0; // Catch most frequent case

  std::list<OUString> fileParts = splitString(file, '.');
  std::list<OUString> progParts = splitString(prog, '.');
  auto fp = fileParts.begin();
  auto pp = progParts.begin();

  for (; fp != fileParts.end(), pp != progParts.end(); ++fp, ++pp) {
    sal_Int32 num_file = fp->toInt32();
    sal_Int32 num_prog = fp->toInt32();

    if (num_file < num_prog)
      return -1;
    else if (num_file > num_prog)
      return +1;
  }

  int rem_file_idx = file.indexOf('~');
  int rem_prog_idx = prog.indexOf('~');

  if (rem_file_idx > 0 && rem_prog_idx > 0) {
    OUString rem_file = file.copy(rem_file_idx);
    OUString rem_prog = prog.copy(rem_prog_idx);

    if (rem_file < rem_prog)
      return -1;
    else if (rem_file < rem_prog)
      return +1;
  }

  return 0;
}

std::string getTempPath() {
#ifdef _MSC_VER
  // This file will usually be located in <User>/AppData/Local/Temp
  TCHAR lpTempPathBuffer[MAX_PATH];
  DWORD dwRetVal = GetTempPath(MAX_PATH, lpTempPathBuffer);
  if ((dwRetVal <= MAX_PATH) && (dwRetVal != 0))
    return std::string(lpTempPathBuffer)  + "\\";
  else
    return "";
#else
  return "/tmp/";
#endif
}

bool runProgram(const std::string& program, const std::string& argument) {
  if (!system(NULL)) return false;
    if (program.find(" ") != std::string::npos) {
        // Safety check on file names with spaces, otherwise we might execute a program with parameters e.g. "format C:"
        std::ifstream ifile;
        ifile.open("program");
        if(!ifile) return false; // File does not exist
        ifile.close();
    }
#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
#endif
  if (system (std::string(program + " " + argument).c_str()) != 0)
        return false;
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif
  return true;
}
