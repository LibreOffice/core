/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OutputWithDepth.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-10 11:32:44 $
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
