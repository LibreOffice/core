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



#ifndef _SV_SALLAYOUT_H
#define _SV_SALLAYOUT_H

#if 0
#include <sv.h>
#include <sallayout.hxx>

class ImplOs2FontEntry;

class Os2SalLayout : public GenericSalLayout
{
public:
                    Os2SalLayout( HPS);
    virtual bool    LayoutText( ImplLayoutArgs& );
    virtual void    AdjustLayout( ImplLayoutArgs& );
    virtual void    DrawText( SalGraphics& ) const;

protected:
    HPS                 mhPS;
    FONTMETRICS         FontMetrics;
    int                 mnBaseAdv;

private:
    int             mnGlyphCount;
    int             mnCharCount;
    sal_Unicode*    mpOutGlyphs;
    int*            mpGlyphAdvances;    // if possible this is shared with mpGlyphAdvances[]
    int*            mpGlyphOrigAdvs;
    int*            mpCharWidths;       // map rel char pos to char width
    int*            mpChars2Glyphs;     // map rel char pos to abs glyph pos
    int*            mpGlyphs2Chars;     // map abs glyph pos to abs char pos
    bool*           mpGlyphRTLFlags;    // BiDi status for glyphs: true=>RTL
    mutable long    mnWidth;
    bool            mbDisableGlyphs;

    int             mnNotdefWidth;
    BYTE            mnCharSet;

};
#endif

#endif // _SV_SALLAYOUT_H
