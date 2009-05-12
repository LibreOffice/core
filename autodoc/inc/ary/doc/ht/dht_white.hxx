/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dht_white.hxx,v $
 * $Revision: 1.3 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef ARY_DHT_WHITE_HXX
#define ARY_DHT_WHITE_HXX

// BASE CLASSES
#include <ary/doc/ht/dht_interpreter.hxx>




namespace ary
{
namespace doc
{
namespace ht
{
    class Component;


/** One blank character in documentation text.
*/
class Blank : public Interpreter
{
  public:
    virtual             ~Blank();

    static Component    Create_Component_();

  private:
    // Interface Interpreter:
    virtual void        do_Accept(
                            Processor &         io_processor,
                            const String &      i_data ) const;
};


/** New paragraph in documentation text.
*/
class NewParagraph : public Interpreter
{
  public:
    virtual             ~NewParagraph();

    static Component    Create_Component_();

  private:
    // Interface Interpreter:
    virtual void        do_Accept(
                            Processor &         io_processor,
                            const String &      i_data ) const;
};

/** An indentation, beginning a line, in documentation text.
*/
class Indentation : public Interpreter
{
  public:
    virtual             ~Indentation();

    static Component    Create_Component_(
                            uintt               i_size );
  private:
    // Interface Interpreter:
    virtual void        do_Accept(
                            Processor &         io_processor,
                            const String &      i_data ) const;
};


/** The point where the first paragraph ends. Needed, if this text
    has to work as a summary for containing the documentation block.
*/
class SummarySeparator: public Interpreter
{
  public:
    virtual             ~SummarySeparator();

    static Component    Create_Component_(
                            bool                i_interrupted );
  private:
    // Interface Interpreter:
    virtual void        do_Accept(
                            Processor &         io_processor,
                            const String &      i_data ) const;
};




}   // namespace ht
}   // namespace doc
}   // namespace ary
#endif
