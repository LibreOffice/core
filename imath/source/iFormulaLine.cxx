/***************************************************************************
    iFormulaLine.cxx  -  iFormulaLine - definition file
    internal representation of an smath formula line in a text document
                             -------------------
    begin                : Sun Feb 5 2012
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

#include <sstream>
#include <regex>
#ifdef INSIDE_SM
#include <imath/msgdriver.hxx>
#include <imath/alignblock.hxx>
#include <imath/func.hxx>
#include <imath/settingsmanager.hxx>
#include <imath/iFormulaLine.hxx>
#else
#include "msgdriver.hxx"
#include "alignblock.hxx"
#include "func.hxx"
#include "settingsmanager.hxx"
#include "iFormulaLine.hxx"
#endif

using namespace GiNaC;

// textItem implementation
std::unique_ptr<textItem> textItem::clone() const {
  return std::make_unique<textItem>(*this);
}

OUString textItem::alignedText(const iFormulaNodeExpression& l) const {
  (void)l;
  if (_text.trim() == OU(""))
    return OU("\"") + _text + OU("\""); // Probably a trailing space, put in quotes to show the problem
  else
    return _text;
}

OUString textItemOperator::alignedText(const iFormulaNodeExpression& l) const {
  (void)l;
  return OU("{} #{} ") + _text.toAsciiUpperCase() + OU(" {}# ");
}

OUString textItemOperator::unalignedText(const iFormulaNodeExpression& l) const {
  (void)l;
  return _text.toAsciiUpperCase();
}

std::unique_ptr<textItem> textItemExpression::clone() const {
  return std::make_unique<textItemExpression>(*this);
}

OUString textItemExpression::alignedText(const iFormulaNodeExpression& l) const {
  return l.printEx(_expr);
}

// iFormulaLine implementation =================================================
iFormulaLine::iFormulaLine(std::vector<OUString>&& formulaParts) :
    _formulaParts(std::move(formulaParts)) {
  MSG_INFO(3,  "Constructing iFormulaLine with formula" << endline);
}

iFormulaLine::iFormulaLine(std::shared_ptr<optionmap> g_options) :
    global_options(g_options) {
  MSG_INFO(3,  "Constructing iFormulaLine with options" << endline);
} // iFormulaLine()

iFormulaLine::iFormulaLine(std::shared_ptr<optionmap> g_options, optionmap&& l_options, std::vector<OUString>&& formulaParts) :
    global_options(g_options), options(std::move(l_options)), _formulaParts(std::move(formulaParts))
{
  MSG_INFO(3,  "Constructing iFormulaLine with global and local options" << endline);
} // iFormulaLine()

iFormulaLine_ptr iFormulaLine::clone() const {
  return std::make_shared<iFormulaLine>(*this);
}

OUString iFormulaLine::print() const {
  return OU("%%ii ") + getCommand() + OU(" ") + getFormula();
}

// We assume that all possible options have values in global_options
const option& iFormulaLine::getOption(const option_name o, const bool force_global) const {
  if (!force_global && hasOption(o)) {
    MSG_INFO(3,  "getOption(): local option found" << endline);
    return options.at(o);
  } else {
    MSG_INFO(3,  "getOption(): global option used" << endline);
    if (global_options->find(o) == global_options->end())
      throw std::runtime_error(std::string("Option #") + std::to_string(o) + " does not exist");
    return global_options->at(o);
  }
} // getOption()

void iFormulaLine::setOptionForce(const option_name name, const option& o) {
  if (!canHaveOptions()) return;
  if (msg::info().checkprio(2)) {
    MSG_INFO(2,  "Forcing line option to '");
    std::ostringstream os;
    o.print(os);
    MSG_INFO(2,  os.str() << "'" << endline);
  }

  if (hasOption(name)) { // option exists already
    option current_option = options.at(name);
    if (msg::info().checkprio(2)) {
      MSG_INFO(2,  "Current local option value is '");
      std::ostringstream os;
      o.print(os);
      MSG_INFO(2,  "'" << endline);
    }

    if (current_option == o) {// has identical value
      MSG_INFO(2,  "Same value, not changing" << endline);
      return;
    } else { // give option the new value
      MSG_INFO(2,  "Setting new local value" << endline);
      options[name] = o;
    }
  } else {
    MSG_INFO(2,  "Adding new local option" << endline);
    options[name] = o;
  }

  changed = true;
} // setOptionForce()

void iFormulaLine::setOption(const option_name name, const option& o) {
  if (!canHaveOptions()) return;

  if (o == global_options->at(name)) {
    MSG_INFO(2,  "Global option has same value, not changing" << endline);
    // Remove the local option if it exists
    if (hasOption(name))
      options.erase(name);
    return;
  }

  setOptionForce(name, o);
} // setOption()

#ifdef _MSC_VER
#include <regex>
#else
#include <regex.h>
#include <unistd.h>
#endif

// Handle decimal point troubles...
OUString adjustLocale(const OUString& s) {
  char dp = imathprint::decimalpoint[0];
  if (dp == '.') return s;
#ifdef _MSC_VER
  // This code can only be compiled with gcc-4.9 or higher and -std=c++11
  //std::regex r("[0-9]*\\.[0-9]*");
  //std::string repl(std::string("$1") + dp + "$2");
  std::regex r("(\\.)([[:digit:]]+)");
  std::string repl = std::string(1, dp) + "$2";
  return OUS8(std::regex_replace(STR(s), r, repl));
#else
  regex_t r;
  regcomp(&r, "\\.[[:digit:]]+", REG_EXTENDED|REG_NEWLINE);
  const int nmatches = 100; // maximum allowed number of matches
  regmatch_t pmatch[nmatches];
  std::string str(STR(s));

  if (regexec(&r, str.c_str(), nmatches, pmatch, 0) == 0) {
    for (unsigned m = 0; m < nmatches; ++m) {
      if (pmatch[m].rm_so == -1)
        break;
      str[pmatch[m].rm_so] = dp;
    }
  }

  regfree(&r);
  return OUS8(str);
#endif
} // adjustLocale()

OUString iFormulaLine::printOptions() const {
  MSG_INFO(3,  "printOptions()" << endline);
  OUString result = Settingsmanager::createOptionString(options);
  if (!result.equalsAscii(""))
    result = result.replaceAt(0,2,OU("{")) + OU("} "); // Removes the leading "; " as a side-effect

  MSG_INFO(3,  "printOptions(): " << STR(result) << endline);
  return result;
} // printOptions()

void iFormulaLine::force_autoformat(const bool value) {
  if (value == true)
    setOptionForce(o_forceautoformat, value);
}

sal_Bool iFormulaLine::autoformat_required() const {
  // Preserving the original formatting of the iFormula is not possible if
  // 1. the user specified the option "autoformat = true"
  // 2. the parser determined that autoformatting is required, and set the option "forceautoformat = true"
  // 3. the user set at least one local formatting options Then we assume that he does not want his formatting
 //     preserved since that would defeat the purpose of setting local formatting options
  if ((hasOption(o_eqraw)) && (options.size() == 1))
    return !options.at(o_eqraw).value.boolean; // Obey user specification

  if (hasOption(o_forceautoformat) && options.at(o_forceautoformat).value.boolean == true)
    return true; // Parser has forced automatic formatting

  return (!getOption(o_eqraw).value.boolean || ((options.find(o_autotextmode) == options.end()) && (options.size() > 0)));
}

std::string iFormulaLine::getGraphLabel() const {
  // return node in dot language
  std::ostringstream address;
  address << (void const *)this;
  // TODO: in and out

  return STR(getCommand()) + address.str();
}

OUString iFormulaLine::getFormula() const {
  OUString formula = OU("");
  for (const auto& p : _formulaParts)
    formula += p;
  return formula;
}

OUString iFormulaLine::printFormula() const {
  return adjustLocale(replaceString(getFormula(), OU("\n%%ii+"), OU("")));
}

void iFormulaLine::setFormula(const OUString& f) {
  _formulaParts = {f};
}

void iFormulaLine::setFormula(std::vector<OUString>&& formulaParts) {
  _formulaParts = std::move(formulaParts);
}

void iFormulaLine::addFormulaPart(const OUString& f) {
  _formulaParts.emplace_back(f);
}

std::set<ex, ex_is_less> collectSymbols(const expression& e) {
  std::set<ex, ex_is_less> result;
  for (const_preorder_iterator i = e.preorder_begin(); i != e.preorder_end(); ++i) {
      if (is_a<symbol>(*i))
        result.emplace(*i);
  }
  return result;
}

// NodeComment
iFormulaNodeComment::iFormulaNodeComment(std::shared_ptr<optionmap> g_options, std::vector<OUString>&& formulaParts) :
  iFormulaLine(g_options, optionmap(), std::move(formulaParts))
{
}

OUString iFormulaNodeComment::print() const {
  return getFormula();
}

// NodeEmptyLine
iFormulaNodeEmptyLine::iFormulaNodeEmptyLine(std::shared_ptr<optionmap> g_options) :
  iFormulaLine(g_options)
{
}

// NodeResult
iFormulaNodeResult::iFormulaNodeResult(const OUString& text) :
    iFormulaLine({text})
{
}

OUString iFormulaNodeResult::print() const {
  return getFormula() + OU(" %%gg");
}

// NodeStatement
iFormulaNodeStatement::iFormulaNodeStatement(std::shared_ptr<optionmap> g_options) :
    iFormulaLine(g_options) {
}

iFormulaNodeStatement::iFormulaNodeStatement(std::shared_ptr<optionmap> g_options, std::vector<OUString>&& formulaParts) :
    iFormulaLine(g_options, optionmap(), std::move(formulaParts)) {
}

OUString iFormulaNodeStatement::print() const {
  return OU("%%ii ") + (options.size() > 0 ? printOptions() + OU(" ") : OU("")) + getCommand() + OU(" ") + getFormula();
}

// NodeStmOptions
iFormulaNodeStmOptions::iFormulaNodeStmOptions(std::shared_ptr<optionmap> g_options, std::vector<OUString>&& formulaParts) :
   iFormulaNodeStatement(g_options, std::move(formulaParts)) {
}

// NodeStmNamespace
iFormulaNodeStmNamespace::iFormulaNodeStmNamespace(std::shared_ptr<optionmap> g_options, const OUString& cmd, std::vector<OUString>&& formulaParts) :
   iFormulaNodeStatement(g_options, std::move(formulaParts)) {
     isBegin = cmd.equalsAscii("BEGIN");
}

// NodeStmFunction
iFormulaNodeStmFunction::iFormulaNodeStmFunction(std::shared_ptr<optionmap> g_options, std::vector<OUString>&& formulaParts) :
   iFormulaNodeStatement(g_options, std::move(formulaParts)) {
  // TODO: function -> out
}

// NodeStmUnitdef
iFormulaNodeStmUnitdef::iFormulaNodeStmUnitdef(std::shared_ptr<optionmap> g_options, std::vector<OUString>&& formulaParts) :
   iFormulaNodeStatement(g_options, std::move(formulaParts)) {
  // TODO: unit -> out?
}

// NodeStmPrefixdef
iFormulaNodeStmPrefixdef::iFormulaNodeStmPrefixdef(std::shared_ptr<optionmap> g_options, std::vector<OUString>&& formulaParts) :
   iFormulaNodeStatement(g_options, std::move(formulaParts)) {
}

// NodeStmVectordef
iFormulaNodeStmVectordef::iFormulaNodeStmVectordef(std::shared_ptr<optionmap> g_options, std::vector<OUString>&& formulaParts) :
   iFormulaNodeStatement(g_options, std::move(formulaParts)) {
  // TODO: vector -> out?
}

// NodeStmMatrixdef
iFormulaNodeStmMatrixdef::iFormulaNodeStmMatrixdef(std::shared_ptr<optionmap> g_options, std::vector<OUString>&& formulaParts) :
   iFormulaNodeStatement(g_options, std::move(formulaParts)) {
  // TODO: matrix -> out?
}

// NodeStmRealvardef
iFormulaNodeStmRealvardef::iFormulaNodeStmRealvardef(std::shared_ptr<optionmap> g_options, std::vector<OUString>&& formulaParts) :
   iFormulaNodeStatement(g_options, std::move(formulaParts)) {
  // TODO: var -> out?
}

// NodeStmPosvardef
iFormulaNodeStmPosvardef::iFormulaNodeStmPosvardef(std::shared_ptr<optionmap> g_options, std::vector<OUString>&& formulaParts) :
   iFormulaNodeStatement(g_options, std::move(formulaParts)) {
  // TODO: var -> out?
}

// NodeStmClearall
iFormulaNodeStmClearall::iFormulaNodeStmClearall(std::shared_ptr<optionmap> g_options) :
    iFormulaNodeStatement(g_options) {
}

// NodeStmDelete
iFormulaNodeStmDelete::iFormulaNodeStmDelete(std::shared_ptr<optionmap> g_options, std::vector<OUString>&& formulaParts) :
   iFormulaNodeStatement(g_options, std::move(formulaParts)) {
}

// NodeStmUpdate
iFormulaNodeStmUpdate::iFormulaNodeStmUpdate(std::shared_ptr<optionmap> g_options, std::vector<OUString>&& formulaParts) :
   iFormulaNodeStatement(g_options, std::move(formulaParts)) {
}

// NodeStmTablecell
iFormulaNodeStmTablecell::iFormulaNodeStmTablecell(std::shared_ptr<optionmap> g_options, std::vector<OUString>&& formulaParts) :
   iFormulaNodeStatement(g_options, std::move(formulaParts)) {
}

// NodeStmCalccell
iFormulaNodeStmCalccell::iFormulaNodeStmCalccell(std::shared_ptr<optionmap> g_options, std::vector<OUString>&& formulaParts) :
   iFormulaNodeStatement(g_options, std::move(formulaParts)) {
}

// NodeStmReadfile
iFormulaNodeStmReadfile::iFormulaNodeStmReadfile(std::shared_ptr<optionmap> g_options, std::vector<OUString>&& formulaParts) :
   iFormulaNodeStatement(g_options, std::move(formulaParts)) {
}

// Node Expression (virtual superclass of Node Ex and Node Eq)
iFormulaNodeExpression::iFormulaNodeExpression(
    Unitmanager* um, std::shared_ptr<optionmap> g_options, optionmap&& l_options,
    std::vector<OUString>&& formulaParts, const OUString& label,
    const expression& expr, const bool hide
  ) : iFormulaLine(g_options, std::move(l_options), std::move(formulaParts)), _label(label), _expr(expr), _hide(hide), unitmgr(um) {
}

iFormulaLine_ptr iFormulaNodeExpression::clone() const {
  return std::make_shared<iFormulaNodeExpression>(*this);
}

OUString iFormulaNodeExpression::print() const {
  return OU("%%ii ") + (_label.getLength() > 0 ? OU("@") + _label + OU("@ ") : OU("")) + printOptions() + getCommand() + (_hide ? OU("* ") : OU(" ")) + getFormula();
}

// Print an expression with the given options and units
OUString iFormulaNodeExpression::printEx(const expression& e) const {
  MSG_INFO(3,  "printEx() for '" << e << "'" << endline);
  // Create a combined list with all global and local units. Local units have precedence, so they must come last
  // Note: create_conversions2() only works properly if ALL units are processed in one go!
  lst units(global_options->at(o_units).value.exvec->begin(), global_options->at(o_units).value.exvec->end());
  if (msg::info().checkprio(3))
      for (const auto& u : units)
        msg::info() <<  "Unit GLOBAL: " << u << endline;

  if (hasOption(o_units)) { // There are units local to this line
    for (const auto& o : *(options.at(o_units).value.exvec))
      units.append(o);

    if (msg::info().checkprio(3))
      for (const auto& u : *(options.at(o_units).value.exvec))
        msg::info() <<  "Unit LOCAL: " << u << endline;
  }

  // Create a combined optionmap with all global and local options. Local options have precedence
  optionmap o = options;
  for (const auto& i : *global_options) {
    if (o.find(i.first) == o.end())
      o.emplace(i.first, i.second); // Local option doesn't exist, use global option instead
  }

  unitvec* conversions = unitmgr->create_conversions2(units, true);
  expression e_subst = subst_units(e, *conversions);
  MSG_INFO(3,  "Substituted: '" << e_subst << "'" << endline);

  // Suppress units
  if (getOption(o_suppress_units).value.boolean) {
    remove_units r_units;
    e_subst = r_units(e_subst);
  }

  // Match differentials
  match_differentials match_diffs;
  ex e_matched;
  if (is_a<equation>(e_subst)) {
    // Work-around because for some reason match_diffs() replaces an equation by a relational, losing the modulus
    const equation& eq = ex_to<equation>(e_subst);
    e_matched = dynallocate<equation>(match_diffs(eq.lhs()), match_diffs(eq.rhs()), eq.getop(), eq.getmod());
  } else {
    e_matched = match_diffs(e_subst);
  }

  MSG_INFO(3, "Matched differentials: '" << e_matched << "'" << endline);

  std::ostringstream os;
  o.emplace(o_basefontheight, basefontheight);
  o.emplace(o_fractionlevel, 0);
  imathprint i(os, &o);
  MSG_INFO(3,  "Created imathprint" << endline);

  if (is_a<Unit>(e_matched))
    numeric(1).print(i, 0); // Top level single unit must print with 1 as coefficient
  e_matched.print(i, 0);
  if (units.nops() > 0 && !getOption(o_suppress_units).value.boolean &&
    (e_matched.is_zero() || (is_a<equation>(e_matched) && ex_to<equation>(e_matched).rhs().is_zero()))) {
    // Special case because GiNaC automatic simplification cancels everything multiplied with zero
    for (const auto& u : units)
      u.print(i);
  }

  MSG_INFO(3,  "printed on stream" << endline);
  delete conversions;
  return OUS8(os.str());
} // printEx()

std::string iFormulaNodeExpression::getGraphLabel() const {
  if (_label.getLength() == 0)
    return iFormulaLine::getGraphLabel();
  return "@" + STR(_label) + "@";
}

// NodeText
iFormulaNodeText::iFormulaNodeText(Unitmanager* um, std::shared_ptr<optionmap> g_options, optionmap&& l_options,
    std::vector<OUString>&& formulaParts,
    std::vector<std::unique_ptr<textItem>>&& textlist) : iFormulaNodeExpression(um, g_options, std::move(l_options), std::move(formulaParts), OU(""), expression(), false),
    _textlist(std::move(textlist))
{
  for (const auto& p : _textlist) {
    textItemExpression* item = dynamic_cast<textItemExpression*>(p.get());
    if (item != nullptr) {
      std::set<ex, ex_is_less> in1 = collectSymbols(item->getExpression());
      in.insert(in1.begin(), in1.end());
    }
  }
}

iFormulaNodeText::iFormulaNodeText(const iFormulaNodeText& other) : iFormulaNodeExpression(other) {
  for (const auto& t : other._textlist)
    _textlist.emplace_back(t->clone());
}

iFormulaLine_ptr iFormulaNodeText::clone() const {
  return std::make_shared<iFormulaNodeText>(*this);
}

void iFormulaNodeText::display(const Reference< XModel >& xModel,
    OUString& unalignedText, const OUString& prev_lhs, alignblock& alignedText, const bool block_alignment) {
  (void)xModel;
  (void)prev_lhs;
  // Do we align and chain?
  bool autoalign = (getOption(o_eqalign).value.align == both);
  alignedText.setAutochain(getOption(o_eqchain).value.boolean);

  // Iterate through the different components of the text
  bool found_operator = false;
  displayedLhs = OU("");

  for (const auto& c : _textlist) {
    if (!autoalign || block_alignment) {
      if (!found_operator && c->isOperator()) {
        displayedLhs = unalignedText;
        found_operator = true;
      }
      unalignedText += c->unalignedText(*this);
    } else if (c->isNewline()) {
      alignedText.newline();
    } else {
      alignedText.addTextItem(*c, *this);
    }
  }
}

unsigned iFormulaNodeText::countLinesWithOperators(bool& have_operator) const {
  unsigned result = 0;

  for (const auto& c : _textlist) {
    if (c->isNewline() && have_operator) {
      result++;
      have_operator = false;
    } else if (c->isOperator()) {
      have_operator = true;
    }
  }

  return result;
}

// Node Ex
iFormulaNodeEx::iFormulaNodeEx(
    Unitmanager* um, std::shared_ptr<optionmap> g_options, optionmap&& l_options,
    std::vector<OUString>&& formulaParts, const OUString& label,
    const expression& expr, const bool hide
  ) : iFormulaNodeExpression(um, g_options, std::move(l_options), std::move(formulaParts), label, expr, hide)
{
  in = collectSymbols(_expr);
}

void iFormulaNodeEx::display(const Reference< XModel >& xModel,
    OUString& unalignedText, const OUString& prev_lhs, alignblock& alignedText, const bool block_alignment) {
  (void)xModel;
  (void)prev_lhs;
  displayedLhs = OU("");
  if (_hide) return;

  // Is raw formatting possible?
  displayedLhs = (autoformat_required()) ?
                    printEx(_expr) : // autoformat
                    printFormula(); // preserve user formatting changing decimal separator according to locale

  // Should the expression/equation be added to the alignblock?
  bool autoalign = (getOption(o_eqalign).value.align == both);
  MSG_INFO(3,  "autoalign: " << (autoalign ? "true" : "false") << endline);

  if (autoalign && !block_alignment) { // add to alignment block
    // The alignl implicitly assumes that expressions always appear on the right hand side of an operator
    alignedText.addTextItem(textItemString(OU("{alignl ") + displayedLhs + OU("}")), *this);
  } else {
    alignedText.finish();
    unalignedText = displayedLhs;
  }
}

// Node Value
iFormulaNodeValue::iFormulaNodeValue(
    Unitmanager* um, std::shared_ptr<optionmap> g_options, optionmap&& l_options,
    std::vector<OUString>&& formulaParts, const OUString& label,
    const expression& expr, const bool hide,
    const expression& lh
  ) : iFormulaNodeExpression(um, g_options, std::move(l_options), std::move(formulaParts), label, expr, hide), _lh(lh)
{
  in = collectSymbols(_lh);
}

iFormulaLine_ptr iFormulaNodeValue::clone() const {
  return std::make_shared<iFormulaNodeValue>(*this);
}

// Node Printval
iFormulaNodePrintval::iFormulaNodePrintval(
    Unitmanager* um, std::shared_ptr<optionmap> g_options, optionmap&& l_options,
    std::vector<OUString>&& formulaParts, const OUString& label,
    const expression& expr, const bool hide,
    const expression& lh,
    const bool algebraic, const bool with
  ) : iFormulaNodeValue(um, g_options, std::move(l_options), std::move(formulaParts), label, expr, hide, lh), _algebraic(algebraic), _with(with) {
}

iFormulaLine_ptr iFormulaNodePrintval::clone() const {
  return std::make_shared<iFormulaNodePrintval>(*this);
}

OUString iFormulaNodePrintval::getCommand() const {
  return OU("PRINT") + (_algebraic ? OU("AVAL") : OU("VAL")) + (_with ? OU("WITH") : OU(""));
}

void iFormulaNodePrintval::display(const Reference< XModel >& xModel,
    OUString& unalignedText, const OUString& prev_lhs, alignblock& alignedText, const bool block_alignment) {
  (void)xModel;
  // This node is never hidden
  displayedLhs = (autoformat_required() ?
     printEx(_lh) :
     adjustLocale(replaceString(_formulaParts[(_with ? 1 : 0)], OU("\n%%ii+"), OU("")))
  );
  OUString rhs = printEx(_expr); // The RHS is always calculated and therefore auto-formatted
  // TODO: What about _label?

  if ((getOption(o_eqalign).value.align == both) && !block_alignment) { // add to alignment block
    alignedText.setAutochain(getOption(o_eqchain).value.boolean); // Chaining only happens inside alignment blocks
    alignedText.addEquation(displayedLhs, OU("="), rhs);
  } else {
    alignedText.finish();

    if (getOption(o_eqchain).value.boolean && prev_lhs.equals(displayedLhs))
      unalignedText = OU("{} = ") + rhs;
    else
      unalignedText = displayedLhs + OU(" = ") + rhs;
  }
}

// Node Explainval
iFormulaNodeExplainval::iFormulaNodeExplainval(
    Unitmanager* um, std::shared_ptr<optionmap> g_options, optionmap&& l_options,
    std::vector<OUString>&& formulaParts, const OUString& label,
    const expression& expr, const bool hide,
    const expression& lh,
    const expression& definition, exhashmap<ex>&& symbols
  ) : iFormulaNodeValue(um, g_options, std::move(l_options), std::move(formulaParts), label, expr, hide, lh), _definition(definition), _symbols(std::move(symbols)) {
}

iFormulaLine_ptr iFormulaNodeExplainval::clone() const {
  return std::make_shared<iFormulaNodeExplainval>(*this);
}

void iFormulaNodeExplainval::display(const Reference< XModel >& xModel,
    OUString& unalignedText, const OUString& prev_lhs, alignblock& alignedText, const bool block_alignment) {
  (void)xModel;
  displayedLhs = (autoformat_required() ? printEx(_lh) : adjustLocale(replaceString(_formulaParts[0], OU("\n%%ii+"), OU(""))));
  OUString rhs = printEx(_expr); // The RHS is always calculated and therefore auto-formatted
  // TODO: What about _label?

  // Prepare the definition string
  exmap variables; // Cannot use exhashmap because subs() doesn't accept it
  std::map<std::string, std::string> replacements;
  for (const auto& s : _symbols) {
    std::string newsym = "@@" + ex_to<symbol>(s.first).get_name() + "@@";
    variables.emplace(s.first, symbol(newsym));
    replacements.emplace(newsym, "(" + STR(printEx(s.second)) + ")");
  }
  std::string defstring = STR(printEx(_definition.subs(variables)));
  for (const auto& r : replacements)
    defstring = std::regex_replace(defstring, std::regex(r.first), r.second);

  if (_hide) displayedLhs = OUS8(defstring);

  if ((getOption(o_eqalign).value.align == both) && !block_alignment) { // add to alignment block
    alignedText.setAutochain(getOption(o_eqchain).value.boolean); // Chaining only happens inside alignment blocks
    if (_hide) {
      alignedText.addEquation(displayedLhs, OU("="), rhs);
    } else if (_definition.is_equal(_lh)) {
      alignedText.addEquation(displayedLhs, OU("="), OUS8(defstring));
      alignedText.addTextItem(textItemOperator("="), *this);
      alignedText.addTextItem(textItemString(rhs), *this);
    } else {
      alignedText.addEquation(displayedLhs, OU("="), printEx(_definition));
      alignedText.addTextItem(textItemOperator("="), *this);
      alignedText.addTextItem(textItemString(OUS8(defstring)), *this);
      alignedText.addTextItem(textItemOperator("="), *this);
      alignedText.addTextItem(textItemString(rhs), *this);
    }
  } else {
    alignedText.finish();
    if (_hide) {
      if (getOption(o_eqchain).value.boolean && prev_lhs.equals(displayedLhs))
        unalignedText = OU("{} = ") + rhs;
      else
        unalignedText = displayedLhs + OU("{} = ") + rhs;
    } else if (_definition.is_equal(_lh)) {
      if (getOption(o_eqchain).value.boolean && prev_lhs.equals(displayedLhs))
        unalignedText = OU("{} = ") + OUS8(defstring) + OU(" = ") + rhs;
      else
        unalignedText = displayedLhs + OU(" = ") + OUS8(defstring) + OU(" = ") + rhs;
    } else {
      if (getOption(o_eqchain).value.boolean && prev_lhs.equals(displayedLhs))
        unalignedText = OU("{} = ") + printEx(_definition) + OU(" = ") + OUS8(defstring) + OU(" = ") + rhs;
      else
        unalignedText = displayedLhs + OU(" = ") + printEx(_definition) + OU(" = ") + OUS8(defstring) + OU(" = ") + rhs;
    }
  }
}

// Node Eq
iFormulaNodeEq::iFormulaNodeEq(
    Unitmanager* um, std::shared_ptr<optionmap> g_options, optionmap&& l_options,
    std::vector<OUString>&& formulaParts, const OUString& label,
    const expression& expr, const bool hide
  ) : iFormulaNodeExpression(um, g_options, std::move(l_options), std::move(formulaParts), label, expr, hide)
{
  ex lhs = ex_to<equation>(_expr).lhs();
  ex rhs = ex_to<equation>(_expr).rhs();
  if (is_a<symbol>(lhs) || is_a<func>(lhs)) {
    in = collectSymbols(rhs);
    out = {lhs};
  } else if (is_a<symbol>(rhs) || is_a<func>(rhs)) {
    in = collectSymbols(lhs);
    out = {rhs};
  } else {
    in = collectSymbols(lhs);
    std::set<ex, ex_is_less> in2 = collectSymbols(rhs);
    in.insert(in2.begin(), in2.end());
    // out = in; // But we should give priority to direct assignments to a single symbol
  }
}

OUString iFormulaNodeEq::print() const {
  return OU("%%ii @") + _label + OU("@ ") + printOptions() + getCommand() + (_hide ? OU("* ") : OU(" ")) + getFormula();
}

void iFormulaNodeEq::display(const Reference< XModel >& xModel,
    OUString& unalignedText, const OUString& prev_lhs, alignblock& alignedText, const bool block_alignment) {
  (void)xModel;
  displayedLhs = OU("");
  if (_hide) return;

  // Is raw formatting possible?
  OUString what = (autoformat_required()) ?
                    printEx(_expr) : // autoformat
                    printFormula(); // preserve user formatting changing decimal separator according to locale

  // TODO: This breaks automatic chaining since labels are always different
  if (getOption(o_showlabels).value.boolean)
      what = OU("(") + _label + OU(")~") + what;

  const equation& eq = ex_to<equation>(_expr);
  OUString oper = OUS8(get_oper(imathprint(), eq.getop(), eq.getmod())).trim();
  int alignpos = what.toAsciiUpperCase().indexOf(oper); // TODO: This is error-prone, better get operator position from parser
  displayedLhs = what.copy(0, alignpos).trim();

  // Should the expression/equation be added to the alignblock?
  if ((getOption(o_eqalign).value.align == both) && !block_alignment) { // add to alignment block
    alignedText.setAutochain(getOption(o_eqchain).value.boolean); // Chaining only happens inside alignment blocks
    alignedText.addEquation(displayedLhs, oper, what.copy(alignpos + oper.getLength()));
  } else {
    alignedText.finish();

    // Omit lhs at the beginning of the result line if the previous line has the identical lhs
    if (getOption(o_eqchain).value.boolean && prev_lhs.equals(displayedLhs))
      unalignedText = OU("{} ") + what.copy(alignpos + oper.getLength());
    else
      unalignedText = what;
  }
}

// Node Const
iFormulaNodeConst::iFormulaNodeConst(
    Unitmanager* um, std::shared_ptr<optionmap> g_options, optionmap&& l_options,
    std::vector<OUString>&& formulaParts, const OUString& label,
    const expression& expr, const bool hide
  ) : iFormulaNodeEq(um, g_options, std::move(l_options), std::move(formulaParts), label, expr, hide) {
}

// Node Funcdef
iFormulaNodeFuncdef::iFormulaNodeFuncdef(
    Unitmanager* um, std::shared_ptr<optionmap> g_options, optionmap&& l_options,
    std::vector<OUString>&& formulaParts, const OUString& label,
    const expression& expr, const bool hide
  ) : iFormulaNodeEq(um, g_options, std::move(l_options), std::move(formulaParts), label, expr, hide) {
}

// Node Vectordef
iFormulaNodeVectordef::iFormulaNodeVectordef(
    Unitmanager* um, std::shared_ptr<optionmap> g_options, optionmap&& l_options,
    std::vector<OUString>&& formulaParts, const OUString& label,
    const expression& expr, const bool hide
  ) : iFormulaNodeEq(um, g_options, std::move(l_options), std::move(formulaParts), label, expr, hide) {
}

// Node Matrixdef
iFormulaNodeMatrixdef::iFormulaNodeMatrixdef(
    Unitmanager* um, std::shared_ptr<optionmap> g_options, optionmap&& l_options,
    std::vector<OUString>&& formulaParts, const OUString& label,
    const expression& expr, const bool hide
  ) : iFormulaNodeEq(um, g_options, std::move(l_options), std::move(formulaParts), label, expr, hide) {
}

// NodeChart
iFormulaNodeChart::iFormulaNodeChart(
    std::shared_ptr<optionmap> g_options, std::vector<OUString>&& formulaParts,
    const OUString& objectName, const unsigned seriesNumber, const OUString& seriesDescription,
    const extsymbol& s, const matrix& xvalues,
    const matrix& yvalues, const expression& yexpression
  ) : iFormulaLine(g_options, optionmap(), std::move(formulaParts)),
      _objectName(objectName), _seriesNumber(seriesNumber), _seriesDescription(seriesDescription),
      _symbol(s), _xvalues(xvalues),
      _yvalues(yvalues), _yexpression(yexpression)
{
  if (_yexpression.is_empty()) {
    if (_xvalues.rows() == 0) {
      _type = y_values;
      in = collectSymbols(_yvalues);
    } else {
      _type = x_and_y_values;
      in = collectSymbols(_xvalues);
      std::set<ex, ex_is_less> iny = collectSymbols(_yvalues);
      in.insert(iny.begin(), iny.end());
    }
  } else {
    _type = x_values_and_y_expression;
    in = collectSymbols(_xvalues);
    std::set<ex, ex_is_less> ine = collectSymbols(_yexpression);
    in.insert(ine.begin(), ine.end());
  }
}

iFormulaLine_ptr iFormulaNodeChart::clone() const {
  return std::make_shared<iFormulaNodeChart>(*this);
}

void iFormulaNodeChart::display(const Reference< XModel >& xModel,
  OUString& unalignedText, const OUString& prev_lhs, alignblock& alignedText, const bool block_alignment) {
  (void)prev_lhs;
  if (isGlobalDocument(xModel)) return; // Access to chart data throws an exception for global documents

  if (_type == y_values)
    setChartData(xModel, _objectName, _yvalues, _seriesNumber);
  else if (_type == x_and_y_values)
    setChartData(xModel, _objectName, _xvalues, _yvalues, _seriesNumber);
  else if (_type == x_values_and_y_expression)
    setChartData(xModel, _objectName, _symbol, _xvalues, _yexpression, _seriesNumber);

  setSeriesDescription(xModel, _objectName, _seriesDescription, _seriesNumber == 1 ? 1 : _seriesNumber - 1);

  (void)unalignedText; (void)alignedText; (void)block_alignment;
}
