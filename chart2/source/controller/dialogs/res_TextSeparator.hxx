/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
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

    OUString GetValue() const;
    void SetValue( const OUString& rSeparator );
    void SetDefault();

private:
    FixedText   m_aFT_Separator;
    ListBox     m_aLB_Separator;

    ::std::map< OUString, sal_uInt16 > m_aEntryMap;

    const sal_uInt16 m_nDefaultPos;
};

//.............................................................................
} //namespace chart
//.............................................................................

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
