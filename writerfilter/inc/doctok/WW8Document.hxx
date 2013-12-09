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

#ifndef INCLUDED_WW8_DOCUMENT_HXX
#define INCLUDED_WW8_DOCUMENT_HXX

#include <boost/shared_ptr.hpp>
#include <sal/types.h>
#include <com/sun/star/uno/Reference.hxx>
#include <resourcemodel/SubSequence.hxx>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <resourcemodel/WW8ResourceModel.hxx>
#include <resourcemodel/OutputWithDepth.hxx>

#include <WriterFilterDllApi.hxx>

namespace writerfilter {
namespace doctok {

using namespace ::com::sun::star;

/**
   A stream containing a WW8 document.

   The content of the stream is a sequence of unsigned bytes. The
   stream consists of substreams that are identified by string
   identifiers.
*/
class WW8Stream
{
public:
    /**
       Pointer to a WW8Stream.
    */
    typedef boost::shared_ptr<WW8Stream> Pointer_t;

    /**
       Type for the content of the stream
     */
    typedef SubSequence<sal_uInt8> Sequence;

    virtual ~WW8Stream();

    /**
       Returns pointer to a substream.

       @param rSid     identifier of substream to return

       @return    the substream
     */
    virtual Pointer_t getSubStream(const OUString & rSid) = 0;

    /**
       Return a continious part of the stream.

       @param nOffset     offset in the stream where the part starts
       @param nCount      length of the part (number of bytes)

       @return    sequence of unsigned bytes
     */
    virtual Sequence get(sal_uInt32 nOffset, sal_uInt32 nCount)
        const = 0;

    // Returns the names of substreams contained in the stream
    virtual string getSubStreamNames() const = 0;

    virtual uno::Sequence<OUString> getSubStreamUNames() const = 0;

    /**
       Dumps content of stream to output.

       @param o     the target output
    */
    virtual void dump(OutputWithDepth<string> & o) const = 0;

    //virtual bool put(sal_uInt32 nOffset, const Sequence & rSeq) = 0;
};

/**
   A property.

*/
class WW8Property
{
public:
    /**
       Ponter to a property.
    */
    typedef boost::shared_ptr<WW8Property> Pointer_t;

    virtual ~WW8Property();

    virtual sal_uInt32 getId() const = 0;
    virtual sal_uInt32 getParam() const = 0;
    virtual WW8Stream::Sequence getParams() const = 0;

    virtual string toString() const = 0;

    /**
       Dumps this object to an output.

    */
    virtual void dump(OutputWithDepth<string> & o) const = 0;
};

/**
   An iterator for traversal of a set of properties.

   Sample code for use of iterator:

   \code
   Return_t function(WW8PropertySet::tPointer pSet)
   {
       do_something;

       WW8PropertySetIterator::tPointer pIt = pSet->begin();
       WW8PropertySetIterator::tPointer pItEnd = pSet->end();

       while ((*pIt) != (*pItEnd))
       {
           do_something();

           ++(*pIt);
       }

       do_something;
   }
   \endcode
*/
class WW8PropertySetIterator
{
public:
    typedef boost::shared_ptr<WW8PropertySetIterator> Pointer_t;

    virtual ~WW8PropertySetIterator();

    /**
       Advance iterator to the next property.
    */
    virtual WW8PropertySetIterator & operator++() = 0;

    /**
       Returns a pointer to the property the iterator references.
    */
    virtual WW8Property::Pointer_t get() const = 0;

    /**
       Checks if the iterator is equal to another one.
    */
    virtual bool equal(const WW8PropertySetIterator & rIt) const = 0;

    /**
       Returns string representation of iterator.
     */
    virtual string toString() const = 0;
};

/**
   Checks if two property set iterators are not equal.
*/
bool operator != (const WW8PropertySetIterator & rA,
                  const WW8PropertySetIterator & rB);

/**
   A set of properties.
*/
class WW8PropertySet
{
public:
    typedef boost::shared_ptr<WW8PropertySet> Pointer_t;

    virtual ~WW8PropertySet();

    /**
       Returns iterator to the start of the set.
     */
    virtual WW8PropertySetIterator::Pointer_t begin() = 0;

    /**
       Returns iterator to the end of the set.
    */
    virtual WW8PropertySetIterator::Pointer_t end() = 0;

    /**
       Dumps property set to output stream.

       @param o    output stream to dump property set to
    */
    virtual void dump(OutputWithDepth<string> & o) const = 0;

    /**
       Iterate through property set and for each element dump a dot
       output stream.

       @param o      output stream to dump dots to
     */
    virtual void dots(ostream & o) = 0;

    virtual bool isPap() const = 0;
    virtual sal_uInt32 get_istd() const = 0;

    /**
       Insert another property set into this property set.

       @param pSet   the set to insert
     */
    virtual void insert(const WW8PropertySet::Pointer_t pSet) = 0;
};

}}

#endif // INCLUDED_WW8_DOCUMENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
