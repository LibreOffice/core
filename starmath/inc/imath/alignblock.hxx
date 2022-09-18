/***************************************************************************
    alignblock.hxx  -  alignblock - header file
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
#ifndef _ALIGNBLOCK_HXX
#define _ALIGNBLOCK_HXX

#include <memory>
#include <vector>

#include "imathutils.hxx"

class textItem;
class iFormulaNodeExpression;

class alignblock {
public:
  /// Initialize the block
  alignblock() : currentResultLine(resultLines.end()), breaks(0), autochain(false), finished(false) {}
  alignblock(const alignblock& other) = delete;
  alignblock& operator=(const alignblock& other) = delete;
#ifdef DEBUG_CONSTR_DESTR
  ~alignblock() {}
#endif

  /// Add an equation to the current line, or start a new block if there is none
  void addEquation(const OUString& lhs, const OUString& oper, const OUString& rhs);

  /// Add a usertext item to the current line, or start a new block if there is none
  void addTextItem(const textItem& item, const iFormulaNodeExpression& l);

  /// End the current line (user has specified 'newline' in the iFormula text
  void newline();

  /// Finish the alignment block
  void finish();

  /// Clear everything and make a fresh start
  void clear();

  void setAutochain(const bool val) { autochain = val; }

  bool isFinished() const { return finished; }

  /// Are there any lines in the block?
  bool isEmpty() const { return resultLines.empty(); }

  /// Print the block
  OUString print() const;

private:
  struct line {
    OUString lhs;
    OUString text;
    unsigned breaks;
    line(const OUString& l = OU(""), const OUString& t = OU(""), const unsigned b = 0) : lhs(l), text(t), breaks(b) {};
  };

  /// The lines
  std::vector<line> resultLines;

  /// The currently active line
  std::vector<line>::iterator currentResultLine;

  /// The number of alignment breaks (e.g. equal signs) in this block
  unsigned breaks;

  /// Should chaining happen?
  bool autochain;

  /// Has the block been finished?
  bool finished;

  /// Add another alignment break to all lines except the current line
  void addBreak();
};

#endif
