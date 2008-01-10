/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: exceptions.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-10 11:33:42 $
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

#ifndef _DOCTOK_EXCEPTIONS_HXX
#define _DOCTOK_EXCEPTIONS_HXX

#include <string>
#include <WriterFilterDllApi.hxx>

namespace writerfilter {
using namespace ::std;

class WRITERFILTER_DLLPUBLIC Exception
{
    string mText;

public:
    Exception(string text) : mText(text) {}

    const string & getText() const { return mText; }
};

class WRITERFILTER_DLLPUBLIC ExceptionNotFound : public Exception
{
public:
    ExceptionNotFound(string text) : Exception(text) {}
};

class WRITERFILTER_DLLPUBLIC ExceptionOutOfBounds : public Exception
{
public:
    ExceptionOutOfBounds(string text) : Exception(text) {}
};
}

#endif // _DOCTOK_EXCEPTIONS_HXX
