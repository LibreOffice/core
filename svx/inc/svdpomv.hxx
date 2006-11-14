/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svdpomv.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 12:53:51 $
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

#error dieser Header entfaellt nun!

#ifndef _SVDPOMV_HXX
#define _SVDPOMV_HXX

#ifndef _SVDMRKV_HXX
#include "svdmrkv.hxx"
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  @@@@@   @@@@  @@  @@  @@  @@   @@  @@@@  @@@@@  @@  @@  @@ @@ @@ @@@@@ @@   @@
//  @@  @@ @@  @@ @@  @@  @@  @@@ @@@ @@  @@ @@  @@ @@ @@   @@ @@ @@ @@    @@ @ @@
//  @@@@@  @@  @@ @@   @@@@   @@@@@@@ @@@@@@ @@@@@  @@@@    @@@@@ @@ @@@@  @@@@@@@
//  @@     @@  @@ @@    @@    @@ @ @@ @@  @@ @@  @@ @@ @@    @@@  @@ @@    @@@ @@@
//  @@      @@@@  @@@@@ @@    @@   @@ @@  @@ @@  @@ @@  @@    @   @@ @@@@@ @@   @@
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class SdrPolyMarkView: public SdrMarkView {
private:
#ifndef _SVDRAW_HXX
    void ImpClearVars();
#endif
public:
    SdrPolyMarkView(SdrModel* pModel1, OutputDevice* pOut = 0L);
    ~SdrPolyMarkView();
};

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SVDPOMV_HXX

