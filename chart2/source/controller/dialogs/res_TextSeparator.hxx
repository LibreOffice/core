/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef CHART2_RES_TEXTSEPARATOR_HXX
#define CHART2_RES_TEXTSEPARATOR_HXX

// header for class FixedText
#include <vcl/fixed.hxx>
// header for class ListBox
#include <vcl/lstbox.hxx>

#include <map>

//.............................................................................
namespace chart
{
//.............................................................................

class TextSeparatorResources
{
public:
    TextSeparatorResources( Window* pParent );
    virtual ~TextSeparatorResources();

    void Show( bool bShow );
    void Enable( bool bEnable );

    void PositionBelowControl( const Window& rWindow );
    void AlignListBoxWidthAndXPos( long nWantedLeftBorder /*use -1 to indicate that this can be automatic*/
                                 , long nWantedRightBorder /*use -1 to indicate that this can be automatic*/
                                 , long nMinimumListBoxWidth /*use -1 to indicate that this can be automatic*/ );
    Point GetCurrentListBoxPosition() const;
    Size GetCurrentListBoxSize() const;

    ::rtl::OUString GetValue() const;
    void SetValue( const ::rtl::OUString& rSeparator );
    void SetDefault();

private:
    FixedText   m_aFT_Separator;
    ListBox     m_aLB_Separator;

    ::std::map< ::rtl::OUString, sal_uInt16 > m_aEntryMap;

    const sal_uInt16 m_nDefaultPos;
};

//.............................................................................
} //namespace chart
//.............................................................................

#endif
