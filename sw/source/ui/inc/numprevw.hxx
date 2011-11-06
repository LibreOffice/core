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



#ifndef _NUMPREVW_HXX
#define _NUMPREVW_HXX


#include <vcl/window.hxx>

class SwNumRule;
/*-----------------02.12.97 10:31-------------------

--------------------------------------------------*/

class NumberingPreview : public Window
{
    const SwNumRule*    pActNum;
    Font                aStdFont;
    long                nPageWidth;
    const String*       pOutlineNames;
    sal_Bool                bPosition;
    sal_uInt16              nActLevel;

    protected:
        virtual void        Paint( const Rectangle& rRect );

    public:
        NumberingPreview(Window* pParent, const ResId& rResId ) :
            Window(pParent, rResId),
            pActNum(0),nPageWidth(0), pOutlineNames(0), bPosition(sal_False), nActLevel(USHRT_MAX) {}
        ~NumberingPreview();

        void    SetNumRule(const SwNumRule* pNum)
                    {pActNum = pNum; Invalidate();};
        void    SetPageWidth(long nPgWidth)
                                {nPageWidth = nPgWidth;}
        void    SetOutlineNames(const String* pNames)
                        {pOutlineNames = pNames;}
        void    SetPositionMode()
                        { bPosition = sal_True;}
        void    SetLevel(sal_uInt16 nSet) {nActLevel = nSet;}

};

#endif


