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



#ifndef _PptEscherEX_HXX
#define _PptEscherEX_HXX
#include <filter/msfilter/escherex.hxx>

// ---------------------------------------------------------------------------------------------
// Werte fuer den sal_uLong im PPT_PST_TextHeaderAtom
enum PPT_TextHeader
{
    PPTTH_TITLE,
    PPTTH_BODY,
    PPTTH_NOTES,
    PPTTH_NOTUSED,
    PPTTH_OTHER,       // Text in a Shape
    PPTTH_CENTERBODY,  // Subtitle in Title-Slide
    PPTTH_CENTERTITLE, // Title in Title-Slide
    PPTTH_HALFBODY,    // Body in two-column slide
    PPTTH_QUARTERBODY  // Body in four-body slide
};

// ---------------------------------------------------------------------------------------------

class PptEscherEx : public EscherEx
{
        sal_uInt32  ImplDggContainerSize();
        void        ImplWriteDggContainer( SvStream& rSt );

        sal_uInt32  ImplOptAtomSize();
        void        ImplWriteOptAtom( SvStream& rSt );

        sal_uInt32  ImplSplitMenuColorsAtomSize();
        void        ImplWriteSplitMenuColorsAtom( SvStream& rSt );

    public:

                PptEscherEx( SvStream& rOut, const rtl::OUString& );
                ~PptEscherEx();

        void    OpenContainer( sal_uInt16 n_EscherContainer, int nRecInstance = 0 );
        void    CloseContainer();

        sal_uInt32 EnterGroup(
            const basegfx::B2DRange* pObjectRange,
            SvMemoryStream* pClientData );

        sal_uInt32  DrawingGroupContainerSize();
        void    WriteDrawingGroupContainer( SvStream& rSt );

        using EscherEx::EnterGroup;
};


#endif
