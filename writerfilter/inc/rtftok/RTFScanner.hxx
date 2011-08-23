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

#ifndef INCLUDED_RTFSCANNER_HXX
#define INCLUDED_RTFSCANNER_HXX

#include <WriterFilterDllApi.hxx>
#include <vector>
#include <rtftok/RTFInputSource.hxx>
#include <rtftok/RTFScannerHandler.hxx>

namespace writerfilter { namespace rtftok {

class WRITERFILTER_DLLPUBLIC RTFScanner {
public:
        RTFScanner(RTFScannerHandler &eventHandler_) : eventHandler(eventHandler_) {};
        virtual ~RTFScanner()	{ }

    const char* YYText()	{ return yytext; }
    int YYLeng()		{ return yyleng; }

    virtual int yylex() = 0;

    int lineno() const		{ return yylineno; }

protected:
    char* yytext;
    int yyleng;
    int yylineno;		// only maintained if you use %option yylineno
     int yy_flex_debug;	// only has effect with -d or "%option debug"
        RTFScannerHandler &eventHandler;


public:
  static writerfilter::rtftok::RTFScanner* createRTFScanner(writerfilter::rtftok::RTFInputSource& inputSource, writerfilter::rtftok::RTFScannerHandler &eventHandler);
};

} } /* end namespace writerfilter::rtftok */


#endif /* INCLUDED_RTFSCANNER_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
