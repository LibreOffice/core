/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: processor.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 17:43:55 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef CSV_TPL_PROCESSOR_HXX
#define CSV_TPL_PROCESSOR_HXX

// USED SERVICES




namespace csv
{


/** Implements an acyclic visitor pattern. This is the abstract
    base for the classes doing the work (the "visitors").
*/
class ProcessorIfc
{
  public:
    virtual             ~ProcessorIfc() {}
};



/** Implements an acyclic visitor pattern. This is the abstract
    base for the classes to be processed (the "visitables").
*/
class ConstProcessorClient
{
  public:
    virtual             ~ConstProcessorClient() {}

    void                Accept(
                            ProcessorIfc &  io_processor ) const
                            { do_Accept(io_processor); }
  private:
    virtual void        do_Accept(
                            ProcessorIfc &  io_processor ) const = 0;
};

/** Implements an acyclic visitor pattern. This is the abstract
    base for the classes to be processed (the "visitables").
*/
class ProcessorClient
{
  public:
    virtual             ~ProcessorClient() {}

    void                Accept(
                            ProcessorIfc &  io_processor )
                            { do_Accept(io_processor); }
  private:
    virtual void        do_Accept(
                            ProcessorIfc &  io_processor ) = 0;
};





/** Typed base for "visitor" classes, leaving the visited
    object const.

    @see ProcessorIfc
    @see Processor<>
*/
template <typename X, typename R = void>
class ConstProcessor
{
  public:
    virtual             ~ConstProcessor() {}

    R                   Process(
                            const X &           i_object )
                        { return do_Process(i_object ); }
 private:
    virtual R           do_Process(
                            const X &           i_object ) = 0;
};


/** Typed base for "visitor" classes which may change the visited
    object.

    @see ProcessorIfc
    @see ConstProcessor<>
*/
template <typename X, typename R = void>
class Processor
{
  public:
    virtual             ~Processor() {}

    R                   Process(
                            X &                 i_object )
                        { return do_Process(i_object ); }
 private:
    virtual R           do_Process(
                            X &                 i_object ) = 0;
};


template <class C>
inline void
CheckedCall( ProcessorIfc &  io_processor,
             const C &       i_client )
{
    ConstProcessor<C> *
        pProcessor = dynamic_cast< csv::ConstProcessor<C> * >
                        (&io_processor);
    if (pProcessor != 0)
        pProcessor->Process(i_client);
}

template <class C>
inline void
CheckedCall( ProcessorIfc &  io_processor,
             C &             io_client )
{
    Processor<C> *
        pProcessor = dynamic_cast< csv::Processor<C> * >
                        (&io_processor);
    if (pProcessor != 0)
        pProcessor->Process(io_client);
}

template <class C>
inline void
AssertedCall( ProcessorIfc &  io_processor,
              const C &       i_client )
{
    ConstProcessor<C> *
        pProcessor = dynamic_cast< csv::ConstProcessor<C> * >
                        (&io_processor);
    csv_assert( pProcessor != 0
                && "csv::AssertedCall() failed. Processed object did not match processor." );
    pProcessor->Process(i_client);
}

template <class C>
inline void
AssertedCall( ProcessorIfc &  io_processor,
              C &             io_client )
{
    Processor<C> *
        pProcessor = dynamic_cast< csv::Processor<C> * >
                        (&io_processor);
    csv_assert( pProcessor != 0
                && "csv::AssertedCall() failed. Processed object did not match processor." );
    pProcessor->Process(io_client);
}




}   // namespace csv
#endif
