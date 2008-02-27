/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: versionhelper.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: lla $ $Date: 2008-02-27 16:19:03 $
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

#ifndef AUTOREGISTER_HXX
#define AUTOREGISTER_HXX

#include <stdio.h>
#include <vector>
#include <ostream>
#include <iostream>

#include <rtl/ustring.hxx>
#include <rtl/string.hxx>

#include "dynamicregister.hxx"

// -----------------------------------------------------------------------------
// ---------------------------------- Version ----------------------------------
// -----------------------------------------------------------------------------
// extern "C" const struct VersionInfo *GetVersionInfo();

// this is a Hack, but at the moment, no change to handle this behaviour better.
struct VersionInfo
{
    const char* pTime;
    const char* pDate;
    const char* pUpd;
    const char* pMinor;
    const char* pBuild;
    const char* pInpath;
};
typedef const struct VersionInfo* (*FktGetVersionInfoPtr)();

// -----------------------------------------------------------------------------
class VersionHelper : public DynamicLibraryHelper
{
    const VersionInfo *m_pInfo;

public:
    VersionHelper(rtl::OUString const& _sDLLName, GetOpt & _aOptions);
//  void print(std::ostream& _aStream);
    void printall(FILE*);

    rtl::OString getTime() const;
    rtl::OString getDate() const;
    rtl::OString getUpd() const;
    rtl::OString getMinor() const;
    rtl::OString getBuild() const;
    rtl::OString getInpath() const;

    bool isOk() const;
};

/** insertion operator for easy output */
// std::ostream &operator <<( std::ostream &stream,
//                           VersionHelper &version );


#endif

