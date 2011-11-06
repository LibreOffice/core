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



#ifndef _VCL_INPUTCTX_HXX
#define _VCL_INPUTCTX_HXX

#include <vcl/sv.h>
#include <vcl/dllapi.h>
#include <vcl/font.hxx>

// ----------------------
// - InputContext-Flags -
// ----------------------

#define INPUTCONTEXT_TEXT               ((sal_uLong)0x00000001)
#define INPUTCONTEXT_EXTTEXTINPUT       ((sal_uLong)0x00000002)
#define INPUTCONTEXT_EXTTEXTINPUT_ON    ((sal_uLong)0x00000004)
#define INPUTCONTEXT_EXTTEXTINPUT_OFF   ((sal_uLong)0x00000008)

// ----------------
// - InputContext -
// ----------------

class VCL_DLLPUBLIC InputContext
{
private:
    Font            maFont;
    sal_uLong           mnOptions;

public:
                    InputContext() { mnOptions = 0; }
                    InputContext( const InputContext& rInputContext ) :
                        maFont( rInputContext.maFont )
                    { mnOptions = rInputContext.mnOptions; }
                    InputContext( const Font& rFont, sal_uLong nOptions = 0 ) :
                        maFont( rFont )
                    { mnOptions = nOptions; }

    void            SetFont( const Font& rFont ) { maFont = rFont; }
    const Font&     GetFont() const { return maFont; }

    void            SetOptions( sal_uLong nOptions ) { mnOptions = nOptions; }
    sal_uLong           GetOptions() const { return mnOptions; }

    InputContext&   operator=( const InputContext& rInputContext );
    sal_Bool            operator==( const InputContext& rInputContext ) const;
    sal_Bool            operator!=( const InputContext& rInputContext ) const
                        { return !(InputContext::operator==( rInputContext )); }
};

inline InputContext& InputContext::operator=( const InputContext& rInputContext )
{
    maFont      = rInputContext.maFont;
    mnOptions   = rInputContext.mnOptions;
    return *this;
}

inline sal_Bool InputContext::operator==( const InputContext& rInputContext ) const
{
    return ((mnOptions  == rInputContext.mnOptions) &&
            (maFont     == rInputContext.maFont));
}

#endif // _VCL_INPUTCTX_HXX
