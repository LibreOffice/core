/*************************************************************************
 *
 *  $RCSfile: gilacces.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: np $ $Date: 2001-06-11 16:05:17 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

