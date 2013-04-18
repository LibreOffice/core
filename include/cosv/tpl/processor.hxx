/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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

}   // namespace csv
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
