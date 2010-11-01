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

#ifndef SOLTOOLS_GILACCES_HXX
#define SOLTOOLS_GILACCES_HXX



class GenericInfoParseTypes
{
  public:
    enum E_Error
    {
        ok = 0,
        cannot_open,
        unexpected_eof,
        syntax_error,
        unexpected_list_end
    };
};



/** This class is an abstract interface for a service, which
    builds a memory structure out of a generic information
    structure, read from a file or other stream.

    There may be different implementations, which build different kinds
    of memory structures.
*/
class GenericInfoList_Builder
{
  public:
    typedef unsigned long   UINT32;

    virtual             ~GenericInfoList_Builder() {}

    virtual void        AddKey(
                            const char *        i_sKey,
                            UINT32              i_nKeyLength,
                            const char *        i_sValue,
                            UINT32              i_nValueLength,
                            const char *        i_sComment,
                            UINT32              i_nCommentLength ) = 0;

    virtual void        OpenList() = 0;
    virtual void        CloseList() = 0;
};


/** This class is an abstract interface for a service, which
    returns the values of a generic information tree out of
    a memory structure.

    There may be different implementations, which browse different
    kinds of memory structures.
*/
class GenericInfoList_Browser
{
  public:
    virtual             ~GenericInfoList_Browser() {}

    virtual bool        Start_CurList() = 0;
    virtual bool        NextOf_CurList() = 0;

    virtual void        Get_CurKey(
                            char *              o_rKey ) const = 0;
    virtual void        Get_CurValue(
                            char *              o_rValue ) const = 0;
    virtual void        Get_CurComment(
                            char *              o_rComment ) const = 0;
    virtual bool        HasSubList_CurKey() const = 0;

    virtual void        Push_CurList() = 0;
    virtual void        Pop_CurList() = 0;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
