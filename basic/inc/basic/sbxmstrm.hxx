/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sbxmstrm.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 12:55:05 $
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

#ifndef _SBXMSTRM_HXX
#define _SBXMSTRM_HXX 1

#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif

#ifndef _SBXDEF_HXX
#include <basic/sbxdef.hxx>
#endif
#ifndef _SBXCORE_HXX
#include <basic/sbxcore.hxx>
#endif

SV_DECL_REF(SbxMemoryStream)

class SbxMemoryStream : public SbxBase, public SvMemoryStream
{
  public:
    SbxMemoryStream(ULONG nInitSize_=512, ULONG nResize_=64) :
        SvMemoryStream(nInitSize_,nResize_) {}
    ~SbxMemoryStream();

    virtual SbxDataType  GetType()  const;
};

SV_IMPL_REF(SbxMemoryStream)

#endif
