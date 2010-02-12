/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
