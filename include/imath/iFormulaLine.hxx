/***************************************************************************
    iFormulaLine.hxx  -  iFormulaLine - header file
    internal representation of an smath formula line in a text document
                             -------------------
    begin                : Sun Feb 5 2011
    copyright            : (C) 2012 by Jan Rheinlaender
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
#ifndef _IFORMULALINE_HXX
#define _IFORMULALINE_HXX

#ifdef INSIDE_SM
#include <imath/imathutils.hxx>
#include <imath/eqc.hxx>
#include <imath/option.hxx>
#else
#include "imathutils.hxx"
#include "eqc.hxx"
#include "option.hxx"
#endif

// Mapping of formula types to the index in the list box of the edit formula dialog
enum formulaType {
  formulaTypeEquation = 0,
  formulaTypeExpression = 1,
  formulaTypeConstant = 2,
  formulaTypeFunctionDeclaration = 3,
  formulaTypeFunctionDefinition = 4,
  formulaTypeVectorDeclaration = 5,
  formulaTypeMatrixDeclaration = 6,
  formulaTypeUnit = 7,
  formulaTypeOptions = 8,
  formulaTypeClearall = 9,
  formulaTypeDelete = 10,
  formulaTypeChart = 11,
  formulaTypeText = 12,
  formulaTypePrintval = 13,
  formulaTypeExplainval = 14,
  formulaTypePrefix = 15,
  formulaTypePosvar = 16,
  formulaTypeRealvar = 17,
  formulaTypeTablecell = 18,
  formulaTypeCalccell = 19,
  formulaTypeReadfile = 20,
  formulaTypeComment = 21, // Note: Unused for now
  formulaTypeResult = 22, // Note: Unused for now
  formulaTypeEmptyLine = 23, // Note: Unused for now
  formulaTypeNamespace = 24,
  formulaTypeUpdate = 25
};

enum depType {
  depNone,
  depIn, // Node depends on other nodes
  depOut, // Node influences other nodes
  depInOut, // Both
  depRecalc, // Change in this node requires recalculation of all dependent nodes
  depRedisplay // Change in this node requires re-displaying all dependent nodes
};

/// Class hierarchy for storing user text
class iFormulaLine;
class iFormulaNodeExpression;
typedef std::shared_ptr<iFormulaLine> iFormulaLine_ptr;
typedef std::shared_ptr<iFormulaNodeExpression> iExpression_ptr;

class textItem {
public:
  textItem(const OUString& t = OU("")) : _text(t) {};
  textItem(const std::string& t) : _text(OUS8(t)) {};
  textItem(const textItem&) = default;
  virtual ~textItem() {};
  virtual std::unique_ptr<textItem> clone() const;
  virtual bool isNewline() const { return false; }
  virtual bool isOperator() const { return false; }
  virtual OUString alignedText(const iFormulaNodeExpression& l) const;
  virtual OUString unalignedText(const iFormulaNodeExpression& l) const { return alignedText(l); }
  virtual unsigned alignedBreaks() const { return 0; }
  virtual OUString alignedLhs() const { return OU(""); }
protected:
  OUString _text;
};
class textItemString : public textItem {
public:
  textItemString(const OUString& s) : textItem(s) {};
  textItemString(const std::string& s) : textItem(s) {};
  OUString alignedLhs() const override { return _text; }
};
class textItemOperator : public textItem {
public:
  textItemOperator(const std::string& o) : textItem(o) {};
  bool isOperator() const override { return true; }
  OUString alignedText(const iFormulaNodeExpression& l) const override;
  OUString unalignedText(const iFormulaNodeExpression& l) const override;
  unsigned alignedBreaks() const override { return 1; }
};
class textItemNewline : public textItem {
public:
  textItemNewline() : textItem(OU("newline")) {};
  bool isNewline() const override { return true; }
};
class textItemExpression : public textItem {
public:
  textItemExpression(const GiNaC::expression& e) : _expr(e) {};
  virtual std::unique_ptr<textItem> clone() const override;
  OUString alignedText(const iFormulaNodeExpression& l) const override;
  GiNaC::expression getExpression() const { return _expr; }
private:
  GiNaC::expression _expr;
};

class alignblock;

/// Stores one text line of an iFormula with its type
// Possible line formats:
// for iStatement:
// %%ii command formula
// for iText:
// %%ii options TEXT textlist
// for iExpression:
// %%ii label options command hide formula
// for iEquation: (note that lhs is also set)
// %%ii label options command hide formula
// for iResult:
// text %%gg
// for iComment:
// %% text
class iFormulaLine {
public:
  // Constructors
  iFormulaLine(std::vector<OUString>&& formulaParts);
  iFormulaLine(std::shared_ptr<GiNaC::optionmap> g_options);
  iFormulaLine(std::shared_ptr<GiNaC::optionmap> g_options, GiNaC::optionmap&& l_options, std::vector<OUString>&& formulaParts);
  iFormulaLine(const iFormulaLine&) = default;
  virtual ~iFormulaLine() {
#ifdef DEBUG_CONSTR_DESTR
    MSG_INFO(3, "Destructing iFormulaLine" << endline);
#endif
};
  // Deep copy
  virtual iFormulaLine_ptr clone() const;

  // Textual output
  /// Print the formula as input by the user, without applying any formatting except decimal separator according to locale
  OUString printFormula() const;
  OUString getFormula() const;
  void setFormula(const OUString& f);
  void setFormula(std::vector<OUString>&& formulaParts);
  void addFormulaPart(const OUString& f);

  /// Build the raw text of the line from its components
  virtual OUString print() const;

  /// Display the formula on the controller or onto the aligned / unaligned text
  virtual void display(const Reference< XModel >&, OUString&, const OUString&, alignblock&, const bool) { /* Nothing to display */ };

  // Properties
  /// Set and get the line's global options
  void setGlobalOptions(std::shared_ptr<GiNaC::optionmap> g_options) { global_options = g_options; }
  std::shared_ptr<GiNaC::optionmap> getGlobalOptions() const { return global_options; }
  // Set and get the line's base font height
  void setBasefontHeight(const unsigned h) { basefontheight = h; }
  unsigned getBasefontHeight() const { return basefontheight; }
  /// Return true if the line has local options
  bool hasOptions() const { return options.size() > 0; }
  /// Return the iFormula command
  virtual OUString getCommand() const { return OU(""); }
  /// Return true if this type of line can have (global or local) options defined (e.g. TEXT, EQDEF, EXDEF)
  virtual bool canHaveOptions() const { return false; }
  /// Returns true if this type of line is an expression or equation
  virtual bool isExpression() const { return false; }
  /// Returns true if this type of node is displayable (e.g. not a comment)
  virtual bool isDisplayable() const { return true; }

  /// For filling dialogs
  virtual formulaType getSelectionType() const { return formulaTypeEquation; };

  // Option management
  /// Check if the option exists for this line
  inline bool hasOption(const option_name o) const { return options.find(o) != options.end(); }
  /// Get the value of an option for this line
  const option& getOption(const option_name o, const bool force_global = false) const;
  /// Set the value of an option for this line if its value differs from the global option
  void setOption(const option_name name, const option& o);
  /// Force the value of an option for this line even if its value is the same as the global option
  void setOptionForce(const option_name name, const option& o);

  /// Print the options
  OUString printOptions() const;

  /// Force automatic formatting of this line by inserting the forceautoformat=true option
  void force_autoformat(const bool value);

  /// Is raw formatting possible or must we format the result line ourselves?
  sal_Bool autoformat_required() const;

  // Dependency management
  virtual depType dependencyType() const { return depNone; }
  // Output the dependencies (ancestors and children) of this iFormula in graphviz dot format
  virtual std::string getGraphLabel() const;
  std::set<GiNaC::ex, GiNaC::ex_is_less> getIn() const { return in; }
  std::set<GiNaC::ex, GiNaC::ex_is_less> getOut() const { return out; }

protected:
  /// the iFormula options specified globally for this line
  std::shared_ptr<GiNaC::optionmap> global_options;

  /// the iFormula options specified for this line only
  GiNaC::optionmap options;

  /// The parts of the formula, as entered verbatim by the user
  std::vector<OUString> _formulaParts;

  /// The base font height of the iFormula and the TextContent object
  unsigned basefontheight;

  /// Changed status of this line
  bool changed;

  /// Dependency tracking
  std::set<GiNaC::ex, GiNaC::ex_is_less> in, out;
};

class iFormulaNodeComment : public iFormulaLine {
public:
  iFormulaNodeComment(std::shared_ptr<GiNaC::optionmap> g_options, std::vector<OUString>&& formulaParts);

  virtual OUString print() const override;

  virtual formulaType getSelectionType() const override { return formulaTypeComment; }
  virtual bool isDisplayable() const override { return false; }
};

class iFormulaNodeEmptyLine : public iFormulaLine {
public:
  iFormulaNodeEmptyLine(std::shared_ptr<GiNaC::optionmap> g_options);

  virtual OUString print() const override { return OU(""); }

  virtual formulaType getSelectionType() const override { return formulaTypeEmptyLine; }
  virtual bool isDisplayable() const override { return false; }
};

class iFormulaNodeResult : public iFormulaLine {
public:
  iFormulaNodeResult(const OUString& text);

  virtual OUString print() const override;

  virtual formulaType getSelectionType() const override { return formulaTypeResult; }
  virtual bool isDisplayable() const override { return false; }
};

class iFormulaNodeStatement : public iFormulaLine {
public:
  iFormulaNodeStatement(std::shared_ptr<GiNaC::optionmap> g_options);
  iFormulaNodeStatement(std::shared_ptr<GiNaC::optionmap> g_options, std::vector<OUString>&& formulaParts);
  virtual ~iFormulaNodeStatement() {};

  virtual OUString print() const override;

  virtual formulaType getSelectionType() const override = 0;
  virtual bool canHaveOptions() const { return true; } // Actually only option echo is relevant
  virtual bool isDisplayable() const override { return false; }
  virtual depType dependencyType() const override { return depRecalc; }
};

class iFormulaNodeStmNamespace : public iFormulaNodeStatement {
public:
  iFormulaNodeStmNamespace(std::shared_ptr<GiNaC::optionmap> g_options, const OUString& cmd, std::vector<OUString>&& formulaParts);
  virtual OUString getCommand() const override { return (isBegin ? OU("BEGIN") : OU("END")); }
  virtual formulaType getSelectionType() const override { return formulaTypeNamespace; }
private:
  bool isBegin;
};

class iFormulaNodeStmOptions : public iFormulaNodeStatement {
public:
  iFormulaNodeStmOptions(std::shared_ptr<GiNaC::optionmap> g_options, std::vector<OUString>&& formulaParts);
  virtual OUString getCommand() const override { return OU("OPTIONS"); }
  virtual formulaType getSelectionType() const override { return formulaTypeOptions; }
  // dependencyType could be depRedisplay for all options except realroots
};

class iFormulaNodeStmFunction : public iFormulaNodeStatement {
public:
  iFormulaNodeStmFunction(std::shared_ptr<GiNaC::optionmap> g_options, std::vector<OUString>&& formulaParts);
  virtual OUString getCommand() const override { return OU("FUNCTION"); }
  virtual formulaType getSelectionType() const override { return formulaTypeFunctionDeclaration; }
  // dependencyType recalc e.g. if function name was changed
};

class iFormulaNodeStmUnitdef : public iFormulaNodeStatement {
public:
  iFormulaNodeStmUnitdef(std::shared_ptr<GiNaC::optionmap> g_options, std::vector<OUString>&& formulaParts);
  virtual OUString getCommand() const override { return OU("UNITDEF"); }
  virtual formulaType getSelectionType() const override { return formulaTypeUnit; }
};

class iFormulaNodeStmPrefixdef : public iFormulaNodeStatement {
public:
  iFormulaNodeStmPrefixdef(std::shared_ptr<GiNaC::optionmap> g_options, std::vector<OUString>&& formulaParts);
  virtual OUString getCommand() const override { return OU("PREFIXDEF"); }
  virtual formulaType getSelectionType() const override { return formulaTypePrefix; }
};

class iFormulaNodeStmVectordef : public iFormulaNodeStatement {
public:
  iFormulaNodeStmVectordef(std::shared_ptr<GiNaC::optionmap> g_options, std::vector<OUString>&& formulaParts);
  virtual OUString getCommand() const override { return OU("VECTORDEF"); }
  virtual formulaType getSelectionType() const override { return formulaTypeVectorDeclaration; }
};

class iFormulaNodeStmMatrixdef : public iFormulaNodeStatement {
public:
  iFormulaNodeStmMatrixdef(std::shared_ptr<GiNaC::optionmap> g_options, std::vector<OUString>&& formulaParts);
  virtual OUString getCommand() const override { return OU("MATRIXDEF"); }
  virtual formulaType getSelectionType() const override { return formulaTypeMatrixDeclaration; }
};

class iFormulaNodeStmRealvardef : public iFormulaNodeStatement {
public:
  iFormulaNodeStmRealvardef(std::shared_ptr<GiNaC::optionmap> g_options, std::vector<OUString>&& formulaParts);
  virtual OUString getCommand() const override { return OU("REALVARDEF"); }
  virtual formulaType getSelectionType() const override { return formulaTypeRealvar; }
};

class iFormulaNodeStmPosvardef : public iFormulaNodeStatement {
public:
  iFormulaNodeStmPosvardef(std::shared_ptr<GiNaC::optionmap> g_options, std::vector<OUString>&& formulaParts);
  virtual OUString getCommand() const override { return OU("POSVARDEF"); }
  virtual formulaType getSelectionType() const override { return formulaTypePosvar; }
};

class iFormulaNodeStmClearall : public iFormulaNodeStatement {
public:
  iFormulaNodeStmClearall(std::shared_ptr<GiNaC::optionmap> g_options);
  virtual OUString getCommand() const override { return OU("CLEAREQUATIONS"); }
  virtual formulaType getSelectionType() const override { return formulaTypeClearall; }
};

class iFormulaNodeStmDelete : public iFormulaNodeStatement {
public:
  iFormulaNodeStmDelete(std::shared_ptr<GiNaC::optionmap> g_options, std::vector<OUString>&& formulaParts);
  virtual OUString getCommand() const override { return OU("DELETE"); }
  virtual formulaType getSelectionType() const override { return formulaTypeDelete; }
};

class iFormulaNodeStmUpdate : public iFormulaNodeStatement {
public:
  iFormulaNodeStmUpdate(std::shared_ptr<GiNaC::optionmap> g_options, std::vector<OUString>&& formulaParts);
  virtual OUString getCommand() const override { return OU("UPDATE"); }
  virtual formulaType getSelectionType() const override { return formulaTypeUpdate; }
};

class iFormulaNodeStmTablecell : public iFormulaNodeStatement {
public:
  iFormulaNodeStmTablecell(std::shared_ptr<GiNaC::optionmap> g_options, std::vector<OUString>&& formulaParts);
  virtual OUString getCommand() const override { return OU("SETTABLECELL"); }
  virtual formulaType getSelectionType() const override { return formulaTypeTablecell; }
  virtual depType dependencyType() const override { return depIn; }
};

class iFormulaNodeStmCalccell : public iFormulaNodeStatement {
public:
  iFormulaNodeStmCalccell(std::shared_ptr<GiNaC::optionmap> g_options, std::vector<OUString>&& formulaParts);
  virtual OUString getCommand() const override { return OU("SETCALCCELLS"); }
  virtual formulaType getSelectionType() const override { return formulaTypeCalccell; }
  virtual depType dependencyType() const override { return depIn; }
};

class iFormulaNodeStmReadfile : public iFormulaNodeStatement {
public:
  iFormulaNodeStmReadfile(std::shared_ptr<GiNaC::optionmap> g_options, std::vector<OUString>&& formulaParts);
  virtual OUString getCommand() const override { return OU("READFILE"); }
  virtual formulaType getSelectionType() const override { return formulaTypeReadfile; }
};

class iFormulaNodeExpression : public iFormulaLine {
public:
  iFormulaNodeExpression(
    Unitmanager* um, std::shared_ptr<GiNaC::optionmap> g_options, GiNaC::optionmap&& l_options,
    std::vector<OUString>&& formulaParts, const OUString& label,
    const GiNaC::expression& expr, const bool hide
  );
  virtual iFormulaLine_ptr clone() const override;

  OUString getLabel() const { return _label; }
  void setLabel(const OUString& label) { _label = label; }
  bool getHide() const { return _hide; }
  void setHide(const bool hide) { _hide = hide; }
  GiNaC::expression getExpression() const { return _expr; }

  virtual OUString print() const override;
  /// Print an expression with options and units of the node
  OUString printEx(const GiNaC::expression& e) const;

  // Note that the displayedLhs is set by display() and is always empty before
  virtual OUString getDisplayedLhs() const { return displayedLhs; }
  virtual unsigned countLinesWithOperators(bool& have_operator) const { have_operator = false; return 0; }
  virtual bool canHaveOptions() const override { return true; }
  virtual bool isExpression() const override { return true; }

  virtual std::string getGraphLabel() const override;
  virtual depType dependencyType() const override { return depIn; }
protected:
  OUString _label;
  GiNaC::expression _expr;
  bool _hide;
  Unitmanager* unitmgr;
  OUString displayedLhs; // Remember the displayed LHS (up to the operator, if there is one), for chaining of formulas
};

class iFormulaNodeText : public iFormulaNodeExpression {
public:
  iFormulaNodeText(
    Unitmanager* um, std::shared_ptr<GiNaC::optionmap> g_options, GiNaC::optionmap&& l_options,
    std::vector<OUString>&& formulaParts, std::vector<std::unique_ptr<textItem>>&& textlist);
  iFormulaNodeText(const iFormulaNodeText& other);
  virtual iFormulaLine_ptr clone() const override;

  virtual void display(const Reference< XModel >& xModel,
    OUString& unalignedText, const OUString& prev_lhs, alignblock& alignedText, const bool block_alignment) override;
  virtual unsigned countLinesWithOperators(bool& have_operator) const override;

  virtual OUString getCommand() const { return OU("TEXT"); }
  virtual bool isExpression() const override { return false; }
  virtual formulaType getSelectionType() const override { return formulaTypeText; }

private:
  /// List of arbitrary user text, operators, quoted strings, newlines and expressions
  std::vector<std::unique_ptr<textItem>> _textlist;
};

class iFormulaNodeEx : public iFormulaNodeExpression {
public:
  iFormulaNodeEx(
    Unitmanager* um, std::shared_ptr<GiNaC::optionmap> g_options, GiNaC::optionmap&& l_options,
    std::vector<OUString>&& formulaParts, const OUString& label,
    const GiNaC::expression& expr, const bool hide
  );

  virtual void display(const Reference< XModel >& xModel,
    OUString& unalignedText, const OUString& prev_lhs, alignblock& alignedText, const bool block_alignment) override;

  virtual OUString getCommand() const override { return OU("EXDEF"); }
  virtual formulaType getSelectionType() const override { return formulaTypeExpression; }
};

class iFormulaNodeValue : public iFormulaNodeExpression {
public:
  iFormulaNodeValue(
    Unitmanager* um, std::shared_ptr<GiNaC::optionmap> g_options, GiNaC::optionmap&& l_options,
    std::vector<OUString>&& formulaParts, const OUString& label,
    const GiNaC::expression& expr, const bool hide,
    const GiNaC::expression& lh
  );
  virtual iFormulaLine_ptr clone() const override;

  virtual unsigned countLinesWithOperators(bool& have_operator) const { have_operator = true; return 0; }
  virtual bool canHaveOptions() const override { return true; }
  virtual bool isExpression() const { return true; }

protected:
  GiNaC::expression _lh;
};

class iFormulaNodePrintval : public iFormulaNodeValue {
public:
  iFormulaNodePrintval(
    Unitmanager* um, std::shared_ptr<GiNaC::optionmap> g_options, GiNaC::optionmap&& l_options,
    std::vector<OUString>&& formulaParts, const OUString& label,
    const GiNaC::expression& expr, const bool hide,
    const GiNaC::expression& lh,
    const bool algebraic = false, const bool with = false
  );
  virtual iFormulaLine_ptr clone() const override;

  virtual void display(const Reference< XModel >& xModel,
    OUString& unalignedText, const OUString& prev_lhs, alignblock& alignedText, const bool block_alignment) override;

  virtual OUString getCommand() const override;
  virtual formulaType getSelectionType() const override { return formulaTypePrintval; }
private:
  bool _algebraic;
  bool _with;
};

class iFormulaNodeExplainval : public iFormulaNodeValue {
public:
  iFormulaNodeExplainval(
    Unitmanager* um, std::shared_ptr<GiNaC::optionmap> g_options, GiNaC::optionmap&& l_options,
    std::vector<OUString>&& formulaParts, const OUString& label,
    const GiNaC::expression& expr, const bool hide,
    const GiNaC::expression& lh,
    const GiNaC::expression& definition, GiNaC::exhashmap<GiNaC::ex>&& symbols
  );
  virtual iFormulaLine_ptr clone() const override;

  virtual void display(const Reference< XModel >& xModel,
    OUString& unalignedText, const OUString& prev_lhs, alignblock& alignedText, const bool block_alignment) override;

  virtual OUString getCommand() const override { return OU("EXPLAINVAL"); }
  virtual formulaType getSelectionType() const override { return formulaTypeExplainval; }

private:
  GiNaC::expression _definition;
  GiNaC::exhashmap<GiNaC::ex> _symbols;
};

class iFormulaNodeEq : public iFormulaNodeExpression {
public:
  iFormulaNodeEq(
    Unitmanager* um, std::shared_ptr<GiNaC::optionmap> g_options, GiNaC::optionmap&& l_options,
    std::vector<OUString>&& formulaParts, const OUString& label,
    const GiNaC::expression& expr, const bool hide
  );
  virtual ~iFormulaNodeEq() {};

  virtual OUString print() const override;
  virtual void display(const Reference< XModel >& xModel,
    OUString& unalignedText, const OUString& prev_lhs, alignblock& alignedText, const bool block_alignment) override;

  virtual OUString getCommand() const override { return OU("EQDEF"); }
  virtual formulaType getSelectionType() const override { return formulaTypeEquation; }
  virtual unsigned countLinesWithOperators(bool& have_operator) const { have_operator = true; return 0; }
  virtual bool canHaveOptions() const override { return true; }
  virtual bool isExpression() const { return true; }
  virtual depType dependencyType() const override { return depInOut; }
};

class iFormulaNodeConst : public iFormulaNodeEq {
public:
  iFormulaNodeConst(
    Unitmanager* um, std::shared_ptr<GiNaC::optionmap> g_options, GiNaC::optionmap&& l_options,
    std::vector<OUString>&& formulaParts, const OUString& label,
    const GiNaC::expression& expr, const bool hide
  );

  virtual OUString getCommand() const override { return OU("CONSTDEF"); }
  virtual formulaType getSelectionType() const override { return formulaTypeConstant; }
};

class iFormulaNodeFuncdef : public iFormulaNodeEq {
public:
  iFormulaNodeFuncdef(
    Unitmanager* um, std::shared_ptr<GiNaC::optionmap> g_options, GiNaC::optionmap&& l_options,
    std::vector<OUString>&& formulaParts, const OUString& label,
    const GiNaC::expression& expr, const bool hide
  );

  virtual OUString getCommand() const override { return OU("FUNCDEF"); }
  virtual formulaType getSelectionType() const override { return formulaTypeFunctionDefinition; }
};

class iFormulaNodeVectordef : public iFormulaNodeEq {
public:
  iFormulaNodeVectordef(
    Unitmanager* um, std::shared_ptr<GiNaC::optionmap> g_options, GiNaC::optionmap&& l_options,
    std::vector<OUString>&& formulaParts, const OUString& label,
    const GiNaC::expression& expr, const bool hide
  );

  virtual OUString getCommand() const override { return OU("VECTORDEF"); }
};

class iFormulaNodeMatrixdef : public iFormulaNodeEq {
public:
  iFormulaNodeMatrixdef(
    Unitmanager* um, std::shared_ptr<GiNaC::optionmap> g_options, GiNaC::optionmap&& l_options,
    std::vector<OUString>&& formulaParts, const OUString& label,
    const GiNaC::expression& expr, const bool hide
  );

  virtual OUString getCommand() const override { return OU("MATRIXDEF"); }
};

class iFormulaNodeChart : public iFormulaLine {
public:
  iFormulaNodeChart(
    std::shared_ptr<GiNaC::optionmap> g_options, std::vector<OUString>&& formulaParts,
    const OUString& objectName, const unsigned seriesNumber, const OUString& seriesDescription,
    const GiNaC::extsymbol& _s, const GiNaC::matrix& xvalues,
    const GiNaC::matrix& yvalues, const GiNaC::expression& yexpression
  );
  virtual iFormulaLine_ptr clone() const override;

  virtual void display(const Reference< XModel >& xModel,
    OUString& unalignedText, const OUString& prev_lhs, alignblock& alignedText, const bool block_alignment) override;

  virtual OUString getCommand() const override { return OU("CHART"); }
  virtual formulaType getSelectionType() const override { return formulaTypeChart; }
  virtual bool isDisplayable() const override { return true; } // Displaying the node updates the chart object
  virtual depType dependencyType() const override { return depIn; }

private:
  enum dataType {
    y_values,
    x_and_y_values,
    x_values_and_y_expression
  } _type;
  OUString _objectName;
  unsigned _seriesNumber;
  OUString _seriesDescription;
  GiNaC::extsymbol _symbol;
  GiNaC::matrix _xvalues;
  GiNaC::matrix _yvalues;
  GiNaC::expression _yexpression;
};

#endif // _IFORMULALINE_HXX
