/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dht_processor.hxx,v $
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

#ifndef ARY_DHT_PROCESSOR_HXX
#define ARY_DHT_PROCESSOR_HXX

// BASE CLASSES
#include <cosv/tpl/processor.hxx>

// USED SERVICES
#include <ary/doc/d_hypertext.hxx>




namespace ary
{
namespace doc
{
namespace ht
{
    class PlainText;
    class HtmlText;

    class Link2Ce;
    class Link2Url;
    class Link2Unknown;
    class Link2Param;

    class Blank;
    class Indentation;
    class NewParagraph;
    class SummarySeparator;


/** Interface for processing {->Component}s of a ->HyperText with
    the visitor pattern.
*/
class Processor :   public csv::ProcessorIfc,
                    public csv::ConstProcessor<HyperText>
{
  public:
    virtual             ~Processor() {}

    using csv::ConstProcessor<HyperText>::Process;

    // Texts
    void                Process(
                            const PlainText &   i_interpreter,
                            const String &      i_data );
    void                Process(
                            const HtmlText &    i_interpreter,
                            const String &      i_data );
    // Links
    void                Process(
                            const Link2Ce &     i_interpreter,
                            const String &      i_data );
    void                Process(
                            const Link2Url &    i_interpreter,
                            const String &      i_data );
    void                Process(
                            const Link2Unknown &
                                                i_interpreter,
                            const String &      i_data );
    void                Process(
                            const Link2Param &  i_interpreter,
                            const String &      i_data );
    // Explicit Whitespace
    void                Process(
                            const Blank &       i_interpreter );
    void                Process(
                            const Indentation & i_interpreter,
                            const String &      i_data );
    void                Process(
                            const NewParagraph &
                                                i_interpreter );
    void                Process(
                            const SummarySeparator &
                                                i_interpreter );
  private:
    // Interface csv::ConstProcessor<HyperText>:
    virtual void        do_Process(
                            const HyperText &   i_cient );
      // Locals
    virtual void        do_Process(
                            const PlainText &   i_interpreter,
                            const String &      i_data ) = 0;
    virtual void        do_Process(
                            const HtmlText &    i_interpreter,
                            const String &      i_data ) = 0;
    virtual void        do_Process(
                            const Link2Ce &     i_interpreter,
                            const String &      i_data ) = 0;
    virtual void        do_Process(
                            const Link2Url &    i_interpreter,
                            const String &      i_data ) = 0;
    virtual void        do_Process(
                            const Link2Unknown &
                                                i_interpreter,
                            const String &      i_data ) = 0;
    virtual void        do_Process(
                            const Link2Param &  i_interpreter,
                            const String &      i_data ) = 0;
    virtual void        do_Process(
                            const Blank &       i_interpreter ) = 0;
    virtual void        do_Process(
                            const Indentation & i_interpreter,
                            const String &      i_data ) = 0;
    virtual void        do_Process(
                            const NewParagraph &
                                                i_interpreter ) = 0;
    virtual void        do_Process(
                            const SummarySeparator &
                                                i_interpreter ) = 0;
};



// IMPLEMENTATION
inline void
Processor::Process( const PlainText &   i_interpreter,
                    const String &      i_data )
{
    do_Process(i_interpreter, i_data);
}

inline void
Processor::Process( const HtmlText &    i_interpreter,
                    const String &      i_data )
{
    do_Process(i_interpreter, i_data);
}

inline void
Processor::Process( const Link2Ce &     i_interpreter,
                    const String &      i_data )
{
    do_Process(i_interpreter, i_data);
}

inline void
Processor::Process( const Link2Url &    i_interpreter,
                    const String &      i_data )
{
    do_Process(i_interpreter, i_data);
}

inline void
Processor::Process( const Link2Unknown &    i_interpreter,
                    const String &          i_data )
{
    do_Process(i_interpreter, i_data);
}

inline void
Processor::Process( const Link2Param &  i_interpreter,
                    const String &      i_data )
{
    do_Process(i_interpreter, i_data);
}

inline void
Processor::Process( const Blank &   i_interpreter )
{
    do_Process(i_interpreter);
}

inline void
Processor::Process( const Indentation & i_interpreter,
                    const String &      i_data )
{
    do_Process(i_interpreter, i_data);
}

inline void
Processor::Process( const NewParagraph &    i_interpreter )
{
    do_Process(i_interpreter);
}

inline void
Processor::Process( const SummarySeparator &    i_interpreter )
{
    do_Process(i_interpreter);
}




}   // namespace ht
}   // namespace doc
}   // namespace ary
#endif
