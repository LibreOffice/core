/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: jobconst.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2007-07-11 15:01:51 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

//________________________________
//  my own includes

#include <jobs/jobconst.hxx>

//________________________________
//  interface includes

//________________________________
//  includes of other projects

//________________________________
//  namespace

namespace framework{

//________________________________
//  non exported const

//________________________________
//  non exported definitions

//________________________________
//  declarations

const ::rtl::OUString JobConst::ANSWER_DEACTIVATE_JOB()
{
    static const ::rtl::OUString PROP = ::rtl::OUString::createFromAscii("Deactivate");
    return PROP;
}

const ::rtl::OUString JobConst::ANSWER_SAVE_ARGUMENTS()
{
    static const ::rtl::OUString PROP = ::rtl::OUString::createFromAscii("SaveArguments");
    return PROP;
}

const ::rtl::OUString JobConst::ANSWER_SEND_DISPATCHRESULT()
{
    static const ::rtl::OUString PROP = ::rtl::OUString::createFromAscii("SendDispatchResult");
    return PROP;
}

} // namespace framework
