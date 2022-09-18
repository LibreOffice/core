/***************************************************************************
    imathutils.hxx  -  Utility functions for iMath - header file
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

#ifndef __IMATHUTILS_HXX
#define __IMATHUTILS_HXX

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _COM_SUN_STAR_AWT_XTOOLKIT_HPP_
#include <com/sun/star/awt/XToolkit.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTCONTENT_HPP_
#include <com/sun/star/text/XTextContent.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTEMBEDDEDOBJECTSSUPPLIER_HPP_
#include <com/sun/star/text/XTextEmbeddedObjectsSupplier.hpp>
#endif

// Handle incompatibilities between Office versions
#if (OO_MAJOR_VERSION < 4) || (OO_IS_AOO == 1)
#define OUSTRINGNUMBER OUString::valueOf
#else
#define OUSTRINGNUMBER OUString::number
#endif

#include <stdexcept>
#include "expression.hxx"
#include "extsymbol.hxx"

#include "com/sun/star/lang/IllegalArgumentException.hpp"
#include "com/sun/star/beans/UnknownPropertyException.hpp"
#include "com/sun/star/awt/XDialogEventHandler.hpp"
#include "com/sun/star/awt/XControlContainer.hpp"
#include "com/sun/star/awt/XControl.hpp"
#include "com/sun/star/awt/XTextComponent.hpp"
#include "com/sun/star/awt/XRadioButton.hpp"
#include "com/sun/star/beans/XHierarchicalPropertySet.hpp"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/beans/XPropertyContainer.hpp"
#include "com/sun/star/awt/XListBox.hpp"
#include "com/sun/star/chart/XChartDocument.hpp"
#include "com/sun/star/chart2/XChartDocument.hpp"
#include "com/sun/star/chart/XChartDataArray.hpp"
#include "com/sun/star/chart2/XDiagram.hpp"
#include "com/sun/star/embed/XEmbeddedObject.hpp"
#include "com/sun/star/rdf/XNamedGraph.hpp"
#include "com/sun/star/task/XStatusIndicator.hpp"
#include "com/sun/star/text/XTextDocument.hpp"
#include "com/sun/star/frame/XDesktop.hpp"
#include "com/sun/star/table/XColumnRowRange.hpp"
#include "com/sun/star/table/XCell.hpp"
#include "com/sun/star/sheet/XSpreadsheetDocument.hpp"
#include "com/sun/star/awt/MessageBoxButtons.hpp"
#include "com/sun/star/container/XNameAccess.hpp"
#include "com/sun/star/container/XIndexAccess.hpp"

using namespace com::sun::star::uno;
using namespace com::sun::star::frame;
using rtl::OUString;
using com::sun::star::awt::XToolkit;
using com::sun::star::awt::XDialog;
using com::sun::star::awt::XWindow;
using com::sun::star::text::XText;
using com::sun::star::text::XTextContent;
using com::sun::star::beans::PropertyValue;
using com::sun::star::text::XTextRange;
using com::sun::star::lang::XComponent;
using com::sun::star::text::XTextEmbeddedObjectsSupplier;
using com::sun::star::lang::IllegalArgumentException;
using com::sun::star::beans::UnknownPropertyException;
using com::sun::star::awt::XDialogEventHandler;
using com::sun::star::awt::XControlContainer;
using com::sun::star::awt::XControl;
using com::sun::star::awt::XTextComponent;
using com::sun::star::awt::XRadioButton;
using com::sun::star::beans::XHierarchicalPropertySet;
using com::sun::star::uno::XComponentContext;
using com::sun::star::beans::XPropertyContainer;
using com::sun::star::awt::XListBox;
using com::sun::star::beans::XPropertySet;
using com::sun::star::chart::XChartDataArray;
using com::sun::star::embed::XEmbeddedObject;
using com::sun::star::rdf::XNamedGraph;
using com::sun::star::task::XStatusIndicator;
using com::sun::star::text::XTextDocument;
using com::sun::star::frame::XDesktop;
using com::sun::star::table::XColumnRowRange;
using com::sun::star::table::XCell;
using com::sun::star::sheet::XSpreadsheetDocument;
using com::sun::star::chart2::XChartDocument;
using com::sun::star::container::XNameAccess;
using com::sun::star::container::XIndexAccess;

/// CLSID of a formula document
#define CLSID_FORMULA OU("078B7ABA-54FC-457F-8551-6147e776a997")
/// CLSID of a chart document (doesn't work for Windows, why???)
#define CLSID_CHART OU("12DCAE26-281F-416F-a234-c3086127382e")

inline rtl::OUString OU(const char* what) { return rtl::OUString::createFromAscii(what); }
// Better use OUS8!
/*inline const rtl::OUString OU8(const char* what) { return rtl::OUString(what, (sal_Int32)(sizeof(what)-1), RTL_TEXTENCODING_UTF8); }*/
inline rtl::OUString OUS8(const std::string& what) { return rtl::OUString(what.c_str(), (sal_Int32)what.length(), RTL_TEXTENCODING_UTF8); }
inline rtl::OUString OUS(const std::string& what) { return rtl::OUString::createFromAscii(what.c_str()); }
inline std::string STR(const rtl::OUString& what) { return std::string(OUStringToOString(what, RTL_TEXTENCODING_UTF8).getStr()); }

/// Show a message box with the UNO based toolkit
short ShowMessageBox(const Reference< XToolkit >& rToolkit, const Reference< XFrame >& rFrame,
                          const OUString& aTitle, const OUString& aMsgText, const unsigned width=300,
                          const long buttons=com::sun::star::awt::MessageBoxButtons::BUTTONS_OK);

/// Check if this document is already loaded into a top-level frame
Reference< XModel > checkDocumentLoaded(Reference< XDesktop >& xDesktop, const OUString& URL);

/// Load a document in the background
Reference< XModel > loadDocument(const Reference < XDesktop >& xDesktop, const OUString& calcURL, const bool readonly);

// Return a system-dependant representation of the file URL
OUString makeSystemPathFor(const OUString& theURL, const Reference<XComponentContext>& xContext);

/// If newURL is relative, use the given absolute URL to make a new absolute URL
OUString makeURLFor(const OUString& newURL, const OUString& absoluteURL, const Reference<XComponentContext>& xContext);

/// Check if this is a global document
sal_Bool isGlobalDocument(const Reference< XModel >& xModel);

/// Return the document type (currently TextDocument or Presentation)
OUString docType(const Reference< XModel >& xModel);

/// Create a generic dialog without reference to a component
Reference<XDialog> createBasicDialog (const Reference < XComponentContext > &xCC, const Reference< XToolkit >& xToolkit,
                                      const Reference< XFrame >& xFrame, const OUString &DialogURL,
                                      const Reference< XInterface >& xHandler = Reference< XInterface >());

/// get the user selection
Reference < XInterface > getUserSelection(const Reference < XController > &xCtrl);
Sequence<Reference < XInterface > > getUserSelections(const Reference < XController > &xCtrl);

/// Set the user selection
void setUserSelection(const Reference < XController > &xCtrl, const Reference < XComponent >& newSelection);

/// Get the selected iFormula, return false if none is selected
// The strict version checks whether the object is actually an iFormula
bool getSelectedFormula(const Reference < XController >& xCtrl, Reference < XComponent >& mFormula, const bool strict = false);

/// Get the document URL or the title for an empty (newly created) document
//const OUString getDocURL(const Reference< XModel >& xModel);

/// Get the unique runtime ID for this document
OUString getDocUID(const Reference< XModel >& xModel);

/// Extract the text from an edit field
OUString getEditfield(const Sequence < PropertyValue >& lArgs);

/// Get text at user selection
//Reference < XText > getDocumentText(const Reference < XController > &xCtrl, Reference < XTextCursor > &xModelCursor);

/// Activate embedded object (in-place)
sal_Bool activateOLE(const Reference< XComponent >& xComponent);

/// Check if the object is activated
sal_Bool isOLEactivated(const Reference< XComponent >& xComponent);

/// Check if any formula is open for editing
sal_Bool formulaActive(const Reference< XModel >& xModel);

/// Check if this component is a certain kind of object
sal_Bool checkIsObject(const Reference < XComponent >& xComponent, const OUString& clsid);
sal_Bool checkIsFormula(const Reference < XComponent >& xComponent);
sal_Bool checkIsChart(const Reference < XComponent >& xComponent);

/// Check if this object is an iFormula
sal_Bool checkIsiFormula(const Reference < XComponent >& xComponent);

/// Extract the formula model from a component. It is assumed that the component really is a formula
Reference< XModel > extractModel(const Reference< XComponent >& xComponent);

/// Extract chart from component
Reference< XChartDocument > getChart(const Reference < XComponent >& xComponent);

/// Count formulas in a presentation document
unsigned countFormulas(const Reference< XModel >& xModel);

/// get the formula text from the embedded formula
OUString getFormulaText(const Reference < XModel >& fModel);

/// set the formula text of the embedded formula
void setFormulaText(const Reference < XModel > &fModel, const OUString &fText);

void setFormulaProperty(const Reference < XModel >& fModel, const OUString &propName, const Any& prop);

/// Get the value of a formula property
unsigned getFormulaUnsignedProperty(const Reference < XModel >& fModel, const OUString &propName);

/// get the object's name
OUString getObjectName(const Reference < XComponent> &comp);

/// set the object's name
void setObjectName(const Reference < XComponent > &comp, const OUString& name);

/// Toggle the formula's text mode
void toggleTextMode(const Reference< XComponent >& f);

/// Copy formula properties from one formula to another
void copyProperties(const Reference< XComponent >& source, const Reference< XComponent >& target);

/// insert a formula and return it's model
Reference < XComponent > insertFormula(const Reference < XModel > &xModel);

/// insert a chart and return it's model
Reference < XComponent > insertChart(const Reference < XModel > &xModel, const Reference < XComponentContext > &xCC);

/// Get chart properties
Reference< XPropertySet > getChartTitleProperties(const Reference< XChartDocument >& chart);
Reference< XPropertySet > getDiagramTitleProperties(const Reference< com::sun::star::chart2::XDiagram >& diagram);
Reference< XPropertySet > getDiagramLegendProperties(const Reference< com::sun::star::chart2::XDiagram >& diagram);
Sequence< Sequence< Sequence< Reference < XPropertySet > > > > getDiagramAxesProperties(const Reference< com::sun::star::chart2::XDiagram >& diagram);
Sequence< Sequence< Sequence< Reference < XPropertySet > > > > getDiagramAxesTitleProperties(const Reference< com::sun::star::chart2::XDiagram >& diagram);
void setDiagramAxesProperties(Reference< com::sun::star::chart2::XDiagram >& diagram, const Sequence< Sequence< Sequence< Reference < XPropertySet > > > >& props);
void setDiagramAxesTitleProperties(Reference< com::sun::star::chart2::XDiagram >& diagram, const Sequence< Sequence< Sequence< Reference < XPropertySet > > > >& props);

/// Set series line properties
void setSeriesProperties(const Reference< XComponent >& xChart, const sal_uInt16 series = 1,
  const sal_uInt16 pointsize = 50, const sal_uInt16 linewidth = 50, const sal_uInt32 linecolor = 0);

/// Get the chart data
Sequence< Sequence<double> > getChartData(const Reference< XComponent >& xChart);

/// Get the chart data array object of a chart object
Reference < XChartDataArray > getChartDataArray(const Reference < com::sun::star::chart2::XChartDocument >& cDoc);

/// Set a series description
void setSeriesDescription(const Reference< XComponent >& xChart, const OUString& desc, const int idx);
void setSeriesDescription(const Reference< XModel >& xModel, const OUString& cName, const OUString& desc, const int idx);

/// Set an axis title
void setTitles(const Reference< XComponent >& xChart, const OUString& main, const OUString& xAxis, const OUString& yAxis);

/// Set the data for a chart series, or add a series
void setChartData(const Reference < XModel >& xModel, const OUString& cName, const GiNaC::matrix& yval, const unsigned iseries);
void setChartData(const Reference < XModel >& xModel, const OUString& cName, const GiNaC::matrix& xval, const GiNaC::matrix& yval, const unsigned iseries);
void setChartData(const Reference < XModel >& xModel, const OUString& cName,
  const GiNaC::extsymbol& s, const GiNaC::matrix& xval, const GiNaC::expression& yexpr, const unsigned iseries);

/// Force the diagram to update
void forceDiagramUpdate(const Reference< XComponent >& xChart);

/// remove a formula
void deleteFormula(const Reference < XModel > &xModel, const Reference < XComponent >& iFormula);

/*
/// get the input formulas of the user (selected original formula and operand entered into the editfield)
void getInputFormulas(const Reference < XController > &xCtrl, const Sequence < PropertyValue >& lArgs, smathdriver& iDriver,
        expression& original, expression& operand);
*/

/// get an embedded iFormula by name
Reference < XComponent > getFormulaByName(const Reference < XNameAccess > &embeddedObjects, const OUString &iFormulaName);

/// get an embedded object by name
Reference < XComponent > getObjectByName(const Reference < XModel >& xModel, const OUString &objectName);
Reference < XComponent > getObjectByName(const Reference< XIndexAccess > &xDrawPages, const OUString &objectName);
Reference < XComponent > getObjectByName(const Reference < XNameAccess > &embeddedObjects, const OUString &objectName);

/// Get intermediate text between two formulas
OUString getInterText(const Reference<XTextContent>& f1, const Reference<XTextContent>& f2);

/// Get registry access to a certain node path
Reference< XHierarchicalPropertySet > getRegistryAccess(const Reference< XComponentContext >& mxCC, const OUString& nodepath);

// Change number of cached inline objects
void setInlineCache(const Reference< XComponentContext >& mxCC, const sal_Int32 num);

// Get number of cacheable inline objects
sal_Int32 getInlineCache(const Reference< XComponentContext >& mxCC);

// Check if the container has the named control
bool hasControl(const Reference < XControlContainer >& xControlContainer, const OUString& controlName);

/// extract selections of listbox control
Sequence<OUString> getListBoxSelections(const Reference < XControlContainer >& xControlContainer, const OUString& controlName);
/// extract first selection of listbox control
OUString getFirstListBoxSelection(const Reference < XControlContainer >& xControlContainer, const OUString& controlName);

/// extract selections of listbox control by item position (important if using translated items)
Sequence<short> getListBoxSelectionsPos(const Reference < XControlContainer >& xControlContainer, const OUString& controlName);

/// extract value of text control
OUString getTextcontrol(const Reference < XControlContainer >& xControlContainer, const OUString& controlName);

/// extract value of radio button
sal_Bool getRadioButton(const Reference < XControlContainer >& xControlContainer, const OUString& buttonName);

/// extract the value of a check box
// -1 is error value
sal_Int32 getCheckBox(const Reference < XControlContainer >& xControlContainer, const OUString& controlName);

/// extract value of numeric field
// -1 is error value
sal_Int32 getNumericFieldPosInt(const Reference < XControlContainer >& xControlContainer, const OUString& controlName);

/// extract value of numeric field
sal_Int32 getNumericFieldInt(const Reference < XControlContainer >& xControlContainer, const OUString& controlName);

/// set the value of a text control
void setTextcontrol(const Reference< XControlContainer >& xContainer, const OUString& controlName, const OUString& text);
void setTextcontrol(const Reference< XControlContainer >& xContainer, const OUString& controlName, const Any& value);

/// set the value of a text label
void setLabelcontrol(const Reference< XControlContainer >& xContainer, const OUString& controlName, const OUString& text);
void setLabelcontrol(const Reference< XControlContainer >& xContainer, const OUString& controlName, const Any& value);

/// set the value of a radio button
void setRadioButton(const Reference< XControlContainer >& xContainer, const OUString& controlName, const sal_Bool value);

/// set the value of a check box
void setCheckBox(const Reference< XControlContainer >& xContainer, const OUString& controlName, const sal_Bool value);

/// set the value of a numeric field (positive integer only)
void setNumericFieldPosInt(const Reference< XControlContainer >& xContainer, const OUString& controlName, const sal_uInt32 value);

/// set the value of a numeric field (integer only)
void setNumericFieldInt(const Reference< XControlContainer >& xContainer, const OUString& controlName, const sal_Int32 value);

/// Select an entry in a list box
void setListBox(const Reference< XControlContainer >& xContainer, const OUString& controlName, const OUString& itemName);
void setListBox(const Reference< XControlContainer >& xContainer, const OUString& controlName, const short itemPos);
/// Select multiple entries in a list box
void setListBox(const Reference< XControlContainer >& xContainer, const OUString& controlName, const std::list<OUString>& itemNames);
void setListBox(const Reference< XControlContainer >& xContainer, const OUString& controlName, const Sequence<short>& itemPos);
/// Deselect all items in a list box
void deselectListBox(const Reference< XControlContainer >& xContainer, const OUString& controlName);

/// Hide/Show a control in a dialog
void showControl(const Reference< XControlContainer >& xContainer, const OUString& controlName, const bool show = true);

/// Enable/Disable a control in a dialog
void enableControl(const Reference< XControlContainer >& xContainer, const OUString& controlName, const bool enable = true);

/// Move a control (relative to existing width and heigth)
void moveControlRel(const Reference< XControlContainer >& xContainer, const OUString& controlName, const int deltaX, const int deltaY);

/// Checks if the name property of the window is one of the supported names and returns always a valid string or null
OUString getWindowName(const Reference< XWindow >& aWindow);

/// Check if a user-defined document property exists
sal_Bool propertyIs(const Reference< XPropertyContainer >& xUserPropsContainer, const OUString& propName);
/// Check if a property in the registry exists
sal_Bool propertyIs(const Reference< XHierarchicalPropertySet >& xProperties, const OUString& propName);

/// Get the text value of a document-specific property
OUString getTextProperty(const Reference< XPropertyContainer >& xUserPropsContainer, const OUString& propName);

/// Get the boolean value of a document-specific property
sal_Bool getBoolProperty(const Reference< XPropertyContainer >& xUserPropsContainer, const OUString& propName);

/// Get the unsigned int value of a document-specific property
#if (OO_MAJOR_VERSION == 3) && (OO_MINOR_VERSION <= 5) || (OO_IS_AOO == 1)
sal_Int64 getPosIntProperty(const Reference< XPropertyContainer >& xUserPropsContainer, const OUString& propName);
#else
sal_uInt32 getPosIntProperty(const Reference< XPropertyContainer >& xUserPropsContainer, const OUString& propName);
#endif

/// Get the integer value of a document-specific property
sal_Int32 getIntProperty(const Reference< XPropertyContainer >& xUserPropsContainer, const OUString& propName);

/// Get the numeric value of a user-defined document property
double getNumberProperty(const Reference< XPropertyContainer >& xUserPropsContainer, const OUString& propName);

/// Get the value of a document-specific property. Create it if it does not exist, and initialize it with the value from the registry
OUString getTextProperty(const Reference< XComponentContext >& mxCC, const Reference<XModel>& xModel,
                               const Reference<XNamedGraph>& xGraph, const Reference< XHierarchicalPropertySet >& xProperties,
                               const OUString& userPropName, const OUString& propName);
sal_Bool getBoolProperty(const Reference< XComponentContext >& mxCC, const Reference<XModel>& xModel,
                               const Reference<XNamedGraph>& xGraph, const Reference< XHierarchicalPropertySet >& xProperties,
                               const OUString& userPropName, const OUString& propName);
#if (OO_MAJOR_VERSION == 3) && (OO_MINOR_VERSION <= 5) || (OO_IS_AOO == 1)
sal_Int64 getPosIntProperty(const Reference< XComponentContext >& mxCC, const Reference<XModel>& xModel,
                                   const Reference<XNamedGraph>& xGraph, const Reference< XHierarchicalPropertySet >& xProperties,
                                   const OUString& userPropName, const OUString& propName);
#else
sal_uInt32 getPosIntProperty(const Reference< XComponentContext >& mxCC, const Reference<XModel>& xModel,
                                   const Reference<XNamedGraph>& xGraph, const Reference< XHierarchicalPropertySet >& xProperties,
                                   const OUString& userPropName, const OUString& propName);
#endif
sal_Int32 getIntProperty(const Reference< XComponentContext >& mxCC, const Reference<XModel>& xModel,
                               const Reference<XNamedGraph>& xGraph, const Reference< XHierarchicalPropertySet >& xProperties,
                               const OUString& userPropName, const OUString& propName);

// Extract document references from the controls of the dialog
OUString extractReferences(const Reference< XControlContainer >& xContainer);
// Set dialog controls according to references listed in the string
void setReferences(const Reference< XControlContainer >& xContainer, const OUString& references);

// Check if this formula is in a paragraph containing non-whitespace text
bool checkTextmodeFormula(const Reference< XTextContent >& formula);

/// Get all formulas in a text document in the order they appear in the text
// Also count total of all XTextContent on the page for setInlineCache hack
void orderXText(const Reference< XText >& xText, std::list< OUString >& formulas, unsigned& count, const Reference < XStatusIndicator >& xStatus);

void orderPresentation(const Reference< XModel >& xModel, std::list< OUString >& formulas, unsigned& count, const Reference < XStatusIndicator >& xStatus);

/// Return the content of the given DDE text field
OUString getTextFieldContent(const Reference< XTextDocument >& xDoc, const OUString& textFieldName);

/// Return a cell of a Writer table
Reference< XCell > getTableCell(const Reference< XTextDocument >& xDoc, const OUString& tableName, const OUString& tableCellName);

/// Return an expression from the content of the given cell. The result will be an integer, a numeric or a stringex
GiNaC::expression getCellExpression(const Reference< XCell >& xCell);

/// Return the content of the given calc cell range
GiNaC::expression calcCellRangeContent(const Reference<XComponentContext>& xContext, const OUString& calcURL, const OUString& sheetName, const OUString& cellRange);

/// Set the content of the given calc cell range to the expression (scalar, vector or matrix type must match the given cell range)
void setCalcCellRange(const Reference<XComponentContext>& xContext, const OUString& calcURL, const OUString& sheetName, const OUString& cellRange, const GiNaC::ex& value);

/// Return a cell range of a Calc document
Reference< XColumnRowRange > getCalcCellRange(const Reference < XSpreadsheetDocument >& xCalcDoc, const OUString& sheetName, const OUString& cellRange);

/// Return an expression from the given calc cell range
GiNaC::expression getCalcRangeExpression(const Reference < XColumnRowRange >& xColumnRowRange);

/// Set the given calc cell range to the given value which may be a scalar, vector or matrix
void setCalcCellRangeExpression(const Reference < XColumnRowRange >& xColumnRowRange, const GiNaC::expression& value);

/// Set the given cell to the given value (if it is a string formula, the formula will be interpreted)
void setCellExpression(const Reference< XCell >& xCell, const GiNaC::expression& value);

/// Parse the content of a string (currently specific to text field content strings)
GiNaC::expression getExpressionFromString(const OUString& s);

/// Create the RDF graph that stores the iMath document-specific properties
Reference<XNamedGraph> createGraph(const Reference< XComponentContext >& mxCC, const Reference<XModel>& xModel);

/// Get the RDF graph that stores the iMath document-specific properties
Reference<XNamedGraph> getGraph(const Reference< XComponentContext >& mxCC, const Reference<XModel>& xModel);

/// Add a statement to the graph
void addStatement(const Reference< XComponentContext >& mxCC, const Reference<XModel>& xModel,
                  const Reference<XNamedGraph>& xGraph, const OUString& predicate, const OUString& value);

/// Update a statement in the graph
void updateStatement(const Reference< XComponentContext >& mxCC, const Reference<XModel>& xModel,
                     const Reference<XNamedGraph>& xGraph, const OUString& predicate, const OUString& value);

/// Check if the graph contains a statement
bool hasStatement(const Reference< XComponentContext >& mxCC, const Reference<XModel>& xModel,
                        const Reference<XNamedGraph>& xGraph, const OUString& predicate);

/// Retrieve the value of a statement from the graph
OUString getStatementString(const Reference< XComponentContext >& mxCC, const Reference<XModel>& xModel,
                                  const Reference<XNamedGraph>& xGraph, const OUString& predicate);
sal_Bool getStatementBool(const Reference< XComponentContext >& mxCC, const Reference<XModel>& xModel,
                                const Reference<XNamedGraph>& xGraph, const OUString& predicate);
sal_uInt32 getStatementPosInt(const Reference< XComponentContext >& mxCC, const Reference<XModel>& xModel,
                                    const Reference<XNamedGraph>& xGraph, const OUString& predicate);
sal_Int32 getStatementInt(const Reference< XComponentContext >& mxCC, const Reference<XModel>& xModel,
                                const Reference<XNamedGraph>& xGraph, const OUString& predicate);

// Remove a statement from the graph
void removeStatement(const Reference< XComponentContext >& mxCC, const Reference<XModel>& xModel,
                     const Reference<XNamedGraph>& xGraph, const OUString& predicate);

/// Save nextnum property in user-defined document properties
//void saveNextnum(Reference< XFrame > xFrame, sal_Int32 nextnum);

/// Get the location where the package is installed
OUString getPackageLocation(const Reference < XComponentContext >& mxContext, const OUString& id);

// About office decimal separator settings
// Tools-Options-Language Settings-Language-Document language has no influence on the decimal separator!
// Tools-Options-Language Settings-Language-User Interface language has no influence on the decimal separator unless the next setting is empty
// Tools-Options-Language Settings-Language-Locale setting changes the decimal separator
// Tools-Options-Language Settings-Language-Decimal separator key seems to have no influence?
/// Get the local name
OUString getLocaleName(const Reference< XComponentContext >& mxCC);

/// Check whether the string has enclosing brackets (that is a matching pair of () at the start and end of the string)
bool hasEnclosingBrackets(const OUString& arg);

typedef std::vector<std::string> strvec;

/// Trim whitespace from a string
std::string trimstring(const std::string& s);

/// Replace a substring inside a string
OUString replaceString(const OUString& str, const OUString& substr, const OUString& repl);

/// Split a string into a list of strings at a given boundary
std::list<OUString> splitString(const OUString& str, const sal_Unicode boundary);

/// Compare two version strings of the form M.m.b~x
// Returns -1 if file version is smaller than program version
// Returns +1 if file version is greater than program version
// Returns 0 if versions are identical
int versionCompare(const OUString& file, const OUString& prog);

/// Return the path to a directory suitable for creating temporary files
// Note: The path contains the trailing backslash (Windows) or slash (Unix)
std::string getTempPath();

/// Run a program (OS-specific) and  wait for it to finish
bool runProgram(const std::string& program, const std::string& argument);
#endif
