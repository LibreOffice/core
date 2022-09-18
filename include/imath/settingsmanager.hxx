/***************************************************************************
    settingsmanager.hxx  -  Header file for class settingsmanager, for managing iMath global and document settings
                             -------------------
    begin                : Sun Nov 07 2017
    copyright            : (C) 2017 by Jan Rheinlaender
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
#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#ifdef INSIDE_SM
#include <imath/option.hxx>
#include <imath/imathutils.hxx>
#else
#include "option.hxx"
#include "imathutils.hxx"
#endif
#include "iFormulaLine.hxx"
#include <ginac/ginac.h>

class Settingsmanager {
public:
  /// Transfer settings from dialog control to iFormula
  static void setLineOptionsFromControls(iFormulaLine& f, const Reference<XControlContainer>& xControlContainer);
  /// Transfer settings from iFormula line to dialog
  static void setControlsFromLineOptions(const iFormulaLine& f, const Reference<XControlContainer>& xControlContainer,
                                         const bool force_global_option = false);

  /// Transfer settings from dialog to office registry (global settings for all documents)
  static void setRegistryFromControls(const Reference<XHierarchicalPropertySet>& xProperties, const Reference<XControlContainer>& xControlContainer);
  /// Transfer settings from office registry to dialog
  static void setControlsFromRegistry(const Reference<XHierarchicalPropertySet>& xProperties, const Reference<XControlContainer>& xControlContainer);

  /// Transfer settings from dialog to document RDF graph (local settings for current document)
  static void setStatementsFromControls(const Reference<XComponentContext>& mxCC, const Reference<XModel>& xModel,
                                        const Reference<XNamedGraph>& xGraph, const Reference<XControlContainer>& xControlContainer);
  /// Transfer settings from document RDF graph to dialog
  static void setControlsFromStatements(const Reference<XComponentContext>& mxCC, const Reference<XModel>& xModel,
                                        const Reference<XNamedGraph>& xGraph, const Reference<XControlContainer>& xControlContainer);

  /// Transfer global and local settings to a map for recalculation
  // Precedence of the settings is:
  // 1. Local settings from RDF graph
  // 2. Local settings from master document
  // 3. Global settings from the office registry
  // If global (registry) settings are used, they are copied to the RDF graph and thus made local to the document
  // for all future recalculations
  static void initializeOptionmap(const Reference<XComponentContext>& mxCC, const Reference<XModel>& xModel,
                                  const Reference<XNamedGraph>& xGraph, const Reference<XHierarchicalPropertySet>& xProperties,
                                  GiNaC::optionmap* o, const bool hasMasterDoc);

  /// Create an iFormula option string from current settings
  static OUString createOptionString(const GiNaC::optionmap& options);
  /// Create an iFormula option string from dialog, using only those settings that differ from the settings in the option map
  static OUString createOptionStringFromControls(const GiNaC::optionmap& options, Reference<XControlContainer>& xControlContainer);

  /// Migrate user-defined document properties to RDF graph statements
  static void migrateProperties(const Reference<XComponentContext>& mxCC, const Reference<XModel>& xModel,
                                const Reference<XNamedGraph>& xGraph, const Reference<XPropertyContainer>& xUserPropsCont);

  /// Remove legacy user-defined document properties
  static void removeLegacyProperties(const Reference<XPropertyContainer>& xUserPropsCont);
private:
  /// Settings data
  struct settingsdata {
    option_type otype; // The data type of the setting
    bool invert; // For historical reasons, some boolean settings need to be inverted
    OUString controlName; // Name of dialog control
    OUString propertyName; // Name of property in Office registry
    OUString statementName; // Name of statement in document RDF graph
    OUString formulaOptionName; // Name of option in iFormula text
    OUString legacyPropertyName; // Name of (legacy) user-defined property name
  };
  static std::map<option_name, settingsdata> settings;
};
#endif
