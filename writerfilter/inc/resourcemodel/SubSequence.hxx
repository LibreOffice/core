/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef INCLUDED_SUB_SEQUENCE_HXX
#define INCLUDED_SUB_SEQUENCE_HXX

#include <com/sun/star/uno/Sequence.hxx>

#include <boost/shared_ptr.hpp>
#include <iostream>
#include <stdio.h>
#include <ctype.h>
#include "exceptions.hxx"
#include <WriterFilterDllApi.hxx>
#include <resourcemodel/OutputWithDepth.hxx>

namespace writerfilter {
using namespace ::std;

template <class T>
class SubSequence;

template <typename T>
void dumpLine(OutputWithDepth<string> & o, SubSequence<T> & rSeq, 
              sal_uInt32 nOffset, sal_uInt32 nStep);

template <class T>
class WRITERFILTER_DLLPUBLIC SubSequence
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

            snprintf(sBuffer, 255, "%08lx: ", n);

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

    void dump(OutputWithDepth<string> & o)
    {
        {
            char sBuffer[256];
            
            snprintf(sBuffer, sizeof(sBuffer), 
                     "<sequence id='%p' offset='%" SAL_PRIxUINT32 "' count='%" SAL_PRIxUINT32 "'>",
                     mpSequence.get(), mnOffset, mnCount);
            o.addItem(sBuffer);
        }
        
        sal_uInt32 n = 0;
        sal_uInt32 nStep = 16;
        
        try 
        {
            sal_uInt32 nCount = getCount();
            while (n < nCount)
            {
                sal_uInt32 nBytes = nCount - n;
                
                if (nBytes > nStep)
                    nBytes = nStep;
                
                SubSequence<T> aSeq(*this, n, nBytes);
                dumpLine(o, aSeq, n, nStep);
                
                n += nBytes;
            }
        }
        catch (...)
        {
            o.addItem("<exception/>");
        }
    
        o.addItem("</sequence>");
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

template <typename T>
void dumpLine(OutputWithDepth<string> & o, SubSequence<T> & rSeq,
              sal_uInt32 nOffset, sal_uInt32 nStep)
{
    sal_uInt32 nCount = rSeq.getCount();
    char sBuffer[256];

    string tmpStr = "<line>";

    snprintf(sBuffer, 255, "%08" SAL_PRIxUINT32 ": ", nOffset);

    tmpStr += sBuffer;

    for (sal_uInt32 i = 0; i < nStep; i++)
    {
        if (i < nCount)
        {
            snprintf(sBuffer, 255, "%02x ", rSeq[i]);
            tmpStr += sBuffer;
        }
        else
            tmpStr += "   ";

        if (i % 8 == 7)
            tmpStr += " ";
    }

    {
        for (sal_uInt32 i = 0; i < nStep; i++)
        {
            if (i < nCount)
            {
                unsigned char c =
                    static_cast<unsigned char>(rSeq[i]);

                if (c=='&')
                    tmpStr += "&amp;";
                else if (c=='<')
                    tmpStr += "&lt;";
                else if (c=='>')
                    tmpStr += "&gt;";
                else if (c < 128 && isprint(c))
                    tmpStr += c;
                else
                    tmpStr += ".";
            }

        }
    }

    tmpStr += "</line>";

    o.addItem(tmpStr);
}

}

#endif // INCLUDED_SUB_SEQUENCE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
