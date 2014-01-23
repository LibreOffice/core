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

// =======================================================================

class AtsTextStyle
:   public ImplMacTextStyle
{
public:
    explicit    AtsTextStyle( const ImplFontSelectData& );
    virtual     ~AtsTextStyle( void );

    virtual SalLayout* GetTextLayout( void ) const;

    virtual void    GetFontMetric( float fDPIY, ImplFontMetricData& ) const;
    virtual bool    GetGlyphBoundRect( sal_GlyphId, Rectangle& ) const;
    virtual bool    GetGlyphOutline( sal_GlyphId, basegfx::B2DPolyPolygon& ) const;

    virtual void    SetTextColor( const RGBAColor& );

private:
    /// ATSU text style object
    ATSUStyle   maATSUStyle;
};

// =======================================================================

