/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: res_TextSeparator.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-10-22 16:47:31 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef CHART2_RES_TEXTSEPARATOR_HXX
#define CHART2_RES_TEXTSEPARATOR_HXX

// header for class FixedText
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
// header for class ListBox
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif

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
                                 , long nWantedRightBorder /*use -1 to indicate that this can be automatic*/ );

    ::rtl::OUString GetValue() const;
    void SetValue( const ::rtl::OUString& rSeparator );
    void SetDefault();

private:
    FixedText   m_aFT_Separator;
    ListBox     m_aLB_Separator;

    ::std::map< ::rtl::OUString, USHORT > m_aEntryMap;

    const USHORT m_nDefaultPos;
};

//.............................................................................
} //namespace chart
//.............................................................................

#endif
