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

#ifndef INCLUDED_SUB_SEQUENCE_HXX
#define INCLUDED_SUB_SEQUENCE_HXX

#include <com/sun/star/uno/Sequence.hxx>

#include <boost/shared_ptr.hpp>
#include <iostream>
#include <stdio.h>
#include <ctype.h>
#include "exceptions.hxx"
#include <WriterFilterDllApi.hxx>

namespace writerfilter {
using namespace ::std;

template <class T>
class SubSequence;

template <class T>
class SubSequence
{
    typedef boost::shared_ptr<com::sun::star::uno::Sequence<T> >
    SequencePointer;

    SequencePointer mpSequence;
    sal_uInt32 mnOffset;
    sal_uInt32 mnCount;

public:
    typedef boost::shared_ptr<SubSequence> Pointer_t;

    SubSequence() : mpSequence(new ::com::sun::star::uno::Sequence<T>()),
                    mnOffset(0), mnCount(0)
    {
    }

    SubSequence(SequencePointer pSequence, sal_uInt32 nOffset_,
                sal_uInt32 nCount_)
        : mpSequence(pSequence), mnOffset(nOffset_), mnCount(nCount_)
    {
    }

    SubSequence(SequencePointer pSequence)
        : mpSequence(pSequence), mnOffset(0), mnCount(pSequence->getLength())
    {
    }

    SubSequence(const SubSequence & rSubSequence, sal_uInt32 nOffset_,
                sal_uInt32 nCount_)
        : mpSequence(rSubSequence.mpSequence),
          mnOffset(rSubSequence.mnOffset + nOffset_),
          mnCount(nCount_)
    {
    }

    SubSequence(const T * pStart, sal_uInt32 nCount_)
        : mpSequence(new com::sun::star::uno::Sequence<T>(pStart, nCount_)),
          mnOffset(0), mnCount(nCount_)
    {
    }

    SubSequence(sal_Int32 nCount_)
        : mpSequence(new com::sun::star::uno::Sequence<T>(nCount_)), mnOffset(0),
          mnCount(nCount_)
    {
    }

    ::com::sun::star::uno::Sequence<T> & getSequence()
    {
        return *mpSequence;
    }

    const ::com::sun::star::uno::Sequence<T> & getSequence() const
    {
        return *mpSequence;
    }

    void reset() {
        mnOffset = 0;
        mnCount = mpSequence->getLength();
    }

    sal_uInt32 getOffset() const { return mnOffset; }
    sal_uInt32 getCount() const { return mnCount; }

    const T & operator[] (sal_uInt32 nIndex) const
    {
        if (mnOffset + nIndex >=
            sal::static_int_cast<sal_uInt32>(mpSequence->getLength()))
            throw ExceptionOutOfBounds("SubSequence::operator[]");

        return (*mpSequence)[mnOffset + nIndex];
    }

    void dump(ostream & o) const
    {
        {
            char sBuffer[256];

            snprintf(sBuffer, sizeof(sBuffer),
                     "<sequence id='%p' offset='%lx' count='%lx'>",
                     mpSequence.get(), mnOffset, mnCount);
            o << sBuffer << endl;
        }

        sal_uInt32 n = 0;
        sal_uInt32 nStep = 16;

        while (n < getCount())
        {
            char sBuffer[256];

            o << "<line>";

            snprintf(sBuffer, 255, "%08lx: ", static_cast<unsigned long>(n));

            o << sBuffer;

            for (sal_uInt32 i = 0; i < nStep; i++)
            {
                if (n + i < getCount())
                {
                    snprintf(sBuffer, 255, "%02x ", operator[](n + i));
                    o << sBuffer;
                }
                else
                    o << "   ";

                if (i % 8 == 7)
                    o << " ";
            }

            {
                for (sal_uInt32 i = 0; i < nStep; i++)
                {
                    if (n + i < getCount())
                    {
                        unsigned char c =
                            static_cast<unsigned char>(operator[](n + i));

                        if (c=='&')
                            o << "&amp;";
                        else if (c=='<')
                            o << "&lt;";
                        else if (c=='>')
                            o << "&gt;";
                        else if (c < 128 && isprint(c))
                            o << c;
                        else
                            o << ".";
                    }

                }
            }

            o << "</line>" << endl;

            n += nStep;
        }

        o << "</sequence>" << endl;
    }

    string toString() const
    {
        sal_uInt32 n = 0;
        sal_uInt32 nStep = 16;

        string sResult;

        while (n < getCount())
        {
            char sBuffer[256];

            snprintf(sBuffer, 255, "<line>%08" SAL_PRIxUINT32 ": ", n);

            sResult += sBuffer;

            for (sal_uInt32 i = 0; i < nStep; i++)
            {
                if (n + i < getCount())
                {
                    snprintf(sBuffer, 255, "%02x ", operator[](n + i));
                    sResult += sBuffer;
                }
                else
                    sResult += "   ";

                if (i % 8 == 7)
                    sResult += " ";
            }

            {
                for (sal_uInt32 i = 0; i < nStep; i++)
                {
                    if (n + i < getCount())
                    {
                        unsigned char c =
                            static_cast<unsigned char>(operator[](n + i));

                        if (c=='&')
                            sResult += "&amp;";
                        else if (c=='<')
                            sResult += "&lt;";
                        else if (c=='>')
                            sResult += "&gt;";
                        else if (c < 128 && isprint(c))
                            sResult += c;
                        else
                            sResult += ".";
                    }
                }
            }

            sResult += "</line>\n";

            n += nStep;
        }

        return sResult;
    }
};

}

#endif // INCLUDED_SUB_SEQUENCE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
