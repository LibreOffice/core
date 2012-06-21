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
