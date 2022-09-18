/***************************************************************************
    alignblock.cxx  -  alignblock - definition file
    facilitate automatic aligning of multiline iFormulas
                             -------------------
    begin                : Tue Jan 29 2019
    copyright            : (C) 2019 by Jan Rheinlaender
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

#include "alignblock.hxx"
#include "iFormulaLine.hxx"
#include "msgdriver.hxx"

void alignblock::clear() {
  resultLines.clear();
  currentResultLine = resultLines.end();
  breaks = 0;
  autochain = false;
  finished = false;
} // clear()

void alignblock::addBreak() {
  for (std::vector<line>::iterator i = resultLines.begin(); i < currentResultLine; i++) {
    if (i->text.trim().getLength() > 0) // Skip empty lines
      i->text += OU(" # {} # {} ");
  }
  breaks++;
} // addBreak()

void alignblock::newline() {
  if (currentResultLine != resultLines.end()) {
    while (currentResultLine->breaks < breaks) {
      currentResultLine->text += OU(" # {} # {} ");
      currentResultLine->breaks++;
    }
  } else { // Start a new line (might be the first line in the block, too)
    resultLines.emplace_back(line());
    currentResultLine = std::prev(resultLines.end());
  }

  currentResultLine++;
} // newline()

void alignblock::addEquation(const OUString& lhs, const OUString& oper, const OUString& rhs) {
  sal_Bool omit_lhs = false;

  // Automatic chaining
  if (autochain && !resultLines.empty()) {
    if ((currentResultLine == resultLines.end()) || (currentResultLine->breaks == 0)) {
      if (std::prev(currentResultLine) >= resultLines.begin())
        // Omit lhs at the beginning of the result line if the previous line has the identical lhs
        omit_lhs = (std::prev(currentResultLine))->lhs.equals(lhs.trim());
    } else if ((currentResultLine != resultLines.end()) && (currentResultLine->breaks > 0)) {
      // Omit lhs for a second (or more) equation on the result line if the previous equation on the line has the identical lhs
      omit_lhs = currentResultLine->lhs.equals(lhs.trim());
    }
  }

  // Add aligning to the equation
  OUString aligned_eq;

  aligned_eq = (omit_lhs ? OU("{} ") : OU("{alignr ") + lhs + OU("}")) +
               OU("#{}") + oper + OU("{}# {alignl ") + rhs + OU("}");
  MSG_INFO(3, "aligned equation: " << STR(aligned_eq) << endline);

  // Add the aligned equation
  if (currentResultLine == resultLines.end()) { // Start a new line (might be the first line in the block, too)
    resultLines.emplace_back(line(lhs, aligned_eq, 1));
    currentResultLine = std::prev(resultLines.end());
    if (resultLines.size() == 1) breaks = 1; // Initialize breaks for first line in block
  } else { // continue an existing line
    if (currentResultLine->text.trim().endsWithAsciiL("#", 1)) {
      currentResultLine->text += aligned_eq;
      currentResultLine->breaks++;
    } else {
      currentResultLine->text += OU(" # {} # ") + aligned_eq;
      currentResultLine->breaks += 2;
    }
    while (currentResultLine->breaks > breaks) addBreak(); // Add more columns to the alignment for all lines preceding the current line
    // we don't touch currentLine->lhs since this has already been set
  }
  MSG_INFO(3,  "addEquation: currentResultLine->text now is: " << STR(currentResultLine->text) << endline);
} // addEquation()

void alignblock::addTextItem(const textItem& item, const iFormulaNodeExpression& l) {
  if (currentResultLine == resultLines.end()) { // Start a new line (might be the first line in the block, too)
    resultLines.emplace_back(line(item.alignedLhs(), item.alignedText(l), item.alignedBreaks()));
    currentResultLine = std::prev(resultLines.end());
    if ((item.alignedBreaks() > 0) && (resultLines.size() == 1)) breaks = item.alignedBreaks();
  } else { // continue an existing line
    currentResultLine->text += item.alignedText(l);
    if (item.alignedBreaks() > 0) {
      currentResultLine->breaks += item.alignedBreaks();
      while (currentResultLine->breaks > breaks) addBreak();
    }
  }
}

void alignblock::finish() {
  MSG_INFO(3,  "alignblock::finish()" << endline);
  finished = true;
  if (currentResultLine == resultLines.end()) return; // Nothing exists that needs to be finished

  while (currentResultLine->breaks < breaks) {
    currentResultLine->text += OU(" # {} # {} ");
    currentResultLine->breaks++;
  }
  MSG_INFO(3,  "alignblock::finish() end" << endline);
} // finish()

OUString alignblock::print() const {
  if (resultLines.empty()) return OU("");

  OUString result = OU("MATRIX { %%gg\n");

  for (std::vector<line>::const_iterator i = resultLines.begin(); i < resultLines.end(); i++)
    result += i->text + (((i + 1 != resultLines.end()) && !i->text.equalsAscii("")) ? OU(" ## %%gg\n") : OU(" %%gg\n"));

  result += OU("}");

  return result;
} // print()

