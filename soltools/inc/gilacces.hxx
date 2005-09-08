/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: gilacces.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 07:25:54 $
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

