/*************************************************************************
 *
 *  $RCSfile: salvd.h,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:41 $
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
//*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-//
//                                                                            //
// (C) 1997 Star Division GmbH, Hamburg, Germany                              //
//                                                                            //
// $Revision: 1.1.1.1 $  $Author: hr $  $Date: 2000-09-18 17:05:41 $    //
//                                                                            //
// $Workfile:   salvd.h  $                                                    //
//  $Modtime:   09 Aug 1997 00:15:48  $                                       //
//                                                                            //
//*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-//

#ifndef _SV_SALVD_H
#define _SV_SALVD_H

// -=-= #includes -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#ifndef _SALSTD_HXX
#include <salstd.hxx>
#endif

// -=-= forwards -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class SalDisplay;
class SalGraphics;
class SalVirtualDevice;

// -=-= SalVirDevData -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class SalVirDevData
{
    friend  class           SalVirtualDevice;

            SalDisplay     *pDisplay_;
            SalGraphics    *pGraphics_;         // current frame graphics

            Pixmap          hDrawable_;

            int             nDX_;
            int             nDY_;
            USHORT          nDepth_;
            BOOL            bGraphics_;         // is Graphics used

    inline                  SalVirDevData();
    inline                  ~SalVirDevData();

public:
            BOOL            Init( SalDisplay *pDisplay,
                                  long nDX, long nDY,
                                  USHORT nBitCount );
    inline  void            InitGraphics( SalVirtualDevice *pVD,
                                          SalGraphics      *pGraphics );

    inline  Display        *GetXDisplay() const;
    inline  SalDisplay     *GetDisplay() const;
    inline  BOOL            IsDisplay() const;
    inline  Pixmap          GetDrawable() const { return hDrawable_; }
    inline  USHORT          GetDepth() const { return nDepth_; }
};

#ifdef _SV_SALDISP_HXX

inline void SalVirDevData::InitGraphics( SalVirtualDevice *pVD,
                                         SalGraphics      *pGraphics )
{ pGraphics_->maGraphicsData.Init( pVD, pGraphics ); }

inline Display *SalVirDevData::GetXDisplay() const
{ return pDisplay_->GetDisplay(); }

inline SalDisplay *SalVirDevData::GetDisplay() const
{ return pDisplay_; }

inline BOOL SalVirDevData::IsDisplay() const
{ return pDisplay_->IsDisplay(); }

#endif

#endif // _SV_SALVD_H

