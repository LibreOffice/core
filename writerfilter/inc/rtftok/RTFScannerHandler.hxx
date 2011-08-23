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

#ifndef INCLUDED_RTFSCANNERHANDLER_HXX
#define INCLUDED_RTFSCANNERHANDLER_HXX

namespace writerfilter { namespace rtftok {

class RTFScannerHandler
{
public:
    virtual void dest(char*token, char*value)=0;
    virtual void ctrl(char*token, char*value)=0;
    virtual void lbrace(void)=0;
    virtual void rbrace(void)=0;
    virtual void addSpaces(int count)=0;
    virtual void addBinData(unsigned char data)=0;
    virtual void addChar(char ch) =0;
    virtual void addCharU(sal_Unicode ch) =0;
    virtual void addHexChar(char* hexch) =0;

};

} } /* end namespace writerfilter::rtftok */

#endif /* INCLUDED_RTFSCANNERHANDLER_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
