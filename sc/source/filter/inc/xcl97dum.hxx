/*************************************************************************
 *
 *  $RCSfile: xcl97dum.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: dr $ $Date: 2001-02-26 06:51:25 $
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

#ifndef _XCL97DUM_HXX
#define _XCL97DUM_HXX


#include "excrecds.hxx"


// --- class ExcDummy8_xx --------------------------------------------

class ExcDummy8_00a : public ExcDummyRec
{
private:
        static const BYTE   pMyData[];
        static const ULONG  nMyLen;
public:
    virtual ULONG               GetLen() const;
    virtual const BYTE*         GetData() const;
};


class ExcDummy8_00b : public ExcDummyRec
{
private:
        static const BYTE   pMyData[];
        static const ULONG  nMyLen;
public:
    virtual ULONG               GetLen() const;
    virtual const BYTE*         GetData() const;
};


class ExcDummy8_040 : public ExcDummyRec
{
private:
        static const BYTE   pMyData[];
        static const ULONG  nMyLen;
public:
    virtual ULONG               GetLen() const;
    virtual const BYTE*         GetData() const;
};


class ExcDummy8_041 : public ExcDummyRec
{
private:
        static const BYTE   pMyData[];
        static const ULONG  nMyLen;
public:
    virtual ULONG               GetLen() const;
    virtual const BYTE*         GetData() const;
};


class ExcDummy8_01 : public ExcDummyRec
{
private:
        static const BYTE   pMyData[];
        static const ULONG  nMyLen;
public:
    virtual ULONG               GetLen() const;
    virtual const BYTE*         GetData() const;
};


class ExcDummy8_Fm : public ExcDummyRec
{
private:
        static const BYTE   pMyData[];
        static const ULONG  nMyLen;
public:
    virtual ULONG               GetLen() const;
    virtual const BYTE*         GetData() const;
};


class ExcDummy8_XF : public ExcDummyRec
{
private:
        static const BYTE   pMyData[];
        static const ULONG  nMyLen;
public:
    virtual ULONG               GetLen() const;
    virtual const BYTE*         GetData() const;
};


class ExcDummy8_Style : public ExcDummyRec
{
private:
        static const BYTE   pMyData[];
        static const ULONG  nMyLen;
public:
    virtual ULONG               GetLen() const;
    virtual const BYTE*         GetData() const;
};


class ExcDummy8_UsesElfs : public ExcDummyRec
{
private:
        static const BYTE   pMyData[];
        static const ULONG  nMyLen;
public:
    virtual ULONG               GetLen() const;
    virtual const BYTE*         GetData() const;
};


class ExcDummy8_Country : public ExcDummyRec
{
private:
        static const BYTE   pMyData[];
        static const ULONG  nMyLen;
public:
    virtual ULONG               GetLen() const;
    virtual const BYTE*         GetData() const;
};


class ExcDummy8_02 : public ExcDummyRec
{
private:
        static const BYTE   pMyData[];
        static const ULONG  nMyLen;
public:
    virtual ULONG               GetLen() const;
    virtual const BYTE*         GetData() const;
};



#endif // _XCL97DUM_HXX
