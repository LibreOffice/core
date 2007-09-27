/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: virtoutp.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 09:07:25 $
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
#ifndef _VIRTOUTP_HXX
#define _VIRTOUTP_HXX

#ifndef _SV_VIRDEV_HXX //autogen
#include <vcl/virdev.hxx>
#endif

#include "swtypes.hxx"      // UCHAR
#include "swrect.hxx"       // SwRect

class ViewShell;
#define VIRTUALHEIGHT 64

/*************************************************************************
 *                      class SwTxtVout
 *************************************************************************/

class SwLayVout
{
    friend void _FrmFinit();    //loescht das Vout
private:
    ViewShell*      pSh;
    OutputDevice*   pOut;
    VirtualDevice*  pVirDev;
    SwRect          aRect;
    SwRect          aOrgRect;
    Size            aSize;
    USHORT          nCount;

    BOOL DoesFit( const Size &rOut );

public:
    SwLayVout() : pSh(0), pOut(0), pVirDev(0), aSize(0, VIRTUALHEIGHT), nCount(0) {}
    ~SwLayVout() { delete pVirDev; }

    /// OD 27.09.2002 #103636# - change 2nd parameter <rRect> - no longer <const>
    void Enter( ViewShell *pShell, SwRect &rRect, BOOL bOn );
    void Leave() { --nCount; Flush(); }

    void SetOrgRect( SwRect &rRect ) { aOrgRect = rRect; }
    const SwRect& GetOrgRect() const { return aOrgRect; }

    BOOL IsFlushable() { return 0 != pOut; }
    void _Flush();
    void Flush() { if( pOut ) _Flush(); }
};



#endif
