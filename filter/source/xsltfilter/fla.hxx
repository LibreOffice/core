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

/*
Function Language for Attributes. See .cxx-File for Syntax. Used in WordML-Filter.
*/
#ifndef __FLA_HXX__
#define __FLA_HXX__

#include <sal/types.h>
#include <rtl/ustring.hxx>
namespace FLA {


class Evaluator 
{
private:
    sal_Unicode environment[100][10];
    sal_Unicode _buf[1024];
    sal_Int32 evalFunc(const sal_Unicode*op, sal_Int32 opLen, sal_Int32 args, sal_Unicode* argv[10], sal_Unicode *result);
    sal_Int32 evalExp(const sal_Unicode *expr, sal_Int32 exprLen, sal_Unicode *buf, sal_Int32 *bufLen);
public:
    const sal_Unicode *eval(const sal_Unicode *expr, sal_Int32 exprLen);
};
}



#endif /* __FLA_HXX__ */
