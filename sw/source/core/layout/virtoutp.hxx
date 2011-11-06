/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _VIRTOUTP_HXX
#define _VIRTOUTP_HXX

#include <vcl/virdev.hxx>

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
    sal_uInt16          nCount;

    sal_Bool DoesFit( const Size &rOut );

public:
    SwLayVout() : pSh(0), pOut(0), pVirDev(0), aSize(0, VIRTUALHEIGHT), nCount(0) {}
    ~SwLayVout() { delete pVirDev; }

    /// OD 27.09.2002 #103636# - change 2nd parameter <rRect> - no longer <const>
    void Enter( ViewShell *pShell, SwRect &rRect, sal_Bool bOn );
    void Leave() { --nCount; Flush(); }

    void SetOrgRect( SwRect &rRect ) { aOrgRect = rRect; }
    const SwRect& GetOrgRect() const { return aOrgRect; }

    sal_Bool IsFlushable() { return 0 != pOut; }
    void _Flush();
    void Flush() { if( pOut ) _Flush(); }
};



#endif
