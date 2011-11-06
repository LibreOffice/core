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



#ifndef _PORTOX_HXX
#define _PORTOX_HXX

#include "portxt.hxx"
//#include "porglue.hxx"

/*************************************************************************
 *                      class SwToxPortion
 *************************************************************************/

class SwToxPortion : public SwTxtPortion
{
public:
    inline  SwToxPortion(){ SetWhichPor( POR_TOX ); }
    virtual void Paint( const SwTxtPaintInfo &rInf ) const;
    OUTPUT_OPERATOR
};

/*************************************************************************
 *                      class SwIsoToxPortion
 *************************************************************************/

class SwIsoToxPortion : public SwToxPortion
{
    KSHORT nViewWidth;

public:
            SwIsoToxPortion();
    virtual sal_Bool Format( SwTxtFormatInfo &rInf );
    virtual void Paint( const SwTxtPaintInfo &rInf ) const;
    virtual SwLinePortion *Compress();
    virtual KSHORT GetViewWidth( const SwTxtSizeInfo &rInf ) const;

    // Accessibility: pass information about this portion to the PortionHandler
    virtual void HandlePortion( SwPortionHandler& rPH ) const;

    OUTPUT_OPERATOR
};

/*************************************************************************
 *                  inline - Implementations
 *************************************************************************/

CLASSIO( SwToxPortion )
CLASSIO( SwIsoToxPortion )


#endif
