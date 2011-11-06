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


#ifndef _VIEWFUNC_HXX
#define _VIEWFUNC_HXX
#include <tools/solar.h>

class ImageButton;
class Point;
class SfxItemSet;
class SfxPrinter;
class SfxTabPage;
class Size;
class SvxRuler;
class SwScrollbar;
class ViewShell;
class Window;
class SwWrtShell;

// folgende Funktionen stehen im viewprt.cxx
void SetPrinter( IDocumentDeviceAccess*, SfxPrinter*, sal_Bool bWeb );
SfxTabPage* CreatePrintOptionsPage( Window*, const SfxItemSet& );
void SetAppPrintOptions( ViewShell* pSh, sal_Bool bWeb );

// folgende Funktionen stehen im viewport.cxx
void ViewResizePixel( const Window &rRef,
                    const Point &rOfst,
                    const Size &rSize,
                    const Size &rEditSz,
                    const sal_Bool bInner,
                    SwScrollbar& rVScrollbar,
                    SwScrollbar& rHScrollbar,
                    ImageButton* pPageUpBtn,
                    ImageButton* pPageDownBtn,
                    ImageButton* pNaviBtn,
                    Window& rScrollBarBox,
                    SvxRuler* pVLineal = 0,
                    SvxRuler* pHLineal = 0,
                    sal_Bool bWebView = sal_False,
                    sal_Bool bVRulerRight = sal_False );


#endif
