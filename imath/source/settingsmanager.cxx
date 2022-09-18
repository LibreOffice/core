/***************************************************************************
    settingsmanager.cpp  -  Class for managing iMath settings
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

#ifdef INSIDE_SM
#include <imath/msgdriver.hxx>
#include <imath/settingsmanager.hxx>
#else
#include "msgdriver.hxx"
#include "settingsmanager.hxx"
#endif

using namespace GiNaC;

// How to add a new option:
// 1. Add to the table at the top of this file
// 2. Add to the enumeration in option.hxx
// 3. Add to the scanner in smathlexer.lxx
// 4. Add to the registry in gui/config/iMathOptionData.xcu.in and the schema in gui/schema/iMathOptionData.xcs
// 5. Provide documentation on the iMath wiki and maybe in iMath-tour.odt
// 6. Add some code to implement the option
// 7. Make accessible through the GUI

std::map<option_name, Settingsmanager::settingsdata> Settingsmanager::settings = {
  {o_autofraction, {
    t_bool, false,
    OU("O_Autofraction"), OU("Formatting/O_Autofraction"), OU("formatting_o_autofraction"), OU("autofraction"), OU("Automatic fraction sizing")}},
  {o_autotextmode, {
    t_bool, false,
    OU("O_Autotextmode"), OU("Formatting/O_Autotextmode"), OU("formatting_o_autotextmode"), OU("autotextmode"), OU("Automatic text mode")}},
  {o_basefontheight, {t_uint, false, OU(""), OU(""), OU(""), OU(""), OU("Base font height")}},
  {o_difftype, { // Should really be txt_Difftype
    t_str, false,
    OU(""), OU("Formatting/O_Difftype"), OU("formatting_o_difftype"), OU("difftype"), OU("Differentials notation")}},
  {o_echoformula, {
    t_bool, false,
    OU("O_Echoformula"), OU("Miscellaneous/O_Echoformula"), OU("miscellaneous_o_echoformula"), OU("echo"), OU("Echo formula")}},
  {o_eqalign, {
    t_align, false,
    OU("O_Autoalign"), OU("Formatting/O_Autoalign"), OU("formatting_o_autoalign"), OU("autoalign"), OU("Automatic aligning")}},
  {o_eqchain, {
    t_bool, false,
    OU("O_Autochain"), OU("Formatting/O_Autochain"), OU("formatting_o_autochain"), OU("autochain"), OU("Automatic chaining")}},
  {o_eqraw, {
    t_bool, true,
    OU("O_Autoformat"), OU("Formatting/O_Autoformat"), OU("formatting_o_autoformat"), OU("autoformat"), OU("Automatic formatting")}},
  {o_externaleditor, {
    t_str, false,
  OU("txt_Externaleditor"), OU("Editing/txt_Externaleditor"), OU(""), OU(""), OU("External formula editor")}},
  {o_forceautoformat, {t_bool, false, OU(""), OU(""), OU(""), OU(""), OU("Force automatic formatting")}},
  {o_implicitmul, {
    t_bool, false,
    OU("O_Implicitmul"), OU("Formatting/O_Implicitmul"), OU("formatting_o_implicitmul"), OU("implicitmul"), OU("Implicit multiplication")}},
  {o_eqsplit, {
    t_int, false,
    OU(""), OU("Formatting/I_Eqsplitpoint"), OU("formatting_i_eqsplitpoint"), OU("eqsplit"), OU("Equation line break point")}},
  {o_eqsplittext, {
    t_str, false,
    OU(""), OU("Formatting/txt_Eqsplittext"), OU("formatting_txt_eqsplittext"), OU("eqsplittext"), OU("Equation line break text")}},
  {o_evalf_real_roots, {
    t_bool, false,
    OU("O_Evalf_Real_Roots"), OU("Internal/O_Evalf_Real_Roots"), OU("internal_o_evalf_real_roots"), OU("realroots"), OU("Evaluate to real roots")}},
  {o_exponent, {
    t_int, false,
    OU("I_Exponent"), OU("Formatting/I_Exponent"), OU("formatting_i_exponent"), OU("exponent"), OU("Exponent")}},
  {o_fixeddigits, {
    t_bool, true,
    OU("O_Fixedpoint"), OU("Formatting/O_Fixedpoint"), OU("formatting_o_fixedpoint"), OU("fixedpoint"), OU("Fixed point notation")}},
  {o_formulafont, {
    t_str, false,
    OU("lst_FormulaFont"), OU("Formatting/txt_FormulaFont"), OU("formatting_txt_formulafont"), OU(""), OU("Formula font")}},
  {o_fractionlevel, {t_uint, false, OU(""), OU(""), OU(""), OU(""), OU("Fraction nesting level")}},
  {o_highsclimit, {
    t_int, false,
    OU("I_Highsclimit"), OU("Formatting/I_Highsclimit"), OU("formatting_i_highsclimit"), OU("highsclimit"), OU("Minimum positive exponent")}},
  {o_lowsclimit, {
    t_int, false,
    OU("I_Lowsclimit"), OU("Formatting/I_Lowsclimit"), OU("formatting_i_lowsclimit"), OU("lowsclimit"), OU("Maximum negative exponent")}},
  {o_minimumtextsize, {
    t_uint, false,
    OU("I_Minimumtextsize"), OU("Formatting/I_Minimumtextsize"), OU("formatting_i_minimumtextsize"), OU("minimumtextsize"), OU("Minimum text size")}},
  {o_precision, {
    t_uint, false,
    OU("I_Precision"), OU("Formatting/I_Precision"), OU("formatting_i_precision"), OU("precision"), OU("Precision")}},
  {o_showlabels, {
    t_bool, false,
    OU("O_ShowLabels"), OU("Formatting/O_ShowLabels"), OU("formatting_o_showlabels"), OU("showlabels"), OU("Show labels")}},
  {o_suppress_units, {
    t_bool, false,
    OU("O_Suppress_Units"), OU("Formatting/O_Suppress_Units"), OU("formatting_o_suppress_units"), OU("suppress_units"), OU("Suppress units")}},
  {o_underflow, {
    t_bool, false,
    OU("O_Underflow"), OU("Internal/O_Underflow"), OU("internal_o_underflow"), OU("underflow"), OU("Inhibit underflow exception")}},
  {o_unitstr, {
    t_str, false,
    OU("txt_Units"), OU("Formatting/txt_PreferredUnits"), OU("formatting_txt_preferredunits"), OU("units"), OU("Preferred units")}},
  {o_units, {t_exvec, false, OU(""), OU(""), OU(""), OU(""), OU("Preferred units")}},
  {o_vecautosize, {
    t_uint, false,
    OU("I_Vecautosize"), OU("Internal/O_Vecautosize"), OU("internal_o_vecautosize"), OU("vecautosize"), OU("Automatic vector size")}},
};

void Settingsmanager::removeLegacyProperties(const Reference<XPropertyContainer>& xUserPropsCont) {
  for (const auto& srec : settings) {
    if (propertyIs(xUserPropsCont, srec.second.legacyPropertyName))
      xUserPropsCont->removeProperty(srec.second.legacyPropertyName);
    else
      MSG_INFO(1, "removeLegacyProperties: property does not exist: " << STR(srec.second.legacyPropertyName) << endline);
  }
}

void Settingsmanager::migrateProperties(const Reference<XComponentContext>& mxCC, const Reference<XModel>& xModel,
                                        const Reference<XNamedGraph>& xGraph, const Reference<XPropertyContainer>& xUserPropsCont) {
  for (const auto& srec : settings) {
    if (!propertyIs(xUserPropsCont, srec.second.legacyPropertyName)) {
      MSG_INFO(1, "migrateProperties: property does not exist: " << STR(srec.second.legacyPropertyName) << endline);
      continue;
    }

    OUString object;

    switch (srec.second.otype) {
      case t_str:
        object = getTextProperty(xUserPropsCont, srec.second.legacyPropertyName);
        break;
      case t_align: // Same handling as boolean property
      case t_bool:
        object = (getBoolProperty(xUserPropsCont, srec.second.legacyPropertyName) ? OU("true") : OU("false"));
        break;
      case t_dbl: break;
      case t_uint:
        object = OUSTRINGNUMBER(getPosIntProperty(xUserPropsCont, srec.second.legacyPropertyName));
        break;
      case t_int:
        object = OUSTRINGNUMBER(getIntProperty(xUserPropsCont, srec.second.legacyPropertyName));
        break;
      case t_exvec: break;
    }

    addStatement(mxCC, xModel, xGraph, srec.second.statementName, object);
    xUserPropsCont->removeProperty(srec.second.legacyPropertyName);
  }
}

OUString makeOptionString(const option_type otype, const OUString& formulaOptionName, const option& o, const bool invert) {
  OUString result = OU("; ") + formulaOptionName + OU("=");

  switch (otype) {
    case t_str:
      if (formulaOptionName.equalsAscii("units")) {
        if (o.value.str->size() > 0) {
          result += OU("{") + OUS8(*(o.value.str)) + OU("}");
        } else {
          return OU("");
        }
      } else {
        result += OU("\"") + OUS8(*(o.value.str)) + OU("\"");
      }
      break;
    case t_bool: {
      bool val = invert ? !o.value.boolean : o.value.boolean;
      result += (val ? OU("true") : OU("false"));
      break;
    }
    case t_dbl: break;
    case t_align:
      result += ((o.value.align == both) ? OU("true") : OU("false"));
      break;
    case t_uint:
      result += OUSTRINGNUMBER((sal_Int32)o.value.uinteger);
      break;
    case t_int:
      result += OUSTRINGNUMBER((sal_Int32)o.value.integer);
      break;
    case t_exvec: break;
  }

  return result;
}

OUString Settingsmanager::createOptionString(const optionmap& options) {
  OUString result;

  for (const auto& i : options) {
    if (i.first == o_units) continue; // We use o_unitstr for creating the option string
    if (i.first == o_forceautoformat) continue; // Internal use only, set by the parser
    std::map<option_name, settingsdata>::const_iterator s_it = settings.find(i.first);
    if (s_it == settings.end()) {
      MSG_INFO(1, "createOptionString: Setting " << STR(s_it->second.formulaOptionName) << " does not exist" << endline);
      continue;
    }
    result += makeOptionString(s_it->second.otype, s_it->second.formulaOptionName, i.second, s_it->second.invert);
  }

  return result;
}

OUString Settingsmanager::createOptionStringFromControls(const optionmap& options, Reference<XControlContainer>& xControlContainer) {
  OUString result;
  option o;

  for (const auto& i : options) {
    std::map<option_name, settingsdata>::const_iterator s_it = settings.find(i.first);
    if (s_it == settings.end()) {
      MSG_INFO(1, "createOptionStringFromControls: Setting " << STR(s_it->second.formulaOptionName) << " does not exist" << endline);
      continue;
    }
    if (!hasControl(xControlContainer, s_it->second.controlName)) {
      MSG_INFO(1, "createOptionStringFromControls: control does not exist: " << STR(s_it->second.controlName) << endline);
      continue;
    }

    switch (s_it->second.otype) {
      case t_str:
        o = option(STR(getTextcontrol(xControlContainer, s_it->second.controlName)));
        break;
      case t_bool:
        o = option(s_it->second.invert ? (getCheckBox(xControlContainer, s_it->second.controlName) != 1) : (getCheckBox(xControlContainer, s_it->second.controlName) == 1));
        break;
      case t_dbl: break;
      case t_align:
        o = option((getCheckBox(xControlContainer, s_it->second.controlName) == 1) ? both : none);
        break;
      case t_uint:
        o = option((unsigned)getNumericFieldPosInt(xControlContainer, s_it->second.controlName));
        break;
      case t_int:
        o = option((int)getNumericFieldInt(xControlContainer, s_it->second.controlName));
        break;
      case t_exvec: break;
    }

    if (i.second != o)
      result += makeOptionString(s_it->second.otype, s_it->second.formulaOptionName, o, s_it->second.invert);
  }

  return result;
}

void Settingsmanager::initializeOptionmap(const Reference<XComponentContext>& mxCC, const Reference<XModel>& xModel,
                                          const Reference<XNamedGraph>& xGraph, const Reference<XHierarchicalPropertySet>& xProperties,
                                          optionmap* o, const bool hasMasterDoc) {
  for (const auto& srec : settings) {
    if (srec.second.propertyName.getLength() == 0 || srec.second.statementName.getLength() == 0) {
            if (srec.second.formulaOptionName.getLength() > 0)
                MSG_INFO(1, "initializeOptionmap: " << STR(srec.second.formulaOptionName) << " is not defined" << endline);
            // otherwise silently ignore (options for internal usage only)
      continue;
    }

    switch (srec.second.otype) {
      case t_str: {
        if (hasMasterDoc) {
          if (hasStatement(mxCC, xModel, xGraph, srec.second.statementName)) {
            OUString localStatement = getStatementString(mxCC, xModel, xGraph, srec.second.statementName);
            if (srec.first == o_unitstr && (*o)[srec.first].value.str != nullptr)
                (*o)[srec.first].value.str->append(((o->at(srec.first).value.str->size() > 0 && localStatement.getLength() > 0) ? "; " : "") +
                                                   STR(localStatement));
            else
              (*o)[srec.first] = option(STR(localStatement));
          // else keep value from master document
          // TODO: There is a memory leak here, because srec.second.value.str is a pointer to a std::string
          }
        } else {
          if (propertyIs(xProperties, srec.second.propertyName))
            (*o)[srec.first] = option(STR(getTextProperty(mxCC, xModel, xGraph, xProperties, srec.second.statementName, srec.second.propertyName)));
          else
            MSG_ERROR(1, "Internal error: Property " << STR(srec.second.propertyName) << " does not exist in the registry." << endline);
        }
        break;
      }
      case t_bool: {
        if (hasMasterDoc) {
          if (hasStatement(mxCC, xModel, xGraph, srec.second.statementName)) {
            bool val = getStatementBool(mxCC, xModel, xGraph, srec.second.statementName); // Beware: Using sal_Bool does not create a t_bool option!
            (*o)[srec.first] = option(srec.second.invert ? !val : val);
          } // else keep value from master document
        } else {
          if (propertyIs(xProperties, srec.second.propertyName)) {
            bool val = getBoolProperty(mxCC, xModel, xGraph, xProperties, srec.second.statementName, srec.second.propertyName); // Beware: Using sal_Bool does not create a t_bool option!
            (*o)[srec.first] = option(srec.second.invert ? !val : val);
          } else {
            MSG_ERROR(1, "Internal error: Property " << STR(srec.second.propertyName) << " does not exist in the registry." << endline);
                    }
        }
        break;
      }
      case t_dbl: break;
      case t_align: {
        if (hasMasterDoc) {
          if (hasStatement(mxCC, xModel, xGraph, srec.second.statementName)) {
            sal_Bool val = getStatementBool(mxCC, xModel, xGraph, srec.second.statementName);
            (*o)[srec.first] = option(val ? both : none);
          } // else keep value from master document
        } else {
          if (propertyIs(xProperties, srec.second.propertyName)) {
            sal_Bool val = getBoolProperty(mxCC, xModel, xGraph, xProperties, srec.second.statementName, srec.second.propertyName);
            (*o)[srec.first] = option(val ? both : none);
          } else {
            MSG_ERROR(1, "Internal error: Property " << STR(srec.second.propertyName) << " does not exist in the registry." << endline);
                    }
        }
        break;
      }
      case t_uint: {
        if (hasMasterDoc) {
          if (hasStatement(mxCC, xModel, xGraph, srec.second.statementName))
            (*o)[srec.first] = option((unsigned)getStatementPosInt(mxCC, xModel, xGraph, srec.second.statementName));
          // else keep value from master document
        } else {
          if (propertyIs(xProperties, srec.second.propertyName))
            (*o)[srec.first] = option((unsigned)getPosIntProperty(mxCC, xModel, xGraph, xProperties, srec.second.statementName, srec.second.propertyName));
                    else
            MSG_ERROR(1, "Internal error: Property " << STR(srec.second.propertyName) << " does not exist in the registry." << endline);
        }
        break;
      }
      case t_int: {
        if (hasMasterDoc) {
          if (hasStatement(mxCC, xModel, xGraph, srec.second.statementName))
            (*o)[srec.first] = option((int)getStatementInt(mxCC, xModel, xGraph, srec.second.statementName));
          // else keep value from master document
        } else {
          if (propertyIs(xProperties, srec.second.propertyName))
            (*o)[srec.first] = option((int)getIntProperty(mxCC, xModel, xGraph, xProperties, srec.second.statementName, srec.second.propertyName));
                    else
            MSG_ERROR(1, "Internal error: Property " << STR(srec.second.propertyName) << " does not exist in the registry." << endline);
        }
        break;
      }
      case t_exvec: break;
    }
  }
}

void Settingsmanager::setLineOptionsFromControls(iFormulaLine& f, const Reference<XControlContainer>& xControlContainer) {
  for (const auto& srec : settings) {
    if (!hasControl(xControlContainer, srec.second.controlName)) {
      MSG_INFO(1, "setLineOptionsFromControls: control does not exist: " << STR(srec.second.controlName)<< endline);
      continue;
    }

    switch (srec.second.otype) {
      case t_str: f.setOption(srec.first, option(STR(getTextcontrol(xControlContainer, srec.second.controlName)))); break;
      case t_bool: {
        bool val = getCheckBox(xControlContainer, srec.second.controlName) == 1;
        f.setOption(srec.first, option(srec.second.invert ? !val : val));
        break;
      }
      case t_dbl: break;
      case t_align: f.setOption(srec.first, option((getCheckBox(xControlContainer, srec.second.controlName) == 1) ? both : none)); break;
      case t_uint: f.setOption(srec.first, option((unsigned)getNumericFieldPosInt(xControlContainer, srec.second.controlName))); break;
      case t_int:  f.setOption(srec.first, option((int)getNumericFieldInt(xControlContainer, srec.second.controlName))); break;
      case t_exvec: break;
    }
  }
}

void Settingsmanager::setControlsFromLineOptions(const iFormulaLine& fLine, const Reference<XControlContainer>& xControlContainer,
                                                 const bool force_global_option) {
  for (const auto& srec : settings) {
    if ((srec.second.controlName.getLength() > 0) && !hasControl(xControlContainer, srec.second.controlName)) {
      MSG_INFO(1, "setControlsFromLineOptions: control does not exist: " << STR(srec.second.controlName)<< endline);
      continue;
    } else if (srec.second.controlName.getLength() == 0) {
      continue;
    }

    switch (srec.second.otype) {
      case t_str:
        if (srec.first == o_unitstr) {
          if (fLine.hasOption(o_unitstr))
            setTextcontrol(xControlContainer, srec.second.controlName, OUS8(*fLine.getOption(srec.first, false).value.str)); // Only show local units here
        } else {
          setTextcontrol(xControlContainer, srec.second.controlName, OUS8(*fLine.getOption(srec.first, force_global_option).value.str));
        }
        break;
      case t_bool: {
        bool val = fLine.getOption(srec.first, force_global_option).value.boolean;
        setCheckBox(xControlContainer, srec.second.controlName, srec.second.invert ? !val : val);
        break;
      }
      case t_dbl: break;
      case t_align:
        setCheckBox(xControlContainer, srec.second.controlName, fLine.getOption(srec.first, force_global_option).value.align == both);
        break;
      case t_uint:
        setNumericFieldPosInt(xControlContainer, srec.second.controlName, fLine.getOption(srec.first, force_global_option).value.uinteger);
        break;
      case t_int:
        setNumericFieldInt(xControlContainer, srec.second.controlName, fLine.getOption(srec.first, force_global_option).value.integer);
        break;
      case t_exvec: break;
    }
  }
}

void Settingsmanager::setRegistryFromControls(const Reference<XHierarchicalPropertySet>& xProperties, const Reference<XControlContainer>& xControlContainer) {
  for (const auto& srec : settings) {
    if (!hasControl(xControlContainer, srec.second.controlName)) {
      MSG_INFO(1, "setRegistryFromControls: control does not exist: " << STR(srec.second.controlName)<< endline);
      continue;
    }

    switch (srec.second.otype) {
      case t_str:
        xProperties->setHierarchicalPropertyValue(srec.second.propertyName, makeAny(getTextcontrol(xControlContainer, srec.second.controlName)));
        break;
      case t_align: // Same as boolean option
      case t_bool:
        xProperties->setHierarchicalPropertyValue(srec.second.propertyName, makeAny(getCheckBox(xControlContainer, srec.second.controlName) == 1));
        break;
      case t_dbl: break;
      case t_uint:
        xProperties->setHierarchicalPropertyValue(srec.second.propertyName, makeAny(getNumericFieldPosInt(xControlContainer, srec.second.controlName)));
        break;
      case t_int:
        xProperties->setHierarchicalPropertyValue(srec.second.propertyName, makeAny(getNumericFieldInt(xControlContainer, srec.second.controlName)));
        break;
      case t_exvec: break;
    }
  }
}

void Settingsmanager::setControlsFromRegistry(const Reference<XHierarchicalPropertySet>& xProperties, const Reference<XControlContainer>& xControlContainer) {
  for (const auto& srec : settings) {
    if (!hasControl(xControlContainer, srec.second.controlName)) {
      MSG_INFO(1, "setControlsFromRegistry: control does not exist: " << STR(srec.second.controlName)<< endline);
      continue;
    }

    switch (srec.second.otype) {
      case t_str: setTextcontrol(xControlContainer, srec.second.controlName, xProperties->getHierarchicalPropertyValue(srec.second.propertyName)); break;
      case t_align: // Same as boolean option
      case t_bool: {
        sal_Bool value = true;
        Any Avalue = xProperties->getHierarchicalPropertyValue(srec.second.propertyName);
        Avalue >>= value;
        setCheckBox(xControlContainer, srec.second.controlName, value);
        break;
      }
      case t_dbl: break;
      case t_uint: {
        long value = 0; // Must use long for the Any cast!
        Any Avalue = xProperties->getHierarchicalPropertyValue(srec.second.propertyName);
        Avalue >>= value;
        setNumericFieldPosInt(xControlContainer, srec.second.controlName, value);
        break;
      }
      case t_int: {
        sal_Int32 value = 0;
        Any Avalue = xProperties->getHierarchicalPropertyValue(srec.second.propertyName);
        Avalue >>= value;
        setNumericFieldInt(xControlContainer, srec.second.controlName, value);
        break;
      }
      case t_exvec: break;
    }
  }
}

void Settingsmanager::setStatementsFromControls(const Reference<XComponentContext>& mxCC, const Reference<XModel>& xModel,
                                                const Reference<XNamedGraph>& xGraph, const Reference<XControlContainer>& xControlContainer) {
for (const auto& srec : settings) {
    if (!hasControl(xControlContainer, srec.second.controlName)) {
      MSG_INFO(1, "setStatementsFromControls: control does not exist: " << STR(srec.second.controlName)<< endline);
      continue;
    }

    switch (srec.second.otype) {
      case t_str:
        updateStatement(mxCC, xModel, xGraph, srec.second.statementName, getTextcontrol(xControlContainer, srec.second.controlName));
        break;
      case t_align: // Same as boolean option
      case t_bool:
        updateStatement(mxCC, xModel, xGraph, srec.second.statementName, (getCheckBox(xControlContainer, srec.second.controlName) == 1) ? OU("true") : OU("false"));
        break;
      case t_dbl: break;
      case t_uint:
        updateStatement(mxCC, xModel, xGraph, srec.second.statementName, OUSTRINGNUMBER(getNumericFieldPosInt(xControlContainer, srec.second.controlName)));
        break;
      case t_int:
        updateStatement(mxCC, xModel, xGraph, srec.second.statementName, OUSTRINGNUMBER(getNumericFieldInt(xControlContainer, srec.second.controlName)));
        break;
      case t_exvec: break;
    }
  }
}

void Settingsmanager::setControlsFromStatements(const Reference<XComponentContext>& mxCC, const Reference<XModel>& xModel,
                                                const Reference<XNamedGraph>& xGraph, const Reference<XControlContainer>& xControlContainer) {
  for (const auto& srec : settings) {
    if (!hasControl(xControlContainer, srec.second.controlName)) {
      MSG_INFO(1, "setControlsFromStatements: control does not exist: " << STR(srec.second.controlName)<< endline);
      continue;
    }

    switch (srec.second.otype) {
      case t_str:
        setTextcontrol(xControlContainer, srec.second.controlName, getStatementString(mxCC, xModel, xGraph, srec.second.statementName));
        break;
      case t_align: // Same as boolean option
      case t_bool:
        setCheckBox(xControlContainer, srec.second.controlName, getStatementBool(mxCC, xModel, xGraph, srec.second.statementName));
        break;
      case t_dbl: break;
      case t_uint:
        setNumericFieldPosInt(xControlContainer, srec.second.controlName, getStatementPosInt(mxCC, xModel, xGraph, srec.second.statementName));
        break;
      case t_int:
        setNumericFieldInt(xControlContainer, srec.second.controlName, getStatementInt(mxCC, xModel, xGraph, srec.second.statementName));
        break;
      case t_exvec: break;
    }
  }
}
