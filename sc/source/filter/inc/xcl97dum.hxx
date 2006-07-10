/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xcl97dum.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 13:53:06 $
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

#ifndef _XCL97DUM_HXX
#define _XCL97DUM_HXX


#include "excrecds.hxx"


// --- class ExcDummy8_xx --------------------------------------------

class ExcDummy8_00a : public ExcDummyRec
{
private:
    static const BYTE       pMyData[];
    static const sal_Size   nMyLen;
public:
    virtual sal_Size        GetLen() const;
    virtual const BYTE*     GetData() const;
};


class ExcDummy8_00b : public ExcDummyRec
{
private:
    static const BYTE       pMyData[];
    static const sal_Size   nMyLen;
public:
    virtual sal_Size        GetLen() const;
    virtual const BYTE*     GetData() const;
};


class ExcDummy8_040 : public ExcDummyRec
{
private:
    static const BYTE       pMyData[];
    static const sal_Size   nMyLen;
public:
    virtual sal_Size        GetLen() const;
    virtual const BYTE*     GetData() const;
};


class ExcDummy8_041 : public ExcDummyRec
{
private:
    static const BYTE       pMyData[];
    static const sal_Size   nMyLen;
public:
    virtual sal_Size        GetLen() const;
    virtual const BYTE*     GetData() const;
};


class ExcDummy8_02 : public ExcDummyRec
{
private:
    static const BYTE       pMyData[];
    static const sal_Size   nMyLen;
public:
    virtual sal_Size        GetLen() const;
    virtual const BYTE*     GetData() const;
};


class XclRefmode : public ExcDummyRec
{
private:
    static const BYTE       pMyData[];
    static const sal_Size   nMyLen;
public:
    virtual sal_Size        GetLen( void ) const;
    virtual const BYTE*     GetData( void ) const;
};


#endif // _XCL97DUM_HXX
