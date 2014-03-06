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
    : mnCurrentDepth(0)
    , mnGroupDepth(0)
    , mOpenTag(aOpenTag)
    , mCloseTag(aEndTag)
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
