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

#ifndef INCLUDED_OUTPUT_WITH_DEPTH
#define INCLUDED_OUTPUT_WITH_DEPTH

#include <vector>
#include <iostream>

namespace writerfilter
{

using namespace ::std;

template <typename T>
class OutputWithDepth
{
    typedef ::std::vector<T> Group_t;
    Group_t mGroup;
    
    unsigned int mnCurrentDepth;
    unsigned int mnGroupDepth;

    T mOpenTag;
    T mCloseTag;

protected:
    virtual void output(const T & aItem) const = 0;
    void outputGroup();
    void finalize();

public:
    OutputWithDepth(const T & aOpenTag, const T & aCloseTag);
    virtual ~OutputWithDepth();

    void openGroup();
    void closeGroup();
    void addItem(const T & aItem);
    void setDepth(unsigned int nDepth);
};

template <typename T>
OutputWithDepth<T>::OutputWithDepth(const T & aOpenTag, const T & aEndTag)
: mOpenTag(aOpenTag), mCloseTag(aEndTag)
{
}

template <typename T>
OutputWithDepth<T>::~OutputWithDepth()
{
}

template <typename T>
void OutputWithDepth<T>::finalize()
{
    outputGroup();
}

template <typename T>
void OutputWithDepth<T>::openGroup()
{
    outputGroup();
    mnGroupDepth = 0;
}

template <typename T>
void OutputWithDepth<T>::closeGroup()
{
    if (mnGroupDepth > mnCurrentDepth)
        for (unsigned int i = 0; i < mnGroupDepth - mnCurrentDepth; ++i)
            output(mOpenTag);
    else if (mnGroupDepth < mnCurrentDepth)
        for (unsigned int i = 0; i < mnCurrentDepth - mnGroupDepth; ++i)
            output(mCloseTag);

    outputGroup();

    mnCurrentDepth = mnGroupDepth;
}

template <typename T>
void OutputWithDepth<T>::addItem(const T & aItem)
{
    mGroup.push_back(aItem);
}

template <typename T>
void OutputWithDepth<T>::setDepth(unsigned int nDepth)
{
    mnGroupDepth = nDepth;
}

template <typename T>
void OutputWithDepth<T>::outputGroup()
{
    typename Group_t::iterator aItEnd = mGroup.end();
    
    for (typename Group_t::iterator aIt = mGroup.begin(); aIt != aItEnd; aIt++)
    {
        output(*aIt);
    }

    mGroup.clear();
}
}
#endif // INCLUDED_OUTPUT_WITH_DEPTH
