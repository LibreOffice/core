/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unoclbck.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2006-08-14 16:23:57 $
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
#ifndef _UNOCLBCK_HXX
#define _UNOCLBCK_HXX
#ifndef _CALBCK_HXX
#include <calbck.hxx>
#endif

class SwXReferenceMark;
class SwFmtRefMark;
class SwFmtFtn;
class SwXFootnote;
class SwTOXMark;
class SwXDocumentIndexMark;

class SwUnoCallBack : public SwModify
{
public:
    SwUnoCallBack(SwModify *pToRegisterIn);
    virtual ~SwUnoCallBack();

    // returns the API object of a reference mark if available
    SwXReferenceMark*   GetRefMark(const SwFmtRefMark& rMark);
    SwXFootnote*        GetFootnote(const SwFmtFtn& rMark);
    SwXDocumentIndexMark* GetTOXMark(const SwTOXMark& rMark);
};
#endif
